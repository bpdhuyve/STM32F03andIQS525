//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the blocking SPI Master Channel driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MEM__DRVMEMSYSFLASH_H
#define MEM__DRVMEMSYSFLASH_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//DRV include section
#include "mem\DrvMem.h"
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
void DrvMemSysFlash_Init(void);

MEM_HNDL DrvMemSysFlash_Register(U32 address, U32 length, U16 sector_size);

BOOL DrvMemSysFlash_Update(MEM_HNDL mem_hndl, U32 address, U32 length);

BOOL DrvMemSysFlash_GetInfo(MEM_HNDL mem_hndl, U32* address_ptr, U32* length_ptr);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* MEM__DRVMEMSYSFLASH_H */
