//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the common part of the CAN Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CAN__DRVCANCHANNEL_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CAN__DRVCANCHANNEL_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CAN__DRVCANCHANNEL_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "can\DrvCanChannel.h"
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
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
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
void DrvCanChannel_Init(void)
{
    LOG_DEV("DrvCanChannel_Init");
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_Config(CAN_CHANNEL_HNDL channel_hndl, CAN_CONFIG_STRUCT* config_struct_ptr)
{
    LOG_DEV("DrvCanChannel_Config");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->config_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->config_hook(channel_hndl->channel_id, config_struct_ptr);
    }
    LOG_WRN("CAN config function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_ConfigMailboxes(CAN_CHANNEL_HNDL channel_hndl, CONFIG_SCHEME cfg, U32 node_info)
{
    LOG_DEV("DrvCanChannel_ConfigMailboxes");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->config_mailboxes_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->config_mailboxes_hook(channel_hndl->channel_id, cfg, node_info);
    }
    LOG_WRN("CAN config mailbox is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_ConfigBitTimeAnalyser(CAN_CHANNEL_HNDL channel_hndl, CAN_TIME_ANALYSE_MODE mode, BOOL capture_all)
{
    LOG_DEV("DrvCanChannel_ConfigBitTimeAnalyser");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->config_bit_time_analyser_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->config_bit_time_analyser_hook(channel_hndl->channel_id, mode, capture_all);
    }
    LOG_WRN("CAN config bit time analyse is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_RegisterRxHook(CAN_CHANNEL_HNDL channel_hndl, CAN_RX_NEW_MSSG_HOOK rx_hook)
{
    LOG_DEV("DrvCanChannel_RegisterRxHook");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->register_rx_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->register_rx_hook(channel_hndl->channel_id, rx_hook);
    }
    LOG_WRN("CAN register RX hook is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_RegisterTxHook(CAN_CHANNEL_HNDL channel_hndl, CAN_TX_GET_NEXT_MSSG_HOOK tx_hook)
{
    LOG_DEV("DrvCanChannel_RegisterTxHook");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->register_tx_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->register_tx_hook(channel_hndl->channel_id, tx_hook);
    }
    LOG_WRN("CAN register TX hook is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_RegisterErrorHook(CAN_CHANNEL_HNDL channel_hndl, CAN_ERROR_HOOK error_hook)
{
    LOG_DEV("DrvCanChannel_RegisterErrorHook");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->register_error_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->register_error_hook(channel_hndl->channel_id, error_hook);
    }
    LOG_WRN("CAN register ERROR hook is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_RegisterBitTimeAnalyseHook(CAN_CHANNEL_HNDL channel_hndl, CAN_BIT_TIME_ANALYSE_HOOK bit_time_analyse_hook)
{
    LOG_DEV("DrvCanChannel_RegisterBitTimeAnalyseHook");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->register_time_analyse_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->register_time_analyse_hook(channel_hndl->channel_id, bit_time_analyse_hook);
    }
    LOG_WRN("CAN register bit time analyse hook is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_NotityTxMessageReady(CAN_CHANNEL_HNDL channel_hndl)
{
    LOG_DEV("DrvCanChannel_NotityTxMessageReady");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->notify_tx_message_ready_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->notify_tx_message_ready_hook(channel_hndl->channel_id);
    }
    LOG_WRN("CAN notify TX message ready hook is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvCanChannel_RecoverFromBusOff(CAN_CHANNEL_HNDL channel_hndl)
{
    LOG_DEV("DrvCanChannel_RecoverFromBusOff");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->recover_from_bus_off_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->recover_from_bus_off_hook(channel_hndl->channel_id);
    }
    LOG_WRN("CAN recover from bus off hook is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void DrvCanChannel_FillExtDataFrameMssgStruct(CAN_MSSG_STRUCT* mssg_ptr, U32 identifier, U8 dlc, U8* data_ptr)
{
    mssg_ptr->identifier_type = EXTENDED;
    mssg_ptr->frame_type = DATA_FRAME;
    mssg_ptr->identifier = identifier;
    mssg_ptr->dlc = dlc;
    if((dlc > 0) && (dlc <= 8))
    {
        MEMCPY((VPTR)&mssg_ptr->data[0], (VPTR)data_ptr, dlc);
    }
}
//================================================================================================//
