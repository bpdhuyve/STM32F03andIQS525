//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Driver for the LCD display 64128M
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRVLCDDISPLAY64128M_H
#define DRVLCDDISPLAY64128M_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
// SYS
#include "core\SysTypes.h"

// DRV
#include "gpio\DrvGpio.h"
#include "timer\DrvPwm.h"
#include "spi\DrvSpiMasterChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define PAGES                               8
#define COLUMNS                             128
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
/// @param   "frame_data_ptr_ptr" : the address of a pointer to the frame data (frame_data_ptr), 
///          the frame data needs to be stored in a two-dimensional array of size [PAGES][COLUMNS].
/// @remark  After changing the frame_data_ptr, call the function DrvLcdDisplay64128M_Refresh() to 
///          put the new data pointed to by frame_data_ptr on the display.
void DrvLcdDisplay64128M_Init(SPI_CHANNEL_HNDL spi_channel_hndl,
                              U32              spi_speed,
                              PWM_CHANNEL_HNDL backlight_pwm_hndl,
                              DRVGPIO_PIN_HNDL chip_select_pin_hndl,
                              DRVGPIO_PIN_HNDL reset_pin_hndl,
                              DRVGPIO_PIN_HNDL a0_pin_hndl,
                              const U8         (**frame_data_ptr_ptr)[PAGES][COLUMNS]);

/// @brief  After changing the frame_data_ptr, call the function DrvLcdDisplay64128M_Refresh() to 
///         put the new data pointed to by frame_data_ptr on the display.
void DrvLcdDisplay64128M_Refresh(void);

void DrvLcdDisplay64128M_SetBrightness(U8 brightness);

void DrvLcdDisplay64128M_SetBacklight(BOOL state);
//================================================================================================//



#endif /* DRVLCDDISPLAY64128M_H */
