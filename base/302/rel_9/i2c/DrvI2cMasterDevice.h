//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the I2C Master Device driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef I2C__DRVI2CMASTERDEVICE_H
#define I2C__DRVI2CMASTERDEVICE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "i2c\DrvI2cMasterChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_I2C_DEVICE_ID               0xFF
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          I2C_DEVICE_ID;

typedef void (*DRVI2CDEVICE_MSG_COMPLETE)(BOOL success);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvI2cMasterDevice_Init(void);
//------------------------------------------------------------------------------------------------//
I2C_DEVICE_ID DrvI2cMasterDevice_Register(I2C_CHANNEL_HNDL i2c_channel_hndl, U8 address, U32 speed);
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterDevice_Config(I2C_DEVICE_ID device_id, I2C_CONFIG_STRUCT* config_struct_ptr);
//------------------------------------------------------------------------------------------------//
// @remark  Function that registers a message complete hook which can be called on the completion
//          of a transfer. If something went wrong during the transfer, the call back hook
//          is also called. Upon a correct transfer, the function argument success is TRUE and
//          FALSE if the transfer was unsuccesful.
//          This message complete hook is usefull if the transfer is characterised with
//          wait_to_complete equal to FALSE
BOOL DrvI2cMasterDevice_MsgComplete(I2C_DEVICE_ID device_id, DRVI2CDEVICE_MSG_COMPLETE msg_complete);
//------------------------------------------------------------------------------------------------//
// @remark  Function that initiates a write operation to an I2C device.
// @param   device_id: the ID of the I2C device which was returned after registering the device with
//                     DrvI2cMasterDevice_Register(channel, address, speed)
// @param   buffer_ptr: pointer to the first U8 byte to be sent
// @param   count: the number of bytes to be sent
// @param   wait_to_complete: if FALSE, DrvI2cMasterDevice_WriteData will return TRUE immediately
//                            after initiating the transfer. A message complete call back hook
//                            should be registered with DrvI2cMasterDevice_MsgComplete in order
//                            to get feedback from the DrvI2cMasterDevice module.
//                            if TRUE, DrvI2cMasterDevice_WriteData will return TRUE if the transfer
//                            was finished with success and in time (if the wait to complete timeout
//                            task define is active), else it will return FALSE
BOOL DrvI2cMasterDevice_WriteData(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete);
//------------------------------------------------------------------------------------------------//
// @remark  Function that initiates a read operation to an I2C device.
// @param   device_id: the ID of the I2C device which was returned after registering the device with
//                     DrvI2cMasterDevice_Register(channel, address, speed)
// @param   buffer_ptr: pointer to the first U8 byte of a receive buffer
// @param   count: the number of bytes to be received
// @param   wait_to_complete: if FALSE, DrvI2cMasterDevice_ReadData will return TRUE immediately
//                            after initiating the transfer. A message complete call back hook
//                            should be registered with DrvI2cMasterDevice_MsgComplete in order
//                            to get feedback from the DrvI2cMasterDevice module.
//                            if TRUE, DrvI2cMasterDevice_ReadData will return TRUE if the transfer
//                            was finished with success and in time (if the wait to complete timeout
//                            task define is active), else it will return FALSE
BOOL DrvI2cMasterDevice_ReadData(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete);

BOOL DrvI2cMasterDevice_ReadData_specificSlaveRegister(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete, U16 slave_reg_address);

BOOL DrvI2cMasterDevice_WriteData_specificSlaveRegister(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete, U16 slave_reg_address);
//------------------------------------------------------------------------------------------------//

void DrvI2cMasterDevice_ChangeAddress(I2C_DEVICE_ID device_id, U8 address);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* I2C__DRVI2CMASTERDEVICE_H */
