//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of chip independent layer for the Bluetooth LE upload service
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef COMMBTLEBOOTLOADERSERVICE_H
#define COMMBTLEBOOTLOADERSERVICE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "mem\DrvMem.h"
#include "board\StdBoardInfo.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef BOOL (*COMM_IS_TX_FREE_HOOK)(void);
typedef BOOL (*COMM_NOTIFY_CTRL_HOOK)(U8* data_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
// The first 32 bytes are used to store a flash struct with version info
#define BTLE_BOOTLOADER_PSILDR_OFFSET            32
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
// @param   mem_hndl:           MEM_HNDL to the memory where the new software should be stored.
// @param   active_software:    If an application software is valid and present in the active memory, 
//                              the software product (with its revisions) should be entered here.
//                              If no valid software is present, the correct product_number (which to filter upon) 
//                              should be entered, the revision_numbers should be equal to 0xFFFF.
// @param   max_product_length: The maximum length of the (to be received) backup software image
void CommBtleBootloaderService_Init(MEM_HNDL mem_hndl, const U16* encryption_table_ptr);

// @remark  Background handler
void CommBtleBootloaderService_Handler(void);

void CommBtleBootloaderService_RegisterCtrlPointData(U8* data_ptr, U16 length);

void CommBtleBootloaderService_RegisterUploadData(U8* data_ptr, U16 length);

void CommBtleBootloaderService_RegisterIsTxFreeHook(COMM_IS_TX_FREE_HOOK hook);

void CommBtleBootloaderService_RegisterNotifyCtrlHook(COMM_NOTIFY_CTRL_HOOK hook);

void CommBtleBootloaderService_RegisterActivateImageHook(EVENT_CALLBACK hook);

BOOL CommBtleBootloaderService_IsUploadBusy(void);
//================================================================================================//



#endif /* COMMBTLEBOOTLOADERSERVICE_H */
