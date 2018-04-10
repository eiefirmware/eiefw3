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
static void nrfInterfaceSM_Error(void);         



/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* nrfInterface_u32Flags */
#define _RX_IN_PROGRESS       0x00000001
#define _TX_IN_PROGRESS       0x00000002

#define _RX_OVERFLOW          0x00010000
/* end nrfInterface_u32Flags */

#define U8_NRF_BUFFER_SIZE    (u8)128 

/* NRF Interface Protocol */
#define NRF_SYNC_BYTE           (u8)0x5A
#define NRF_SYNC_INDEX          (u8)0
#define NRF_LENGTH_INDEX        (u8)1
#define NRF_COMMAND_INDEX       (u8)2
#define NRF_DATA_INDEX          (u8)3

#define NRF_CMD_LED             (u8)0x01
#define NRF_CMD_LCD1            (u8)0x02
#define NRF_CMD_LCD2            (u8)0x03
#define NRF_CMD_DEBUG           (u8)0x04
#define NRF_CMD_BUZZ            (u8)0x05

#define NRF_CMD_BUTTON          (u8)0x21
/* end */


#endif /* __NRF_INTERFACE_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
