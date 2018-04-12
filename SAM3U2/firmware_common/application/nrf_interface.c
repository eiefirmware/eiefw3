/*!*********************************************************************************************************************
@file nrf_interface.c                                                                
@brief nRF interface application.

Provides a communication link between the SAM3U2 and nRF51422 processors.
The communication uses a SPI slave with flow control on the SAM3U2.
A simple protocol will be used for the messages:

[START_BYTE, LENGTH, COMMAND, DATA0, …, DATAn]
where,
START_BYTE = 0x5A
LENGTH = 1 + the number of data bytes

Messages will always be complete when transmitted or received.
The slave will initialize with SRDY asserted so it is ready for a message.
The master shall not assert CS if SRDY is not asserted.
The master will assert CS when it is clocking a message.
The slave shall deassert SRDY at the start of a message transfer.
The slave shall not assert MRDY if a message is being clocked.


Reserved Commands nRF51422 to SAM3U2 (0x00 - 0x1F):  
CMD  ARG_BYTE(s)        FUNCTION
--------------------------------------------------------
0x01 LED,STATE          LED number, ON(1) OFF (0)
0x02 LCD Message1       Null-terminated string to forward to LCD line 1 (line is erased first)
0x03 LCD Message2       Null-terminated string to forward to LCD line 2 (line is erased first)
0x04 Debug Message      Null-terminated string to forward to DebugPrintf();
0x05 BUZZER,FREQ        Activate BUZZER (1 or 2) at FREQ (0 for off)


Reserved Commands SAM3U2 to nRF51422 (0x20 - 0x3F):  
CMD  ARG_BYTE(s)           FUNCTION
--------------------------------------------------------
0x21 BUTTON_PRESSED        BUTTON number (works only with WasButtonPressed();

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void nrfInterfaceInitialize(void)
- void nrfInterfaceRunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>nrfInterface"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32nrfInterfaceFlags;                      /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */

extern volatile u32 NRF_SSP_FLAGS;                        /* From configuration.h */

extern u8 G_au8UtilMessageOK[];                           /*!< @brief From utilities.c */
extern u8 G_au8UtilMessageFAIL[];                         /*!< @brief From utilities.c */
extern u8 G_au8UtilMessageON[];                           /*!< @brief From utilities.c */
extern u8 G_au8UtilMessageOFF[];                          /*!< @brief From utilities.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "nrfInterface_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type nrfInterface_pfStateMachine;               /*!< @brief The state machine function pointer */
static u32 nrfInterface_u32Timeout;                           /*!< @brief Timeout counter used across states */
static u32 nrfInterface_u32Flags;
static u32 nrfInterface_u32MsgToken;

static SspConfigurationType nrfInterface_sSspConfig;          /* Configuration information for SSP peripheral */
static SspPeripheralType* nrfInterface_Ssp;                   /* Pointer to SSP peripheral object */

static u8  nrfInterface_au8RxBuffer[U8_NRF_BUFFER_SIZE];      /* Space for verified received ANT messages */
static u8* nrfInterface_pu8RxBufferNextChar;                  /* Pointer to next char to be written in the AntRxBuffer */

static u8 nrfInterface_u8RxBytes;                             /* Bytes received in current transfer */
static u8 nrfInterface_u8TxBytes;                             /* Bytes sent in current transfer */

static u8 nrfInterface_au8Message[U8_NRF_BUFFER_SIZE];        /* Latest received message */

static u8 nrfInterface_au8TestResponse[] = {NRF_SYNC_BYTE, NRF_CMD_TEST_RESPONSE_LENGTH, NRF_CMD_TEST_RESPONSE};


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn u8 nrfNewMessageCheck(void)

@brief Determines if a new message is available from the nRF Interface.

The function returns the command number of the message in memory so the
client can determine if the command belongs to it.  The assumption is that 
more than one task would not be interested in the same message as a different 
task.

Requires:
- nrfInterface_u32Flags _NEW_APP_MESSAGE is set it a new message is available

Promises:
- Returns the command number of the message

*/
u8 nrfNewMessageCheck(void)
{
  /* Return the command number if the new message flag is set */
  if(nrfInterface_u32Flags & _NEW_APP_MESSAGE)
  {
    return nrfInterface_au8Message[NRF_COMMAND_INDEX];
  }
  
  /* Otherwise return no command */
  return NRF_CMD_EMPTY;
  
} /* end nrfNewMessageCheck() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn void nrfGetAppMessage(u8* pu8AppBuffer_);
@brief Transfer the current message to task.

The assumption is that the task has confirmed the message number and has
an appropriately sized buffer waiting.

Requires:
@PARAM pu8AppBuffer_ is a buffer in the task with enough space for the message

Promises:
- Copies all valid message bytes from nrfInterface_au8Message to pu8AppBuffer_
- _NEW_APP_MESSAGE is cleared

*/
void nrfGetAppMessage(u8* pu8AppBuffer_)
{
  memcpy(pu8AppBuffer_, (const u8*)nrfInterface_au8Message, 
         (nrfInterface_au8Message[NRF_LENGTH_INDEX] + NRF_OVERHEAD_BYTES) );
  
  nrfInterface_u32Flags &= ~_NEW_APP_MESSAGE;
  
} /* end nrfGetAppMessage() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn u32 nrfQueueMessage(u8* pu8AppMessage_)
@brief Queues a message from a task to the nRF processor

Requires:
@PARAM pu8AppMessage_ is a correctly formatted message to the 51422

Promises:
- Adds the message to the transmit queue.
- Returns the message token.

*/
u32 nrfQueueMessage(u8* pu8AppMessage_)
{
  u32 u32Token;
  u8 u8Length;
  
  u8Length = *(pu8AppMessage_ + NRF_LENGTH_INDEX) + NRF_OVERHEAD_BYTES;
  u32Token = SspWriteData(nrfInterface_Ssp, u8Length, pu8AppMessage_);

  return u32Token;
  
} /* end nrfQueueMessage() */


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void nrfInterfaceInitialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void nrfInterfaceInitialize(void)
{
  /* Put the nRF51422 in reset. If the nRF51422 was debugging, this will
  disconnect the debugger but NOT reset the processor because the processor
  will be in debug mode.  The EIE board will have to be power cycled to return
  to normal reset operation. */
  AT91C_BASE_PIOB->PIO_CODR = PB_21_ANT_RESET;
  AT91C_BASE_PIOB->PIO_OER  = PB_21_ANT_RESET;

  /* Announce on the debug port that nrfInterface setup is starting and intialize pointers */
  DebugPrintf("Starting nRF Interface...");
  
  /* Initialize buffer pointers */  
  nrfInterface_pu8RxBufferNextChar = nrfInterface_au8RxBuffer;
  nrfInterface_u8RxBytes = 0;
  nrfInterface_u8TxBytes = 0;
 
  /* Initialize SRDY and MRDY */
  NRF_MRDY_DEASSERT();
  NRF_SRDY_DEASSERT();

  /* Configure the SSP resource to be used for the application */
  nrfInterface_sSspConfig.SspPeripheral      = NRF_SPI;
  nrfInterface_sSspConfig.pCsGpioAddress     = NRF_SPI_CS_GPIO;
  nrfInterface_sSspConfig.u32CsPin           = NRF_SPI_CS_PIN;
  nrfInterface_sSspConfig.eBitOrder          = LSB_FIRST;
  nrfInterface_sSspConfig.eSspMode           = SPI_SLAVE_FLOW_CONTROL;
  nrfInterface_sSspConfig.fnSlaveTxFlowCallback = nrfTxFlowControlCallback;
  nrfInterface_sSspConfig.fnSlaveRxFlowCallback = nrfRxFlowControlCallback;
  nrfInterface_sSspConfig.pu8RxBufferAddress = nrfInterface_au8RxBuffer;
  nrfInterface_sSspConfig.ppu8RxNextByte     = &nrfInterface_pu8RxBufferNextChar;
  nrfInterface_sSspConfig.u16RxBufferSize    = U8_NRF_BUFFER_SIZE;

  nrfInterface_Ssp = SspRequest(&nrfInterface_sSspConfig);
  NRF_SSP_FLAGS = 0;
  
  /* Release the nRF51422 reset by returning the pin to an input */
  AT91C_BASE_PIOB->PIO_ODR = PB_21_ANT_RESET;
  
  /* Report status out the debug port */
  if(nrfInterface_Ssp != NULL)  
  {
    DebugPrintf(G_au8UtilMessageOK);

    /* Debugging state indication */
    LedPWM(WHITE, LED_PWM_5);
    LedOff(PURPLE);
    LedOff(BLUE);
    nrfInterface_pfStateMachine = nrfInterfaceSM_Idle;
  }
  else
  {
    /* SSP failed, so cannot communicate */
    DebugPrintf(G_au8UtilMessageFAIL);
    nrfInterface_pfStateMachine = nrfInterfaceSM_Error;
  }

} /* end nrfInterfaceInitialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void nrfInterfaceRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void nrfInterfaceRunActiveState(void)
{
  nrfInterface_pfStateMachine();

} /* end nrfInterfaceRunActiveState */


/*!-----------------------------------------------------------------------------
@fn  void nrfTxFlowControlCallback(void)
@brief Callback function during SPI transmit 

Note: Since this function is called from an ISR, it should execute as quickly as possible. 

Requires:
- NONE 

Promises:
-

*/
void nrfTxFlowControlCallback(void)
{
  /* Keep the flow control in this state - no harm in doing it every byte */
  NRF_MRDY_ASSERT();
  NRF_SRDY_DEASSERT();
  
  nrfInterface_u8TxBytes++;
 
} /* end nrfTxFlowControlCallback() */


/*!-----------------------------------------------------------------------------
@fn void nrfRxFlowControlCallback(void)
@brief Callback function to toggle flow control during reception.  

The peripheral task receiving the message must invoke this function after each byte.  

Note: Since this function is called from an ISR, it should execute as quickly as possible. 


Requires:
- ISRs are off already since this is totally not re-entrant
- A received byte was just written to the Rx buffer

Promises:
- nrfInterface_pu8RxBufferNextChar is advanced safely so it is ready to 
receive the next byte.  The buffer is not supposed to be circular so
_SSP_RX_OVERFLOW is flagged.  Wrap the pointer back, anyway.
*/
void nrfRxFlowControlCallback(void)
{
  /* Count the byte and move the pointer */
  NRF_SRDY_DEASSERT();
  nrfInterface_u8RxBytes++;
  
  nrfInterface_pu8RxBufferNextChar++;
  if(nrfInterface_pu8RxBufferNextChar == &nrfInterface_au8RxBuffer[U8_NRF_BUFFER_SIZE])
  {
    NRF_SSP_FLAGS |= _SSP_RX_OVERFLOW;
    nrfInterface_pu8RxBufferNextChar = &nrfInterface_au8RxBuffer[0];
  }
    
} /* end nrfRxFlowControlCallback() */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* Watch for a CS flag or wait for
a message to be queued to the nRF Interface */
static void nrfInterfaceSM_Idle(void)
{
  /* If the CS flag gets asserted, the NRF is ready to clock the system */
  if( IS_CS_ASSERTED() )
  {
    /* Note the start time for timeout checking */
    nrfInterface_u32Timeout = G_u32SystemTime1ms;
 
    /* If MRDY is asserted, then we are transmitting */
    if( IS_MRDY_ASSERTED() )
    {
      /* Ready to go, so assert SRDY. */
      for(u32 i = 0; i < 100; i++);
      NRF_SRDY_ASSERT();

     /* Debugging state indication */
      LedPWM(BLUE, LED_PWM_5);
      LedOff(WHITE);
      LedOff(PURPLE);
      nrfInterface_pfStateMachine = nrfInterfaceSM_Tx;
    }
    /* Otherwise we are receiving */
    else
    {
      /* Set flag and advance to Rx wait state */
      nrfInterface_u8RxBytes = 0;
      for(u32 i = 0; i < 100; i++);
      NRF_SRDY_ASSERT();
      
     /* Debugging state indication */
      LedPWM(PURPLE, LED_PWM_5);
      LedOff(WHITE);
      LedOff(BLUE);
      nrfInterface_pfStateMachine = nrfInterfaceSM_Rx;
    }
  } 
  
} /* end nrfInterfaceSM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Receive a message from the nRF Interface.  This state holds
until the message is completely received and the message is 
processed immediately. Message transmission is complete when the 
Master deasserts CS. Note that for short messages this will occur prior
to entering this state.
*/
static void nrfInterfaceSM_Rx(void)
{
  u8 u8Length;
    
  /* Debugging state indication */
  LedOff(WHITE);
  LedOn(PURPLE);
  LedOff(BLUE);

  /* Watch for CS to deassert */
  if( !IS_CS_ASSERTED() )
  {
    /* The message is complete.  Parse it out to determine what happens. */
    if(nrfInterface_au8RxBuffer[NRF_SYNC_INDEX] == NRF_SYNC_BYTE)
    {
      /* SYNC is verified, so check if this is a board command or 
      application message */
      if(nrfInterface_au8RxBuffer[NRF_COMMAND_INDEX] >= NRF_APP_MESSAGE_START)
      {
        /* An application message is posted to G_au8nrfInterfaceMessage */ 
        u8Length = nrfInterface_au8RxBuffer[NRF_LENGTH_INDEX];
        u8Length += NRF_OVERHEAD_BYTES;
        memcpy(nrfInterface_au8Message, (const u8*)nrfInterface_au8RxBuffer, u8Length);
        nrfInterface_u32Flags |= _NEW_APP_MESSAGE;
      } /* end APP message */
      else
      {
        /* Board commands */
        switch (nrfInterface_au8RxBuffer[NRF_COMMAND_INDEX])
        {
          case NRF_CMD_LED:
          {
            if(nrfInterface_au8RxBuffer[NRF_CMD_ARG2_INDEX])
            {
              LedOn((LedNumberType)nrfInterface_au8RxBuffer[NRF_CMD_ARG1_INDEX]);
            }
            else
            {
              LedOff((LedNumberType)nrfInterface_au8RxBuffer[NRF_CMD_ARG1_INDEX]);
            }
            break;
          }

          case NRF_CMD_TEST:
          {
            /* If a test message is received, send response back */
            DebugPrintf("\n\rSPI test message received\n\r");
            nrfInterface_u32MsgToken = nrfQueueMessage(nrfInterface_au8TestResponse);
            break;
          }
  
          default:
          {
            DebugPrintf("\n\rUnhandled nRF Command\n\r");
            break;
          }
          
        } /* end switch */
      } /* end board command */
    } /* end good SYNC byte */

    /* In all cases, reset the buffer pointer and counters */
    nrfInterface_pu8RxBufferNextChar = nrfInterface_au8RxBuffer;
    nrfInterface_u8RxBytes = 0;
    NRF_SRDY_DEASSERT();
    
   /* Debugging state indication */
    LedPWM(WHITE, LED_PWM_5);
    LedOff(PURPLE);
    LedOff(BLUE);

    nrfInterface_pfStateMachine = nrfInterfaceSM_Idle;
    
  } /* end !CS_ASSERTED */

#if 1
  /* Watch for timeout */
  if( IsTimeUp(&nrfInterface_u32Timeout, U32_TRX_TIMEOUT_MS) )
  {
    nrfInterface_u32Flags |= _RX_TIMEOUT;
    DebugPrintf("nRF Rx timeout\n\r");

    nrfInterface_pu8RxBufferNextChar = nrfInterface_au8RxBuffer;
    nrfInterface_u8RxBytes = 0;

   /* Debugging state indication */
    LedPWM(WHITE, LED_PWM_5);
    LedOff(PURPLE);
    LedOff(BLUE);
    nrfInterface_pfStateMachine = nrfInterfaceSM_Idle;

    NRF_SRDY_DEASSERT();
    ACK_CS_ASSERTED();

   /* Debugging state indication */
    LedPWM(WHITE, LED_PWM_5);
    LedOff(PURPLE);
    LedOff(BLUE);
    nrfInterface_pfStateMachine = nrfInterfaceSM_Idle;
  }
#endif
  
} /* end nrfInterfaceSM_Rx() */
                
 
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit a message through the nRF Interface.  This state holds
until the message transmission is complete and the 
Master deasserts CS based on the length byte provided in the message.
Note that for short messages this will occur prior to entering this state.
*/
static void nrfInterfaceSM_Tx(void)   
{
  /* Watch for CS to deassert */
  if( !IS_CS_ASSERTED() )
  {
    NRF_MRDY_DEASSERT();
    NRF_MRDY_DEASSERT();
    ACK_CS_ASSERTED();
    nrfInterface_u8TxBytes = 0;

   /* Debugging state indication */
    LedPWM(WHITE, LED_PWM_5);
    LedOff(PURPLE);
    LedOff(BLUE);
    nrfInterface_pfStateMachine = nrfInterfaceSM_Idle;
  } /* end !CS_ASSERTED */

#if 1
  /* Watch for timeout */
  if( IsTimeUp(&nrfInterface_u32Timeout, U32_TRX_TIMEOUT_MS) )
  {
    nrfInterface_u8TxBytes = 0;
    nrfInterface_u32Flags |= _TX_TIMEOUT;
    NRF_SRDY_DEASSERT();
    NRF_MRDY_DEASSERT();
    ACK_CS_ASSERTED();
    DebugPrintf("nRF Tx timeout\n\r");

   /* Debugging state indication */
    LedPWM(WHITE, LED_PWM_5);
    LedOff(PURPLE);
    LedOff(BLUE);
    nrfInterface_pfStateMachine = nrfInterfaceSM_Idle;
  }
#endif
  
} /* end nrfInterfaceSM_Tx() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void nrfInterfaceSM_Error(void)          
{
  
} /* end nrfInterfaceSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
