//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Sensirion SDP6xx/5xx sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SENSOR__DRVSENSORSENSIRIONSDP6XX5XX_H
#define SENSOR__DRVSENSORSENSIRIONSDP6XX5XX_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "sensor\DrvSensor.h"
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
// @remark  Init function
void DrvSensorSensirionSdp6xx5xx_Init(void);

// @remark  Register function
SENSOR_HNDL DrvSensorSensirionSdp6xx5xx_Register(I2C_CHANNEL_HNDL i2c_channel);
//================================================================================================//



#endif /* SENSOR__DRVSENSORSENSIRIONSDP6XX5XX_H */