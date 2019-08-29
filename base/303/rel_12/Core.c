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
// @remark  Defines if the watchdog must be enabled
#ifndef WATCHDOG_ENABLED
    #error "WATCHDOG_ENABLED not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines the watchdog timeout in ms
#ifndef WATCHDOG_TIMEOUT_IN_MS
    #define WATCHDOG_TIMEOUT_IN_MS                          WD_MIN_OVERFLOW_TIME_DEFAULT
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines which modes are available: APPMAIN (0), APPTEST & APPMAIN (1) and APPTEST only (2)
#ifndef INCLUDE_TESTMODE
    #define INCLUDE_TESTMODE                                1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef INCLUDE_BOOTMODE
    #define INCLUDE_BOOTMODE                                0
#endif
//------------------------------------------------------------------------------------------------//
#ifndef USE_STACK_SAFETYCHECKER
    #define USE_STACK_SAFETYCHECKER                         0
#endif
//------------------------------------------------------------------------------------------------//
#if (USE_STACK_SAFETYCHECKER != 0)
    #ifndef STACK_SAFETY_BYTE
        #define STACK_SAFETY_BYTE                           0x63
    #endif
    #ifndef STACK_SAFETY_SIZE
        #define STACK_SAFETY_SIZE                           32
    #endif
    #ifndef STACK_POINTER_START_ADDRESS
        #error "STACK_POINTER_START_ADDRESS not defined in AppConfig.h"
    #endif
    #ifndef STACK_POINTER_END_ADDRESS
        #error "STACK_POINTER_END_ADDRESS not defined in AppConfig.h"
    #endif
#endif
//------------------------------------------------------------------------------------------------//
#ifndef USE_GPIO_PIN_HNDL_TO_DEBUG_CRITICAL_SECTION
    #define USE_GPIO_PIN_HNDL_TO_DEBUG_CRITICAL_SECTION     0
#endif
//------------------------------------------------------------------------------------------------//
#if (USE_GPIO_PIN_HNDL_TO_DEBUG_CRITICAL_SECTION != 0)
    #ifndef DEBUG_CRITICAL_SECTION_PORT_NUMBER
        #error "DEBUG_CRITICAL_SECTION_PORT_NUMBER not defined in AppConfig.h"
    #endif
    #ifndef DEBUG_CRITICAL_SECTION_PIN_NUMBER
        #error "DEBUG_CRITICAL_SECTION_PIN_NUMBER not defined in AppConfig.h"
    #endif
    #define DEBUG_CRITICAL_SECTION_PIN_MASK                 (((U32)(0x01)) << DEBUG_CRITICAL_SECTION_PIN_NUMBER)
#endif
//------------------------------------------------------------------------------------------------//
#ifdef USE_FREERTOS
    // @remark  Defines if the number of freeRTOS tasks to be registered
    #ifndef FREERTOS_TASK_COUNT
        #define FREERTOS_TASK_COUNT                         10
    #endif
    #ifndef COREFREERTOS_TERM_MINIMAL_STACK_SIZE
        #define COREFREERTOS_TERM_MINIMAL_STACK_SIZE        configMINIMAL_STACK_SIZE
    #endif
    #ifndef MAX_NESTED_CRITICAL_SECTIONS
        #define MAX_NESTED_CRITICAL_SECTIONS                10
    #endif
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines if a lighter version of Core is used (no CoreTask, less strings)
#ifndef LIGHT_MODE
    #define LIGHT_MODE                                      0
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines if software needs to reset on error
#ifndef RESET_ON_ERROR
    #define RESET_ON_ERROR                                  0
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines the number of module handler that can be registered
#ifndef MODULE_HANDLER_COUNT
    #define MODULE_HANDLER_COUNT                            10
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "gpio\SysPin.h"
#include "gpio\SysGpio.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    EVENT_CALLBACK  init;
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
static void Core_ErrorBackground(void);

static void Core_InitModuleHandlers(void);
static void Core_CallModuleHandlers(void);

#if (USE_STACK_SAFETYCHECKER)
static void Stack_InitSafetyChecker(void);
static void Stack_SafetyChecker(void);
#endif

#ifdef USE_FREERTOS
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void CoreFreeRtos_Hwm(void);
#endif

void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );

static void TermTask( void *pvParameters );
#endif
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @remark  function called during initialisation of application
extern void AppMain_Init(void);
// @remark  function called in background of application
extern void AppMain_Handler(void);
// @remark  function called on error in application
extern void AppMain_OnErrorReport(void);

#if (INCLUDE_TESTMODE)
// @remark  function called during initialisation of testmode
extern void AppTest_Init(void);
// @remark  function called in background of testmode
extern void AppTest_Handler(void);
// @remark  function called on error in testmode
extern void AppTest_OnErrorReport(void);
// @remark  returns TRUE if application must enter testmode at startup (e.g. based on testpin)
extern BOOL AppTest_CheckEnterTestmode(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static U8                       core_state;

static U16                      core_disable_isr_count;

#ifdef USE_FREERTOS
static xTaskHandle              core_freertos_tasks[FREERTOS_TASK_COUNT];
static U8                       core_freertos_task_count;
#endif

#if (INCLUDE_TESTMODE == 1)
static const CORE_HANDLES       core_handles[2] = {{AppMain_Init, AppMain_OnErrorReport},
                                                   {AppTest_Init, AppTest_OnErrorReport}};
#elif (INCLUDE_TESTMODE == 2)
static const CORE_HANDLES       core_handles[1] = {{AppTest_Init, AppTest_OnErrorReport}};
#else
static const CORE_HANDLES       core_handles[1] = {{AppMain_Init, AppMain_OnErrorReport}};
#endif

static const CORE_HANDLES*      core_handles_ptr;

#ifdef USE_FREERTOS
static unsigned portBASE_TYPE   saved_interrupt_status_stack[MAX_NESTED_CRITICAL_SECTIONS];
static U16                      core_disable_isr_count_high_water_mark;
#endif

static EVENT_CALLBACK           core_module_handlers[MODULE_HANDLER_COUNT];
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
    
    #if (LIGHT_MODE == 0)
    {
        LOG_TRM("Core");
        LOG_TRM("- state     : %s %s %s", PCSTR(mode[(core_state & CORE_STATE_TEST) > 0]),
                                          PCSTR(state[(core_state & CORE_STATE_ACTIVE) > 0]),
                                          PCSTR(error[(core_state & CORE_STATE_ERROR) > 0]));
        #ifdef ENABLE_WATCHDOG_PROFILING
        {
            LOG_TRM("- watchdog  : min %d - avg %d", PU32(watchdog_minimum_time_left), PU32(watchdog_average_time_left));
        }
        #endif
    }
    #else
    {
        LOG_TRM("%s %s %s", PCSTR(mode[(core_state & CORE_STATE_TEST) > 0]),
                            PCSTR(state[(core_state & CORE_STATE_ACTIVE) > 0]),
                            PCSTR(error[(core_state & CORE_STATE_ERROR) > 0]));
    }
    #endif
    
    #if (USE_STACK_SAFETYCHECKER)
    {
        U8*     check_ptr = (U8*)STACK_POINTER_END_ADDRESS;
        
        #if (STACK_POINTER_START_ADDRESS < STACK_POINTER_END_ADDRESS)
        {
            while(*check_ptr == STACK_SAFETY_BYTE)
            {
                check_ptr--;
            }
            #if (LIGHT_MODE == 0)
            {
                LOG_TRM("- stack hwm : %d/%d", PU32((U32)check_ptr - STACK_POINTER_START_ADDRESS), PU32(STACK_POINTER_END_ADDRESS - STACK_POINTER_START_ADDRESS));
            }
            #else
            {
                LOG_TRM("HWM %d/%d", PU32((U32)check_ptr - STACK_POINTER_START_ADDRESS), PU32(STACK_POINTER_END_ADDRESS - STACK_POINTER_START_ADDRESS));
            }
            #endif
        }
        #else
        {
            while(*check_ptr == STACK_SAFETY_BYTE)
            {
                check_ptr++;
            }
            #if (LIGHT_MODE == 0)
            {
                LOG_TRM("- stack hwm : %d/%d", PU32(STACK_POINTER_START_ADDRESS - (U32)check_ptr), PU32(STACK_POINTER_START_ADDRESS - STACK_POINTER_END_ADDRESS));
            }
            #else
            {
                LOG_TRM("HWM %d/%d", PU32(STACK_POINTER_START_ADDRESS - (U32)check_ptr), PU32(STACK_POINTER_START_ADDRESS - STACK_POINTER_END_ADDRESS));
            }
            #endif
        }
        #endif
    }
    #endif
}
#endif
//------------------------------------------------------------------------------------------------//
static void Core_ErrorBackground(void)
{
    #if (RESET_ON_ERROR)
    {
        Core_Reset();
    }
    #else
    {
        while(1)
        {
            System_KickDog();
            CoreLog_Handler();
            CoreTerm_Handler();
        }
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
static void Core_InitModuleHandlers(void)
{
    MEMSET((VPTR)core_module_handlers, 0, SIZEOF(core_module_handlers));
    
#if (INCLUDE_TESTMODE == 1)
    if((core_state & CORE_STATE_TEST) > 0)  {core_module_handlers[0] = AppTest_Handler;}
    else                                    {core_module_handlers[0] = AppMain_Handler;}
#elif (INCLUDE_TESTMODE == 2)
    core_module_handlers[0] = AppTest_Handler;
#else
    core_module_handlers[0] = AppMain_Handler;
#endif
}
//------------------------------------------------------------------------------------------------//
static void Core_CallModuleHandlers(void)
{
    EVENT_CALLBACK* callback_ptr = core_module_handlers;
    
    do
    {
        // check if empty
        if(*callback_ptr == NULL)
        {
            return;
        }
        // else call
        (*callback_ptr)();
        // increment
        callback_ptr++;
    }
    while(callback_ptr < &core_module_handlers[MODULE_HANDLER_COUNT]);
}
//------------------------------------------------------------------------------------------------//
// SAFETYCHECKER
//------------------------------------------------------------------------------------------------//
#if (USE_STACK_SAFETYCHECKER)
#pragma optimize=none
static void Stack_InitSafetyChecker(void)
{
    U8  address = 0;
    
    #if (STACK_POINTER_START_ADDRESS < STACK_POINTER_END_ADDRESS)
    {
        MEMSET((VPTR)((U32)&address + 10), STACK_SAFETY_BYTE, STACK_POINTER_END_ADDRESS - ((U32)&address + 10));   //fill last part with safety bytes
    }
    #else
    {
        MEMSET((VPTR)STACK_POINTER_END_ADDRESS, STACK_SAFETY_BYTE, ((U32)&address - 10) - STACK_POINTER_END_ADDRESS);   //fill last part with safety bytes
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
static void Stack_SafetyChecker(void)
{
    U8  i = STACK_SAFETY_SIZE;
    U8* check_ptr;
    
    #if (STACK_POINTER_START_ADDRESS < STACK_POINTER_END_ADDRESS)
    {
        check_ptr = (U8*)(STACK_POINTER_END_ADDRESS - STACK_SAFETY_SIZE);
    }
    #else
    {
        check_ptr = (U8*)(STACK_POINTER_END_ADDRESS);
    }
    #endif
    
    while(i > 0)
    {
        if(*check_ptr != STACK_SAFETY_BYTE)
        {
            LOG_ERR("STACK_OVERFLOW");
        }
        i--;
        check_ptr++;
    }
}
#endif
//------------------------------------------------------------------------------------------------//
// FREERTOS
//------------------------------------------------------------------------------------------------//
#ifdef USE_FREERTOS
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void CoreFreeRtos_Hwm(void)
{
    static BOOL idle_task_registered = FALSE;
    U8          i;
    
    if(idle_task_registered == FALSE)
    {
        CoreFreeRtos_Register(xTaskGetIdleTaskHandle());
        idle_task_registered = TRUE;
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
            CoreLog_Flush();
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
    
    #if (USE_STACK_SAFETYCHECKER)
    {
        Stack_SafetyChecker();
    }
    #endif
    
    CoreBuffer_SafetyChecker();
    CoreLog_Handler();
    #if (LIGHT_MODE == 0)
    {
        CoreTask_Handler();
    }
    #endif
    
    Core_CallModuleHandlers();
}
//------------------------------------------------------------------------------------------------//
void vApplicationTickHook(void)
{
    
}
//------------------------------------------------------------------------------------------------//
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    LOG_ERR("Stack of Task %s overflowed", PCSTR(pcTaskName));
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
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @remark  none
int main(void)
{
#ifdef USE_FREERTOS
    xTaskHandle task = NULL;
#endif
    
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
    core_state              = 0;
    core_disable_isr_count  = 0;
    core_handles_ptr        = core_handles;
    
    SysInt_Init();
    SysPin_Init();
    #if (USE_GPIO_PIN_HNDL_TO_DEBUG_CRITICAL_SECTION != 0)
    {
        SysPin_InitPinMask((GPIO_PORT)DEBUG_CRITICAL_SECTION_PORT_NUMBER,
                           DEBUG_CRITICAL_SECTION_PIN_MASK,
                           PIN_OUTPUT_PUSH_PULL_DRV_WEAK);
        SysGpio_SetPinMask((GPIO_PORT)DEBUG_CRITICAL_SECTION_PORT_NUMBER,
                           DEBUG_CRITICAL_SECTION_PIN_MASK,
                           FALSE);
    }
    #endif
    
    #if (USE_STACK_SAFETYCHECKER)
    {
        Stack_InitSafetyChecker();
    }
    #endif
    
    CoreBuffer_Init();
    CoreQ_Init();
    CoreTerm_Init(NULL);
    CoreLog_Init(NULL);
    #if (LIGHT_MODE == 0)
    {
        CoreTask_Init();
    }
    #endif
    
    CoreBuffer_Info();
    CoreQ_Info();
    #if (LIGHT_MODE == 0)
    {
        CoreTask_Info();
    }
    #endif
    CoreTerm_RegisterCommand("CoreInfo", "CORE info", 0, Command_CoreInfo, FALSE);
    
    #ifdef USE_FREERTOS
    {
        CoreTerm_RegisterCommand("OsHwm", "OS HWM", 0, CoreFreeRtos_Hwm, FALSE);
        
        MEMSET((VPTR)core_freertos_tasks, 0, SIZEOF(core_freertos_tasks));
        core_freertos_task_count = 0;
        
        xTaskCreate(TermTask, (const portCHAR*)"TERM", COREFREERTOS_TERM_MINIMAL_STACK_SIZE, NULL, 3, &task);
        CoreFreeRtos_Register(task);
    }
    #endif
    
    #if (INCLUDE_TESTMODE == 1)
    {
        CoreTerm_RegisterCommand("TestMode", "Reset the board into testmode", 0, Core_ResetToTestMode, FALSE);
        if((system_signatures.testmode_signature == TESTMODE_SIGNATURE) || AppTest_CheckEnterTestmode())
        {
            LOG_TRM("TESTMODE");
            core_state |= CORE_STATE_TEST;
            core_handles_ptr = &core_handles[1];
        }
        system_signatures.testmode_signature = 0;
    }
    #elif (INCLUDE_TESTMODE == 2)
    {
        LOG_TRM("TESTMODE");
        core_state |= CORE_STATE_TEST;
    }
    #endif
    
    #if (INCLUDE_BOOTMODE == 1)
    {
        CoreTerm_RegisterCommand("BootMode", "Reset the board into bootmode", 0, Core_ResetToBoot, FALSE);
    }
    #endif
    
    Core_InitModuleHandlers();
    
    core_handles_ptr->init();
    
    core_state |= CORE_STATE_ACTIVE;
    
    #if (LIGHT_MODE == 0)
    {
        CoreTask_StartAll();
    }
    #endif
    
    #ifdef USE_FREERTOS
    {
        vTaskStartScheduler();
        
        // we only get here when vTaskEndScheduler() was called
        LOG_ERR("Dropped out of FreeRTOS");
        Core_ErrorBackground();
    }
    #else
    {
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
            #if (LIGHT_MODE == 0)
            {
                CoreTask_Handler();
            }
            #endif
            
            Core_CallModuleHandlers();
        }
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
// @remark  register background handler to be called in active mode
BOOL Core_RegisterModuleHandler(EVENT_CALLBACK module_handler)
{
    EVENT_CALLBACK* callback_ptr = core_module_handlers;
    
    // loop to find first free spot
    do
    {
        // check if already in
        if(*callback_ptr == module_handler)
        {
            return TRUE;
        }
        // check if empty spot
        if(*callback_ptr == NULL)
        {
            *callback_ptr = module_handler;
            return TRUE;
        }
        // increment
        callback_ptr++;
    }
    while(callback_ptr < &core_module_handlers[MODULE_HANDLER_COUNT]);
    
    LOG_ERR("Cannot register module handler");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// @remark  returns if this is the first error
BOOL Core_OnErrorReport(void)
{
    // if already in ERROR: do nothing
    if((core_state & CORE_STATE_ERROR) > 0)
    {
        // return FALSE to indicate no jump to Core_OnErrorHandler is needed
        return FALSE;
    }
    
    // enable watchdog (for the case the application runs without watchdog)
    System_EnableWatchdog(WATCHDOG_TIMEOUT_IN_MS);
    System_KickDog();
    
    // set state to ERROR
    core_state |= CORE_STATE_ERROR;
    #if (LIGHT_MODE == 0)
    {
        CoreTask_StopAll();
    }
    #endif
    // call on error report handle
    core_handles_ptr->on_error_report();
    
    // return TRUE to indicate jump to Core_OnErrorHandler is needed
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark  should only be called once (on first error)
void Core_OnErrorHandler(void)
{
    #ifdef USE_FREERTOS
    {
        if((core_state & CORE_STATE_ACTIVE) == CORE_STATE_ACTIVE)
        {
             vTaskEndScheduler();   // will invoke drop out of FreeRTOS
             return;
        }
    }
    #endif
    
    Core_ErrorBackground();
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_DebugSetSciChannel(SCI_CHANNEL_HNDL channel_hndl)
{
    CoreTerm_SwitchSciChannel(channel_hndl);
    CoreLog_SwitchSciChannel(channel_hndl);
}
//------------------------------------------------------------------------------------------------//
// @remark  If USE_FREERTOS is defined, Core_CriticalEnter() may never be called from an IRQ with
//          a higher priority than configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (FreeRTOSConfig.h)
void Core_CriticalEnter(void)
{    
    #ifdef USE_FREERTOS
    {
        if(core_disable_isr_count >= MAX_NESTED_CRITICAL_SECTIONS)
        {
            LOG_ERR("Maximum allowed nested critical sections reached");
        }
        // portSET_INTERRUPT_MASK_FROM_ISR() is used instead of taskENTER_CRITICAL_FROM_ISR() 
        // for backwards compatibility with FreeRTOS versions before 9.0.0
        saved_interrupt_status_stack[core_disable_isr_count] = portSET_INTERRUPT_MASK_FROM_ISR();
    }
    #else
    {
        SysInt_DisableInterrupts();
    }
    #endif
    
    core_disable_isr_count++;
    #ifdef USE_FREERTOS
    if(core_disable_isr_count > core_disable_isr_count_high_water_mark)
    {
        core_disable_isr_count_high_water_mark = core_disable_isr_count;
    }
    #endif    
    
    #if (USE_GPIO_PIN_HNDL_TO_DEBUG_CRITICAL_SECTION != 0)
    //first disable isr's, then toggle debug pin !!!
    //critical sections from background would appear longer than they are by possible ISR's that sneak in!
    {
        if(core_disable_isr_count == 1)
        {
            SysGpio_SetPinMask((GPIO_PORT)DEBUG_CRITICAL_SECTION_PORT_NUMBER,
                               DEBUG_CRITICAL_SECTION_PIN_MASK,
                               TRUE);
        }
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
// @remark  none
void Core_CriticalExit(void)
{
    while(core_disable_isr_count == 0); //FDNR: i'm not sure that this is the best way to deal with it in Pltfrm 2 : an LOG_ERR() might be better
    core_disable_isr_count--;
    if(core_disable_isr_count == 0)
    {
        #if (USE_GPIO_PIN_HNDL_TO_DEBUG_CRITICAL_SECTION != 0)
        {
            SysGpio_SetPinMask((GPIO_PORT)DEBUG_CRITICAL_SECTION_PORT_NUMBER,
                               DEBUG_CRITICAL_SECTION_PIN_MASK,
                               FALSE);
        }
        //first toggle debug pin, then enable isr's !!!
        //critical sections from background would appear longer than they are by the immediate ISR which gets high prio !!
        #endif
        
        #ifndef USE_FREERTOS
        {
            SysInt_EnableInterrupts();
        }
        #endif
    }
    
    #ifdef USE_FREERTOS
    {
        // portCLEAR_INTERRUPT_MASK_FROM_ISR() is used instead of taskEXIT_CRITICAL_FROM_ISR() 
        // for backwards compatibility with FreeRTOS versions before 9.0.0
        portCLEAR_INTERRUPT_MASK_FROM_ISR(saved_interrupt_status_stack[core_disable_isr_count]);
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
void Core_Reset(void)
{
    System_EnableWatchdog(10);
    CoreLog_Flush();
    System_Reset();
    for(;;){}
}
//------------------------------------------------------------------------------------------------//
void Core_ResetToBoot(void)
{
    system_signatures.reset_to_boot_signature = BOOTMODE_SIGNATURE;
    Core_Reset();
}
//------------------------------------------------------------------------------------------------//
void Core_ResetToTestMode(void)
{
    #if (INCLUDE_TESTMODE)
    {
        system_signatures.testmode_signature = TESTMODE_SIGNATURE;
        Core_Reset();
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
U8 Core_GetCoreState(void)
{
    return core_state;
}
//------------------------------------------------------------------------------------------------//
#ifdef USE_FREERTOS
// @remark  none
void CoreFreeRtos_Register(xTaskHandle task)
{
    if(core_freertos_task_count < FREERTOS_TASK_COUNT)
    {
        core_freertos_tasks[core_freertos_task_count++] = task;
    }
}
#endif
//================================================================================================//
