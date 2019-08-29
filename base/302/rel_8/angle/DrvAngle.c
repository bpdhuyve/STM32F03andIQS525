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
//------------------------------------------------------------------------------------------------//
// @brief  Defines the terminal baudrate to be used
#ifndef ANGLE__DRV_COMPARE_BUFFER
    #define ANGLE__DRV_COMPARE_BUFFER   8
#endif
//------------------------------------------------------------------------------------------------//
#ifndef ANGLE__DRV_COUNT
    #define ANGLE__DRV_COUNT            ANGLE_BUS_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "angle\DrvAngle.h"


//delete this...
#include "gpio\SysGpio.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    NOT_ACTIVE,
    ABSOLUTE_POSITION,  // one time callback on position 
    ON_ANGLE            // callback on every angle, example 60° or sommething
}
ON_ANGEL_STATE;

typedef struct
{
    EVENT_VPTR_CALLBACK angle_reached_hook;
    U32                 angle;
    ON_ANGEL_STATE      state;
    VPTR                void_pointer;
}
ON_ANGLE_BUFFER;

typedef struct
{
    U8                  angle_id;
    ON_ANGLE_BUFFER     angles_buffer[ANGLE__DRV_COMPARE_BUFFER];
    ON_ANGLE_BUFFER*    current_angle_pntr;
}
DRVANGLE_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrvAngle_SetNextCompareAngle(ANGLE_HNDL hndl, U32 compare_angle);
static void DrvAngle_SetNextCompareAngleIfNeeded(ANGLE_HNDL hndl, U32 compare_angle, ON_ANGLE_BUFFER* new_angle);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static DRVANGLE_CTRL_STRUCT     drv_angle_ctrl_struct[ANGLE__DRV_COUNT];
static ANGLE_ON_INDEX_PULSE     on_index_pulse_hooks[ANGLE__DRV_COUNT];

//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DrvAngle_SetNextCompareAngle(ANGLE_HNDL hndl, U32 compare_angle)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    
    U32 min_angle = 0xFFFFFFFF;
    EVENT_VPTR_CALLBACK min_angle_hook = NULL;
    
    for(U8 i =0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {      
         //beslis volgende in te stellen hook
         if(angle_ctrl_pntr->angles_buffer[i].state != NOT_ACTIVE && angle_ctrl_pntr->angles_buffer[i].angle < min_angle)
         {
             min_angle = angle_ctrl_pntr->angles_buffer[i].angle;
             min_angle_hook = angle_ctrl_pntr->angles_buffer[i].angle_reached_hook;
             angle_ctrl_pntr->current_angle_pntr = &angle_ctrl_pntr->angles_buffer[i];
         }
    }
    
    if((min_angle_hook != NULL))
    {
        hndl->hook_list_ptr->set_compare_angle_hook(hndl->angle_id, min_angle);
    }   
}
//------------------------------------------------------------------------------------------------//
static void DrvAngle_SetNextCompareAngleIfNeeded(ANGLE_HNDL hndl, U32 compare_angle, ON_ANGLE_BUFFER* new_angle)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    Core_CriticalEnter();
    //beslis volgende in te stellen hook
    if((new_angle->angle < angle_ctrl_pntr->current_angle_pntr->angle) || (angle_ctrl_pntr->current_angle_pntr->state == NOT_ACTIVE))
    {
        angle_ctrl_pntr->current_angle_pntr = new_angle;
        hndl->hook_list_ptr->set_compare_angle_hook(hndl->angle_id, new_angle->angle);;
    }   
    Core_CriticalExit();
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvAngle_Init(void)
{
    MODULE_INIT_ONCE();
    //place init code which must only be executed once here
    
    for(ANGLE_ID j=0; j < ANGLE__DRV_COUNT; j++)
    {
        for(U8 i =0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
        {      
            drv_angle_ctrl_struct[j].angles_buffer[i].state = NOT_ACTIVE;
        }
        drv_angle_ctrl_struct[j].current_angle_pntr = &drv_angle_ctrl_struct[j].angles_buffer[ANGLE__DRV_COMPARE_BUFFER-1];
    }

    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here

}
//------------------------------------------------------------------------------------------------//
BOOL DrvAngle_Config(ANGLE_HNDL hndl, ANGLE_CONFIG_STRUCT* config_struct_ptr)
{
    DRVANGLE_CTRL_STRUCT* angle_ctrl_pntr;
    angle_ctrl_pntr = drv_angle_ctrl_struct;
    
    if((hndl != NULL) && (hndl->hook_list_ptr != NULL) && (hndl->hook_list_ptr->config_hook != NULL))
    {   
        if(hndl->angle_id < ANGLE__DRV_COUNT)
        {
            angle_ctrl_pntr->angle_id = hndl->angle_id;
            return hndl->hook_list_ptr->config_hook(hndl->angle_id, config_struct_ptr);
        }
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
BOOL DrvAngle_RegisterHookOnAngle(ANGLE_HNDL hndl, U16 angle, EVENT_VPTR_CALLBACK hook,  VPTR data_ptr)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    Core_CriticalEnter();
    U32 position_know;
    U16 angle_know;
    U32 angle_position;
    for(U8 i =0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {
         if(angle_ctrl_pntr->angles_buffer[i].state == NOT_ACTIVE)
         {
             position_know = DrvAngle_GetAbsolutePosition(hndl);
             angle_know = position_know & 0xFFFF0000;
             
             if(angle_know > angle)
             {
                 position_know += 0x00010000; //plus one round
             }
            
             angle_position = (position_know & 0xFFFF0000) | angle_know;

             angle_ctrl_pntr->angles_buffer[i].state = ON_ANGLE;
             angle_ctrl_pntr->angles_buffer[i].angle_reached_hook = hook;
             angle_ctrl_pntr->angles_buffer[i].angle = angle_position;  //starts on the absolute position angle
             angle_ctrl_pntr->angles_buffer[i].void_pointer = data_ptr;
             DrvAngle_SetNextCompareAngleIfNeeded(hndl, angle_ctrl_pntr->angles_buffer[i].angle, &angle_ctrl_pntr->angles_buffer[i]);
             Core_CriticalExit();
             return TRUE;
         }
    }
    Core_CriticalExit();
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvAngle_RegisterHookOnAbsolutePosition(ANGLE_HNDL hndl, U32 ab_position, EVENT_VPTR_CALLBACK hook, VPTR data_ptr)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    
    Core_CriticalEnter();
    for(U8 i =0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {
         if(angle_ctrl_pntr->angles_buffer[i].state == NOT_ACTIVE)
         {
             angle_ctrl_pntr->angles_buffer[i].state = ABSOLUTE_POSITION;
             angle_ctrl_pntr->angles_buffer[i].angle_reached_hook = hook;
             angle_ctrl_pntr->angles_buffer[i].angle = ab_position;
             angle_ctrl_pntr->angles_buffer[i].void_pointer = data_ptr;
             DrvAngle_SetNextCompareAngleIfNeeded(hndl, ab_position, &angle_ctrl_pntr->angles_buffer[i]);
             Core_CriticalExit();
             return TRUE;
         }
    }
    Core_CriticalExit();
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_OnCompare(ANGLE_HNDL hndl, U32 compare_angle)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    
    Core_CriticalEnter();
    
    //call callback hook
    if((angle_ctrl_pntr->current_angle_pntr->angle_reached_hook != NULL) && (angle_ctrl_pntr->current_angle_pntr->state != NOT_ACTIVE))
    {
       angle_ctrl_pntr->current_angle_pntr->angle_reached_hook(angle_ctrl_pntr->current_angle_pntr->void_pointer);
    }
    
    //update state
    if(angle_ctrl_pntr->current_angle_pntr->state == ON_ANGLE)
    {
        angle_ctrl_pntr->current_angle_pntr->angle += hndl->cycle_step;
    }
    else
    {
        angle_ctrl_pntr->current_angle_pntr->state = NOT_ACTIVE;
    }
    
    //get the lowest active value in the buffer and set it
    DrvAngle_SetNextCompareAngle(hndl, compare_angle);
    Core_CriticalExit();
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_OnIndexPulse(ANGLE_HNDL hndl, U32 angle_at_index_pulse)
{
    if(on_index_pulse_hooks[hndl->angle_id] != NULL)
    {
        on_index_pulse_hooks[hndl->angle_id](angle_at_index_pulse);
    }
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_RegisterHookOnIndexPulse(ANGLE_HNDL hndl, ANGLE_ON_INDEX_PULSE hook)
{
    on_index_pulse_hooks[hndl->angle_id] = hook;
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_ClearAllRegisterOnAbsolutePosition(ANGLE_HNDL hndl)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    
    Core_CriticalEnter();
    for(U8 i = 0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {
         if(angle_ctrl_pntr->angles_buffer[i].state == ABSOLUTE_POSITION)
         {
             angle_ctrl_pntr->angles_buffer[i].state = NOT_ACTIVE;
         }
    }
    Core_CriticalExit();
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_ClearAllRegisterOnAngle(ANGLE_HNDL hndl)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    
    Core_CriticalEnter();
    for(U8 i = 0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {
         if(angle_ctrl_pntr->angles_buffer[i].state == ON_ANGLE)
         {
             angle_ctrl_pntr->angles_buffer[i].state = NOT_ACTIVE;
         }
    }
    Core_CriticalExit();
}
//================================================================================================//
