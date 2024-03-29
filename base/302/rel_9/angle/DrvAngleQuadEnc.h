//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Angle implementation specific for PICAN/PCMS PT1024
// Only allows for a single PT1024 bus per device / software.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ANGLE_DRVANGLEQUADENC_H
#define ANGLE_DRVANGLEQUADENC_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "angle/SysAngleQuadEnc.h"

//DRV include section
#include "angle\DrvAngle.h"
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
void DrvAngleQuadEnc_Init(void);

// @remark  Returns a hndl to the implementation for the given angle bus
ANGLE_HNDL DrvAngleQuadEnc_Register(ANGLE_BUS angle_bus);

// @remark  None
U32 DrvAngleQuadEnc_GetAbsolutePos(ANGLE_BUS angle_bus);

// @remark  None
U16 DrvAngleQuadEnc_GetRelativePos(ANGLE_BUS angle_bus);
//================================================================================================//



#endif /* ANGLE_DRVANGLEQUADENC_H */
