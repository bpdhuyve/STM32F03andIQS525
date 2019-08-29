//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independant interface for PT1024 implementation of ISysAngle
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ISYS_ANGLE_PT1024_H
#define ISYS_ANGLE_PT1024_H
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void SysAnglePt1024_Init(void);

// @remark  Background handler, used in case the compare feature is faked by check in background
void SysAnglePt1024_Handler(void);

// @remark  Register the OnIndexPulse hook, which is in fact always the local function OnIndexPulse in DrvAnglePt1024.c
void SysAnglePt1024_RegisterOnIndexPulse(ANGLE_INDEX_PULSE_HOOK hook);

// @remark  Register the OnCompare hook, which is in fact always the local function OnCompare in DrvAnglePt1024.c
void SysAnglePt1024_RegisterOnCompare(ANGLE_COMPARE_HOOK hook);

// @remark  Init memory stuff specific for the given angle_bus
BOOL SysAnglePt1024_Bus_Init(ANGLE_BUS angle_bus);

// @remark  Init the peripherals needed for the PT1024 bus according the params given by the struct
BOOL SysAnglePt1024_Bus_Config(ANGLE_BUS angle_bus, ANGLE_CONFIG_STRUCT* config_struct_ptr);

// @remark  If the given angle is in the past [(compare - angle_now) >= 0x80000000], the onCompareHook is called directly from this context
BOOL SysAnglePt1024_Bus_SetCompareAngle(ANGLE_BUS angle_bus, U32 angle);

// @remark  The software & hardware counter gets updated by this function call.
//          Best not to call this function when the pulses can be expected because the delay between
//          this function call and the write operation to the HW counter could cause "missed pulses"
//
// PS:      Could fail if the SysAnglePt1024_Bus_Init or SysAnglePt1024_Bus_Config have not been called.
//          In this case a LOG_WRN() statement will be executed !
void SysAnglePt1024_Bus_SetAbsolutePosition(ANGLE_BUS angle_bus, U32 angle);

// @remark  Could fail if the SysAnglePt1024_Bus_Init or SysAnglePt1024_Bus_Config have not been called.
//          In this case a LOG_ERR() statement will be executed !
U32 SysAnglePt1024_Bus_GetAbsolutePosition(ANGLE_BUS angle_bus);

// @remark  Cannot fail, gets fractional U16 pos within cycle
U16 SysAnglePt1024_Bus_GetRelativePosition(ANGLE_BUS angle_bus);

// @remark  REVERSE counting is NOT supported yet, the disconnect will be used instead
//
// PS:      Could fail if the SysAnglePt1024_Bus_Init or SysAnglePt1024_Bus_Config have not been called.
//          In this case a LOG_WRN() statement will be executed !
void SysAnglePt1024_Bus_SetCountingMode(ANGLE_BUS angle_bus, COUNTING_MODE counting_mode);
//================================================================================================//



#endif /* ISYS_ANGLE_PT1024_H */
