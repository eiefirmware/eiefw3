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


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Interrupts_" and be declared as static.
***********************************************************************************************************************/


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
#define SD_PRESENT 0
  
#ifndef SD_PRESENT  

#else
  
  u32 u32Result = NRF_SUCCESS;
  
  // Must enable the SoftDevice Interrupt first.
  u32Result |= sd_nvic_SetPriority(SD_EVT_IRQn, NRF_APP_PRIORITY_LOW);
  u32Result |= sd_nvic_EnableIRQ(SD_EVT_IRQn);
  
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


} /* end GPIOTE_IRQHandler() */





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
