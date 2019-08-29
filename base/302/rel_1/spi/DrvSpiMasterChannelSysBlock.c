//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking SPI Master Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SPI__DRVSPIMASTERCHANNELSYSBLOCK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SPI__DRVSPIMASTERCHANNELSYSBLOCK_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SPI__DRVSPIMASTERCHANNELSYSBLOCK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef DRVSPIMASTERCHANNELSYSBLOCK_COUNT
	#define DRVSPIMASTERCHANNELSYSBLOCK_COUNT			SPI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "spi\DrvSpiMasterChannelSysBlock.h"
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
static BOOL DrvSpiMasterChannelSysBlock_ExchangeData(SPI_CHANNEL_ID channel_id, U8* write_data_ptr, U8* read_data_ptr, U16 count);
static BOOL DrvSpiMasterChannelSysBlock_Config(SPI_CHANNEL_ID channel_id, SPI_CONFIG_STRUCT* config_struct_ptr);
static void DrvSpiMasterChannelSysBlock_MsgComplete(SPI_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SPI_CHANNEL_HOOK_LIST                spi_channel_hook_list;
static SPI_CHANNEL_STRUCT                   spi_channel_struct[DRVSPIMASTERCHANNELSYSBLOCK_COUNT];
static U8                                   spi_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSpiMasterChannelSysBlock_ExchangeData(SPI_CHANNEL_ID channel_id, U8* write_data_ptr, U8* read_data_ptr, U16 count)
{
    return SysSpiMasterBlock_Channel_ExchangeData((SPI_CHANNEL)channel_id, write_data_ptr, read_data_ptr, count);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSpiMasterChannelSysBlock_Config(SPI_CHANNEL_ID channel_id, SPI_CONFIG_STRUCT* config_struct_ptr)
{
    return SysSpiMasterBlock_Channel_Config((SPI_CHANNEL)channel_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static void DrvSpiMasterChannelSysBlock_MsgComplete(SPI_CHANNEL channel)
{
    SPI_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("DrvSpiMasterChannelSysBlock_MsgComplete");
    if(drvspimasterchannel_msg_complete_hook != NULL)
    {
        for(channel_hndl = spi_channel_struct; channel_hndl <= &spi_channel_struct[spi_channel_count]; channel_hndl++)
        {
            if(channel_hndl->channel_id == (U8)channel)
            {
                drvspimasterchannel_msg_complete_hook(channel_hndl);
                break;
            }
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSpiMasterChannelSysBlock_Init(void)
{
    LOG_DEV("DrvSpiMasterChannelSysBlock_Init");
    SysSpiMasterBlock_Init();
    SysSpiMasterBlock_RegisterMsgComplete(DrvSpiMasterChannelSysBlock_MsgComplete);
    
    spi_channel_hook_list.exchange_hook = DrvSpiMasterChannelSysBlock_ExchangeData;
    spi_channel_hook_list.config_hook = DrvSpiMasterChannelSysBlock_Config;
    
    MEMSET((VPTR)spi_channel_struct, 0, SIZEOF(spi_channel_struct));
    spi_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
SPI_CHANNEL_HNDL DrvSpiMasterChannelSysBlock_Register(SPI_CHANNEL channel)
{
    SPI_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("DrvSpiMasterChannelSysBlock_Register");
    for(channel_hndl = spi_channel_struct; channel_hndl < &spi_channel_struct[spi_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == channel)
        {
            return channel_hndl;
        }
    }
    if(spi_channel_count < DRVSPIMASTERCHANNELSYSBLOCK_COUNT)
    {
        if(SysSpiMasterBlock_Channel_Init(channel))
        {
            channel_hndl->hook_list_ptr = &spi_channel_hook_list;
            channel_hndl->channel_id = (U8)channel;
            spi_channel_count++;
            return channel_hndl;
        }
    }
    LOG_ERR("Illegal SPI channel - %d", PU8(channel));
    return NULL;
}
//================================================================================================//
