//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Header of the Sensirion SHT11 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SENSOR__DRVSENSORSENSIRIONSHT11_H
#define SENSOR__DRVSENSORSENSIRIONSHT11_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "gpio\DrvGpio.h"
#include "sensor\DrvSensor.h"
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
// @brief	inits this module
void DrvSensorSensirionSht11_Init(void);

// @brief   register sensor
// @remark  the data_hndl must be able to switch between input and output!!
SENSOR_HNDL DrvSensorSensirionSht11_Register(DRVGPIO_PIN_HNDL clk_hndl, DRVGPIO_PIN_HNDL data_hndl);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /*SENSOR__DRVSENSORSENSIRIONSHT11_H*/
