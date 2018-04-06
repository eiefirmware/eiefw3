/**********************************************************************************************************************
File: bleperipheral.c                                                                

Description:
This is a bleperipheral .c file new source code
**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32BLEPeripheralFlags;                 /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SocInt_" and be declared as static.
***********************************************************************************************************************/
//static u32 bleperipheral_u32Timeout;                      /* Timeout counter used across states */
static ble_gap_adv_params_t             m_adv_params;                                /**< Parameters to be passed to the stack when starting advertising. */
static ble_gap_sec_params_t             m_sec_params;                                /**< Security requirements for this application. */
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;     /**< Handle of the current connection. */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------
Function: bleperipheralInitialize

Description:
Initializes the GAP and GATT layers services for the system. Starts the GAP Advertising.

Requires:
  - None

Promises:
  - Returns TRUE if all sub-systems are successfully activated.
  - Returns FALSE if a sub-system failed and was not activated successfully.
*/
bool bleperipheralInitialize(void)
{
  bool bResult = false;

  // Set up all the base services for the peripheral mode.
  bResult |= bleperipheral_gap_params_init();
  bResult |= bleperipheral_advertising_init();
  bResult |= bleperipheral_services_init();
  bleperipheral_sec_params_init();
  bResult |= bleperipheral_advertising_start();
  
  return bResult;
}


/*----------------------------------------------------------------------------------------------------------------------
Function: blePeripheralEventHandler(ble_evt*)

Description:
The Event handler which handles the various GATT and GAP events received from the SoftDevice once BLE Services have been activated.

Requires:
  - ble_evt_t*: Pointer to the single buffer holding the current ble_evt_t from the SoftDevice.

Promises:
  - Decodes the ble_evt_t and handles message accordingly.
  - Returns TRUE if ble_evt_t decoded and action taken.
  - Returns FALSE if ble_evt_t decoded and action failed.
*/
bool bleperipheralEventHandler(ble_evt_t* p_ble_evt)
{
    u32 u32ErrorCode = NRF_SUCCESS;
    
    switch (p_ble_evt->header.evt_id)
    {
      case BLE_GAP_EVT_CONNECTED:
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        BPEngenuicsOnConnect(p_ble_evt);
        break;

      case BLE_GAP_EVT_DISCONNECTED:
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        BPEngenuicsOnDisconnect();
        bleperipheral_advertising_start();
        break;

      case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        u32ErrorCode = sd_ble_gap_sec_params_reply(m_conn_handle,
                                               BLE_GAP_SEC_STATUS_SUCCESS,
                                               &m_sec_params);
        break;

      case BLE_GAP_EVT_TIMEOUT:
        break;

      case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        u32ErrorCode = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0);
        break;
          
      case BLE_GATTS_EVT_WRITE:
        BPEngenuicsOnWrite(p_ble_evt);
        break;

      default:
        // No implementation needed.
        break;
    }

    return (u32ErrorCode == NRF_SUCCESS);
}


/*----------------------------------------------------------------------------------------------------------------------
Function: bool bleperipheralIsConnectedandEnabled(void)

Description:
The Event handler which handles the various GATT and GAP events received from the SoftDevice once BLE Services have been activated.

Requires:
  - ble_evt_t*: Pointer to the single buffer holding the current ble_evt_t from the SoftDevice.

Promises:
  - Decodes the ble_evt_t and handles message accordingly.
  - Returns TRUE if ble_evt_t decoded and action taken.
  - Returns FALSE if ble_evt_t decoded and action failed.
*/
bool bleperipheralIsConnectedandEnabled(void)
{
   // Check if connected and service enabled on BLE.
   return (m_conn_handle != BLE_CONN_HANDLE_INVALID);
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------
Function: bleperipheral_gap_params_init

Description:
Initializes the GAP parameters for the device. Sets the device name, sets the device 
appearance type and defines the peripheral requested rates.

Requires:
  - None

Promises:
  - Returns TRUE if GAP params are successfully completed.
  - Returns FALSE if GAP params are not set successfully.
*/
static bool bleperipheral_gap_params_init(void)
{
    u32 u32ErrorCode = NRF_SUCCESS;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    u32ErrorCode |= sd_ble_gap_device_name_set(&sec_mode, DEVICE_NAME, strlen(DEVICE_NAME));
    u32ErrorCode |= sd_ble_gap_appearance_set(BLEPERIPHERAL_DEVICE_APPEARANCE);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    u32ErrorCode |= sd_ble_gap_ppcp_set(&gap_conn_params);
    
    return (u32ErrorCode == NRF_SUCCESS);
}

/*----------------------------------------------------------------------------------------------------------------------
Function: bleperipheral_advertising_init

Description:
Initializes the advertising parameters for the device.

Requires:
  - None

Promises:
  - Returns TRUE if advertising params are successfully completed.
  - Returns FALSE if advertising params are not set successfully.
*/
static bool bleperipheral_advertising_init(void)
{
    u32 u32ErrorCode = NRF_SUCCESS;
    ble_advdata_t advdata;
    u8 u8Flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    ble_uuid_t adv_uuids[] =
    {
      {BLE_UUID_HEART_RATE_SERVICE,         BLE_UUID_TYPE_BLE},
      {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
    };

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags.size              = sizeof(u8Flags);
    advdata.flags.p_data            = &u8Flags;
    advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = adv_uuids;

    u32ErrorCode |= ble_advdata_set(&advdata, NULL);

    // Initialise advertising parameters (used when starting advertising)
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL;
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
    
    return u32ErrorCode == NRF_SUCCESS;
    
}


/*----------------------------------------------------------------------------------------------------------------------
Function: bleperipheral_advertising_init

Description:
Initializes any BLE Peripheral Services.

Requires:
  - None

Promises:
  - Returns TRUE if service initialization successful.
  - Returns FALSE if service initialization fails.
*/
static bool bleperipheral_services_init(void)
{
  // Initialize the BPEngenuics service.
  if ( !BPEngenuicsInitialize() )
  {
    return false;
  }
  
  return true;
  
}


/*----------------------------------------------------------------------------------------------------------------------
Function: bleperipheral_advertising_start

Description:
Start Advertising

Requires:
  - None

Promises:
  - Returns TRUE if advertising start successful.
  - Returns FALSE if advertising start fails.
*/
static bool bleperipheral_advertising_start(void)
{
    u32 u32ErrorCode;

    u32ErrorCode = sd_ble_gap_adv_start(&m_adv_params);
    return (u32ErrorCode == NRF_SUCCESS);
}


/*----------------------------------------------------------------------------------------------------------------------
Function: bleperipheral_sec_params_init

Description:
Initializes security parameteres.

Requires:
  - None

Promises:
  - None
*/
static void bleperipheral_sec_params_init(void)
{
    m_sec_params.timeout      = SEC_PARAM_TIMEOUT;
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}






/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
