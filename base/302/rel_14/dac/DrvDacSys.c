//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the common part of the SPI Master Channel driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DAC__DRVDACSOFTTRIG_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DAC__DRVDACSOFTTRIG_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               DAC__DRVDACSOFTTRIG_LOG_LEVEL
#endif
// @brief  Defines the maximum number of blocking ADC channels
//------------------------------------------------------------------------------------------------//
#ifndef DRVDACSOFTTRIG_COUNT
	#define DRVDACSOFTTRIG_COUNT		2
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "dac\DrvDacSys.h"
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
static BOOL DrvDacSys_SetValue(DAC_CHANNEL_ID channel_id, U16 dac_value);
static BOOL DrvDacSys_GetValue(DAC_CHANNEL_ID channel_id, U16* dac_value);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static DAC_CHANNEL_HOOK_LIST                dac_channel_hook_list;
static DAC_CHANNEL_STRUCT                   dac_channel_struct[DRVDACSOFTTRIG_COUNT];
static U8                                   dac_channel_count;
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvDacSys_SetValue(DAC_CHANNEL_ID channel_id, U16 dac_value)
{
    return SysDac_Channel_SetValue((DAC_CHANNEL)channel_id, dac_value);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvDacSys_GetValue(DAC_CHANNEL_ID channel_id, U16* dac_value)
{
    return SysDac_Channel_GetValue((DAC_CHANNEL)channel_id, dac_value);
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvDacSys_Init(void)
{
    LOG_DEV("DrvDac_Init");
    SysDac_Init();
    
    dac_channel_hook_list.set_value_hook = DrvDacSys_SetValue;
    dac_channel_hook_list.get_value_hook = DrvDacSys_GetValue;

    MEMSET((VPTR)dac_channel_struct, 0, SIZEOF(dac_channel_struct));
    dac_channel_count = 0;
}
//-------------------------- ----------------------------------------------------------------------//
DAC_CHANNEL_HNDL DrvDacSys_Register(DAC_CHANNEL channel)
{
    DAC_CHANNEL_HNDL    channel_hndl;

    LOG_DEV("DrvDac_Register");

    for(channel_hndl = dac_channel_struct; channel_hndl < &dac_channel_struct[dac_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == (U8)channel)
        {
            SysDac_Channel_Init(channel);
            return channel_hndl;
        }
    }

    if(dac_channel_count < DRVDACSOFTTRIG_COUNT)
    {
        if(SysDac_Channel_Init(channel))
        {
            channel_hndl->hook_list_ptr = &dac_channel_hook_list;
            channel_hndl->channel_id = (U8)channel;
            dac_channel_count++;
            return channel_hndl;
        }
        LOG_WRN("Failed to init channel - %d", PU8(channel));
        return NULL;
    }
    LOG_ERR("DAC count overrun");
    return NULL;
}
//================================================================================================//
