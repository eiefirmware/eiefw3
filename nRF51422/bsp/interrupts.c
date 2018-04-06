/**********************************************************************************************************************
File: interrupts.c                                                                

Description:
This is a interrupts .c file new source code.
System-level interrupt handlers are defined here.  Driver-specific handlers will be found in
their respective source files.

All SoC interrupts are in soc_integration.c

This might be too fragmented, so we reserve the right to change it up after we play with it for a while.

**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32InterruptsFlags;                     /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */
extern volatile bool G_abButtonDebounceActive[TOTAL_BUTTONS];
extern volatile u32 G_au32ButtonDebounceTimeStart[TOTAL_BUTTONS];


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Interrupts_" and be declared as static.
***********************************************************************************************************************/
//static u32 Interrupts_u32Timeout;                     /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------
Function: InterruptsInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  - None.

Promises:
  - Returns TRUE if SoftDevice Interrupts are successfully enabled, FALSE otherwise.
*/
bool InterruptsInitialize(void)
{
#define SD_PRESENT 1
  
#ifndef SD_PRESENT  
  NVIC_ClearPendingIRQ(RTC1_IRQn);
  NVIC_EnableIRQ(RTC1_IRQn);
  
  NVIC_ClearPendingIRQ(GPIOTE_IRQn);
  NVIC_EnableIRQ(GPIOTE_IRQn);
#else
  
  u32 u32Result = NRF_SUCCESS;
  
  // Must enable the SoftDevice Interrupt first.
  u32Result |= sd_nvic_SetPriority(SD_EVT_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_EnableIRQ(SD_EVT_IRQn);
  
  // Enable the RTC Peripheral.
  u32Result |= sd_nvic_SetPriority(RTC1_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_EnableIRQ(RTC1_IRQn);

  // Enable the GPIOTE Peripheral.
  u32Result |= sd_nvic_SetPriority(GPIOTE_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_EnableIRQ(GPIOTE_IRQn);

  return (u32Result == NRF_SUCCESS);
#endif

} /* end InterruptsInitialize() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Handlers                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

void HardFault_Handler(u32 u32ProgramCounter_, u32 u32LinkRegister_)
{
  (void)u32ProgramCounter_;
  (void)u32LinkRegister_;

   while(1); // loop for debugging
}


void TIMER1_IRQHandler(void)
{ 
  while(1);
}

void RTC1_IRQHandler(void)
{
  // Clear the Tick Event
  NRF_RTC1->EVENTS_TICK = 0;
  
  // Update global counters.
  G_u32SystemTime1ms++;
  if ((G_u32SystemTime1ms % 1000) == 0)
  {
    G_u32SystemTime1s++;
  }
}


/*--------------------------------------------------------------------------------------------------------------------
Interrupt handler: SD_EVT_IRQHandler

Description:
Processes soft device events.

Requires:
  - enabled via sd_nvic_XXX

Promises:
  -  Sets global system flags indicating that BLE and ANT events are pending.
     It is possible that either ANT or BLE events OR ANT & BLE events are pending.
     The application shall handle all the cases. 
*/
void SD_EVT_IRQHandler(void)
{
  /* Set Flag that ANT and BLE Events pending. */
  G_u32SystemFlags |= (_SYSTEM_PROTOCOL_EVENT); 
}


/*--------------------------------------------------------------------------------------------------------------------
Interrupt handler: GPIOTE_IRQHandler

Description:
Processes GPIOTE Events such as Pin and Port InterruptsB

Requires:
  - Enabled via sd_nvic_XXX

Promises:
  - Handles the GPIOTE events for the enabled pins. 
*/
void GPIOTE_IRQHandler(void)
{
  u8 u8ButtonIndex;

  // Disable further GPIOTE Interrupts. Button module will reenable it once button is released.
  NRF_GPIOTE->INTENCLR = (GPIOTE_INTENCLR_IN0_Msk | GPIOTE_INTENCLR_IN1_Msk | GPIOTE_INTENCLR_IN2_Msk);

  // Loop over 3 GPIOTE Channels
  for (u8 u8Channel = 0; u8Channel < 3; u8Channel++)
  {
    // Check if Interrupt occured on this channel.
    if (NRF_GPIOTE->EVENTS_IN[u8Channel] == 1)
    {
       u8ButtonIndex = ( (u8Channel * 2) + u8Channel) + ButtonGetActiveColumn();
       
       // Clear Channel Interrupt and Set Button Debounce Flags on buttons.
       // SW_ROWx + Active COLx corresponds to the Index of the button being pressed (0-8).
       NRF_GPIOTE->EVENTS_IN[u8Channel] = 0;   // Clear Channel Event.
       G_abButtonDebounceActive[u8ButtonIndex] = true;
       G_au32ButtonDebounceTimeStart[u8ButtonIndex] = G_u32SystemTime1ms;
       return;
    }
  }

} /* end GPIOTE_IRQHandler() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
