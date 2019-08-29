//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the CAN Channel driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CAN__DRVCANCHANNEL_H
#define CAN__DRVCANCHANNEL_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "can\IsysCan.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8 CAN_CHANNEL_ID;

typedef BOOL (*CAN_CONFIG)(CAN_CHANNEL_ID channel_id, CAN_CONFIG_STRUCT* config_struct_ptr);

typedef BOOL (*CAN_CONFIG_MAILBOXES)(CAN_CHANNEL_ID channel_id, CONFIG_SCHEME cfg, U32 node_info);

typedef BOOL (*CAN_REGISTER_RX_HOOK)(CAN_CHANNEL_ID channel_id, CAN_RX_NEW_MSSG_HOOK rx_hook);

typedef BOOL (*CAN_REGISTER_TX_HOOK)(CAN_CHANNEL_ID channel_id, CAN_TX_GET_NEXT_MSSG_HOOK tx_hook);

typedef BOOL (*CAN_REGISTER_ERROR_HOOK)(CAN_CHANNEL_ID channel_id, CAN_ERROR_HOOK error_hook);

typedef BOOL (*CAN_NOTIFY_TX_MESSAGE_READY)(CAN_CHANNEL_ID channel_id);

typedef BOOL (*CAN_RECOVER_FROM_BUS_OFF)(CAN_CHANNEL_ID channel_id);

typedef struct
{
    CAN_CONFIG                      config_hook;
    CAN_CONFIG_MAILBOXES            config_mailboxes_hook;
	CAN_REGISTER_RX_HOOK            register_rx_hook;
	CAN_REGISTER_TX_HOOK            register_tx_hook;
	CAN_REGISTER_ERROR_HOOK			register_error_hook;
    CAN_NOTIFY_TX_MESSAGE_READY     notify_tx_message_ready_hook;
    CAN_RECOVER_FROM_BUS_OFF        recover_from_bus_off_hook;
}
CAN_CHANNEL_HOOK_LIST;

typedef struct
{
    CAN_CHANNEL_HOOK_LIST*	        hook_list_ptr;
    CAN_CHANNEL_ID                  channel_id;
}
CAN_CHANNEL_STRUCT;

typedef CAN_CHANNEL_STRUCT*         CAN_CHANNEL_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvCanChannel_Init(void);

BOOL DrvCanChannel_Config(CAN_CHANNEL_HNDL channel_hndl, CAN_CONFIG_STRUCT* config);

BOOL DrvCanChannel_ConfigMailboxes(CAN_CHANNEL_HNDL channel_hndl, CONFIG_SCHEME cfg, U32 node_info);

BOOL DrvCanChannel_RegisterRxHook(CAN_CHANNEL_HNDL channel_hndl, CAN_RX_NEW_MSSG_HOOK rx_hook);

BOOL DrvCanChannel_RegisterTxHook(CAN_CHANNEL_HNDL channel_hndl, CAN_TX_GET_NEXT_MSSG_HOOK tx_hook);

BOOL DrvCanChannel_RegisterErrorHook(CAN_CHANNEL_HNDL channel_hndl, CAN_ERROR_HOOK error_hook);

BOOL DrvCanChannel_NotityTxMessageReady(CAN_CHANNEL_HNDL channel_hndl);

BOOL DrvCanChannel_RecoverFromBusOff(CAN_CHANNEL_HNDL channel_hndl);

void DrvCanChannel_FillExtDataFrameMssgStruct(CAN_MSSG_STRUCT* mssg_ptr, U32 identifier, U8 dlc, U8* data_ptr);
//================================================================================================//

#endif /* CAN__DRVCANCHANNEL_H */
