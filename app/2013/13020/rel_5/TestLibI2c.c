//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Master i2c testlib version
// 
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBI2C_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBI2C_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBI2C_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "i2c\DrvI2cMasterChannel.h"
#include "i2c\DrvI2cMasterChannelSysInt.h"
#include "i2c\DrvI2cMasterDevice.h"

// APP
#include "TestLibI2c.h"
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
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static BOOL TestI2c_InitDevice(U8 address, U32 speed, I2C_CHANNEL i2c_channel);
static void TestI2c_MessageComplete(BOOL success);
static void Command_I2cDeviceInit(void);
static void Command_I2cDeviceReadRegister(void);
static void Command_I2cDeviceWriteRegister(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static BOOL TestI2c_InitDevice(U8 address, U32 speed, I2C_CHANNEL i2c_channel)
{   
    return DrvI2cMasterDevice_MsgComplete(DrvI2cMasterDevice_Register(DrvI2cMasterChannelSysInt_Register(i2c_channel),
                                                                      address,
                                                                      speed),
                                          TestI2c_MessageComplete);
}
//------------------------------------------------------------------------------------------------//
static void TestI2c_MessageComplete(BOOL success)
{
    CoreTerm_PrintFeedback(success);
}
//------------------------------------------------------------------------------------------------//
static void Command_I2cDeviceInit(void)
{
    CoreTerm_PrintFeedback(TestI2c_InitDevice((U8)CoreTerm_GetArgumentAsU32(0), CoreTerm_GetArgumentAsU32(1), (I2C_CHANNEL)CoreTerm_GetArgumentAsU32(2)));
}
//------------------------------------------------------------------------------------------------//
static void Command_I2cDeviceReadRegister(void)
{
    U8 i2c_device_id    = (U8)CoreTerm_GetArgumentAsU32(0);
    U8 register_address = (U8)CoreTerm_GetArgumentAsU32(1);
    U8 data_buffer;
    
    DrvI2cMasterDevice_WriteData(i2c_device_id, &register_address, 1, TRUE);
    DrvI2cMasterDevice_ReadData(i2c_device_id, &data_buffer, 1, TRUE);
    
    LOG_TRM("i2c device %d register 0x%02h: 0x%02h", PU8(i2c_device_id), PU8(register_address), PU8(data_buffer));
}
//------------------------------------------------------------------------------------------------//
static void Command_I2cDeviceWriteRegister(void)
{
    U8 i2c_device_id    = (U8)CoreTerm_GetArgumentAsU32(0);
    U8 data_buffer[2] = {(U8)CoreTerm_GetArgumentAsU32(1),
                         (U8)CoreTerm_GetArgumentAsU32(2)};
    
    CoreTerm_PrintFeedback(DrvI2cMasterDevice_WriteData(i2c_device_id, &data_buffer[0], 2, TRUE));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibI2c_Init(void)
{
    MODULE_INIT_ONCE();
    
    CoreTerm_RegisterCommand("I2cDeviceInit","inits an i2c device (a= slave address, b= speed, c= channel you want the device to connect to)", 3, Command_I2cDeviceInit, TRUE);
    CoreTerm_RegisterCommand("I2cDeviceReadRegister","reads a device register (a= device id, b= register address)", 2, Command_I2cDeviceReadRegister, TRUE);
    CoreTerm_RegisterCommand("I2cDeviceWriteRegister","write a device register (a= device id, b= register address, c= value)", 3, Command_I2cDeviceWriteRegister, TRUE);
    
    MODULE_INIT_DONE();
}
//================================================================================================//