/**********************************************************************************************************************
File: anttt.c                                                                

Description:
Implements TIC-TAC-TOE using data input from ANT, BLE, or SPI.

**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAnttt"
***********************************************************************************************************************/
/* New variables */
u32 G_u32AntttFlags;                                     /* Global state flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */
extern volatile u32 G_u32BPEngenuicsFlags;             /* From bleperipheral_engenuics.c  */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Anttt_pfnStateMachine;              /* The application state machine function pointer */
static u32 Anttt_u32Timeout;                           /* Timeout counter used across states */
static u32 Anttt_u32Result;

static u8 Anttt_au8SpiReceiveBuffer[U8_SPI0_BUFFER_SIZE];


static u8 Anttt_au8TestResponse[] = {NRF_SYNC_BYTE, NRF_CMD_TEST_RESPONSE_LENGTH, NRF_CMD_TEST_RESPONSE};
static u8 Anttt_au8AckMessage[]   = {NRF_SYNC_BYTE, ANTTT_APP_MESSAGE_ACK_LENGTH, ANTTT_APP_MESSAGE_ACK};
static u8 Anttt_au8NAckMessage[]  = {NRF_SYNC_BYTE, ANTTT_APP_MESSAGE_NACK_LENGTH, ANTTT_APP_MESSAGE_NACK};


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: AntttInitialize

Description:
Initializes the State Machine and its variables.

Requires:

Promises:
*/
void AntttInitialize(void)
{
  u8 au8SpiTestMessage[] = {NRF_SYNC_BYTE, NRF_CMD_TEST_LENGTH, NRF_CMD_TEST};
  u8 u8ReceivedBytes;
  u32 u32Result = 0;
  
  /* Send a test message via SPI to check connection to SAM3U2 */
  LedOn(RED);
  u32Result = SpiMasterSend(au8SpiTestMessage, sizeof(au8SpiTestMessage) );
  
  if(u32Result == NRF_SUCCESS)
  {
    /* Wait for response */
    Anttt_u32Timeout = G_u32SystemTime1ms;
    while( (!NRF_GPIOTE->EVENTS_IN[EVENT_MRDY_ASSERTED]) &&
           (!IsTimeUp(&Anttt_u32Timeout, U32_MRDY_TIMEOUT_MS) ) );
    
    if(NRF_GPIOTE->EVENTS_IN[EVENT_MRDY_ASSERTED])
    {
      /* Clear the MRDY event and receive bytes */
      NRF_GPIOTE->EVENTS_IN[EVENT_MRDY_ASSERTED] = 0;
      u8ReceivedBytes = SpiMasterReceive(Anttt_au8SpiReceiveBuffer);
      
      /* Check if the received message is the expected test response */
      if( (Anttt_au8SpiReceiveBuffer[NRF_SYNC_INDEX] == NRF_SYNC_BYTE) &&
          (Anttt_au8SpiReceiveBuffer[NRF_COMMAND_INDEX] == NRF_CMD_TEST_RESPONSE))
      {
        LedOff(RED);
      }
    }
    else
    {
      LedBlink(RED, LED_4HZ);
    }
  }
  
  /* Activate blinking blue LED to indicate BLE is searching */
  LedBlink(BLUE, LED_4HZ);  
  
  Anttt_pfnStateMachine = AntttSM_Idle;
  
} /* end AntttInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function AntttRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void AntttRunActiveState(void)
{
  Anttt_pfnStateMachine();

} /* end AntttRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------------------------------------*/
/* State Machine definitions                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn static void AntttSM_Idle(void)
@brief Looks for a connection to a game, either from BLE, ANT,
or over SPI. This first command must be a GAME_REQUEST.
*/
static void AntttSM_Idle(void)
{
  static bool bBlinkOn = true;
  u8 u8SpiMsgLength;
  
  /* Check if a BLE module is connected to client */
  if (G_u32BPEngenuicsFlags == _BPENGENUICS_CONNECTED)
  {
    /* Set LEDs and proceed to wait state */
    LedOn(BLUE);
    bBlinkOn = false;
    
    /* Look for a BLE game request */
  }
  else
  {
    if(!bBlinkOn)
    {
      LedBlink(BLUE, LED_4HZ);
      bBlinkOn = true;
    }
  }
  
  /* Read any SPI messages */
  if(NRF_GPIOTE->EVENTS_IN[EVENT_MRDY_ASSERTED])
  {
    /* Clear the MRDY event and receive bytes */
    NRF_GPIOTE->EVENTS_IN[EVENT_MRDY_ASSERTED] = 0;
    u8ReceivedBytes = SpiMasterReceive(Anttt_au8SpiReceiveBuffer);

    /* Verify sync byte */
    if( Anttt_au8SpiReceiveBuffer[NRF_SYNC_INDEX] == NRF_SYNC_BYTE)
    {
      u8SpiMsgLength = Anttt_au8SpiReceiveBuffer[NRF_LENGTH_INDEX];
      
      /* ANTTT will only respond to a Test Message in this state */
      if(Anttt_au8SpiReceiveBuffer[NRF_COMMAND_INDEX] == NRF_CMD_TEST)
      {
        Anttt_u32Result = SpiMasterSend(Anttt_au8TestResponse, 
                                        (NRF_CMD_TEST_RESPONSE_LENGTH + NRF_OVERHEAD_BYTES) );
      }
    }
  } /* end SPI message processing */
 
  /* ANT connectivity */
} /* end */


/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Game
Plays a complete game of Tic-Tac-Toe.  
*/



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
