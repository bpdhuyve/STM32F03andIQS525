//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Master Application Link Layer of the OpenTherm communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef OPENTHERM__COMMALOPENTHERMMASTER_H
#define OPENTHERM__COMMALOPENTHERMMASTER_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef OPENTHERM_OBJECT_COUNT
	#define OPENTHERM_OBJECT_COUNT              128
#endif
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
// @brief   Prototype of data read response function
typedef void (*OPENTHERMMASTER_DATA_READ_RESPONSE_HOOK)(OT_DATA_ID data_id, DATA_READ_RESPONSE read_response);

// @brief   Prototype of data read response function
typedef void (*OPENTHERMMASTER_DATA_WRITE_RESPONSE_HOOK)(OT_DATA_ID data_id, DATA_WRITE_RESPONSE write_response);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern OT_DATA_VALUE                commalopenthermmaster_value[OPENTHERM_OBJECT_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommAlOpenThermMaster_Init(OPENTHERM_CHANNEL opentherm_channel);

// @remark  Function to enable/disable the OpenTerm Master
void CommAlOpenThermMaster_Enable(BOOL set_enable);

// @remark  Function to trigger the reading of a data ID
BOOL CommAlOpenThermMaster_TriggerDataRead(OT_DATA_ID data_id);

// @remark  Function to trigger the writing of a data ID
// @remark  Make sure the data is written to the commalopenthermmaster_value buffer before calling this function
BOOL CommAlOpenThermMaster_TriggerDataWrite(OT_DATA_ID data_id);

// @remark  Function to register the function to be called to feedback the data read response
void CommAlOpenThermMaster_RegisterDataReadResponseHook(OPENTHERMMASTER_DATA_READ_RESPONSE_HOOK data_read_response_hook);

// @remark  Function to register the function to be called to feedback the data write response
void CommAlOpenThermMaster_RegisterDataWriteResponseHook(OPENTHERMMASTER_DATA_WRITE_RESPONSE_HOOK data_write_response_hook);
//================================================================================================//



#endif /* OPENTHERM__COMMALOPENTHERMMASTER_H */

