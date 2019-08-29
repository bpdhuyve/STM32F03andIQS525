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
    DECODE_NOT_PRESSED,
    DECODE_SHORT_PRESSED,
    DECODE_DOUBLE_PRESSED,
    DECODE_WAIT_FOR_NOT_PRESSED,
}
DECODE_STATE;

typedef struct
{
    S8              anti_bounce;
    BOOL            pressed;
    U16             pressed_count;
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

static DECODE_STATE             stdbutton_decode_state;
static BUTTON_BITMASK           stdbutton_decode_buttonmask;
static U16                      stdbutton_decode_timeout;

static BUTTON_BITMASK           stdbutton_command_is_press;
static BUTTON_BITMASK           stdbutton_command_short_press;
static BUTTON_BITMASK           stdbutton_command_long_press;

#if DOUBLE_PRESS_TIMEOUT_COUNT > 0
static BUTTON_BITMASK           stdbutton_command_double_press;
#endif

#if HOLD_TIMEOUT_COUNT > 0
static BUTTON_BITMASK           stdbutton_command_hold_press;
static BUTTON_BITMASK           stdbutton_command_hold_long_press;

static BUTTON_BITMASK           stdbutton_hold_buttonmask;
static U16                      stdbutton_hold_timeout;
static U16                      stdbutton_holdsub_timeout;
static U8                       stdbutton_hold_counter;
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
    MEMSET((VPTR)stdbutton_data, 0, SIZEOF(stdbutton_data));
    
    stdbutton_decode_state = DECODE_WAIT_FOR_NOT_PRESSED;
    stdbutton_decode_buttonmask = 0;
    stdbutton_decode_timeout = 0;
    
    #if HOLD_TIMEOUT_COUNT > 0
    {
        stdbutton_hold_buttonmask = 0;
        stdbutton_hold_timeout = 0;
        stdbutton_holdsub_timeout = 0;
        stdbutton_hold_counter = 0;
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
void StdButtonDecode(BUTTON_BITMASK button_input_mask)
{
    BUTTON_HISTORY* bttn_ptr;
    BUTTON_BITMASK  bttn_mask;
    U8              i;
    
    // CHECK WHICH BUTTONS ARE PRESSED
    stdbutton_command_is_press = 0;
    
    for(i=0; i<BUTTON_COUNT; i++)
    {
        bttn_ptr = &stdbutton_data[i];
        bttn_mask = 0x01 << i;
        
        // ANTIBOUNCE
        if(bttn_ptr->pressed)
        {
            if((button_input_mask & bttn_mask) == 0)
            {
                bttn_ptr->pressed = (BOOL)(--bttn_ptr->anti_bounce > 0);
            }
            else
            {
                bttn_ptr->anti_bounce = ANTIBOUNCE_COUNT;
            }
        }
        else
        {
            if((button_input_mask & bttn_mask) > 0)
            {
                bttn_ptr->pressed = (BOOL)(++bttn_ptr->anti_bounce >= ANTIBOUNCE_COUNT);
            }
            else
            {
                bttn_ptr->anti_bounce = 0;
            }
        }
        
        // LONG PRESS COUNT
        if(bttn_ptr->pressed)
        {
            stdbutton_command_is_press |= bttn_mask;
            if(bttn_ptr->pressed_count < LONG_PRESS_TIMEOUT_COUNT)
            {
                bttn_ptr->pressed_count++;
            }
        }
        else
        {
            bttn_ptr->pressed_count = 0;
        }
    }
    
    // SHORT - DOUBLE - LONG PRESS HANDLING
    stdbutton_command_short_press = 0;
    stdbutton_command_long_press = 0;
    #if DOUBLE_PRESS_TIMEOUT_COUNT > 0
    {
        stdbutton_command_double_press = 0;
    }
    #endif
    
    switch(stdbutton_decode_state)
    {
    case DECODE_NOT_PRESSED:
        if(stdbutton_command_is_press > 0)
        {
            stdbutton_decode_timeout = 0;
            stdbutton_decode_buttonmask = stdbutton_command_is_press;
            stdbutton_decode_state = DECODE_SHORT_PRESSED;
        }
        break;
        
    case DECODE_SHORT_PRESSED:
        // store buttons that are pressed shortly
        stdbutton_decode_buttonmask |= stdbutton_command_is_press;
        
        if(stdbutton_command_is_press > 0)
        {
            // as long as a button is pressed, keep counting
            if(++stdbutton_decode_timeout >= LONG_PRESS_TIMEOUT_COUNT)
            {
                // only take buttons that are pressed long enough (at least half the time)
                for(i=0; i<BUTTON_COUNT; i++)
                {
                    if(stdbutton_data[i].pressed_count > (LONG_PRESS_TIMEOUT_COUNT/2))
                    {
                        stdbutton_command_long_press |= 1 << i;
                    }
                }
                LOG_DBG("LONG %04x", PU16(stdbutton_command_long_press));
                stdbutton_decode_state = DECODE_WAIT_FOR_NOT_PRESSED;
            }
        }
        else 
        {
            #if DOUBLE_PRESS_TIMEOUT_COUNT > 0
            {
                stdbutton_decode_timeout = 0;
                stdbutton_decode_state = DECODE_DOUBLE_PRESSED;
            }
            #else
            {
                stdbutton_command_short_press = stdbutton_decode_buttonmask;
                LOG_DBG("SHORT %04x", PU16(stdbutton_command_short_press));
                stdbutton_decode_state = DECODE_WAIT_FOR_NOT_PRESSED;
            }
            #endif
        }
        break;
        
    case DECODE_DOUBLE_PRESSED:
        #if DOUBLE_PRESS_TIMEOUT_COUNT > 0
        {
            if(stdbutton_command_is_press == stdbutton_decode_buttonmask)
            {
                stdbutton_command_double_press = stdbutton_decode_buttonmask;
                LOG_DBG("DOUBLE %04x", PU16(stdbutton_command_double_press));
                stdbutton_decode_state = DECODE_WAIT_FOR_NOT_PRESSED;
            }
            else if(++stdbutton_decode_timeout >= DOUBLE_PRESS_TIMEOUT_COUNT)
            {
                stdbutton_command_short_press = stdbutton_decode_buttonmask;
                LOG_DBG("SHORT %04x", PU16(stdbutton_command_short_press));
                stdbutton_decode_state = DECODE_WAIT_FOR_NOT_PRESSED;
            }
        }
        #endif
        break;
        
    case DECODE_WAIT_FOR_NOT_PRESSED: // action is terminated, waiting for all buttons to be released
        if(stdbutton_command_is_press == 0)
        {
            stdbutton_decode_state = DECODE_NOT_PRESSED;
        }
        break;
    }
    
    // HOLD - HOLD LONG HANDLING
    #if HOLD_TIMEOUT_COUNT > 0
    {
        stdbutton_command_hold_press = 0;
        stdbutton_command_hold_long_press = 0;
        
        if(stdbutton_command_is_press > 0)
        {
            if(stdbutton_hold_buttonmask == stdbutton_command_is_press)
            {
                // pressed buttonmask is equal to stored mask
                // if subset of stored mask was registered, add time subset was active
                stdbutton_hold_timeout += stdbutton_holdsub_timeout;
                stdbutton_holdsub_timeout = 0;
                if(++stdbutton_hold_timeout >= HOLD_TIMEOUT_COUNT)
                {
                    stdbutton_hold_timeout = 0;
                    if(stdbutton_hold_counter < HOLD_LONG_COUNT)
                    {
                        stdbutton_command_hold_press = stdbutton_hold_buttonmask;
                        LOG_DBG("HOLD %04x", PU16(stdbutton_command_hold_press));
                        stdbutton_hold_counter++;
                    }
                    else
                    {
                        stdbutton_command_hold_long_press = stdbutton_hold_buttonmask;
                        LOG_DBG("HOLDLONG %04x", PU16(stdbutton_command_hold_long_press));
                    }
                }
            }
            else if((stdbutton_hold_buttonmask & stdbutton_command_is_press) == stdbutton_command_is_press)
            {
                // pressed buttonmask is subset of stored mask
                // if subset runs equally long as stored, swap to subset
                if(++stdbutton_holdsub_timeout >= stdbutton_hold_timeout)
                {
                    stdbutton_hold_buttonmask = stdbutton_command_is_press;
                    stdbutton_holdsub_timeout = 0;
                    stdbutton_hold_counter = 0;
                }
            }
            else
            {
                // otherwise take over pressed buttonmask and restart
                stdbutton_hold_buttonmask = stdbutton_command_is_press;
                stdbutton_hold_timeout = 1;
                stdbutton_holdsub_timeout = 0;
                stdbutton_hold_counter = 0;
            }
        }
        else
        {
            stdbutton_hold_buttonmask = 0;
            stdbutton_hold_timeout = 0;
            stdbutton_holdsub_timeout = 0;
            stdbutton_hold_counter = 0;
        }
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
    {
        return stdbutton_command_double_press;
    }
    #else
    {
        return 0;
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetHoldPress(void)
{
    #if HOLD_TIMEOUT_COUNT > 0
    {
        return stdbutton_command_hold_press;
    }
    #else
    {
        return 0;
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
BUTTON_BITMASK StdButtonGetHoldLongPress(void)
{
    #if HOLD_TIMEOUT_COUNT > 0
    {
        return stdbutton_command_hold_long_press;
    }
    #else
    {
        return 0;
    }
    #endif
}
//================================================================================================//
