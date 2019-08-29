//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This module determines the board type
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBGPIO_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBGPIO_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBGPIO_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if all functions or only mask functions should be shown
#ifndef TESTLIBGPIO_INCLUDE_PIN_COMMANDS
	#define TESTLIBGPIO_INCLUDE_PIN_COMMANDS       1
#endif
// @brief  Defines if the pin functionality should get exposed
#ifndef TESTLIBGPIO_INCLUDE_RAW_INIT_COMMAND
    #define TESTLIBGPIO_INCLUDE_RAW_INIT_COMMAND   0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DUCO LIB
#include "TestLibGpio.h"

// DRV
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
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
#if (TESTLIBGPIO_INCLUDE_PIN_COMMANDS > 0)
void Command_GpioInit(void);
void Command_GpioSet(void);
void Command_GpioGet(void);
#endif
#if (TESTLIBGPIO_INCLUDE_RAW_INIT_COMMAND > 0)
static void Command_GpioRawInit(void);
#endif
static void Command_GpioInitMask(void);
static void Command_GpioSetMask(void);
static void Command_GpioGetMask(void);
BOOL CommandAid_GetPort(U8 argumentNumber, GPIO_PORT* port_ptr);
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
#if (TESTLIBGPIO_INCLUDE_PIN_COMMANDS > 0)
void Command_GpioInit(void)
{
    SYS_PIN_FUNC tempFunction;

    if (CoreTerm_GetArgumentAsU32(2) == 0)
    {
        tempFunction = PIN_INPUT;
    }
    else
    {
        tempFunction = PIN_OUTPUT;
    }
    GPIO_PORT port;
    CommandAid_GetPort(0,&port);
    SysPin_InitPin(port, CoreTerm_GetArgumentAsU32(1), tempFunction);
    CoreTerm_PrintAcknowledge();
}
//--------------------------------------------------------------------------------------------------------------------//
void Command_GpioSet(void)
{
    GPIO_PORT port;
    CommandAid_GetPort(0,&port);
    U32 pinmask = (1<<(CoreTerm_GetArgumentAsU32(1)));

    SysGpio_SetPinMask(port, pinmask, (BOOL)CoreTerm_GetArgumentAsU32(2));
    CoreTerm_PrintAcknowledge();
}
//--------------------------------------------------------------------------------------------------------------------//
void Command_GpioGet(void)
{
    GPIO_PORT port;
    CommandAid_GetPort(0,&port);
    U32 pinmask = (1<<(CoreTerm_GetArgumentAsU32(1)));

    if ((SysGpio_GetPinMask(port,pinmask)) == pinmask)
    {
        CoreTerm_PrintBool(TRUE);
    }
    else
    {
        CoreTerm_PrintBool(FALSE);
    }
}
#endif
//------------------------------------------------------------------------------------------------//
#if (TESTLIBGPIO_INCLUDE_RAW_INIT_COMMAND > 0)
static void Command_GpioRawInit(void)
{
    SYS_PIN_FUNC tempFunction = (SYS_PIN_FUNC)CoreTerm_GetArgumentAsU32(2);

    GPIO_PORT port;
    CommandAid_GetPort(0,&port);
    SysPin_InitPin(port, CoreTerm_GetArgumentAsU32(1), tempFunction);
    CoreTerm_PrintAcknowledge();
}
//--------------------------------------------------------------------------------------------------------------------//
#endif
static void Command_GpioInitMask(void)
{
    SYS_PIN_FUNC    tempFunction;
    U8              argnb = 1;
    GPIO_PORT       gpio_port;

    if(CoreTerm_GetArgumentAsU32(0) == 0)
    {
        tempFunction = PIN_INPUT;
    }
    else
    {
        tempFunction = PIN_OUTPUT;
    }

    LOG_TRM("GPIO INIT %d", PU8(CoreTerm_GetArgumentAsU32(0)));
    while(CommandAid_GetPort(argnb, &gpio_port))
    {
        argnb++;
        LOG_TRM("PORT %c : 0x%08h", PCHAR('A' + gpio_port), PU32(CoreTerm_GetArgumentAsU32(argnb)));
        SysPin_InitPinMask(gpio_port, CoreTerm_GetArgumentAsU32(argnb), tempFunction);
        argnb++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_GpioSetMask(void)
{
    BOOL            set_high = (BOOL)(CoreTerm_GetArgumentAsU32(0) > 0);
    U8              argnb = 1;
    GPIO_PORT       gpio_port;

    LOG_TRM("GPIO SET %d", PU8(set_high));
    while(CommandAid_GetPort(argnb, &gpio_port))
    {
        argnb++;
        LOG_TRM("PORT %c : 0x%08h", PCHAR('A' + gpio_port), PU32(CoreTerm_GetArgumentAsU32(argnb)));
        SysGpio_SetPinMask(gpio_port, CoreTerm_GetArgumentAsU32(argnb), set_high);
        argnb++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_GpioGetMask(void)
{
    U8              argnb = 0;
    GPIO_PORT       gpio_port;

    LOG_TRM("GPIO GET");
    while(CommandAid_GetPort(argnb, &gpio_port))
    {
        LOG_TRM("PORT %c : 0x%08h", PCHAR('A' + gpio_port), PU32(SysGpio_GetPinMask(gpio_port, 0xFFFFFFFF)));
        argnb++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
BOOL CommandAid_GetPort(U8 argumentNumber, GPIO_PORT* port_ptr)
{
    CHAR tempstring[20];

    CoreTerm_GetArgumentAsString(argumentNumber, tempstring);

    if(tempstring[0] == 0)
    {
        return FALSE;
    }

    if (tempstring[0] >= 'a')   //port argument is supplied as letter
    {
        *port_ptr = (GPIO_PORT)(tempstring[0] - 'a');
    }
    else    //argument is supplied as number, certain processors indiacte by numbers (nxp)
    {
        *port_ptr = (GPIO_PORT)CoreTerm_GetArgumentAsU32(argumentNumber);
    }
    return TRUE;
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibGpio_Init(void)
{
    MODULE_INIT_ONCE();
/*
#if (TESTLIBGPIO_INCLUDE_PIN_COMMANDS > 0)
    CoreTerm_RegisterCommand("GpioInit","init a gpio pin (a=portnr, b=pinnr, c=function 0:input 1:output)",3,Command_GpioInit,TRUE);
    CoreTerm_RegisterCommand("GpioSet","sets a gpio pin (a=portnr, b=pinnr, c=value)",3,Command_GpioSet,TRUE);
    CoreTerm_RegisterCommand("GpioGet","gets a gpio pin (a=portnr, b=pinnr)",2,Command_GpioGet,TRUE);
#endif
#if (TESTLIBGPIO_INCLUDE_RAW_INIT_COMMAND > 0)
    CoreTerm_RegisterCommand("GpioRawInit","init a pin (a=portnr, b=pinnr, c=function (0-7: see user manual))",3,Command_GpioRawInit,TRUE);
#endif
    CoreTerm_RegisterCommand("GpioInitMasks", "inits mask c on port b to a (a=0:input,1:output), supply var args as 'a1 b1 c1 b2 c2 ...'", CMD_VAR_ARGUMENTS, Command_GpioInitMask, TRUE);
    CoreTerm_RegisterCommand("GpioSetMasks", "sets mask c on port b to a (a=0:low,1:high), supply var args as 'a1 b1 c1 b2 c2 ...'", CMD_VAR_ARGUMENTS, Command_GpioSetMask, TRUE);
    CoreTerm_RegisterCommand("GpioGetMasks", "get the complete port mask (a=port), supply var arg as 'a1 a2 ...'", CMD_VAR_ARGUMENTS, Command_GpioGetMask, TRUE);
*/
    MODULE_INIT_DONE();
}
//================================================================================================//

