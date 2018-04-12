/***********************************************************************************************************************
File: anttt-eief1-01.c                                                                

Description:
This file provides board support functions for the nRF51422 processor on the Engenuics eief1-01 boards.
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAnttt"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SystemTime1ms;                       /* Global system time incremented every ms, max 2^32 (~49 days) */
volatile u32 G_u32SystemTime1s;                        /* Global system time incremented every second, max 2^32 (~136 years) */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                 /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_" and be declared as static.
***********************************************************************************************************************/


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn bool ClockSetup(void)
@brief Loads all registers required to set up the processor clocks.  

The main clock, HFCLK is sourced from the
16MHz crystal.  The slow clock, LFCLK, will be synthesized from the 16MHz.

Requires:
- NONE

Promises:
- HFCLK is running.
- Returns status of SD SVCALLs

*/
bool ClockSetup(void)
{
  u32 u32Result = NRF_SUCCESS;
  u32 u32ClockStartTimeout = OSC_STARTUP_TIMOUT;
  
#ifdef SOFTDEVICE_ENABLED  
  u32 u32Status = 0;

  /* Ensure the 16MHz Crystal is running */
  u32Result |= sd_clock_hfclk_is_running(&u32Status);
  if(!u32Status)
  {
    /* Use the SD SVCALLs to start the HF clock and wait for it to run */
    u32Result |= sd_clock_hfclk_request();
    while( !u32Status && (u32ClockStartTimeout != 0) )
    {
      u32Result |= sd_clock_hfclk_is_running(&u32Status);
      u32ClockStartTimeout--;
    }
    
    /* Check for timeout - if the clock didn't start, turn it off and flag it */
    if(u32ClockStartTimeout == 0)
    {
      G_u32SystemFlags |= _SYSTEM_HFCLK_NO_START;
    }
  }
#else  
  /* Start the main clock (HFCLK) and wait for the event to indicate it has started */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  while( !NRF_CLOCK->EVENTS_HFCLKSTARTED && (--u32ClockStartTimeout != 0) );
  
  /* Check for timeout - if the clock didn't start, turn it off and flag it */
  if(u32ClockStartTimeout == 0)
  {
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
    G_u32SystemFlags |= _SYSTEM_HFCLK_NO_START;
  }
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  
  /* Setup and start the 32.768kHz (LFCLK) clock (synthesized from HFCLK) */
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Synth << CLOCK_LFCLKSRC_SRC_Pos);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  
  /* No need for timeout as an HFCLK of some sort is guaranteed to be running so LFCLK has to start */
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;  
#endif

  return (u32Result == NRF_SUCCESS);
  
} /* end ClockSetup */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void GpioSetup(void)
@brief Loads all registers required to set up GPIO on the processor.

Requires:
- All configurations must match connected hardware.

Promises:
- All I/O lines are set for their required function and start-state

*/
void GpioSetup(void)
{
  /* Set all of the pin function registers in port 0 */
  NRF_GPIO->PIN_CNF[P0_29_INDEX] = P0_29_LED_RED_CNF;
  NRF_GPIO->PIN_CNF[P0_28_INDEX] = P0_28_LED_YLW_CNF;
  NRF_GPIO->PIN_CNF[P0_27_INDEX] = P0_27_LED_GRN_CNF;
  NRF_GPIO->PIN_CNF[P0_26_INDEX] = P0_26_LED_BLU_CNF;
  
  NRF_GPIO->PIN_CNF[P0_16_INDEX] = P0_16_TP65_CNF;

  NRF_GPIO->PIN_CNF[P0_15_INDEX] = P0_15_NRF_POMI_CNF;
  NRF_GPIO->PIN_CNF[P0_14_INDEX] = P0_14_NRF_PIMO_CNF;

#if 0 /* Leave the SPI pins unconfigured for now */
  NRF_GPIO->PIN_CNF[P0_13_INDEX] = P0_13_ANT_USPI2_MOSI_CNF;
  NRF_GPIO->PIN_CNF[P0_12_INDEX] = P0_12_ANT_USPI2_MISO_CNF;
  NRF_GPIO->PIN_CNF[P0_11_INDEX] = P0_11_ANT_USPI2_SCK_CNF;
  NRF_GPIO->PIN_CNF[P0_10_INDEX] = P0_10_ANT_USPI2_CS_CNF;
#endif
  
} /* end GpioSetup() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void WatchDogSetup(void)
@brief Configures the watchdog timer.  

The dog runs at 32.768kHz from the slow built-in RC clock source which varies 
over operating conditions from 30kHz to 60kHz.
Since the main loop time / sleep time should be 1 ms most of the time, choosing a value
of 5 seconds should be plenty to avoid watchdog resets.  

Note: the processor allows the MR register to be written just once.

Requires:
- NONE

Promises:
- Watchdog is set for 5 second timeout but not yet enabled

*/
void WatchDogSetup(void)
{
  /* Currently not configured */
  
} /* end WatchDogSetup() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void PowerSetup(void)
@brief Loads registers to configure various power control features of the 51422.

Requires:
- NONE

Promises:
- 

*/
void PowerSetup(void)
{
  /* Set the sub power mode to constant latency (pg. 42 in the ref manual) */
  NRF_POWER->TASKS_LOWPWR   = 0;
  NRF_POWER->TASKS_CONSTLAT = 1;
  
} /* end PowerSetup() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void SysTickSetup(void)
@brief Initializes the 1ms System Tick from the RTC1 peripheral.

Requires:
- ClockSetup() to be called prior to ensure that HFCLK and LFCLK are already running.
- SoftDevice has been enabled.

Promises:
- RTC1 is active and providing a 1ms interrupt.

*/
bool SysTickSetup(void)
{
  u32 u32Result = NRF_SUCCESS;

  /* Configure the RTC to give a 1ms tick */
  NRF_RTC1->TASKS_STOP = 1;
  NRF_RTC1->PRESCALER = RTC_PRESCALE_INIT;
  NRF_RTC1->INTENSET = (1 << RTC_INTENSET_TICK_Pos);
  
  /* Clear then start the RTC */
  NRF_RTC1->TASKS_CLEAR = 1;
  NRF_RTC1->TASKS_START = 1;

#ifdef SOFTDEVICE_ENABLED  
  /* Enable the interrupt via the SD */
  u32Result |= sd_nvic_SetPriority(RTC1_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_EnableIRQ(RTC1_IRQn);
#else

#ifdef INTERRUPTS_ENABLED
  /* Enable the RTC interrupt */
  NVIC_SetPriority(RTC1_IRQn, NRF_APP_PRIORITY_LOW);
  NVIC_EnableIRQ(RTC1_IRQn);
#endif /* INTERRUPTS_ENABLED */
  
#endif /* SOFTDEVICE_ENABLED */
  
  return (u32Result == NRF_SUCCESS);
  
} /* end SysTickSetup() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void SystemSleep(void)
@brief Puts the system into sleep mode. 

Requires:
- SoftDevice is enabled.

Promises:
- Configures processor for maximum sleep while still allowing any required
  interrupt to wake it up.

*/
void SystemSleep(void)
{    
#ifdef SOFTDEVICE_ENABLED  
  sd_app_evt_wait();
#else

#ifdef INTERRUPTS_ENABLED
  __WFI();
#else
  for(u32 i = 0; i < 1600; i++);

  /* Update global counters */
  G_u32SystemTime1ms++;
  if ((G_u32SystemTime1ms % 1000) == 0)
  {
    G_u32SystemTime1s++;
    LedToggle(YELLOW);
  }
#endif /* INTERRUPTS_ENABLED */
  
#endif /* SOFTDEVICE_ENABLED */  
  
} /* end SystemSleep(void) */





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/



