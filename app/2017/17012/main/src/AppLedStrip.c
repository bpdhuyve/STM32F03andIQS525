//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file controle the ledstrips mounted in the demo casing
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define APP_LED_STRIP_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef APP_LED_STRIP_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               APP_LED_STRIP_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
//SYS
#include "gpio\SysPin.h"
//DRV lib include section
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
#include "sci\DrvSciChannel.h"
#include "sci\DrvSciChannelSysInt.h"
//STD lib include section
//COM lib include section
#include "dmx\CommDllDmx.h"
//APP include section
#include "AppLedStrip.h"

//COMM
#include "dmx\CommDllDmx.h"

//#define Q_PREFIX(postfix)                   Dmx_##postfix
//#define Q_SIZE                              10
//#include "sci\DrvSciQTxTpl.h"
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
static void Task_Write_RGB(VPTR data_ptr);
static void EventHandler_ChangeTxPinMode(TX_PIN_MODE mode);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static DRVGPIO_PIN_HNDL     rs485_de;
static SCI_CHANNEL_HNDL     rs485_channel_hndl;

static U8  rgbarray[4] = {0,0,0};     //3bytes data to dmx bakske b0=red b1=green b2=blue
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void Task_Write_RGB(VPTR data_ptr)
{
    CommDllDmx_SendDMXdata(rgbarray,3);
}
//------------------------------------------------------------------------------------------------//
void EventHandler_ChangeTxPinMode(TX_PIN_MODE mode)
{
    if (mode==TX_PIN_MODE_GPIO_OUT_LOW)
    {
        SysPin_InitPin(GPIO_PORT_A, 9,PIN_OUTPUT);
    }
    else
    {
        SysPin_InitPin(GPIO_PORT_A, 9,PIN_ALTERNAT_SCI_TX);
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void AppLedStrip_Init(void)
{
    MODULE_INIT_ONCE();

    //dmx pins init
//    SysPin_InitPin(GPIO_PORT_1, 15,PIN_ALTERNAT_P1_15_U2_TXD);
//    SysPin_InitPin(GPIO_PORT_1, 16,PIN_ALTERNAT_P1_16_U2_RXD);

    //pinnen zitten op de zelfde poort als de logger
    SysPin_InitPin(GPIO_PORT_A,  9, (SYS_PIN_FUNC) PIN_ALTERNAT_SCI_TX); // LOG TX
    SysPin_InitPin(GPIO_PORT_A, 10, (SYS_PIN_FUNC) PIN_ALTERNAT_SCI_RX); // LOG RX

    //register RS485 channel
    rs485_de = DrvGpioSys_RegisterPin(GPIO_PORT_A, 11, PIN_OUTPUT);
    rs485_channel_hndl = DrvSciChannelSysInt_Register(SCI_CHANNEL_USART1);
    CommDllDmx_Init(rs485_channel_hndl,rs485_de,EventHandler_ChangeTxPinMode);

    CoreTask_Start(CoreTask_RegisterTask(15e3, Task_Write_RGB, NULL, 200, ""));    //we sturen de led waarde zoiezo door iedere 30 ms want als we dit neit doen stop da dmx bakske met de leds aan te sturen

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void AppLedStrip_SetRgb(COLOR_RGB color)
{
    //mvbl2: led strip is not connected correctly in the swipe 0 demo
    rgbarray[1] = color.red;
    rgbarray[2] = color.green;
    rgbarray[0] = color.blue;
}
//================================================================================================//
