/***********************************************************************************************************************
File: buttons.c                                                                

Description:
Button functions and state machine.  The application handles all debouncing and glitch filtering.

------------------------------------------------------------------------------------------------------------------------
API:
Types:

Public:
bool IsButtonPressed(u32 u32Button_)
Returns true if a particular button is currently pressed (and debounced).

bool WasButtonPressed(u32 u32Button_)
Returns true if a particular button was pressed since last time it was checked even if it is no longer pressed.
ButtonAcknowledge is typically called immediately after WasButtonPressed() returns true to clear the button
pressed state.

void ButtonAcknowledge(u32 u32Button_)
Clears the New Press state of a button -- generally always called after WasButtonPressed() returns true.

bool IsButtonHeld(u32 u32Button_, u32 u32ButtonHeldTime_)
Returns true if a button has been held for u32ButtonHeldTime_ time in milliseconds.

Protected:
void ButtonInitialize(void)
Configures the button system for the product including enabling button GPIO interrupts.  

u32 GetButtonBitLocation(u8 u8Button_, ButtonPortType ePort_)
Returns the location of the button within its port (should be required only for interrupt service routines).  


***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Button"
***********************************************************************************************************************/
/* New variables */
volatile bool G_abButtonDebounceActive[TOTAL_BUTTONS];           /* Flags for buttons being debounced */
volatile u32 G_au32ButtonDebounceTimeStart[TOTAL_BUTTONS];       /* Button debounce start time */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;         /* From board-specific source file */

extern volatile u32 G_u32SystemFlags;          /* From main.c */
extern volatile u32 G_u32ApplicationFlags;     /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Button_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Button_pfnStateMachine;                  /* The Button application state machine function pointer */

static ButtonStateType Button_aeCurrentState[TOTAL_BUTTONS];/* Current pressed state of button */
static ButtonStateType Button_aeNewState[TOTAL_BUTTONS];    /* New (pending) pressed state of button */
static u32 Button_au32HoldTimeStart[TOTAL_BUTTONS];         /* System 1ms time when a button press started */
static bool Button_abNewPress[TOTAL_BUTTONS];               /* Flags to indicate a button was pressed */    
static u8 Button_u8ActiveCol;                               /* Current Active Button Column */                                                                                    


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: IsButtonPressed

Description:
Determine if a particular button is currently pressed at this moment in time.
The button must still be pressed at the time of this inquiry for the function
to return true.

Requires:
  - u32Button_ is a valid button index
  - Button_aeCurrentState[u32Button_] is a valid index
 
Promises:
  - Returns true if Button_aeCurrentState[u32Button_] is pressed; otherwise returns false
*/
bool IsButtonPressed(u32 u32Button_)
{
  if( Button_aeCurrentState[u32Button_] == PRESSED)
  {
    return(true);
  }
  else
  {
    return(false);
  }

} /* end IsButtonPressed() */


/*----------------------------------------------------------------------------------------------------------------------
Function: WasButtonPressed

Description:
Determines if a particular button was pressed since last time it was checked. 
The button may or may not still be pressed when this inquiry is made.  Mulitple
button presses are not tracked.  The user should call ButtonAcknowledge immediately
following this function to clear the state.

Requires:
  - u32 u32Button_ is a valid button index
  - Button_aeCurrentState[u32Button_] is valid
 
Promises:
  - Returns true if Button_abNewPress[u32Button_] is true; other wise returns false
*/
bool WasButtonPressed(u32 u32Button_)
{
  if( Button_abNewPress[u32Button_] == true)
  {
    return(true);
  }
  else
  {
    return(false);
  }

} /* end WasButtonPressed() */


/*----------------------------------------------------------------------------------------------------------------------
Function: ButtonAcknowledge

Description:
Clears the New Press state of a button.

Requires:
  - u32Button_ is a valid button index
 
Promises:
  - The flag at Button_abNewPress[u32Button_] is set to false
*/
void ButtonAcknowledge(u32 u32Button_)
{
  Button_abNewPress[u32Button_] = false;

} /* end ButtonAcknowledge() */


/*----------------------------------------------------------------------------------------------------------------------
Function: IsButtonHeld

Description:
Queries to see if a button has been held for a certain time.  The button
must still be pressed when this function is called if it is to return true.

Requires:
  - u32Button_ is a valid button index
  - u32ButtonHeldTime is a time in ms 
 
Promises:
  - Returns true if eButton_ has been held longer than u32ButtonHeldTime_
*/
bool IsButtonHeld(u32 u32Button_, u32 u32ButtonHeldTime_)
{
 if( IsButtonPressed(u32Button_) && 
     IsTimeUp(&Button_au32HoldTimeStart[u32Button_], u32ButtonHeldTime_ ) )
 {
   return(true);
 }
 else
 {
   return(false);
 }

} /* end IsButtonHeld() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: ButtonInitialize

Description:
Configures the button system for the product including enabling button GPIO interrupts.  

Requires:
  - GPIO configuration is already complete for all button inputs
  - Button interrupt initializations and handler functions are ready
 
Promises:
  - G_abButtonDebounceActive, LGaeButtonPreviousState and Button_aeCurrentState 
    are intialized
  - GGstButtonTrackballPosition fields are all initialized to default values
  - The button state machine is initialized to Idle
*/
void ButtonInitialize(void)
{
  Button_u8ActiveCol = 0;      // Set initial active col.
  
  /* Setup default data for all of the buttons in the system */
  for(u8 i = 0; i < TOTAL_BUTTONS; i++)
  {
    G_abButtonDebounceActive[i] = false;
    Button_aeCurrentState[i]    = RELEASED;
    Button_aeNewState[i]        = RELEASED;
  }
  
  // Initialize BUTTON_COLS to initial state.
  nrf_gpio_pin_set(BUTTON_COL1_PIN);    // Disabled Column (ACTIVE LOW)
  nrf_gpio_pin_set(BUTTON_COL2_PIN);    // Disabled Column
  nrf_gpio_pin_set(BUTTON_COL3_PIN);    // Disabled Column
  
  // Enable Interrupts.
  nrf_gpiote_event_config(BUTTON_ROW1_GPIOTE_CHANNEL, BUTTON_ROW1_PIN, NRF_GPIOTE_POLARITY_TOGGLE);
  nrf_gpiote_event_config(BUTTON_ROW2_GPIOTE_CHANNEL, BUTTON_ROW2_PIN, NRF_GPIOTE_POLARITY_TOGGLE);
  nrf_gpiote_event_config(BUTTON_ROW3_GPIOTE_CHANNEL, BUTTON_ROW3_PIN, NRF_GPIOTE_POLARITY_TOGGLE);
  NRF_GPIOTE->INTENSET = (GPIOTE_INTENSET_IN0_Msk | GPIOTE_INTENSET_IN1_Msk | GPIOTE_INTENSET_IN2_Msk);  
  
  /* Init complete: set function pointer and application flag */
  Button_pfnStateMachine = ButtonSM_Idle;
  
 } /* end ButtonInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function ButtonRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void ButtonRunActiveState(void)
{
  Button_pfnStateMachine();

} /* end ButtonRunActiveState */



/*----------------------------------------------------------------------------------------------------------------------
Function ButtonGetActiveColumn()

Description:
Accesses private data

Requires:
  - None

Promises:
  - Provides Button_u8ActiveCol
*/
u8 ButtonGetActiveColumn(void)
{
   return Button_u8ActiveCol;
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function ButtonGetActiveColumn()

Description:
Accesses private data

Requires:
  - None

Promises:
  - Advances the active column matrix
*/
static void ButtonRotateColumns(void)
{
  if (G_u32SystemTime1ms % BUTTON_COLUMN_SWITCH_TIME_MS)
  {
   Button_u8ActiveCol++;
   if (Button_u8ActiveCol >= 3)
   {
     Button_u8ActiveCol = 0;
   }
   
   switch (Button_u8ActiveCol)
   {
    case 0:
      nrf_gpio_pin_clear(BUTTON_COL1_PIN);
      nrf_gpio_pin_set(BUTTON_COL2_PIN);
      nrf_gpio_pin_set(BUTTON_COL3_PIN);
      break;
      
    case 1:
      nrf_gpio_pin_clear(BUTTON_COL2_PIN);
      nrf_gpio_pin_set(BUTTON_COL1_PIN);
      nrf_gpio_pin_set(BUTTON_COL3_PIN);
      break;
      
    case 2:
      nrf_gpio_pin_clear(BUTTON_COL3_PIN);
      nrf_gpio_pin_set(BUTTON_COL1_PIN);
      nrf_gpio_pin_set(BUTTON_COL2_PIN);
      break;
      
    default:
      break;
   }
  }
}


/*----------------------------------------------------------------------------------------------------------------------
Function ButtonStillPressed()

Description:
Checks if a button is still pressed

Requires:
  - u8Button_ is the button of interest

Promises:
  - Checks if u8Button_ is currently pressed
*/
static bool ButtonStillPressed(u8 u8Button_)
{
   u8 u8Row = (u8Button_ - ButtonGetActiveColumn()) / 3;   // Row corresponding 

   // Map Row Index to Pin and check if pin is low (active low)
   if (u8Row == 0)
   {
     return (nrf_gpio_pin_read(BUTTON_ROW1_PIN) == 0);
   }
   else if (u8Row == 1)
   {
     return (nrf_gpio_pin_read(BUTTON_ROW2_PIN) == 0);
   }
   else if (u8Row == 2)
   {
     return (nrf_gpio_pin_read(BUTTON_ROW3_PIN) == 0);
   }

   return false;
   
} /* end ButtonStillPressed() */


/***********************************************************************************************************************
State Machine Function Definitions

The button state machine monitors button activity and manages debouncing and
maintaining the global button states.
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Do nothing but wait for a debounce time to start */
static void ButtonSM_Idle(void)                
{
  bool bIsAnyButtonDebouncing = false;
  u8 u8Status;
  
  for(u8 i = 0; i < TOTAL_BUTTONS; i++)
  {
    if(G_abButtonDebounceActive[i])
    {
      bIsAnyButtonDebouncing = true;
      Button_pfnStateMachine = ButtonSM_ButtonActive;
    }
  }
   
  // Check that no button is debouncing.
  if (!bIsAnyButtonDebouncing)
  {
     SystemEnterCriticalSection(&u8Status);
     ButtonRotateColumns();
     SystemExitCriticalSection(u8Status);  
  }
  
} /* end ButtonSM_Idle(void) */


/*--------------------------------------------------------------------------------------------------------------------*/
static void ButtonSM_ButtonActive(void)         
{
  /* Start by resseting back to Idle in case no buttons are active */
  Button_pfnStateMachine = ButtonSM_Idle;

  /* Check for buttons that are debouncing */
  for(u8 i = 0; i < TOTAL_BUTTONS; i++)
  {
    if(G_abButtonDebounceActive[i] )
    {
      /* Still have an active button */
      Button_pfnStateMachine = ButtonSM_ButtonActive;
      
      if( IsTimeUp((u32*)&G_au32ButtonDebounceTimeStart[i], BUTTON_DEBOUNCE_TIME) )
      {
         if(ButtonStillPressed(i))
         {
            Button_aeNewState[i] = PRESSED;
         }
         else
         {
            Button_aeNewState[i] = RELEASED;
         }
        
        /* Update if the button state has changed */
        if( Button_aeNewState[i] != Button_aeCurrentState[i] )
        {
          Button_aeCurrentState[i] = Button_aeNewState[i];
          if(Button_aeCurrentState[i] == PRESSED)
          {
            Button_abNewPress[i] = true;
            Button_au32HoldTimeStart[i] = G_u32SystemTime1ms;
          }
        }

        /* Regardless of a good press or not, clear the debounce active flag and re-enable the interrupts */
        G_abButtonDebounceActive[i] = false;
        NRF_GPIOTE->INTENSET = (GPIOTE_INTENSET_IN0_Msk | GPIOTE_INTENSET_IN1_Msk | GPIOTE_INTENSET_IN2_Msk);  
      } /* end if( IsTimeUp...) */
    } /* end if(G_abButtonDebounceActive[index]) */
  } /* end for i */

} /* end ButtonSM_ButtonActive() */





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
