//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Main Core module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//------------------------------------------------------------------------------------------------//
#ifndef CORE_H
#define CORE_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @remark  Defines if the module init checking should be active
#ifndef MODULE_INIT_CHECK
    #define MODULE_INIT_CHECK               1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "core\SysLibAll.h"

//DRIVER lib include section
#include "sci\DrvSciChannel.h"

//CORE lib include section
#include "CoreBuffer.h"
#include "CoreConvert.h"
#include "CoreLog.h"
#include "CoreQ.h"
#include "CoreString.h"
#include "CoreTask.h"
#include "CoreTerm.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define TESTMODE_SIGNATURE                  0x7E5730DE
#define BOOTMODE_SIGNATURE                  0xFD12ACE7

#define PREV(struct_ptr, TYPE)              ((TYPE*)struct_ptr->prev_ptr)
#define NEXT(struct_ptr, TYPE)              ((TYPE*)struct_ptr->next_ptr)

// @remark  macro's to check if a module has been inited before using it
#if MODULE_INIT_CHECK
    #define MODULE_DECLARE()       static MODULE_INIT_LEVEL module_init = MODULE_NO_INIT
    #define MODULE_INIT_ONCE()     if(module_init == MODULE_NO_INIT){module_init = MODULE_INIT_START;
    #define MODULE_INIT_DONE()     module_init = MODULE_INIT_DONE;}
    #define MODULE_CHECK()         if(module_init != MODULE_INIT_DONE){LOG_ERR("%s", PCSTR(core_module_error_string));}
#else
    #define MODULE_DECLARE()
    #define MODULE_INIT_ONCE()
    #define MODULE_INIT_DONE()
    #define MODULE_CHECK()
#endif

#define CORE_STATE_ACTIVE                   0x01
#define CORE_STATE_TEST                     0x02
#define CORE_STATE_ERROR                    0x80

#define CALL_HOOK(x)                if(x != NULL)x                 //simple define to prevent calling a null hook
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    MODULE_NO_INIT,
    MODULE_INIT_START,
    MODULE_INIT_DONE,
}
MODULE_INIT_LEVEL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern const STRING     core_module_error_string;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#ifndef __cplusplus
// @remark  none
int main(void);
#endif
//------------------------------------------------------------------------------------------------//
// @remark  register background handler of a module to be called in active mode
BOOL Core_RegisterModuleHandler(EVENT_CALLBACK module_handler);
//------------------------------------------------------------------------------------------------//
// @remark  unregister background handler of a module
void Core_UnregisterModuleHandler(EVENT_CALLBACK module_handler);
//------------------------------------------------------------------------------------------------//
// @remark  returns if this is the first error
BOOL Core_OnErrorReport(void);
//------------------------------------------------------------------------------------------------//
// @remark  should only be called once (on first error)
void Core_OnErrorHandler(void);
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_DebugSetSciChannel(SCI_CHANNEL_HNDL channel_hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_CriticalEnter(void);
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_CriticalExit(void);
//------------------------------------------------------------------------------------------------//
// @remark  resets the processor by enabling the watchdog and wait in an endless loop
void Core_Reset(void);
//------------------------------------------------------------------------------------------------//
// @remark  resets the processor and the bootsoftware should see the signature in ram and stay in boot
void Core_ResetToBoot(void);
//------------------------------------------------------------------------------------------------//
// @remark  resets the processor into testmode
void Core_ResetToTestMode(void);
//------------------------------------------------------------------------------------------------//
// @remark  returns the core state using the CORE_STATE_ bitmasks
U8 Core_GetCoreState(void);
//------------------------------------------------------------------------------------------------//
#ifdef USE_FREERTOS
// @remark  none
void CoreFreeRtos_Register(xTaskHandle task);
#endif
//================================================================================================//



#endif // CORE_H

