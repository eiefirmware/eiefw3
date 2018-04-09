/*!*********************************************************************************************************************
@file configuration.h      
@brief Main configuration header file for project.  

This file bridges many of the generic features of the 
firmware to the specific features of the design. The definitions should be updated
to match the target hardware.  
 ***********************************************************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/**********************************************************************************************************************
Runtime Switches
***********************************************************************************************************************/
//#define SOFTDEVICE_ENABLED  
//#define INTERRUPTS_ENABLED  

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Includes
**********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "system_nrf51.h"

/* Standard C and nRF SoC/SDK headers */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"


#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_error.h"
#include "soc_integration.h"
#include "ant_integration.h"
#include "ble_integration.h"
#include "bleperipheral.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_gatts.h"
#include "ble_types.h"
#include "ble_advdata.h"
#include "ble_srv_common.h"
#include "bleperipheral_engenuics.h"


/* nRF51422 implementation headers */
#include "eief1_nrf51-01.h"
#include "interrupts.h"
#include "main.h"
#include "typedefs.h"
#include "utilities.h"


/* Driver header files */
#include "leds_eief1_nrf51.h" 

/* Application header files */
#include "anttt.h"




#endif /* __CONFIG_H */

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

