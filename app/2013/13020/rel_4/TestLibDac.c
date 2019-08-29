//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This module determines the board type
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBdac_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBDAC_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               TESTLIBDAC_LOG_LEVEL
#endif

#ifndef TESTLIBDAC_BULK_SAMPLES_SEPARATOR
    #define TESTLIBDAC_BULK_SAMPLES_SEPARATOR  "\r"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DUCO LIB
#include "TestLibDac.h"

// DRV
#include "dac\SysDac.h"
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
static void Command_DacChannelInit(void);
static void Command_DacChannelGet(void);
static void Command_DacChannelSet(void);
static void Command_DacChannelInitAndSet(void);
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
static void Command_DacChannelInit(void)
{
    DAC_CHANNEL         dac_channel = (DAC_CHANNEL)CoreTerm_GetArgumentAsU32(0);

    if(SysDac_Channel_Init(dac_channel))
    {
        CoreTerm_PrintAcknowledge();
    }
    else
    {
        CoreTerm_PrintFailed();
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_DacChannelSet(void)
{
    static U16 dac_conversion_result;
    DAC_CHANNEL dac_channel = (DAC_CHANNEL)CoreTerm_GetArgumentAsU32(0);
    U16 dac_value = CoreTerm_GetArgumentAsU32(1);
    
    if (!SysDac_Channel_SetValue(dac_channel, dac_value))
    {
        CoreTerm_PrintFailed();
        return;
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_DacChannelGet(void)
{
    static U16 dac_conversion_result;
    DAC_CHANNEL dac_channel = (DAC_CHANNEL)CoreTerm_GetArgumentAsU32(0);
    U16 dac_value = 0;
    
    if (SysDac_Channel_GetValue(dac_channel, &dac_value))
    {
        LOG_TRM("dac channel %d = %d", PU8(dac_channel), PU16(dac_value));
    }
    else
    {
        CoreTerm_PrintFailed();
        return;
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_DacChannelInitAndSet(void)
{
    static U16 dac_conversion_result;
    DAC_CHANNEL dac_channel = (DAC_CHANNEL)CoreTerm_GetArgumentAsU32(0);
    U16 dac_value = CoreTerm_GetArgumentAsU32(1);
    
    if(SysDac_Channel_Init(dac_channel))
    {
            if (SysDac_Channel_SetValue(dac_channel, dac_value))
            {
                LOG_TRM("dac channel %d = %d", PU8(dac_channel), PU16(dac_value));
            }
            else
            {
                CoreTerm_PrintFailed();
                return;
            }
    }
    else
    {
        CoreTerm_PrintFailed();
        return;
    }
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibDac_Init(void)
{
    MODULE_INIT_ONCE();

    CoreTerm_RegisterCommand("DacChannelInit","inits a dac channel ( a= channelNr)", 1, Command_DacChannelInit, TRUE);
    CoreTerm_RegisterCommand("DacChannelSet","Sets the raw value of an dac channel (a = channelNr, b = dac_value)", 2, Command_DacChannelSet, TRUE);
    CoreTerm_RegisterCommand("DacChannelGet","Gets a value of an dac channel (a = channelNr)", 1, Command_DacChannelGet, TRUE);
    CoreTerm_RegisterCommand("DacChannelInitAndSet","inits aad get selected dac channel ( a= channelNr) (b= dac value)", 2, Command_DacChannelInitAndSet, TRUE);

    MODULE_INIT_DONE();
}
//================================================================================================//

