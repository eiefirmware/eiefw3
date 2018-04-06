/***********************************************************************************************************************
File: leds_anttt.c                                                                

Description:
LED driver that provides on, off, toggle, blink and PWM functionality.
The basic on/off/toggle functionality is applied directly to the LEDs.
Blinking and PWMing of LEDs rely on a 1ms system tick to provide timing at
regular 1ms calls to LedUpdate().

------------------------------------------------------------------------------------------------------------------------
API:

Public:
void LedOn(LedNumberType eLED_)
void LedOff(LedNumberType eLED_)
void LedToggle(LedNumberType eLED_)
void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_)
void LedBlink(LedNumberType eLED_, LedRateType eBlinkRate_)

Protected:
void LedInitialize(void)

***********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxLed"
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* New variables (all shall start with G_xxLed*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern volatile u32 G_u32ApplicationFlags;             /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Led_" and be declared as static.
***********************************************************************************************************************/
/* LED locations: order must correspond to the order set in LedNumberType in the header file. */
static u32 Led_au32BitPositions[] = {P0_20_LED_HOME_1, P0_17_LED_HOME_2, P0_30_LED_HOME_3, P0_12_LED_HOME_4, P0_06_LED_HOME_5, 
                                     P0_29_LED_HOME_6, P0_10_LED_HOME_7, P0_01_LED_HOME_8, P0_22_LED_HOME_9, 
                                     P0_19_LED_AWAY_1, P0_18_LED_AWAY_2, P0_28_LED_AWAY_3, P0_13_LED_AWAY_4, P0_07_LED_AWAY_5, 
                                     P0_21_LED_AWAY_6, P0_11_LED_AWAY_7, P0_02_LED_AWAY_8, P0_24_LED_AWAY_9,
                                     P0_05_LED_STATUS_RED, P0_04_LED_STATUS_YLW, P0_03_LED_STATUS_GRN};

/* Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
     eMode         eRate      u16Count       eCurrentDuty     eActiveState     ePort      LedNumberType */
static LedConfigType Leds_asLedArray[TOTAL_LEDS] = 
{
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*        */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*       */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*         */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*        */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*        */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*       */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*       */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*        */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*       */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*        */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*       */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*        */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*       */
 {LED_NORMAL_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /*        */
};   
 

/***********************************************************************************************************************
* Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: LedOn

Description:
Turn the specified LED on.  Automatically takes care of the active low vs. active
high LEDs.  

Requires:
  - eLED_ is a valid LED index
  - Definitions in Leds_asLedArray[eLED_] are correct
  - Supports port 0 GPIO only

Promises:
  - Requested LED is turned on 
  - Requested LED is always set to LED_NORMAL_MODE mode
*/
void LedOn(LedNumberType eLED_)
{

  if(Leds_asLedArray[eLED_].eActiveState == LED_ACTIVE_HIGH)
  {
    NRF_GPIO->OUTSET = Led_au32BitPositions[eLED_];
  }
  else
  {
    NRF_GPIO->OUTCLR = Led_au32BitPositions[eLED_];
  }
  
  /* Always set the LED back to LED_NORMAL_MODE mode */
	Leds_asLedArray[(u8)eLED_].eMode = LED_NORMAL_MODE;

} /* end LedOn() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedOff

Description:
Turn the specified LED off.  Automatically takes care of the active low vs. active
high LEDs.  

Requires:
  - eLED_ is a valid LED index
  - Definitions in Leds_asLedArray[eLED_] are correct

Promises:
  - Requested LED is turned off
  - Requested LED is always set to LED_NORMAL_MODE mode
*/
void LedOff(LedNumberType eLED_)
{
  if(Leds_asLedArray[eLED_].eActiveState == LED_ACTIVE_HIGH)
  {
    NRF_GPIO->OUTCLR = Led_au32BitPositions[eLED_];
  }
  else
  {
    NRF_GPIO->OUTSET = Led_au32BitPositions[eLED_];
  }

  /* Always set the LED back to LED_NORMAL_MODE mode */
  Leds_asLedArray[(u8)eLED_].eMode = LED_NORMAL_MODE;
  
} /* end LedOff() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedToggle

Description:
Toggle the specified LED.

Requires:
  - eLED_ is a valid LED index
  - eLED_ *should* be in LED_NORMAL_MODE

Promises:
  - Requested LED is toggled
*/
void LedToggle(LedNumberType eLED_)
{
  u32 u32Current_Leds;
  
  u32Current_Leds = NRF_GPIO->IN;
  u32Current_Leds ^= Led_au32BitPositions[eLED_];
  NRF_GPIO->OUT = u32Current_Leds;
                                            
} /* end LedToggle() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedPWM

Description:
Sets an LED to PWM mode

Requires:
  - eLED_ is a valid LED index
  - ePwmRate_ is an allowed duty cycle:
    LED_PWM_0, LED_PWM_5, LED_PWM_10, ..., LED_PWM_95, LED_PWM_100

Promises:
  - Requested LED is set to PWM mode at the duty cycle specified
*/
void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_)
{
	Leds_asLedArray[(u8)eLED_].eMode = LED_PWM_MODE;
	Leds_asLedArray[(u8)eLED_].eRate = ePwmRate_;
	Leds_asLedArray[(u8)eLED_].u16Count = (u16)ePwmRate_;
  Leds_asLedArray[(u8)eLED_].eCurrentDuty = LED_PWM_DUTY_HIGH;

} /* end LedPWM() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedBlink

Description:
Sets an LED to BLINK mode.

Requires:
  - eLED_ is a valid LED index
  - eBlinkRate_ is an allowed frequency:
    LED_0_5HZ, LED_1HZ, LED_2HZ, LED_4HZ, LED_8HZ

Promises:
  - Requested LED is set to BLINK mode at the rate specified
*/
void LedBlink(LedNumberType eLED_, LedRateType eBlinkRate_)
{
	Leds_asLedArray[(u8)eLED_].eMode = LED_BLINK_MODE;
	Leds_asLedArray[(u8)eLED_].eRate = eBlinkRate_;
	Leds_asLedArray[(u8)eLED_].u16Count = eBlinkRate_;

} /* end LedBlink() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: LedInitialize

Description:
Initialization of LED system paramters and visual LED check.

Requires:
  - G_u32SystemTime1ms ticking
  - All LEDs already initialized to LED_NORMAL_MODE mode ON

Promises:
  - All LEDs in LED_NORMAL_MODE mode with OFF
*/
void LedInitialize(void)
{
  LedNumberType aeLedSequenceHome[] = {HOME1, HOME2, HOME3, HOME6, HOME9, HOME8, HOME7, HOME4};
  LedNumberType aeLedSequenceAway[] = {AWAY1, AWAY4, AWAY7, AWAY8, AWAY9, AWAY6, AWAY3, AWAY2};
  
  /* All status lights on */
  //LedOn(STATUS_RED);
  //LedOn(STATUS_YLW);
  //LedOn(STATUS_GRN);

#if 0 /* Picture mode */
  LedOn(HOME1);
  LedOn(HOME5);
  LedOn(HOME7);
  LedOn(HOME2);
  LedOn(AWAY9);
  LedOn(AWAY4);
  LedOn(AWAY6);
  LedOn(AWAY8);

  while(1);
#endif

  LedOff(HOME5);
  LedOn(AWAY5);
  
  
  /* Sequentially light up the LEDs (blocking is allowed during init)*/
  for(u8 i = 0; i < 8; i++)
  {
    LedToggle(HOME5);
    LedToggle(AWAY5);
    LedOn(aeLedSequenceHome[i]);
    LedOn(aeLedSequenceAway[i]);
    
    for(u32 j = 0; j < 200000; j++);
    
    LedOff(aeLedSequenceHome[i]);
    LedOff(aeLedSequenceAway[i]);
  }

#if 0
  /* Sequentially light up the LEDs */
  for(u8 i = 0; i < 18; i++)
  {
    LedOn(aeLedSequence[i]);
    for(u32 j = 0; j < 300000; j++);
  }
#endif

  /* Pause for show */
  for(u32 i = 0; i < 2000000; i++);

  
} /* end LedInitialize() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: LedUpdate

Description:
Update all LEDs for the current cycle.

Requires:
 - G_u32SystemTime1ms is counting

Promises:
   - All LEDs updated based on their counters
*/
void LedUpdate(void)
{
	/* Loop through each LED */
  for(u8 i = 0; i < TOTAL_LEDS; i++)
  {
    /* Check if LED is PWMing */
    if(Leds_asLedArray[(LedNumberType)i].eMode == LED_PWM_MODE)
    {
      /* Handle special case of 0% duty cycle */
      if( Leds_asLedArray[i].eRate == LED_PWM_0 )
      {
        LedOff( (LedNumberType)i );
      }
      
      /* Handle special case of 100% duty cycle */
      else if( Leds_asLedArray[i].eRate == LED_PWM_100 )
      {
        LedOn( (LedNumberType)i );
      }
  
      /* Otherwise, regular PWM: decrement counter; toggle and reload if counter reaches 0 */
      else
      {
        if(--Leds_asLedArray[(LedNumberType)i].u16Count == 0)
        {
          if(Leds_asLedArray[(LedNumberType)i].eCurrentDuty == LED_PWM_DUTY_HIGH)
          {
            /* Turn the LED off and update the counters for the next cycle */
            LedOff( (LedNumberType)i );
            Leds_asLedArray[(LedNumberType)i].u16Count = LED_PWM_PERIOD - Leds_asLedArray[(LedNumberType)i].eRate;
            Leds_asLedArray[(LedNumberType)i].eCurrentDuty = LED_PWM_DUTY_LOW;
          }
          else
          {
            /* Turn the LED on and update the counters for the next cycle */
            LedOn( (LedNumberType)i );
            Leds_asLedArray[i].u16Count = Leds_asLedArray[i].eRate;
            Leds_asLedArray[i].eCurrentDuty = LED_PWM_DUTY_HIGH;
          }
        }
      }

      /* Set the LED back to PWM mode since LedOff and LedOn set it to normal mode */
     	Leds_asLedArray[(LedNumberType)i].eMode = LED_PWM_MODE;
      
    } /* end if PWM mode */
    
    /* LED is in LED_BLINK_MODE mode */
    else if(Leds_asLedArray[(LedNumberType)i].eMode == LED_BLINK_MODE)
    {
      /* Decrement counter; toggle and reload if counter reaches 0 */
      if( --Leds_asLedArray[(LedNumberType)i].u16Count == 0)
      {
        LedToggle( (LedNumberType)i );
        Leds_asLedArray[(LedNumberType)i].u16Count = Leds_asLedArray[(LedNumberType)i].eRate;
      }
    }
  } /* end for */
  
} /* end LedUpdate() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/


