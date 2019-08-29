//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of a DMA driven SCI Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SCI__DRVSCICHANNELSYSDMA_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SCI__DRVSCICHANNELSYSDMA_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SCI__DRVSCICHANNELSYSDMA_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of interrupt driven SCI channels
#ifndef DRVSCICHANNELSYSDMA_COUNT
	#define DRVSCICHANNELSYSDMA_COUNT			SCI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sci\DrvSciChannelSysDma.h"
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
static BOOL DrvSciChannelSysDma_Config(SCI_CHANNEL_ID channel_id, SCI_CONFIG_STRUCT* config_struct_ptr);
static BOOL DrvSciChannelSysDma_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);
static BOOL DrvSciChannelSysDma_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);
static BOOL DrvSciChannelSysDma_NotityTxDataReady(SCI_CHANNEL_ID channel_id);
static void DrvSciChannelSysDma_SysMsgComplete(SCI_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const SCI_CHANNEL_HOOK_LIST          sci_channel_hook_list = {DrvSciChannelSysDma_Config,
                                                                     DrvSciChannelSysDma_RegisterRxHook,
                                                                     DrvSciChannelSysDma_RegisterTxHook,
                                                                     DrvSciChannelSysDma_NotityTxDataReady,
                                                                     NULL,
                                                                     NULL};

static SCI_CHANNEL_STRUCT                   sci_channel_struct[DRVSCICHANNELSYSDMA_COUNT];
static U8                                   sci_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysDma_Config(SCI_CHANNEL_ID channel_id, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    return SysSciDma_Channel_Config((SCI_CHANNEL)channel_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysDma_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook)
{
    return SysSciDma_Channel_RegisterRxHook((SCI_CHANNEL)channel_id, rx_new_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysDma_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook)
{
    return SysSciDma_Channel_RegisterTxHook((SCI_CHANNEL)channel_id, tx_get_next_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSysDma_NotityTxDataReady(SCI_CHANNEL_ID channel_id)
{
    return SysSciDma_Channel_NotityTxDataReady((SCI_CHANNEL)channel_id);
}
//------------------------------------------------------------------------------------------------//
static void DrvSciChannelSysDma_SysMsgComplete(SCI_CHANNEL channel)
{
    SCI_CHANNEL_HNDL    channel_hndl;

    LOG_DEV("DrvSciChannelSysDma_SysMsgComplete");
    if(drvscichannel_msg_complete_hook != NULL)
    {
        for(channel_hndl = sci_channel_struct; channel_hndl < &sci_channel_struct[sci_channel_count]; channel_hndl++)
        {
            if(channel_hndl->channel_id == (U8)channel)
            {
                drvscichannel_msg_complete_hook(channel_hndl);  //call external hook in DrvSciChannel.c
                break;
            }
        }
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSciChannelSysDma_Init(void)
{
    MODULE_INIT_ONCE();

    LOG_DEV("DrvSciChannelSysDma_Init");
    SysSciDma_Init();
    SysSciDma_RegisterMsgComplete(DrvSciChannelSysDma_SysMsgComplete);

    MEMSET((VPTR)sci_channel_struct, 0, SIZEOF(sci_channel_struct));
    sci_channel_count = 0;

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
SCI_CHANNEL_HNDL DrvSciChannelSysDma_Register(SCI_CHANNEL sci_channel, DMA_CHANNEL dma_rx_channel, U16 rx_buffer_size, DMA_CHANNEL dma_tx_channel, U16 tx_buffer_size)
{
    MODULE_CHECK();

    SCI_CHANNEL_HNDL    channel_hndl;

    LOG_DEV("    DrvSciChannelSysDma_Register");
    for(channel_hndl = sci_channel_struct; channel_hndl < &sci_channel_struct[sci_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == sci_channel)
        {
            return channel_hndl;
        }
    }
    if(sci_channel_count < DRVSCICHANNELSYSDMA_COUNT)
    {
        if(SysSciDma_Channel_Init(sci_channel, dma_rx_channel, rx_buffer_size, dma_tx_channel, tx_buffer_size))
        {
            channel_hndl->hook_list_ptr = (SCI_CHANNEL_HOOK_LIST*)&sci_channel_hook_list;
            channel_hndl->channel_id = sci_channel;
            sci_channel_count++;
            return channel_hndl;
        }
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return NULL;
}
//================================================================================================//
