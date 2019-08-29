//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Main Core module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//------------------------------------------------------------------------------------------------//
#define CORE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CORE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines if the watchdog must be enabled, WD will be enabled if PRODUCT_TEST_NUMBER == 0
#ifndef WATCHDOG_ENABLED
    #if ((PRODUCT_VERSION_NUMBER != 0) && (PRODUCT_TEST_NUMBER == 0))
        #define WATCHDOG_ENABLED        1
    #else
        #define WATCHDOG_ENABLED        0
    #endif
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines the watchdog timeout in ms
#ifndef WATCHDOG_TIMEOUT_IN_MS
    #define WATCHDOG_TIMEOUT_IN_MS      WD_MIN_OVERFLOW_TIME_DEFAULT
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines if testmode must be included (1) or not (0)
#ifndef INCLUDE_TESTMODE
    #define INCLUDE_TESTMODE            1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef USE_STACK_SAFETYCHECKER
    #define USE_STACK_SAFETYCHECKER     0
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STACK_SAFETY_BYTE
    #define STACK_SAFETY_BYTE           0x63
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STACK_SAFETY_SIZE
    #define STACK_SAFETY_SIZE           32
#endif
//------------------------------------------------------------------------------------------------//
#if USE_STACK_SAFETYCHECKER
#ifndef STACK_POINTER_START_ADDRESS
    #error "STACK_POINTER_START_ADDRESS not defined in AppConfig.h"
#endif
#endif
//------------------------------------------------------------------------------------------------//
#if USE_STACK_SAFETYCHECKER
#ifndef STACK_POINTER_END_ADDRESS
    #error "STACK_POINTER_END_ADDRESS not defined in AppConfig.h"
#endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

#include "gpio\SysPin.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define TESTMODE_SIGNATURE                  0x7E5730DE

#define CORE_STATE_ACTIVE                   0x01
#define CORE_STATE_TEST                     0x02
#define CORE_STATE_ERROR                    0x80
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    EVENT_CALLBACK  init;
    EVENT_CALLBACK  handler;
    EVENT_CALLBACK  on_error_report;
}
CORE_HANDLES;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreInfo(void);
#endif

#if (USE_STACK_SAFETYCHECKER)
static void Stack_InitSafetyChecker(U8* stack_start_ptr, U8* stack_end_ptr);
static void Stack_SafetyChecker(void);
#endif
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
extern void AppMain_Init(void);
extern void AppMain_Handler(void);
extern void AppMain_OnErrorReport(void);

#if (INCLUDE_TESTMODE)
extern void AppTest_Init(void);
extern void AppTest_Handler(void);
extern void AppTest_OnErrorReport(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static U8                   core_state = 0;
static U16                  core_disable_isr_count = 0;

#if (INCLUDE_TESTMODE)
static const CORE_HANDLES   core_handles[2] = {{AppMain_Init, AppMain_Handler, AppMain_OnErrorReport},
                                               {AppTest_Init, AppTest_Handler, AppTest_OnErrorReport}};
#else
static const CORE_HANDLES   core_handles[1] = {{AppMain_Init, AppMain_Handler, AppMain_OnErrorReport}};
#endif

static const CORE_HANDLES*  core_handles_ptr = core_handles;

#if (USE_STACK_SAFETYCHECKER)
static U8*   stackcheck_check_ptr;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
const STRING            core_module_error_string = "Module called without init";
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
// @remark  none
static void Command_CoreInfo(void)
{
    STRING mode[2] = {"APP", "TEST"};
    STRING state[2] = {"INIT", "ACTIVE"};
    STRING error[2] = {"", "ERROR"};

    LOG_TRM("Core");
    LOG_TRM("- state     : %s %s %s", PCSTR(mode[(core_state & CORE_STATE_TEST) > 0]),
                                      PCSTR(state[(core_state & CORE_STATE_ACTIVE) > 0]),
                                      PCSTR(error[(core_state & CORE_STATE_ERROR) > 0]));
}
#endif
//------------------------------------------------------------------------------------------------//
 #if (USE_STACK_SAFETYCHECKER)
static void Stack_InitSafetyChecker(U8* stack_start_ptr, U8* stack_end_ptr)
{
    if (stack_start_ptr < stack_end_ptr)//start address is lower than end address, this means stack goes up
    {
        MEMSET((VPTR)(stack_end_ptr-STACK_SAFETY_SIZE), STACK_SAFETY_BYTE, STACK_SAFETY_SIZE);   //fill last part with safety bytes
        stackcheck_check_ptr = stack_end_ptr-STACK_SAFETY_SIZE;
    }
    else  //start address is higher than end address, this means stack goes down
    {
        MEMSET((VPTR)(stack_end_ptr), STACK_SAFETY_BYTE, STACK_SAFETY_SIZE);   //fill last part with safety bytes
        stackcheck_check_ptr = stack_end_ptr;
    }
}
//------------------------------------------------------------------------------------------------//
static void Stack_SafetyChecker(void)
{
    U8* check_ptr;

    check_ptr = stackcheck_check_ptr;
    while(check_ptr < (stackcheck_check_ptr + STACK_SAFETY_SIZE))
    {
	    if(*check_ptr != STACK_SAFETY_BYTE)
	    {
	        LOG_ERR("STACK_OVERFLOW");
	    }
	    check_ptr++;
    }
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @remark  none
int main(void)
{
	#if(WATCHDOG_ENABLED)
	{
    	System_EnableWatchdog(WATCHDOG_TIMEOUT_IN_MS);
    	System_KickDog();
	}
	#else
	{
    	#warning "watchdog disabled"
	}
	#endif

    System_Init();
    SysInt_Init();
    SysPin_Init();

    #if (USE_STACK_SAFETYCHECKER)
    {
        Stack_InitSafetyChecker((U8*)STACK_POINTER_START_ADDRESS, (U8*)STACK_POINTER_END_ADDRESS);
    }
    #endif

    CoreBuffer_Init();
    CoreQ_Init();
    CoreTerm_Init(NULL);
    CoreLog_Init(NULL);
    CoreTask_Init();

    CoreBuffer_Info();
    CoreQ_Info();
    CoreTask_Info();
    CoreTerm_RegisterCommand("CoreInfo", "CORE info", 0, Command_CoreInfo, FALSE);

    #if (INCLUDE_TESTMODE)
    {
        CoreTerm_RegisterCommand("TestMode", "Reset the board into testmode", 0, Core_ResetToTestMode, FALSE);
        if(system_signatures.testmode_signature == TESTMODE_SIGNATURE)
        {
            LOG_TRM("TESTMODE");
            core_state |= CORE_STATE_TEST;
            core_handles_ptr = &core_handles[1];
        }
        system_signatures.testmode_signature = 0;
    }
    #endif

    core_handles_ptr->init();

    core_state |= CORE_STATE_ACTIVE;

    CoreTask_StartAll();

    while(1)
    {
        System_KickDog();

        #if (USE_STACK_SAFETYCHECKER)
        {
            Stack_SafetyChecker();
        }
        #endif

        CoreBuffer_SafetyChecker();

        CoreLog_Handler();
        CoreTerm_Handler();
        CoreTask_Handler();

        core_handles_ptr->handler();
    }
}
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL Core_OnErrorReport(void)
{
    if((core_state & CORE_STATE_ERROR) > 0)
    {
        return FALSE;
    }
    core_state |= CORE_STATE_ERROR;
    CoreTask_StopAll();
    core_handles_ptr->on_error_report();
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_OnErrorHandler(void)
{
    while(1)
    {
        System_KickDog();
        CoreLog_Handler();
        CoreTerm_Handler();
    }
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_DebugSetSciChannel(SCI_CHANNEL_HNDL channel_hndl)
{
    CoreTerm_SwitchSciChannel(channel_hndl);
    CoreLog_SwitchSciChannel(channel_hndl);
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_CriticalEnter(void)
{
    core_disable_isr_count++;
    SysInt_DisableInterrupts();
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_CriticalExit(void)
{
    while(core_disable_isr_count == 0);
    core_disable_isr_count--;
    if(core_disable_isr_count == 0)
    {
        SysInt_EnableInterrupts();
    }
}
//------------------------------------------------------------------------------------------------//
void Core_ResetToTestMode(void)
{
    #if (INCLUDE_TESTMODE)
    {
        system_signatures.testmode_signature = TESTMODE_SIGNATURE;
        System_Reset();
    }
    #endif
}
//================================================================================================//
