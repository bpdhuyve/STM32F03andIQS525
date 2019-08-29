//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the interrupt driven SCI Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SCI__DRVSCICHANNELSYSINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SCI__DRVSCICHANNELSYSINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SCI__DRVSCICHANNELSYSINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of interrupt driven SCI channels
#ifndef DRVSCICHANNELSYSINT_COUNT
	#define DRVSCICHANNELSYSINT_COUNT			SCI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sci\DrvSciChannelSysInt.h"
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
static BOOL DrvSciChannelSysInt_Config(SCI_CHANNEL_ID channel_id, SCI_CONFIG_STRUCT* config_struct_ptr);
static BOOL DrvSciChannelSysInt_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);
static BOOL DrvSciChannelSysInt_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);
static BOOL DrvSciChannelSysInt_NotityTxDataReady(SCI_CHANNEL_ID channel_id);
#ifdef INCLUDE_MPCM
static BOOL DrvSciChannelSysInt_ConfigAsMpcm(SCI_CHANNEL_ID channel_id, SCI_SPEED speed, BOOL allow_rx);
static BOOL DrvSciChannelSysInt_SetMpcmFilter(SCI_CHANNEL_ID channel_id, BOOL enable);
#endif
static void DrvSciChannelSysInt_SysMsgComplete(SCI_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SCI_CHANNEL_HOOK_LIST                sci_channel_hook_list;
static SCI_CHANNEL_STRUCT                   sci_channel_struct[DRVSCICHANNELSYSINT_COUNT];
static U8                                   sci_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysInt_Config(SCI_CHANNEL_ID channel_id, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    return SysSciInt_Channel_Config((SCI_CHANNEL)channel_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysInt_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook)
{
    return SysSciInt_Channel_RegisterRxHook((SCI_CHANNEL)channel_id, rx_new_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysInt_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook)
{
    return SysSciInt_Channel_RegisterTxHook((SCI_CHANNEL)channel_id, tx_get_next_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysInt_NotityTxDataReady(SCI_CHANNEL_ID channel_id)
{
    return SysSciInt_Channel_NotityTxDataReady((SCI_CHANNEL)channel_id);
}
//------------------------------------------------------------------------------------------------//
#ifdef INCLUDE_MPCM
static BOOL DrvSciChannelSysInt_ConfigAsMpcm(SCI_CHANNEL_ID channel_id, SCI_SPEED speed, BOOL allow_rx)
{
    return SysSciInt_Channel_ConfigAsMpcm((SCI_CHANNEL)channel_id, speed, allow_rx);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysInt_SetMpcmFilter(SCI_CHANNEL_ID channel_id, BOOL enable)
{
    return SysSciInt_Channel_SetMpcmFilter((SCI_CHANNEL)channel_id, enable);
}
#endif
//------------------------------------------------------------------------------------------------//
static void DrvSciChannelSysInt_SysMsgComplete(SCI_CHANNEL channel)
{
    SCI_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("DrvSciChannelSysInt_SysMsgComplete");
    if(drvscichannel_msg_complete_hook != NULL)
    {
        for(channel_hndl = sci_channel_struct; channel_hndl < &sci_channel_struct[sci_channel_count]; channel_hndl++)
        {
            if(channel_hndl->channel_id == (U8)channel)
            {
                drvscichannel_msg_complete_hook(channel_hndl);
                break;
            }
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSciChannelSysInt_Init(void)
{
    LOG_DEV("DrvSciChannelSysInt_Init");
    SysSciInt_Init();
    SysSciInt_RegisterMsgComplete(DrvSciChannelSysInt_SysMsgComplete);
    
    sci_channel_hook_list.config_hook = DrvSciChannelSysInt_Config;
    sci_channel_hook_list.register_rx_hook = DrvSciChannelSysInt_RegisterRxHook;
    sci_channel_hook_list.register_tx_hook = DrvSciChannelSysInt_RegisterTxHook;
    sci_channel_hook_list.notify_tx_data_ready = DrvSciChannelSysInt_NotityTxDataReady;
#ifdef INCLUDE_MPCM
    sci_channel_hook_list.config_as_mpcm_hook = DrvSciChannelSysInt_ConfigAsMpcm;
    sci_channel_hook_list.set_mpcm_filter_hook = DrvSciChannelSysInt_SetMpcmFilter;
#else
    sci_channel_hook_list.config_as_mpcm_hook = NULL;
    sci_channel_hook_list.set_mpcm_filter_hook = NULL;
#endif
    
    MEMSET((VPTR)sci_channel_struct, 0, SIZEOF(sci_channel_struct));
    sci_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
SCI_CHANNEL_HNDL DrvSciChannelSysInt_Register(SCI_CHANNEL channel)
{
    SCI_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("    DrvSciChannelSysInt_Register");
    for(channel_hndl = sci_channel_struct; channel_hndl < &sci_channel_struct[sci_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == channel)
        {
            return channel_hndl;
        }
    }
    if(sci_channel_count < DRVSCICHANNELSYSINT_COUNT)
    {
        if(SysSciInt_Channel_Init(channel))
        {
            channel_hndl->hook_list_ptr = &sci_channel_hook_list;
            channel_hndl->channel_id = channel;
            sci_channel_count++;
            return channel_hndl;
        }
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return NULL;
}
//================================================================================================//
