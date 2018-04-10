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
volatile u32 G_u32nrfInterfaceFlags;                          /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "nrfInterface_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type nrfInterface_pfStateMachine;               /*!< @brief The state machine function pointer */
//static u32 nrfInterface_u32Timeout;                           /*!< @brief Timeout counter used across states */
static u32 nrfInterface_u32Flags;

static SspConfigurationType nrfInterface_sSspConfig;          /* Configuration information for SSP peripheral */
static SspPeripheralType* nrfInterface_Ssp;                   /* Pointer to SSP peripheral object */

static u8  nrfInterface_au8RxBuffer[U8_NRF_BUFFER_SIZE];      /* Space for verified received ANT messages */
static u8* nrfInterface_pu8RxBufferNextChar;                  /* Pointer to next char to be written in the AntRxBuffer */
static u8* nrfInterface_pu8RxBufferCurrentChar;               /* Pointer to the current char in the AntRxBuffer */
static u8* nrfInterface_pu8RxBufferUnreadMsg;                 /* Pointer to unread chars in the AntRxBuffer */

static u8 nrfInterface_u8RxBytes;                           /* Unread chars in the AntRxBuffer */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

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
  /* Announce on the debug port that nrfInterface setup is starting and intialize pointers */
  DebugPrintf("Starting nRF Interface...");

  /* Initialize buffer pointers */  
  nrfInterface_pu8AntRxBufferNextChar    = nrfInterface_au8RxBuffer;
  nrfInterface_pu8AntRxBufferCurrentChar = nrfInterface_au8RxBuffer;
  nrfInterface_pu8AntRxBufferUnreadMsg   = nrfInterface_au8RxBuffer;
  nrfInterface_u8AntNewRxMessages = 0;
 
  /* Reset the 51422 and initialize SRDY and MRDY */
  SYNC_MRDY_DEASSERT();
  SYNC_SRDY_DEASSERT();

  /* Configure the SSP resource to be used for the application */
  nrfInterface_sSspConfig.SspPeripheral      = NRF_SPI;
  nrfInterface_sSspConfig.pCsGpioAddress     = NRF_SPI_CS_GPIO;
  nrfInterface_sSspConfig.u32CsPin           = NRF_SPI_CS_PIN;
  nrfInterface_sSspConfig.eBitOrder          = LSB_FIRST;
  nrfInterface_sSspConfig.eSspMode           = SPI_SLAVE_FLOW_CONTROL;
  nrfInterface_sSspConfig.fnSlaveTxFlowCallback = nrfTxFlowControlCallback;
  nrfInterface_sSspConfig.fnSlaveRxFlowCallback = nrfRxFlowControlCallback;
  nrfInterface_sSspConfig.pu8RxBufferAddress = Ant_au8AntRxBuffer;
  nrfInterface_sSspConfig.ppu8RxNextByte     = &nrfInterface_pu8RxBufferNextChar;
  nrfInterface_sSspConfig.u16RxBufferSize    = ANT_RX_BUFFER_SIZE;

  nrfInterface_Ssp = SspRequest(&nrfInterface_sSspConfig);
  NRF_SSP_FLAGS = 0;
  
  /* Report status out the debug port */
  if(nrfInterface_Ssp != NULL)  
  {
    DebugPrintf(G_au8UtilMessageOK);
    nrfInterface_pfStateMachine = nrfSM_Idle;
  }
  else
  {
    /* SSP failed, so cannot communicate */
    DebugPrintf(G_au8UtilMessageFAIL);
    nrfInterface_pfStateMachine = nrfSM_Error;
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
  /* Indicate the task is ready for data to be sent */
  NRF_SRDY_ASSERT();
  
  
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
- nrfInterface_pu8RxBufferNextChar is advanced safely so it is ready to receive the next byte
*/
void nrfRxFlowControlCallback(void)
{
  nrfInterface_pu8RxBufferNextChar++;
  
  
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
    /* MRDY is also asserted, then we are transmitting */
    if (IS_MRDY_ASSERTED)
    {
      /* MRDY can be cleared since the transaction is now in progress. */
      NRF_MRDY_DEASSERT();
      nrfInterface_pfStateMachine = nrfInterfaceSM_Tx;
    }
    /* Otherwise we are receiving */
    else
    {
      /* Set flag and advance to Rx wait state */
      NRF_SRDY_ASSERT();
      nrfInterface_pfStateMachine = nrfInterfaceSM_Rx;
    }
  }
  
  
} /* end nrfInterfaceSM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Receive a message from the nRF Interface.  This state holds
until the message is completely received and the message is 
processed immediately. Message transmission is complete when the 
Master releases CS.
*/
static void nrfInterfaceSM_Rx(void)          
{
  u8 u8Length;
  
  /* Watch for CS to deassert */
  if( !IS_CS_ASSERTED() )
  {
    /* The message is complete.  Parse it out to determine what happens. */
    if(nrfInterface_au8RxBuffer[NRF_SYNC_INDEX] == NRF_SYNC_BYTE)
    {
      u8Length = nrfInterface_au8RxBuffer[NRF_LENGTH_INDEX];
    }
  }
  
  /* In all cases, reset the buffer pointers */
  
} /* end nrfInterfaceSM_Error() */
                
                
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void nrfInterfaceSM_Error(void)          
{
  
} /* end nrfInterfaceSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
