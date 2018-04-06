/**********************************************************************************************************************
File: anttt.c                                                                

Description:
Implements TIC-TAC-TOE using data input from ANT or BLE.



**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAnttt"
***********************************************************************************************************************/
/* New variables */
u32 G_u32AntttFlags;                                     /* Global state flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */
extern volatile u32 G_u32BPEngenuicsFlags;             /* From bleperipheral_engenuics.c  */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Anttt_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Anttt_pfnStateMachine;              /* The application state machine function pointer */



/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: AntttInitialize

Description:
Initializes the State Machine and its variables.

Requires:

Promises:
*/
void AntttInitialize(void)
{
  Anttt_pfnStateMachine = AntttSM_Idle;
  
} /* end AntttInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function AntttRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void AntttRunActiveState(void)
{
  Anttt_pfnStateMachine();

} /* end AntttRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------------------------------------------*/
/* State Machine definitions                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
State: AntttSM_Idle
*/
static void AntttSM_Idle(void)
{
    
} 



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
