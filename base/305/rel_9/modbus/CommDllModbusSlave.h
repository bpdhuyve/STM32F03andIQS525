//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the Data Link Layer of the MODBUS protocol implemented as SLAVE
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MODBUS__COMMDLLMODBUSSLAVE_H
#define MODBUS__COMMDLLMODBUSSLAVE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//DRIVER lib include section
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Enum with defintions of the errors which can occur in this layer.
// @remark  !!! Append only for compatibility reasons !!!
typedef enum
{
    DLLMODBUSSLAVE_ERR_NO_ERROR = 0,
    DLLMODBUSSLAVE_ERR_REPLY_TIMEOUT_OCCURRED = 1,
    DLLMODBUSSLAVE_ERR_FRAME_LENGTH_TOO_LONG = 2,
    DLLMODBUSSLAVE_ERR_FRAME_RX_TOO_LONG = 3,
    DLLMODBUSSLAVE_ERR_RX_WHILE_TX_BUSY = 4,
    DLLMODBUSSLAVE_ERR_TX_WHILE_NOT_IN_TX_MODE = 5,
    DLLMODBUSSLAVE_ERR_RX_WHILE_NOT_IN_RX_MODE = 6,
    DLLMODBUSSLAVE_ERR_FRAME_RX_LOST = 7,
    DLLMODBUSSLAVE_ERR_INTERFRAME_TIMEOUT_REPLY_NOT_SENT = 8,
    DLLMODBUSSLAVE_ERR_INFRAME_SILENT_INTERVALS = 9,
    DLLMODBUSSLAVE_ERR_FRAME_RX_TOO_SHORT = 10,
    DLLMODBUSSLAVE_ERR_FRAME_RX_CRC_ERROR_1 = 11,
    DLLMODBUSSLAVE_ERR_FRAME_RX_CRC_ERROR_2 = 12,
    DLLMODBUSSLAVE_ERR_NO_FRAME_HANDLER = 13,
    DLLMODBUSSLAVE_ERR_UNKNOWN_STATE = 14,
    DLLMODBUSSLAVE_ERR_FRAME_TX_TOO_LONG = 15,
    DLLMODBUSSLAVE_ERR_FRAME_TX_LOST = 16
}
DLLMODBUSSLAVE_ERROR;

// @brief   Prototype of the frame handling function
typedef void (*DLLMODBUSSLAVE_RECV_FRAME_HOOK)(U8* frame_ptr, U8 length);

// @brief   Prototype of error handling hook
typedef void (*DLLMODBUSSLAVE_ERROR_HANDLING_HOOK)(DLLMODBUSSLAVE_ERROR error);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the DataLink Layer entity
void CommDllModbusSlave_Init(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED sci_speed);

// @remark none
void CommDllModbusSlave_RegisterFrameHook(DLLMODBUSSLAVE_RECV_FRAME_HOOK frame_hook);

// @remark none
void CommDllModbusSlave_RegisterTimeoutHook(EVENT_CALLBACK timeout_hook);

// @remark none
void CommDllModbusSlave_RegisterTxCompleteHook(EVENT_CALLBACK tx_complete_hook);

// @remark none
void CommDllModbusSlave_RegisterErrorHandlingHook(DLLMODBUSSLAVE_ERROR_HANDLING_HOOK error_handeling_hook);

// @brief   Function to be called in the background loop
// This function will be called in background. It will call be executed when a complete new message is received.
// The frame handling function will be called and the message will be interpreted.
void CommDllModbusSlave_Handler(void);

// @brief   Function to send a frame.
void CommDllModbusSlave_SendFrame(U8* ptr_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// C L E A R / U N D E F    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* MODBUS__COMMDLLMODBUSSLAVE_H */
