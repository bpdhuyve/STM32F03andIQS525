//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the memory interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MEM__DRVMEM_H
#define MEM__DRVMEM_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          MEM_ID;

typedef BOOL (*MEM_CLEARFLUSH_HOOK)(MEM_ID mem_id);

typedef BOOL (*MEM_ACTION_HOOK)(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);

typedef struct
{
    MEM_CLEARFLUSH_HOOK             mem_clear_hook;
    MEM_ACTION_HOOK                 mem_read_hook;
    MEM_ACTION_HOOK                 mem_write_hook;
    MEM_ACTION_HOOK                 mem_verify_hook;
    MEM_CLEARFLUSH_HOOK             mem_flush_hook;
}
MEM_HOOK_LIST;

typedef struct
{
    MEM_HOOK_LIST*	                hook_list_ptr;
    MEM_ID                          mem_id;
}
MEM_STRUCT;

typedef MEM_STRUCT*                 MEM_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/// @brief  module init, do this before using the module
void DrvMem_Init(void);

/// @brief  clears the entire memory
/// @param  "mem_hndl"       memory handle, created in the specific memory implementation module
BOOL DrvMem_Clear(MEM_HNDL mem_hndl);

/// @param  "mem_hndl"       memory handle, created in the specific memory implementation module
/// @param  "data_ptr"       data will be copied to the address contained in this pointer
/// @param  "length"         specifies how many bytes this function will read
/// @return returns true if action was successful 
BOOL DrvMem_ReadData(MEM_HNDL mem_hndl, U32 address_offset, U8* data_ptr, U16 length);

/// @param  "mem_hndl"       memory handle, created in the specific memory implementation module
/// @return returns true if action was successful 
BOOL DrvMem_WriteData(MEM_HNDL mem_hndl, U32 address_offset, U8* data_ptr, U16 length);

/// @brief  checks of the data in the flash matches with other provided data
/// @param  "mem_hndl"       memory handle, created in the specific memory implementation module
/// @return returns true if the data matches
BOOL DrvMem_VerifyData(MEM_HNDL mem_hndl, U32 address_offset, U8* data_ptr, U16 length);

/// @brief  use this flush after using DrvMem_WriteData
/// @param  "mem_hndl"       memory handle, created in the specific memory implementation module
BOOL DrvMem_Flush(MEM_HNDL mem_hndl);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* MEM__DRVMEM_H */
