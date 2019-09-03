//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking I2C Master Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define I2C__DRVI2CMASTERCHANNELSYSINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef I2C__DRVI2CMASTERCHANNELSYSINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL              I2C__DRVI2CMASTERCHANNELSYSINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking I2C Master channels
#ifndef DRVI2CMASTERCHANNELSYSINT_COUNT
	#define DRVI2CMASTERCHANNELSYSINT_COUNT			I2C_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "i2c\DrvI2cMasterChannelSysInt.h"
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
static BOOL DrvI2cMasterChannelSysInt_WriteData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count);
static BOOL DrvI2cMasterChannelSysInt_ReadData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count);
static BOOL DrvI2cMasterChannelSysInt_ReadData_specific_slave_reg(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count, U16 slave_reg, U16 addr_len);
static BOOL DrvI2cMasterChannelSysInt_Config(I2C_CHANNEL_ID channel_id, I2C_CONFIG_STRUCT* config_struct_ptr);
static void DrvI2cMasterChannelSysInt_MsgComplete(I2C_CHANNEL channel, BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const I2C_CHANNEL_HOOK_LIST          i2c_channel_hook_list = {DrvI2cMasterChannelSysInt_WriteData, DrvI2cMasterChannelSysInt_ReadData, DrvI2cMasterChannelSysInt_ReadData_specific_slave_reg, DrvI2cMasterChannelSysInt_Config};
static I2C_CHANNEL_STRUCT                   i2c_channel_struct[DRVI2CMASTERCHANNELSYSINT_COUNT];
static U8                                   i2c_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvI2cMasterChannelSysInt_WriteData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count)
{
    return SysI2cMasterInt_Channel_WriteData((I2C_CHANNEL)channel_id, address, data_ptr, count);
}

//------------------------------------------------------------------------------------------------//
static BOOL DrvI2cMasterChannelSysInt_ReadData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count)
{
    return SysI2cMasterInt_Channel_ReadData((I2C_CHANNEL)channel_id, address, data_ptr, count);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvI2cMasterChannelSysInt_ReadData_specific_slave_reg(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count, U16 slave_reg, U16 addr_len)
{
    return SysI2cMasterInt_Channel_ReadData_specific_slave_reg((I2C_CHANNEL_ID)channel_id, address, data_ptr, count, slave_reg, addr_len);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvI2cMasterChannelSysInt_Config(I2C_CHANNEL_ID channel_id, I2C_CONFIG_STRUCT* config_struct_ptr)
{
    return SysI2cMasterInt_Channel_Config((I2C_CHANNEL)channel_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static void DrvI2cMasterChannelSysInt_MsgComplete(I2C_CHANNEL channel, BOOL success)
{
    I2C_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("DrvI2cMasterChannelSysInt_MsgComplete");
    if(drvi2cmasterchannel_msg_complete_hook != NULL)
    {
        for(channel_hndl = i2c_channel_struct; channel_hndl <= &i2c_channel_struct[i2c_channel_count]; channel_hndl++)
        {
            if(channel_hndl->channel_id == (U8)channel)
            {
                drvi2cmasterchannel_msg_complete_hook(channel_hndl, success);
                break;
            }
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvI2cMasterChannelSysInt_Init(void)
{
    LOG_DEV("DrvI2cMasterChannelSysInt_Init");
    SysI2cMasterInt_Init();
    SysI2cMasterInt_RegisterMsgComplete(DrvI2cMasterChannelSysInt_MsgComplete);
    
    MEMSET((VPTR)i2c_channel_struct, 0, SIZEOF(i2c_channel_struct));
    i2c_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
I2C_CHANNEL_HNDL DrvI2cMasterChannelSysInt_Register(I2C_CHANNEL channel)
{
    I2C_CHANNEL_HNDL    channel_hndl;
    
    LOG_DEV("DrvI2cMasterChannelSysInt_Register");
    for(channel_hndl = i2c_channel_struct; channel_hndl < &i2c_channel_struct[i2c_channel_count]; channel_hndl++)
    {
        if(channel_hndl->channel_id == channel)
        {
            return channel_hndl;
        }
    }
    if(i2c_channel_count < DRVI2CMASTERCHANNELSYSINT_COUNT)
    {
        if(SysI2cMasterInt_Channel_Init(channel))
        {
            channel_hndl->hook_list_ptr = (I2C_CHANNEL_HOOK_LIST*)&i2c_channel_hook_list;
            channel_hndl->channel_id = (U8)channel;
            i2c_channel_count++;
            return channel_hndl;
        }
    }
    LOG_ERR("Illegal I2C channel - %d", PU8(channel));
    return NULL;
}
//================================================================================================//
