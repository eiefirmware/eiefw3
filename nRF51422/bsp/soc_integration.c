/*!********************************************************************************************************************
@file soc_integration.c                                                                
@brief This is a soc_integration .c file new source code
**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SocIntegrationFlags;                 /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SocInt_" and be declared as static.
***********************************************************************************************************************/
//static u32 SocInt_u32Timeout;                      /* Timeout counter used across states */



/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn SocIntegrationInitialize
@brief Initializes the SoftDevice to use the Protocol Stacks. 

Enables the SD_EVT_IRQ to allow forwarding of Protocol Interrupts to the
application.

Requires:
- Called once during initialization.

Promises:
- Returns TRUE if SoftDevice enabled successfully.
- Returns FALSE if SoftDevice enabling failed.

*/
bool SocIntegrationInitialize(void)
{
  u32 u32Result;
  
  u32Result = sd_softdevice_enable(NRF_CLOCK_LFCLKSRC_SYNTH_250_PPM, SocSoftdeviceAssertCallback);

  return (u32Result == NRF_SUCCESS);
  
} /* end SocIntegrationInitialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void SocIntegrationHandler(void)
@brief This is the global event checker for Protocol Events. 

This function is called continuously from main(). It checks if the 
_SYSTEM_PROTOCOL_EVENT is set. If so, it calls the dispatchers for the protocol event handlers.

Requires:
- SoftDevice is enabled
- BLE and ANT have been initialized
- Application is running

Promises:
- Proper dispatching of Protocol events to its handlers
- Clears the _SYSTEM_PROTOCOL_EVENT system flag

*/
void SocIntegrationHandler(void)
{
  // Check if pending event.
  if (G_u32SystemFlags & _SYSTEM_PROTOCOL_EVENT)
  {
    // Clear pending event and process protocol events.
    G_u32SystemFlags &= ~_SYSTEM_PROTOCOL_EVENT;
    ANTIntegrationHandler();
    BLEIntegrationHandler();
  }
}


/*!----------------------------------------------------------------------------------------------------------------------
@fn void SocSoftdeviceAssertCallback(uint32_t ulPC, uint16_t usLineNum, const uint8_t *pucFileName)
@brief Assertion handler from the Softdevice.

Requires:
- NONE

Promises:
- Halts the system.
- Provides the PC counter, Line Num and the FileName in the SoftDevice Code that caused the assertion.

*/
void SocSoftdeviceAssertCallback(uint32_t ulPC, uint16_t usLineNum, const uint8_t *pucFileName)
{
  while (1);
  
} /* end SocSoftdeviceAssertCallback() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
