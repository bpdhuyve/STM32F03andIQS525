//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of test library for memories
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBMEM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBMEM_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBMEM_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef TESTLIBMEM_DEFAULT_BUFFER_SIZE
    #define TESTLIBMEM_DEFAULT_BUFFER_SIZE      256
#endif
//------------------------------------------------------------------------------------------------//
#ifndef TESTLIBMEM_MEM_COUNT
    #define TESTLIBMEM_MEM_COUNT                3
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "mem\DrvMem.h"

// TEST LIB
#include "TestLibMem.h"
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
    MEM_HNDL    mem_hndl;
    STRING      info_string;
}
TESTLIBMEM_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static TESTLIBMEM_STRUCT        mem_hndl_struct[TESTLIBMEM_MEM_COUNT];
static U8                       mem_hndl_count;
static U8                       mem_buffer[TESTLIBMEM_DEFAULT_BUFFER_SIZE];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_MemRead(void)
{
    TESTLIBMEM_STRUCT* struct_ptr;
    U8* mem_buffer_ptr;
    U32 address;
    U16 length;
    U8  temp;
    
    if(CoreTerm_GetArgumentAsU32(0) > TESTLIBMEM_MEM_COUNT)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    struct_ptr = &mem_hndl_struct[CoreTerm_GetArgumentAsU32(0)];
    address    = CoreTerm_GetArgumentAsU32(1);
    length     = MIN(CoreTerm_GetArgumentAsU32(2), TESTLIBMEM_DEFAULT_BUFFER_SIZE);
    MEMSET(mem_buffer, 0, TESTLIBMEM_DEFAULT_BUFFER_SIZE);
    
    if(DrvMem_ReadData(struct_ptr->mem_hndl, address, mem_buffer, length))
    {
        LOG_TRM("%s: address 0x%08h, length %d", PCSTR(struct_ptr->info_string), PU32(address), PU16(length));
        mem_buffer_ptr = mem_buffer;
        while(length > 0)
        {
            temp = MIN(length, 16);
            LOG_TRM("%02h", PU8A(mem_buffer_ptr, temp));
            mem_buffer_ptr += temp;
            length -= temp;
            CoreLog_Flush();
        }
        CoreTerm_PrintAcknowledge();
        return;
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void Command_MemWrite(void)
{
    MEM_HNDL mem_hndl;
    U32 address;
    U16 length;
    U16 i;
    
    if(CoreTerm_GetArgumentAsU32(0) > TESTLIBMEM_MEM_COUNT)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    mem_hndl = mem_hndl_struct[CoreTerm_GetArgumentAsU32(0)].mem_hndl;
    address  = CoreTerm_GetArgumentAsU32(1);
    length   = MIN(CoreTerm_GetArgumentAsU32(2), TESTLIBMEM_DEFAULT_BUFFER_SIZE);
    
    for(i = 0; i < TESTLIBMEM_DEFAULT_BUFFER_SIZE; i++)
    {
        mem_buffer[i] = i % 256;
    }
    
    CoreTerm_PrintFeedback(DrvMem_WriteData(mem_hndl, address, mem_buffer, length));
}
//------------------------------------------------------------------------------------------------//
static void Command_MemClear(void)
{
    MEM_HNDL mem_hndl;
    
    if(CoreTerm_GetArgumentAsU32(0) > TESTLIBMEM_MEM_COUNT)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    mem_hndl = mem_hndl_struct[CoreTerm_GetArgumentAsU32(0)].mem_hndl;
    
    CoreTerm_PrintFeedback(DrvMem_Clear(mem_hndl));
}
//------------------------------------------------------------------------------------------------//
static void Command_MemFlush(void)
{
    MEM_HNDL mem_hndl;
    
    if(CoreTerm_GetArgumentAsU32(0) > TESTLIBMEM_MEM_COUNT)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    mem_hndl = mem_hndl_struct[CoreTerm_GetArgumentAsU32(0)].mem_hndl;
    
    CoreTerm_PrintFeedback(DrvMem_Flush(mem_hndl));
}
//------------------------------------------------------------------------------------------------//
static void Command_MemInfo(void)
{
    TESTLIBMEM_STRUCT* struct_ptr = mem_hndl_struct;
    U8 i;
    
    LOG_TRM("Test lib mem struct:");
    for(i = 0; i < mem_hndl_count; i++, struct_ptr++)
    {
        LOG_TRM("%d: %s", PU8(i), PCSTR(struct_ptr->info_string));
    }
    
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibMem_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET(mem_hndl_struct, 0, TESTLIBMEM_MEM_COUNT);
    MEMSET(mem_buffer, 0, TESTLIBMEM_DEFAULT_BUFFER_SIZE);
    mem_hndl_count = 0;
    
    CoreTerm_RegisterCommand("MemRead",  "Mem read (index, address, length)",  3, Command_MemRead, TRUE);
    CoreTerm_RegisterCommand("MemWrite", "Mem write (index, address, length)", 3, Command_MemWrite, TRUE);
    CoreTerm_RegisterCommand("MemClear", "Mem clear (index)", 1, Command_MemClear, TRUE);
    CoreTerm_RegisterCommand("MemFlush", "Mem flush (index)", 1, Command_MemFlush, TRUE);
    CoreTerm_RegisterCommand("MemInfo",  "Mem info", 0, Command_MemInfo, TRUE);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
BOOL TestLibMem_Register(MEM_HNDL mem_hndl, const STRING info)
{
    TESTLIBMEM_STRUCT* struct_ptr;
    
    if((mem_hndl == NULL) || (info == NULL))
    {
        return FALSE;
    }
    
    for(struct_ptr = mem_hndl_struct; struct_ptr < &mem_hndl_struct[mem_hndl_count]; struct_ptr++)
    {
        if((struct_ptr->mem_hndl->mem_id == mem_hndl->mem_id) && 
           (struct_ptr->mem_hndl->hook_list_ptr == mem_hndl->hook_list_ptr))
        {
            CoreTerm_PrintFailed();
            return FALSE;
        }
    }
    
    if(mem_hndl_count < TESTLIBMEM_MEM_COUNT)
    {
        mem_hndl_struct[mem_hndl_count].mem_hndl    = mem_hndl;
        mem_hndl_struct[mem_hndl_count].info_string = info;
        mem_hndl_count++;
        return TRUE;
    }
    LOG_TRM("TESTLIBMEM_MEM_COUNT too small");
    return FALSE;
}
//================================================================================================//
