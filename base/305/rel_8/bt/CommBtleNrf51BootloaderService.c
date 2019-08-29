//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of Nordic specific layer for the Bluetooth LE upload service
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMMBTLENRF51BOOTLOADERSERVICE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMBTLENRF51BOOTLOADERSERVICE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               COMMBTLENRF51BOOTLOADERSERVICE_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// COM
#include "CommBtleNrf51BootloaderService.h"
#include "CommBtleBootloaderService.h"

#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DFU_BASE_UUID                        {{0x62, 0x53, 0x56, 0x1B, 0x10, 0x26, 0xC4, 0xAE, 0x10, 0x48, 0x43, 0x2E, 0x00, 0x00, 0x84, 0x25}} /**< Used vendor specific UUID. */
#define BLE_DFU_MAX_DATA_LEN                 (GATT_MTU_SIZE_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Flamco Pa Service module. */

#define BLE_DFU_SERVICE_UUID                 0x0001
#define BLE_DFU_STATUS_CHAR_UUID             0x0002
#define BLE_DFU_CTRL_POINT_CHAR_UUID         0x0003
#define BLE_DFU_DATA_CHAR_UUID               0x0004
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U16                         conn_handle;                           /**< Handle of the current connection (as provided by the SoftDevice). This will be BLE_CONN_HANDLE_INVALID when not in a connection. */
    U16                         service_handle;                        /**< Handle of DFU Service (as provided by the SoftDevice). */
    U8                          uuid_type;                             /**< UUID type assigned for DFU Service by the SoftDevice. */
    ble_gatts_char_handles_t    dfu_status_handle;                     /**< Handles related to the DFU Status characteristic. */
    ble_gatts_char_handles_t    dfu_ctrl_point_handle;                 /**< Handles related to the DFU Control Point characteristic. */
    ble_gatts_char_handles_t    dfu_upload_data_handle;                /**< Handles related to the DFU Upload Data characteristic. */
    BOOL                        is_notification_enabled;
}
ble_dfu_t;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void OnConnectEvent(ble_evt_t* ble_evt_ptr);
static void OnDisconnectEvent(ble_evt_t* ble_evt_ptr);
static void OnWriteEvent(ble_evt_t* ble_evt_ptr);
static U32  AddStatusCharacteristic(void);
static U32  AddCtrlPointCharacteristic(void);
static U32  AddUploadDataCharacteristic(void);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_DfuNotify(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static ble_dfu_t                                dfu_struct;

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static CHAR                                     char_buffer[100];
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void OnConnectEvent(ble_evt_t* ble_evt_ptr)
{
    dfu_struct.conn_handle = ble_evt_ptr->evt.gap_evt.conn_handle;
}
//------------------------------------------------------------------------------------------------//
static void OnDisconnectEvent(ble_evt_t* ble_evt_ptr)
{
    UNUSED_PARAMETER(ble_evt_ptr);
    dfu_struct.conn_handle = BLE_CONN_HANDLE_INVALID;
}
//------------------------------------------------------------------------------------------------//
static void OnWriteEvent(ble_evt_t* ble_evt_ptr)
{
    ble_gatts_evt_write_t* write_event_ptr = &ble_evt_ptr->evt.gatts_evt.params.write;
    
    if((write_event_ptr->handle == dfu_struct.dfu_status_handle.cccd_handle) && (write_event_ptr->len == 2))
    {
        // Make it possible to send data to the Central
        LOG_TRM("[DFU] Status cccd written %d", PU8A(write_event_ptr->data, write_event_ptr->len));
        dfu_struct.is_notification_enabled = (BOOL)ble_srv_is_notification_enabled(write_event_ptr->data);
    }
    else if(write_event_ptr->handle == dfu_struct.dfu_ctrl_point_handle.value_handle)
    {
        LOG_DBG("[DFU] Ctrl handle written - %d", PU8A(write_event_ptr->data, write_event_ptr->len));
        CommBtleBootloaderService_RegisterCtrlPointData(write_event_ptr->data, write_event_ptr->len);
    }
    else if(write_event_ptr->handle == dfu_struct.dfu_upload_data_handle.value_handle)
    {
        LOG_DBG("[DFU] Upload handle written - %d", PU8A(write_event_ptr->data, write_event_ptr->len));
        CommBtleBootloaderService_RegisterUploadData(write_event_ptr->data, write_event_ptr->len);
    }
}
//------------------------------------------------------------------------------------------------//
static U32 AddStatusCharacteristic(void)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    
    memset(&cccd_md, 0, sizeof(cccd_md));
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;
    
    ble_uuid.type = dfu_struct.uuid_type;
    ble_uuid.uuid = BLE_DFU_STATUS_CHAR_UUID;
    
    memset(&attr_md, 0, sizeof(attr_md));
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_DFU_MAX_DATA_LEN;
    
    return sd_ble_gatts_characteristic_add(dfu_struct.service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &dfu_struct.dfu_status_handle);
}
//------------------------------------------------------------------------------------------------//
static U32 AddCtrlPointCharacteristic(void)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;
    
    ble_uuid.type = dfu_struct.uuid_type;
    ble_uuid.uuid = BLE_DFU_CTRL_POINT_CHAR_UUID;
    
    memset(&attr_md, 0, sizeof(attr_md));
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_DFU_MAX_DATA_LEN;
    
    return sd_ble_gatts_characteristic_add(dfu_struct.service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &dfu_struct.dfu_ctrl_point_handle);
}
//------------------------------------------------------------------------------------------------//
static U32 AddUploadDataCharacteristic(void)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;
    
    ble_uuid.type = dfu_struct.uuid_type;
    ble_uuid.uuid = BLE_DFU_DATA_CHAR_UUID;
    
    memset(&attr_md, 0, sizeof(attr_md));
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_DFU_MAX_DATA_LEN;
    
    return sd_ble_gatts_characteristic_add(dfu_struct.service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &dfu_struct.dfu_upload_data_handle);
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_DfuNotify(void)
{
    MEMSET(char_buffer, 0, 100);
    
    CoreTerm_GetAllArgumentAsString(char_buffer);
    CommBtleNrf51BootloaderService_NotifyUploadCtrl((U8*)char_buffer, CoreString_GetLength(char_buffer));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
U32  CommBtleNrf51BootloaderService_Init(void)
{
    ble_uuid128_t dfu_base_uuid = DFU_BASE_UUID;
    ble_uuid_t    ble_uuid;
    U32           err_code;
    
    // Initialize the service structure.
    dfu_struct.conn_handle             = BLE_CONN_HANDLE_INVALID;
    dfu_struct.is_notification_enabled = FALSE;
    
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&dfu_base_uuid, &dfu_struct.uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    ble_uuid.type = dfu_struct.uuid_type;
    ble_uuid.uuid = BLE_DFU_SERVICE_UUID;
    
    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &dfu_struct.service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add the Status Characteristic.
    err_code = AddStatusCharacteristic();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add the Control Point Characteristic.
    err_code = AddCtrlPointCharacteristic();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add the Upload Data Characteristic.
    err_code = AddUploadDataCharacteristic();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
#if (TERM_LEVEL > TERM_LEVEL_NONE)
    CoreTerm_RegisterCommand("DfuNotify", "Send a string on the notify characteristic", 255, Command_DfuNotify, TRUE);
#endif
    
    return NRF_SUCCESS;
}
//------------------------------------------------------------------------------------------------//
void CommBtleNrf51BootloaderService_OnBleEvent(ble_evt_t* ble_evt_ptr)
{
    if(ble_evt_ptr == NULL)
    {
        return;
    }
    
    switch(ble_evt_ptr->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        OnConnectEvent(ble_evt_ptr);
        break;
        
    case BLE_GAP_EVT_DISCONNECTED:
        OnDisconnectEvent(ble_evt_ptr);
        break;
        
    case BLE_GATTS_EVT_WRITE:
        OnWriteEvent(ble_evt_ptr);
        break;
        
    default:
        break;
    }
}
//------------------------------------------------------------------------------------------------//
BOOL CommBtleNrf51BootloaderService_NotifyUploadCtrl(U8* data_ptr, U8 length)
{
    ble_gatts_hvx_params_t hvx_params;
    
    if((dfu_struct.conn_handle == BLE_CONN_HANDLE_INVALID) || (dfu_struct.is_notification_enabled == FALSE))
    {
        return FALSE;
    }
    
    if(length > BLE_DFU_MAX_DATA_LEN)
    {
        return FALSE;
    }
    
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = dfu_struct.dfu_status_handle.value_handle;
    hvx_params.p_data = data_ptr;
    hvx_params.p_len  = (U16*)&length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    
    LOG_DBG("[DFU] Notify %02h", PU8A(data_ptr, length));
    return (BOOL)(sd_ble_gatts_hvx(dfu_struct.conn_handle, &hvx_params) == NRF_SUCCESS);
}
//================================================================================================//