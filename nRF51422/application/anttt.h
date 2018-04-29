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
of whether the game is being played via ANT or BLE.  

For messages exchanged over the air, the protocol message structure 
is designed to fit into the fixed 8-byte ANT message length. No length
information is required.  A two-byte synchronization counter is used 
that will increment for all unique messages traded.

COMMAND_ID CMD_DATA0...CMD_DATA4 MSG_COUNT_HI MSG_COUNT_LO

The same command IDs are used for the nRF51422 to SAM3U2 SPI messaging.
These follow the nRF Interface protocol without the padded unused
bytes or message counter. This protocol requires a message length.

*/

#define ANTTT_OTA_COMMAND_SIZE                  (u8)8
#define ANTTT_OTA_COMMAND_OFFSET_ID             (u8)0
#define ANTTT_OTA_COMMAND_OFFSET_DATA           (u8)1
#define ANTTT_OTA_COMMAND_OFFSET_MSG_COUNT      (u8)6
#define ANTTT_OTA_COMMAND_OFFSET_DATA0          (u8)1
#define ANTTT_OTA_COMMAND_OFFSET_DATA1          (u8)2
#define ANTTT_OTA_COMMAND_OFFSET_DATA2          (u8)3
#define ANTTT_OTA_COMMAND_OFFSET_DATA3          (u8)4
#define ANTTT_OTA_COMMAND_OFFSET_DATA4          (u8)5
#define ANTTT_OTA_COMMAND_OFFSET_MSG_COUNT_HI   (u8)6
#define ANTTT_OTA_COMMAND_OFFSET_MSG_COUNT_LO   (u8)7

/* ANTTT is allocated 16 messages - not all are used */
#define ANTTT_STARTING_MESSAGE_NUMBER           (u8)0xA0
#define ANTTT_ENDING_MESSAGE_NUMBER             (u8)0xAF

/* 
Message: SYNC_CHECK
Target: OTA device <--> EIE nRF51422
Format: ID 0 0 0 0 0 [_u32MessageNumber & 0x0000FFFF]
Response: SYNC_CHECK
Description: used at any time to send to another ANTTT 
device to check communication and confirm the current 
message synchronization number.
*/
#define ANTTT_APP_MSG_SYNC_CHECK                (u8)0xA0


/* A game request message */
#define ANTTT_APP_MSG_GAME_REQUEST              (u8)0xA2
#define ANTTT_APP_MSG_GAME_REQUEST_LENGTH       (u8)2
#define GAME_REQUEST_OFFSET_STARTER             (u8)1
#define GAME_REQUEST_DATA_LOCAL_STARTS          (u8)0x01
#define GAME_REQUEST_DATA_REMOTE_STARTS         (u8)0x02

#define MSG_GAME_REQUEST_D0

/* Terminal board reset message */
#define ANTTT_APP_MSG_RESET                     (u8)0xA1
#define ANTTT_APP_MSG_RESET_LENGTH              (u8)1


#define ANTTT_APP_MSG_GAME_RELEASE              (u8)0xA3

/* A game move message */
#define ANTTT_APP_MSG_MOVE                      (u8)0xAA
#define ANTTT_APP_MSG_MOVE_LENGTH               (u8)2
#define ANTTT_COMMAND_POSITION_OFFSET           (u8)0x01

/* ACK and NACK messages */
#define ANTTT_APP_MSG_ACK                       (u8)0xAF   
#define ANTTT_APP_MSG_ACK_LENGTH                (u8)1   

#define ANTTT_APP_MSG_NACK                      (u8)0xAE   
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
