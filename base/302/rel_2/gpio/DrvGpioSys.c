//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the system GPIO
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define GPIO__DRVGPIOSYS_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef GPIO__DRVGPIOSYS_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               GPIO__DRVGPIOSYS_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of gpio pins that can be registered
#ifndef DRVGPIOSYS_MAX_PINS
	#define DRVGPIOSYS_MAX_PINS			        10
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of gpio pins that are allowed to change their state
#ifndef DRVGPIOSYS_MAX_SWITCH_PINS
	#define DRVGPIOSYS_MAX_SWITCH_PINS			0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "gpio\DrvGpioSys.h"
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
    U16             pin_id;
    SYS_PIN_FUNC    input_func;
    SYS_PIN_FUNC    output_func;
}
SWITCH_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if DRVGPIOSYS_MAX_SWITCH_PINS
static void DrvGpioSys_ReInitPin(U16 pin_id, GPIO_PIN_FUNCTION io_pin_func);
#endif
static void DrvGpioSys_SetPin(U16 pin_id, BOOL value);
static BOOL DrvGpioSys_GetPin(U16 pin_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static DRV_GPIO_PIN_STRUCT          gpio_pin_struct[DRVGPIOSYS_MAX_PINS];
static U8                           gpio_pin_count;
static GPIO_DRV_HOOK_LIST           gpio_pin_hook_list;
#if DRVGPIOSYS_MAX_SWITCH_PINS
static SWITCH_STRUCT                gpio_switch_struct[DRVGPIOSYS_MAX_SWITCH_PINS];
static U8                           gpio_switch_count;
static GPIO_DRV_HOOK_LIST           gpio_pin_hook_switch_list;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if DRVGPIOSYS_MAX_SWITCH_PINS
static void DrvGpioSys_ReInitPin(U16 pin_id, GPIO_PIN_FUNCTION io_pin_func)
{
    SWITCH_STRUCT*  switch_hndl;

    for(switch_hndl = gpio_switch_struct; switch_hndl < &gpio_switch_struct[gpio_switch_count]; switch_hndl++)
    {
        if(switch_hndl->pin_id == pin_id)
        {
            if(io_pin_func == GPIO_PIN_INPUT)
            {
                SysPin_InitPinMask((GPIO_PORT)(pin_id >> 8), 0x0001 << (pin_id & 0xFF), switch_hndl->input_func);
            }
            else
            {
                SysPin_InitPinMask((GPIO_PORT)(pin_id >> 8), 0x0001 << (pin_id & 0xFF), switch_hndl->output_func);
                SysGpio_SetPinMask((GPIO_PORT)(pin_id >> 8), 0x0001 << (pin_id & 0xFF), (BOOL)(io_pin_func == GPIO_PIN_OUTPUT_START_HIGH));
            }
        }
    }
}
#endif
//------------------------------------------------------------------------------------------------//
static void DrvGpioSys_SetPin(U16 pin_id, BOOL value)
{
    GPIO_PORT port = (GPIO_PORT)(pin_id >> 8);
    U32 pin_mask = 0x0001 << (pin_id & 0x00FF);

    SysGpio_SetPinMask(port, pin_mask, value);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvGpioSys_GetPin(U16 pin_id)
{
    GPIO_PORT port = (GPIO_PORT)(pin_id >> 8);
    U32 pin_mask = 0x0001 << (pin_id & 0x00FF);

    return (BOOL)(SysGpio_GetPinMask(port, pin_mask) == pin_mask);
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvGpioSys_Init(void)
{
    SysGpio_Init();

    gpio_pin_hook_list.re_init_hook = NULL;
    gpio_pin_hook_list.set_hook = DrvGpioSys_SetPin;
    gpio_pin_hook_list.get_hook = DrvGpioSys_GetPin;

    MEMSET((VPTR)gpio_pin_struct, 0, SIZEOF(gpio_pin_struct));
    gpio_pin_count = 0;

#if DRVGPIOSYS_MAX_SWITCH_PINS
    gpio_pin_hook_switch_list.re_init_hook = DrvGpioSys_ReInitPin;
    gpio_pin_hook_switch_list.set_hook = DrvGpioSys_SetPin;
    gpio_pin_hook_switch_list.get_hook = DrvGpioSys_GetPin;

    MEMSET((VPTR)gpio_switch_struct, 0, SIZEOF(gpio_switch_struct));
    gpio_switch_count = 0;
#endif
}
//------------------------------------------------------------------------------------------------//
DRVGPIO_PIN_HNDL DrvGpioSys_RegisterPin(GPIO_PORT io_port, U8 io_pin_nr, SYS_PIN_FUNC pin_func)
{
    DRVGPIO_PIN_HNDL        pin_hndl;
    U16                     pin_id = (((U16)io_port) << 8) | io_pin_nr;

    //check all registred pins and check if it has been registred yet
    for(pin_hndl = gpio_pin_struct; pin_hndl < &gpio_pin_struct[gpio_pin_count]; pin_hndl++)
    {
        if(pin_hndl->pin_id == pin_id)
        {
            LOG_WRN("re-register port %d pin %d", PU8(io_port), PU8(io_pin_nr));
            SysPin_InitPin(io_port, io_pin_nr, pin_func);
            return pin_hndl;  //same pin_id found, return same handle
        }
    }

    if(gpio_pin_count < DRVGPIOSYS_MAX_PINS)
    {
        pin_hndl->pin_id = pin_id;
        pin_hndl->hook_list_ptr = &gpio_pin_hook_list;
        SysPin_InitPin(io_port, io_pin_nr, pin_func);
        gpio_pin_count++;
        return pin_hndl;
    }
    LOG_ERR("Pin register count overrun");
    return NULL;   //pin hndl null means no pin
}
//------------------------------------------------------------------------------------------------//
BOOL DrvGpioSys_EnableSwitch(DRVGPIO_PIN_HNDL pin_hndl, SYS_PIN_FUNC input_pin_func, SYS_PIN_FUNC output_pin_func)
{
#if DRVGPIOSYS_MAX_SWITCH_PINS
    SWITCH_STRUCT*  switch_hndl = &gpio_switch_struct[gpio_switch_count];

    if(gpio_switch_count < DRVGPIOSYS_MAX_SWITCH_PINS)
    {
        switch_hndl->pin_id = pin_hndl->pin_id;
        switch_hndl->input_func = input_pin_func;
        switch_hndl->output_func = output_pin_func;
        pin_hndl->hook_list_ptr = &gpio_pin_hook_switch_list;
        gpio_switch_count++;
        return TRUE;
    }
    return FALSE;
#else
    LOG_WRN("GPIO switch not enabled");
    return FALSE;
#endif
}
//================================================================================================//