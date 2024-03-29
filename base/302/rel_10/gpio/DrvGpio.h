//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// gpio abstraction driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef GPIO__DRVGPIO_H
#define GPIO__DRVGPIO_H
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
typedef enum
{
    GPIO_PIN_INPUT = 0,
    GPIO_PIN_OUTPUT = 1,
    GPIO_PIN_OUTPUT_START_HIGH = 2
}
GPIO_PIN_FUNCTION;

typedef void (*GPIO_RE_INIT_HOOK)(U16 pin_id, GPIO_PIN_FUNCTION function);

typedef void (*GPIO_SET_HOOK)(U16 pin_id, BOOL value);

typedef BOOL (*GPIO_GET_HOOK)(U16 pin_id);

typedef struct
{
    GPIO_RE_INIT_HOOK               re_init_hook;
    GPIO_SET_HOOK                   set_hook;
    GPIO_GET_HOOK                   get_hook;
}
GPIO_DRV_HOOK_LIST;

typedef struct
{
    U16                             pin_id;
    GPIO_DRV_HOOK_LIST*	            hook_list_ptr;
}
DRV_GPIO_PIN_STRUCT;

typedef DRV_GPIO_PIN_STRUCT*        DRVGPIO_PIN_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvGpio_Init(void);
void DrvGpio_SetPin(DRVGPIO_PIN_HNDL pin_hndl, BOOL value);
BOOL DrvGpio_GetPin(DRVGPIO_PIN_HNDL pin_hndl);

/// @remark the gpio implentation must support this, is you use DrvGpioSys you see explanation of DrvGpioSys_EnableSwitch
void DrvGpio_ReInitPin(DRVGPIO_PIN_HNDL pin_hndl, GPIO_PIN_FUNCTION function);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* GPIO__DRVGPIO_H */
