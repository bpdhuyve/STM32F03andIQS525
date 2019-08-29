//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Main Core module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//------------------------------------------------------------------------------------------------//
#define COREFREERTOS_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COREFREERTOS_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COREFREERTOS_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines if the watchdog must be enabled, WD will be enabled if PRODUCT_TEST_NUMBER == 0
#ifndef WATCHDOG_ENABLED
    #if ((PRODUCT_VERSION_NUMBER != 0) && (PRODUCT_TEST_NUMBER == 0))
        #define WATCHDOG_ENABLED            1
    #else
        #define WATCHDOG_ENABLED            0
    #endif
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines the watchdog timeout in ms
#ifndef WATCHDOG_TIMEOUT_IN_MS
    #define WATCHDOG_TIMEOUT_IN_MS          WD_MIN_OVERFLOW_TIME_DEFAULT
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines if the number of freeRTOS tasks to be registered
#ifndef FREERTOS_TASK_COUNT
    #define FREERTOS_TASK_COUNT             10
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines if testmode must be included (1) or not (0)
#ifndef INCLUDE_TESTMODE
    #define INCLUDE_TESTMODE                1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "CoreFreeRtos.h"

// SYS
#include "gpio\SysPin.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define TESTMODE_SIGNATURE            0x7E5730DE

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
static void CoreFreeRtos_Hwm(void);
#endif

void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );

static void TermTask( void *pvParameters );
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
extern void AppMain_Init(void);
extern void AppMain_Handler(void);
extern void AppMain_OnErrorReport(void);

#if (INCLUDE_TESTMODE > 0)
extern void AppTest_Init(void);
extern void AppTest_Handler(void);
extern void AppTest_OnErrorReport(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static U8                       core_state = 0;
static xTaskHandle              core_freertos_tasks[FREERTOS_TASK_COUNT];
static U8                       core_freertos_task_count;

#if (INCLUDE_TESTMODE > 0)
static const CORE_HANDLES       core_handles[2] = {{AppMain_Init, AppMain_Handler, AppMain_OnErrorReport},
                                                   {AppTest_Init, AppTest_Handler, AppTest_OnErrorReport}};
#else
static const CORE_HANDLES       core_handles[1] = {{AppMain_Init, AppMain_Handler, AppMain_OnErrorReport}};
#endif
static const CORE_HANDLES*      core_handles_ptr = core_handles;
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
//------------------------------------------------------------------------------------------------//
static void CoreFreeRtos_Hwm(void)
{
    U8  i;
    if(core_freertos_tasks[core_freertos_task_count-1] != xTaskGetIdleTaskHandle())
    {
        CoreFreeRtos_Register(xTaskGetIdleTaskHandle());
    }
    for(i = 0; i < core_freertos_task_count; i++)
    {
        if(core_freertos_tasks[i] != NULL)
        {
            LOG_TRM("%2d - %-10s - %d [%d]",
                    PU8(i),
                    PCSTR(pcTaskGetTaskName(core_freertos_tasks[i])),
                    PU16(uxTaskGetStackHighWaterMark(core_freertos_tasks[i])),
                    PU8(uxTaskPriorityGet(core_freertos_tasks[i])));
        }
    }
    LOG_TRM("Use : %d/%d", PU8(core_freertos_task_count), PU8(FREERTOS_TASK_COUNT));
    LOG_TRM("Free Heap Size : %d", PU16(xPortGetFreeHeapSize()));
}
#endif
//------------------------------------------------------------------------------------------------//
void vApplicationIdleHook(void)
{
    System_KickDog();
    
    CoreLog_Handler();
    CoreTask_Handler();
    CoreBuffer_SafetyChecker();
    
    core_handles_ptr->handler();
}
//------------------------------------------------------------------------------------------------//
void vApplicationTickHook(void)
{
    
}
//------------------------------------------------------------------------------------------------//
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    LOG_TRM("Stack of Task %s overflowed", PCSTR(pcTaskName));
}
//------------------------------------------------------------------------------------------------//
static void TermTask( void *pvParameters )
{
    while(1)
	{
        CoreTerm_Handler();
        vTaskDelay(1);
	}
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @remark  none
int main(void)
{
    xTaskHandle task = NULL;
    
#if WATCHDOG_ENABLED
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
    
    CoreBuffer_Init();
    CoreQ_Init();
    CoreTerm_Init(NULL);
    CoreLog_Init(NULL);
    CoreTask_Init();

    CoreBuffer_Info();
    CoreQ_Info();
    CoreTask_Info();
    CoreTerm_RegisterCommand("CoreInfo", "CORE info", 0, Command_CoreInfo, FALSE);
    
    CoreTerm_RegisterCommand("OsHwm", "OS HWM", 0, CoreFreeRtos_Hwm, FALSE);
    
    MEMSET((VPTR)core_freertos_tasks, 0, SIZEOF(core_freertos_tasks));
    core_freertos_task_count = 0;
    
#if (INCLUDE_TESTMODE > 0)
    CoreTerm_RegisterCommand("TestMode", "Reset the board into testmode", 0, Core_ResetToTestMode, FALSE);
    if(system_signatures.testmode_signature == TESTMODE_SIGNATURE)
    {
        LOG_TRM("TESTMODE");
        core_state |= CORE_STATE_TEST;
        core_handles_ptr = &core_handles[1];
    }
    system_signatures.testmode_signature = 0;
#endif
    
    core_handles_ptr->init();
    
    xTaskCreate(TermTask, (signed portCHAR*)"TERM", configMINIMAL_STACK_SIZE, NULL, 3, &task);
    CoreFreeRtos_Register(task);
    
    core_state |= CORE_STATE_ACTIVE;
    
    CoreTask_StartAll();
    
    vTaskStartScheduler();
    
    // we only get here when vTaskEndScheduler() was called
    while(1)
    {
        System_KickDog();
        CoreLog_Handler();
        CoreTerm_Handler();
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
    vTaskEndScheduler();
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
    vPortEnterCritical();
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_CriticalExit(void)
{
    vPortExitCritical();
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreFreeRtos_Register(xTaskHandle task)
{
    if(core_freertos_task_count < FREERTOS_TASK_COUNT)
    {
        core_freertos_tasks[core_freertos_task_count++] = task;
    }
}
//------------------------------------------------------------------------------------------------//
void Core_ResetToTestMode(void)
{
#if (INCLUDE_TESTMODE > 0)
    system_signatures.testmode_signature = TESTMODE_SIGNATURE;
    System_Reset();
#endif
}
//================================================================================================//
