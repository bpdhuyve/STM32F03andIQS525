//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module that manages a heap
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COREHEAP_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COREHEAP_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_ALL//DEFAULT
#else
	#define CORELOG_LEVEL               COREHEAP_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the total space used for the heap
#ifndef HEAP_SIZE
    #define HEAP_SIZE                   0x10000
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if a warning (0) or an error (1) has to be thrown when a fault occurs
// possible faults are:
// - not able to allocate
// - freeing item out of bounds
// - freeing already free item
#ifndef HANDLE_FAULT_AS_ERROR
    #define HANDLE_FAULT_AS_ERROR       0
#endif
//------------------------------------------------------------------------------------------------//
#ifndef INCLUDE_TRACE
    #define INCLUDE_TRACE               0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// CORE
#include "CoreHeap.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SIGNATURE_HEAP              0x1CEB00DA

#if (HANDLE_FAULT_AS_ERROR == 1)
    #define HANDLE_FAULT(...)       LOG_ERR(##__VA_ARGS__)
#else
    #define HANDLE_FAULT(...)       LOG_WRN(##__VA_ARGS__)
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U32     signature;
    VPTR    prev_item_ptr;
#if INCLUDE_TRACE != 0
    U32     timestamp;
    U32     requestor;
#endif
    U32     item_size   : 31;
    U32     is_taken    : 1;
}
HEAP_INFO;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Heap_CheckIfValid(HEAP_INFO* item_ptr);
#if INCLUDE_TRACE != 0
static void CoreHeap_1msTask(VPTR data_ptr);
#endif

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_HeapInfo(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

#pragma data_alignment=32
static U8               heap_buffer[HEAP_SIZE];

static U16              part_count_now;
static U16              part_count_hwm;

static U32              bytes_taken_now;
static U32              bytes_taken_hwm;

static BOOL             is_init = FALSE;

#if INCLUDE_TRACE != 0
static U32              ms_count;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL Heap_CheckIfValid(HEAP_INFO* item_ptr)
{
    // check if in heap buffer
    if(((U8*)item_ptr < heap_buffer) || ((U8*)item_ptr >= &heap_buffer[HEAP_SIZE]))
    {
        return FALSE;
    }
    
    // check signature
    if(item_ptr->signature != SIGNATURE_HEAP)
    {
        LOG_ERR("HEAP SIGNATURE");
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
#if INCLUDE_TRACE != 0
static void CoreHeap_1msTask(VPTR data_ptr)
{
    ms_count++;
}
#endif
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_HeapInfo(void)
{
    HEAP_INFO*  item_ptr;
    U32         largest_free = 0;
    
    item_ptr = (HEAP_INFO*)heap_buffer;
    
    while(((U8*)item_ptr >= heap_buffer) && ((U8*)item_ptr < &heap_buffer[HEAP_SIZE]))
    {
#if INCLUDE_TRACE != 0
        if(item_ptr->is_taken)
        {
            LOG_TRM("%02x %5d %08x %10d", PU32(item_ptr), PU32(item_ptr->item_size), PU32(item_ptr->requestor), PU32(item_ptr->timestamp));
        }
        else
        {
            LOG_TRM("%02x %5d", PU32(item_ptr), PU32(item_ptr->item_size));
        }
#endif
        if((item_ptr->is_taken == 0) && (item_ptr->item_size > largest_free))
        {
            largest_free = item_ptr->item_size;
        }
        item_ptr = (HEAP_INFO*)((U8*)item_ptr + item_ptr->item_size);
#if INCLUDE_TRACE != 0
        CoreLog_Flush();
#endif
    }
    
    LOG_TRM("HEAP INFO:");
    LOG_TRM(" - total           %5d bytes", PU32(HEAP_SIZE));
    LOG_TRM(" - taken now       %5d bytes", PU32(bytes_taken_now));
    LOG_TRM("         hwm       %5d bytes", PU32(bytes_taken_hwm));
    LOG_TRM(" - parts now       %5d parts", PU32(part_count_now));
    LOG_TRM("         hwm       %5d parts", PU32(part_count_hwm));
    LOG_TRM(" - free  now       %5d bytes", PU32(HEAP_SIZE - bytes_taken_now));
    LOG_TRM("         largest   %5d bytes", PU32(largest_free));
    LOG_TRM(" - fragmentation   %5d %%", PU8(((F32)1 - ((F32)largest_free / (F32)(HEAP_SIZE - bytes_taken_now)))*100));
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CoreHeap_Init(void)
{
    HEAP_INFO*  first_item_ptr = (HEAP_INFO*)heap_buffer;
    
    MODULE_INIT_ONCE();
    
    first_item_ptr->signature       = SIGNATURE_HEAP;
    first_item_ptr->prev_item_ptr   = NULL;
    first_item_ptr->item_size       = HEAP_SIZE;
    first_item_ptr->is_taken        = 0;
    
    part_count_now = 1;
    part_count_hwm = 1;
    bytes_taken_now = 0;
    bytes_taken_hwm = 0;
    
    CoreTerm_RegisterCommand("HeapInfo", "HEAP info", 0, Command_HeapInfo, TRUE);
    
#if INCLUDE_TRACE != 0
    ms_count = 0;
    CoreTask_Start(CoreTask_RegisterTask(1000, CoreHeap_1msTask, NULL, 127, "HEAP 1MS"));
#endif

    is_init = TRUE;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
VPTR CoreHeap_Alloc(U32 request_size)
{
    HEAP_INFO*  item_ptr;
    HEAP_INFO*  second_best_item_ptr;
    HEAP_INFO*  new_item_ptr;
    HEAP_INFO*  next_item_ptr;
#if INCLUDE_TRACE != 0
    U32         dummy_u32;
#endif
    
    // if not inited, call init self
    if(is_init == FALSE)
    {
        CoreHeap_Init();
    }
    
#if INCLUDE_TRACE != 0
    dummy_u32 = 1;
#endif
    
    // add overhead bytesize and round up to multiple of 4 bytes
    request_size += SIZEOF(HEAP_INFO) + 3;
    request_size &= ~((U32)0x00000003);
    
    // scan all existing items
    item_ptr = (HEAP_INFO*)heap_buffer;
    second_best_item_ptr = NULL;
    while(Heap_CheckIfValid(item_ptr))
    {
        // check if item is free
        if(item_ptr->is_taken == 0)
        {
            // check if item is of requested size: claim
            if(item_ptr->item_size == request_size)
            {
                // mark as taken
#if INCLUDE_TRACE != 0
                item_ptr->requestor = (U32)(*(&dummy_u32 + 12));
                item_ptr->timestamp = ms_count;
#endif
                item_ptr->is_taken = 1;
                
                // do admin
                bytes_taken_now += item_ptr->item_size;
                if(bytes_taken_now > bytes_taken_hwm) {bytes_taken_hwm = bytes_taken_now;}
                
                // return pointer
                return ((U8*)item_ptr + SIZEOF(HEAP_INFO));
            }
            // otherwise check if better fitted than previous second best item
            else if((item_ptr->item_size > request_size) &&
                    ((second_best_item_ptr == NULL) || (item_ptr->item_size < second_best_item_ptr->item_size)))
            {
                second_best_item_ptr = item_ptr;
            }
        }
        
        // jump to next
        item_ptr = (HEAP_INFO*)((U8*)item_ptr + item_ptr->item_size);
    }
    
    // if no item of requested size was found: try to take second best item
    item_ptr = second_best_item_ptr;
    if(Heap_CheckIfValid(item_ptr))
    {
        // check if large enough to split (at least 4 bytes to alloc)
        if(item_ptr->item_size >= (request_size + (SIZEOF(HEAP_INFO) + 4)))
        {
            // create new
            new_item_ptr = (HEAP_INFO*)((U8*)item_ptr + request_size);
            
            new_item_ptr->signature     = SIGNATURE_HEAP;
            new_item_ptr->prev_item_ptr = (VPTR)item_ptr;
            new_item_ptr->item_size     = item_ptr->item_size - request_size;
            new_item_ptr->is_taken      = 0;
            
            // update item
            item_ptr->item_size         = request_size;
            
            // update next
            next_item_ptr = (HEAP_INFO*)((U8*)new_item_ptr + new_item_ptr->item_size);
            if(Heap_CheckIfValid(next_item_ptr))
            {
                next_item_ptr->prev_item_ptr = (VPTR)new_item_ptr;
            }
            
            // do admin
            if(++part_count_now > part_count_hwm)   {part_count_hwm = part_count_now;}
        }
        
        // mark as taken
#if INCLUDE_TRACE != 0
        item_ptr->requestor = (U32)(*(&dummy_u32 + 12));
        item_ptr->timestamp = ms_count;
#endif
        item_ptr->is_taken = 1;
        
        // do admin
        bytes_taken_now += item_ptr->item_size;
        if(bytes_taken_now > bytes_taken_hwm) {bytes_taken_hwm = bytes_taken_now;}
        
        // return pointer
        return ((U8*)item_ptr + SIZEOF(HEAP_INFO));
    }
    
    // failed to alloc
#if INCLUDE_TRACE != 0
    HANDLE_FAULT("Req 0x%08x, Size %d, Alloc failed", PU32(*(&dummy_u32 + 12)), PU32(request_size));
#else
    HANDLE_FAULT("Alloc failed, Size %d", PU32(request_size));
#endif
    return NULL;
}
//------------------------------------------------------------------------------------------------//
void CoreHeap_Free(VPTR data_ptr)
{
    HEAP_INFO*  item_ptr;
    HEAP_INFO*  next_item_ptr;
    HEAP_INFO*  prev_item_ptr;
    
    // take item
    item_ptr = (HEAP_INFO*)((U8*)data_ptr - SIZEOF(HEAP_INFO));
    
    // check protect
    if(Heap_CheckIfValid(item_ptr) == FALSE)
    {
        HANDLE_FAULT("Item 0x%08x OOB", PU32(data_ptr));
        return;
    }
    // check if still taken
    if(item_ptr->is_taken == 0)
    {
        HANDLE_FAULT("Item 0x%08x already free", PU32(data_ptr));
        return;
    }
    
    // mark as free
    item_ptr->is_taken = 0;
    // do admin
    bytes_taken_now -= item_ptr->item_size;
    
    // try to merge with next
    next_item_ptr = (HEAP_INFO*)((U8*)item_ptr + item_ptr->item_size);
    if(Heap_CheckIfValid(next_item_ptr) && (next_item_ptr->is_taken == 0))
    {
        // add next size to this
        item_ptr->item_size += next_item_ptr->item_size;
        
        // update new next
        next_item_ptr = (HEAP_INFO*)((U8*)item_ptr + item_ptr->item_size);
        if(Heap_CheckIfValid(next_item_ptr))
        {
            next_item_ptr->prev_item_ptr = (VPTR)item_ptr;
        }
        
        // do admin
        part_count_now--;
    }
    
    // try to merge with previous
    prev_item_ptr = (HEAP_INFO*)item_ptr->prev_item_ptr;
    if(Heap_CheckIfValid(prev_item_ptr) && (prev_item_ptr->is_taken == 0))
    {
        // add item size to prev
        prev_item_ptr->item_size += item_ptr->item_size;
        
        // update new next
        next_item_ptr = (HEAP_INFO*)((U8*)prev_item_ptr + prev_item_ptr->item_size);
        if(Heap_CheckIfValid(next_item_ptr))
        {
            next_item_ptr->prev_item_ptr = (VPTR)prev_item_ptr;
        }
        
        // do admin
        part_count_now--;
    }
}
//================================================================================================//
