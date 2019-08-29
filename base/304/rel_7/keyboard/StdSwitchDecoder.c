//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module to decode switch-inputs, taking into account anti-bounce
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define KEYBOARD__STDSWITCHDECODER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef KEYBOARD__STDSWITCHDECODER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               KEYBOARD__STDSWITCHDECODER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of buttons the application wants to use
#ifndef SWITCH_COUNT
    #error "SWITCH_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of times StdButtonDecode() must be called with a 'button-down' 
//         before it actually is accepted as 'button-down'
#ifndef ANTIBOUNCE_COUNT
    #error "ANTIBOUNCE_COUNT not defined in AppConfig.h"
#else
    #if ANTIBOUNCE_COUNT <= 0 || ANTIBOUNCE_COUNT > 255
        #error "ANTIBOUNCE_COUNT out of bounds"
    #endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD include section
#include "keyboard\StdSwitchDecoder.h"
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
    BOOL     switch_state;
    U8  switch_antibounce_count;
}
SWITCH_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SWITCH_CTRL              switch_ctrl[SWITCH_COUNT];
static U8               switch_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdSwitchDecoderInit(void)
{
    MEMSET((VPTR)switch_ctrl, 0, sizeof(switch_ctrl));
    switch_count = 0;
}
//------------------------------------------------------------------------------------------------//
SWITCH_HNDL StdSwitchDecoderRegister(void)
{
    if(switch_count < SWITCH_COUNT)
    {
        switch_count++;
        return (switch_count - 1);
    }
    return INVALID_SWITCH_HNDL;
}
//------------------------------------------------------------------------------------------------//
BOOL StdSwitchDecode(SWITCH_HNDL switch_hndl, BOOL switch_closed)
{
    SWITCH_CTRL*    switch_ctrl_ptr;
    
    if(switch_hndl == INVALID_SWITCH_HNDL)
    {
        return FALSE;
    }
    switch_ctrl_ptr = &switch_ctrl[switch_hndl];
    
    if(switch_closed == TRUE)
    {
        if(switch_ctrl_ptr->switch_state == TRUE)
        {
            switch_ctrl_ptr->switch_antibounce_count = ANTIBOUNCE_COUNT;
        }
        else if(++(switch_ctrl_ptr->switch_antibounce_count) == ANTIBOUNCE_COUNT)
        {
            switch_ctrl_ptr->switch_state = TRUE;
        }
    }
    else
    {
        if(switch_ctrl_ptr->switch_state == FALSE)
        {
            switch_ctrl_ptr->switch_antibounce_count = 0;
        }
        else if(--(switch_ctrl_ptr->switch_antibounce_count) == 0)
        {
            switch_ctrl_ptr->switch_state = FALSE;
        }
    }
    
    return switch_ctrl_ptr->switch_state;
}
//================================================================================================//
