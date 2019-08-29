//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file helps the users to manage the buffers
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORELIST_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORELIST_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CORELIST_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of lists (default is 1)
#ifndef MAX_LIST_COUNT
    #define MAX_LIST_COUNT              1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the unit test has to be included
#ifndef INCLUDE_UNIT_TEST
    #define INCLUDE_UNIT_TEST           0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "CoreList.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define ILLEGAL_SLOT                    255
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U8* list_buffer_ptr;        // pointer to place in buffer were list data and sequence are stored. First max_item_count bytes contain the sequence, the data is stored thereafter
    U8  item_size;
    U8  max_item_count;
    U8  data_count;
    U8  start_slot;             // start slot is the offset where the first list entry is located
}
LIST_INFO_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if INCLUDE_UNIT_TEST
static void CoreList_UnitTest_Run(void);
static void CoreList_UnitTest_CheckData(LIST_HNDL list, U16* data_ptr);
#endif
static U8 List_GetFirstFreeList(void);
static void List_CheckIfValidList(LIST_INFO_STRUCT* list_ptr);
static U8 List_GetFirstFreeSlot(LIST_INFO_STRUCT* list_ptr);
static U8 List_GetSlotOfIndex(LIST_INFO_STRUCT* list_ptr, U8 index);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

LIST_INFO_STRUCT            list_info_struct[MAX_LIST_COUNT];
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
#if INCLUDE_UNIT_TEST
// unittest functie, er wordt een list gemaakt en telkens worden enkele acties hierop gedaan
// na iedere actie wordt gekeken of de list nog correct is
// enkel te gebruiken bij testen aanpassingen aan corelist
static void CoreList_UnitTest_Run(void)
{
    LIST_HNDL   list = CoreList_Create(2, 10);
    U16         data;

    //add @ top
    data = 'a';
    CoreList_Add(list, (VPTR)&data);
    U16 check[] = {'a'};
    CoreList_UnitTest_CheckData(list, check);

    data = 'b';
    CoreList_Add(list, (VPTR)&data);
    U16 check2[] = {'a','b'};
    CoreList_UnitTest_CheckData(list, check2);

    data = 'c';
    CoreList_Add(list, (VPTR)&data);
    U16 check3[] = {'a','b','c'};
    CoreList_UnitTest_CheckData(list, check3);

    //add @ middle
    data = 'd';
    CoreList_Insert(list, 1, (VPTR)&data);
    U16 check4[] = {'a','d','b','c'};
    CoreList_UnitTest_CheckData(list, check4);

    data = 'e';
    CoreList_Insert(list, 1, (VPTR)&data);
    U16 check5[] = {'a','e','d','b','c'};
    CoreList_UnitTest_CheckData(list, check5);

    //add @ start
    data = 'f';
    CoreList_Insert(list, 0, (VPTR)&data);
    U16 check6[] = {'f','a','e','d','b','c'};
    CoreList_UnitTest_CheckData(list, check6);

    //delete @ top
    CoreList_Remove(list, CoreList_GetCount(list)-1);
    U16 check7[] = {'f','a','e','d','b'};
    CoreList_UnitTest_CheckData(list, check7);

    //delete @ middle
    CoreList_Remove(list, 1);
    U16 check8[] = {'f','e','d','b'};
    CoreList_UnitTest_CheckData(list, check8);

    //delete @ start
    CoreList_Remove(list, 0);
    U16 check9[] = {'e','d','b'};
    CoreList_UnitTest_CheckData(list, check9);

    //add terug stuff to check of ie de gaten correct opvult
    data = 'g';
    CoreList_Add(list, (VPTR)&data);
    U16 check10[] = {'e','d','b','g'};
    CoreList_UnitTest_CheckData(list, check10);

    data = 'h';
    CoreList_Add(list, (VPTR)&data);
    U16 check11[] = {'e','d','b','g','h'};
    CoreList_UnitTest_CheckData(list, check11);

    data = 'i';
    CoreList_Add(list, (VPTR)&data);
    U16 check12[] = {'e','d','b','g','h','i'};
    CoreList_UnitTest_CheckData(list, check12);
}
//------------------------------------------------------------------------------------------------//
static void CoreList_UnitTest_CheckData(LIST_HNDL list, U16* data_ptr)
{
    U16 array[50];
    U8  i;
    
    CoreList_ToArray(list, (VPTR)array);

    for(i = 0; i < CoreList_GetCount(list); i++)
    {
        if(array[i] != *(data_ptr + i))
        {
            LOG_ERR("error, data does not match");
        }
    }
}
#endif
//------------------------------------------------------------------------------------------------//
static U8 List_GetFirstFreeList(void)
{
    U8 i;
    
    for(i = 0; i < MAX_LIST_COUNT; i++)
    {
        if(list_info_struct[i].list_buffer_ptr == NULL)
        {
            break;
        }
    }
    
    return i;   //if no place is found, then MAX_LIST_COUNT will be returned indicating that there is no more place
}
//------------------------------------------------------------------------------------------------//
// @remark void return because this function will throw an exception if list is not ok
static void List_CheckIfValidList(LIST_INFO_STRUCT* list_ptr)
{
    if((list_ptr >= &list_info_struct[MAX_LIST_COUNT]) || (list_ptr->list_buffer_ptr == NULL))
    {
        LOG_ERR("invalid list");
    }
}
//------------------------------------------------------------------------------------------------//
// @return the internal index where the next free place is (this index is not to be mistaken with the index visable for the user)
// @remark this function may only be called when there is at least 1 free slot, otherwise an exception is thrown.
static U8 List_GetFirstFreeSlot(LIST_INFO_STRUCT* list_ptr)
{
    U8* next_slot_ptr   = list_ptr->list_buffer_ptr;
    U8  slot;
    U8  slot_of_last_index;
    
    //if no data then next free is always 0
    if (list_ptr->data_count == 0)
    {
        return 0;
    }
    
    // the first free slot is the slot that is not linking to another slot and has no slot referring to itself
    // all active slots link to a next one, except for the last one. Therefore it must be checked if the slot is not linking to
    // another slot and it is not the last active slot
    slot_of_last_index = List_GetSlotOfIndex(list_ptr, list_ptr->data_count - 1);
    
    for(slot = 0; slot < list_ptr->max_item_count; slot++)
    {
        if((next_slot_ptr[slot] == ILLEGAL_SLOT) && (slot != slot_of_last_index))
        {
            return slot;
        }
    }
    
    LOG_ERR("No free slot found");
    return ILLEGAL_SLOT;
}
//------------------------------------------------------------------------------------------------//
// gets the dataslot of the desired index
// index = te logical position in the list, (this is what the user sees)
// slot = the offset of the data in the internal data storage (actual offset is slot*datasize)
// @remark this function may only be called with a valid requested index, otherwise an exception is thrown.
U8 List_GetSlotOfIndex(LIST_INFO_STRUCT* list_ptr, U8 index)
{
    U8* next_slot_ptr   = list_ptr->list_buffer_ptr;
    U8  slot            = list_ptr->start_slot;
    
    if(index >= list_ptr->data_count)
    {
        LOG_ERR("Illegal index");
        return ILLEGAL_SLOT;
    }
    
    while(index)
    {
        slot = next_slot_ptr[slot];
        index--;
    }
    
    return slot;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief init the module, must be done before using this module
void CoreList_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)list_info_struct, 0, SIZEOF(list_info_struct));
    
    MODULE_INIT_DONE();

#if INCLUDE_UNIT_TEST
    CoreList_UnitTest_Run();
#endif
}
//------------------------------------------------------------------------------------------------//
// @brief function to create a static list
// @param "item_size"      the size in bytes of one item
// @param "max_item_count" the list capacity (how much items you can put in the list),
// @return the list handle you need to use the other functions
// @remark space is allocated from corebuffer when calling this function, the space will be (item_size*max_item_count)+list_management_data
LIST_HNDL CoreList_Create(U8 item_size, U8 max_item_count)
{
    LIST_HNDL list;
    
    MODULE_CHECK();
    
    list = List_GetFirstFreeList();
    //check if still space for new list in listinfo
    if (list >= MAX_LIST_COUNT)
    {
        LOG_ERR("no more space for new list");
        return NULL;
    }
    
    list_info_struct[list].list_buffer_ptr  = CoreBuffer_CreateStaticU8(max_item_count * (item_size + 1), "List");
    list_info_struct[list].item_size        = item_size;
    list_info_struct[list].max_item_count   = max_item_count;
    
    CoreList_Clear(list);
    
    return list;
}
//------------------------------------------------------------------------------------------------//
// @brief deletes everything from the list
void CoreList_Clear(LIST_HNDL list)
{
    LIST_INFO_STRUCT*   list_ptr        = &list_info_struct[list];
    U8*                 next_slot_ptr   = list_ptr->list_buffer_ptr;
    
    //---------- failsafe checks ----------
    MODULE_CHECK();
    
    // check if valid list (will throw exception if not valid)
    List_CheckIfValidList(list_ptr);
    
    //---------- function logic ----------
    list_ptr->data_count = 0;
    list_ptr->start_slot = ILLEGAL_SLOT;
    MEMSET((VPTR)next_slot_ptr, ILLEGAL_SLOT, list_ptr->max_item_count);
}
//------------------------------------------------------------------------------------------------//
// @brief adds item @ end of the list
// @param "data_ptr"  pointer to the data, the data must be the size of the "item_size" of the list
// @return TRUE if successful
BOOL CoreList_Add(LIST_HNDL list, VPTR data_ptr)
{
    //no checks needed, stuff is checked in insert function and getcount function
    return CoreList_Insert(list, CoreList_GetCount(list), data_ptr);
}
//------------------------------------------------------------------------------------------------//
// @brief adds an item @ the specified index in the list
// @param "data_ptr"  pointer to the data, the data must be the size of the "item_size" of the list
// @return TRUE if successful
BOOL CoreList_Insert(LIST_HNDL list, U8 index, VPTR data_ptr)
{
    LIST_INFO_STRUCT*   list_ptr        = &list_info_struct[list];
    U8*                 next_slot_ptr   = list_ptr->list_buffer_ptr;
    U8*                 list_data_ptr   = list_ptr->list_buffer_ptr + list_ptr->max_item_count;
    U8                  slot;
    U8                  slot_of_previous_index;
    
    //---------- failsafe checks ----------
    MODULE_CHECK();
    
    // check if valid list (will throw exception if not valid)
    List_CheckIfValidList(list_ptr);
    
    // check if legal index (equal to datacount is allowed = append to end of list)
    if(index > list_ptr->data_count)
    {
        return FALSE;
    }
    
    // check if list is not full yet
    if(list_ptr->data_count >= list_ptr->max_item_count)
    {
        return FALSE;
    }
    
    //---------- function logic ----------
    slot = List_GetFirstFreeSlot(list_ptr);
    MEMCPY((VPTR)(list_data_ptr + (slot * list_ptr->item_size)), data_ptr, list_ptr->item_size);
    
    // if index = 0, replace start index
	if(index == 0)
    {
		next_slot_ptr[slot]                     = list_ptr->start_slot;
		list_ptr->start_slot                    = slot;
    }
    // otherwise: place in between
	else
    {
        slot_of_previous_index                  = List_GetSlotOfIndex(list_ptr, index - 1);
		next_slot_ptr[slot]                     = next_slot_ptr[slot_of_previous_index];
		next_slot_ptr[slot_of_previous_index]   = slot;
    }
    
    list_ptr->data_count++;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @brief removes the item @ the specified index in the list
// @return TRUE if successful
BOOL CoreList_Remove(LIST_HNDL list, U8 index)
{
    LIST_INFO_STRUCT*   list_ptr        = &list_info_struct[list];
    U8*                 next_slot_ptr   = list_ptr->list_buffer_ptr;
    U8                  slot_to_clear;
    
    //---------- failsafe checks ----------
    MODULE_CHECK();
    
    // check if valid list (will throw exception if not valid)
    List_CheckIfValidList(list_ptr);
    
    // check if valid index
    if(index >= list_ptr->data_count)
    {
        return FALSE;
    }
    
    //---------- function logic ----------
    // if index = 0, replace start index
	if(index == 0)
    {
        slot_to_clear = list_ptr->start_slot;
		list_ptr->start_slot = next_slot_ptr[list_ptr->start_slot];
    }
    // otherwise link previous to next
	else
    {
        slot_to_clear = List_GetSlotOfIndex(list_ptr, index);
        next_slot_ptr[List_GetSlotOfIndex(list_ptr, index - 1)] = next_slot_ptr[slot_to_clear];
    }
    
    next_slot_ptr[slot_to_clear] = ILLEGAL_SLOT;
    list_ptr->data_count--;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @brief reads an item from the list @ the specified index
// @param "data_ptr"  pointer to memory location where the data will be copied
// @return TRUE if successful
// @warning make sure the data pointer provides enough space, the minimum required is defined in CoreList_CreateList
BOOL CoreList_Read(LIST_HNDL list, U8 index, VPTR data_ptr)
{
    LIST_INFO_STRUCT*   list_ptr        = &list_info_struct[list];
    U8*                 list_data_ptr   = list_ptr->list_buffer_ptr + list_ptr->max_item_count;
    
    //---------- failsafe checks ----------
    MODULE_CHECK();
    
    // check if valid list (will throw exception if not valid)
    List_CheckIfValidList(list_ptr);
    
    // check if valid index
    if(index >= list_ptr->data_count)
    {
        return FALSE;
    }
    
    //---------- function logic ----------
    MEMCPY(data_ptr, list_data_ptr + (List_GetSlotOfIndex(list_ptr, index) * list_ptr->item_size), list_ptr->item_size);
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @brief returns the number of items currently in the list
U8 CoreList_GetCount(LIST_HNDL list)
{
    LIST_INFO_STRUCT*   list_ptr = &list_info_struct[list];
    
    //---------- failsafe checks ----------
    MODULE_CHECK();
    
    // check if valid list (will throw exception if not valid)
    List_CheckIfValidList(list_ptr);
    
    //---------- function logic ----------
    return list_ptr->data_count;
}
//------------------------------------------------------------------------------------------------//
// @brief copy list data to a memory location in the order it is in @ the moment this function is called
// @warning make sure the data pointer provides enough space, the minimum space required will depend on the item count in the list + the item size
void CoreList_ToArray(LIST_HNDL list, VPTR destination_ptr)
{
    LIST_INFO_STRUCT*   list_ptr = &list_info_struct[list];
    
    //---------- failsafe checks ----------
    MODULE_CHECK();
    
    // check if valid list (will throw exception if not valid)
    List_CheckIfValidList(list_ptr);
    
    //---------- function logic ----------
    U8 i;
    for(i = 0; i < list_ptr->data_count; i++)
    {
        CoreList_Read(list, i, destination_ptr);
        destination_ptr += list_ptr->item_size;
    }
}
//================================================================================================//
