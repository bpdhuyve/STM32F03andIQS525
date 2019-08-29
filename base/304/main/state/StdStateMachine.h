//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Generic implementation of state machine
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STDSTATEMACHINE_H
#define STDSTATEMACHINE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_STDSTATEMACHINE_HNDL        0xFF
#define INVALID_STDSTATEMACHINE_STATE       0xFF
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
	EVENT_CALLBACK   on_entry;
	EVENT_CALLBACK   handler;
	EVENT_CALLBACK   on_exit;
    STRING           name;
}
STD_STATE_MACHINE_STRUCT;

typedef struct
{
    const STD_STATE_MACHINE_STRUCT* struct_ptr;
    U8                              number_of_states;
    U8                              init_state;
    STRING                          module_name;
}
STD_STATE_MACHINE_INIT_STRUCT;

typedef U8                  STD_STATE_MACHINE_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void StdStateMachine_Init(void);

STD_STATE_MACHINE_HNDL StdStateMachine_Register(const STD_STATE_MACHINE_STRUCT* struct_ptr,
                                                U8 number_of_states,
                                                U8 init_state,
                                                STRING module_name);

U8 StdStateMachine_GetActiveState(STD_STATE_MACHINE_HNDL hndl);

BOOL StdStateMachine_EnterState(STD_STATE_MACHINE_HNDL hndl, U8 new_state);

void StdStateMachine_PrintState(STD_STATE_MACHINE_HNDL hndl);
//================================================================================================//



#endif /* STDSTATEMACHINE_H */
