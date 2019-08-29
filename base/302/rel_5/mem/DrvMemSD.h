//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the memory interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MEM__DRVMEMSD_H
#define MEM__DRVMEMSD_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//DRV include section
#include "mem\DrvMem.h"
#include "spi\DrvSpiMasterDevice.h"
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
void DrvMemSD_Init(void);

void DrvMemSD_Handler(void);

MEM_HNDL DrvMemSD_Register(SPI_DEVICE_ID spi_device_id, DRVGPIO_PIN_HNDL pin_cd_hndl, DRVGPIO_PIN_HNDL pin_wp_hndl, U32 spi_speed);

BOOL DrvMemSD_IsPresent(MEM_HNDL mem_hndl);

U32 DrvMemSD_GetCardSize(MEM_HNDL mem_hndl);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* MEM__DRVMEMSD_H */
