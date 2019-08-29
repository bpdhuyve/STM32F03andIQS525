//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the I2C Master Device driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define I2C__DRVI2CMASTERDEVICE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef I2C__DRVI2CMASTERDEVICE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               I2C__DRVI2CMASTERDEVICE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of I2C Master devices
#ifndef I2C_DEVICE_COUNT
	#define I2C_DEVICE_COUNT			                3
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "DrvI2cMasterDevice.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    I2C_CHANNEL_HNDL            i2c_channel_hndl;
    U8                          address;
    I2C_CONFIG_STRUCT           config_struct;
    DRVI2CDEVICE_MSG_COMPLETE   msg_complete;
    BOOL                        active;
}
I2C_DEVICE_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrvI2cMasterDevice_ChannelCallBack(I2C_CHANNEL_HNDL channel_hndl, BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static I2C_DEVICE_STRUCT            i2c_device_struct[I2C_DEVICE_COUNT];
static U8                           i2c_device_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DrvI2cMasterDevice_ChannelCallBack(I2C_CHANNEL_HNDL channel_hndl, BOOL success)
{
    U8              device_id;
    I2C_DEVICE_STRUCT*      i2c_dev_ptr;
    
    LOG_DEV("DrvI2cMasterDevice_ChannelCallBack");
    
    for(i2c_dev_ptr = i2c_device_struct, device_id = 0; i2c_dev_ptr < &i2c_device_struct[i2c_device_count]; i2c_dev_ptr++, device_id++)
    {
        if((i2c_dev_ptr->i2c_channel_hndl == channel_hndl) && (i2c_dev_ptr->active == TRUE))
        {
            break;
        }
    }
    
    if(i2c_dev_ptr != &i2c_device_struct[i2c_device_count])
    {
        i2c_dev_ptr->active = FALSE;
        
        if(i2c_dev_ptr->msg_complete != NULL)
        {
            i2c_dev_ptr->msg_complete(success);
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvI2cMasterDevice_Init(void)
{
    LOG_DEV("DrvI2cMasterDevice_Init");
    i2c_device_count = 0;
    MEMSET((VPTR)i2c_device_struct, 0, SIZEOF(i2c_device_struct));
    
    DrvI2cMasterChannel_RegisterMsgComplete(DrvI2cMasterDevice_ChannelCallBack);
}
//------------------------------------------------------------------------------------------------//
I2C_DEVICE_ID DrvI2cMasterDevice_Register(I2C_CHANNEL_HNDL i2c_channel_hndl, U8 address, U32 speed)
{
    I2C_DEVICE_STRUCT*      i2c_dev_hndl = &i2c_device_struct[i2c_device_count];
    
    LOG_DEV("DrvI2cMasterDevice_Register");
    
    if(i2c_channel_hndl == NULL)
    {
        LOG_ERR("illegal i2c channel");
        return INVALID_I2C_DEVICE_ID;
    }
    
    if(i2c_device_count < I2C_DEVICE_COUNT)
    {
        i2c_dev_hndl->config_struct.speed = speed;
        i2c_dev_hndl->i2c_channel_hndl = i2c_channel_hndl;
        i2c_dev_hndl->address = address;
        i2c_device_count++;
        
        return (i2c_device_count-1);
    }
    
    LOG_ERR("I2C device count overrun");
    return INVALID_I2C_DEVICE_ID;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterDevice_Config(I2C_DEVICE_ID device_id, I2C_CONFIG_STRUCT* config_struct_ptr)
{
    LOG_DEV("DrvI2cMasterDevice_Config");
    
    if(device_id < i2c_device_count)
    {
        if(config_struct_ptr != NULL)
        {
            MEMCPY((VPTR)&i2c_device_struct[device_id].config_struct, (VPTR)config_struct_ptr, SIZEOF(I2C_CONFIG_STRUCT));
            return TRUE;
        }
    }
    LOG_WRN("I2C illegal device config - %d", PU8(device_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterDevice_MsgComplete(I2C_DEVICE_ID device_id, DRVI2CDEVICE_MSG_COMPLETE msg_complete)
{
    LOG_DEV("DrvI2cMasterDevice_MsgComplete");
    
    if(device_id < i2c_device_count)
    {
        i2c_device_struct[device_id].msg_complete = msg_complete;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterDevice_WriteData(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete)
{
    I2C_DEVICE_STRUCT*      i2c_dev_hndl = &i2c_device_struct[device_id];
    I2C_DEVICE_STRUCT*      i2c_dev_ptr;
    
    LOG_DEV("DrvI2cMasterDevice_WriteData - %d", PU8(device_id));
    
    if(device_id < i2c_device_count)
    {
        for(i2c_dev_ptr = i2c_device_struct; i2c_dev_ptr < &i2c_device_struct[i2c_device_count]; i2c_dev_ptr++)
        {
            if((i2c_dev_ptr->i2c_channel_hndl == i2c_dev_hndl->i2c_channel_hndl) &&
               (i2c_dev_ptr->active == TRUE))
            {
                LOG_WRN("I2C channel is busy (W) - %d", PU8(device_id));
                return FALSE;
            }
        }
        
        i2c_dev_hndl->active = TRUE;
        
        if(DrvI2cMasterChannel_Config(i2c_dev_hndl->i2c_channel_hndl, &(i2c_dev_hndl->config_struct)) &&
           DrvI2cMasterChannel_WriteData(i2c_dev_hndl->i2c_channel_hndl, i2c_dev_hndl->address, buffer_ptr, count))
        {
            if(wait_to_complete)
            {
                while(i2c_dev_hndl->active)
                {}
            }
            return TRUE;
        }
        
        i2c_dev_hndl->active = FALSE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterDevice_ReadData(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete)
{
    I2C_DEVICE_STRUCT*      i2c_dev_hndl = &i2c_device_struct[device_id];
    I2C_DEVICE_STRUCT*      i2c_dev_ptr;
    
    LOG_DEV("DrvI2cMasterDevice_ReadData - %d", PU8(device_id));
    
    if(device_id < i2c_device_count)
    {
        for(i2c_dev_ptr = i2c_device_struct; i2c_dev_ptr < &i2c_device_struct[i2c_device_count]; i2c_dev_ptr++)
        {
            if((i2c_dev_ptr->i2c_channel_hndl == i2c_dev_hndl->i2c_channel_hndl) &&
               (i2c_dev_ptr->active == TRUE))
            {
                LOG_WRN("I2C channel is busy (R) - %d", PU8(device_id));
                return FALSE;
            }
        }
        
        i2c_dev_hndl->active = TRUE;
        
        if(DrvI2cMasterChannel_Config(i2c_dev_hndl->i2c_channel_hndl, &(i2c_dev_hndl->config_struct)) &&
           DrvI2cMasterChannel_ReadData(i2c_dev_hndl->i2c_channel_hndl, i2c_dev_hndl->address, buffer_ptr, count))
        {
            if(wait_to_complete)
            {
                while(i2c_dev_hndl->active)
                {}
            }
            return TRUE;
        }
        
        i2c_dev_hndl->active = FALSE;
    }
    return FALSE;
}
//================================================================================================//
