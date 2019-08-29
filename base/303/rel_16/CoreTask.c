//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module for running repetitive tasks with a fixed period.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORETASK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORETASK_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CORETASK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the tick period of the task tick in µs
#ifndef TASK_TICK_IN_US
    #define TASK_TICK_IN_US             1000
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of tasks one wants to support
#ifndef TASK_COUNT
    #error "TASK_COUNT not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS lib include section
#include "core\SysTick.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DEFAULT_TASK_PERIOD                 10000

#define VALIDATE_AND_GET_TASK_PTR(task)     TASK_OBJECT* task_ptr = &coretask_array[(TASK_HNDL)(task - 1)];\
                                            if(((TASK_HNDL)(task - 1) >= TASK_COUNT) || (task_ptr->callback_func == NULL)) {LOG_ERR("illegal hndl"); return FALSE;}

//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U16                 priority           : 8;
    U16                 bitflag_asserted   : 1;
    U16                 bitflag_sleeping   : 1;
    U16                 bitflag_dummies    : 6;

    U32                 us_per_task_period;
    U32                 us_left;

    TASK_CALLBACK_FUNC  callback_func;
    VPTR                callback_data_ptr;

    VPTR                prev_task_ptr;
    VPTR                next_task_ptr;

#if (INCLUDE_INFO_STRING == 1)
    STRING              name;
#endif
}
TASK_OBJECT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void CoreTask_InitTask(TASK_OBJECT* task_ptr);
static void CoreTask_PlaceTaskIntoSortedChain(TASK_OBJECT* task_ptr);
static void CoreTask_Tick(void);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreTaskInfo(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static TASK_OBJECT              coretask_array[TASK_COUNT];
static TASK_OBJECT*             coretask_first_task_ptr;
static BOOL                     coretask_scheduler_active;
static U32                      coretask_tickcount;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CoreTask_InitTask(TASK_OBJECT* task_ptr)
{
    task_ptr->us_per_task_period    = DEFAULT_TASK_PERIOD;
    task_ptr->us_left               = DEFAULT_TASK_PERIOD;
    task_ptr->callback_func         = NULL;
    task_ptr->callback_data_ptr     = NULL;
    task_ptr->priority              = 128;
    task_ptr->bitflag_asserted      = 0;
    task_ptr->bitflag_sleeping      = 1;
}
//------------------------------------------------------------------------------------------------//
static void CoreTask_PlaceTaskIntoSortedChain(TASK_OBJECT* task_ptr)
{
    TASK_OBJECT* loper_task_ptr;

    if(coretask_first_task_ptr == NULL) /*** special geval ***/
    {
        task_ptr->prev_task_ptr = NULL;
        task_ptr->next_task_ptr = NULL;
    }
    else
    {
        loper_task_ptr = coretask_first_task_ptr;
        while(loper_task_ptr != NULL)
        {
            if(loper_task_ptr->priority > task_ptr->priority)  /*** loper has to become my next_task_ptr ***/
            {
                task_ptr->next_task_ptr = (VPTR)loper_task_ptr;
                task_ptr->prev_task_ptr = loper_task_ptr->prev_task_ptr;
                break;
            }
            else if(loper_task_ptr->next_task_ptr == NULL)  /*** loper has to become my previous_task ***/
            {
                task_ptr->next_task_ptr = NULL;
                task_ptr->prev_task_ptr = (VPTR)loper_task_ptr;
                break;
            }
            loper_task_ptr = (TASK_OBJECT*) loper_task_ptr->next_task_ptr;
        }
    }

    SysTick_DisableInterrupt();
    if(task_ptr->next_task_ptr != NULL)
    {
        ((TASK_OBJECT*)task_ptr->next_task_ptr)->prev_task_ptr = (VPTR)task_ptr;
    }
    if(task_ptr->prev_task_ptr != NULL)
    {
        ((TASK_OBJECT*)task_ptr->prev_task_ptr)->next_task_ptr = (VPTR)task_ptr;
    }
    else
    {
        coretask_first_task_ptr = task_ptr;
    }
    SysTick_EnableInterrupt();
}
//------------------------------------------------------------------------------------------------//
static void CoreTask_Tick(void)
{
    TASK_OBJECT* loper_task_ptr;
    TASK_OBJECT* next_loper_task_ptr; //usefull in case of task killed by its own task_callback_func;
    
    coretask_tickcount++;
    
    loper_task_ptr = coretask_first_task_ptr;
    while(loper_task_ptr != NULL)
    {
        next_loper_task_ptr = (TASK_OBJECT*) loper_task_ptr->next_task_ptr;
        if(loper_task_ptr->bitflag_sleeping == 0) /*** TASK is active ***/
        {
            if(loper_task_ptr->us_left > TASK_TICK_IN_US)
            {
                loper_task_ptr->us_left -= TASK_TICK_IN_US;
            }
            else /*** TASK is ASSERTED ***/
            {
                loper_task_ptr->us_left = loper_task_ptr->us_per_task_period;  //reset ticks to period
                if(loper_task_ptr->priority < 128)  /*** TASK handled on interrupt ***/
                {
                    loper_task_ptr->callback_func(loper_task_ptr->callback_data_ptr); //execute callback function
                }
                else  /*** TASK handled in background ***/
                {
                    loper_task_ptr->bitflag_asserted = 1; //set flag to notify background
                }
            }
        }
        loper_task_ptr = next_loper_task_ptr;
    }
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreTaskInfo(void)
{
    TASK_HNDL       task_hndl;
    TASK_OBJECT*    task_ptr;
    U8              task_count = 0;
    
    LOG_TRM("CoreTask");
    for(task_hndl = 1, task_ptr = coretask_array; task_hndl <= TASK_COUNT; task_hndl++, task_ptr++)
    {
        if(task_ptr->callback_func != NULL)
        {
            task_count++;
            #if (INCLUDE_INFO_STRING == 1)
            {
                LOG_TRM("%2d - %8d us - PRIO %d [%c] - %s",
                        PU8(task_hndl),
                        PU32(task_ptr->us_per_task_period),
                        PU8(task_ptr->priority),
                        PCHAR('A' + (('S' - 'A') * task_ptr->bitflag_sleeping)),
                        PCSTR(task_ptr->name));
            }
            #else
            {
                LOG_TRM("%2d - %8d us - PRIO %d [%c]",
                        PU8(task_hndl),
                        PU32(task_ptr->us_per_task_period),
                        PU8(task_ptr->priority),
                        PCHAR('A' + (('S' - 'A') * task_ptr->bitflag_sleeping)));
            }
            #endif
            CoreLog_Flush();
        }
    }
    LOG_TRM("Use: %d/%d", PU8(task_count), PU8(TASK_COUNT));
    LOG_TRM("Period: %d [us]", PU16(TASK_TICK_IN_US));
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CoreTask_Init(void)
{
    TASK_OBJECT* task_ptr;
    
    MODULE_INIT_ONCE();
    
    SysTick_Init(TASK_TICK_IN_US, CoreTask_Tick);
    
    coretask_scheduler_active   = FALSE;
    coretask_first_task_ptr     = NULL;
    coretask_tickcount          = 0;
    
    for(task_ptr = coretask_array; task_ptr < &coretask_array[TASK_COUNT]; task_ptr++)
    {
        CoreTask_InitTask(task_ptr);
    }
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CoreTask_Handler(void)
{
    TASK_OBJECT* loper_task_ptr;
    TASK_OBJECT* next_loper_task_ptr; //usefull in case of task killed by its own task_callback_func;
    
    loper_task_ptr = coretask_first_task_ptr;
    while(loper_task_ptr != NULL)
    {
        next_loper_task_ptr = (TASK_OBJECT*) loper_task_ptr->next_task_ptr;
        if((loper_task_ptr->bitflag_sleeping == 0) && (loper_task_ptr->bitflag_asserted == 1))
        {
            /*** TASK is active & asserted ***/
            loper_task_ptr->bitflag_asserted = 0;
            loper_task_ptr->callback_func(loper_task_ptr->callback_data_ptr); //execute callback function
        }
        loper_task_ptr = next_loper_task_ptr;
    }
}
//------------------------------------------------------------------------------------------------//
TASK_HNDL CoreTask_RegisterTask(U32 task_period_in_us, TASK_CALLBACK_FUNC callback, VPTR data_ptr, U8 priority, STRING name)
{
    TASK_HNDL       task_hndl;
    TASK_OBJECT*    task_ptr;
    
    MODULE_CHECK();
    
    // CHECK IF LEGAL
    if(callback == NULL)
    {
        LOG_ERR("TASK callback is NULL");
        return INVALID_TASK_HNDL;
    }
    
    // CHECK IF CAN BE PLACED
    for(task_hndl = 1, task_ptr = coretask_array; task_hndl <= TASK_COUNT; task_hndl++, task_ptr++)
    {
        if(task_ptr->callback_func == NULL)
        {
            break;
        }
    }
    if(task_hndl > TASK_COUNT)
    {
        LOG_ERR("TASK no empty task found");
        return INVALID_TASK_HNDL;
    }
    
    // VERIFY PERIOD
    if((task_period_in_us % TASK_TICK_IN_US) > 0)
    {
        LOG_WRN("TASK period of task %s no tickcount multiple", PCSTR(name));
    }
    
    // REGISTER
    task_ptr->us_per_task_period    = task_period_in_us;
    task_ptr->us_left               = task_period_in_us;
    task_ptr->callback_func         = callback;
    task_ptr->callback_data_ptr     = data_ptr;
    
    task_ptr->priority              = priority;
    task_ptr->bitflag_asserted      = 0;
    task_ptr->bitflag_sleeping      = 1; /*** user must explicitly START the task ***/
    
    #if (INCLUDE_INFO_STRING == 1)
    {
        task_ptr->name              = name;
    }
    #endif
    
    CoreTask_PlaceTaskIntoSortedChain(task_ptr);
    
    return task_hndl;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_SetPeriod(TASK_HNDL task, U32 task_period_in_us)
{
    VALIDATE_AND_GET_TASK_PTR(task);
    
    #if (INCLUDE_INFO_STRING == 1)
    {
        if((task_period_in_us % TASK_TICK_IN_US) > 0)
        {
            LOG_WRN("TASK period of task %s no tickcount multiple", PCSTR(task_ptr->name));
        }
    }
    #endif
    
    task_ptr->us_per_task_period = task_period_in_us;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_GetPeriod(TASK_HNDL task, U32* task_period_in_us_ptr)
{
    VALIDATE_AND_GET_TASK_PTR(task);
    
    *task_period_in_us_ptr = task_ptr->us_per_task_period;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_SetDataPtr(TASK_HNDL task, VPTR data_ptr)
{
    VALIDATE_AND_GET_TASK_PTR(task);
    
    task_ptr->callback_data_ptr = data_ptr;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_Start(TASK_HNDL task)
{
    VALIDATE_AND_GET_TASK_PTR(task);
    
    task_ptr->us_left = task_ptr->us_per_task_period;  //reset ticks to period
    task_ptr->bitflag_asserted = 0;
    task_ptr->bitflag_sleeping = 0;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_Stop(TASK_HNDL task)
{
    VALIDATE_AND_GET_TASK_PTR(task);
    
    task_ptr->bitflag_sleeping = 1;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_Kill(TASK_HNDL task)
{
    VALIDATE_AND_GET_TASK_PTR(task);
    
    SysTick_DisableInterrupt();
    if(task_ptr->prev_task_ptr != NULL)
    {
        ((TASK_OBJECT*)task_ptr->prev_task_ptr)->next_task_ptr = task_ptr->next_task_ptr;
    }
    else
    {
        coretask_first_task_ptr = (TASK_OBJECT*)task_ptr->next_task_ptr;
    }
    if(task_ptr->next_task_ptr != NULL)
    {
        ((TASK_OBJECT*)task_ptr->next_task_ptr)->prev_task_ptr = task_ptr->prev_task_ptr;
    }
    SysTick_EnableInterrupt();
    
    CoreTask_InitTask(task_ptr);
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_IsTaskRunning(TASK_HNDL task)
{
    VALIDATE_AND_GET_TASK_PTR(task);
    
    return (BOOL)(task_ptr->bitflag_sleeping == 0);
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_StartAll(void)
{
    if((coretask_first_task_ptr != NULL) && SysTick_EnableInterrupt() && SysTick_Start())
    {
        coretask_scheduler_active = TRUE;
    }
    return coretask_scheduler_active;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_StopAll(void)
{
    if(SysTick_DisableInterrupt() && SysTick_Stop())
    {
        coretask_scheduler_active = FALSE;
    }
    return (BOOL)(coretask_scheduler_active == FALSE);
}
//------------------------------------------------------------------------------------------------//
void CoreTask_Info(void)
{
    CoreTerm_RegisterCommand("CoreTaskInfo", "CORE task info", 0, Command_CoreTaskInfo, FALSE);
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTask_ReStart(TASK_HNDL task)
{
    CoreTask_Stop(task);
    return CoreTask_Start(task);
}
//------------------------------------------------------------------------------------------------//
U32 CoreTask_GetTickCount(void)
{
    return coretask_tickcount;
}
//================================================================================================//
