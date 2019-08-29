//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Prototypes of GPIO interface using Sx864x
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRV_SLIDER_SX864x_H
#define DRV_SLIDER_SX864x_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "gpio\DrvGpio.h"
#include "i2c\DrvI2cMasterChannel.h"
//================================================================================================//

//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvGpioSx864x_Init(I2C_CHANNEL_HNDL i2c_channel);

DRVGPIO_PIN_HNDL DrvGpioSx864x_Register(U8 button_nr, SYS_PIN_FUNC pin_func, BOOL is_buffered);
//================================================================================================//

#endif /* DRV_SLIDER_SX864x_H */