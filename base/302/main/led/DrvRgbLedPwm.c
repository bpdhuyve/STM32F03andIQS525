//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the RGB LED PWM driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define LED__RGBLEDPWM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef LED__RGBLEDPWM_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               LED__RGBLEDPWM_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number PWM based RGB leds
#ifndef RGBLED_COUNT
	#define RGBLED_COUNT			    1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "led\DrvRgbLedPwm.h"
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
    PWM_CHANNEL_HNDL    pwm_hndl_red;
    PWM_CHANNEL_HNDL    pwm_hndl_green;
    PWM_CHANNEL_HNDL    pwm_hndl_blue;
}
RGBLEDPWM_INFO;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
BOOL RgbLedPwm_SetLevel(RGBLED_ID led_id, U8 red, U8 green, U8 blue);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const RGBLED_HOOK_LIST               rgbledpwm_hook_list = {RgbLedPwm_SetLevel};
static RGBLED_STRUCT                        rgbledpwm_struct[RGBLED_COUNT];
static RGBLEDPWM_INFO                       rgbledpwm_info[RGBLED_COUNT];
static U8                                   rgbledpwm_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL RgbLedPwm_SetLevel(RGBLED_ID led_id, U8 red, U8 green, U8 blue)
{
    RGBLEDPWM_INFO* led_info_ptr = &rgbledpwm_info[led_id];
    
    if(led_id >= rgbledpwm_count)
    {
        return FALSE;
    }
    
    DrvPwm_SetDutyCycle(led_info_ptr->pwm_hndl_red,   (U16)red   * 0x0101);
    DrvPwm_SetDutyCycle(led_info_ptr->pwm_hndl_green, (U16)green * 0x0101);
    DrvPwm_SetDutyCycle(led_info_ptr->pwm_hndl_blue,  (U16)blue  * 0x0101);
    return TRUE;
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRgbLedPwm_Init(void)
{
    MODULE_INIT_ONCE();

    MEMSET((VPTR)rgbledpwm_struct, 0, SIZEOF(rgbledpwm_struct));
    MEMSET((VPTR)rgbledpwm_info, 0, SIZEOF(rgbledpwm_info));
    rgbledpwm_count = 0;

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
RGBLED_HNDL DrvRgbLedPwm_Register(PWM_CHANNEL_HNDL pwm_red, PWM_CHANNEL_HNDL pwm_green, PWM_CHANNEL_HNDL pwm_blue)
{
    MODULE_CHECK();

    RGBLED_HNDL     led_hndl = &rgbledpwm_struct[rgbledpwm_count];
    RGBLEDPWM_INFO* led_info_ptr = &rgbledpwm_info[rgbledpwm_count];
    
    if(rgbledpwm_count < RGBLED_COUNT)
    {
        led_info_ptr->pwm_hndl_red      = pwm_red;
        led_info_ptr->pwm_hndl_green    = pwm_green;
        led_info_ptr->pwm_hndl_blue     = pwm_blue;
        
        led_hndl->hook_list_ptr         = (RGBLED_HOOK_LIST*)&rgbledpwm_hook_list;
        led_hndl->led_id                = rgbledpwm_count;
        led_hndl->led_color             = RGBLED_BLACK;
        led_hndl->led_intensity         = RGBLED_OFF;
        
        rgbledpwm_count++;
        
        // init OFF and enable
        RgbLedPwm_SetLevel(led_hndl->led_id, 0, 0, 0);
        DrvPwm_Enable(pwm_red);
        DrvPwm_Enable(pwm_green);
        DrvPwm_Enable(pwm_blue);
        
        return led_hndl;
    }
    
    LOG_ERR("RGB LED PWM count overrun");
    return NULL;
}
//================================================================================================//
