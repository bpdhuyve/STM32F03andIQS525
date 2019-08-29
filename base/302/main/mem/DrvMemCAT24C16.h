//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of M24512 I�C Eeprom
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MEM__DRVMEMCAT24C16_H
#define MEM__DRVMEMCAT24C16_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "DrvMem.h"
#include "i2c\DrvI2cMasterChannel.h"
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
// @remark  Init function
void DrvMemCat24C16_Init(void);

// @remark  Background handler
MEM_HNDL DrvMemCat24C16_Register(I2C_CHANNEL_HNDL i2c_channel, U8 i2c_address, U32 flash_address, U16 page_count);
//================================================================================================//



#endif /* MEM__DRVMEMCAT24C16_H */
