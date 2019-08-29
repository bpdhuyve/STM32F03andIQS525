//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Angle/position information handling
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ISYS_ANGLE_H
#define ISYS_ANGLE_H
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
typedef enum
{
    ANGLE_INDEX_PULSE_CONFIG_DO_NOTHING,         //don"t use HW support to auto correct on index pulse
    ANGLE_INDEX_PULSE_CONFIG_INIT_ONE_TIME_ONLY, //use HW support 1st time only, eg. on isr the HW feature is turned off
    ANGLE_INDEX_PULSE_CONFIG_INIT_ON_EVERY_INDEX, //use HW support to init at every index puls (only useful for 1 cycle information)
}
ANGLE_INDEX_PULSE_CONFIG;

typedef struct
{
    U16                         resolution; // pt1024 resolution = 2^10 = 1024
    ANGLE_INDEX_PULSE_CONFIG    init_by_using_index_pulse;
    U32                         init_value_at_index_pulse;
}
ANGLE_CONFIG_STRUCT;

typedef enum
{
    COUNTING_MODE_DISCONNECT,
    COUNTING_MODE_FORWARD,
    COUNTING_MODE_REVERSE
}
COUNTING_MODE;

// @remark  Hook to be called by SysAnglePt1024 on indexpuls detection.  The callee will always be a function in DrvAnglePt1024.c
//          So, in theory, this hook could be replaced by a direct function call.
//          However, this is not done to decouple Sys lib implementation from Drv lib
//          Why ?  Is there an example case where this would be usefull ? Or is it to be able to says : NEVER calls from SYS lib to DRV lib
//
// @remark  MUST return TRUE if a correction was done (using the implementation behind DrvAngle_SetAbsolutePosition)
//          The piece of code calling this hook can use it/might need it to know wether or not it should
//          increment/decrement the software cycle counter (if it uses one)
typedef BOOL (*ANGLE_INDEX_PULSE_HOOK)(ANGLE_BUS angle_bus, U32 angle_at_index_pulse);

// @remark  Hook to be called by SysAnglePt1024 on compare event.  The callee will always be a function in DrvAnglePt1024.c
//          So, in theory, this hook could be replaced by a direct function call.
//          However, this is not done to decouple Sys lib implementation from Drv lib
//          Why ?  Is there an example case where this would be usefull ? Or is it to be able to says : NEVER calls from SYS lib to DRV lib
//
// PS:      the param compare_angle is to ease implementation on the callee side, there the callee can check
//          for which angle he is being called-back
typedef void (*ANGLE_COMPARE_HOOK)(ANGLE_BUS angle_bus, U32 compare_angle);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* ISYS_ANGLE_H */
