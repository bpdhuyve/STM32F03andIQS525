//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the interrupt driven I2C Master system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef I2C__ISYSI2CMASTERINT_H
#define I2C__ISYSI2CMASTERINT_H
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
void SysI2cMasterInt_Init(void);

BOOL SysI2cMasterInt_RegisterMsgComplete(I2C_MSG_COMPLETE msg_complete_hook);

BOOL SysI2cMasterInt_Channel_Init(I2C_CHANNEL channel);

BOOL SysI2cMasterInt_Channel_Config(I2C_CHANNEL channel, I2C_CONFIG_STRUCT* config_struct_ptr);

BOOL SysI2cMasterInt_Channel_WriteData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U16 count);

BOOL SysI2cMasterInt_Channel_ReadData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U16 count);


//------------------------------------------------------------------------------------------------//
// @remark  Function that initiates a zrite operation to an I2C device, zriting to a specific register on the slave
// @param   device_id: the ID of the I2C device which was returned after registering the device with
//                     DrvI2cMasterDevice_Register(channel, address, speed)
// @param   buffer_ptr: pointer to the first U8 byte of a receive buffer
// @param   count: the number of bytes to be received
// @param   slave_reg_address: address of the register on the slave to write to
// @param   wait_to_complete: if FALSE, DrvI2cMasterDevice_ReadData will return TRUE immediately
//                            after initiating the transfer. A message complete call back hook
//                            should be registered with DrvI2cMasterDevice_MsgComplete in order
//                            to get feedback from the DrvI2cMasterDevice module.
//                            if TRUE, DrvI2cMasterDevice_ReadData will return TRUE if the transfer
//                            was finished with success and in time (if the wait to complete timeout
//                            task define is active), else it will return FALSE
BOOL SysI2cMasterInt_Channel_ReadData_specific_slave_reg(I2C_CHANNEL channel_id, U8 address, U8* data_ptr, U16 count, U16 slave_reg);

//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* I2C__ISYSI2CMASTERINT_H */
