//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking ADC system.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define ADC__SYSADCBLOCK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef ADC__SYSADCBLOCK_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               ADC__SYSADCBLOCK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef SYSADCBLOCK_COUNT
	#define SYSADCBLOCK_COUNT		    ADC_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS include section
#include "adc\SysAdcBlock.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef ADC_TypeDef*            ADC_REG_HNDL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static ADC_SAMPLE_TIME              sysadcblock_adc_sampletimes[SYSADCBLOCK_COUNT];
static ADC_CONVERSION_COMPLETE      sysadcblock_conv_complete_hook;
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
// @remark none
void SysAdcBlock_Init(void)
{
    sysadcblock_conv_complete_hook = NULL;
    MEMSET((VPTR)sysadcblock_adc_sampletimes, 0, SIZEOF(sysadcblock_adc_sampletimes));
}
//------------------------------------------------------------------------------------------------//
// @remark none
BOOL SysAdcBlock_Module_Init(ADC_MODULE module)
{
    if(module < ADC_MODULE_COUNT)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        
        if(ADC1->CR & ADC_CR_ADEN)
        {
            ADC1->CR = ADC_CR_ADDIS;             // give ADC disable command
            while(ADC1->CR & ADC_CR_ADDIS){};    // wait till ADDIS flag is gone
        }
        
        ADC1->CR |= ADC_CR_ADCAL;            // start calibration
        while(ADC1->CR & ADC_CR_ADCAL){};    // wait till calibration complete
        
        ADC1->CFGR1 = ADC_CFGR1_DISCEN;      // select discontinuous mode, single conversion, SW triggered, 12bits, right aligned
        ADC1->CFGR2 = ADC_CFGR2_JITOFFDIV2;  // select clock PCLK/2 (should be dynamic based on core/system setting
        
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// @remark none
BOOL SysAdcBlock_RegisterAdcComplete(ADC_CONVERSION_COMPLETE adc_complete_hook)
{
    sysadcblock_conv_complete_hook = adc_complete_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark none
BOOL SysAdcBlock_Channel_Init(ADC_CHANNEL channel, ADC_CONFIG_STRUCT* config_struct_ptr)
{
    if((channel < SYSADCBLOCK_COUNT) && (config_struct_ptr->adc_module < ADC_MODULE_COUNT))
    {
        sysadcblock_adc_sampletimes[channel] = config_struct_ptr->sample_time;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// @remark none
BOOL SysAdcBlock_Channel_StartConversion(ADC_CHANNEL channel, U16* value_ptr)
{
    if(channel < SYSADCBLOCK_COUNT)
    {
        ADC1->CHSELR = (1 << channel);                      // set channel to convert
        ADC1->SMPR = sysadcblock_adc_sampletimes[channel];  // set sample time
        
        ADC1->CR |= ADC_CR_ADEN;                            // enable ADC
        while((ADC1->ISR & ADC_ISR_ADRDY) == 0){};          // wait till ready for conversion
        
        ADC1->CR |= ADC_CR_ADSTART;                         // start conversion
        while(ADC1->CR & ADC_CR_ADSTART){};                 // wait until conversion complete
        
        *value_ptr = ADC1->DR;                              // read data
        
        ADC1->CR = ADC_CR_ADDIS;                            // disable ADC
        
        if(sysadcblock_conv_complete_hook != NULL)
        {
            sysadcblock_conv_complete_hook(channel);
        }
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//
