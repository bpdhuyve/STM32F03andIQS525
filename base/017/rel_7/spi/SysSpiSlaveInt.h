//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor dependent prototypes and definitions for the interrupt based SPI slave system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SYSSPISLAVEINT_H
#define SYSSPISLAVEINT_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "spi\ISysSpi.h"

//SYS include section
#include "spi\SysSpi.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
typedef void (*SPI_RX_NEW_U8_DATA_HOOK)(U8* data_ptr, U8 length);
typedef void (*SPI_RX_NEW_U16_DATA_HOOK)(U16* data_ptr, U8 length);

/// prototype for the TxDataNeeded event
/// @param "data_ptr" pointer to a piece of memory where the event much copy the data, how much data is specified by the length argument
/// @param "length" the length of the data that the driver wants to fetch, this is the datacount that must be copyed into the data_ptr
/// @return the number of data bytes that are available in the event, return 0 if you dont have anymore data to send
typedef U8 (*SPI_TX_GET_NEXT_U8_DATA_HOOK)(U8* data_ptr, U8 length);
typedef U8 (*SPI_TX_GET_NEXT_U16_DATA_HOOK)(U16* data_ptr, U8 length);
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
// @remark  Init function
void SysSpiSlaveInt_Init(void);

BOOL SysSpiSlaveInt_Channel_Init(SPI_CHANNEL channel);

BOOL SysSpiSlaveInt_Channel_Config(SPI_CHANNEL channel, SPI_CONFIG_STRUCT* config_struct_ptr);

BOOL SysSpiSlaveInt_Channel_RegisterRxU8DataHook(SPI_CHANNEL channel, SPI_RX_NEW_U8_DATA_HOOK rx_new_data_hook);

BOOL SysSpiSlaveInt_Channel_RegisterRxU16DataHook(SPI_CHANNEL channel, SPI_RX_NEW_U16_DATA_HOOK rx_new_data_hook);

BOOL SysSpiSlaveInt_Channel_RegisterTxU8DataHook(SPI_CHANNEL channel, SPI_TX_GET_NEXT_U8_DATA_HOOK tx_get_next_data_hook);

BOOL SysSpiSlaveInt_Channel_RegisterTxU16DataHook(SPI_CHANNEL channel, SPI_TX_GET_NEXT_U16_DATA_HOOK tx_get_next_data_hook);

U16  SysSpiSlaveInt_GetOverrunErrorCount(SPI_CHANNEL channel);

U16  SysSpiSlaveInt_GetFrameFormatErrorCount(SPI_CHANNEL channel);

U16  SysSpiSlaveInt_GetCrcErrorCount(SPI_CHANNEL channel);

U16  SysSpiSlaveInt_GetUnderrunErrorCount(SPI_CHANNEL channel);

//================================================================================================//



#endif /* SYSSPISLAVEINT_H */
