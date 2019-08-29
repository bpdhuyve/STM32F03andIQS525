//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file implements simple queue's that are configured at runtime
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COREQ_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COREQ_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COREQ_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of queues (default is 1)
#ifndef QUEUE_COUNT
    #define QUEUE_COUNT                 1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define VALIDATE_AND_GET_Q_PTR(q_hndl)      Q_CTRL_STRUCT*  q_ctrl_ptr = &coreq_ctrl_struct[(Q_HNDL)(q_hndl - 1)];\
                                            if((Q_HNDL)(q_hndl - 1) >= QUEUE_COUNT) {LOG_ERR("illegal hndl"); return FALSE;}
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U8* q_buffer_base_ptr;
    U8  q_item_size;
    U16 q_size_plus1;
    U16 q_write_index;
    U16 q_read_index;
    U16 q_hwm;
    U16 q_alloc;
    U16 q_toadd;
    #if (INCLUDE_INFO_STRING == 1)
        STRING      q_name;
    #endif
}
Q_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U16 QSpace(Q_CTRL_STRUCT* q_ctrl_ptr);
static U16 QCount(Q_CTRL_STRUCT* q_ctrl_ptr);
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreQInfo(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static Q_CTRL_STRUCT        coreq_ctrl_struct[QUEUE_COUNT];
static U8                   coreq_counter = 0;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static U16 QSpace(Q_CTRL_STRUCT* q_ctrl_ptr)
{
    return (q_ctrl_ptr->q_size_plus1 - 1 - QCount(q_ctrl_ptr));
}
//------------------------------------------------------------------------------------------------//
static U16 QCount(Q_CTRL_STRUCT* q_ctrl_ptr)
{
    U16 count = q_ctrl_ptr->q_write_index - q_ctrl_ptr->q_read_index;
    if(count > q_ctrl_ptr->q_size_plus1)
    {
        return count += q_ctrl_ptr->q_size_plus1;
    }
    return count;
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreQInfo(void)
{
    Q_CTRL_STRUCT*  q_ctrl_ptr;
    Q_HNDL          q_hndl;

    LOG_TRM("CoreQ");
    for(q_hndl = 1, q_ctrl_ptr = coreq_ctrl_struct; q_hndl <= coreq_counter; q_hndl++, q_ctrl_ptr++)
    {
        #if (INCLUDE_INFO_STRING == 1)
        {
            LOG_TRM("%2d - %4d/%-4d - HWM %4d [%db] - %s",
                    PU8(q_hndl),
                    PU16(QCount(q_ctrl_ptr)),
                    PU16(q_ctrl_ptr->q_size_plus1 - 1),
                    PU16(q_ctrl_ptr->q_hwm),
                    PU16(q_ctrl_ptr->q_item_size),
                    PCSTR(q_ctrl_ptr->q_name));
        }
        #else
        {
            LOG_TRM("%2d - %4d/%-4d - HWM %4d [%db]",
                    PU8(q_hndl),
                    PU16(QCount(q_ctrl_ptr)),
                    PU16(q_ctrl_ptr->q_size_plus1 - 1),
                    PU16(q_ctrl_ptr->q_hwm),
                    PU16(q_ctrl_ptr->q_item_size));
        }
        #endif
        CoreLog_Flush();
    }
    LOG_TRM("Use: %d/%d", PU8(coreq_counter), PU8(QUEUE_COUNT));
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CoreQ_Init(void)
{
    MODULE_INIT_ONCE();

    MEMSET((VPTR)coreq_ctrl_struct, 0, SIZEOF(coreq_ctrl_struct));
    coreq_counter = 0;

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
Q_HNDL CoreQ_Register(U16 q_size, U8 item_size, STRING name)
{
    Q_CTRL_STRUCT*  q_ctrl_ptr = &coreq_ctrl_struct[coreq_counter];

    if(coreq_counter >= QUEUE_COUNT)
    {
        LOG_ERR("Q limit reached - Cannot Register %s", PCSTR(name));
        return INVALID_Q_HNDL;
    }
    
    q_ctrl_ptr->q_buffer_base_ptr = CoreBuffer_CreateStaticU8((q_size + 1) * item_size, name);
    q_ctrl_ptr->q_item_size = item_size;
    q_ctrl_ptr->q_size_plus1 = q_size + 1;
    q_ctrl_ptr->q_write_index = 0;
    q_ctrl_ptr->q_read_index = 0;
    q_ctrl_ptr->q_hwm = q_size;
    q_ctrl_ptr->q_alloc = 0;
    q_ctrl_ptr->q_toadd = 0;
    #if (INCLUDE_INFO_STRING == 1)
    {
        q_ctrl_ptr->q_name = name;
    }
    #endif

    coreq_counter++;
    return coreq_counter;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_Write(Q_HNDL q_hndl, VPTR data_ptr, U16 count)
{
    U16     start_index;

    if(CoreQ_WriteAlloc(q_hndl, count, &start_index))
    {
        CoreQ_WritePart(q_hndl, start_index, data_ptr, count);
        return CoreQ_WriteDone(q_hndl, count);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_WriteAlloc(Q_HNDL q_hndl, U16 count, U16* start_index_ptr)
{
    BOOL    return_value = FALSE;
    
    VALIDATE_AND_GET_Q_PTR(q_hndl);

    Core_CriticalEnter();
    q_ctrl_ptr->q_alloc += count;                   // due to this, the write index cannot change anymore
    if(QSpace(q_ctrl_ptr) >= (q_ctrl_ptr->q_alloc + q_ctrl_ptr->q_toadd))   // check if enough space is available
    {
        return_value = TRUE;
        *start_index_ptr = q_ctrl_ptr->q_write_index + q_ctrl_ptr->q_alloc + q_ctrl_ptr->q_toadd - count;
    }
    else
    {
        q_ctrl_ptr->q_alloc -= count;
    }
    Core_CriticalExit();
    
    return return_value;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_WritePart(Q_HNDL q_hndl, U16 start_index, VPTR data_ptr, U16 part_count)
{
    U16     end_index;
    U16     part1;
    
    VALIDATE_AND_GET_Q_PTR(q_hndl);

    end_index = start_index + part_count;
    if((start_index < q_ctrl_ptr->q_size_plus1) && (end_index > q_ctrl_ptr->q_size_plus1))
    {
        part1 = q_ctrl_ptr->q_size_plus1 - start_index;
        MEMCPY((VPTR)&q_ctrl_ptr->q_buffer_base_ptr[start_index * q_ctrl_ptr->q_item_size],
               (VPTR)data_ptr,
               (part1 * q_ctrl_ptr->q_item_size));
        MEMCPY((VPTR)&q_ctrl_ptr->q_buffer_base_ptr[0],
               (VPTR)&((U8*)data_ptr)[part1 * q_ctrl_ptr->q_item_size],
               ((part_count - part1) * q_ctrl_ptr->q_item_size));
    }
    else
    {
        if(start_index >= q_ctrl_ptr->q_size_plus1)
        {
            start_index -= q_ctrl_ptr->q_size_plus1;
        }
        MEMCPY((VPTR)&q_ctrl_ptr->q_buffer_base_ptr[start_index * q_ctrl_ptr->q_item_size],
               (VPTR)data_ptr,
               (part_count * q_ctrl_ptr->q_item_size));
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_WriteDone(Q_HNDL q_hndl, U16 count)
{
    VALIDATE_AND_GET_Q_PTR(q_hndl);
    
    Core_CriticalEnter();
    if(q_ctrl_ptr->q_alloc < count)
    {
        Core_CriticalExit();
        return FALSE;
    }
    q_ctrl_ptr->q_toadd += count;
    q_ctrl_ptr->q_alloc -= count;
    if((q_ctrl_ptr->q_alloc == 0) && (q_ctrl_ptr->q_toadd > 0))
    {
        q_ctrl_ptr->q_write_index += q_ctrl_ptr->q_toadd;
        if(q_ctrl_ptr->q_write_index >= q_ctrl_ptr->q_size_plus1)
        {
            q_ctrl_ptr->q_write_index -= q_ctrl_ptr->q_size_plus1;
        }
        q_ctrl_ptr->q_toadd = 0;
        q_ctrl_ptr->q_hwm = MIN(q_ctrl_ptr->q_hwm, QSpace(q_ctrl_ptr));
    }
    Core_CriticalExit();
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_Read(Q_HNDL q_hndl, VPTR data_ptr, U16 count)
{
    if(CoreQ_Peek(q_hndl, data_ptr, count))
    {
        return CoreQ_Drop(q_hndl, count);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U16 CoreQ_ReadSmart(Q_HNDL q_hndl, VPTR data_ptr, U16 max_size)
{
    U16 items_to_get = MIN(CoreQ_GetCount(q_hndl), max_size);
    
    if(items_to_get > 0 && CoreQ_Read(q_hndl, data_ptr, items_to_get))
    {
        return items_to_get;
    }    
    return 0;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_Peek(Q_HNDL q_hndl, VPTR data_ptr, U16 count)
{
    U16     start_index;
    U16     end_index;
    U16     part1;

    VALIDATE_AND_GET_Q_PTR(q_hndl);
    
    if(QCount(q_ctrl_ptr) < count)
    {
        return FALSE;
    }
    
    start_index = q_ctrl_ptr->q_read_index;
    end_index = q_ctrl_ptr->q_read_index + count;
    if(end_index > q_ctrl_ptr->q_size_plus1)
    {
        part1 = q_ctrl_ptr->q_size_plus1 - start_index;
        MEMCPY((VPTR)data_ptr,
               (VPTR)&q_ctrl_ptr->q_buffer_base_ptr[start_index * q_ctrl_ptr->q_item_size],
               (part1 * q_ctrl_ptr->q_item_size));
        MEMCPY((VPTR)&((U8*)data_ptr)[part1 * q_ctrl_ptr->q_item_size],
               (VPTR)&q_ctrl_ptr->q_buffer_base_ptr[0],
               ((count - part1) * q_ctrl_ptr->q_item_size));
    }
    else
    {
        MEMCPY((VPTR)data_ptr,
               (VPTR)&q_ctrl_ptr->q_buffer_base_ptr[start_index * q_ctrl_ptr->q_item_size],
               (count * q_ctrl_ptr->q_item_size));
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_Drop(Q_HNDL q_hndl, U16 count)
{
    VALIDATE_AND_GET_Q_PTR(q_hndl);
    
    if(QCount(q_ctrl_ptr) < count)
    {
        return FALSE;
    }
    
    Core_CriticalEnter();
    q_ctrl_ptr->q_read_index = q_ctrl_ptr->q_read_index + count;
    if(q_ctrl_ptr->q_read_index >= q_ctrl_ptr->q_size_plus1)
    {
        q_ctrl_ptr->q_read_index -= q_ctrl_ptr->q_size_plus1;
    }
    Core_CriticalExit();
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreQ_DropAll(Q_HNDL q_hndl)
{
    VALIDATE_AND_GET_Q_PTR(q_hndl);
    
    Core_CriticalEnter();
    q_ctrl_ptr->q_read_index = q_ctrl_ptr->q_write_index;
    Core_CriticalExit();
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
U16 CoreQ_GetCount(Q_HNDL q_hndl)
{
    VALIDATE_AND_GET_Q_PTR(q_hndl);
    
    return QCount(q_ctrl_ptr);
}
//------------------------------------------------------------------------------------------------//
U16 CoreQ_GetSpace(Q_HNDL q_hndl)
{
    VALIDATE_AND_GET_Q_PTR(q_hndl);
    
    return QSpace(q_ctrl_ptr);
}
//------------------------------------------------------------------------------------------------//
void CoreQ_Info(void)
{
    CoreTerm_RegisterCommand("CoreQInfo", "CORE queue info", 0, Command_CoreQInfo, FALSE);
}
//================================================================================================//
