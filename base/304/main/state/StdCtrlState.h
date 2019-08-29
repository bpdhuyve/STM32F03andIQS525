//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to register and handle basic state machine
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STATE__STDCTRLSTATE_H
#define STATE__STDCTRLSTATE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_STDCTRLSTATE_HNDL               0xFF
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    STDCTRLSTATE_INIT           = 0,
    STDCTRLSTATE_DISABLED       = 1,
    STDCTRLSTATE_ACTIVE         = 2,
    STDCTRLSTATE_ERROR          = 3,
}
STDCTRLSTATE;

typedef void (*ON_STATE_ENTER)(STDCTRLSTATE state);
typedef BOOL (*INIT_DONE_CHECK)(void);
typedef BOOL (*ERROR_MASK_CHECK)(U32 error_mask);

typedef struct
{
    ON_STATE_ENTER          on_state_enter_hook;
    INIT_DONE_CHECK         init_done_check_hook;
    ERROR_MASK_CHECK        error_mask_check_hook;
    EVENT_CALLBACK          active_control_hook;
    STRING                  module_name;
}
STDCTRLSTATE_CONFIG_STRUCT;

typedef U8                  STDCTRLSTATE_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void StdCtrlState_Init(void);

STDCTRLSTATE_HNDL StdCtrlState_Register(const STDCTRLSTATE_CONFIG_STRUCT* config_ptr);

BOOL StdCtrlState_RequestActive(STDCTRLSTATE_HNDL hndl, BOOL request_active);

BOOL StdCtrlState_SetTickDivider(STDCTRLSTATE_HNDL hndl, U16 tick_divider);

BOOL StdCtrlState_GetState(STDCTRLSTATE_HNDL hndl, STDCTRLSTATE* state_ptr);

BOOL StdCtrlState_GetErrorMask(STDCTRLSTATE_HNDL hndl, U32* error_mask_ptr);

void StdCtrlState_PrintStateAndError(STDCTRLSTATE_HNDL hndl, const STRING* error_mask_names);
//================================================================================================//



#endif /* STATE__STDCTRLSTATE_H */

