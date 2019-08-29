//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Application main header file.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRV_SLIDER_SX864x_H
#define DRV_SLIDER_SX864x_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//#include "sys.h"
#include "i2c\DrvI2cMasterChannel.h"
#include "i2c\DrvI2cMasterChannelSysInt.h"
#include "i2c\DrvI2cMasterDevice.h"
#include "slider\DrvSlider.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DEFAULT_SEMTECH_SLAVE_ADDRESS       0x2B
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
/// brief explanation.
/// @param  "i2c_channel" handle of the i2c channel where the semtec chip is located on, channel must be inited
/// @param  "address" i2c address of the semtec chip on the i2c bus
/// @return slider handel to use the slider with the functions in DrvSlider.h
/// @remark warning, for now using multiple chips in one setup will not work, internally the slider id needs to be used therefor
SLIDER_HNDL DrvSliderSx864x_Register(I2C_CHANNEL_HNDL i2c_channel, U8 address);

/// function to just test if there is a Sx864x with that adress connected to the i2c channel
/// @param  "i2c_channel" handle of the i2c channel where the semtec chip is located on, channel must be inited
/// @param  "address" i2c address of the semtec chip on the i2c bus
/// @return TRUE if connection is ok, FALSE if not
/// @remark function can be performed before DrvSliderSx864x_Register or stand alone!!!
BOOL DrvSliderSx864x_TestConnection(I2C_CHANNEL_HNDL i2c_channel, U8 address);
//================================================================================================//

#endif /* DRV_SLIDER_SX864x_H */