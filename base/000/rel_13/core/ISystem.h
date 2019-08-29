//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions of CPU and System functionality.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORE__ISYSTEM_H
#define CORE__ISYSTEM_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of application signatures to be allocated
#ifndef APPLICATION_SIGNATURE_COUNT
	#define APPLICATION_SIGNATURE_COUNT     6
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "core\ISysCompiler.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define WD_MIN_OVERFLOW_TIME_DEFAULT            50  //ms
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Prototype of a standard event callback function with no return type and no arguments, normaly used to signal an event
typedef void (*EVENT_CALLBACK)(void);

// @brief   Prototype of a standard event callback function with no return type and a VPTR argument
typedef void (*EVENT_VPTR_CALLBACK)(VPTR data_ptr);

// @brief   Struct containing signatures used to determine behaviour after reset
typedef struct
{
    U32     reset_to_boot_signature;
    U32     testmode_signature;
#if (APPLICATION_SIGNATURE_COUNT) > 0
    U32     application_signature[APPLICATION_SIGNATURE_COUNT];
#endif
}
SYSTEM_SIGNATURES;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
// @brief   variable containing reset signatures
extern SYSTEM_SIGNATURES       system_signatures;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Initialiser for the System module
void System_Init(void);

// @remark  Function that enables the WATCHDOG with the given overflow_time in ms.
//          This function will be called before System_Init() from main().
//          The implementation has to allow this.
void System_EnableWatchdog(U32 overflow_time_in_ms);

// @remark  Watchdog kick method.
void System_KickDog(void);

// @remark  This function allows you to reset based on an alternative method (e.g.: NVIC_SystemReset())
//          and this function is called in Core_Reset()
//          This function can be implemented empty when a watchdog reset is sufficient for your system.
void System_Reset(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CORE__ISYSTEM_H */
