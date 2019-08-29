//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Generic implementation of state machine
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STDSTATEMACHINE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDSTATEMACHINE_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               STDSTATEMACHINE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STATEMACHINE_COUNT
    #define STATEMACHINE_COUNT          1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// STD
#include "state\StdStateMachine.h"
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
    const STD_STATE_MACHINE_STRUCT*     state_struct_ptr;
    STRING                              module_name;
    U8                                  number_of_states;
    U8                                  active_state;
}
CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void BackgroundHandler(void);
static BOOL EnterState(STD_STATE_MACHINE_HNDL hndl, U8 next_state);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static CTRL_STRUCT                      std_state_machine_ctrl_struct[STATEMACHINE_COUNT];
static U8                               std_state_machine_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void BackgroundHandler(void)
{
    static CTRL_STRUCT* state_machine_ptr = std_state_machine_ctrl_struct;
    STD_STATE_MACHINE_STRUCT* state_ptr;
    
    if(std_state_machine_count == 0)
    {
        return;
    }
    
    // Handle current state within state machine
    state_ptr = (STD_STATE_MACHINE_STRUCT*)&state_machine_ptr->state_struct_ptr[state_machine_ptr->active_state];
    
    if(state_machine_ptr->active_state >= state_machine_ptr->number_of_states)
    {
        LOG_WRN("Invalid state %d", PU8(state_machine_ptr->active_state));
        return;
    }
    CALL_HOOK(state_ptr->handler)();
    
    // Go to next state machine
    state_machine_ptr++;
    if(state_machine_ptr >= &std_state_machine_ctrl_struct[std_state_machine_count])
    {
        state_machine_ptr = std_state_machine_ctrl_struct;
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL EnterState(STD_STATE_MACHINE_HNDL hndl, U8 next_state)
{
    CTRL_STRUCT* state_machine_ptr = &std_state_machine_ctrl_struct[hndl];
    
    if(hndl >= std_state_machine_count)
    {
        LOG_WRN("Invalid hndl %d", PU8(hndl));
        return FALSE;
    }
    
    if(next_state >= state_machine_ptr->number_of_states)
    {
        LOG_WRN("Invalid next state %d", PU8(next_state));
        return FALSE;
    }
    
    STD_STATE_MACHINE_STRUCT* current_state_ptr = (STD_STATE_MACHINE_STRUCT*)&state_machine_ptr->state_struct_ptr[state_machine_ptr->active_state];
    STD_STATE_MACHINE_STRUCT* next_state_ptr = (STD_STATE_MACHINE_STRUCT*)&state_machine_ptr->state_struct_ptr[next_state];
    
    LOG_TRM("%s State machine: %s -> %s", PCSTR(state_machine_ptr->module_name), 
                                          PCSTR(current_state_ptr->name), 
                                          PCSTR(next_state_ptr->name));
    
    // Exit current state
    CALL_HOOK(current_state_ptr->on_exit)();
    
    // Enter new state
    CALL_HOOK(next_state_ptr->on_entry)();
    
    // Update state
    state_machine_ptr->active_state = next_state;
    
    return TRUE;
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
void StdStateMachine_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)std_state_machine_ctrl_struct, 0, SIZEOF(std_state_machine_ctrl_struct));
    std_state_machine_count = 0;
    
    Core_RegisterModuleHandler(BackgroundHandler);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
STD_STATE_MACHINE_HNDL StdStateMachine_Register(const STD_STATE_MACHINE_STRUCT* struct_ptr,
                                                U8 number_of_states,
                                                U8 init_state,
                                                STRING module_name)
{
    MODULE_CHECK();
    
    if(struct_ptr == NULL)
    {
        LOG_WRN("struct_ptr is NULL");
        return INVALID_STDSTATEMACHINE_HNDL;
    }
    
    if(number_of_states == 0)
    {
        LOG_WRN("number_of_states is zero");
        return INVALID_STDSTATEMACHINE_HNDL;
    }
    
    if(init_state >= number_of_states)
    {
        LOG_WRN("Invalid init state %d", PU8(init_state));
        return INVALID_STDSTATEMACHINE_HNDL;
    }
    
    if(module_name == NULL)
    {
        LOG_WRN("Invalid module_name");
        return INVALID_STDSTATEMACHINE_HNDL;
    }
    
    if(std_state_machine_count < STATEMACHINE_COUNT)
    {
        std_state_machine_ctrl_struct[std_state_machine_count].state_struct_ptr = struct_ptr;
        std_state_machine_ctrl_struct[std_state_machine_count].number_of_states = number_of_states;
        std_state_machine_ctrl_struct[std_state_machine_count].module_name = module_name;
        std_state_machine_ctrl_struct[std_state_machine_count].active_state = init_state;
        
        std_state_machine_count++;
        return (std_state_machine_count - 1);
    }
    
    LOG_ERR("Failed to register StdStateMachine");
    return INVALID_STDSTATEMACHINE_HNDL;
}
//------------------------------------------------------------------------------------------------//
U8 StdStateMachine_GetActiveState(STD_STATE_MACHINE_HNDL hndl)
{
    CTRL_STRUCT* state_machine_ptr = &std_state_machine_ctrl_struct[hndl];
    
    if(hndl >= std_state_machine_count)
    {
        LOG_WRN("Invalid hndl %d", PU8(hndl));
        return INVALID_STDSTATEMACHINE_STATE;
    }
    
    return state_machine_ptr->active_state;
}
//------------------------------------------------------------------------------------------------//
BOOL StdStateMachine_EnterState(STD_STATE_MACHINE_HNDL hndl, U8 new_state)
{
    return EnterState(hndl, new_state);
}
//------------------------------------------------------------------------------------------------//
void StdStateMachine_PrintState(STD_STATE_MACHINE_HNDL hndl)
{
    CTRL_STRUCT* state_machine_ptr = &std_state_machine_ctrl_struct[hndl];
    
    if(hndl >= std_state_machine_count)
    {
        LOG_WRN("Invalid hndl %d", PU8(hndl));
        return;
    }
    
    STD_STATE_MACHINE_STRUCT* current_state_ptr = (STD_STATE_MACHINE_STRUCT*)&state_machine_ptr->state_struct_ptr[state_machine_ptr->active_state];
    LOG_TRM("%s Current state: %s", PCSTR(state_machine_ptr->module_name), PCSTR(current_state_ptr->name));
}
//================================================================================================//
