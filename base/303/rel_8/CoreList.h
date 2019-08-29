//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file helps the users to manage the buffers
//
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
// @brief init the module, must be done before using this module
void CoreList_Init(void);
//------------------------------------------------------------------------------------------------//
// @brief function to create a static list
// @param "item_size"      the size in bytes of one item
// @param "max_item_count" the list capacity (how much items you can put in the list),
// @return the list handle you need to use the other functions
// @remark space is allocated from corebuffer when calling this function, the space will be (item_size*max_item_count)+list_management_data
LIST_HNDL CoreList_Create(U8 item_size, U8 max_item_count);
//------------------------------------------------------------------------------------------------//
// @brief deletes everything from the list
void CoreList_Clear(LIST_HNDL list);
//------------------------------------------------------------------------------------------------//
// @brief adds item @ end of the list
// @param "data_ptr"  pointer to the data, the data must be the size of the "item_size" of the list
// @return TRUE if successful
BOOL CoreList_Add(LIST_HNDL list, VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
// @brief adds an item @ the specified index in the list
// @param "data_ptr"  pointer to the data, the data must be the size of the "item_size" of the list
// @return TRUE if successful
BOOL CoreList_Insert(LIST_HNDL list, U8 index, VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
// @brief removes the item @ the specified index in the list
// @return TRUE if successful
BOOL CoreList_Remove(LIST_HNDL list, U8 index);
//------------------------------------------------------------------------------------------------//
// @brief reads an item from the list @ the specified index
// @param "data_ptr"  pointer to memory location where the data will be copied
// @return TRUE if successful
// @warning make sure the data pointer provides enough space, the minimum required is defined in CoreList_CreateList
BOOL CoreList_Read(LIST_HNDL list, U8 index, VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
// @brief returns the number of items currently in the list
U8 CoreList_GetCount(LIST_HNDL list);
//------------------------------------------------------------------------------------------------//
// @brief copy list data to a memory location in the order it is in @ the moment this function is called
// @warning make sure the destination pointer provides enough space, the minimum space required will depend on the item count in the list + the item size
void CoreList_ToArray(LIST_HNDL list, VPTR destination_pointer);
//------------------------------------------------------------------------------------------------//
// future functions proposals
// U8 CoreList_Find(VPTR data_ptr);	//finds the item in the list and returns the index
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif // CORELIST_H

