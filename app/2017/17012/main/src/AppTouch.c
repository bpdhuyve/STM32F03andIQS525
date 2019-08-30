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
#include <stdlib.h>
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
static U8               evenOrNot = 0;
static U8               timer = 50;
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
    U8 testdata[4] = {0x04, 0x035, 0xAB, 0x76};
    U8 stop[3] = {0xEE, 0xEE, 0xEE};
    
    //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 2, TRUE, 0x0435);
    //DrvI2cMasterDevice_WriteData(i2c_device_id, testdata, 4, TRUE);
    //DrvI2cMasterDevice_WriteData_specificSlaveRegister(i2c_device_id, stop, 2, TRUE, 0x0433);
    //DrvI2cMasterDevice_WriteData(i2c_device_id, testdata, 2, TRUE);
    //DrvI2cMasterDevice_WriteData_specificSlaveRegister(i2c_device_id, stop, 1, TRUE, 0xEEEE);
    
    
    // Wake i2C and wait at least 150us
    DrvI2cMasterDevice_WriteData(i2c_device_id, data, 1, TRUE);
    while (timer>0)
    {
      timer--; // timer is 200 at start, takes around 170us to count to zero
    }
    // Threshold settings data
    data[0] = 0x22; //THRESHOLD_SETTINGS;
    data[1] = 0x33; //PROXTHRESHOLD_VAL;			// Prox Threshold
    data[2] = 0x44; // TOUCHMULTIPLIER_VAL;			// Touch Multiplier
    data[3] = 0x55; //TOUCHSHIFTER_VAL;				// Touch Shifter
    data[4] = PMPROXTHRESHOLD_VAL;			// PM Prox Threshold
    data[5] = (unsigned char)(SNAPTHRESHOLD_VAL>>8);	// Snap threshold
    data[6] = (unsigned char)SNAPTHRESHOLD_VAL;		// Snap threshold
    data[7] = PROXTHRESHOLD2_VAL;		        // Non-trackpad channels prox threshold
    data[8] = TOUCHMULTIPLIER2_VAL;			// Non-trackpad channels Touch Multiplier
    data[9] = TOUCHSHIFTER2_VAL;		        // Non-trackpad channels Touch Shifter
       
    DrvI2cMasterDevice_WriteData(i2c_device_id, data, 4, TRUE);
        
    
    U8 data2[10];
    // Threshold settings data
    data2[0] = 0x04;
    data2[1] = 0x35;			// Prox Threshold
    data2[2] = 0x41;			// Touch Multiplier
    data2[3] = 0x51;				// Touch Shifter
    data2[4] = 0x61;			// PM Prox Threshold
    data2[5] = (unsigned char)(SNAPTHRESHOLD_VAL>>8);	// Snap threshold
    data2[6] = (unsigned char)SNAPTHRESHOLD_VAL;		// Snap threshold
    data2[7] = PROXTHRESHOLD2_VAL;		        // Non-trackpad channels prox threshold
    data2[8] = TOUCHMULTIPLIER2_VAL;			// Non-trackpad channels Touch Multiplier
    data2[9] = 0x99;		        // Non-trackpad channels Touch Shifter
    //DrvI2clMasterDevice_WriteData(i2c_device_id, &data2[0], 2, TRUE);
  
//    write to address 0x0433
//      if(DrvI2cMasterDevice_WriteData(i2c_device_id, &data2[0], 4, TRUE))
//      {
//      MEMSET((VPTR)data_buffer, 0, SIZEOF(data_buffer));
//      DrvI2cMasterDevice_ReadData(i2c_device_id, data_buffer, 10, TRUE);
//      }
//    
//    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 4, TRUE, 0x0435);
//    DrvI2cMasterDevice_WriteData(i2c_device_id, &data[0], 4, TRUE);
//    DrvI2cMasterDevice_WriteData(i2c_device_id, &data[0], 4, TRUE);
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void AppTouch_Init(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
    MODULE_INIT_ONCE();
    U8 data[2] = {0,0};
    i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, address, 100000);
    
    //drvI2C nog aanpassen zodat read met repeated start altijd doet en niet stop start...
    //if(DrvI2cMasterDevice_WriteData(i2c_device_id, data, 1, TRUE))
    //{
    //  MEMSET((VPTR)data_buffer, 0, SIZEOF(data_buffer));
    //  DrvI2cMasterDevice_ReadData(i2c_device_id, data_buffer, 10, TRUE);
    //}
    //MEMSET((VPTR)data_buffer, 0, SIZEOF(data_buffer));
    
    AppTouch_settings();
    //DrvI2cMasterDevice_WriteData(i2c_device_id, stop, 1, TRUE);
    //DrvI2cMasterDevice_WriteData(i2c_device_id, stop, 1, TRUE);
    //DrvI2cMasterDevice_ReadData(i2c_device_id, data_buffer, 10, TRUE);

    //DrvI2cMasterDevice_WriteData(i2c_device_id, (U8*)(0xE8 >> 1), 1, TRUE);
    //DrvI2cMasterDevice_ReadData(i2c_device_id, data_buffer, 10, TRUE);
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
  
    //U8 testdata[2] = {0xAB, 0x76};
    //DrvI2cMasterDevice_WriteData(i2c_device_id, testdata, 4, TRUE);
    //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 2, TRUE, 0x0435);
  
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
U8 AppTouch_GetTouch2(U16* x, U16* y)
{
  
  U8 testdata[4] = {0x06, 0x73, 0xAA, 0xBB};
  U8 defaultReg[4] = {0x66, 0x88, 0x099, 0x55};
    if (evenOrNot % 2 == 0)
      
    {
      if (evenOrNot == 0)
      {
        DrvI2cMasterDevice_WriteData_specificSlaveRegister(i2c_device_id, defaultReg, 4, TRUE,0x0673);
      }
      else
      {
        DrvI2cMasterDevice_WriteData(i2c_device_id, testdata, 4, TRUE);
      }
    }
    else if (evenOrNot > 1)
    {
      DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 2, TRUE, 0x0673);
    }
    else
    {
    }
    
    evenOrNot = evenOrNot % 254;
    evenOrNot++;
    
    return 0;
}
//================================================================================================//
