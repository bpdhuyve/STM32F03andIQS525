//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file implements queue's that are configured at runtime
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef COREQ_H
#define COREQ_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @remark  invalid Q handle definition
#define INVALID_Q_HNDL                          0xFF
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @remark  Q handle typedef
typedef U8                              Q_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreQ_Init(void);
//------------------------------------------------------------------------------------------------//
/// @brief              registers a queue
/// @param "q_size"     the maxium number of items you want to put in the queue (size NOT in bytes but in elements)
/// @param "item_size"  the size of the items you want to put in the queue (in bytes)
/// @remark             the queue will take (q_size * item_size) bytes space from corebuffer
Q_HNDL CoreQ_Register(U16 q_size, U8 item_size, STRING name);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_Write(Q_HNDL hndl, VPTR data_ptr, U16 count);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_WriteAlloc(Q_HNDL hndl, U16 count, U16* start_index_ptr);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_WritePart(Q_HNDL hndl, U16 start_index, VPTR data_ptr, U16 part_count);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_WriteDone(Q_HNDL hndl, U16 count);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_Read(Q_HNDL hndl, VPTR data_ptr, U16 count);
//------------------------------------------------------------------------------------------------//
// @remark  returns the actual number of bytes that have been read out of the queue
U16 CoreQ_ReadSmart(Q_HNDL hndl, VPTR data_ptr, U16 max_count);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_Peek(Q_HNDL hndl, VPTR data_ptr, U16 count);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_Drop(Q_HNDL hndl, U16 count);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreQ_DropAll(Q_HNDL hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
U16 CoreQ_GetCount(Q_HNDL hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
U16 CoreQ_GetSpace(Q_HNDL hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreQ_Info(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* COREQ_H */

