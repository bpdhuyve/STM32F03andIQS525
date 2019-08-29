//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independant interface for PT1024 implementation of ISysAngle
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ISYS_ANGLE_QUADENC_H
#define ISYS_ANGLE_QUADENC_H
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
typedef void (*QUADENC_ENCODERFAULTS_HOOK)(BOOL error, S16 mismatch);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void SysAngleQuadEnc_Init(void);

// @remark  Background handler, used in case the compare feature is faked by check in background
void SysAngleQuadEnc_Handler(void);

// @remark  Register the OnIndexPulse hook, which is in fact always the local function OnIndexPulse in DrvAngleQuadEnc.c
void SysAngleQuadEnc_RegisterOnIndexPulseEnc(ANGLE_INDEX_PULSE_HOOK hook);

// @remark  Register the OnCompare hook, which is in fact always the local function OnCompare in DrvAngleQuadEnc.c
void SysAngleQuadEnc_RegisterOnCompareEnc(ANGLE_COMPARE_HOOK hook);

// @remark  Init memory stuff specific for the given angle_bus
BOOL SysAngleQuadEnc_InitEnc(ANGLE_BUS angle_bus);

// @remark  Init the peripherals needed for the PT1024 bus according the params given by the struct
BOOL SysAngleQuadEnc_ConfigEnc(ANGLE_BUS angle_bus, ANGLE_CONFIG_STRUCT* config_struct_ptr);

// @remark change absolute position of the encoder on the first index puls you see
BOOL SysAngleQuadEnc_SetCompareAngleEnc(ANGLE_BUS angle_bus, U32 angle);

// @remark change absolute position of the encoder on the first index puls you see
void SysAngleQuadEnc_ChangeAbsolutePositionOnIndexPuls(ANGLE_BUS angle_bus, U32 value_at_index_puls);

// @remark  The software & hardware counter gets updated by this function call.
//          Best not to call this function when the pulses can be expected because the delay between
//          this function call and the write operation to the HW counter could cause "missed pulses"
//
// PS:      Could fail if the SysAnglePt1024_Bus_Init or SysAnglePt1024_Bus_Config have not been called.
//          In this case a LOG_WRN() statement will be executed !
void SysAngleQuadEnc_SetAbsolutePositionEnc(ANGLE_BUS angle_bus, U32 angle);

// @remark  Could fail if the SysAnglePt1024_Bus_Init or SysAnglePt1024_Bus_Config have not been called.
//          In this case a LOG_ERR() statement will be executed !
U32 SysAngleQuadEnc_GetAbsolutePositionEnc(ANGLE_BUS angle_bus);

// @remark  Cannot fail, gets fractional U16 pos within cycle
U16 SysAngleQuadEnc_GetRelativePositionEnc(ANGLE_BUS angle_bus);

// @remark  on the index puls we check if there are no pulses lost
//          if pulses are lost we will call all who are registerd
void SysAngleQuadEnc_RegisterOnEncoderFaults(ANGLE_BUS angle_bus, QUADENC_ENCODERFAULTS_HOOK hook);
//================================================================================================//



#endif /* ISYS_ANGLE_PT1024_H */
