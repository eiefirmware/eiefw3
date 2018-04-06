/***********************************************************************************************************************
File: main.h

Description:
Header file for main.c.
*******************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

/***********************************************************************************************************************
* Firmware Version - Printed FIRMWARE_MAIN_REV.FIRMWARE_SUB_REV1 FIRMWARE_SUB_REV2
* See releasenotes.txt for firmware details.
***********************************************************************************************************************/
#define FIRMWARE_MAIN_REV               '0'
#define FIRMWARE_SUB_REV1               '0'
#define FIRMWARE_SUB_REV2               '1'


/***********************************************************************************************************************
* Constant Definitions
***********************************************************************************************************************/
/* G_u32SystemFlags */
#define _SYSTEM_HFCLK_NO_START          0x00000001        /* Set if the main oscilator does not start as expected */

#define _SYSTEM_PROTOCOL_EVENT          0x00010000        /* Set when at least one Soft Device event needs to be processed */

#define _SYSTEM_SLEEPING                0x40000000        /* Set into sleep mode to go back to sleep if woken before 1ms period */
#define _SYSTEM_INITIALIZING            0x80000000        /* Set when system is in initialization phase */


#define M3_MESSAGE_PERIOD               (u32)1000         /* Time in ms between polls to M3 processor */


/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/


#endif /* __MAIN_H */