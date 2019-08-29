//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the interrupt driven SCI Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SCI__DRVSCICHANNELSYSVCP_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SCI__DRVSCICHANNELSYSVCP_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SCI__DRVSCICHANNELSYSVCP_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of interrupt driven SCI channels
#ifndef DRVSCICHANNELSYSVCP_COUNT
	#define DRVSCICHANNELSYSVCP_COUNT			SCI_CHANNEL_COUNT
#endif
//================================================================================================//


//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "DrvSciChannelSysVcp.h"
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
static BOOL DrvSciChannelSysVcp_Config(SCI_CHANNEL_ID channel_id, SCI_CONFIG_STRUCT* config_struct_ptr);
static BOOL DrvSciChannelSysVcp_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);
static BOOL DrvSciChannelSysVcp_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);
static BOOL DrvSciChannelSysVcp_NotityTxDataReady(SCI_CHANNEL_ID channel_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SCI_CHANNEL_HOOK_LIST                sci_channel_hook_list;
static SCI_CHANNEL_STRUCT                   sci_channel_struct[DRVSCICHANNELSYSVCP_COUNT];
static U8                                   sci_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysVcp_Config(SCI_CHANNEL_ID channel_id, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    return SysSciVcp_Channel_Config((SCI_CHANNEL)channel_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysVcp_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook)
{
    return SysSciVcp_Channel_RegisterRxHook((SCI_CHANNEL)channel_id, rx_new_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysVcp_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook)
{
    return SysSciVcp_Channel_RegisterTxHook((SCI_CHANNEL)channel_id, tx_get_next_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysVcp_NotityTxDataReady(SCI_CHANNEL_ID channel_id)
{
    return SysSciVcp_Channel_NotityTxDataReady((SCI_CHANNEL)channel_id);
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSciChannelSysVcp_Init(void)
{
    LOG_DEV("DrvSciChannelSysVcp_Init");
    SysSciVcp_Init();
    
    sci_channel_hook_list.config_hook = DrvSciChannelSysVcp_Config;
    sci_channel_hook_list.register_rx_hook = DrvSciChannelSysVcp_RegisterRxHook;
    sci_channel_hook_list.register_tx_hook = DrvSciChannelSysVcp_RegisterTxHook;
    sci_channel_hook_list.notify_tx_data_ready = DrvSciChannelSysVcp_NotityTxDataReady;

    sci_channel_hook_list.config_as_mpcm_hook = NULL;
    sci_channel_hook_list.set_mpcm_filter_hook = NULL;
   
    MEMSET((VPTR)sci_channel_struct, 0, SIZEOF(sci_channel_struct));
    sci_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
SCI_CHANNEL_HNDL DrvSciChannelSysVcp_Register(SCI_CHANNEL channel)
{
  
    SCI_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("    DrvSciChannelSysVcp_Register");
    for(channel_hndl = sci_channel_struct; channel_hndl < &sci_channel_struct[sci_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == channel)
        {
            return channel_hndl;
        }
    }
    if(sci_channel_count < DRVSCICHANNELSYSVCP_COUNT)
    {
        if(SysSciVcp_Channel_Init(channel))
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
