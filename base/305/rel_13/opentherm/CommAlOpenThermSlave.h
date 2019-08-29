//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Slave Application Link Layer of the OpenTherm communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef OPENTHERM__COMMALOPENTHERMSLAVE_H
#define OPENTHERM__COMMALOPENTHERMSLAVE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "opentherm/CommDllOpenTherm.h"
#include "opentherm/CommAlOpenThermCommon.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Prototype of data read request function
typedef DATA_READ_RESPONSE (*OPENTHERMSLAVE_DATA_READ_REQUEST_HOOK)(OT_DATA_ID data_id, OT_DATA_VALUE* data_value_ptr);

// @brief   Prototype of data write request function
typedef DATA_WRITE_RESPONSE (*OPENTHERMSLAVE_DATA_WRITE_REQUEST_HOOK)(OT_DATA_ID data_id, OT_DATA_VALUE data_value);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommAlOpenThermSlave_Init(OPENTHERM_CHANNEL opentherm_channel);

// @remark  Function to register the function to be called when data needs to be read
void CommAlOpenThermSlave_RegisterDataReadRequestHook(OPENTHERMSLAVE_DATA_READ_REQUEST_HOOK datareadrequest_hook);

// @remark  Function to register the function to be called when data needs to be written
void CommAlOpenThermSlave_RegisterDataWriteRequestHook(OPENTHERMSLAVE_DATA_WRITE_REQUEST_HOOK datawriterequest_hook);

// @remark  Function to check if communication is active
BOOL CommAlOpenThermSlave_IsConnected(void);
//================================================================================================//



#endif /* OPENTHERM__COMMALOPENTHERMSLAVE_H */

