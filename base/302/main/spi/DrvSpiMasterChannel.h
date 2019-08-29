//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the SPI Master Channel driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SPI__DRVSPIMASTERCHANNEL_H
#define SPI__DRVSPIMASTERCHANNEL_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "spi\ISysSpi.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                  SPI_CHANNEL_ID;

typedef BOOL (*SPI_EXCHANGE_HOOK)(SPI_CHANNEL_ID channel_id, U8* write_data_ptr, U8* read_data_ptr, U16 count);

typedef BOOL (*SPI_CONFIG_HOOK)(SPI_CHANNEL_ID channel_id, SPI_CONFIG_STRUCT* config_struct_ptr);

typedef struct
{
    SPI_EXCHANGE_HOOK               exchange_hook;
    SPI_CONFIG_HOOK                 config_hook;
}
SPI_CHANNEL_HOOK_LIST;

typedef struct
{
    SPI_CHANNEL_HOOK_LIST*	        hook_list_ptr;
    SPI_CHANNEL_ID                  channel_id;
}
SPI_CHANNEL_STRUCT;

typedef SPI_CHANNEL_STRUCT*         SPI_CHANNEL_HNDL;

typedef void (*DRVSPI_MSG_COMPLETE)(SPI_CHANNEL_HNDL channel_hndl);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern DRVSPI_MSG_COMPLETE         drvspimasterchannel_msg_complete_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvSpiMasterChannel_Init(void);

BOOL DrvSpiMasterChannel_RegisterMsgComplete(DRVSPI_MSG_COMPLETE msg_complete_hook);


///@brief used to send/receive data over SPI
///@param write_data_ptr: ptr2 data to be written
///       read_data_ptr: ptr2 read data
///       count: # bytes
///@retvalue
BOOL DrvSpiMasterChannel_Exchange(SPI_CHANNEL_HNDL channel_hndl, U8* write_data_ptr, U8* read_data_ptr, U16 count);

BOOL DrvSpiMasterChannel_Config(SPI_CHANNEL_HNDL channel_hndl, SPI_CONFIG_STRUCT* config_struct_ptr);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SPI__DRVSPIMASTERCHANNEL_H */
