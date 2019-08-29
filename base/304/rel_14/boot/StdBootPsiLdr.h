//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file contains the bootldr functionality
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef BOOT__STDBOOTPSILDR_H
#define BOOT__STDBOOTPSILDR_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define PSILDR_HEADER_SIZE                      16
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//contains all the info of program header
#pragma pack(1)
typedef struct
{
    U8      start_of_header;        //fixed @ 0x48 to indentify start of header
    U8      header_version;         //reserverved for version of psi ldr format, fornow fixed @ 0x01
    U32     destination_address;    //address where the data should be copyed
    U32     data_length;            //data lenth in bytes of the data following direct after a header
    U16     reserved_bytes;        //reserverd bytes to match the ldr specification
    U16     data_crc;               //16bit crc on all the data
    U16     header_crc;             //16bit crc on the 14 bytes above
}
BOOTLDR_APP_RAW_HEADER;
#pragma pack()
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern U32 stdbootpsildr_appaddress;
extern U32 stdbootpsildr_appendaddress;
extern U32 stdbootpsildr_newappaddress;
extern U32 stdbootpsildr_newappendaddress;
extern U32 stdbootpsildr_sectorsize;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
BOOL StdBootPsiLdr_CheckForNewAppHeaders(void);
BOOL StdBootPsiLdr_ValidateNewAppHeaders(void);
void StdBootPsiLdr_EraseNewAppHeaderDestinations(void);
void StdBootPsiLdr_LoadNewAppHeaders(void);
void StdBootPsiLdr_EraseNewAppRegion(void);
BOOL StdBootPsiLdr_CheckIfNewAppIsDifferent(void);
BOOL StdBootPsiLdr_IsValidHeader(BOOTLDR_APP_RAW_HEADER* header_ptr);
BOOL StdBootPsiLdr_ContainsValidData(BOOTLDR_APP_RAW_HEADER* header_ptr, U8* data_ptr);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* BOOT__STDBOOTPSILDR_H */

