//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the SPI Master Device driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SPI__DRVSPIMASTERDEVICE_H
#define SPI__DRVSPIMASTERDEVICE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "spi\DrvSpiMasterChannel.h"
#include "gpio\DrvGpio.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_SPI_DEVICE_ID               0xFF

#define DrvSpiMasterDevice_WriteData(device_id, data_ptr, count)               DrvSpiMasterDevice_Exchange(device_id, data_ptr, NULL, count)
#define DrvSpiMasterDevice_ReadData(device_id, data_ptr, count)                DrvSpiMasterDevice_Exchange(device_id, NULL, data_ptr, count)
#define DrvSpiMasterDevice_ExchangeData(device_id, wr_data, rd_data_ptr)       DrvSpiMasterDevice_Exchange(device_id, &wr_data, rd_data_ptr, 1)

#define DrvSpiMasterDevice_SelectWriteData(device_id, data_ptr, count)         DrvSpiMasterDevice_SelectExchange(device_id, data_ptr, NULL, count)
#define DrvSpiMasterDevice_SelectReadData(device_id, data_ptr, count)          DrvSpiMasterDevice_SelectExchange(device_id, NULL, data_ptr, count)
#define DrvSpiMasterDevice_SelectExchangeData(device_id, wr_data, rd_data_ptr) DrvSpiMasterDevice_SelectExchange(device_id, &wr_data, rd_data_ptr, 1)
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          SPI_DEVICE_ID;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvSpiMasterDevice_Init(void);

SPI_DEVICE_ID DrvSpiMasterDevice_Register(SPI_CHANNEL_HNDL spi_channel_hndl, DRVGPIO_PIN_HNDL pin_cs,
                                         SPI_CONFIG_STRUCT* config_struct_ptr, EVENT_CALLBACK msg_complete);
BOOL DrvSpiMasterDevice_Config(SPI_DEVICE_ID device_id, SPI_CONFIG_STRUCT* config_struct_ptr);
BOOL DrvSpiMasterDevice_MsgComplete(SPI_DEVICE_ID device_id, EVENT_CALLBACK msg_complete);

BOOL DrvSpiMasterDevice_Select(SPI_DEVICE_ID device_id);
BOOL DrvSpiMasterDevice_Deselect(SPI_DEVICE_ID device_id);

BOOL DrvSpiMasterDevice_Exchange(SPI_DEVICE_ID device_id, U8* write_byte_ptr, U8* read_byte_ptr, U16 count);
BOOL DrvSpiMasterDevice_SelectExchange(SPI_DEVICE_ID device_id, U8* write_byte_ptr, U8* read_byte_ptr, U16 count);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SPI__DRVSPIMASTERDEVICE_H */
