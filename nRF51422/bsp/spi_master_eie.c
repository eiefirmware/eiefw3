/*!*********************************************************************************************************************
@file spi_master_eie.c                                                                
@brief EiE SPI master implementation using SPI master peripheral 
**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SpiMasterFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SpiMaster_" and be declared as static.
***********************************************************************************************************************/
static u32 SpiMaster_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn void SpiMasterSend(void)
@brief Completes a standard SPI data transmission.

By definition, SPI always transmits and receives simultaneously.  However,
for most applications data is half-duplex and will be implemented as
such for this driver.  Data that is received during this transmission
will be discarded.

Requires:
@PARAM pu8TxBuffer_ points to the data to be transmitted.
@PARAM u8Length_ is the number of bytes that will be sent.

Promises:
- Hardware and SPI peripheral are ready for use

*/
u32 SpiMasterSend(u8* pu8TxBuffer_, u8 u8Length_)
{
  u32 u32Result = NRF_SUCCESS;
  u8  u8ByteCount = 0;

  /* Assert CS and wait for SRDY to drop (should already be low) */
  SpiMaster_u32Timeout = G_u32SystemTime1ms;
  ASSERT_SPI0_CS;
  while( (!NRF_GPIOTE->IN[EVENT_SRDY_ASSERTED]) ||
          (IsTimeUp(&SpiMaster_u32Timeout, U32_SRDY_TIMEOUT) );
          
  /* Clear event and check that MRDY is not also set. Exit if MRDY
  is asserted.  Leave CS asserted and do not clear the event.*/
  NRF_GPIOTE->IN[EVENT_SRDY_ASSERTED] = 0;
  if(NRF_GPIOTE->IN[EVENT_MRDY_ASSERTED])
  {
    return NRF_ERROR_BUSY;
  }

  
  /* Check for timeout */
  if( IsTimeUp(&SpiMaster_u32Timeout, U32_SRDY_TIMEOUT)
  {
    return NRF_ERROR_TIMEOUT;
  }
  spi_master_send_initial_bytes(p_spi_instance);
  
}


/*!----------------------------------------------------------------------------------------------------------------------
@fn void SpiMasterSendReceive(void)
@brief Completes a standard SPI data transmission.

By definition, SPI always transmits and receives simultaneously.  Therefore
the client should provide both a transmit and receive buffer.  If communication
is known to be one-way, set the 

Requires:
@PARAM pu8TxBuffer_ points to the data to be transmitted; if this is a 
receive-only transaction, this may be set to NULL.
@PARAM pu8RxBuffer_ points to the buffer where data is to be received; 
if this is a transmit-only transaction, this may be set to NULL.

Promises:
- Hardware and SPI peripheral are ready for use

*/
u32 SpiMasterReceive(u8* pu8RxBuffer_, u8 u8Length_)
{
  u32 u32Result = NRF_SUCCESS;
  u8  u8ByteCount = 0;

  /* Assert CS and wait for SRDY to drop */
  ASSERT_SPI0_CS;
  while( (!NRF_GPIOTE->IN[EVENT_SRDY_ASSERTED]) ||
          IsTimeUp(
        nrf_gpio_pin_clear(p_spi_instance->pin_slave_select);
        spi_master_send_initial_bytes(p_spi_instance);
  
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn void SpiMasterInitialize(void)
@brief Initializes the State Machine and its variables.

Requires:
- NONE

Promises:
- Hardware and SPI peripheral are ready for use

*/
void SpiMasterInitialize(void)
{
  u32 u32Result;
  
  /* Set up SPI-specific GPIO */
  NRF_GPIO->PIN_CNF[ANT_SPI_MOSI_BIT] = P0_13_ANT_USPI2_MOSI_CNF;
  NRF_GPIO->PIN_CNF[ANT_SPI_MISO_BIT] = P0_12_ANT_USPI2_MISO_CNF;
  NRF_GPIO->PIN_CNF[ANT_SPI_SCK_BIT]  = P0_11_ANT_USPI2_SCK_CNF;
  NRF_GPIO->PIN_CNF[ANT_SEN_BIT]      = P0_10_ANT_USPI2_CS_CNF;
  NRF_GPIO->PIN_CNF[ANT_SRDY_BIT]     = P0_09_ANT_SRDY_CNF;
  NRF_GPIO->PIN_CNF[ANT_MRDY_BIT]     = P0_08_ANT_MRDY_CNF;
  
  NRF_GPIO->OUTSET = P0_11_ANT_USPI2_SCK;
  NRF_GPIO->OUTCLR = P0_13_ANT_USPI2_MOSI;

  /* Configure SPI hardware */
  NRF_SPI0->PSELMOSI = P0_13_ANT_SPI_MOSI_INDEX;
  NRF_SPI0->PSELMISO = P0_12_ANT_SPI_MISO_INDEX;  
  NRF_SPI0->PSELSCK  = P0_11_ANT_SPI_SCK_INDEX;
  
  /* Set up events on the SRDY and MRDY flow control input pins */
  NRF_GPIOTE->CONFIG[EVENT_SRDY_ASSERTED] = GPIOTE_CONFIG_P0_09_ANT_SRDY_CNF;
  NRF_GPIOTE->CONFIG[EVENT_MRDY_ASSERTED] = GPIOTE_CONFIG_P0_08_ANT_MRDY_CNF;
  NRF_GPIOTE->IN[EVENT_SRDY_ASSERTED] = 0;
  NRF_GPIOTE->IN[EVENT_MRDY_ASSERTED] = 0;
  
  /* Clear waiting interrupts and events */
  NRF_SPI0->EVENTS_READY = 0;

  /* Set up SPI Master peripheral */
  NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M1 << SPI_FREQUENCY_FREQUENCY_Pos;
  NRF_SPI0->CONFIG    = (SPI_CONFIG_ORDER_LsbFirst << SPI_CONFIG_ORDER_Pos | \
                         SPI_CONFIG_CPHA_Leading   << SPI_CONFIG_CPHA_Pos  | \
                         SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos );
  
  /* Configure Interrupts */
#ifdef SOFTDEVICE_ENABLED  
  /* Enable the interrupt via the SD */
  u32Result |= sd_nvic_SetPriority(SPI0_TWI0_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_ClearPendingIRQ(SPI0_TWI0_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_EnableIRQ(SPI0_TWI0_IRQn);
#else

#ifdef INTERRUPTS_ENABLED
  /* Enable the RTC interrupt */
  NVIC_SetPriority(SPI0_TWI0_IRQn, NRF_APP_PRIORITY_LOW);
  NVIC_EnableIRQ(SPI0_TWI0_IRQn);
#endif /* INTERRUPTS_ENABLED */
  
#endif /* SOFTDEVICE_ENABLED */

    
  /* Test operation */

} /* end SpiMasterInitialize() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
