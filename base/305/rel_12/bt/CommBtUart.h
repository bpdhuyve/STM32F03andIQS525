//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to communicate with the nordic bt uart chip, the chip must be running software 14016
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef COMM_BT_UARTBT_H
#define COMM_BT_UARTBT_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum    //enum values have a reason -> mapt to bt term interface
{
    STATE_STANDBY                      = 0, //no device verbonden
    STATE_ADVERTISING_TO_ALL           = 1, //iedereen mag proberen te verbinden
    STATE_ADVERTISING_TO_WHITELIST     = 2, //enkel whitelisted devices mogen proberen te verbinden, the bt state nonconectable is also represented in this state, non conectable means device is put in adv_whitelist but there are no devices in the whitelist
    STATE_CONNECTED_NO_ENCRYPTION,          //device moet keynog ingeven
    STATE_CONNECTED_ENCRYPTION,             //device verbonden
    STATE_UNKOWN,
}
DEVICE_STATE;

// @brief    hook Prototypes
typedef void (*EVENT_PASSKEY_RECEIVED)(U32);
typedef void (*EVENT_SWVERSION_RECEIVED)(STRING);
typedef void (*EVENT_CHIPID_RECEIVED)(STRING);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/// @brief inits the uart used for the communication with the nordic chip
void CommBtUart_Init(SCI_CHANNEL_HNDL uart_hndl);

/// @brief background handler. This function will call the different events when needed
void CommBtUart_Handler(void);

//------------------------------------------------------------------------------------------------//
/// @brief sends a out a string wireless over the nordic uart protocol to the master connected to the bt chip
BOOL CommBtUart_SendData(STRING data);

/// @brief function to register a event function to be called on reception of some uart data from the smartphone
void CommBtUart_RegisterEvent_DataReceived(EVENT_CALLBACK event);

/// @brief call this function if you want to fetch the last received data
STRING CommBtUart_GetReceivedData(void);

//------------------------------------------------------------------------------------------------//
/// @brief change the devicestate of the bt, used to allow a connection to be set up
BOOL CommBtUart_SetDeviceState(DEVICE_STATE state);

/// @brief get the latest known device state
DEVICE_STATE CommBtUart_GetDeviceState(void);

/// @brief async request the devicestate, the answer can be fetched with CommBtUart_GetDeviceState whenever it is received
void CommBtUart_UpdateDeviceState(void);

/// @brief register event if the devicestate changes
void CommBtUart_RegisterEvent_DeviceStateChanged(EVENT_CALLBACK event);

//------------------------------------------------------------------------------------------------//
/// @brief change the devicename that the smartphone sees when scanning for bt devices
void CommBtUart_SetDeviceName(STRING name);

//------------------------------------------------------------------------------------------------//
/// @brief async request the passkey that the smartphone needs to connect to the bt, the answer will be given trough the EVENT_PASSKEY_RECEIVED event
/// @remark the bt will signal the passkey event directly if a device wants to connect so this function is not really needed
void CommBtUart_RequestDevicePassKey(void);

/// @brief register event to notify if a generated passkey is received
void CommBtUart_RegisterEvent_PassKeyReceived(EVENT_PASSKEY_RECEIVED event);

//------------------------------------------------------------------------------------------------//
/// @brief async request the sw version, the answer will be given trough the EVENT_SWVERSION_RECEIVED event
void CommBtUart_RequestSwVersion(void);

/// @brief register event to notify if the sw version is received from the bt chip
void CommBtUart_RegisterEvent_SwVersionReceived(EVENT_SWVERSION_RECEIVED event);

//------------------------------------------------------------------------------------------------//
/// @brief async request the unique processor id, the answer will be given trough the EVENT_CHIPID_RECEIVED event
void CommBtUart_RequestChipId(void);

/// @brief register event to notify if the sw version is received from the bt chip
void CommBtUart_RegisterEvent_ChipIdReceived(EVENT_CHIPID_RECEIVED event);

/// @brief none
BOOL CommBtUart_SetDeviceMITM(BOOL mitm_on);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// C L E A R / U N D E F    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* COMM_BT_UARTBT_H */

