/*!**********************************************************************************************************************
@file eief1_nrf51-01.h                                                                
@brief Provides board support functions for the nRF51422 processor 
on the Engenuics eief1-01 boards.
***********************************************************************************************************************/

#ifndef __EIEF1_NRF51
#define __EIEF1_NRF51

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
void GpioSetup(void);
void WatchDogSetup(void);

bool ClockSetup(void);
void PowerSetup(void);
bool InterruptsSetup(void);
bool SysTickSetup(void);
void SystemSleep(void);


/***********************************************************************************************************************
Perihperal Setup Initializations

Bookmarks:
@@@@@ Clock, Power Control, Systick and Watchdog setup values
!!!!! GPIO pin names
##### GPIO initial setup values

***********************************************************************************************************************/

/***********************************************************************************************************************
@@@@@ Clock, Systick and Power Control setup values
***********************************************************************************************************************/
#define FOSC                      __SYSTEM_CLOCK    /* Crystal speed from system_nrf51.c */
#define OSC_STARTUP_TIMOUT        (u32)1000000      /* Timeout for oscillator to start up */
#define LFCLK_FREQ                (u32)32768
#define HFCLK_FREQ                (u32)16000000

#define RTC_PRESCALE_INIT         (u32)31         /* Set for ~1ms: (32768Hz / 1000Hz) - 1 = 31 */


/***********************************************************************************************************************
!!!!! GPIO pin names
***********************************************************************************************************************/
/* Hardware Definition for nRF51422 on EIEF1-01 */

/* Port 0 bit positions */
#define P0_31_                    (u32)0x80000000
#define P0_30_NC                  (u32)0x40000000
#define P0_29_LED_RED             (u32)0x20000000
#define P0_28_LED_YLW             (u32)0x10000000
#define P0_27_LED_GRN             (u32)0x08000000
#define P0_26_LED_BLU             (u32)0x04000000 
#define P0_25_NC                  (u32)0x02000000
#define P0_24_NC                  (u32)0x01000000
#define P0_23_NC                  (u32)0x00800000
#define P0_22_NC                  (u32)0x00400000 
#define P0_21_NC                  (u32)0x00200000
#define P0_20_NC                  (u32)0x00100000
#define P0_19_NC                  (u32)0x00080000
#define P0_18_NC                  (u32)0x00040000
#define P0_17_NC                  (u32)0x00020000
#define P0_16_TP65                (u32)0x00010000
#define P0_15_NRF_POMI            (u32)0x00008000
#define P0_14_NRF_PIMO            (u32)0x00004000
#define P0_13_ANT_USPI2_MOSI      (u32)0x00002000
#define P0_12_ANT_USPI2_MISO      (u32)0x00001000
#define P0_11_ANT_USPI2_SCK       (u32)0x00000800
#define P0_10_ANT_SEN             (u32)0x00000400
#define P0_10_ANT_USPI2_CS        (u32)0x00000400
#define P0_09_ANT_SRDY            (u32)0x00000200
#define P0_08_ANT_MRDY            (u32)0x00000100
#define P0_07_NC                  (u32)0x00000080
#define P0_06_NC                  (u32)0x00000040
#define P0_05_NC                  (u32)0x00000020
#define P0_04_NC                  (u32)0x00000010
#define P0_03_NC                  (u32)0x00000008
#define P0_02_NC                  (u32)0x00000004
#define P0_01_NC                  (u32)0x00000002 
#define P0_00_NC                  (u32)0x00000001 

#define LED_RED_BIT_NUMBER        (u32)29
#define LED_YLW_BIT_NUMBER        (u32)28
#define LED_GRN_BIT_NUMBER        (u32)27
#define LED_BLU_BIT_NUMBER        (u32)26 
#define TP65_BIT_NUMBER           (u32)16
#define NRF_POMI_BIT_NUMBER       (u32)15
#define NRF_PIMO_BIT_NUMBER       (u32)14
#define ANT_USPI2_MOSI_BIT_NUMBER (u32)13
#define ANT_USPI2_MISO_BIT_NUMBER (u32)12
#define ANT_USPI2_SCK_BIT_NUMBER  (u32)11
#define ANT_SEN_BIT_NUMBER        (u32)10
#define ANT_SRDY_BIT_NUMBER       (u32)9
#define ANT_MRDY_BIT_NUMBER       (u32)8


#define P0_31_INDEX               (u32)31
#define P0_30_INDEX               (u32)30
#define P0_29_INDEX               (u32)29
#define P0_28_INDEX               (u32)28
#define P0_27_INDEX               (u32)27
#define P0_26_INDEX               (u32)26
#define P0_25_INDEX               (u32)25
#define P0_24_INDEX               (u32)24
#define P0_23_INDEX               (u32)23
#define P0_22_INDEX               (u32)22
#define P0_21_INDEX               (u32)21
#define P0_20_INDEX               (u32)20
#define P0_19_INDEX               (u32)19
#define P0_18_INDEX               (u32)18
#define P0_17_INDEX               (u32)17
#define P0_16_INDEX               (u32)16
#define P0_15_INDEX               (u32)15
#define P0_14_INDEX               (u32)14
#define P0_13_INDEX               (u32)13
#define P0_12_INDEX               (u32)12
#define P0_11_INDEX               (u32)11
#define P0_10_INDEX               (u32)10
#define P0_09_INDEX               (u32)9
#define P0_08_INDEX               (u32)8
#define P0_07_INDEX               (u32)7
#define P0_06_INDEX               (u32)6
#define P0_05_INDEX               (u32)5
#define P0_04_INDEX               (u32)4
#define P0_03_INDEX               (u32)3
#define P0_02_INDEX               (u32)2
#define P0_01_INDEX               (u32)1
#define P0_00_INDEX               (u32)0



/***********************************************************************************************************************
##### GPIO setup values
***********************************************************************************************************************/

/* Pin configuration - Pins are set up using the PIN_CNF registers 
(one register per pin) */

#define P0_29_LED_RED_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_28_LED_YLW_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_27_LED_GRN_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0H1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_26_LED_BLU_CNF     ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_16_TP65_CNF        ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

/* UART Pins */
#define P0_15_NRF_POMI_CNF    ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_14_NRF_PIMO_CNF    ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

/* SPI0 Pins + Handshaking Pins */
#define P0_13_ANT_USPI2_MOSI_CNF ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                   (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                   (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                   (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_12_ANT_USPI2_MISO_CNF ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                   (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                   (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                   (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                   (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_11_ANT_USPI2_SCK_CNF  ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                   (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                   (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                   (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_10_ANT_USPI2_CS_CNF   ( (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)   | \
                                   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | \
                                   (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                   (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                   (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )
                                
#define P0_09_ANT_SRDY_CNF       ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                   (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                   (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                   (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                   (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )

#define P0_08_ANT_MRDY_CNF       ( (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)   | \
                                   (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) | \
                                   (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  | \
                                   (GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos) | \
                                   (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) )


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/




#endif /* __EIEF1_NRF51 */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
