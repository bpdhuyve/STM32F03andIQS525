//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the RGB LED GPIO driver (simple)
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define LED__RGBLEDGPIO_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef LED__RGBLEDGPIO_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               LED__RGBLEDGPIO_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number GPIO based RGB leds
#ifndef RGBLED_COUNT
	#define RGBLED_COUNT			    1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "led\DrvRgbLedGpio.h"
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
    DRVGPIO_PIN_HNDL    gpio_hndl_red;
    DRVGPIO_PIN_HNDL    gpio_hndl_green;
    DRVGPIO_PIN_HNDL    gpio_hndl_blue;
    BOOL                active_high;
}
RGBLEDGPIO_INFO;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
BOOL RgbLedGpio_SetLevel(RGBLED_ID led_id, U8 red, U8 green, U8 blue);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const RGBLED_HOOK_LIST               rgbledgpio_hook_list = {RgbLedGpio_SetLevel};
static RGBLED_STRUCT                        rgbledgpio_struct[RGBLED_COUNT];
static RGBLEDGPIO_INFO                      rgbledgpio_info[RGBLED_COUNT];
static U8                                   rgbledgpio_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL RgbLedGpio_SetLevel(RGBLED_ID led_id, U8 red, U8 green, U8 blue)
{
    RGBLEDGPIO_INFO* led_info_ptr = &rgbledgpio_info[led_id];
    
    if(led_id >= rgbledgpio_count)
    {
        return FALSE;
    }
    
    DrvGpio_SetPin(led_info_ptr->gpio_hndl_red,   (BOOL)(led_info_ptr->active_high == (BOOL)(red   >= 0x80)));
    DrvGpio_SetPin(led_info_ptr->gpio_hndl_green, (BOOL)(led_info_ptr->active_high == (BOOL)(green >= 0x80)));
    DrvGpio_SetPin(led_info_ptr->gpio_hndl_blue,  (BOOL)(led_info_ptr->active_high == (BOOL)(blue  >= 0x80)));
    return TRUE;
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRgbLedGpio_Init(void)
{
    MODULE_INIT_ONCE();

    MEMSET((VPTR)rgbledgpio_struct, 0, SIZEOF(rgbledgpio_struct));
    MEMSET((VPTR)rgbledgpio_info, 0, SIZEOF(rgbledgpio_info));
    rgbledgpio_count = 0;

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
RGBLED_HNDL DrvRgbLedGpio_Register(DRVGPIO_PIN_HNDL gpio_red, DRVGPIO_PIN_HNDL gpio_green, DRVGPIO_PIN_HNDL gpio_blue, BOOL active_high)
{
    MODULE_CHECK();

    RGBLED_HNDL         led_hndl = &rgbledgpio_struct[rgbledgpio_count];
    RGBLEDGPIO_INFO*    led_info_ptr = &rgbledgpio_info[rgbledgpio_count];
    
    if(rgbledgpio_count < RGBLED_COUNT)
    {
        led_info_ptr->gpio_hndl_red     = gpio_red;
        led_info_ptr->gpio_hndl_green   = gpio_green;
        led_info_ptr->gpio_hndl_blue    = gpio_blue;
        led_info_ptr->active_high       = active_high;
        
        led_hndl->hook_list_ptr         = (RGBLED_HOOK_LIST*)&rgbledgpio_hook_list;
        led_hndl->led_id                = rgbledgpio_count;
        led_hndl->led_color             = RGBLED_BLACK;
        led_hndl->led_intensity         = RGBLED_OFF;
        
        rgbledgpio_count++;
        
        // init OFF
        RgbLedGpio_SetLevel(led_hndl->led_id, 0, 0, 0);
        
        return led_hndl;
    }
    
    LOG_ERR("RGB LED GPIO count overrun");
    return NULL;
}
//================================================================================================//
