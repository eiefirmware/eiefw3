/***********************************************************************************************************************
* File: anttt-ehdw-04.h                                                                
* 
* Description:
* This file provides header information for the board support functions for nRF51422 processor on the anttt-ehdw-04 board.
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
/* Hardware Definition for nRF51422 on ANTTT-EHDW-01 */

/* Port 0 bit positions */
#define P0_31_               (u32)0x80000000
#define P0_30_LED_HOME_3     (u32)0x40000000
#define P0_29_LED_HOME_6     (u32)0x20000000
#define P0_28_LED_AWAY_3     (u32)0x10000000
#define P0_27_NC             (u32)0x08000000
#define P0_26_SW_ROW1        (u32)0x04000000 
#define P0_25_NC             (u32)0x02000000
#define P0_24_LED_AWAY_9     (u32)0x01000000
#define P0_23_COLUMN3        (u32)0x00800000
#define P0_22_LED_HOME_9     (u32)0x00400000 
#define P0_21_LED_AWAY_6     (u32)0x00200000
#define P0_20_LED_HOME_1     (u32)0x00100000
#define P0_19_LED_AWAY_1     (u32)0x00080000
#define P0_18_LED_AWAY_2     (u32)0x00040000
#define P0_17_LED_HOME_2     (u32)0x00020000
#define P0_16_BUZZER         (u32)0x00010000
#define P0_15_COLUMN2        (u32)0x00008000
#define P0_14_COLUMN1        (u32)0x00004000
#define P0_13_LED_AWAY_4     (u32)0x00002000
#define P0_12_LED_HOME_4     (u32)0x00001000
#define P0_11_LED_AWAY_7     (u32)0x00000800
#define P0_10_LED_HOME_7     (u32)0x00000400
#define P0_09_SW_ROW3        (u32)0x00000200
#define P0_08_SW_ROW2        (u32)0x00000100
#define P0_07_LED_AWAY_5     (u32)0x00000080
#define P0_06_LED_HOME_5     (u32)0x00000040
#define P0_05_LED_STATUS_RED (u32)0x00000020
#define P0_04_LED_STATUS_YLW (u32)0x00000010
#define P0_03_LED_STATUS_GRN (u32)0x00000008
#define P0_02_LED_AWAY_8     (u32)0x00000004
#define P0_01_LED_HOME_8     (u32)0x00000002 
#define P0_00_GND            (u32)0x00000001 

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

/* Pin configuration (pg. 58) - Pins are set up using the PIN_CNF registers (one register per pin)
*/
#define P0_30_LED_HOME_3_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_29_LED_HOME_6_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_28_LED_AWAY_3_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_27_NC_CNF          ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_26_SW_ROW1_CNF     ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_25_NC_CNF          ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_24_LED_AWAY_9_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_23_COLUMN3_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_22_LED_HOME_9_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_21_LED_AWAY_6_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_20_LED_HOME_1_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_19_LED_AWAY_1_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_18_LED_AWAY_2_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_17_LED_HOME_2_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Pulldown    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_16_BUZZER_CNF      ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_15_COLUMN2_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_14_COLUMN1_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_13_LED_AWAY_4_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_12_LED_HOME_4_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_11_LED_AWAY_7_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_10_LED_HOME_7_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_09_SW_ROW3_CNF     ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_08_SW_ROW2_CNF     ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_07_LED_AWAY_5_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_06_LED_HOME_5_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_05_LED_STATUS_RED_CNF ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_04_LED_STATUS_YLW_CNF ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_03_LED_STATUS_GRN_CNF ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_02_LED_AWAY_8_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_01_LED_HOME_8_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_00_GND_CNF         ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

                                
                                

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

#define TEMPLATE_INIT (u32)0x
/*
    31 [0] P0_31_
    30 [0] P0_30_LED_HOME_3
    29 [0] P0_29_LED_HOME_6
    28 [0] P0_28_LED_AWAY_3

    27 [0] P0_27_NC
    26 [0] P0_26_LED_BLUE
    25 [0] P0_25_NC
    24 [0] P0_24_LED_AWAY_9

    23 [0] P0_23_COLUMN3
    22 [0] P0_22_LED_HOME_9
    21 [0] P0_21_LED_AWAY_6
    20 [0] P0_20_LED_HOME_1

    19 [0] P0_19_LED_AWAY_1
    18 [0] P0_18_LED_AWAY_2
    17 [0] P0_17_LED_HOME_2
    16 [0] P0_16_BUZZER

    15 [0] P0_15_COLUMN2
    14 [0] P0_14_COLUMN1
    13 [0] P0_13_LED_AWAY_4
    12 [0] P0_12_LED_HOME_4

    11 [0] P0_11_LED_AWAY_7
    10 [0] P0_10_LED_HOME_7
    09 [0] P0_09_SW_ROW3
    08 [0] P0_08_SW_ROW2

    07 [0] P0_07_LED_AWAY_5
    06 [0] P0_06_LED_HOME_5
    05 [0] P0_05_LED_STATUS_RED
    04 [0] P0_04_LED_STATUS_YLW

    03 [0] P0_03_LED_STATUS_GRN
    02 [0] P0_02_LED_AWAY_8
    01 [0] P0_01_LED_HOME_8
    00 [0] P0_00_GND
*/




#endif /* __ANTTT_H */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
