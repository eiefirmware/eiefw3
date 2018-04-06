/***********************************************************************************************************************
File: main.c                                                                

Description:
Container for the nRF51422 firmware.  
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SystemFlags = 0;                     /* Global system flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern fnCode_type ANTTT_SM;

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/
u32 Main_u32ErrorCode;
u8 Main_u8TestMessage[] = "9876 test message from ANT";

/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Main Program
Main has two sections:

1. Initialization which is run once on power-up or reset.  All drivers and applications are setup here without timing
contraints but must complete execution regardless of success or failure of starting the application. 

2. Main loop.  This is an event-driven system that sequentially executes tasks that require servicing in the event queue.

***********************************************************************************************************************/
void main(void)
{
  G_u32SystemFlags |= _SYSTEM_INITIALIZING;
  
  // Due to the switch that the application makes from the User mode system to SoftDevice
  // mode System, this module sets up the basic clocks and I/Os, followed by enabling the SD.
  // Once the SD is enabled, the application shall only use the SD provided system calls

  __disable_interrupt();

  /* Clock, GPIO and SoftDevice setup */  
  ClockSetup();
  GpioSetup();

  __enable_interrupt();
  
  // Enable the s310 SoftDevice Stack. If Failure, we shall not progress as 
  // successive code is dependent on SD success.
  if (!SocIntegrationInitialize())
  {
    while (1);
  }
  
  /* Low Level Initialization Modules */
  WatchDogSetup(); /* During development, set to not reset processor if timeout */
  SysTickSetup();
  InterruptsInitialize();
    
  /* Driver initialization */

  ANTIntegrationInitialize();
  BLEIntegrationInitialize();
  bleperipheralInitialize();
  
  /* Application initialization */
  AntttInitialize();
  
  /* Exit initialization */
  G_u32SystemFlags &= ~_SYSTEM_INITIALIZING;
  
  /* Main loop */  
  while(1)
  {
    SocIntegrationHandler();
    AntttRunActiveState();
   
    /* System sleep*/
    SystemSleep();
    
  } /* end while(1) main super loop */
  
} /* end main() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
