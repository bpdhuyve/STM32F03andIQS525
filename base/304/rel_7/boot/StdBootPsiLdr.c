//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file contains the bootldr functionality
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define BOOT__STDBOOTPSILDR_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef BOOT__STDBOOTPSILDR_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               BOOT__STDBOOTPSILDR_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "mem\SysFlash.h"

// STD
#include "crc\StdCrc.h"
#include "boot\StdBootPsiLdr.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define APP_ADDRESS                 stdbootpsildr_appaddress
#define END_APP_ADDRESS             stdbootpsildr_appendaddress

#define NEW_APP_ADDRESS             stdbootpsildr_newappaddress
#define END_NEW_APP_ADDRESS         stdbootpsildr_newappendaddress
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static STRING  block = "- Block ";
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
U32 stdbootpsildr_appaddress;
U32 stdbootpsildr_appendaddress;
U32 stdbootpsildr_newappaddress;
U32 stdbootpsildr_newappendaddress;
U32 stdbootpsildr_sectorsize;
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
BOOL StdBootPsiLdr_CheckForNewAppHeaders(void)
{
    // at the new application location there should be a raw header, check for it
    BOOTLDR_APP_RAW_HEADER* header = (BOOTLDR_APP_RAW_HEADER*)NEW_APP_ADDRESS;
    
    return (BOOL)(header->start_of_header == 0x48);
}
//------------------------------------------------------------------------------------------------//
BOOL StdBootPsiLdr_ValidateNewAppHeaders(void)
{
    BOOTLDR_APP_RAW_HEADER* header_ptr = (BOOTLDR_APP_RAW_HEADER*)NEW_APP_ADDRESS;
    
    LOG_TRM("VALIDATE");
    
    while(StdBootPsiLdr_IsValidHeader(header_ptr))
    {
        if(StdBootPsiLdr_ContainsValidData(header_ptr, (U8*)(((U32)header_ptr) + PSILDR_HEADER_SIZE)) == FALSE)
        {
            return FALSE;
        }
        
        header_ptr = (BOOTLDR_APP_RAW_HEADER*)(((U32)header_ptr) + PSILDR_HEADER_SIZE + (header_ptr->data_length)); //set new appadress to next block
    }
    
    CoreTerm_PrintAcknowledge();
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
void StdBootPsiLdr_EraseNewAppHeaderDestinations(void)
{
    BOOTLDR_APP_RAW_HEADER* header_ptr = (BOOTLDR_APP_RAW_HEADER*)NEW_APP_ADDRESS;
    U32 address;
    U8  i = 1;
    
    LOG_TRM("ERASE TARGET");

    while(StdBootPsiLdr_IsValidHeader(header_ptr))
    {
        LOG_TRM("%s%d", PCSTR(block), PU8(i));

        for(address = header_ptr->destination_address; address < (header_ptr->destination_address + header_ptr->data_length); address += stdbootpsildr_sectorsize)
        {
            SysFlash_Clear(address);
        }

        header_ptr = (BOOTLDR_APP_RAW_HEADER*)(((U32)header_ptr) + PSILDR_HEADER_SIZE + (header_ptr->data_length)); //set new appadress to next block
        i++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
void StdBootPsiLdr_LoadNewAppHeaders(void)
{
    BOOTLDR_APP_RAW_HEADER* header_ptr = (BOOTLDR_APP_RAW_HEADER*)NEW_APP_ADDRESS;
    U8  i = 1;
    
    LOG_TRM("COPY");
    
    while(StdBootPsiLdr_IsValidHeader(header_ptr))
    {
        LOG_TRM("%s%d", PCSTR(block), PU8(i));
        
        SysFlash_Write(header_ptr->destination_address, (U8*)((U32)header_ptr + PSILDR_HEADER_SIZE), header_ptr->data_length);

        // check if there are more blocks with headers, they should follow right after the last block
        header_ptr = (BOOTLDR_APP_RAW_HEADER*)(((U32)header_ptr) + PSILDR_HEADER_SIZE + (header_ptr->data_length)); //set new appadress to next block
        i++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
void StdBootPsiLdr_EraseNewAppRegion(void)
{
    U32 address;

    LOG_TRM("ERASE SOURCE [0x%08h : 0x%08h]", PU32(NEW_APP_ADDRESS), PU32(END_NEW_APP_ADDRESS));

    for(address = NEW_APP_ADDRESS; address < END_NEW_APP_ADDRESS; address += stdbootpsildr_sectorsize)
    {
        SysFlash_Clear(address);
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
BOOL StdBootPsiLdr_IsValidHeader(BOOTLDR_APP_RAW_HEADER* header_ptr)
{
    // check first byte
    if(header_ptr->start_of_header != 0x48)
    {
        return FALSE;
    }
    
    LOG_TRM("%s [v%d] @ 0x%08h - %d bytes", PCSTR(block), PU8(header_ptr->header_version), PU32(header_ptr->destination_address), PU32(header_ptr->data_length));
    
    // check header crc
    if(header_ptr->header_crc != StdCrcGenerateCrc16IBM((U8*)header_ptr, 14))
    {
        LOG_TRM("  NOK - DataHeader corrupt");
        return FALSE;
    }
    
    // check version
    if(header_ptr->header_version != 1)
    {
        LOG_TRM("  NOK - DataHeader version not supported");
        return FALSE;
    }
    
    // check if data falls not out the application part
    if((header_ptr->destination_address < APP_ADDRESS) || ((header_ptr->destination_address + header_ptr->data_length) > END_APP_ADDRESS))
    {
        LOG_TRM("  NOK - Target out of range");
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdBootPsiLdr_ContainsValidData(BOOTLDR_APP_RAW_HEADER* header_ptr, U8* data_ptr)
{
    // check data crc
    if(header_ptr->data_crc != StdCrcGenerateCrc16IBM(data_ptr, header_ptr->data_length))
    {
        LOG_TRM("  NOK - Data corrupt");
        return FALSE;
    }
    
    return TRUE;
}
//================================================================================================//