/**********************************************************************************************************************
File: spi_master.h                                                                

Description:
Header file for spi_master.c source.
**********************************************************************************************************************/

#ifndef __SPI_MASTER_H
#define __SPI_MASTER_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/

/**********************************************************************************************************************
MACROS
**********************************************************************************************************************/
#define ASSERT_SPI0_CS         (NRF_GPIO->OUTCLR = P0_10_ANT_USPI2_CS)
#define DEASSERT_SPI0_CS       (NRF_GPIO->OUTSET = P0_10_ANT_USPI2_CS)


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
#define U8_SPI0_BUFFER_SIZE     (u8)128    /* Maximum buffer size (TX/RX) */
#define U8_SPI_DUMMY            (u8)0xF0   /* Dummy byte sent if rx-only transaction */
#define EVENT_SRDY_ASSERTED     (u8)0      /* GPIOTE event # */
#define EVENT_MRDY_ASSERTED     (u8)1      /* GPIOTE event # */

#define U32_SRDY_TIMEOUT_MS     (u32)100   /* Time for SRDY to assert after CS */

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

#define NRF_APP_MESSAGE_START   (u8)0x40

#define NRF_CMD_RESERVED0       (u8)0xF0
#define NRF_CMD_RESERVED1       (u8)0xF1
#define NRF_CMD_RESERVED2       (u8)0xF2
#define NRF_CMD_RESERVED3       (u8)0xF3
#define NRF_CMD_RESERVED4       (u8)0xF4
#define NRF_CMD_RESERVED5       (u8)0xF5
#define NRF_CMD_RESERVED6       (u8)0xF6
#define NRF_CMD_RESERVED7       (u8)0xF7
#define NRF_CMD_RESERVED8       (u8)0xF8
#define NRF_CMD_RESERVED9       (u8)0xF9
#define NRF_CMD_RESERVEDA       (u8)0xFA
#define NRF_CMD_RESERVEDB       (u8)0xFB
#define NRF_CMD_RESERVEDC       (u8)0xFC
#define NRF_CMD_RESERVEDD       (u8)0xFD
#define NRF_CMD_TEST_RESPONSE   (u8)0xFE
#define NRF_CMD_TEST            (u8)0xFF

#define NRF_CMD_TEST_LENGTH              (u8)1
#define NRF_CMD_TEST_RESPONSE_LENGTH     (u8)1

/* end */

#define GPIOTE_CONFIG_P0_09_ANT_SRDY_CNF = ( (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos) | \
                                             (ANT_SRDY_BIT                  << GPIOTE_CONFIG_PSEL_Pos)     | \
                                             (GPIOTE_CONFIG_MODE_Event      << GPIOTE_CONFIG_MODE_Pos)     )

#define GPIOTE_CONFIG_P0_08_ANT_MRDY_CNF = ( (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos) | \
                                             (ANT_MRDY_BIT                  << GPIOTE_CONFIG_PSEL_Pos)     | \
                                             (GPIOTE_CONFIG_MODE_Event      << GPIOTE_CONFIG_MODE_Pos)     )

/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
u32 SpiMasterSend(u8* pu8TxBuffer_, u8 u8Length_);
u8 SpiMasterReceive(u8* pu8RxBuffer);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void SpiMasterInitialize(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/



#endif /* __SPI_MASTER_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
