//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Driver for managing angular position information
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define ANGLE__DRV_ANGLE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef ANGLE__DRV_ANGLES_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               ANGLE__DRV_ANGLE_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "angle\DrvAngle.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvAngle_Init(void)
{
    MODULE_INIT_ONCE();
    //place init code which must only be executed once here

    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here

}
//------------------------------------------------------------------------------------------------//
BOOL DrvAngle_Config(ANGLE_HNDL hndl, ANGLE_CONFIG_STRUCT* config_struct_ptr)
{
    if((hndl != NULL) && (hndl->hook_list_ptr != NULL) && (hndl->hook_list_ptr->config_hook != NULL))
    {
        return hndl->hook_list_ptr->config_hook(hndl->angle_id, config_struct_ptr);
    }
    LOG_WRN("ANGLE config function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_SetAbsolutePosition(ANGLE_HNDL hndl, U32 angle)
{
    if((hndl != NULL) && (hndl->hook_list_ptr != NULL) && (hndl->hook_list_ptr->set_absolute_position_hook != NULL))
    {
        hndl->hook_list_ptr->set_absolute_position_hook(hndl->angle_id, angle);
    }
    else
    {
        LOG_WRN("ANGLE SetAbsolutePosition is NULL"); //attention: WARNING log !!
    }
}
//------------------------------------------------------------------------------------------------//
U32 DrvAngle_GetAbsolutePosition(ANGLE_HNDL hndl)
{
    if((hndl != NULL) && (hndl->hook_list_ptr != NULL) && (hndl->hook_list_ptr->get_absolute_position_hook != NULL))
    {
        return hndl->hook_list_ptr->get_absolute_position_hook(hndl->angle_id);
    }
    LOG_ERR("ANGLE GetAbsolutePosition is NULL"); //attention: ERROR log !!
    return 0; //will not be reached because of the above ERR statement (will end up in Core_OnErrorHandler)
}
//------------------------------------------------------------------------------------------------//
U16 DrvAngle_GetRelativePosition(ANGLE_HNDL hndl)
{
    if((hndl != NULL) && (hndl->hook_list_ptr != NULL) && (hndl->hook_list_ptr->get_relative_position_hook != NULL))
    {
        return hndl->hook_list_ptr->get_relative_position_hook(hndl->angle_id);
    }
    LOG_ERR("ANGLE GetAbsolutePosition is NULL"); //attention: ERROR log !!
    return 0; //will not be reached because of the above ERR statement (will end up in Core_OnErrorHandler)
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_SetCountingMode(ANGLE_HNDL hndl, COUNTING_MODE counting_mode)
{
    if((hndl != NULL) && (hndl->hook_list_ptr != NULL) && (hndl->hook_list_ptr->set_counting_mode_hook != NULL))
    {
        hndl->hook_list_ptr->set_counting_mode_hook(hndl->angle_id, counting_mode);
    }
    else
    {
        LOG_ERR("ANGLE SetCountingMode is NULL"); //attention: ERROR log !!
    }
}
//------------------------------------------------------------------------------------------------//
BOOL DrvAngle_ConfigForAngleEvents(ANGLE_HNDL hndl, ANGLE_EVENT_CONFIG_STRUCT* config_struct_ptr)
{
    //TODO
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvAngle_RegisterHookOnAngle(ANGLE_HNDL hndl, U32 angle, EVENT_VPTR_CALLBACK hook)
{
    //TODO
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_OnCompare(ANGLE_HNDL hndl, U32 compare_angle)
{
    //TODO
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_OnIndexPulse(ANGLE_HNDL hndl, U32 angle_at_index_pulse)
{
    //TODO
}
//================================================================================================//
