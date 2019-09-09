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

    
    U8 separator[4] = {0xEE,0xEE,0x00,0x01};
    U8 config[4] = {0x05,0x8E,0x6C,0x24};
    
    //----
    
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 6, FALSE, 0x0000);
    DrvI2cMasterDevice_WriteData(i2c_device_id, separator, 4, FALSE);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 100, FALSE, 0x0431);
    DrvI2cMasterDevice_WriteData(i2c_device_id, separator, 4, FALSE);
    
    //----
    
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 46, TRUE, 0x000B);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 20, TRUE, 0x0039);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 30, TRUE, 0x0059);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 30, TRUE, 0x0077);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 100, TRUE, 0x0095);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 1, TRUE, 0xEEEE);
      
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 46, TRUE, 0x000B);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 20, TRUE, 0x0039);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 30, TRUE, 0x0059);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 30, TRUE, 0x0077);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 100, TRUE, 0x0095);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 1, TRUE, 0xEEEE);
      
    //----
    
    DrvI2cMasterDevice_WriteData(i2c_device_id, config, 4, TRUE);   // write system config 0 and 1
    
    //----
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 46, TRUE, 0x000B);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 20, TRUE, 0x0039);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 30, TRUE, 0x0059);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 30, TRUE, 0x0077);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 100, TRUE, 0x0095);
    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, big_buffer, 1, TRUE, 0xEEEE);
    
//    
//    
//    
//    
//    // touchpad threshold settings configuration
//    data[0] = 0xAA; //THRESHOLD_SETTINGS;                       // = 0x11, address for writing threshold settings, p32 datasheet
//    data[1] = 0xBB; //PROXTHRESHOLD_VAL;			// Prox Threshold
//    data[2] = 0xEE; //TOUCHMULTIPLIER_VAL;			// Touch Multiplier
//    data[3] = 0xEE; //TOUCHSHIFTER_VAL;				// Touch Shifter
//    data[4] = 0xBB;// PMPROXTHRESHOLD_VAL;			// PM Prox Threshold
//    data[5] = (unsigned char)(SNAPTHRESHOLD_VAL>>8);	// Snap threshold
//    data[6] = (unsigned char)SNAPTHRESHOLD_VAL;		// Snap threshold
//    data[7] = PROXTHRESHOLD2_VAL;		        // Non-trackpad channels prox threshold
//    data[8] = TOUCHMULTIPLIER2_VAL;			// Non-trackpad channels Touch Multiplier
//    data[9] = TOUCHSHIFTER2_VAL;		        // Non-trackpad channels Touch Shifter
//    
//    // Wake i2C and then wait at least 150us (if we don't wait 150us the device doesn't wake)
//    DrvI2cMasterDevice_WriteData_specificSlaveRegister(i2c_device_id, &data[0], 2, TRUE, 0x0433);
//    while (timer>0)
//    {
//      timer--; // timer is 200 at start, takes around 170us to count to zero
//    }
    
    
    
//    timer = 100;
//    DrvI2cMasterDevice_WriteData(i2c_device_id,&data[2], 3, TRUE); //,0xEEEE);
//    while (timer>0)
//    {
//      timer--; // timer is 200 at start, takes around 170us to count to zero
//    }
//    timer = 100;
//    DrvI2cMasterDevice_WriteData(i2c_device_id, &data[2], 1, TRUE);
//    while (timer>0)
//    {
//      timer--; // timer is 200 at start, takes around 170us to count to zero
//    }
//    timer = 100;
//    DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data, 2, TRUE,0x0433);
//    while (timer>0)
//    {
//      timer--; // timer is 200 at start, takes around 170us to count to zero
//    }
//    DrvI2cMasterDevice_WriteData(i2c_device_id, data, 10, TRUE);     
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
    
    AppTouch_settings();  // configure all parameters related to the touchpad
    
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
  
    //read  x,y data from the touchpad
    if(DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 10, TRUE, 0x0011) )//DrvI2cMasterDevice_ReadData(i2c_device_id, &data_buffer[0], 8, TRUE))
    {
        *x = ((data_buffer[5] << 8) + data_buffer[6]);
        *y = ((data_buffer[7] << 8) + data_buffer[8]); 
        return (TRUE); //data_buffer[0] & 0x0F);
    }
    

    return 0;
}

//================================================================================================//
// @ brief : setup default read register (for reading x,y data)
//  rest of loop is used to write and read to registers - only for testing purposes
U8 AppTouch_GetData(U16* x, U16* y)
{
  
  U8 testdata[5] = {0xAA, 0xBB, 0xCC};
  U8 defaultReg[4] = {0x00, 0x11, 0x099, 0x55};
    if (evenOrNot % 2 == 0)
      
    {
      if (evenOrNot == 0)
      {
        //DrvI2cMasterDevice_WriteData_specificSlaveRegister(i2c_device_id, defaultReg, 4, TRUE,0x056D); // write 2 bytes to 0x0673
        DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 4, TRUE,0x056D);  // read 3 bytes from 0x0673
      }
      else
      {
        DrvI2cMasterDevice_WriteData_specificSlaveRegister(i2c_device_id, data_buffer, 2, TRUE,0xEEEE);  // read 3 bytes from 0x0673
      }
    }
    else if (evenOrNot > 1 && evenOrNot < 4)
    {
      //DrvI2cMasterDevice_ReadData_repStart(i2c_device_id, data_buffer, 8, TRUE, 0x01);   
      //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 6, TRUE, 0x0000);
    }
    else
    {
    }
    
    //evenOrNot = evenOrNot % 254;
    if (evenOrNot == 5){
      evenOrNot--;
    }
    evenOrNot++;
    
    return 0;
}
//================================================================================================//
