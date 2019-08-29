//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the common part of the SCI Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SCI__DRVSCICHANNEL_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SCI__DRVSCICHANNEL_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SCI__DRVSCICHANNEL_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of RS485 channels to be configured
#ifndef SCI_RS485_COUNT
	#define SCI_RS485_COUNT			    0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    SCI_CHANNEL_HNDL    channel_hndl;
    DRVGPIO_PIN_HNDL    pin_de;
    RS485_RX_LEVEL      rx_level;
    BOOL             active;
}
SCI_RS485_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if SCI_RS485_COUNT
static void DrvSciChannel_MsgComplete(SCI_CHANNEL_HNDL channel_hndl);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

#if SCI_RS485_COUNT
static SCI_RS485_CTRL_STRUCT                sci_rs484_ctrl_struct[SCI_RS485_COUNT];
static U8                           sci_rs485_ctrl_count;
static DRVSCI_MSG_COMPLETE                  sci_msg_complete_hook;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
DRVSCI_MSG_COMPLETE                         drvscichannel_msg_complete_hook = NULL;     //function hook that will be called by external file
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if SCI_RS485_COUNT
static void DrvSciChannel_MsgComplete(SCI_CHANNEL_HNDL channel_hndl)
{
    SCI_RS485_CTRL_STRUCT*  rs485_ctrl_ptr;
    
    LOG_DEV("DrvSciChannel_MsgComplete");
    
    for(rs485_ctrl_ptr = sci_rs484_ctrl_struct; rs485_ctrl_ptr < &sci_rs484_ctrl_struct[sci_rs485_ctrl_count]; rs485_ctrl_ptr++)
    {
        if(rs485_ctrl_ptr->channel_hndl == channel_hndl)
        {
            LOG_DBG("RS485 to RX");
            DrvGpio_SetPin(rs485_ctrl_ptr->pin_de, (BOOL)(rs485_ctrl_ptr->rx_level == RS485_RX_LEVEL_HIGH));
            rs485_ctrl_ptr->active = FALSE;
            break;
        }
    }
    
    if(sci_msg_complete_hook != NULL)
    {
        sci_msg_complete_hook(channel_hndl);
    }
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSciChannel_Init(void)
{
    MODULE_INIT_ONCE();
    
    LOG_DEV("DrvSciChannel_Init");
    
    #if SCI_RS485_COUNT
        MEMSET((VPTR)sci_rs484_ctrl_struct, 0, SIZEOF(sci_rs484_ctrl_struct));
        sci_rs485_ctrl_count = 0;
        sci_msg_complete_hook = NULL;

        drvscichannel_msg_complete_hook = DrvSciChannel_MsgComplete;
    #else
        drvscichannel_msg_complete_hook = NULL;
    #endif
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_RegisterEvent_TxComplete(DRVSCI_MSG_COMPLETE msg_complete_hook)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_RegisterEvent_TxComplete");
    #if SCI_RS485_COUNT
        sci_msg_complete_hook = msg_complete_hook;
    #else
        drvscichannel_msg_complete_hook = msg_complete_hook;
    #endif
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_ConfigAsRs485(SCI_CHANNEL_HNDL channel_hndl, DRVGPIO_PIN_HNDL pin_de, RS485_RX_LEVEL rx_level)
{
    MODULE_CHECK();
    
    #if SCI_RS485_COUNT
        SCI_RS485_CTRL_STRUCT*  rs485_ctrl_ptr = &sci_rs484_ctrl_struct[sci_rs485_ctrl_count];
    #endif
    
    LOG_DEV("DrvSciChannel_ConfigAsRs485");
    #if SCI_RS485_COUNT
        if(sci_rs485_ctrl_count < SCI_RS485_COUNT)
        {
            rs485_ctrl_ptr->channel_hndl = channel_hndl;
            rs485_ctrl_ptr->pin_de = pin_de;
            rs485_ctrl_ptr->rx_level = rx_level;
            rs485_ctrl_ptr->active = FALSE;
            sci_rs485_ctrl_count++;
            DrvGpio_SetPin(pin_de, (BOOL)(rx_level == RS485_RX_LEVEL_HIGH));
            return TRUE;
        }
    #endif
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_Config(SCI_CHANNEL_HNDL channel_hndl, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_Config");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->config_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->config_hook(channel_hndl->channel_id, config_struct_ptr);
    }
    LOG_WRN("SCI config function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_RegisterEvent_RxDataReceived(SCI_CHANNEL_HNDL channel_hndl, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_RegisterEvent_RxDataReceived");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->register_rx_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->register_rx_hook(channel_hndl->channel_id, rx_new_byte_hook);
    }
    LOG_WRN("SCI register RX hook function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_RegisterEvent_TxDataNeeded(SCI_CHANNEL_HNDL channel_hndl, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_RegisterEvent_TxDataNeeded");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->register_tx_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->register_tx_hook(channel_hndl->channel_id, tx_get_next_byte_hook);
    }
    LOG_WRN("SCI register TX hook function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_NotifyTxDataReady(SCI_CHANNEL_HNDL channel_hndl)
{
    MODULE_CHECK();

    #if SCI_RS485_COUNT
        SCI_RS485_CTRL_STRUCT*  rs485_ctrl_ptr;
    #endif
    
    LOG_DEV("DrvSciChannel_NotifyTxDataReady");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->notify_tx_data_ready != NULL))
    {
        #if SCI_RS485_COUNT
            for(rs485_ctrl_ptr = sci_rs484_ctrl_struct; rs485_ctrl_ptr < &sci_rs484_ctrl_struct[sci_rs485_ctrl_count]; rs485_ctrl_ptr++)
            {
                if(rs485_ctrl_ptr->channel_hndl == channel_hndl)
                {
                    LOG_DBG("RS485 to TX");
                    DrvGpio_SetPin(rs485_ctrl_ptr->pin_de, (BOOL)(rs485_ctrl_ptr->rx_level == RS485_RX_LEVEL_LOW));
                    rs485_ctrl_ptr->active = TRUE;
                    break;
                }
            }
        #endif
        return channel_hndl->hook_list_ptr->notify_tx_data_ready(channel_hndl->channel_id);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// MPCM
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_ConfigAsMpcm(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED speed, BOOL allow_rx)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_ConfigAsMpcm");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->config_as_mpcm_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->config_as_mpcm_hook(channel_hndl->channel_id, speed, allow_rx);
    }
    LOG_WRN("SCI config as MPCM function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_SetMpcmFilter(SCI_CHANNEL_HNDL channel_hndl, BOOL enable)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_SetMpcmFilter");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->set_mpcm_filter_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->set_mpcm_filter_hook(channel_hndl->channel_id, enable);
    }
    LOG_WRN("SCI set MPCM filter function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// LIN
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_ConfigAsLin(SCI_CHANNEL_HNDL channel_hndl, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_ConfigAsLin");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->config_as_lin_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->config_as_lin_hook(channel_hndl->channel_id, config_struct_ptr);
    }
    LOG_WRN("SCI config as LIN function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_RegisterEvent_LinBreakDetected(SCI_CHANNEL_HNDL channel_hndl, EVENT_CALLBACK lin_break_detected_hook)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_RegisterEvent_LinBreakDetected");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->lin_break_detected_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->lin_break_detected_hook(channel_hndl->channel_id, lin_break_detected_hook);
    }
    LOG_WRN("SCI register LIN break detected hook function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannel_SendLinBreak(SCI_CHANNEL_HNDL channel_hndl)
{
    MODULE_CHECK();
    
    LOG_DEV("DrvSciChannel_ConfigAsLin");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->send_lin_break_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->send_lin_break_hook(channel_hndl->channel_id);
    }
    LOG_WRN("SCI send LIN break function is NULL");
    return FALSE;
}
//================================================================================================//
