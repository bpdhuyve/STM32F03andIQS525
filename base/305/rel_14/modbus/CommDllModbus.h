//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the Data Link Layer of the MODBUS protocol.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MODBUS__COMMDLLMODBUS_H
#define MODBUS__COMMDLLMODBUS_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
// DRV
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_MODBUS_CHANNEL                          0
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Modbus DLL channel definition
typedef U8          MODBUS_CHANNEL;

// @brief   Enum with definitions of the errors which can occur in this layer.
// @remark  !!! Append only for compatibility reasons !!!
typedef enum
{
    DLLMODBUS_ERR_NO_ERROR                              = 0,
    DLLMODBUS_ERR_REPLY_TIMEOUT_OCCURRED                = 1,
    DLLMODBUS_ERR_FRAME_LENGTH_TOO_LONG                 = 2,
    DLLMODBUS_ERR_FRAME_RX_TOO_LONG                     = 3,
    DLLMODBUS_ERR_RX_WHILE_TX_BUSY                      = 4,
    DLLMODBUS_ERR_TX_WHILE_NOT_IN_TX_MODE               = 5,
    DLLMODBUS_ERR_RX_WHILE_NOT_IN_RX_MODE               = 6,
    DLLMODBUS_ERR_FRAME_RX_LOST                         = 7,
    DLLMODBUS_ERR_INTERFRAME_TIMEOUT_REPLY_NOT_SENT     = 8,
    DLLMODBUS_ERR_INFRAME_SILENT_INTERVALS              = 9,
    DLLMODBUS_ERR_FRAME_RX_TOO_SHORT                    = 10,
    DLLMODBUS_ERR_FRAME_RX_CRC_ERROR_1                  = 11,
    DLLMODBUS_ERR_FRAME_RX_CRC_ERROR_2                  = 12,
    DLLMODBUS_ERR_NO_FRAME_HANDLER                      = 13,
    DLLMODBUS_ERR_UNKNOWN_STATE                         = 14,
    DLLMODBUS_ERR_FRAME_TX_TOO_LONG                     = 15,
    DLLMODBUS_ERR_FRAME_TX_LOST                         = 16,
}
DLLMODBUS_ERROR;

// @brief   Prototype of the frame handling function
typedef void (*DLLMODBUS_RECV_FRAME_HOOK)(MODBUS_CHANNEL modbus_channel, U8* frame_ptr, U8 length);

// @brief   Prototype of error handling hook
typedef void (*DLLMODBUS_ERROR_HANDLING_HOOK)(MODBUS_CHANNEL modbus_channel, DLLMODBUS_ERROR error);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the Modbus DataLink Layer
void CommDllModbus_Init(void);

// @brief   Function to be called in the background loop.
//          Received frames will be handled from this context
// @remark  if AUTO_REGISTER_BACKGROUND_HANLDER is set to 1, this function will be registered automatically to the core
//          otherwise, the application must ensure correct calling.
void CommDllModbus_Handler(void);

// @brief   Function to register a Modbus channel
// @param   channel_hndl  Handler to the SCI peripheral
// @param   sci_config    Configuration settings for the SCI peripheral
// @return  Returns the Modbus channel. If no channel could be provided, it will return INVALID_MODBUS_CHANNEL
MODBUS_CHANNEL CommDllModbus_RegisterChannel(SCI_CHANNEL_HNDL channel_hndl, SCI_CONFIG_STRUCT* sci_config);

// @brief   Function to register the function to be called when a message is received on the channel
// @return  Returns if registration succeeded
BOOL CommDllModbus_RegisterFrameHook(MODBUS_CHANNEL modbus_channel, DLLMODBUS_RECV_FRAME_HOOK recv_frame_hook);

// @brief   Function to register the function to be called when an error occurs (optional)
// @return  Returns if registration succeeded
BOOL CommDllModbus_RegisterErrorHandlingHook(MODBUS_CHANNEL modbus_channel, DLLMODBUS_ERROR_HANDLING_HOOK error_handling_hook);

// @brief   Function to register the function to be called to give feedback when a frame is transmitted on the modbus channel(optional)
// @return  Returns if registration succeeded
BOOL CommDllModbus_RegisterTxFeedbackHook(MODBUS_CHANNEL modbus_channel, EVENT_CALLBACK txFeedback_hook);

// @brief   Function to register the function to be called to give feedback when a frame is received on the modbus channel (optional)
// @return  Returns if registration succeeded
BOOL CommDllModbus_RegisterRxFeedbackHook(MODBUS_CHANNEL modbus_channel, EVENT_CALLBACK rxFeedback_hook);

// @brief   Function to send a frame on a modbus channel
// @return  Returns if sending succeeded
BOOL CommDllModbus_SendFrame(MODBUS_CHANNEL modbus_channel, U8* frame_ptr, U8 length);

// @brief   Function to update the configuration of the underlying SCI channel of the MODBUS channel
// @return  Returns if update succeeded
BOOL CommDllModbus_UpdateSCIChannelConfig(MODBUS_CHANNEL modbus_channel, SCI_CONFIG_STRUCT* sci_config);
//================================================================================================//

#endif /* MODBUS__COMMDLLMODBUS_H */

