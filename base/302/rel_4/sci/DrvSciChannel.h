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
void DrvSciChannel_Init(void);

BOOL DrvSciChannel_RegisterMsgComplete(DRVSCI_MSG_COMPLETE msg_complete_hook);

BOOL DrvSciChannel_ConfigAsRs485(SCI_CHANNEL_HNDL channel_hndl, DRVGPIO_PIN_HNDL pin_de, RS485_RX_LEVEL rx_level);

BOOL DrvSciChannel_Config(SCI_CHANNEL_HNDL channel_hndl, SCI_CONFIG_STRUCT* config_struct_ptr);

BOOL DrvSciChannel_RegisterRxHook(SCI_CHANNEL_HNDL channel_hndl, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);

BOOL DrvSciChannel_RegisterTxHook(SCI_CHANNEL_HNDL channel_hndl, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);

BOOL DrvSciChannel_NotifyTxDataReady(SCI_CHANNEL_HNDL channel_hndl);

BOOL DrvSciChannel_ConfigAsMpcm(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED speed, BOOL allow_rx);

//@brief enable/disable the mpcm address filtering, when enabled only rx bytes where the 9th address indication bit is high will be received
BOOL DrvSciChannel_SetMpcmFilter(SCI_CHANNEL_HNDL channel_hndl, BOOL enable);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SCI__DRVSCICHANNEL_H */
