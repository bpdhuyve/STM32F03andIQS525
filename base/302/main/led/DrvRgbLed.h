//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for RGB LED driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef LED__RGBLED_H
#define LED__RGBLED_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define RGBLED_BRIGHT                   0xFF
#define RGBLED_HALF                     0x7F
#define RGBLED_DARK                     0x3F
#define RGBLED_OFF                      0x00
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                              RGBLED_ID;

// color enumeration is a mask with 2 bits per color : 00rrggbb
// 0 = OFF (O), 1 = QUARTER (Q), 2 = HALF (H), 3 = FULL (F)
typedef enum
{
    // black/white
    RGBLED_BLACK        = 0x00,     // OOO
    RGBLED_WHITE        = 0x3F,     // FFF
    
    // basics
    RGBLED_RED          = 0x30,     // FOO
    RGBLED_YELLOW       = 0x3C,     // FFO
    RGBLED_GREEN        = 0x0C,     // OFO
    RGBLED_CYAN         = 0x0F,     // OFF
    RGBLED_BLUE         = 0x03,     // OOF
    RGBLED_MAGENTA      = 0x33,     // FOF
    
    // specials
    RGBLED_ORANGE       = 0x38,     // FHO
    RGBLED_DARKORANGE   = 0x34,     // FQO
    RGBLED_LIGHTGREEN   = 0x1D,     // QFQ
}
RGBLED_COLOR;

typedef U8                              RGBLED_INTENSITY;

typedef BOOL (*RGBLED_SET_LEVEL)(RGBLED_ID led_id, U8 red, U8 green, U8 blue);

typedef struct
{
    RGBLED_SET_LEVEL                    set_level_hook;
}
RGBLED_HOOK_LIST;

typedef struct
{
    RGBLED_ID                           led_id;
    RGBLED_COLOR                        led_color;
    RGBLED_INTENSITY                    led_intensity;
    RGBLED_HOOK_LIST*	                hook_list_ptr;
}
RGBLED_STRUCT;

typedef RGBLED_STRUCT*                  RGBLED_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvRgbLed_Init(void);

BOOL DrvRgbLed_SetRgb(RGBLED_HNDL led_hndl, U8 red, U8 green, U8 blue);

BOOL DrvRgbLed_SetColor(RGBLED_HNDL led_hndl, RGBLED_COLOR new_color, RGBLED_INTENSITY new_intensity);

BOOL DrvRgbLed_UpdateColor(RGBLED_HNDL led_hndl, RGBLED_COLOR new_color);

BOOL DrvRgbLed_UpdateIntensity(RGBLED_HNDL led_hndl, RGBLED_INTENSITY new_intensity);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* LED__RGBLED_H */
