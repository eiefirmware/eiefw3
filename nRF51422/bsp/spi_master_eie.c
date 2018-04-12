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
- All data is clocked out on SPI
- Returns NRF_ status

*/
u32 SpiMasterSend(u8* pu8TxBuffer_, u8 u8Length_)
{
  u8  u8Dummy;

  /* Assert CS and wait for SRDY to drop */
  SpiMaster_u32Timeout = G_u32SystemTime1ms;
  ASSERT_SPI0_CS;
  while( (!NRF_GPIOTE->EVENTS_IN[EVENT_SRDY_ASSERTED]) &&
         (!IsTimeUp(&SpiMaster_u32Timeout, U32_SRDY_TIMEOUT_MS) ) );
          
  /* Clear SRDY event and check that MRDY is not also set. */
  NRF_GPIOTE->EVENTS_IN[EVENT_SRDY_ASSERTED] = 0;
  if(NRF_GPIOTE->EVENTS_IN[EVENT_MRDY_ASSERTED])
  {
    /* Exit if MRDY is asserted.  Leave CS asserted and do not clear the event. */
    LedOn(RED);
    return NRF_ERROR_BUSY;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&SpiMaster_u32Timeout, U32_SRDY_TIMEOUT_MS) )
  {
    LedOn(RED);
    return NRF_ERROR_TIMEOUT;
  }
   
  /* All is good, so send the bytes */
  for(u8 i = 0; i < u8Length_; i++)
  {
    /* Clear the event and queue the byte */
    NRF_SPI0->EVENTS_READY = 0;
    NRF_SPI0->TXD = *(pu8TxBuffer_ + i);
    
    /* Wait for the transmit event before moving on */
    while(!NRF_SPI0->EVENTS_READY); 
    
    /* Read the received dummy byte to clear RXD */
    u8Dummy = NRF_SPI0->RXD;
  }
  
  /* Transmission is complete, so clear CS after a short delay */
  for(u32 i = 0; i < U32_CS_DEASSERT_DELAY; i++);
  DEASSERT_SPI0_CS;

  return NRF_SUCCESS;
  
} /* end SpiMasterSend() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn u8 SpiMasterReceive(u8* pu8RxBuffer)
@brief Completes a standard SPI data transmission.

By definition, SPI always transmits and receives simultaneously.  For this
driver, data is half duplex.  If the SAM3U2 has data to send, MRDY will
be

Requires:
- The incoming message MUST follow the application protocol so the
second byte is the length of the message.  
@PARAM pu8RxBuffer_ points to the buffer where data is to be received; 

Promises:
- Returns the number of bytes received

*/
u8 SpiMasterReceive(u8* pu8RxBuffer_)
{
  u8 u8ByteCount = 0;
  u8 u8Length = NRF_OVERHEAD_BYTES;
  
  /* Assert CS and wait for SRDY to drop */
  SpiMaster_u32Timeout = G_u32SystemTime1ms;
  ASSERT_SPI0_CS;
  while( (!NRF_GPIOTE->EVENTS_IN[EVENT_SRDY_ASSERTED]) &&
         (!IsTimeUp(&SpiMaster_u32Timeout, U32_SRDY_TIMEOUT_MS) ) );
          
  /* Clear SRDY event and check for timeout */
  NRF_GPIOTE->EVENTS_IN[EVENT_SRDY_ASSERTED] = 0;
  if( IsTimeUp(&SpiMaster_u32Timeout, U32_SRDY_TIMEOUT_MS) )
  {
    return NRF_ERROR_TIMEOUT;
  }
   
  /* All is good, so receive the bytes */
  for(u8 i = 0; i < u8Length; i++)
  {
    /* Clear the event and queue the dummy byte */
    NRF_SPI0->EVENTS_READY = 0;
    NRF_SPI0->TXD = U8_SPI_DUMMY;
    
    /* Wait for the transmit event before moving on */
    while(!NRF_SPI0->EVENTS_READY); 
    
    /* Read the received byte  */
    *(pu8RxBuffer_ + i) = NRF_SPI0->RXD;
    
    /* Update length when we receive the value */
    if(i == NRF_LENGTH_INDEX)
    {
      u8Length += *(pu8RxBuffer_ + i);
    }
    
    u8ByteCount++;
  }
    
  /* Reception is complete, so clear CS */
  for(u32 i = 0; i < U32_CS_DEASSERT_DELAY; i++);
  DEASSERT_SPI0_CS;

  return u8ByteCount;
  
} /* end SpiMasterReceive() */


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
  /* Set pin outputs per user guide and for desired start states when
  the pins are changed to output */
  DEASSERT_SPI0_CS;
  NRF_GPIO->OUTSET = P0_11_ANT_USPI2_SCK;
  NRF_GPIO->OUTCLR = P0_13_ANT_USPI2_MOSI;

  /* Now set up the pin configurations */
  NRF_GPIO->PIN_CNF[ANT_USPI2_MOSI_BIT_NUMBER] = P0_13_ANT_USPI2_MOSI_CNF;
  NRF_GPIO->PIN_CNF[ANT_USPI2_MISO_BIT_NUMBER] = P0_12_ANT_USPI2_MISO_CNF;
  NRF_GPIO->PIN_CNF[ANT_USPI2_SCK_BIT_NUMBER]  = P0_11_ANT_USPI2_SCK_CNF;
  NRF_GPIO->PIN_CNF[ANT_SEN_BIT_NUMBER]        = P0_10_ANT_USPI2_CS_CNF;
  NRF_GPIO->PIN_CNF[ANT_SRDY_BIT_NUMBER]       = P0_09_ANT_SRDY_CNF;
  NRF_GPIO->PIN_CNF[ANT_MRDY_BIT_NUMBER]       = P0_08_ANT_MRDY_CNF;
  
  /* Configure SPI periperal */
  NRF_SPI0->PSELMOSI  = ANT_USPI2_MOSI_BIT_NUMBER;
  NRF_SPI0->PSELMISO  = ANT_USPI2_MISO_BIT_NUMBER;  
  NRF_SPI0->PSELSCK   = ANT_USPI2_SCK_BIT_NUMBER;
  NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M1 << SPI_FREQUENCY_FREQUENCY_Pos;
  NRF_SPI0->CONFIG    = SPI0_CONFIG_CNF;
  
  /* Set up events on the SRDY and MRDY flow control input pins */
  NRF_GPIOTE->CONFIG[EVENT_SRDY_ASSERTED] = GPIOTE_CONFIG_P0_09_ANT_SRDY_CNF;
  NRF_GPIOTE->CONFIG[EVENT_MRDY_ASSERTED] = GPIOTE_CONFIG_P0_08_ANT_MRDY_CNF;

  /* Initialize pending events and enable the peripheral */
  NRF_GPIOTE->EVENTS_IN[EVENT_SRDY_ASSERTED] = 0;
  NRF_GPIOTE->EVENTS_IN[EVENT_MRDY_ASSERTED] = 0;
  NRF_SPI0->EVENTS_READY = 0;
  NRF_SPI0->ENABLE = 1;

#if 0 /* For now, let this be event-driven */
  /* Configure Interrupts */
#ifdef SOFTDEVICE_ENABLED  
  u32 u32Result;

  /* Enable the interrupt via the SD */
  u32Result |= sd_nvic_SetPriority(SPI0_TWI0_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_ClearPendingIRQ(SPI0_TWI0_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_EnableIRQ(SPI0_TWI0_IRQn);
#else

#ifdef INTERRUPTS_ENABLED
  /* Enable the SPI interrupt */
  NVIC_SetPriority(SPI0_TWI0_IRQn, NRF_APP_PRIORITY_LOW);
  NVIC_EnableIRQ(SPI0_TWI0_IRQn);
#endif /* INTERRUPTS_ENABLED */
  
#endif /* SOFTDEVICE_ENABLED */

#endif
    
} /* end SpiMasterInitialize() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
