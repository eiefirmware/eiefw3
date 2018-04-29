/*!**********************************************************************************************************************
@file anttt.c                                                                
@brief Implements TIC-TAC-TOE using data input from ANT or BLE.

**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAnttt"
***********************************************************************************************************************/
/* New variables */
u32 G_u32AntttFlags;                                   /*!< @brief Global state flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1ms;                /*!< @brief From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /*!< @brief From board-specific source file */
extern volatile u32 G_u32BPEngenuicsFlags;             /*!< @brief From bleperipheral_engenuics.c  */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Anttt_pfnStateMachine;              /*!< @brief The application state machine function pointer */
static fnCode_type Anttt_pfnNextState;                 /*!< @brief Pointer loaded when a command is ACKED */
static fnCode_type Anttt_pfnPrevState;                 /*!< @brief Pointer loaded when a command is NACKED */

static u32 Anttt_u32Timeout;                           /*!< @brief Timeout counter used across states */

static u32 Anttt_u32Result;

static AntttConnectionType Anttt_eConnection;
static bool Anttt_bBleBlinkOn;
static bool Anttt_bAntBlinkOn;

static u8 Anttt_au8SpiReceiveBuffer[U8_SPI0_BUFFER_SIZE];
static u32 Anttt_u32MessageNumber;

static u8 Anttt_au8Message[ANTTT_COMMAND_SIZE];

/* nRF Interface Messages */
static u8 Anttt_au8TestResponse[] = {NRF_SYNC_BYTE, NRF_CMD_TEST_RESPONSE_LENGTH, NRF_CMD_TEST_RESPONSE};
static u8 Anttt_au8AckMessage[]   = {NRF_SYNC_BYTE, ANTTT_APP_MSG_ACK_LENGTH, ANTTT_APP_MSG_ACK};
static u8 Anttt_au8NackMessage[]  = {NRF_SYNC_BYTE, ANTTT_APP_MSG_NACK_LENGTH, ANTTT_APP_MSG_NACK};

static u8 Anttt_au8BoardReset[]   = {NRF_SYNC_BYTE, ANTTT_APP_MSG_RESET_LENGTH, ANTTT_APP_MSG_RESET};
static u8 Anttt_au8GameMove[]     = {NRF_SYNC_BYTE, ANTTT_APP_MSG_MOVE_LENGTH, ANTTT_APP_MSG_MOVE, 0};


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
@fn void AntttInitialize(void)
@brief Initializes the State Machine and its variables.

Requires:
- NONE

Promises:
-NONE

*/
void AntttInitialize(void)
{
  u8 au8SpiTestMessage[] = {NRF_SYNC_BYTE, NRF_CMD_TEST_LENGTH, NRF_CMD_TEST};
  u8 u8ReceivedBytes;
  u32 u32Result = 0;

  /* Initialize globals */
  Anttt_eConnection = ANTTT_CONN_NONE;
  Anttt_bBleBlinkOn = true;
  Anttt_bAntBlinkOn = true;
  AntttResetCommandMessage();
  
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

  /* Make sure the game board is reset */
  u32Result = SpiMasterSend(au8SpiTestMessage, sizeof(au8SpiTestMessage) );
  
  if(u32Result != NRF_SUCCESS)
  {
    
  }

  
  /* Activate blinking blue LED to indicate BLE is searching */
  LedBlink(BLUE, LED_4HZ);  
  
  Anttt_pfnStateMachine = AntttSM_Idle;
  
} /* end AntttInitialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void AntttRunActiveState(void)
@brief Selects and runs one iteration of the current state in the state machine.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void AntttRunActiveState(void)
{
  Anttt_pfnStateMachine();

} /* end AntttRunActiveState */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn void AntttResetCommandMessage(void)
@brief Sets all bytes in Anttt_au8Message to 0.

Requires:
- NONE

Promises:
- Anttt_au8Message[--] = 0;

*/
void AntttResetCommandMessage(void)
{
  for(u8 i = 0; i < ANTTT_COMMAND_SIZE; i++)
  {
    Anttt_au8Message[i] = 0;
  }
  
} /* end AntttResetCommandMessage() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void AntttAddCurrentMessageCounter(u8* pu8Message_)
@brief Adds the current value of Anttt_u32MessageNumber into the message specified

Requires:
@PARAM pu8Message_ points to a stanard 8-byte OTA message where the current Anttt_u32MessageNumber
is to be added

Promises:
- The lower 2 bytes of Anttt_u32MessageNumber are placed 
at *(pu8Message_ + ANTTT_OTA_COMMAND_OFFSET_MSG_COUNT)
- Anttt_u32MessageNumber incremented

*/
void AntttAddCurrentMessageCounter(u8* pu8Message_)
{
  *(pu8Message_ + ANTTT_COMMAND_OFFSET_MSG_COUNT_HI) = (u8)( (Anttt_u32MessageNumber & 0x0000FF00) >> 8);
  *(pu8Message_ + ANTTT_COMMAND_OFFSET_MSG_COUNT_LO) = (u8)( (Anttt_u32MessageNumber & 0x000000FF) >> 0);
  Anttt_u32MessageNumber++;
  
} /* end AntttAddCurrentMessageCounter() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void AntttCheckSpiMessages(void)
@brief Looks for any incoming SPI messages, reads and processes them.

Requires:
- NONE

Promises:
- An incoming SPI message is read and processed
- Anttt_au8SpiReceiveBuffer will hold the latest message data

*/
void AntttCheckSpiMessages(void)
{
  u8 u8ReceivedBytes;
  u8 u8SpiMsgLength;

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
      
      /* Process the message */
      switch(Anttt_au8SpiReceiveBuffer[NRF_COMMAND_INDEX])
      {
        case NRF_CMD_TEST:
        {
          Anttt_u32Result = SpiMasterSend(Anttt_au8TestResponse, 
                                         (NRF_CMD_TEST_RESPONSE_LENGTH + NRF_OVERHEAD_BYTES) );
          break;
        }

        default:
        {
          break;
        }
        
      }
    }
  } /* end SPI message processing */

} /* end AntttCheckSpiMessages */


/*--------------------------------------------------------------------------------------------------------------------*/
/* State Machine definitions                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn static void AntttSM_Idle(void)
@brief Looks for a connection to a game, either from BLE, ANT. 

The first device to connect gets to play.
For a BLE connection, this application is responsible for sending 
the 
Anttt_u32MessageNumber is always reset back to 1 for a new connection.

This state will not return until the other device disconnects and stops trying to
make a connection.  
*/
static void AntttSM_Idle(void)
{
  /* Check if a BLE module is connected to client */
  if (G_u32BPEngenuicsFlags == _BPENGENUICS_CONNECTED)
  {
    /* Prepare and queue the game request message including a randomized starter */
    Anttt_u32MessageNumber = 1;
    AntttResetCommandMessage();
    
    Anttt_au8Message[ANTTT_COMMAND_OFFSET_ID] = ANTTT_APP_MSG_GAME_REQUEST;
    
    /* Random start based on system tick LSB */
    if(G_u32SystemTime1ms & 0x00000001)
    {
      Anttt_au8Message[GAME_REQUEST_OFFSET_STARTER] = GAME_REQUEST_DATA_LOCAL_STARTS;
    }
    else
    {
      Anttt_au8Message[GAME_REQUEST_OFFSET_STARTER] = GAME_REQUEST_DATA_REMOTE_STARTS;
    }
    
    /* Add and increment the current message counter then pass to BLE send */
    AntttAddCurrentMessageCounter(Anttt_au8Message);
    BPEngenuicsSendData(Anttt_au8Message, ANTTT_COMMAND_SIZE);
    
    /* Set LEDs and proceed to wait state */
    LedOff(GREEN);
    LedOn(BLUE);
    Anttt_eConnection = ANTTT_CONN_BLE;
    Anttt_bBleBlinkOn = false;
    Anttt_u32MessageNumber = 0;
    Anttt_pfnNextState = AntttSM_BleGameRequest;
    Anttt_pfnPrevState = Anttt_pfnPrevState;
    Anttt_pfnStateMachine = AntttSM_BleGameRequest;
  }
  else
  {
    if(!Anttt_bBleBlinkOn)
    {
      LedBlink(BLUE, LED_4HZ);
      Anttt_bBleBlinkOn = true;
    }
  }
 
  /* ANT connectivity */
  /* Check if an ANT module is connected to client */
  if (G_u32AntIntFlags == _ANTENGENUICS_CONNECTED)
  {
    /* Set LEDs and proceed to wait state */
    LedOff(BLUE);
    LedOn(GREEN);
    Anttt_bAntBlinkOn = false;
    Anttt_eConnection = ANTTT_CONN_ANT;
    Anttt_u32MessageNumber = 0;
    Anttt_pfnStateMachine = AntttSM_AntGameRequest;
  }
  else
  {
    if(!Anttt_bAntBlinkOn)
    {
      LedBlink(GREEN, LED_4HZ);
      Anttt_bAntBlinkOn = true;
    }
  }
 
} /* end */


/*!--------------------------------------------------------------------------------------------------------------------
@fn static void AntttSM_BleGameRequest(void)
@brief Sends the game request message to the other device. 

*/
static void AntttSM_BleGameRequest(void)
{
  /* Check connection */
  if (G_u32BPEngenuicsFlags & BPENGENUICS_CONNECTED)
  {
    
  }
  else
  {
    /* Disconnected from client */
    AntttInitialize();
  }

} /* end AntttSM_BleGameRequest() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn static void AntttSM_AntGameRequest(void)
@brief Sends the game request message to the other device. 

*/
static void AntttSM_BleGameRequest(void)
{
  
} /* end AntttSM_AntGameRequest() */


/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Game
Plays a complete game of Tic-Tac-Toe.  
*/



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
