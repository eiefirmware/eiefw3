/***********************************************************************************************************************
File: anttt-ehdw-04.c                                                                

Description:
This file provides board support functions for the nRF51422 processor on the Engenuics ANTTT boards.
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
  NRF_GPIO->PIN_CNF[P0_30_INDEX] = P0_30_LED_HOME_3_CNF;
  NRF_GPIO->PIN_CNF[P0_29_INDEX] = P0_29_LED_HOME_6_CNF;
  NRF_GPIO->PIN_CNF[P0_28_INDEX] = P0_28_LED_AWAY_3_CNF;
  NRF_GPIO->PIN_CNF[P0_27_INDEX] = P0_27_NC_CNF;
  NRF_GPIO->PIN_CNF[P0_26_INDEX] = P0_26_SW_ROW1_CNF;
  NRF_GPIO->PIN_CNF[P0_25_INDEX] = P0_25_NC_CNF;
  NRF_GPIO->PIN_CNF[P0_24_INDEX] = P0_24_LED_AWAY_9_CNF;
  NRF_GPIO->PIN_CNF[P0_23_INDEX] = P0_23_COLUMN3_CNF;
  NRF_GPIO->PIN_CNF[P0_22_INDEX] = P0_22_LED_HOME_9_CNF;
  NRF_GPIO->PIN_CNF[P0_21_INDEX] = P0_21_LED_AWAY_6_CNF;
  NRF_GPIO->PIN_CNF[P0_20_INDEX] = P0_20_LED_HOME_1_CNF;
  NRF_GPIO->PIN_CNF[P0_19_INDEX] = P0_19_LED_AWAY_1_CNF;
  NRF_GPIO->PIN_CNF[P0_18_INDEX] = P0_18_LED_AWAY_2_CNF;
  NRF_GPIO->PIN_CNF[P0_17_INDEX] = P0_17_LED_HOME_2_CNF;
  NRF_GPIO->PIN_CNF[P0_16_INDEX] = P0_16_BUZZER_CNF;
  NRF_GPIO->PIN_CNF[P0_15_INDEX] = P0_15_COLUMN2_CNF;
  NRF_GPIO->PIN_CNF[P0_14_INDEX] = P0_14_COLUMN1_CNF;
  NRF_GPIO->PIN_CNF[P0_13_INDEX] = P0_13_LED_AWAY_4_CNF;
  NRF_GPIO->PIN_CNF[P0_12_INDEX] = P0_12_LED_HOME_4_CNF;
  NRF_GPIO->PIN_CNF[P0_11_INDEX] = P0_11_LED_AWAY_7_CNF;
  NRF_GPIO->PIN_CNF[P0_10_INDEX] = P0_10_LED_HOME_7_CNF;
  NRF_GPIO->PIN_CNF[P0_09_INDEX] = P0_09_SW_ROW3_CNF;
  NRF_GPIO->PIN_CNF[P0_08_INDEX] = P0_08_SW_ROW2_CNF;
  NRF_GPIO->PIN_CNF[P0_07_INDEX] = P0_07_LED_AWAY_5_CNF;
  NRF_GPIO->PIN_CNF[P0_06_INDEX] = P0_06_LED_HOME_5_CNF;
  NRF_GPIO->PIN_CNF[P0_05_INDEX] = P0_05_LED_STATUS_RED_CNF;
  NRF_GPIO->PIN_CNF[P0_04_INDEX] = P0_04_LED_STATUS_YLW_CNF;
  NRF_GPIO->PIN_CNF[P0_03_INDEX] = P0_03_LED_STATUS_GRN_CNF;
  NRF_GPIO->PIN_CNF[P0_02_INDEX] = P0_02_LED_AWAY_8_CNF;
  NRF_GPIO->PIN_CNF[P0_01_INDEX] = P0_01_LED_HOME_8_CNF;
  NRF_GPIO->PIN_CNF[P0_00_INDEX] = P0_00_GND_CNF;
   
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



