//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of M24512 I²C Eeprom
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MEM__DRVMEMM24512_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__DRVMEMM24512_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               MEM__DRVMEMM24512_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of M24512 eeprom handles
#ifndef DRVMEMM24512_COUNT
	#define DRVMEMM24512_COUNT              1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMM24512_PAGE_SIZE
    #define DRVMEMM24512_PAGE_SIZE          128
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMM24512_NUMBER_OF_PAGES
    #define DRVMEMM24512_NUMBER_OF_PAGES    512
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "mem\DrvMemM24512.h"
#include "i2c\DrvI2cMasterDevice.h"
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
    I2C_CHANNEL_HNDL    i2c_channel;
	I2C_DEVICE_ID	    i2c_device_id;
    U8                  i2c_address;
    U32                 address;
    U32                 length;
}
MEM_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemM24512_Clear(MEM_ID mem_id);
static BOOL DrvMemM24512_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemM24512_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemM24512_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemM24512_Flush(MEM_ID mem_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const MEM_HOOK_LIST          mem_hook_list = {DrvMemM24512_Clear, DrvMemM24512_Read, DrvMemM24512_Write, DrvMemM24512_Verify, DrvMemM24512_Flush};
static MEM_CTRL_STRUCT		        mem_ctrl_struct[DRVMEMM24512_COUNT];
static MEM_STRUCT                   mem_struct[DRVMEMM24512_COUNT];
static U8							mem_count;
static U8                           mem_buffer[DRVMEMM24512_PAGE_SIZE + 2];

static TASK_HNDL                    write_cycle_timeout_task;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DrvMemM24512_WriteCycleTimeoutTask(VPTR data_ptr)
{
    CoreTask_Stop(write_cycle_timeout_task);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemM24512_TryWrite(I2C_DEVICE_ID device_id, U8* buffer_ptr, U16 count)
{
    CoreTask_Start(write_cycle_timeout_task);
    
    while(CoreTask_IsTaskRunning(write_cycle_timeout_task))
    {
        if(DrvI2cMasterDevice_WriteData(device_id, buffer_ptr, count, TRUE))
        {
            CoreTask_Stop(write_cycle_timeout_task);
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemM24512_Clear(MEM_ID mem_id)
{
    MEM_CTRL_STRUCT* mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32 length  = mem_ctrl_struct_ptr->length;
    U32 address = mem_ctrl_struct_ptr->address;
    
    while(length > 0)
    {
        MEMSET(mem_buffer, 0xFF, DRVMEMM24512_PAGE_SIZE + 2);
        
        if(DrvMemM24512_Write(mem_id, address, mem_buffer, DRVMEMM24512_PAGE_SIZE) == FALSE)
        {
            return FALSE;
        }
        
        address += DRVMEMM24512_PAGE_SIZE;
        length  -= DRVMEMM24512_PAGE_SIZE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemM24512_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U8 address_data[2];
    
    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }
    
    address_data[0] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 8) & 0xFF);
    address_data[1] = (U8)((mem_ctrl_struct_ptr->address + address_offset) & 0xFF);
    
    if(DrvMemM24512_TryWrite(mem_ctrl_struct_ptr->i2c_device_id, address_data, 2))
    {
        return DrvI2cMasterDevice_ReadData(mem_ctrl_struct_ptr->i2c_device_id, data_ptr, length, TRUE);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemM24512_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32                 page_address = mem_ctrl_struct_ptr->address + address_offset;
    U16                 page_length;
    
    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }
    
    while(length > 0)
    {
        System_KickDog();
        
        // determine (remaining)
        page_length = DRVMEMM24512_PAGE_SIZE - (page_address & (DRVMEMM24512_PAGE_SIZE - 1));
        if(length < page_length)
        {
            page_length = length;
        }
        
        MEMCPY(&mem_buffer[2], data_ptr, page_length);
        mem_buffer[0] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 8) & 0xFF);
        mem_buffer[1] = (U8)((mem_ctrl_struct_ptr->address + address_offset) & 0xFF);
        
        if(DrvMemM24512_TryWrite(mem_ctrl_struct_ptr->i2c_device_id, mem_buffer, page_length + 2) == FALSE)
        {
            return FALSE;
        }
        
        length -= page_length;
        data_ptr += page_length;
        page_address += page_length;
        address_offset += page_length;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemM24512_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U8 address_data[2];
    U8 flash_byte;
    
    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }
    
    address_data[0] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 8) & 0xFF);
    address_data[1] = (U8)((mem_ctrl_struct_ptr->address + address_offset) & 0xFF);
    
    if(DrvMemM24512_TryWrite(mem_ctrl_struct_ptr->i2c_device_id, address_data, 2) == FALSE)
    {
        return FALSE;
    }
    
    while(length > 0)
    {
        if(DrvI2cMasterDevice_ReadData(mem_ctrl_struct_ptr->i2c_device_id, &flash_byte, 1, TRUE) == FALSE)
        {
            break;
        }
        if(flash_byte != *data_ptr)
        {
            break;
        }
        data_ptr++;
        length--;
    }
    
    return (BOOL)(length == 0);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemM24512_Flush(MEM_ID mem_id)
{
    // no flushing needed, system can write single bytes.
    return TRUE;
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvMemM24512_Init(void)
{
    MODULE_INIT_ONCE();
    
	mem_count = 0;
    
	MEMSET((VPTR)mem_ctrl_struct, 0, SIZEOF(mem_ctrl_struct));
	MEMSET((VPTR)mem_struct, 0, SIZEOF(mem_struct));
	MEMSET((VPTR)mem_buffer, 0, SIZEOF(mem_buffer));
    
    write_cycle_timeout_task = CoreTask_RegisterTask(5000, DrvMemM24512_WriteCycleTimeoutTask, NULL, 100, "Eeprom TO task");
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
MEM_HNDL DrvMemM24512_Register(I2C_CHANNEL_HNDL i2c_channel, U8 i2c_address, U32 flash_address, U16 page_count)
{
	MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr;
    MEM_HNDL            mem_hndl;
    U32                 length = page_count * DRVMEMM24512_PAGE_SIZE;
    I2C_DEVICE_ID       device_id;
    
    if((flash_address & (DRVMEMM24512_PAGE_SIZE - 1)) != 0)
    {
        LOG_WRN("Illegal flash address");
        return NULL;
    }
    if(((flash_address / DRVMEMM24512_PAGE_SIZE) + page_count) > DRVMEMM24512_NUMBER_OF_PAGES)
    {
        LOG_WRN("Illegal length");
        return NULL;
    }
    
    for(mem_hndl = mem_struct, mem_ctrl_struct_ptr = mem_ctrl_struct; mem_hndl < &mem_struct[mem_count]; mem_hndl++, mem_ctrl_struct_ptr++)
    {
        if((mem_ctrl_struct_ptr->i2c_channel == i2c_channel) && 
           (mem_ctrl_struct_ptr->i2c_address == i2c_address) && 
           (mem_ctrl_struct_ptr->length == length) &&
           (mem_ctrl_struct_ptr->address == flash_address))
        {
            return mem_hndl;
        }
        if((mem_ctrl_struct_ptr->i2c_address == i2c_address) &&
           !((flash_address + length <= mem_ctrl_struct_ptr->address) || (flash_address >= mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->length)))
        {
            LOG_WRN("Overlap");
            return NULL;
        }
    }
    
    if(mem_count < DRVMEMM24512_COUNT)
    {
        device_id = DrvI2cMasterDevice_Register(i2c_channel, i2c_address, 200000);
        
        if(device_id != INVALID_I2C_DEVICE_ID)
        {
            mem_ctrl_struct_ptr->i2c_device_id = device_id;
            mem_ctrl_struct_ptr->i2c_channel = i2c_channel;
            mem_ctrl_struct_ptr->i2c_address = i2c_address;
            mem_ctrl_struct_ptr->address = flash_address;
            mem_ctrl_struct_ptr->length = length;
            
            mem_hndl->hook_list_ptr = (MEM_HOOK_LIST*)&mem_hook_list;
            mem_hndl->mem_id = mem_count;
            mem_count++;
            
            return mem_hndl;
        }
    }
    return NULL;
}
//================================================================================================//
