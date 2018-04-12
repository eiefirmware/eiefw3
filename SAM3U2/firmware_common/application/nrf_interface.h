/*!*********************************************************************************************************************
@file nrf_interface.h                                                                
@brief Header file for nrf_interface

**********************************************************************************************************************/

#ifndef __NRF_INTERFACE_H
#define __NRF_INTERFACE_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/*******************************************************************************
* Macros 
*******************************************************************************/
#define IS_CS_ASSERTED()      (NRF_SSP_FLAGS & _SSP_CS_ASSERTED)   /*!< @brief Macro returns TRUE if SEN is asserted */        
#define ACK_CS_ASSERTED()     (NRF_SSP_FLAGS &= ~_SSP_CS_ASSERTED) /*!< @brief Macro to clear the _SSP_CS_ASSERTED flag */         

#define IS_MRDY_ASSERTED()    (NRF_MRDY_READ_REG == 0) /*!< @brief Macro returns TRUE if MRDY is asserted */
#define NRF_MRDY_ASSERT()     (NRF_MRDY_CLEAR_REG)     /*!< @brief Macro to assert MRDY */   
#define NRF_MRDY_DEASSERT()   (NRF_MRDY_SET_REG)       /*!< @brief Macro to deassert MRDY */

#define NRF_SRDY_ASSERT()     (NRF_SRDY_CLEAR_REG)     /*!< @brief Macro to assert SRDY */       
#define NRF_SRDY_DEASSERT()   (NRF_SRDY_SET_REG)       /*!< @brief Macro to deassert SRDY */


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
u8 nrfNewMessageCheck(void); 
void nrfGetAppMessage(u8* pu8AppBuffer_);
u32 nrfQueueMessage(u8* pu8AppMessage_);

/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void nrfInterfaceInitialize(void);
void nrfInterfaceRunActiveState(void);

void nrfTxFlowControlCallback(void);
void nrfRxFlowControlCallback(void);

/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void nrfInterfaceSM_Idle(void);    

static void nrfInterfaceSM_Rx(void);       
static void nrfInterfaceSM_Tx(void); 

static void nrfInterfaceSM_Error(void);         


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* nrfInterface_u32Flags */
#define _RX_IN_PROGRESS         0x00000001
#define _TX_IN_PROGRESS         0x00000002

#define _NEW_APP_MESSAGE        0x00000100

#define _RX_OVERFLOW            0x00010000
#define _RX_TIMEOUT             0x00020000
#define _TX_TIMEOUT             0x00040000
/* end nrfInterface_u32Flags */

#define U8_NRF_BUFFER_SIZE      (u8)128 

#define U32_TRX_TIMEOUT_MS      (u32)1000

/* NRF Interface Protocol */
#define NRF_SYNC_BYTE           (u8)0x5A
#define NRF_OVERHEAD_BYTES      (u8)2

#define NRF_SYNC_INDEX          (u8)0
#define NRF_LENGTH_INDEX        (u8)1
#define NRF_COMMAND_INDEX       (u8)2
#define NRF_CMD_ARG1_INDEX      (u8)3
#define NRF_CMD_ARG2_INDEX      (u8)4
#define NRF_DATA_INDEX          (u8)3

#define NRF_CMD_EMPTY           (u8)0x00
#define NRF_CMD_LED             (u8)0x01
#define NRF_CMD_LCD1            (u8)0x02
#define NRF_CMD_LCD2            (u8)0x03
#define NRF_CMD_DEBUG           (u8)0x04
#define NRF_CMD_BUZZ            (u8)0x05

#define NRF_CMD_BUTTON          (u8)0x21

#define NRF_CMD_RESERVED0       (u8)0x70
#define NRF_CMD_RESERVED1       (u8)0x71
#define NRF_CMD_RESERVED2       (u8)0x72
#define NRF_CMD_RESERVED3       (u8)0x73
#define NRF_CMD_RESERVED4       (u8)0x74
#define NRF_CMD_RESERVED5       (u8)0x75
#define NRF_CMD_RESERVED6       (u8)0x76
#define NRF_CMD_RESERVED7       (u8)0x77
#define NRF_CMD_RESERVED8       (u8)0x78
#define NRF_CMD_RESERVED9       (u8)0x79
#define NRF_CMD_RESERVEDA       (u8)0x7A
#define NRF_CMD_RESERVEDB       (u8)0x7B
#define NRF_CMD_RESERVEDC       (u8)0x7C
#define NRF_CMD_RESERVEDD       (u8)0x7D
#define NRF_CMD_TEST_RESPONSE   (u8)0x7E
#define NRF_CMD_TEST            (u8)0x7F

#define NRF_APP_MESSAGE_START   (u8)0x80

#define NRF_CMD_TEST_LENGTH              (u8)1
#define NRF_CMD_TEST_RESPONSE_LENGTH     (u8)1

/* end */


#endif /* __NRF_INTERFACE_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
