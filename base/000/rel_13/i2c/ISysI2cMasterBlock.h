//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the interrupt driven I2C Master system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef I2C__ISYSI2CMASTERBLOCK_H
#define I2C__ISYSI2CMASTERBLOCK_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "i2c\ISysI2c.h"
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
void SysI2cMasterBlock_Init(void);

BOOL SysI2cMasterBlock_RegisterMsgComplete(I2C_MSG_COMPLETE msg_complete_hook);

BOOL SysI2cMasterBlock_Channel_Init(I2C_CHANNEL channel);

BOOL SysI2cMasterBlock_Channel_Config(I2C_CHANNEL channel, I2C_CONFIG_STRUCT* config_struct_ptr);

BOOL SysI2cMasterBlock_Channel_WriteData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U16 count);

BOOL SysI2cMasterBlock_Channel_ReadData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U16 count);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* I2C__ISYSI2CMASTERBLOCK_H */
