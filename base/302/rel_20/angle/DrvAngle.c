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
//------------------------------------------------------------------------------------------------//
#ifndef ANGLE__MAX_REGISTER_ONINDEXPULS
    #define ANGLE__MAX_REGISTER_ONINDEXPULS            3
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
static BOOL DrvAngle_IsAngleCloserToPos(U32 pos_know, U32 current_angle, U32 new_angle);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static DRVANGLE_CTRL_STRUCT     drv_angle_ctrl_struct[ANGLE__DRV_COUNT];
static ANGLE_ON_INDEX_PULSE     on_index_pulse_hooks[ANGLE__DRV_COUNT][ANGLE__MAX_REGISTER_ONINDEXPULS];
static U8                       register_on_indexpuls_count;
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
    U32 position_now = DrvAngle_GetAbsolutePosition(hndl);
    ON_ANGLE_BUFFER* chosen_angle_pntr = NULL;
    
    for(U8 i =0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {      
         if(angle_ctrl_pntr->angles_buffer[i].state != NOT_ACTIVE)
         {
             if((chosen_angle_pntr == NULL) || 
                DrvAngle_IsAngleCloserToPos(position_now, chosen_angle_pntr->angle, angle_ctrl_pntr->angles_buffer[i].angle))
             {
                chosen_angle_pntr = &angle_ctrl_pntr->angles_buffer[i];
                angle_ctrl_pntr->current_angle_pntr = &angle_ctrl_pntr->angles_buffer[i];
             }
         }
    }
    
    if((chosen_angle_pntr != NULL))
    {
        hndl->hook_list_ptr->set_compare_angle_hook(hndl->angle_id, chosen_angle_pntr->angle);
    }   
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvAngle_IsAngleCloserToPos(U32 pos_know, U32 current_angle, U32 new_angle)
{
    if(((current_angle > pos_know) && (new_angle < current_angle) && (new_angle > pos_know))  // |-------- POS_NOW   -------- NEW_ANGLE     -------- CURRENT_ANGLE --------|
    || ((current_angle < pos_know) && (new_angle <  current_angle))                           // |-------- NEW_ANGLE -------- CURRENT_ANGLE -------- POS NOW       --------|
    || ((current_angle < pos_know) && (new_angle > pos_know)))                                // |-------- NEW_ANGLE -------- POS NOW       -------- CURRENT_ANGLE --------|    
    {
        return TRUE;
    }
    return FALSE;
}
 //------------------------------------------------------------------------------------------------//      
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
    
    register_on_indexpuls_count = 0;

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
BOOL DrvAngle_RegisterHookOnRelativePosition(ANGLE_HNDL hndl, U16 angle, EVENT_VPTR_CALLBACK hook,  VPTR data_ptr)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    U32 position_now;
    U16 angle_know;
    U32 angle_position;
    
    Core_CriticalEnter();
    for(U8 i =0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {
        if(angle_ctrl_pntr->angles_buffer[i].state == NOT_ACTIVE)
        {
            position_now = DrvAngle_GetAbsolutePosition(hndl);
            angle_know = position_now & 0x0000FFFF;
            
            angle_position = (position_now & 0xFFFF0000) | angle;
            
            if(angle_know > angle)
            {
                angle_position += 0x10000; //plus one round
            }

            angle_ctrl_pntr->angles_buffer[i].state = ON_ANGLE;
            angle_ctrl_pntr->angles_buffer[i].angle_reached_hook = hook;
            angle_ctrl_pntr->angles_buffer[i].angle = angle_position;  //starts on the absolute position angle
            angle_ctrl_pntr->angles_buffer[i].void_pointer = data_ptr;
             
            if((angle_ctrl_pntr->current_angle_pntr->state == NOT_ACTIVE)    
               || DrvAngle_IsAngleCloserToPos(position_now, angle_ctrl_pntr->current_angle_pntr->angle, angle_position))   
             {
                angle_ctrl_pntr->current_angle_pntr = &angle_ctrl_pntr->angles_buffer[i];
                hndl->hook_list_ptr->set_compare_angle_hook(hndl->angle_id, angle_position);
             }
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
    U32 position_now;
    
    Core_CriticalEnter();
    for(U8 i =0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {
        if(angle_ctrl_pntr->angles_buffer[i].state == NOT_ACTIVE)
        {
            position_now = DrvAngle_GetAbsolutePosition(hndl);
            
            angle_ctrl_pntr->angles_buffer[i].state = ABSOLUTE_POSITION;
            angle_ctrl_pntr->angles_buffer[i].angle_reached_hook = hook;
            angle_ctrl_pntr->angles_buffer[i].angle = ab_position;
            angle_ctrl_pntr->angles_buffer[i].void_pointer = data_ptr;
            
            //komt deze eerder dan de nu ingestelde angle, zoja stel deze in   
            if((angle_ctrl_pntr->current_angle_pntr->state == NOT_ACTIVE) 
                || DrvAngle_IsAngleCloserToPos(position_now, angle_ctrl_pntr->current_angle_pntr->angle, ab_position))
            {
                angle_ctrl_pntr->current_angle_pntr = &angle_ctrl_pntr->angles_buffer[i];
                hndl->hook_list_ptr->set_compare_angle_hook(hndl->angle_id, ab_position);
            }

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
    
    if((angle_ctrl_pntr->current_angle_pntr != NULL) && (angle_ctrl_pntr->current_angle_pntr->state != NOT_ACTIVE))
    {
        //update state
        if(angle_ctrl_pntr->current_angle_pntr->state == ON_ANGLE)
        {
            angle_ctrl_pntr->current_angle_pntr->angle += 0x10000;
        }
        else
        {
            angle_ctrl_pntr->current_angle_pntr->state = NOT_ACTIVE;
        }

        //call callback hook
        if(angle_ctrl_pntr->current_angle_pntr->angle_reached_hook != NULL)
        {
            angle_ctrl_pntr->current_angle_pntr->angle_reached_hook(angle_ctrl_pntr->current_angle_pntr->void_pointer);
        }
    }
    
    //get the lowest active value in the buffer and set it
    DrvAngle_SetNextCompareAngle(hndl, compare_angle);
    Core_CriticalExit();
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_OnIndexPulse(ANGLE_HNDL hndl, U32 angle_at_index_pulse)
{
    for(U8 i = 0;i < ANGLE__MAX_REGISTER_ONINDEXPULS; i++)
    {
        if(on_index_pulse_hooks[hndl->angle_id][i] != NULL)
        {
            on_index_pulse_hooks[hndl->angle_id][i](angle_at_index_pulse);
        }
    }
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_ChangeAbsolutePostionOnIndexPuls(ANGLE_HNDL hndl, U32 angle)
{
    if((hndl != NULL) && (hndl->hook_list_ptr != NULL) && (hndl->hook_list_ptr->set_absolute_position_at_indexpuls != NULL))
    {
        hndl->hook_list_ptr->set_absolute_position_at_indexpuls(hndl->angle_id, angle);
    }
    else
    {
        LOG_WRN("ANGLE ChangeAbsolutePostionOnIndexPuls is NULL"); //attention: WARNING log !!
    }
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_RegisterHookOnIndexPulse(ANGLE_HNDL hndl, ANGLE_ON_INDEX_PULSE hook)
{
    if(register_on_indexpuls_count < ANGLE__MAX_REGISTER_ONINDEXPULS)
    {
        on_index_pulse_hooks[hndl->angle_id][register_on_indexpuls_count++] = hook;
    }
}
//------------------------------------------------------------------------------------------------//
void DrvAngle_ClearAllRegisterAngles(ANGLE_HNDL hndl)
{
    DRVANGLE_CTRL_STRUCT*     angle_ctrl_pntr;
    angle_ctrl_pntr = &drv_angle_ctrl_struct[hndl->angle_id];
    
    Core_CriticalEnter();
    angle_ctrl_pntr->current_angle_pntr = NOT_ACTIVE;
    for(U8 i = 0; i < ANGLE__DRV_COMPARE_BUFFER; i++)
    {
         if(angle_ctrl_pntr->angles_buffer[i].state != NOT_ACTIVE)
         {
             angle_ctrl_pntr->angles_buffer[i].state = NOT_ACTIVE;
         }
    }
    Core_CriticalExit();
}
//================================================================================================//
