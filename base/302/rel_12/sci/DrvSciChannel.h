//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the SCI Channel driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SCI__DRVSCICHANNEL_H
#define SCI__DRVSCICHANNEL_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "sci\ISysSci.h"

//DRV include section
#include "gpio\DrvGpio.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                  SCI_CHANNEL_ID;

typedef BOOL (*SCI_CONFIG)(SCI_CHANNEL_ID channel_id, SCI_CONFIG_STRUCT* config_struct_ptr);

typedef BOOL (*SCI_REGISTER_RX_HOOK)(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);

typedef BOOL (*SCI_REGISTER_TX_HOOK)(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);

typedef BOOL (*SCI_NOTITY_TX_DATA_READY)(SCI_CHANNEL_ID channel_id);

typedef BOOL (*SCI_CONFIG_AS_MPCM)(SCI_CHANNEL_ID channel_id, SCI_SPEED speed, BOOL allow_rx);

typedef BOOL (*SCI_SET_MPCM_FILTER_HOOK)(SCI_CHANNEL_ID channel_id, BOOL enable);

typedef struct
{
    SCI_CONFIG                      config_hook;
	SCI_REGISTER_RX_HOOK            register_rx_hook;
    SCI_REGISTER_TX_HOOK            register_tx_hook;
    SCI_NOTITY_TX_DATA_READY        notify_tx_data_ready;
    SCI_CONFIG_AS_MPCM              config_as_mpcm_hook;
    SCI_SET_MPCM_FILTER_HOOK        set_mpcm_filter_hook;
}
SCI_CHANNEL_HOOK_LIST;

typedef struct
{
    SCI_CHANNEL_HOOK_LIST*	        hook_list_ptr;
    SCI_CHANNEL_ID                  channel_id;
}
SCI_CHANNEL_STRUCT;

typedef SCI_CHANNEL_STRUCT*         SCI_CHANNEL_HNDL;

typedef enum
{
    RS485_RX_LEVEL_LOW,
    RS485_RX_LEVEL_HIGH,
}
RS485_RX_LEVEL;

typedef void (*DRVSCI_MSG_COMPLETE)(SCI_CHANNEL_HNDL channel_hndl);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern DRVSCI_MSG_COMPLETE          drvscichannel_msg_complete_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/// inits the module
/// @remark must be done before using this module
void DrvSciChannel_Init(void);

/// init a sci channel
/// @remark a config must be done before using the the other functions in this driver
BOOL DrvSciChannel_Config(SCI_CHANNEL_HNDL channel_hndl, SCI_CONFIG_STRUCT* config_struct_ptr);

/// register an event for when a byte is received on the rx pin, the event prototype will supply the data
/// @return TRUE is the event was successfully registered, FALSE is something went wrong (unexisting channel,...)
BOOL DrvSciChannel_RegisterEvent_RxDataReceived(SCI_CHANNEL_HNDL channel_hndl, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);

/// register an event for whenever a byte can be send by the driver
/// @param "tx_get_next_byte_hook" the event must supply the data and also return if more data is available, for more info see the SCI_TX_GET_NEXT_BYTE_HOOK prototype
/// @return TRUE is the event was successfully registered, FALSE is something went wrong (unexisting channel,...)
/// @remark to start a transmission of data one must thus first register this function that will supply the data, and also call the DrvSciChannel_NotifyTxDataReady function to start the transmission
BOOL DrvSciChannel_RegisterEvent_TxDataNeeded(SCI_CHANNEL_HNDL channel_hndl, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);

/// flags the driver that the tx data is ready
/// @remark after calling this function the driver will call the event registered by DrvSciChannel_RegisterEvent_TxDataNeeded to fetch the tx data
BOOL DrvSciChannel_NotifyTxDataReady(SCI_CHANNEL_HNDL channel_hndl);

/// register an event that will be called when all the tx data is away
BOOL DrvSciChannel_RegisterEvent_TxComplete(DRVSCI_MSG_COMPLETE msg_complete_hook);

/// config the channel as an rs485 port
/// @remark this function is not a stand alone config function ! so it is meant to be used after a DrvSciChannel_Config or DrvSciChannel_ConfigAsMpcm !!!
BOOL DrvSciChannel_ConfigAsRs485(SCI_CHANNEL_HNDL channel_hndl, DRVGPIO_PIN_HNDL pin_de, RS485_RX_LEVEL rx_level);

/// config the channel as an Mpcm port (Multi-processor Communication mode, a name coming from old 8bit atmels)
/// @remark this function is a stand alone config function, so you cannot use it together with the normal DrvSciChannel_Config
BOOL DrvSciChannel_ConfigAsMpcm(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED speed, BOOL allow_rx);

/// function to enable/disable the mpcm address filtering
/// @param "enable" when TRUE only rx bytes where the 9th address indication bit is set will be received
BOOL DrvSciChannel_SetMpcmFilter(SCI_CHANNEL_HNDL channel_hndl, BOOL enable);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// REVERSE COMPATIBLE DEFINITION LIST
//------------------------------------------------------------------------------------------------//
#define DrvSciChannel_RegisterMsgComplete(msg_complete_hook)                DrvSciChannel_RegisterEvent_TxComplete(msg_complete_hook)
#define DrvSciChannel_RegisterRxHook(channel_hndl, rx_new_byte_hook)        DrvSciChannel_RegisterEvent_RxDataReceived(channel_hndl, rx_new_byte_hook)
#define DrvSciChannel_RegisterTxHook(channel_hndl, tx_get_next_byte_hook)   DrvSciChannel_RegisterEvent_TxDataNeeded(channel_hndl, tx_get_next_byte_hook)
//================================================================================================//



#endif /* SCI__DRVSCICHANNEL_H */
