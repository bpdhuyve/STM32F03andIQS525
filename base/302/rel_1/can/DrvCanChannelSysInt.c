//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the interrupt driven CAN Channel driver.
//
// Processor       : independent 
// Implementation  : specific
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CAN__DRVCANCHANNELSYSINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CAN__DRVCANCHANNELSYSINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CAN__DRVCANCHANNELSYSINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of interrupt driven CAN channels
#ifndef DRVCANCHANNELSYSINT_COUNT
	#define DRVCANCHANNELSYSINT_COUNT			CAN_CHANNEL_COUNT //all available peripheral
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "can\DrvCanChannelSysInt.h"
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
static BOOL DrvCanChannelSysInt_Config(CAN_CHANNEL_ID channel_id, CAN_CONFIG_STRUCT* config_struct_ptr);
static BOOL DrvCanChannelSysInt_ConfigMailboxes(CAN_CHANNEL_ID channel_id, CONFIG_SCHEME cfg, U32 node_info);
static BOOL DrvCanChannelSysInt_RegisterRxHook(CAN_CHANNEL_ID channel_id, CAN_RX_NEW_MSSG_HOOK rx_new_byte_hook);
static BOOL DrvCanChannelSysInt_RegisterTxHook(CAN_CHANNEL_ID channel_id, CAN_TX_GET_NEXT_MSSG_HOOK tx_get_next_byte_hook);
static BOOL DrvCanChannelSysInt_RegisterErrorHook(CAN_CHANNEL_ID channel_id, CAN_ERROR_HOOK error_hook);
static BOOL DrvCanChannelSysInt_NotityTxMessageReady(CAN_CHANNEL_ID channel_id);
static BOOL DrvCanChannelSysInt_RecoverFromBusOff(CAN_CHANNEL_ID channel_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static CAN_CHANNEL_HOOK_LIST                can_channel_hook_list;
static CAN_CHANNEL_STRUCT                   can_channel_struct[DRVCANCHANNELSYSINT_COUNT];
static U8                                   can_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvCanChannelSysInt_Config(CAN_CHANNEL_ID channel_id, CAN_CONFIG_STRUCT* config_struct_ptr)
{
    return SysCanInt_Channel_Config((CAN_CHANNEL)channel_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvCanChannelSysInt_ConfigMailboxes(CAN_CHANNEL_ID channel_id, CONFIG_SCHEME cfg, U32 node_info)
{
    return SysCanInt_Channel_ConfigMailboxes((CAN_CHANNEL)channel_id, cfg, node_info);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvCanChannelSysInt_RegisterRxHook(CAN_CHANNEL_ID channel_id, CAN_RX_NEW_MSSG_HOOK rx_new_byte_hook)
{
    return SysCanInt_Channel_RegisterRxHook((CAN_CHANNEL)channel_id, rx_new_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvCanChannelSysInt_RegisterTxHook(CAN_CHANNEL_ID channel_id, CAN_TX_GET_NEXT_MSSG_HOOK tx_get_next_byte_hook)
{
    return SysCanInt_Channel_RegisterTxHook((CAN_CHANNEL)channel_id, tx_get_next_byte_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvCanChannelSysInt_RegisterErrorHook(CAN_CHANNEL_ID channel_id, CAN_ERROR_HOOK error_hook)
{
    return SysCanInt_Channel_RegisterErrorHook((CAN_CHANNEL)channel_id, error_hook);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvCanChannelSysInt_NotityTxMessageReady(CAN_CHANNEL_ID channel_id)
{
    return SysCanInt_Channel_NotifyTxMessageReady((CAN_CHANNEL)channel_id);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvCanChannelSysInt_RecoverFromBusOff(CAN_CHANNEL_ID channel_id)
{
    return SysCanInt_Channel_RecoverFromBusOff((CAN_CHANNEL)channel_id);
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvCanChannelSysInt_Init(void)
{
    LOG_DEV("DrvCanChannelSysInt_Init");
    SysCanInt_Init();

    can_channel_hook_list.config_hook = DrvCanChannelSysInt_Config;
    can_channel_hook_list.config_mailboxes_hook = DrvCanChannelSysInt_ConfigMailboxes;
    can_channel_hook_list.register_rx_hook = DrvCanChannelSysInt_RegisterRxHook;
    can_channel_hook_list.register_tx_hook = DrvCanChannelSysInt_RegisterTxHook;
    can_channel_hook_list.register_error_hook = DrvCanChannelSysInt_RegisterErrorHook;
    can_channel_hook_list.notify_tx_message_ready_hook = DrvCanChannelSysInt_NotityTxMessageReady;
    can_channel_hook_list.recover_from_bus_off_hook = DrvCanChannelSysInt_RecoverFromBusOff;
    
    MEMSET((VPTR)&can_channel_struct[0], 0, SIZEOF(can_channel_struct));
    can_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
CAN_CHANNEL_HNDL DrvCanChannelSysInt_Register(CAN_CHANNEL channel)
{
    CAN_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("    DrvCanChannelSysInt_Register");
    for(channel_hndl = &can_channel_struct[0]; channel_hndl < &can_channel_struct[can_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == channel)
        {
            return channel_hndl;
        }
    }
    if(can_channel_count < DRVCANCHANNELSYSINT_COUNT)
    {
        if(SysCanInt_Channel_Init(channel))
        {
            channel_hndl->hook_list_ptr = &can_channel_hook_list;
            channel_hndl->channel_id = (U8) channel;
            can_channel_count++;
            return channel_hndl;
        }
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return NULL;
}
//================================================================================================//
