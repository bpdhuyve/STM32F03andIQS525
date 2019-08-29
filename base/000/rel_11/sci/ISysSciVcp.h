//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the interrupt driven SCI system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SCI__ISYSSCIVCP_H
#define SCI__ISYSSCIVCP_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines if MPCM has to be included
#ifndef INCLUDE_MPCM
    //#define INCLUDE_MPCM to enable the MPCM functions
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "sci\ISysSci.h"
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

void SysSciVcp_Init(void);


BOOL SysSciVcp_Channel_Init(SCI_CHANNEL channel);

BOOL SysSciVcp_Channel_Config(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr);

BOOL SysSciVcp_Channel_RegisterRxHook(SCI_CHANNEL channel, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);

BOOL SysSciVcp_Channel_RegisterTxHook(SCI_CHANNEL channel, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);

BOOL SysSciVcp_Channel_NotityTxDataReady(SCI_CHANNEL channel);


//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SCI__ISYSSCIINT_H */
