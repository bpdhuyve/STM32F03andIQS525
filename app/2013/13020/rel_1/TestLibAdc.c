//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This module determines the board type
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBADC_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBADC_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBADC_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DUCO LIB
#include "TestLibAdc.h"

// DRV
#include "adc\SysAdcBlock.h"
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
static void Command_AdcChannelInit(void);
static void Command_AdcChannelGet(void);
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
static void Command_AdcChannelInit(void)
{
	#warning enumeratie niet in sync met namen stm (ADC_MODULE_1 = 0) voorlopig -1 om te confiusion in de terminal te fixen
    ADC_MODULE adc_module = (ADC_MODULE)(CoreTerm_GetArgumentAsU32(1) - 1);
    ADC_CHANNEL adc_channel = (ADC_CHANNEL)CoreTerm_GetArgumentAsU32(0);

    SysAdcBlock_Module_Init(adc_module);

	#warning deze testcmds gaan problemen geven bij andere processors want de config struct zal anders zijn
    ADC_CONFIG_STRUCT adc_config_struct;
    adc_config_struct.adc_module = adc_module;
    adc_config_struct.sample_time = ADC_1_5_CYCLES;

    if (SysAdcBlock_Channel_Init(adc_channel,&adc_config_struct))
    {
        CoreTerm_PrintAcknowledge();
    }
    else
    {
        CoreTerm_PrintFailed();
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_AdcChannelGet(void)
{
    static U16 adc_conversion_result;
    ADC_CHANNEL adc_channel = (ADC_CHANNEL)CoreTerm_GetArgumentAsU32(0);
    U32 nr_of_samples = CoreTerm_GetArgumentAsU32(1);

    if (nr_of_samples == 0)
    {
        nr_of_samples = 1; //if user forgets to type the number of samples, he probably means he wants one sample
    }

    for (;nr_of_samples>0;nr_of_samples--)
    {
        if (SysAdcBlock_Channel_StartConversion(adc_channel, &adc_conversion_result))
        {
            LOG_TRM("adc channel %d = %d", PU8(adc_channel), PU16(adc_conversion_result));
            //CoreLog_Flush(); //if user want bv 1000 samples we heve to flush the buffer otherwise we will get log lost warnings
        }
        else
        {
            CoreTerm_PrintFailed();
            return;
        }
    }
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibAdc_Init(void)
{
    MODULE_INIT_ONCE();

    CoreTerm_RegisterCommand("AdcChannelInit","inits a adc channel (a= channel nr, b= moduleNr you want the channel to connect to)", 2, Command_AdcChannelInit, TRUE);
    CoreTerm_RegisterCommand("AdcChannelGet","gets raw samples of an adc channel (a=channelNr  b=number of samples)", 2, Command_AdcChannelGet, TRUE);

    MODULE_INIT_DONE();
}
//================================================================================================//

