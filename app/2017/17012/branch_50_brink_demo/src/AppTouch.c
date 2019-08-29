//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Application main source file
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define APPTOUCH_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef APPTOUCH_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               APPTOUCH_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "gpio\SysPin.h"

// DRV

// APP
#include "AppTouch.h"
#include "IQS5xx.h"
#include "IQS5xx_Init.h"

//other...
#include "i2c\SysI2cMasterInt.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void AppTouch_settings(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static I2C_DEVICE_ID	i2c_device_id;
static U8	        data_buffer[60];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void AppTouch_settings(void)
{
    //reset...
    U8 data[10];

    // Threshold settings data
    data[0] = THRESHOLD_SETTINGS;
    data[1] = 0x0F;     //PROXTHRESHOLD_VAL;			// Prox Threshold
    data[2] = 0x08;     //TOUCHMULTIPLIER_VAL;			// Touch Multiplier
    data[3] =  0x06;     //TOUCHSHIFTER_VAL;				// Touch Shifter
    data[4] =  0x0A;     //PMPROXTHRESHOLD_VAL;			// PM Prox Threshold
    #define SNAPTHRESHOLD_VAL		0x32
    data[5] = (unsigned char)(SNAPTHRESHOLD_VAL>>8);	// Snap threshold
    data[6] = (unsigned char)SNAPTHRESHOLD_VAL;		// Snap threshold
    data[7] =  0x0A;     //PROXTHRESHOLD2_VAL;		        // Non-trackpad channels prox threshold
    data[8] =  0x05;     //TOUCHMULTIPLIER2_VAL;			// Non-trackpad channels Touch Multiplier
    data[9] =  0x07;     //TOUCHSHIFTER2_VAL;		        // Non-trackpad channels Touch Shifter

    DrvI2cMasterDevice_WriteData(i2c_device_id, &data[0], 10, TRUE);
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void AppTouch_Init(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
    MODULE_INIT_ONCE();
//    U8 data[2] = {0,0};

     i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, address, 100000);

       //drvI2C nog aanpassen zodat read met repeated start altijd doet en niet stop start...
       //blijkbaar is dit een probleem...
//    if(DrvI2cMasterDevice_WriteData(i2c_device_id, data, 1, TRUE))
//    {
//        MEMSET((VPTR)data_buffer, 0, SIZEOF(data_buffer));
//        DrvI2cMasterDevice_ReadData(i2c_device_id, data_buffer, 2, TRUE);
//    }

    AppTouch_settings();

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void AppTouch_Handler(void)
{
    MODULE_CHECK();
}
//------------------------------------------------------------------------------------------------//
U8 AppTouch_GetTouch(U16* x, U16* y)
{
    //just read data it will be xy data ...
    if(DrvI2cMasterDevice_ReadData(i2c_device_id, &data_buffer[0], 8, TRUE))
    {
        *x = ((data_buffer[2] << 8) + data_buffer[3]);
        *y = ((data_buffer[4] << 8) + data_buffer[5]);
        return (data_buffer[0] & 0x0F);
    }
    return 0;
}
//================================================================================================//
