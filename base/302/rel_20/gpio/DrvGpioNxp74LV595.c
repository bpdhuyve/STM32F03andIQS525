//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of NXP 74LV595 shift register
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define GPIO__DRVGPIONXP74LV595_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef GPIO__DRVGPIONXP74LV595_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               GPIO__DRVGPIONXP74LV595_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of shift registers
#ifndef DRVGPIONXP74LV595_COUNT
	#define DRVGPIONXP74LV595_COUNT 	1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "gpio\DrvGpioNxp74LV595.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DRVGPIONXP74LV595_PIN_COUNT     DRVGPIONXP74LV595_COUNT * 8
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    DRVGPIO_PIN_HNDL    ds_pin_hndl;
    DRVGPIO_PIN_HNDL    shcp_pin_hndl;
    DRVGPIO_PIN_HNDL    stcp_pin_hndl;
    U8                  data;
}
NXP74LV595_DATA_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrvGpioNxp74LV595_SetPin(U16 pin_id, BOOL value);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static DRV_GPIO_PIN_STRUCT          nxp74lv595_pin_struct[DRVGPIONXP74LV595_PIN_COUNT];
static U8                           nxp74lv595_pin_count;
static NXP74LV595_DATA_STRUCT       nxp74lv595_device_struct[DRVGPIONXP74LV595_COUNT];
static U8                           nxp74lv595_device_count;
static GPIO_DRV_HOOK_LIST           nxp74lv595_hook_list;
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
static void DrvGpioNxp74LV595_SetPin(U16 pin_id, BOOL value)
{
    NXP74LV595_DATA_STRUCT*     device_struct_ptr;
    NXP_74LV595_HNDL            device_hndl = (NXP_74LV595_HNDL)(pin_id >> 8);
    U8                          pin_mask = 0x01 << (pin_id & 0x0007);
    U8                          i;
    
    if(device_hndl < nxp74lv595_device_count)
    {
        device_struct_ptr = &nxp74lv595_device_struct[device_hndl];
        if(value == TRUE)
        {
            device_struct_ptr->data |= pin_mask;
        }
        else
        {
            device_struct_ptr->data &= ~pin_mask;
        }
        
        // shift out
        pin_mask = device_struct_ptr->data;
        for(i = 0; i < 8; i++, pin_mask <<= 1)
        {
            DrvGpio_SetPin(device_struct_ptr->ds_pin_hndl, (BOOL)(pin_mask & 0x80));       // set DS
            DrvGpio_SetPin(device_struct_ptr->shcp_pin_hndl, TRUE);                        // clock SHCP
            DrvGpio_SetPin(device_struct_ptr->shcp_pin_hndl, FALSE);                       // reset SHCP
        }
        DrvGpio_SetPin(device_struct_ptr->ds_pin_hndl, FALSE);                             // reset DS
        DrvGpio_SetPin(device_struct_ptr->stcp_pin_hndl, TRUE);                            // clock STCP
        DrvGpio_SetPin(device_struct_ptr->stcp_pin_hndl, FALSE);                           // reset STCP
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvGpioNxp74LV595_Init(void)
{
    nxp74lv595_hook_list.re_init_hook = NULL;
    nxp74lv595_hook_list.set_hook = DrvGpioNxp74LV595_SetPin;
    nxp74lv595_hook_list.get_hook = NULL;

    MEMSET((VPTR)nxp74lv595_pin_struct, 0, SIZEOF(nxp74lv595_pin_struct));
    nxp74lv595_pin_count = 0;
    
    MEMSET((VPTR)nxp74lv595_device_struct, 0, SIZEOF(nxp74lv595_device_struct));
    nxp74lv595_device_count = 0;
}
//------------------------------------------------------------------------------------------------//
NXP_74LV595_HNDL DrvGpioNxp74LV595_RegisterShiftRegister(DRVGPIO_PIN_HNDL ds_pin_hndl,
                                                         DRVGPIO_PIN_HNDL shcp_pin_hndl,
                                                         DRVGPIO_PIN_HNDL stcp_pin_hndl)
{
    NXP74LV595_DATA_STRUCT*     device_struct_ptr = &nxp74lv595_device_struct[nxp74lv595_device_count];
    
    if(nxp74lv595_device_count < DRVGPIONXP74LV595_COUNT)
    {
        device_struct_ptr->ds_pin_hndl      = ds_pin_hndl;
        device_struct_ptr->shcp_pin_hndl    = shcp_pin_hndl;
        device_struct_ptr->stcp_pin_hndl    = stcp_pin_hndl;
        DrvGpio_SetPin(ds_pin_hndl, FALSE);
        DrvGpio_SetPin(shcp_pin_hndl, FALSE);
        DrvGpio_SetPin(stcp_pin_hndl, FALSE);
        device_struct_ptr->data             = 0;
        nxp74lv595_device_count++;
        return (NXP_74LV595_HNDL)(nxp74lv595_device_count-1);
    }
    return INVALID_NXP_74LV595_HNDL;
}
//------------------------------------------------------------------------------------------------//
DRVGPIO_PIN_HNDL DrvGpioNxp74LV595_RegisterPin(NXP_74LV595_HNDL nxp_74lv595_hndl, U8 pin_nr)
{
    DRVGPIO_PIN_HNDL        pin_hndl;
    U16                     pin_id = (((U16)nxp_74lv595_hndl) << 8) | pin_nr;

    //check all registred pins and check if it has been registred yet
    for(pin_hndl = nxp74lv595_pin_struct; pin_hndl < &nxp74lv595_pin_struct[nxp74lv595_pin_count]; pin_hndl++)
    {
        if(pin_hndl->pin_id == pin_id)
        {
            LOG_WRN("re-register port %d pin %d", PU8(io_port), PU8(io_pin_nr));
            return pin_hndl;  //same pin_id found, return same handle
        }
    }

    if(nxp74lv595_pin_count < DRVGPIONXP74LV595_PIN_COUNT)
    {
        pin_hndl->pin_id = pin_id;
        pin_hndl->hook_list_ptr = &nxp74lv595_hook_list;
        nxp74lv595_pin_count++;
        return pin_hndl;
    }
    
    LOG_ERR("Pin register count overrun");
    return NULL;   //pin hndl null means no pin
}
//================================================================================================//