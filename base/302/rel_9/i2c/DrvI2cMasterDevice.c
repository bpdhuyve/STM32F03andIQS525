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
//------------------------------------------------------------------------------------------------//
// @brief  Defines if a timeout is used which expires when wait-to-complete takes longer than
//         I2C_DEVICE_WAIT_TO_COMPLETE_TIMEOUT_TIME_IN_MS ms. Mind the priority of the 
//         timeout task!
#ifndef I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK
    #define I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK    0
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the wait-to-complete timeout time
#ifndef I2C_DEVICE_WAIT_TO_COMPLETE_TIMEOUT_TIME_IN_MS
    #define I2C_DEVICE_WAIT_TO_COMPLETE_TIMEOUT_TIME_IN_MS  500
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
    I2C_CONFIG_STRUCT           config_struct;
    DRVI2CDEVICE_MSG_COMPLETE   msg_complete;
    U8                          address;
    BOOL                        active;
    BOOL                        success;
}
I2C_DEVICE_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrvI2cMasterDevice_ChannelCallBack(I2C_CHANNEL_HNDL channel_hndl, BOOL success);

#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
static void DrvI2cMasterDevice_WaitToCompleteTimeout(VPTR data_ptr);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static I2C_DEVICE_STRUCT            i2c_device_struct[I2C_DEVICE_COUNT];
static U8                           i2c_device_count;
U8*                                 data;
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
static volatile BOOL                i2c_wait_to_complete_timeout_flag;
static TASK_HNDL                    i2c_wait_to_complete_timeout_task;

#endif
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
        i2c_dev_ptr->success  = success;
        i2c_dev_ptr->active   = FALSE;
        
        if(i2c_dev_ptr->msg_complete != NULL)
        {
            i2c_dev_ptr->msg_complete(success);
        }
    }
}
//------------------------------------------------------------------------------------------------//
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
static void DrvI2cMasterDevice_WaitToCompleteTimeout(VPTR data_ptr)
{
    i2c_wait_to_complete_timeout_flag = TRUE;
    CoreTask_Stop(i2c_wait_to_complete_timeout_task);
}
#endif
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

#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
    i2c_wait_to_complete_timeout_flag = FALSE;
    i2c_wait_to_complete_timeout_task = CoreTask_RegisterTask(I2C_DEVICE_WAIT_TO_COMPLETE_TIMEOUT_TIME_IN_MS * 1000, DrvI2cMasterDevice_WaitToCompleteTimeout, NULL, 100, "I2c wait-to-complete timeout task");
#endif
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
		i2c_dev_hndl->success = FALSE;
		
        if(DrvI2cMasterChannel_Config(i2c_dev_hndl->i2c_channel_hndl, &(i2c_dev_hndl->config_struct)) &&
           DrvI2cMasterChannel_WriteData(i2c_dev_hndl->i2c_channel_hndl, i2c_dev_hndl->address, buffer_ptr, count))
        {
            if(wait_to_complete)
            {
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                i2c_wait_to_complete_timeout_flag = FALSE;
                CoreTask_Start(i2c_wait_to_complete_timeout_task);
#endif
                while(i2c_dev_hndl->active
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                      && (i2c_wait_to_complete_timeout_flag == FALSE)
#endif
                      )
                {}
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                if(i2c_wait_to_complete_timeout_flag)
                {
                    LOG_WRN("I2C device timed out - %d", PU8(device_id));
                    i2c_dev_hndl->active = FALSE;
                    return FALSE;
                }
                CoreTask_Stop(i2c_wait_to_complete_timeout_task);
#endif
                
                return i2c_dev_hndl->success;
            }
            return TRUE;
        }
        
        i2c_dev_hndl->active = FALSE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterDevice_WriteData_specificSlaveRegister(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete, U16 slave_reg_address)
{
    I2C_DEVICE_STRUCT*      i2c_dev_hndl = &i2c_device_struct[device_id];
    I2C_DEVICE_STRUCT*      i2c_dev_ptr;
    
    LOG_DEV("DrvI2cMasterDevice_WriteData - %d", PU8(device_id));
    
    data = (U8*)calloc(count + 2, sizeof(U8));
    *(data) = (slave_reg_address >> 8);
    *(data + 1) = (slave_reg_address & 0x0FF);
    for (int i = 0; i < count; i++)
    {
      *(data + 2 + i) = *(buffer_ptr + i);
    }
    
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
		i2c_dev_hndl->success = FALSE;
		
        if(DrvI2cMasterChannel_Config(i2c_dev_hndl->i2c_channel_hndl, &(i2c_dev_hndl->config_struct)) &&
           DrvI2cMasterChannel_WriteData_specificSlaveRegister(i2c_dev_hndl->i2c_channel_hndl, i2c_dev_hndl->address, data, count, (slave_reg_address<<8) | ( (slave_reg_address>>8) & 0x00FF)))
        {
            free(data);
            if(wait_to_complete)
            {
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                i2c_wait_to_complete_timeout_flag = FALSE;
                CoreTask_Start(i2c_wait_to_complete_timeout_task);
#endif
                while(i2c_dev_hndl->active
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                      && (i2c_wait_to_complete_timeout_flag == FALSE)
#endif
                      )
                {}
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                if(i2c_wait_to_complete_timeout_flag)
                {
                    LOG_WRN("I2C device timed out - %d", PU8(device_id));
                    i2c_dev_hndl->active = FALSE;
                    return FALSE;
                }
                CoreTask_Stop(i2c_wait_to_complete_timeout_task);
#endif
                
                return i2c_dev_hndl->success;
            }
            return TRUE;
        }
        free(data);
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
		i2c_dev_hndl->success = FALSE;
		
        if(DrvI2cMasterChannel_Config(i2c_dev_hndl->i2c_channel_hndl, &(i2c_dev_hndl->config_struct)) &&
           DrvI2cMasterChannel_ReadData(i2c_dev_hndl->i2c_channel_hndl, i2c_dev_hndl->address, buffer_ptr, count))
        {
            if(wait_to_complete)
            {
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                i2c_wait_to_complete_timeout_flag = FALSE;
                CoreTask_Start(i2c_wait_to_complete_timeout_task);
#endif
                while(i2c_dev_hndl->active
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                      && (i2c_wait_to_complete_timeout_flag == FALSE)
#endif
                    )
                {}
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                if(i2c_wait_to_complete_timeout_flag)
                {
                    LOG_WRN("I2C device timed out - %d", PU8(device_id));
                    i2c_dev_hndl->active = FALSE;
                    return FALSE;
                }
                CoreTask_Stop(i2c_wait_to_complete_timeout_task);
#endif
                
                return i2c_dev_hndl->success;
            }
            return TRUE;
        }
        
        i2c_dev_hndl->active = FALSE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvI2cMasterDevice_ReadData_specificSlaveRegister(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count, BOOL wait_to_complete, U16 slave_reg_address)
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
		i2c_dev_hndl->success = FALSE;
		
        if(DrvI2cMasterChannel_Config(i2c_dev_hndl->i2c_channel_hndl, &(i2c_dev_hndl->config_struct)) &&
           DrvI2cMasterChannel_ReadData_specificSlaveRegister(i2c_dev_hndl->i2c_channel_hndl, i2c_dev_hndl->address, buffer_ptr, count, (slave_reg_address<<8) | ( (slave_reg_address>>8) & 0x00FF)))
        {
            if(wait_to_complete)
            {
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                i2c_wait_to_complete_timeout_flag = FALSE;
                CoreTask_Start(i2c_wait_to_complete_timeout_task);
#endif
                while(i2c_dev_hndl->active
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                      && (i2c_wait_to_complete_timeout_flag == FALSE)
#endif
                    )
                {}
#if (I2C_DEVICE_USE_WAIT_TO_COMPLETE_TIMEOUT_TASK > 0)
                if(i2c_wait_to_complete_timeout_flag)
                {
                    LOG_WRN("I2C device timed out - %d", PU8(device_id));
                    i2c_dev_hndl->active = FALSE;
                    return FALSE;
                }
                CoreTask_Stop(i2c_wait_to_complete_timeout_task);
#endif
                
                return i2c_dev_hndl->success;
            }
            return TRUE;
        }
        
        i2c_dev_hndl->active = FALSE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void DrvI2cMasterDevice_ChangeAddress(I2C_DEVICE_ID device_id, U8 address)
{
    I2C_DEVICE_STRUCT*      i2c_dev_hndl = &i2c_device_struct[device_id];
    
    if(device_id < i2c_device_count)
    {
        i2c_dev_hndl->address = address;
    }
}
//================================================================================================//
