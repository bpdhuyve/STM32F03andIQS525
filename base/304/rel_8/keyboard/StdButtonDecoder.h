//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module holding a statemachine to decode button-inputs into actions/commands
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef KEYBOARD__STDBUTTONDECODER_H
#define KEYBOARD__STDBUTTONDECODER_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of buttons the application wants to use
#ifndef BUTTON_COUNT
    #error "BUTTON_COUNT not defined in AppConfig.h"
#else
    #if(BUTTON_COUNT > 32)
        #error "BUTTON_COUNT cannot be more than 32"
    #endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
#if(BUTTON_COUNT <= 8)
    typedef U8            BUTTON_BITMASK;
#elif(BUTTON_COUNT <= 16)
    typedef U16           BUTTON_BITMASK;
#elif(BUTTON_COUNT <= 32)
    typedef U32           BUTTON_BITMASK;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialises the module.
void StdButtonDecoderInit(void);

// @brief   Looks at the given button_input_mask and decides - using history (previous calls) - what happened
//          and reflects that in the different command button bitmasks
// @param   button_input_mask          input mask where b0 reflects the state of button_0,
//                                           '0' means button is released
//                                           '1' means button is down
void StdButtonDecode(BUTTON_BITMASK button_input_mask);

//gewoon doorgeven wat de input was (zonder bounce)
BUTTON_BITMASK StdButtonGetIsPress(void);

//mask will be flaged only once (when you stop pressing the button)
BUTTON_BITMASK StdButtonGetShortPress(void);

//mask will be flaged only once (when you stop pressing the button)
BUTTON_BITMASK StdButtonGetLongPress(void);

//mask will be flaged only once (when you stop pressing the button)
BUTTON_BITMASK StdButtonGetDoublePress(void);

//mask will be flagged repeatedly until he holdlongpress time is reached
BUTTON_BITMASK StdButtonGetHoldPress(void);

//mask will be flagged repeatedly from when the holdpress time is passed
BUTTON_BITMASK StdButtonGetHoldLongPress(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* KEYBOARD__STDBUTTONDECODER_H */
