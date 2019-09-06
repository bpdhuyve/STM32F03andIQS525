//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Application main source file
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define APPMAIN_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef APPMAIN_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               APPMAIN_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "gpio\SysPin.h"

// DRV
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
#include "sci\DrvSciChannel.h"
#include "sci\DrvSciChannelSysInt.h"
#include "i2c\DrvI2cMasterChannel.h"
#include "i2c\DrvI2cMasterChannelSysInt.h"
#include "i2c\DrvI2cMasterDevice.h"

// APP
#include "AppMain.h"
#include "AppLeds.h"
#include "AppLogic.h"
#include "AppTouch.h"
#include "AppLedStrip.h"
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
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void AppMain_Init(void)
{
    MODULE_INIT_ONCE();

    RCC_LSEConfig(RCC_LSE_OFF);
    I2C_CHANNEL_HNDL    i2c_channel;

    // --- SYSTEM ---
    // SYS PIN INIT
//
//    SysPin_InitPin(GPIO_PORT_A,  9, (SYS_PIN_FUNC) PIN_ALTERNAT_SCI_TX); // LOG TX
//    SysPin_InitPin(GPIO_PORT_A, 10, (SYS_PIN_FUNC) PIN_ALTERNAT_SCI_RX); // LOG RX

    //pull up for i2c is not present on hardware board!!!!!!!!
    SysPin_InitPin(GPIO_PORT_F, 7, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SDA_ON_ | PIN_ALTERNAT_FUNCTION_0 | PIN_INPUT_PULL_UP));  // I2C1 SDA
    SysPin_InitPin(GPIO_PORT_F, 6, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SCL_ON_ | PIN_ALTERNAT_FUNCTION_0 | PIN_INPUT_PULL_UP));  // I2C1 SCL
    SysPin_InitPin(GPIO_PORT_A, 3, (SYS_PIN_FUNC)(PIN_OUTPUT));         // IQS NRST
    SysPin_InitPin(GPIO_PORT_A, 2, (SYS_PIN_FUNC)(PIN_INPUT_PULL_UP));  // IQS RDY

//    SysPin_InitPin(GPIO_PORT_F, 7, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SDA_ON_ | PIN_ALTERNAT_FUNCTION_0));  // I2C1 SDA
//    SysPin_InitPin(GPIO_PORT_F, 6, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SCL_ON_ | PIN_ALTERNAT_FUNCTION_0));  // I2C1 SCL

//    SysPin_InitPin(GPIO_PORT_B, 10, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SCL_ON_|PIN_ALTERNAT_FUNCTION_1));          // I2C2 SCL
//    SysPin_InitPin(GPIO_PORT_B, 11, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SDA_ON_|PIN_ALTERNAT_FUNCTION_1));          // I2C2 SDA

//    SysPin_InitPin(GPIO_PORT_A, 11, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SCL_ON_|PIN_ALTERNAT_FUNCTION_5));          // I2C2 SCL
//    SysPin_InitPin(GPIO_PORT_A, 12, (SYS_PIN_FUNC)(PIN_ALTERNAT_I2C_SDA_ON_|PIN_ALTERNAT_FUNCTION_5));          // I2C2 SDA

    // --- DRIVER ---
    // GPIO
    DrvGpio_Init();
    DrvGpioSys_Init();

    // SCI
    DrvSciChannel_Init();
    DrvSciChannelSysInt_Init();

    // I2C
    DrvI2cMasterChannel_Init();
    DrvI2cMasterChannelSysInt_Init();
    DrvI2cMasterDevice_Init();

    // --- CORE ---
    // DEBUG
    //geen debug zit samen op de rs485 Core_DebugSetSciChannel(DrvSciChannelSysInt_Register(SCI_CHANNEL_USART1));
    //Core_DebugSetSciChannel(DrvSciChannelSysInt_Register(SCI_CHANNEL_USART1));

    // --- APPLICATION ---


//    i2c_channel = DrvI2cMasterChannelSysInt_Register(I2C_CHANNEL_2);
    i2c_channel = DrvI2cMasterChannelSysInt_Register(I2C_CHANNEL_1);
    AppTouch_Init(i2c_channel, (0xE8 >> 1));   // default device address is 0xE8
 // AppTouch_Init(i2c_channel, (0x68 ));



/*
    COLOR_RGB color_red = {150,0,0};
    COLOR_RGB color_green = {0,150,0};
    COLOR_RGB color_blue = {0,0,150};
    AppLeds_SetLed(LED_1,color_red);
    AppLeds_SetLed(LED_2,color_green);
    AppLeds_SetLed(LED_3,color_blue);
    AppLeds_SetLed(LED_4,color_red);
    AppLeds_SetLed(LED_5,color_green);
    AppLeds_SetLed(LED_6,color_blue);
    AppLeds_SetLed(LED_7,color_red);
    AppLeds_SetLed(LED_8,color_green);
    AppLeds_SetLed(LED_9,color_blue);
    AppLeds_SetLed(LED_10,color_red);
    AppLeds_SetLed(LED_11,color_green);
    AppLeds_SetLed(LED_12,color_blue);
*/

    AppLeds_Init();
    AppLedStrip_Init();
    AppLogic_Init();

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void AppMain_Handler(void)
{
    MODULE_CHECK();
    AppTouch_Handler();

}
//------------------------------------------------------------------------------------------------//
void AppMain_OnErrorReport(void)
{

}
//================================================================================================//
