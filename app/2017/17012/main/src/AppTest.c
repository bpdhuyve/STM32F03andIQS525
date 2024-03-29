//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Application main source file
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define APPTEST_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef APPTEST_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               APPTEST_LOG_LEVEL
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

// TEST LIB
#include "TestLibGpio.h"

// COMM

// APP
#include "AppTest.h"
#include "AppLeds.h"
#include "AppLedStrip.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
static TASK_HNDL	            task_hndl_LedAnimator = NULL;
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void Task_LedAnimator(VPTR data_ptr);
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
static void Task_LedAnimator(VPTR data_ptr)
{
    //test to check if all leds are working
    COLOR_RGB wit = {255,255,255};
    COLOR_RGB zwart = {0,0,0} ;

    AppLeds_SetLed(LED_ALL,zwart);

    static U8 led_on = 0;
    AppLeds_SetLed((LEDS) (1<<led_on),wit);
    led_on++;

    if (led_on >=12)
    {
        led_on = 0;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void AppTest_Init(void)
{
    MODULE_INIT_ONCE();

    RCC_LSEConfig(RCC_LSE_OFF);

    // --- SYSTEM ---
    // SYS PIN INIT

    SysPin_InitPin(GPIO_PORT_A,  9, (SYS_PIN_FUNC) PIN_ALTERNAT_SCI_TX);           // LOG TX
    SysPin_InitPin(GPIO_PORT_A, 10, (SYS_PIN_FUNC) PIN_ALTERNAT_SCI_RX);           // LOG RX

    // --- DRIVER ---
    // GPIO
    DrvGpio_Init();
    DrvGpioSys_Init();

    // SCI
    DrvSciChannel_Init();
    DrvSciChannelSysInt_Init();


    // --- CORE ---
    // DEBUG
    Core_DebugSetSciChannel(DrvSciChannelSysInt_Register(SCI_CHANNEL_USART1));

    // --- TESTLIB/APPLICATION ---
    TestLibGpio_Init();

    MODULE_INIT_DONE();

    AppLeds_Init();
    AppLedStrip_Init();
    task_hndl_LedAnimator = CoreTask_RegisterTask(500e3, Task_LedAnimator, NULL, 200, "Task_LedAnimator");
    CoreTask_Start(task_hndl_LedAnimator);
}
//------------------------------------------------------------------------------------------------//
void AppTest_Handler(void)
{
    MODULE_CHECK();
}
//------------------------------------------------------------------------------------------------//
void AppTest_OnErrorReport(void)
{

}
//------------------------------------------------------------------------------------------------//
BOOL AppTest_CheckEnterTestmode(void)
{
    return FALSE;
}
//================================================================================================//
