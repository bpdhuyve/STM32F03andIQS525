//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Network layer for MODBUS Master
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MODBUS__COMMNLMODBUSMASTER_H
#define MODBUS__COMMNLMODBUSMASTER_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "CommDllModbus.h"
//================================================================================================//


//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommNlModbusMaster_Init(void);
void CommNlModbusMaster_RegisterModbusChannel(MODBUS_CHANNEL modbus_channel);

// @brief   Function to register a timeout handler
// @param   modbus_channel  The MODBUS channel on which the timeout handler is registered
// @param   timeout_hook    The actual handler function
void CommNlModbusMaster_RegisterTimeoutHook(MODBUS_CHANNEL modbus_channel, EVENT_CALLBACK timeout_hook);

// @brief   Function to register a receive handler
void CommNlModbusMaster_RegisterReceiveHook(MODBUS_CHANNEL modbus_channel, EVENT_VPTR_CALLBACK receive_hook);

BOOL CommNlModbusMaster_SendFrame(MODBUS_CHANNEL modbus_channel, U8* frame_ptr, U8 length);
//================================================================================================//



#endif /* MODBUS__COMMNLMODBUSMASTER_H */