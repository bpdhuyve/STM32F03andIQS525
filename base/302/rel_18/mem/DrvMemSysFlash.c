//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking SPI Master Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MEM__DRVMEMSYSFLASH_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__DRVMEMSYSFLASH_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MEM__DRVMEMSYSFLASH_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMSYSFLASH_COUNT
	#define DRVMEMSYSFLASH_COUNT		1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMSYSFLASH_IGNORE_FLASH_CLEAR_FAIL
	#define DRVMEMSYSFLASH_IGNORE_FLASH_CLEAR_FAIL		0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "mem\DrvMemSysFlash.h"

//SYS
#include "mem\SysFlash.h"
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
    U32 address;
    U32 length;
    U32 sector_size;
#if MIN_WRITE_LENGTH_IN_BYTES > 1
    U8  data_buffer[MIN_WRITE_LENGTH_IN_BYTES];
    U32 data_buffer_base;
    BOOL data_buffer_filled;
#endif
}
MEM_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysFlash_Clear(MEM_ID mem_id);
static BOOL DrvMemSysFlash_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemSysFlash_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemSysFlash_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemSysFlash_Flush(MEM_ID mem_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static MEM_HOOK_LIST                        mem_hook_list;
static MEM_STRUCT                           mem_struct[DRVMEMSYSFLASH_COUNT];
static MEM_CTRL_STRUCT                      mem_ctrl_struct[DRVMEMSYSFLASH_COUNT];
static U8                                   mem_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysFlash_Clear(MEM_ID mem_id)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32                 offset;

    for(offset = 0; offset < mem_ctrl_struct_ptr->length; offset += mem_ctrl_struct_ptr->sector_size)
    {
        System_KickDog();
        if(SysFlash_Clear(mem_ctrl_struct_ptr->address + offset) == FALSE)
        {
            #if DRVMEMSYSFLASH_IGNORE_FLASH_CLEAR_FAIL == 0
                return FALSE;
            #endif
        }
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysFlash_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }
    MEMCPY((VPTR)data_ptr, (VPTR)(mem_ctrl_struct_ptr->address + address_offset), length);
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysFlash_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
#if MIN_WRITE_LENGTH_IN_BYTES > 1
    U8                  offset;
    U16                 data_len;
#endif

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }

#if MIN_WRITE_LENGTH_IN_BYTES > 1
    // if data in buffer
    if(mem_ctrl_struct_ptr->data_buffer_filled)
    {
        // add new data if possible
        if((address_offset >= mem_ctrl_struct_ptr->data_buffer_base) &&
           (address_offset < mem_ctrl_struct_ptr->data_buffer_base + MIN_WRITE_LENGTH_IN_BYTES))
        {
            offset = address_offset - mem_ctrl_struct_ptr->data_buffer_base;
            data_len = MIN(length, MIN_WRITE_LENGTH_IN_BYTES - offset);

            MEMCPY((VPTR)&mem_ctrl_struct_ptr->data_buffer[offset], (VPTR)data_ptr, data_len);

            address_offset += data_len;
            data_ptr += data_len;
            length -= data_len;
        }
        // if still data to be added, write buffer
        if(length > 0)
        {
            SysFlash_Write(mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->data_buffer_base, mem_ctrl_struct_ptr->data_buffer, MIN_WRITE_LENGTH_IN_BYTES);
            // clean
            MEMSET((VPTR)mem_ctrl_struct_ptr->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
            mem_ctrl_struct_ptr->data_buffer_base = 0;
            mem_ctrl_struct_ptr->data_buffer_filled = FALSE;
        }
    }
    // if data to be writen and start address is not aligned, add to buffer
    if((length > 0) && (address_offset & (MIN_WRITE_LENGTH_IN_BYTES-1)))
    {
        offset = address_offset & (MIN_WRITE_LENGTH_IN_BYTES-1);
        data_len = MIN(length, MIN_WRITE_LENGTH_IN_BYTES - offset);

        mem_ctrl_struct_ptr->data_buffer_base = address_offset - offset;
        MEMCPY((VPTR)&mem_ctrl_struct_ptr->data_buffer[offset], (VPTR)data_ptr, data_len);
        mem_ctrl_struct_ptr->data_buffer_filled = TRUE;

        address_offset += data_len;
        data_ptr += data_len;
        length -= data_len;
        // if still data to be added, write buffer
        if(length > 0)
        {
            SysFlash_Write(mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->data_buffer_base, mem_ctrl_struct_ptr->data_buffer, MIN_WRITE_LENGTH_IN_BYTES);
            // clean
            MEMSET((VPTR)mem_ctrl_struct_ptr->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
            mem_ctrl_struct_ptr->data_buffer_base = 0;
            mem_ctrl_struct_ptr->data_buffer_filled = FALSE;
        }
    }
    // write bytes in multiples of miniam write length
    if((length / MIN_WRITE_LENGTH_IN_BYTES) > 0)
    {
        data_len = (length / MIN_WRITE_LENGTH_IN_BYTES) * MIN_WRITE_LENGTH_IN_BYTES;

        SysFlash_Write(mem_ctrl_struct_ptr->address + address_offset, data_ptr, data_len);

        address_offset += data_len;
        data_ptr += data_len;
        length -= data_len;
    }
    // if data remains, add to buffer
    if(length > 0)
    {
        mem_ctrl_struct_ptr->data_buffer_base = address_offset;
        MEMCPY((VPTR)&mem_ctrl_struct_ptr->data_buffer[0], (VPTR)data_ptr, length);
        mem_ctrl_struct_ptr->data_buffer_filled = TRUE;
    }

    return TRUE;
#else
    return SysFlash_Write(mem_ctrl_struct_ptr->address + address_offset, data_ptr, length);
#endif
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysFlash_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U8*                 flash_ptr = (U8*)(mem_ctrl_struct_ptr->address + address_offset);

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }
    while(length--)
    {
        if(*flash_ptr != *data_ptr)
        {
            return FALSE;
        }
        flash_ptr++;
        data_ptr++;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysFlash_Flush(MEM_ID mem_id)
{
#if MIN_WRITE_LENGTH_IN_BYTES > 1
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];

    if(mem_ctrl_struct_ptr->data_buffer_filled)
    {
        SysFlash_Write(mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->data_buffer_base, mem_ctrl_struct_ptr->data_buffer, MIN_WRITE_LENGTH_IN_BYTES);
        
        MEMSET((VPTR)mem_ctrl_struct_ptr->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
        mem_ctrl_struct_ptr->data_buffer_base = 0;
        mem_ctrl_struct_ptr->data_buffer_filled = FALSE;
    }
#endif
    return TRUE;
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvMemSysFlash_Init(void)
{
    MODULE_INIT_ONCE();
    
    SysFlash_Init();
    
    mem_hook_list.mem_clear_hook = DrvMemSysFlash_Clear;
    mem_hook_list.mem_read_hook = DrvMemSysFlash_Read;
    mem_hook_list.mem_write_hook = DrvMemSysFlash_Write;
    mem_hook_list.mem_verify_hook = DrvMemSysFlash_Verify;
    mem_hook_list.mem_flush_hook = DrvMemSysFlash_Flush;

    MEMSET((VPTR)mem_struct, 0, SIZEOF(mem_struct));
    MEMSET((VPTR)mem_ctrl_struct, 0, SIZEOF(mem_ctrl_struct));
    mem_count = 0;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
MEM_HNDL DrvMemSysFlash_Register(U32 address, U32 length, U32 sector_size)
{
    MEM_HNDL            mem_hndl;
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr;
    
    MODULE_CHECK();
    
    for(mem_hndl = mem_struct, mem_ctrl_struct_ptr = mem_ctrl_struct; mem_hndl < &mem_struct[mem_count]; mem_hndl++, mem_ctrl_struct_ptr++)
    {
        if((mem_ctrl_struct_ptr->address == address) && (mem_ctrl_struct_ptr->length == length))
        {
            return mem_hndl;
        }
        if(!((address + length <= mem_ctrl_struct_ptr->address) || (address >= mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->length)))
        {
            LOG_WRN("Overlap");
            return NULL;
        }
    }
    
    if(mem_count < DRVMEMSYSFLASH_COUNT)
    {
        mem_ctrl_struct_ptr->address = address;
        mem_ctrl_struct_ptr->length = length;
        mem_ctrl_struct_ptr->sector_size = sector_size;
        #if MIN_WRITE_LENGTH_IN_BYTES > 1
            MEMSET((VPTR)mem_ctrl_struct_ptr->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
            mem_ctrl_struct_ptr->data_buffer_base = 0;
            mem_ctrl_struct_ptr->data_buffer_filled = FALSE;
        #endif
        mem_hndl->hook_list_ptr = &mem_hook_list;
        mem_hndl->mem_id = mem_count;
        mem_count++;
        return mem_hndl;
    }
    return NULL;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvMemSysFlash_Update(MEM_HNDL mem_hndl, U32 address, U32 length)
{
    MEM_HNDL            mem_hndl_temp;
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr;
    
    MODULE_CHECK();
    
    if(mem_hndl->hook_list_ptr != &mem_hook_list)
    {
        return FALSE;
    }
    
    for(mem_hndl_temp = mem_struct, mem_ctrl_struct_ptr = mem_ctrl_struct; mem_hndl_temp < &mem_struct[mem_count]; mem_hndl_temp++, mem_ctrl_struct_ptr++)
    {
        if(mem_hndl_temp != mem_hndl)
        {
            if(!((address + length <= mem_ctrl_struct_ptr->address) || (address >= mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->length)))
            {
                LOG_WRN("Overlap");
                return FALSE;
            }
        }
    }
    
    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_hndl->mem_id];
    
    mem_ctrl_struct_ptr->address = address;
    mem_ctrl_struct_ptr->length  = length;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvMemSysFlash_GetInfo(MEM_HNDL mem_hndl, U32* address_ptr, U32* length_ptr)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_hndl->mem_id];
    
    MODULE_CHECK();
    
    if(mem_hndl->hook_list_ptr != &mem_hook_list)
    {
        return FALSE;
    }
    
    *address_ptr = mem_ctrl_struct_ptr->address;
    *length_ptr = mem_ctrl_struct_ptr->length;
    return TRUE;
}
//================================================================================================//
