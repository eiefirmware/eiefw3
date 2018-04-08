/*!**********************************************************************************************************************
@file leds_eief1_nRF51.c                                                                
@brief LED driver that provides on, off, toggle, blink and PWM functionality.

The basic on/off/toggle functionality is applied directly to the LEDs.
Blinking and PWMing of LEDs rely on a 1ms system tick to provide timing at
regular 1ms calls to LedUpdate().

------------------------------------------------------------------------------------------------------------------------
API:
LedNameType: BLUE, GREEN, YELLOW, RED

LedRateType: LED_0_5HZ, LED_1HZ, LED_2HZ, LED_4HZ, LED_8HZ, 
             LED_PWM_0, LED_PWM_5, ..., LED_PWM_100
*Note that PWM values in LedRateType are continuous, thus stepping a variable of LedRateType by one will select the next 
PWM level. However, decrementing past LED_PWM_0 or incrementing past LED_PWM_100 is undefined.

Public:
void LedOn(LedNameType eLED_)
void LedOff(LedNameType eLED_)
void LedToggle(LedNameType eLED_)
void LedPWM(LedNameType eLED_, LedRateType ePwmRate_)
void LedBlink(LedNameType eLED_, LedRateType eBlinkRate_)

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
extern u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Led_" and be declared as static.
***********************************************************************************************************************/
/* LED locations: order must correspond to the order set in LedNameType in the header file. */
static u32 Led_au32BitPositions[] = {P0_26_LED_BLU, P0_27_LED_GRN, P0_28_LED_YLW, P0_29_LED_RED};

/* Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
     eMode         eRate      u16Count       eCurrentDuty     eActiveState        LedNameType */
static LedConfigType Leds_asLedArray[TOTAL_LEDS] = 
{{LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /* BLUE       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /* GREEN      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /* YELLOW     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH}, /* RED        */
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
void LedOn(LedNameType eLED_)
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
void LedOff(LedNameType eLED_)
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
void LedToggle(LedNameType eLED_)
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
void LedPWM(LedNameType eLED_, LedRateType ePwmRate_)
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
void LedBlink(LedNameType eLED_, LedRateType eBlinkRate_)
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
  //u32 u32Timer;

  /* A simple on, wait, off sequence to test the driver */
  LedOn(BLUE);
  LedOn(RED);
  LedOn(GREEN);
  LedOn(YELLOW);
  for(u32 i = 0; i < 2000000; i++);
  LedOff(BLUE);
  LedOff(RED);
  LedOff(GREEN);
  LedOff(YELLOW);
  
#if 0
  /* Turn all LEDs on full, then fade them out over a few seconds */
  for(u8 i = 20; i > 0; i--)
  {
    
    /* Spend 40ms in each level of intensity */
    for(u16 j = 40; j > 0; j--)
    {
      u32Timer = G_u32SystemTime1ms;
      while( !IsTimeUp(&u32Timer, 1) );
      LedUpdate();
    }
    /* Pause for a bit on the first iteration to show the LEDs on for little while */
    if(i == 20)
    {
      while( !IsTimeUp(&u32Timer, 1500) );
    }
    
    /* Set the LED intensity for the next iteration */
    for(u8 j = 0; j < TOTAL_LEDS; j++)
    {
      Leds_asLedArray[j].eRate = (LedRateType)(i - 1);
    }
  }

  /* Final update to set last state, hold for a short period */
  LedUpdate();
  while( !IsTimeUp(&u32Timer, 200) );
#endif
  
  
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
    if(Leds_asLedArray[(LedNameType)i].eMode == LED_PWM_MODE)
    {
      /* Handle special case of 0% duty cycle */
      if( Leds_asLedArray[i].eRate == LED_PWM_0 )
      {
        LedOff( (LedNameType)i );
      }
      
      /* Handle special case of 100% duty cycle */
      else if( Leds_asLedArray[i].eRate == LED_PWM_100 )
      {
        LedOn( (LedNameType)i );
      }
  
      /* Otherwise, regular PWM: decrement counter; toggle and reload if counter reaches 0 */
      else
      {
        if(--Leds_asLedArray[(LedNameType)i].u16Count == 0)
        {
          if(Leds_asLedArray[(LedNameType)i].eCurrentDuty == LED_PWM_DUTY_HIGH)
          {
            /* Turn the LED off and update the counters for the next cycle */
            LedOff( (LedNameType)i );
            Leds_asLedArray[(LedNameType)i].u16Count = LED_PWM_PERIOD - Leds_asLedArray[(LedNameType)i].eRate;
            Leds_asLedArray[(LedNameType)i].eCurrentDuty = LED_PWM_DUTY_LOW;
          }
          else
          {
            /* Turn the LED on and update the counters for the next cycle */
            LedOn( (LedNameType)i );
            Leds_asLedArray[i].u16Count = Leds_asLedArray[i].eRate;
            Leds_asLedArray[i].eCurrentDuty = LED_PWM_DUTY_HIGH;
          }
        }
      }

      /* Set the LED back to PWM mode since LedOff and LedOn set it to normal mode */
     	Leds_asLedArray[(LedNameType)i].eMode = LED_PWM_MODE;
      
    } /* end if PWM mode */
    
    /* LED is in LED_BLINK_MODE mode */
    else if(Leds_asLedArray[(LedNameType)i].eMode == LED_BLINK_MODE)
    {
      /* Decrement counter; toggle and reload if counter reaches 0 */
      if( --Leds_asLedArray[(LedNameType)i].u16Count == 0)
      {
        LedToggle( (LedNameType)i );
        Leds_asLedArray[(LedNameType)i].u16Count = Leds_asLedArray[(LedNameType)i].eRate;
      }
    }
  } /* end for */
} /* end LedUpdate() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/


