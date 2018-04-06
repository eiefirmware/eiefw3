/**********************************************************************************************************************
File: bleperipheral_engenuics.c                                                                

Description:
BLE Peripheral Service module for the Engenuics BLE Profile.
**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32BPEngenuicsFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "BPEngenuics_" and be declared as static.
***********************************************************************************************************************/
//static u32 BPEngenuics_u32Timeout;           /* Timeout counter used across states */
static u8  BPEngenuics_u8UuidType;                       /* UUID Type for BLEEngenuics service */ 
static u16 BPEngenuics_u16ServiceHandle;                 /* Handle for the GATTS Service */
static ble_gatts_char_handles_t BPEngenuics_eTxHandles;  /* TX Characteristic Handles */
static ble_gatts_char_handles_t BPEngenuics_eRxHandles;  /* RX Characteristic Handles */          
static u16 BPEngenuics_u16ConnHandle;                    /* Connection handle indicating if the device is GAP Connected to Client*/
static bool BPEngenuics_bNotifcationEnabled;             /* Flag to indicate if Notifications have been enabled by the Client */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
bool BPEngenuicsSendData(u8* buffer, u8 size)
{
  ble_gatts_hvx_params_t hvx;   // Indication / Notification structure.

  if (size > BPENGENUICS_MAX_CHAR_LEN)
    return false;
  
  // Check that the module is connected AND notifications are enabled.
  if ((BPEngenuics_u16ConnHandle != BLE_CONN_HANDLE_INVALID) && (BPEngenuics_bNotifcationEnabled))
  {
    memset(&hvx, 0, sizeof(hvx));
    hvx.handle = BPEngenuics_eTxHandles.value_handle;
    hvx.p_data = buffer;
    hvx.p_len = (uint16_t*)&size;
    hvx.type = BLE_GATT_HVX_NOTIFICATION;
    
    return (sd_ble_gatts_hvx(BPEngenuics_u16ConnHandle, &hvx) == NRF_SUCCESS);
  }
  else
  {
    return false;
  }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------
Function: BPEngenuicsInitialize

Description:
Initializes the State Machine and its variables.

Requires:
   - Called after the stack has been initialized.
   - Called from the BLEIntegrationHandlerInitialize

Promises:
  - Characteristics are registered to the Services.
  - Service is registered with the stack for operations.
*/
bool BPEngenuicsInitialize(void)
{
  u32 error;

  // Initialize.
  BPEngenuics_u16ConnHandle = BLE_CONN_HANDLE_INVALID;
  BPEngenuics_bNotifcationEnabled = false;

  // Add the services and characteristics.
  error = BPEngenuicsAddService();
  error |= BPEngenuicsAddRxCharacteristic();
  error |= BPEngenuicsAddTxCharacteristic();

  return (error == NRF_SUCCESS);
} /* end BPEngenuicsInitialize() */

/*--------------------------------------------------------------------------------------------------------------------
Function: BPEngenuicsOnConnect

Description:
Initializes the State Machine and its variables.

Requires:
  - BPEngenuicsInitialize has been called.
  - peEvent_ points to an BLE event

Promises:
  - On connection sets _BPENGENUICS_CONNECTED to notify the module that it is in the connected state 
*/
void BPEngenuicsOnConnect(ble_evt_t* peEvent_)
{
  // Update the BPEngenuics_u16ConnHandle. Module knows that it is connected to a client.
  BPEngenuics_u16ConnHandle = peEvent_->evt.gap_evt.conn_handle;
  G_u32BPEngenuicsFlags |= _BPENGENUICS_CONNECTED;
}


/*--------------------------------------------------------------------------------------------------------------------
Function: BPEngenuicsOnDisconnect

Description:
Initializes the State Machine and its variables.

Requires:
   - BPEngenuicsInitialize has been called

Promises:
  - Notifies the module that it is in the disconnected state.
*/
void BPEngenuicsOnDisconnect(void)
{
  // Invalidate the BPEngenuics_u16ConnHandle. Module knows that it is in a disconnected state
  BPEngenuics_u16ConnHandle = BLE_CONN_HANDLE_INVALID;
  G_u32BPEngenuicsFlags &= ~_BPENGENUICS_CONNECTED;
}


/*--------------------------------------------------------------------------------------------------------------------
Function: BPEngenuicsOnWrite

Description:
Handles the Service enabling/disabling. Handles the Value Char writes from Client.

Requires:
   - Called after the module has been initialized.
   - evt is the ble_evt_t* containing the appropriate GATTS Write information
   
Promises:
  - Handles Enabling/Disabling on the BPEngenucis TX Value Characteristic.
  - Handles Rx Messages sent from the client on the RX Value Characteristic.
*/
void BPEngenuicsOnWrite(ble_evt_t* peEvent_)
{
   // Create our ble_gatts_evt_write_t object.
    ble_gatts_evt_write_t* peEventWrite = &peEvent_->evt.gatts_evt.params.write;
    
    // Check if it is the TX Handle CCCD write event and len is 2.
    if ((peEventWrite->handle == BPEngenuics_eTxHandles.cccd_handle) && (peEventWrite->len == 2))
    {
      // Check if service needs to be enabled/disabled.
      if (ble_srv_is_notification_enabled(peEventWrite->data))
      {
        BPEngenuics_bNotifcationEnabled = true;
        G_u32BPEngenuicsFlags |= _BPENGENUICS_SERVICE_ENABLED;
      }
      else
      {
        BPEngenuics_bNotifcationEnabled = false;
        G_u32BPEngenuicsFlags &= ~_BPENGENUICS_SERVICE_ENABLED;
      }
    }
    else if (peEventWrite->handle == BPEngenuics_eRxHandles.value_handle)    
    {
      CallbackBleperipheralEngenuicsDataRx(peEventWrite->data, peEventWrite->len);
    }    
}


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: BPEngenuicsAddService

Description:
Registers the service

Requires:
   - Called after the module has been intialized.
   
Promises:
  - Adds the BPEngenuics Rx Service.
  - Registers a properietary 128bit Service UUID base.
  - Followed by 16bit Service identifier. 
*/
static u32 BPEngenuicsAddService(void)
{
   u32 u32Error;
   ble_uuid_t ble_uuid;
   ble_uuid128_t ble_base_uuid = BPENGENUICS_BASE_UUID;

   // Add Service UUID with base .
   u32Error = sd_ble_uuid_vs_add(&ble_base_uuid, &BPEngenuics_u8UuidType);

   // Add the 16bit unique value to base.
   ble_uuid.type = BPEngenuics_u8UuidType;
   ble_uuid.uuid = BPENGENUICS_SERVICE_UUID;
   u32Error = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &BPEngenuics_u16ServiceHandle);

   return u32Error;
}


/*--------------------------------------------------------------------------------------------------------------------
Function: BPEngenuicsAddRxcharacteristic

Description:
Adds the Rx Characteristic to the BLE Service.

Requires:
   - Called during module initialization
   
Promises:
  - Adds the BPEngenuics Rx Characteristic.
*/
static u32 BPEngenuicsAddRxCharacteristic(void)
{
    ble_gatts_char_md_t rxchar_metadata;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

   // Metadata for the Rx Characteristic.
    memset(&rxchar_metadata, 0, sizeof(rxchar_metadata));
    rxchar_metadata.char_props.write = 1;
    rxchar_metadata.char_props.write_wo_resp = 1;
  
    // Load the RX CHAR UUID.
    ble_uuid.type = BPEngenuics_u8UuidType;
    ble_uuid.uuid = BPENGENUICS_RX_CHAR_UUID;

    // Metadata for the Rx Attribute.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.vlen = 1;

    // Setup of the Rx Attribute.
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = BPENGENUICS_MAX_CHAR_LEN;
    attr_char_value.max_len   = BPENGENUICS_MAX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(BPEngenuics_u16ServiceHandle, &rxchar_metadata, &attr_char_value, &BPEngenuics_eRxHandles);
}

/*--------------------------------------------------------------------------------------------------------------------
Function: BPEngenuicsAddTxcharacteristic

Description:
Adds the Tx Characteristic to the BLE Service.

Requires:
   - Called during module initialization.
   
Promises:
  - Adds the BPEngenuics Tx Characteristic.
*/
static u32 BPEngenuicsAddTxCharacteristic(void)
{
    ble_gatts_char_md_t txchar_metadata;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // ClientConfigurationDescriptor Metadata.
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    // Location of CCCD characteristic is on user stack.
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    // Metadata for the Tx Characteristic.
    memset(&txchar_metadata, 0, sizeof(txchar_metadata));
    txchar_metadata.char_props.notify = 1;
    txchar_metadata.p_cccd_md = &cccd_md;

    // Load the TX CHAR UUID.
    ble_uuid.type = BPEngenuics_u8UuidType;
    ble_uuid.uuid = BPENGENUICS_TX_CHAR_UUID;

    // Metadata for the Tx Attribute.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.vlen = 1;

    // Setup of the Tx Attribute.
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = BPENGENUICS_MAX_CHAR_LEN;
    attr_char_value.max_len   = BPENGENUICS_MAX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(BPEngenuics_u16ServiceHandle, &txchar_metadata, &attr_char_value, &BPEngenuics_eTxHandles);
}


/*----------------------------------------------------------------------------------------------------------------------
Function: CallbackBleperipheralEngenuicsDataRx

Description:
Callback to handle the message received on the RX Characteristic of the bleperipheral_engenuics profile.

Requires:
  - data: pointer to array containing data.
  - len: Length of array

Promises:
  - Application handling of the message.
*/
static void CallbackBleperipheralEngenuicsDataRx(u8* u8Data_, u8 u8Length_)
{
  // Forward handling to ANTTT module.
  
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
