//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking ADC driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define ADC__DRVADCSYSBLOCK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef ADC__DRVADCSYSBLOCK_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               ADC__DRVADCSYSBLOCK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking ADC channels
#ifndef DRVADCSYSBLOCK_COUNT
	#define DRVADCSYSBLOCK_COUNT		5
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "adc\DrvAdcSysBlock.h"
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
static BOOL DrvAdcSysBlock_StartConversion(ADC_CHANNEL_ID channel_id, U16* value_ptr);
static void DrvAdcSysBlock_MsgComplete(ADC_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static ADC_CHANNEL_HOOK_LIST                adc_channel_hook_list;
static ADC_CHANNEL_STRUCT                   adc_channel_struct[DRVADCSYSBLOCK_COUNT];
static U8                                   adc_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvAdcSysBlock_StartConversion(ADC_CHANNEL_ID channel_id, U16* value_ptr)
{
    return SysAdcBlock_Channel_StartConversion((ADC_CHANNEL)channel_id, value_ptr);
}
//------------------------------------------------------------------------------------------------//
static void DrvAdcSysBlock_MsgComplete(ADC_CHANNEL channel)
{
    ADC_CHANNEL_HNDL    channel_hndl;

    LOG_DEV("DrvAdcSysBlock_MsgComplete");
    for(channel_hndl = adc_channel_struct; channel_hndl <= &adc_channel_struct[adc_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == (U8)channel)
        {
            DrvAdc_ConversionComplete(channel_hndl);
            break;
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvAdcSysBlock_Init(void)
{
    LOG_DEV("DrvAdcSysBlock_Init");
    SysAdcBlock_Init();
    SysAdcBlock_RegisterAdcComplete(DrvAdcSysBlock_MsgComplete);

    adc_channel_hook_list.start_conversion_hook = DrvAdcSysBlock_StartConversion;

    MEMSET((VPTR)adc_channel_struct, 0, SIZEOF(adc_channel_struct));
    adc_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
ADC_CHANNEL_HNDL DrvAdcSysBlock_Register(ADC_CHANNEL channel, ADC_CONFIG_STRUCT* config_struct_ptr)
{
    ADC_CHANNEL_HNDL    channel_hndl;

    LOG_DEV("DrvAdcSysBlock_Register");

    for(channel_hndl = adc_channel_struct; channel_hndl < &adc_channel_struct[adc_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == (U8)channel)
        {
            SysAdcBlock_Channel_Init(channel, config_struct_ptr);
            return channel_hndl;
        }
    }

    if(adc_channel_count < DRVADCSYSBLOCK_COUNT)
    {
        if(SysAdcBlock_Channel_Init(channel, config_struct_ptr))
        {
            channel_hndl->hook_list_ptr = &adc_channel_hook_list;
            channel_hndl->channel_id = (U8)channel;
            adc_channel_count++;
            return channel_hndl;
        }
        LOG_WRN("Failed to init channel - %d", PU8(channel));
        return NULL;
    }
    LOG_ERR("ADC count overrun");
    return NULL;
}
//================================================================================================//
