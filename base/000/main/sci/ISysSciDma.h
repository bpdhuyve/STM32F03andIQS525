//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the interrupt driven SCI system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SCI__ISYSSCIDMA_H
#define SCI__ISYSSCIDMA_H
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
void SysSciDma_Init(void);

//hook voor alle channels samen ?, zou dit niet moeten zijn voor enkel channel 1 hook ?
BOOL SysSciDma_RegisterMsgComplete(SCI_MSG_COMPLETE msg_complete_hook);

BOOL SysSciDma_Channel_Init(SCI_CHANNEL sci_channel, DMA_CHANNEL dma_rx_channel, U16 rx_buffer_size, DMA_CHANNEL dma_tx_channel, U16 tx_buffer_size);

BOOL SysSciDma_Channel_Config(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr);

BOOL SysSciDma_Channel_RegisterRxHook(SCI_CHANNEL channel, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);

BOOL SysSciDma_Channel_RegisterTxHook(SCI_CHANNEL channel, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);

BOOL SysSciDma_Channel_NotityTxDataReady(SCI_CHANNEL channel);

#ifdef INCLUDE_MPCM
BOOL SysSciDma_Channel_ConfigAsMpcm(SCI_CHANNEL channel, SCI_SPEED speed, BOOL allow_rx);

BOOL SysSciDma_Channel_SetMpcmFilter(SCI_CHANNEL channel, BOOL enable);
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SCI__ISYSSCIDMA_H */
