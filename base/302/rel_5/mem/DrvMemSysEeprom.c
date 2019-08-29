//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the system EEPROM driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MEM__DRVMEMSYSEEPROM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__DRVMEMSYSEEPROM_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MEM__DRVMEMSYSEEPROM_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef DRVMEMSYSEEPROM_COUNT
	#define DRVMEMSYSEEPROM_COUNT		1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMSYSEEPROM_IGNORE_FLASH_CLEAR_FAIL
	#define DRVMEMSYSEEPROM_IGNORE_FLASH_CLEAR_FAIL		0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "mem\DrvMemSysEeprom.h"

//SYS
#include "mem\SysEeprom.h"
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
    U16 sector_size;
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
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_EepromClear(void);
static void Command_EepromRead(void);
static void Command_EepromWrite(void);
#endif

static BOOL DrvMemSysEeprom_Clear(MEM_ID mem_id);
static BOOL DrvMemSysEeprom_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemSysEeprom_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemSysEeprom_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemSysEeprom_Flush(MEM_ID mem_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static MEM_HOOK_LIST                        mem_hook_list;
static MEM_STRUCT                           mem_struct[DRVMEMSYSEEPROM_COUNT];
static MEM_CTRL_STRUCT                      mem_ctrl_struct[DRVMEMSYSEEPROM_COUNT];
static U8                                   mem_count;

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static U8                                   test_buffer[1024];
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_EepromClear(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);

    if(mem_id >= mem_count)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    CoreTerm_PrintFeedback(DrvMemSysEeprom_Clear(mem_id));
}
//------------------------------------------------------------------------------------------------//
static void Command_EepromRead(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);
    U16             length = CoreTerm_GetArgumentAsU32(2);
    U16             part_len;
    U8*             data_ptr;

    if((mem_id >= mem_count) || (length > 1024))
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    if(DrvMemSysEeprom_Read(mem_id, CoreTerm_GetArgumentAsU32(1), test_buffer, length))
    {
        data_ptr = test_buffer;
        while(length > 0)
        {
            part_len = MIN(32, length);
            LOG_TRM("%02x", PU8A(data_ptr, part_len));
            length -= part_len;
            data_ptr += part_len;
            CoreLog_Flush();
        }
        CoreTerm_PrintAcknowledge();
        return;
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void Command_EepromWrite(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);
    U16             length = CoreTerm_GetArgumentAsU32(2);
    U8              byte = CoreTerm_GetArgumentAsU32(3);

    if((mem_id >= mem_count) || (length > 1024))
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    MEMSET((VPTR)test_buffer, byte, length);
    
    CoreTerm_PrintFeedback(DrvMemSysEeprom_Write(mem_id, CoreTerm_GetArgumentAsU32(1), test_buffer, length));
}
#endif
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysEeprom_Clear(MEM_ID mem_id)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32                 offset;
    
    for(offset = 0; offset < mem_ctrl_struct_ptr->length; offset += mem_ctrl_struct_ptr->sector_size)
    {
        System_KickDog();
        if(SysEeprom_Clear(mem_ctrl_struct_ptr->address + offset) == FALSE)
        {
            #if DRVMEMSYSEEPROM_IGNORE_FLASH_CLEAR_FAIL == 0
                return FALSE;                
            #endif
        }
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysEeprom_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
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
static BOOL DrvMemSysEeprom_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
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
            SysEeprom_Write(mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->data_buffer_base, mem_ctrl_struct_ptr->data_buffer, MIN_WRITE_LENGTH_IN_BYTES);
            // clean
            MEMSET((VPTR)mem_ctrl_struct->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
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
            SysEeprom_Write(mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->data_buffer_base, mem_ctrl_struct_ptr->data_buffer, MIN_WRITE_LENGTH_IN_BYTES);
            // clean
            MEMSET((VPTR)mem_ctrl_struct->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
            mem_ctrl_struct_ptr->data_buffer_base = 0;
            mem_ctrl_struct_ptr->data_buffer_filled = FALSE;
        }
    }
    // write bytes in multiples of miniam write length
    if((length / MIN_WRITE_LENGTH_IN_BYTES) > 0)
    {
        data_len = (length / MIN_WRITE_LENGTH_IN_BYTES) * MIN_WRITE_LENGTH_IN_BYTES;
        
        SysEeprom_Write(mem_ctrl_struct_ptr->address + address_offset, data_ptr, data_len);
        
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
    return SysEeprom_Write(mem_ctrl_struct_ptr->address + address_offset, data_ptr, length);
#endif
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSysEeprom_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
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
static BOOL DrvMemSysEeprom_Flush(MEM_ID mem_id)
{
#if MIN_WRITE_LENGTH_IN_BYTES > 1
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    
    if(mem_ctrl_struct_ptr->data_buffer_filled)
    {
        SysEeprom_Write(mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->data_buffer_base, mem_ctrl_struct_ptr->data_buffer, MIN_WRITE_LENGTH_IN_BYTES);
        
        MEMSET((VPTR)mem_ctrl_struct->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
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
void DrvMemSysEeprom_Init(void)
{
    SysEeprom_Init();

    mem_hook_list.mem_clear_hook = DrvMemSysEeprom_Clear;
    mem_hook_list.mem_read_hook = DrvMemSysEeprom_Read;
    mem_hook_list.mem_write_hook = DrvMemSysEeprom_Write;
    mem_hook_list.mem_verify_hook = DrvMemSysEeprom_Verify;
    mem_hook_list.mem_flush_hook = DrvMemSysEeprom_Flush;

    MEMSET((VPTR)mem_struct, 0, SIZEOF(mem_struct));
    MEMSET((VPTR)mem_ctrl_struct, 0, SIZEOF(mem_ctrl_struct));
    mem_count = 0;
    
    CoreTerm_RegisterCommand("EepromClear", "EEPROM Clear (a: mem_id)", 1, Command_EepromClear, TRUE);
    CoreTerm_RegisterCommand("EepromRead", "EEPROM Read (a: mem_id, b:addr_offset, c:length)", 3, Command_EepromRead, TRUE);
    CoreTerm_RegisterCommand("EepromWrite", "EEPROM Write (a: mem_id, b:addr_offset, c:length, d:data_byte)", 4, Command_EepromWrite, TRUE);
}
//------------------------------------------------------------------------------------------------//
MEM_HNDL DrvMemSysEeprom_Register(U32 address, U32 length, U16 sector_size)
{
    MEM_HNDL            mem_hndl;
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr;
    
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
    
    if(mem_count < DRVMEMSYSEEPROM_COUNT)
    {
        mem_ctrl_struct_ptr->address = address;
        mem_ctrl_struct_ptr->length = length;
        mem_ctrl_struct_ptr->sector_size = sector_size;
#if MIN_WRITE_LENGTH_IN_BYTES > 1
        MEMSET((VPTR)mem_ctrl_struct->data_buffer, 0xFF, MIN_WRITE_LENGTH_IN_BYTES);
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
BOOL DrvMemSysEeprom_GetInfo(MEM_HNDL mem_hndl, U32* address_ptr, U32* length_ptr)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_hndl->mem_id];
    
    if(mem_hndl->hook_list_ptr != &mem_hook_list)
    {
        return FALSE;
    }
    
    *address_ptr = mem_ctrl_struct_ptr->address;
    *length_ptr = mem_ctrl_struct_ptr->length;
    return TRUE;
}
//================================================================================================//
