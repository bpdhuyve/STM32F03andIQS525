//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Determination of board specifics
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STDBOARDINFO_H
#define STDBOARDINFO_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define BOARD_INFO_HARDWARE_NUMBER_LENGTH       10
#define BOARD_INFO_SERIAL_NUMBER_LENGTH         12
#define BOARD_INFO_HARDWARE_NAME_LENGTH         17  // 1 byte for '\0' included
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U16     product_number;
    U16     major_revision;
    U16     minor_revision;
    U16     test_revision;
}
PRODUCT_VERSION;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void StdBoardInfo_Init(void);

// @remark  The ptr data_ptr should point to a buffer of at least 10 bytes
void StdBoardInfo_GetHardwareNumber(U8* data_ptr);

// @remark  The ptr data_ptr should point to a buffer of at least 12 bytes
void StdBoardInfo_GetSerialNumber(U8* data_ptr);

// @remark  The STRING name should point to a buffer of at least 17 bytes
void StdBoardInfo_GetHardwareName(STRING name);

U16 StdBoardInfo_GetHardwareFunction(void);

U8 StdBoardInfo_GetHardwareVersion(void);

U8 StdBoardInfo_GetHardwareRevision(void);

U16 StdBoardInfo_GetBoardFamilyNumber(void);

U16 StdBoardInfo_GetCompatibilityNumber(void);

U32  StdBoardInfo_GetReservedField(void);

void StdBoardInfo_GetBootSwVersion(PRODUCT_VERSION* version_ptr);
//================================================================================================//



#endif /* STDBOARDINFO_H */
