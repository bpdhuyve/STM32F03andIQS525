//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Driver for managing angular position information
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ANGLE__DRV_ANGLE_H
#define ANGLE__DRV_ANGLE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "angle\ISysAngle.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8  ANGLE_ID;
typedef U32 ANGLE_CYCLE_STEP;

typedef BOOL (*ANGLE_CONFIG)(ANGLE_ID angle_id, ANGLE_CONFIG_STRUCT* config_struct_ptr);

typedef BOOL (*ANGLE_SET_COMPARE_ANGLE)(ANGLE_ID angle_id, U32 angle);

typedef void (*ANGLE_SET_ABSOLUTE_POSITION)(ANGLE_ID angle_id, U32 angle);

typedef U32 (*ANGLE_GET_ABSOLUTE_POSITION)(ANGLE_ID angle_id);

typedef U16 (*ANGLE_GET_RELATIVE_POSITION)(ANGLE_ID angle_id);

typedef void (*ANGLE_SET_COUNTING_MODE)(ANGLE_ID angle_id, COUNTING_MODE counting_mode);

typedef struct
{
    ANGLE_CONFIG                       config_hook;
    ANGLE_SET_COMPARE_ANGLE            set_compare_angle_hook;
    ANGLE_SET_ABSOLUTE_POSITION        set_absolute_position_hook;
    ANGLE_GET_ABSOLUTE_POSITION        get_absolute_position_hook;
    ANGLE_GET_RELATIVE_POSITION        get_relative_position_hook;
    ANGLE_SET_COUNTING_MODE            set_counting_mode_hook;   
}
ANGLE_HOOK_LIST;

typedef struct
{
    ANGLE_HOOK_LIST*	     hook_list_ptr;
    ANGLE_ID                 angle_id;
    ANGLE_CYCLE_STEP         cycle_step;    
}
ANGLE_STRUCT;

typedef ANGLE_STRUCT*        ANGLE_HNDL;

typedef enum
{
    ANGLE_EVENT_DIRECTION_FORWARD,
    ANGLE_EVENT_DIRECTION_REVERSE
}
ANGLE_EVENT_DIRECTION;

typedef struct
{
    ANGLE_EVENT_DIRECTION    direction;
}
ANGLE_EVENT_CONFIG_STRUCT;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void DrvAngle_Init(void);

// @remark  None
BOOL DrvAngle_Config(ANGLE_HNDL hndl, ANGLE_CONFIG_STRUCT* config_struct_ptr);

// @remark  Format is fractional : 16bit cycle + 16bit fractional within cycle
//          Possibly software & hardware counters are updated
void DrvAngle_SetAbsolutePosition(ANGLE_HNDL hndl, U32 angle);

// @remark  Returns the actual position (cycle + pos within cycle)
U32 DrvAngle_GetAbsolutePosition(ANGLE_HNDL hndl);

// @remark  Returns the actual position within a cycle
U16 DrvAngle_GetRelativePosition(ANGLE_HNDL hndl);

// @remark  Some implementations might not support this, meaning, they will not react on this call
//          However, they should implement the function and include a warning log statement in it !
void DrvAngle_SetCountingMode(ANGLE_HNDL hndl, COUNTING_MODE counting_mode);

// @remark  Enables the angle bus to allow registering "hooks on angles"
BOOL DrvAngle_ConfigForAngleEvents(ANGLE_HNDL hndl, ANGLE_EVENT_CONFIG_STRUCT* config_struct_ptr);

// @remark  If the angle is in the past, it is possible, but not sure, the hook will called from this
//          registration context.  This behaviour is SysLib dependant.         
//          gives a callback every time the angle is reached and starts 
//          start from the absolute position you give with argument angle
BOOL DrvAngle_RegisterHookOnAngle(ANGLE_HNDL hndl, U32 angle, EVENT_VPTR_CALLBACK hook,  VPTR data_ptr);

// @remark  If the angle is in the past, it is possible, but not sure, the hook will called from this
//          registration context.  This behaviour is SysLib dependant.
BOOL DrvAngle_RegisterHookOnAbsolutePosition(ANGLE_HNDL hndl, U32 angle, EVENT_VPTR_CALLBACK hook,  VPTR data_ptr);

// @remark  The compare_angle param is the original given angle for which this callback is being called
//          It is not the actual angle at compare event !
//          The reason is to be able to use it for doublechecking that you get called for the right angle
void DrvAngle_OnCompare(ANGLE_HNDL hndl, U32 compare_angle);

// @remark  The angle_at_index_pulse is the value of the timer at the moment of index puls.
//          This could be supported by hardware or not... SysLib dependant.
void DrvAngle_OnIndexPulse(ANGLE_HNDL hndl, U32 angle_at_index_pulse);
// @remark  None
void DrvAngle_ClearAllRegisterOnAbsolutePosition(ANGLE_HNDL hndl);
//================================================================================================//



#endif /* ANGLE__DRV_ANGLE_H */
