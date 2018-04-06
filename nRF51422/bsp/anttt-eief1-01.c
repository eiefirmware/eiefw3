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

/*----------------------------------------------------------------------------------------------------------------------
Function: WatchDogSetup

Description:
Configures the watchdog timer.  The dog runs at 32.768kHz from the slow built-in RC clock source which varies 
over operating conditions from 30kHz to 60kHz.
Since the main loop time / sleep time should be 1 ms most of the time, choosing a value
of 5 seconds should be plenty to avoid watchdog resets.  

Note: the processor allows the MR register to be written just once.

Requires:
  - 

Promises:
  - Watchdog is set for 5 second timeout but not yet enabled
*/
void WatchDogSetup(void)
{
  /* Currently not configured */
  
} /* end WatchDogSetup() */


/*----------------------------------------------------------------------------------------------------------------------
Function: GpioSetup

Description
Loads all registers required to set up GPIO on the processor.

Requires:
  - All configurations must match connected hardware.

Promises:
  - All I/O lines are set for their required function and start-state
*/
void GpioSetup(void)
{
  /* Set all of the pin function registers in port 0 */
  NRF_GPIO->PIN_CNF[P0_29_INDEX] = P0_29_LED_RED_CNF;
   
} /* end GpioSetup() */


/*----------------------------------------------------------------------------------------------------------------------
Function: PowerSetup

Description
Loads registers to configure various power control features of the 51422.

Requires:
  - 

Promises:
  - 
*/
void PowerSetup(void)
{
  /* Set the sub power mode to constant latency (pg. 42 in the ref manual) */
  NRF_POWER->TASKS_LOWPWR   = 0;
  NRF_POWER->TASKS_CONSTLAT = 1;
  
} /* end PowerSetup() */


/*----------------------------------------------------------------------------------------------------------------------
Function: ClockSetup

Description:
Loads all registers required to set up the processor clocks.  The main clock, HFCLK is sourced from the
16MHz crystal.  The slow clock, LFCLK, will be synthesized from the 16MHz.

Requires:
  - None.

Promises:
  - HFCLK is running.
  - LFCLK is running.
*/
void ClockSetup(void)
{
  u32 u32ClockStartTimeout = OSC_STARTUP_TIMOUT;
  
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
  
} /* end ClockSetup */


/*----------------------------------------------------------------------------------------------------------------------
Function: InterruptSetup

Description:
Performs initial interrupt setup.

Requires:
  -

Promises:
  - 
*/
void InterruptSetup(void)
{
  
} /* end InterruptSetup */


/*----------------------------------------------------------------------------------------------------------------------
Function: SysTickSetup

Description:
Initializes the 1ms System Ticks from the RTC1 peripheral.

Requires:
  - ClockSetup() to be called prior to ensure that HFCLK and LFCLK are already running.
  - SoftDevice has been enabled.

Promises:
  - RTC1 is active and providing a 1ms interrupt.
*/
void SysTickSetup(void)
{
  /* Configure the RTC to give a 1ms tick */
  NRF_RTC1->TASKS_STOP = 1;
  NRF_RTC1->PRESCALER = RTC_PRESCALE_INIT;
  NRF_RTC1->EVTENSET = (1 << RTC_EVTEN_TICK_Pos);
  NRF_RTC1->INTENSET = (1 << RTC_INTENSET_TICK_Pos);
  
  /* Clear then start the RTC */
  NRF_RTC1->TASKS_CLEAR = 1;
  NRF_RTC1->TASKS_START = 1;
  
} /* end SysTickSetup() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SystemSleep

Description:
Puts the system into sleep mode. 

Requires:
  - SoftDevice is enabled.

Promises:
  - Configures processor for maximum sleep while still allowing any required
    interrupt to wake it up.
*/
void SystemSleep(void)
{    
    sd_app_evt_wait();
    
} /* end SystemSleep(void) */


/*----------------------------------------------------------------------------------------------------------------------
@fn bool SystemEnterCriticalSection(u8* pu8NestedStatus_)
@brief SD sourced function for disabling interrupts 

Requires:
- SoftDevice is enabled
- pu8NestedStatus_

Promises:
- Application interrupts will be disabled. 
*/
bool SystemEnterCriticalSection(u8* pu8NestedStatus_)
{
  sd_nvic_critical_region_enter(pu8NestedStatus_);
  
  return (pu8NestedStatus_ == 0);
}


/*----------------------------------------------------------------------------------------------------------------------
@fn bool SystemExitCriticalSection(u8 u8NestedStatus_)

@brief SD sourced function for re-enabling interrupts 

Requires:
- SoftDevice is enabled.

Promises:
- Application interrupts will be re-enabled if there are
no further nested critical sections.
*/
bool SystemExitCriticalSection(u8 u8NestedStatus_)
{
  sd_nvic_critical_region_exit(u8NestedStatus_);
  
  return (u8NestedStatus_ == 0);
}




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/



