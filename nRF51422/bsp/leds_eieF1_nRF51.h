/*!**********************************************************************************************************************
@file leds_eief1_nRF51.h                                                               
@brief Header file for leds_eief1_nRF51.c

******************************************************************************/

#ifndef __LEDS_H
#define __LEDS_H

#include "configuration.h"

/******************************************************************************
Type Definitions
******************************************************************************/
typedef enum {BLUE = 0, GREEN, YELLOW, RED} LedNameType;

typedef enum {LED_NORMAL_MODE, LED_PWM_MODE, LED_BLINK_MODE} LedModeType;
typedef enum {LED_ACTIVE_LOW = 0, LED_ACTIVE_HIGH = 1} LedActiveType;
typedef enum {LED_PWM_DUTY_LOW = 0, LED_PWM_DUTY_HIGH = 1} LedPWMDutyType;

#define LED_PWM_PERIOD    (u8)20

/* Standard blinky values.  If other values are needed, add them at the end of the enum */
typedef enum {LED_0_5HZ = 1000, LED_1HZ = 500, LED_2HZ = 250, LED_4HZ = 125, LED_8HZ = 63,
              LED_PWM_0 = 0, LED_PWM_5 = 1, LED_PWM_10 = 2, LED_PWM_15 = 3, LED_PWM_20 = 4, 
              LED_PWM_25 = 5, LED_PWM_30 = 6, LED_PWM_35 = 7, LED_PWM_40 = 8, LED_PWM_45 = 9, 
              LED_PWM_50 = 10, LED_PWM_55 = 11, LED_PWM_60 = 12, LED_PWM_65 = 13, LED_PWM_70 = 14, 
              LED_PWM_75 = 15, LED_PWM_80 = 16, LED_PWM_85 = 17, LED_PWM_90 = 18, LED_PWM_95 = 19, 
              LED_PWM_100 = LED_PWM_PERIOD
             } LedRateType;

typedef struct 
{
  LedModeType eMode;
  LedRateType eRate;
  u16 u16Count;
  LedPWMDutyType eCurrentDuty;
  LedActiveType eActiveState;
}LedConfigType;


/******************************************************************************
* Constants
******************************************************************************/
#define TOTAL_LEDS            (u8)4        /* Total number of LEDs in the system */



/******************************************************************************
* Function Declarations
******************************************************************************/
/* Public Functions */
void LedOn(LedNameType eLED_);
void LedOff(LedNameType eLED_);
void LedToggle(LedNameType eLED_);
void LedPWM(LedNameType eLED_, LedRateType ePwmRate_);
void LedBlink(LedNameType eLED_, LedRateType ePwmRate_);

/* Protected Functions */
void LedInitialize(void);

/* Private Functions */
void LedUpdate(void);


/******************************************************************************
* State Machine Function Prototypes
******************************************************************************/
void LedSM_Idle(void);       /* No blinking LEDs */
void LedSM_Blinky(void);     /* At least one blinky LED so values need checking */


#endif /* __LEDS_H */
