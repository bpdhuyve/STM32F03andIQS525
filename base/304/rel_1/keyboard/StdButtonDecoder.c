//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module holding a statemachine to decode button-inputs into actions/commands
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define KEYBOARD__STDBUTTONDECODER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef KEYBOARD__STDBUTTONDECODER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               KEYBOARD__STDBUTTONDECODER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of times StdButtonDecode() must be called with a 'button-down' 
//         before it actually is accepted as 'button-down'
#ifndef ANTIBOUNCE_COUNT
    #error "ANTIBOUNCE_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of times StdButtonDecode() must be called with a 'button-down' 
//         before it actually is accepted as 'long-pressed'
#ifndef LONG_PRESS_TIMEOUT_COUNT
    #error "LONG_PRESS_TIMEOUT_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of times StdButtonDecode() must be called with a 'button-release' 
//         before we stop waiting for a potential 2nd 'button-down' to enable 'double-pressed'
#ifndef DOUBLE_PRESS_TIMEOUT_COUNT
    #error "DOUBLE_PRESS_TIMEOUT_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of times StdButtonDecode() must be called with a 'button-down' 
//         as an interval for the key hold
#ifndef HOLD_TIMEOUT_COUNT
    #error "HOLD_TIMEOUT_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of times the hold timeout has to be striken before the button mask is considered to
//         be held long
#ifndef HOLD_LONG_COUNT
    #error "HOLD_LONG_COUNT not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD include section
#include "keyboard\StdButtonDecoder.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    WAIT_FOR_NOT_PRESSED,
    NOT_PRESSED,
    SHORT_PRESSED,
    PRE_DOUBLE_PRESSED,
    DOUBLE_PRESSED,
    SHORT_PRESSED_READY,
    LONG_PRESSED_READY,
    DOUBLE_PRESSED_READY
}
BUTTON_STATE;

typedef struct
{
    S8              anti_bounce;
    U16             timeout_counter;
    BUTTON_STATE    state;
}
BUTTON_HISTORY;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static BUTTON_HISTORY           stdbutton_data[BUTTON_COUNT];

static BUTTON_BITMASK           stdbutton_command_is_press;
static BUTTON_BITMASK           stdbutton_command_short_press;
static BUTTON_BITMASK           stdbutton_command_long_press;

#if DOUBLE_PRESS_TIMEOUT_COUNT > 0
static BUTTON_BITMASK           stdbutton_command_double_press;
#endif

#if HOLD_TIMEOUT_COUNT > 0
static BUTTON_BITMASK           stdbutton_command_hold_press;
static BUTTON_BITMASK           stdbutton_command_hold_long_press;

static BUTTON_BITMASK           stdbutton_hold_button_bitmask;
static U16                      stdbutton_hold_timeout_counter;
static U16                      stdbutton_hold_counter;
#endif
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
void StdButtonDecoderInit(void)
{
    BUTTON_HISTORY* bttn_ptr = &stdbutton_data[0];
    while(bttn_ptr < &stdbutton_data[BUTTON_COUNT])
    {
        bttn_ptr->anti_bounce = 0;
        bttn_ptr->timeout_counter = 0;
        bttn_ptr->state = WAIT_FOR_NOT_PRESSED;
        bttn_ptr++;
    }
#if HOLD_TIMEOUT_COUNT > 0
    stdbutton_hold_button_bitmask = 0;
    stdbutton_hold_timeout_counter = 0;
    stdbutton_hold_counter = 0;
#endif
}
//------------------------------------------------------------------------------------------------//
void StdButtonDecode(BUTTON_BITMASK button_input_mask)
{
    BUTTON_HISTORY* bttn_ptr;
    BUTTON_BITMASK bttn_mask;
    U8 i;
    U8 buttons_ready_count = 0;
    
    for(i=0; i<BUTTON_COUNT; i++)
    {
        bttn_ptr = &stdbutton_data[i];
        bttn_mask = 0x01 << i;
        
        switch(bttn_ptr->state)
        {
        case WAIT_FOR_NOT_PRESSED:
            if((button_input_mask & bttn_mask) == 0) //button released
            {
                bttn_ptr->anti_bounce--;
                if(bttn_ptr->anti_bounce <= 0)
                {
                    bttn_ptr->state = NOT_PRESSED;
                    bttn_ptr->timeout_counter = 0;
                }
            }
            break;
        case NOT_PRESSED:
            if(button_input_mask & bttn_mask) //button down
            {
                bttn_ptr->anti_bounce++;
                if(bttn_ptr->anti_bounce >= ANTIBOUNCE_COUNT)
                {
                    bttn_ptr->state = SHORT_PRESSED;
                    bttn_ptr->timeout_counter = 0;
                }
            }
            else //button released
            {
                bttn_ptr->anti_bounce = 0;
                buttons_ready_count++;
            }
            break;

        case SHORT_PRESSED:
            if(button_input_mask & bttn_mask) //button down
            {
                bttn_ptr->anti_bounce = ANTIBOUNCE_COUNT;
            }
            else //button released
            {
                bttn_ptr->anti_bounce--;
                if(bttn_ptr->anti_bounce <= 0)
                {
#if DOUBLE_PRESS_TIMEOUT_COUNT > 0  
                    bttn_ptr->state = PRE_DOUBLE_PRESSED;
#else
                    bttn_ptr->state = SHORT_PRESSED_READY;
                    buttons_ready_count++;
#endif
                    bttn_ptr->timeout_counter = 0;
                }
            }
            bttn_ptr->timeout_counter++;
            if(bttn_ptr->timeout_counter >= LONG_PRESS_TIMEOUT_COUNT)
            {
                bttn_ptr->state = LONG_PRESSED_READY;
            }
            break;

#if DOUBLE_PRESS_TIMEOUT_COUNT > 0            
        case PRE_DOUBLE_PRESSED:
            if(button_input_mask & bttn_mask) //button down
            {
                bttn_ptr->anti_bounce++;
                if(bttn_ptr->anti_bounce >= ANTIBOUNCE_COUNT)
                {
                    bttn_ptr->state = DOUBLE_PRESSED;
                }
            }
            else //button released
            {
                bttn_ptr->anti_bounce = 0;
            }
            bttn_ptr->timeout_counter++;
            if(bttn_ptr->timeout_counter >= DOUBLE_PRESS_TIMEOUT_COUNT)
            {
                bttn_ptr->anti_bounce = 0;
                bttn_ptr->state = SHORT_PRESSED_READY;
                buttons_ready_count++;
            }
            break;
            
        case DOUBLE_PRESSED:
            if(button_input_mask & bttn_mask) //button down
            {
                bttn_ptr->anti_bounce = ANTIBOUNCE_COUNT;
            }
            else //button released
            {
                bttn_ptr->anti_bounce--;
                if(bttn_ptr->anti_bounce <= 0)
                {
                    bttn_ptr->state = DOUBLE_PRESSED_READY;
                    buttons_ready_count++;
                }
            }
            break;
#endif
            
        case SHORT_PRESSED_READY:
        case LONG_PRESSED_READY:
        case DOUBLE_PRESSED_READY:
            buttons_ready_count++;
            break;
        }
    }

    //build-up the actions/commands
    stdbutton_command_short_press = 0;
    stdbutton_command_long_press = 0;
#if DOUBLE_PRESS_TIMEOUT_COUNT > 0
    stdbutton_command_double_press = 0;
#endif
    if(buttons_ready_count == BUTTON_COUNT)
    {
        for(i=0; i<BUTTON_COUNT; i++)
        {
            bttn_ptr = &stdbutton_data[i];
            bttn_mask = ((BUTTON_BITMASK)0x01) << i;
            
            switch(bttn_ptr->state)
            {
            case SHORT_PRESSED_READY:
                LOG_DBG("_S_%d", PU8(i));
                stdbutton_command_short_press |= bttn_mask;
                bttn_ptr->state = NOT_PRESSED;
                break;
            case LONG_PRESSED_READY:
                LOG_DBG("_L_%d", PU8(i));
                stdbutton_command_long_press |= bttn_mask;
                bttn_ptr->state = WAIT_FOR_NOT_PRESSED;
                break;
#if DOUBLE_PRESS_TIMEOUT_COUNT > 0
            case DOUBLE_PRESSED_READY:
                LOG_DBG("_D_%d", PU8(i));
                stdbutton_command_double_press |= bttn_mask;
                bttn_ptr->state = NOT_PRESSED;
                break;
#endif
            }
        }
    }
    
    stdbutton_command_is_press = 0;
    for(i=0; i<BUTTON_COUNT; i++)
    {
        bttn_ptr = &stdbutton_data[i];
        bttn_mask = ((BUTTON_BITMASK)0x01) << i;
        
        if(bttn_ptr->state != NOT_PRESSED)
        {
            stdbutton_command_is_press |= bttn_mask;
        }
    }

#if HOLD_TIMEOUT_COUNT > 0
    stdbutton_command_hold_press = 0;
    stdbutton_command_hold_long_press = 0;
    bttn_mask = 0;
    for(i=0; i<BUTTON_COUNT; i++)
    {
        bttn_ptr = &stdbutton_data[i];
        if((bttn_ptr->state == SHORT_PRESSED) ||
           (bttn_ptr->state == LONG_PRESSED_READY) ||
           (bttn_ptr->state == WAIT_FOR_NOT_PRESSED))
        {
            bttn_mask |= 0x01 << i;
        }
    }
    if((bttn_mask > 0) && (bttn_mask == stdbutton_hold_button_bitmask))
    {
        if(++stdbutton_hold_timeout_counter >= HOLD_TIMEOUT_COUNT)
        {
            stdbutton_hold_timeout_counter = 0;
            for(i=0; i<BUTTON_COUNT; i++)
            {
                if((bttn_mask & ((BUTTON_BITMASK)0x01 << i)) > 0)
                {
                    LOG_DBG("_H_%d", PU8(i));
                }
            }
            if(++stdbutton_hold_counter > HOLD_LONG_COUNT)
            {
                stdbutton_command_hold_long_press = bttn_mask;
            }
            else
            {
                stdbutton_command_hold_press = bttn_mask;
            }
        }
    }
    else
    {
        stdbutton_hold_button_bitmask = bttn_mask;
        stdbutton_hold_timeout_counter = 0;
        stdbutton_hold_counter = 0;
    }
#endif
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetIsPress(void)
{
    return stdbutton_command_is_press;
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetShortPress(void)
{
    return stdbutton_command_short_press;
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetLongPress(void)
{
    return stdbutton_command_long_press;
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetDoublePress(void)
{
#if DOUBLE_PRESS_TIMEOUT_COUNT > 0
    return stdbutton_command_double_press;
#else
    return 0;
#endif
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetHoldPress(void)
{
#if HOLD_TIMEOUT_COUNT > 0
    return stdbutton_command_hold_press;
#else
    return 0;
#endif
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetHoldLongPress(void)
{
#if HOLD_TIMEOUT_COUNT > 0
    return stdbutton_command_hold_long_press;
#else
    return 0;
#endif
}
//================================================================================================//
