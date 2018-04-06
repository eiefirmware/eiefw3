/***********************************************************************************************************************
File: anttt-eief1-01.h                                                                
 
Description:
This file provides board support functions for the nRF51422 processor on the Engenuics eief1-01 boards.
***********************************************************************************************************************/

#ifndef __ANTTT_H
#define __ANTTT_H

/***********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Constants
***********************************************************************************************************************/


/***********************************************************************************************************************
* Macros
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void WatchDogSetup(void);
void PowerSetup(void);
void GpioSetup(void);
void ClockSetup(void);
void InterruptSetup(void);
void SysTickSetup(void);
void SystemSleep(void);
bool SystemEnterCriticalSection(u8* nested_status);
bool SystemExitCriticalSection(u8 nested_status);


/***********************************************************************************************************************
Perihperal Setup Initializations

Bookmarks:
@@@@@ Clock, Power Control, Systick and Watchdog setup values
!!!!! GPIO pin names
##### GPIO initial setup values
$$$$$ PWM setup values

***********************************************************************************************************************/

/***********************************************************************************************************************
@@@@@ Clock, Systick and Power Control setup values
***********************************************************************************************************************/
#define FOSC                 __SYSTEM_CLOCK    /* Crystal speed from system_nrf51.c */
#define OSC_STARTUP_TIMOUT   (u32)1000000      /* Timeout for oscillator to start up */
  
/* Timer 1
To get roughly a 1ms tick, set the prescale register value to 0 which results in a prescale value of 1.
Then we can count up to 33 to get a 1.0071ms tick.  That gives 993 for the value to count to for the 1 second tick. */

#define LFCLK_FREQ           (u32)32768
#define HFCLK_FREQ           (u32)16000000

#define RTC_PRESCALE_INIT    (u32)5         /* 2^5 = 32, 32768 / 32 = 1024Hz = 0.0009765625s period ~= 1ms */

/* Watch Dog Values */

/* TIMER
The built-in timer will provide the system tick
It is clocked from HFCLK.  To get the desired 1ms tick use a compare period of 0.001 / (1/HFCLK) or HFCLK/1000.
*/
#define TIMER_COUNT_1MS      (u32)(HFCLK_FREQ / 1000)


/***********************************************************************************************************************
!!!!! GPIO pin names
***********************************************************************************************************************/
/* Hardware Definition for nRF51422 on EIEF1-01 */

/* Port 0 bit positions */
#define P0_31_     (u32)0x80000000
#define P0_30_     (u32)0x40000000
#define P0_29_     (u32)0x20000000
#define P0_28_     (u32)0x10000000
#define P0_27_     (u32)0x08000000
#define P0_26_     (u32)0x04000000 
#define P0_25_     (u32)0x02000000
#define P0_24_     (u32)0x01000000
#define P0_23_     (u32)0x00800000
#define P0_22_     (u32)0x00400000 
#define P0_21_     (u32)0x00200000
#define P0_20_     (u32)0x00100000
#define P0_19_     (u32)0x00080000
#define P0_18_     (u32)0x00040000
#define P0_17_     (u32)0x00020000
#define P0_16_     (u32)0x00010000
#define P0_15_     (u32)0x00008000
#define P0_14_     (u32)0x00004000
#define P0_13_     (u32)0x00002000
#define P0_12_     (u32)0x00001000
#define P0_11_     (u32)0x00000800
#define P0_10_     (u32)0x00000400
#define P0_09_     (u32)0x00000200
#define P0_08_     (u32)0x00000100
#define P0_07_     (u32)0x00000080
#define P0_06_     (u32)0x00000040
#define P0_05_     (u32)0x00000020
#define P0_04_     (u32)0x00000010
#define P0_03_     (u32)0x00000008
#define P0_02_     (u32)0x00000004
#define P0_01_     (u32)0x00000002 
#define P0_00_     (u32)0x00000001 

#define P0_31_INDEX          (u32)31
#define P0_30_INDEX          (u32)30
#define P0_29_INDEX          (u32)29
#define P0_28_INDEX          (u32)28
#define P0_27_INDEX          (u32)27
#define P0_26_INDEX          (u32)26
#define P0_25_INDEX          (u32)25
#define P0_24_INDEX          (u32)24
#define P0_23_INDEX          (u32)23
#define P0_22_INDEX          (u32)22
#define P0_21_INDEX          (u32)21
#define P0_20_INDEX          (u32)20
#define P0_19_INDEX          (u32)19
#define P0_18_INDEX          (u32)18
#define P0_17_INDEX          (u32)17
#define P0_16_INDEX          (u32)16
#define P0_15_INDEX          (u32)15
#define P0_14_INDEX          (u32)14
#define P0_13_INDEX          (u32)13
#define P0_12_INDEX          (u32)12
#define P0_11_INDEX          (u32)11
#define P0_10_INDEX          (u32)10
#define P0_09_INDEX          (u32)9
#define P0_08_INDEX          (u32)8
#define P0_07_INDEX          (u32)7
#define P0_06_INDEX          (u32)6
#define P0_05_INDEX          (u32)5
#define P0_04_INDEX          (u32)4
#define P0_03_INDEX          (u32)3
#define P0_02_INDEX          (u32)2
#define P0_01_INDEX          (u32)1
#define P0_00_INDEX          (u32)0



/***********************************************************************************************************************
##### GPIO setup values
***********************************************************************************************************************/

/* Pin configuration - Pins are set up using the PIN_CNF registers (one register per pin)
*/
#define P0_29_LED_RED_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )


                                
                                

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/




#endif /* __ANTTT_H */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
