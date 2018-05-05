/*!*********************************************************************************************************************
@file anttt.h    
@brief Header file for anttt
**********************************************************************************************************************/

#ifndef __ANTTTT_H
#define __ANTTTT_H


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* G_u32AntttFlags */
#define _ANTTT_FLAGS_UNHANDLED_SPI_MSG     0x00010000
/* end G_u32AntttFlags */


/* ANTTT Communication Protocol
This protocol will be used to exchange game messages regardless
of whether the game is being played via ANT or BLE. The same 
command IDs and message structures are used for the nRF51422 to 
SAM3U2 SPI messaging.

The protocol message structure is designed to fit into the fixed 
8-byte ANT message length, so no length information is required in
within the 8-bytes.  This payload will be inserted into 
whichever protocol frame is used for transfer (BLE, ANT, SPI). The
intent is that these messages are processed all the same at the
ANTTT application level.

Over-the-air (OTA) messages include a two-byte synchronization counter 
that will increment for all unique messages traded.  This is not
required for SPI messages.  

The 8 byte messages look like this:

COMMAND_ID CMD_DATA0...CMD_DATA4 MSG_COUNT_HI MSG_COUNT_LO
*/

#define ANTTT_COMMAND_SIZE                      (u8)8
#define ANTTT_COMMAND_OFFSET_ID                 (u8)0
#define ANTTT_COMMAND_OFFSET_DATA               (u8)1
#define ANTTT_COMMAND_OFFSET_MSG_COUNT          (u8)6
#define ANTTT_COMMAND_OFFSET_DATA0              (u8)1
#define ANTTT_COMMAND_OFFSET_DATA1              (u8)2
#define ANTTT_COMMAND_OFFSET_DATA2              (u8)3
#define ANTTT_COMMAND_OFFSET_DATA3              (u8)4
#define ANTTT_COMMAND_OFFSET_DATA4              (u8)5
#define ANTTT_COMMAND_OFFSET_MSG_COUNT_HI       (u8)6
#define ANTTT_COMMAND_OFFSET_MSG_COUNT_LO       (u8)7



/* ANTTT is allocated 16 messages - not all are used */
#define ANTTT_STARTING_MESSAGE_NUMBER           (u8)0xA0
#define ANTTT_ENDING_MESSAGE_NUMBER             (u8)0xAF
//#define ANTTT_MAXIMUM_MESSAGE_LENGTH            (u8)10

/* 
 Message: SYNC_CHECK
  Target: OTA device <-> EIE nRF51422
  Format: ID 0 0 0 0 0 [_u32MessageNumber & 0x0000FFFF]
Response: SYNC_CHECK

Description: used at any time to send to another ANTTT 
device to check communication and confirm the current 
message synchronization number.
*/
#define ANTTT_APP_MSG_SYNC_CHECK                (u8)0xA0
#define ANTTT_APP_MSG_SYNC_CHECK_LENGTH         (u8)1

/* 
 Message: RESET
  Target: OTA device   <-> EIE nRF51422
          EIE nRF51422 --> EIE SAM3U2
  Format: ID 0 0 0 0 0 [_u32MessageNumber & 0x0000FFFF]
Response: NONE

Description: Requests the receiver resets back to the starting
point of a new game (waiting for GAME_REQUEST).
*/
#define ANTTT_APP_MSG_RESET                     (u8)0xA1
#define ANTTT_APP_MSG_RESET_LENGTH              (u8)1

/*
 Message: GAME_REQUEST
  Target: OTA device   <-> EIE nRF51422
          EIE nRF51422 --> EIE SAM3U2
  Format: ID STARTER 0 0 0 0 [_u32MessageNumber & 0x0000FFFF]
Response: NONE

Description: Confirms the start of game and who goes first.
*/
#define ANTTT_APP_MSG_GAME_REQUEST              (u8)0xA2
#define ANTTT_APP_MSG_GAME_REQUEST_LENGTH       (u8)2

#define GAME_REQUEST_OFFSET_STARTER             (u8)1

#define GAME_REQUEST_DATA_LOCAL_STARTS          (u8)0x01
#define GAME_REQUEST_DATA_REMOTE_STARTS         (u8)0x02


/*
 Message: GAME_MOVE
  Target: OTA device   <-> EIE nRF51422
          EIE nRF51422 <-> EIE SAM3U2
  Format: ID POSITION STATUS 0 0 0 [_u32MessageNumber & 0x0000FFFF]
Response: NONE

Description: Submits a game move to the receiver and confirms
the game status (still playing, home has won, away has won, or
game ends in a draw).  
Valid positions are 0 (top left) to 8 (bottom right).
*/
#define ANTTT_APP_MSG_GAME_MOVE                 (u8)0xAA
#define ANTTT_APP_MSG_GAME_MOVE_LENGTH          (u8)3

#define GAME_MOVE_OFFSET_POSITION               (u8)1
#define GAME_MOVE_OFFSET_STATUS                 (u8)2

#define GAME_MOVE_DATA_STATUS_PLAYING           (u8)1
#define GAME_MOVE_DATA_STATUS_WINNER_HOME       (u8)2
#define GAME_MOVE_DATA_STATUS_WINNER_AWAY       (u8)3
#define GAME_MOVE_DATA_STATUS_DRAW              (u8)4

/*
 Message: ACK
  Target: OTA device   <-> EIE nRF51422
  Format: ID 0 0 0 0 0 [_u32MessageNumber & 0x0000FFFF]
Response: NONE

Description: General ACK message to an OTA command
*/
#define ANTTT_APP_MSG_ACK                       (u8)0xAE   
#define ANTTT_APP_MSG_ACK_LENGTH                (u8)1   

/*
 Message: NACK
  Target: OTA device   <-> EIE nRF51422
  Format: ID 0 0 0 0 0 [_u32MessageNumber & 0x0000FFFF]
Response: NONE

Description: General NACK message to an OTA command
*/
#define ANTTT_APP_MSG_NACK                      (u8)0xAF   
#define ANTTT_APP_MSG_NACK_LENGTH               (u8)1   



/* end ANTTT Communication Protocol */

#define U32_ANTTT_SPI_CHECK_PERIOD_MS   (u32)1000


/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/

typedef enum {ANTTT_CONN_ANT, ANTTT_CONN_BLE, ANTTT_CONN_NONE} AntttConnectionType;


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void AntttInitialize(void);
void AntttRunActiveState(void);


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void AntttResetCommandMessage(void);
void AntttAddCurrentMessageCounter(u8* pu8Message_);
void AntttCheckSpiMessages(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void AntttSM_Idle(void);

static void AntttSM_BleGameRequest(void);
static void AntttSM_BleGameRequest(void);



#endif /* __ANTTT_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
