//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the common part of the I2C Master Channel driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define I2C__DRVI2CMASTERCHANNEL_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef I2C__DRVI2CMASTERCHANNEL_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               I2C__DRVI2CMASTERCHANNEL_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "i2c\DrvI2cMasterChannel.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
DRVI2C_MSG_COMPLETE                         drvi2cmasterchannel_msg_complete_hook;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvI2cMasterChannel_Init(void)
{
    LOG_DEV("DrvI2cMasterChannel_Init");
    drvi2cmasterchannel_msg_complete_hook = NULL;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterChannel_RegisterMsgComplete(DRVI2C_MSG_COMPLETE msg_complete_hook)
{
    LOG_DEV("DrvI2cMasterChannel_RegisterMsgComplete");
    drvi2cmasterchannel_msg_complete_hook = msg_complete_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterChannel_WriteData(I2C_CHANNEL_HNDL channel_hndl, U8 address, U8* write_data_ptr, U16 count)
{
    LOG_DEV("DrvI2cMasterChannel_WriteData");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->write_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->write_hook(channel_hndl->channel_id, address, write_data_ptr, count);
    }
    LOG_WRN("I2C write function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterChannel_WriteData_specificSlaveRegister(I2C_CHANNEL_HNDL channel_hndl, U8 address, U8* write_data_ptr, U16 count, U16 slave_reg_address)
{
    LOG_DEV("DrvI2cMasterChannel_ReadData");
  
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->read_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->write_hook(channel_hndl->channel_id, address, write_data_ptr, count);
    }
    LOG_WRN("I2C read function is NULL");
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterChannel_ReadData(I2C_CHANNEL_HNDL channel_hndl, U8 address, U8* read_data_ptr, U16 count)
{
    LOG_DEV("DrvI2cMasterChannel_ReadData");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->read_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->read_hook(channel_hndl->channel_id, address, read_data_ptr, count);
    }
    LOG_WRN("I2C read function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterChannel_ReadData_specificSlaveRegister(I2C_CHANNEL_HNDL channel_hndl, U8 address, U8* read_data_ptr, U16 count, U16 slave_reg_address)
{
    LOG_DEV("DrvI2cMasterChannel_ReadData");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->read_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->read_hook_specificSlaveRegister(channel_hndl->channel_id, address, read_data_ptr, count, slave_reg_address);
    }
    LOG_WRN("I2C read function is NULL");
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterChannel_Config(I2C_CHANNEL_HNDL channel_hndl, I2C_CONFIG_STRUCT* config_struct_ptr)
{
    LOG_DEV("DrvI2cMasterChannel_Config");
    
    if((channel_hndl != NULL) && (channel_hndl->hook_list_ptr != NULL) && (channel_hndl->hook_list_ptr->config_hook != NULL))
    {
        return channel_hndl->hook_list_ptr->config_hook(channel_hndl->channel_id, config_struct_ptr);
    }
    LOG_WRN("I2C config function is NULL");
    return FALSE;
}
//================================================================================================//
