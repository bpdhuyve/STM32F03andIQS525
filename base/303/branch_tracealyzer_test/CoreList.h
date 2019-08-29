//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//
// corelist = static implementation of a list (uses no heap)
// this list element takes only data with a fixed length, later a CoreVarList module could be created that takes variable sized elements and that just uses corelist as it exists to store them
//================================================================================================//
#ifndef CORELIST_H
#define CORELIST_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_LIST_HNDL               255
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                              LIST_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/// @brief init the module, must be done before using this module
void CoreList_Init(void);
//------------------------------------------------------------------------------------------------//
/// @brief  function to create a static list
/// @param	"max_item_count" the list capacity (how many items can this list maximum hold)
/// @return the list handle that is needed to modify the list (using the other functions of this module)
/// @remark memory is allocated from corebuffer when calling this function (the memory size will be (item_size*max_item_count)+list_management_data)
LIST_HNDL CoreList_Create(U8 item_size_in_bytes, U8 max_item_count);
//------------------------------------------------------------------------------------------------//
/// @brief 	deletes everything from the list
void CoreList_Clear(LIST_HNDL list);
//------------------------------------------------------------------------------------------------//
/// @brief 	adds item @ end of the list
/// @param 	"data_ptr"  pointer to the data, the data must be the size of the "item_size" of the list
/// @return TRUE if successful
BOOL CoreList_Add(LIST_HNDL list, VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
// @brief adds an item multiple times @ end of the list
// @param "data_ptr"  pointer to the data, the data must be the size of the "item_size" of the list
// @return TRUE if successful
BOOL CoreList_Fill(LIST_HNDL list, VPTR data_ptr, U8 times_to_add);
//------------------------------------------------------------------------------------------------//
/// @brief 	adds an item @ the specified index in the list
/// @param 	"data_ptr"  pointer to the data, the data must be the size of the "item_size" of the list
/// @return TRUE if successful
BOOL CoreList_Insert(LIST_HNDL list, U8 index, VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
/// @brief 	removes the item @ the specified index in the list
/// @return TRUE if successful
BOOL CoreList_Remove(LIST_HNDL list, U8 index);
//------------------------------------------------------------------------------------------------//
/// @brief  reads an item from the list @ the specified index
/// @param  "data_ptr"  pointer to memory location where the data will be copied
///         make sure the pointer provides enough space!, the minimum required is defined in CoreList_CreateList
/// @return TRUE if successful
BOOL CoreList_Read(LIST_HNDL list, U8 index, VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
/// @brief 	returns the number of items currently in the list
U8 CoreList_GetCount(LIST_HNDL list);
//------------------------------------------------------------------------------------------------//
// @brief returns the max number of items that can fit in the list
U8 CoreList_GetMaxCount(LIST_HNDL list);
//------------------------------------------------------------------------------------------------//
// @brief returns the places still available to put stuff in the list
U8 CoreList_GetFreeSpace(LIST_HNDL list);
//------------------------------------------------------------------------------------------------//
/// @brief  copy list data to a memory location in its current order @ the moment this function is called
/// @param  "destination_pointer" pointer that must contains the address where the data will be copied, make sure the pointer provides enough space
///         the minimum space required will depend on the item count in the list + the item size
void CoreList_ToArray(LIST_HNDL list, VPTR destination_pointer);
//------------------------------------------------------------------------------------------------//
/// @brief  copies a list into another list, all items that are already in the destination list will be erased!
/// @param  "destination_list"  WARNING, the desination list must already created with the same item size as the source list
/// @return TRUE if successfull
BOOL CoreList_Copy(LIST_HNDL destination_list, LIST_HNDL source_list);
//------------------------------------------------------------------------------------------------//
//future functions proposals
//U8 CoreList_Find(VPTR data_ptr);	//finds the item in the list and returns the index
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif // CORELIST_H
