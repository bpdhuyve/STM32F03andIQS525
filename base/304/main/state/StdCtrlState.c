//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to register and handle basic state machine
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STATE__STDCTRLSTATE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STATE__STDCTRLSTATE_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               STATE__STDCTRLSTATE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the control task period
#ifndef CONTROL_TASK_PERIOD_IN_MS
    #define CONTROL_TASK_PERIOD_IN_MS           1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of state machines that can be registered
#ifndef STATEMACHINE_COUNT
    #define STATEMACHINE_COUNT                  1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD
#include "StdCtrlState.h"
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
    const STDCTRLSTATE_CONFIG_STRUCT*   config_ptr;
    STDCTRLSTATE                        state;
    BOOL                                active_requested;
    U32                                 error_mask;
    U16                                 tick_divider;
    U16                                 tick_counter;
}
STDCTRLSTATE_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void StdCtrlState_EnterState(STDCTRLSTATE_CTRL_STRUCT* ctrl_ptr, STDCTRLSTATE state);
static void StdCtrlState_CtrlTask(VPTR data_ptr);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static STDCTRLSTATE_CTRL_STRUCT         stdctrlstate_ctrl_struct[STATEMACHINE_COUNT];
static U8                               stdctrlstate_count;

const STRING    stdctrlstate_names[] = {"INIT", "DISABLED", "ACTIVE", "ERROR"};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void StdCtrlState_EnterState(STDCTRLSTATE_CTRL_STRUCT* ctrl_ptr, STDCTRLSTATE state)
{
    ctrl_ptr->state = state;
    
    LOG_DBG("[%s] Enter %s", PCSTR(ctrl_ptr->config_ptr->module_name), PCSTR(stdctrlstate_names[state]));
    
    if(ctrl_ptr->config_ptr->on_state_enter_hook != NULL)
    {
        ctrl_ptr->config_ptr->on_state_enter_hook(state);
    }
}
//------------------------------------------------------------------------------------------------//
static void StdCtrlState_HandleState(STDCTRLSTATE_CTRL_STRUCT* ctrl_ptr)
{
    U32 bitmask = 1;
    
    // Update error mask
    while(bitmask)
    {
        if((ctrl_ptr->config_ptr->error_mask_check_hook != NULL) && (ctrl_ptr->config_ptr->error_mask_check_hook(bitmask)))
        {
            ctrl_ptr->error_mask |= bitmask;
        }
        else
        {
            ctrl_ptr->error_mask &= ~bitmask;
        }
        bitmask <<= 1;
    }
    
    // CHECK for init done
    if(ctrl_ptr->state == STDCTRLSTATE_INIT)
    {
        if((ctrl_ptr->config_ptr->init_done_check_hook == NULL) || (ctrl_ptr->config_ptr->init_done_check_hook() == TRUE))
        {
            StdCtrlState_EnterState(ctrl_ptr, STDCTRLSTATE_DISABLED);
        }
    }
    if(ctrl_ptr->state == STDCTRLSTATE_INIT)
    {
        ctrl_ptr->tick_counter = 0;
        return;
    }
    
    // CHECK for error
    if((ctrl_ptr->error_mask != 0) != (ctrl_ptr->state == STDCTRLSTATE_ERROR))
    {
        if(ctrl_ptr->error_mask != 0)
        {
            StdCtrlState_EnterState(ctrl_ptr, STDCTRLSTATE_ERROR);
        }
        else
        {
            StdCtrlState_EnterState(ctrl_ptr, STDCTRLSTATE_DISABLED);
        }
    }
    if(ctrl_ptr->state == STDCTRLSTATE_ERROR)
    {
        ctrl_ptr->tick_counter = 0;
        return;
    }
    
    // CHECK if disabled
    if((ctrl_ptr->active_requested == TRUE) != (ctrl_ptr->state == STDCTRLSTATE_ACTIVE))
    {
        if(ctrl_ptr->active_requested == TRUE)
        {
            StdCtrlState_EnterState(ctrl_ptr, STDCTRLSTATE_ACTIVE);
        }
        else
        {
            StdCtrlState_EnterState(ctrl_ptr, STDCTRLSTATE_DISABLED);
        }
    }
    if(ctrl_ptr->state == STDCTRLSTATE_DISABLED)
    {
        ctrl_ptr->tick_counter = 0;
        return;
    }
    
    // If enabled: PID control every eev_control_time_in_ms
    if((ctrl_ptr->tick_divider > 0) && (++(ctrl_ptr->tick_counter) >= ctrl_ptr->tick_divider))
    {
        ctrl_ptr->tick_counter = 0;
        if(ctrl_ptr->config_ptr->active_control_hook != NULL)
        {
            ctrl_ptr->config_ptr->active_control_hook();
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void StdCtrlState_CtrlTask(VPTR data_ptr)
{
    STDCTRLSTATE_CTRL_STRUCT*   ctrl_ptr = stdctrlstate_ctrl_struct;
    
    while(ctrl_ptr < &stdctrlstate_ctrl_struct[stdctrlstate_count])
    {
        StdCtrlState_HandleState(ctrl_ptr);
        ctrl_ptr++;
    }
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdCtrlState_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)stdctrlstate_ctrl_struct, 0, SIZEOF(stdctrlstate_ctrl_struct));
    stdctrlstate_count = 0;
    
    CoreTask_Start(CoreTask_RegisterTask(CONTROL_TASK_PERIOD_IN_MS * 1000, StdCtrlState_CtrlTask, NULL, 128, "StdCtrlState"));
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
STDCTRLSTATE_HNDL StdCtrlState_Register(const STDCTRLSTATE_CONFIG_STRUCT* config_ptr)
{
    U8  i;
    
    MODULE_CHECK();
    
    if(config_ptr == NULL)
    {
        return INVALID_STDCTRLSTATE_HNDL;
    }
    
    for(i = 0; i < stdctrlstate_count; i++)
    {
        if(stdctrlstate_ctrl_struct[i].config_ptr == config_ptr)
        {
            return (STDCTRLSTATE_HNDL)i;
        }
    }
    
    if(i < STATEMACHINE_COUNT)
    {
        stdctrlstate_ctrl_struct[i].config_ptr = config_ptr;
        stdctrlstate_count++;
        
        StdCtrlState_EnterState(&stdctrlstate_ctrl_struct[i], STDCTRLSTATE_INIT);
        return i;
    }
    
    LOG_ERR("Failed to register StdCtrlState %s", PCSTR(config_ptr->module_name));
    return INVALID_STDCTRLSTATE_HNDL;
}
//------------------------------------------------------------------------------------------------//
BOOL StdCtrlState_RequestActive(STDCTRLSTATE_HNDL hndl, BOOL request_active)
{
    if(hndl >= stdctrlstate_count) {return FALSE;}
    stdctrlstate_ctrl_struct[hndl].active_requested = request_active;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdCtrlState_SetTickDivider(STDCTRLSTATE_HNDL hndl, U16 tick_divider)
{
    if(hndl >= stdctrlstate_count) {return FALSE;}
    stdctrlstate_ctrl_struct[hndl].tick_divider = tick_divider;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdCtrlState_GetState(STDCTRLSTATE_HNDL hndl, STDCTRLSTATE* state_ptr)
{
    if(hndl >= stdctrlstate_count) {return FALSE;}
    *state_ptr = stdctrlstate_ctrl_struct[hndl].state;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdCtrlState_GetErrorMask(STDCTRLSTATE_HNDL hndl, U32* error_mask_ptr)
{
    if(hndl >= stdctrlstate_count) {return FALSE;}
    *error_mask_ptr = stdctrlstate_ctrl_struct[hndl].error_mask;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
void StdCtrlState_PrintStateAndError(STDCTRLSTATE_HNDL hndl, const STRING* error_mask_names)
{
    U8      i = 0;
    U32     error = stdctrlstate_ctrl_struct[hndl].error_mask;
    BOOL    add_hyphen = FALSE;
    
    if(hndl >= stdctrlstate_count) {return;}
    
    LOG_TRM(" - STATE                   : %s", PCSTR(stdctrlstate_names[stdctrlstate_ctrl_struct[hndl].state]));
    LOG(" - ERROR                   : ", LOG_LEVEL_TERM);
    if(error == 0)
    {
        LOG_TRM("NONE");
    }
    else
    {
        while(error)
        {
            if(error & 0x0001)
            {
                if(add_hyphen) {LOG(" - ", LOG_LEVEL_TERM);}
                add_hyphen = TRUE;
                LOG("%s", LOG_LEVEL_TERM, PCSTR(error_mask_names[i]));
            }
            error >>= 1;
            i++;
        }
        LOG_TRM("");
    }
}
//================================================================================================//
