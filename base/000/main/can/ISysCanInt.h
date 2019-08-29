//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor       : independant
// Implementation  : specific
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CAN__ISYSCANINT_H
#define CAN__ISYSCANINT_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "can\ISysCan.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void SysCanInt_Init(void);

BOOL SysCanInt_Channel_Init(CAN_CHANNEL channel);

BOOL SysCanInt_Channel_Config(CAN_CHANNEL channel, CAN_CONFIG_STRUCT* config_struct_ptr);

BOOL SysCanInt_Channel_ConfigMailboxes(CAN_CHANNEL channel, CONFIG_SCHEME cfg, U32 node_info);

BOOL SysCanInt_Channel_ConfigBitTimeAnalyser(CAN_CHANNEL channel, CAN_TIME_ANALYSE_MODE mode, BOOL capture_all);

BOOL SysCanInt_Channel_RegisterRxHook(CAN_CHANNEL channel, CAN_RX_NEW_MSSG_HOOK hook);

BOOL SysCanInt_Channel_RegisterTxHook(CAN_CHANNEL channel, CAN_TX_GET_NEXT_MSSG_HOOK hook);

BOOL SysCanInt_Channel_RegisterErrorHook(CAN_CHANNEL channel, CAN_ERROR_HOOK hook);

BOOL SysCanInt_Channel_RegisterBitTimeAnalyseHook(CAN_CHANNEL channel, CAN_BIT_TIME_ANALYSE_HOOK hook);

BOOL SysCanInt_Channel_NotifyTxMessageReady(CAN_CHANNEL channel);

BOOL SysCanInt_Channel_RecoverFromBusOff(CAN_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CAN__ISYSCANINT_H */
