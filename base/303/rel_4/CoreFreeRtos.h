//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Main Core module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//------------------------------------------------------------------------------------------------//
#ifndef COREFREERTOS_H
#define COREFREERTOS_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
// KERNEL
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
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
// @remark  none
void CoreFreeRtos_Register(xTaskHandle task);
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif // COREFREERTOS_H

