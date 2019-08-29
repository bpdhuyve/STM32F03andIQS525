//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Determination of board specifics
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STDBOARDINFO_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDBOARDINFO_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STDBOARDINFO_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STDBOARDINFO_INFO_BLOCK_ADDRESS
    #error "STDBOARDINFO_INFO_BLOCK_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STDBOARDINFO_BOOT_VERSION_ADDRESS
    #error "STDBOARDINFO_BOOT_VERSION_ADDRESS not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// STD
#include "StdBoardInfo.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
#pragma pack(1)
typedef struct
{
	U8   hardware_number[BOARD_INFO_HARDWARE_NUMBER_LENGTH];
	U8   serial_number[BOARD_INFO_SERIAL_NUMBER_LENGTH];
	U8   hardware_name[BOARD_INFO_HARDWARE_NAME_LENGTH - 1];
	U8   hardware_version;
	U8   hardware_revision;
	U16  hardware_function;
	U16  board_family_number;
	U16  compatibility_number;
	U32  reserved; // Application specific
}
BOARD_INFO_BLOCK;
#pragma pack()
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_BoardInfo(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static BOARD_INFO_BLOCK*                board_info_ptr;
static PRODUCT_VERSION*                 boot_version_ptr;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_BoardInfo(void)
{
    U8 array[BOARD_INFO_HARDWARE_NAME_LENGTH];
    PRODUCT_VERSION boot_version;
    
    // BOARD INFO
    StdBoardInfo_GetHardwareName((STRING)array);
    LOG_TRM("Board:             %c %d %dV%d", PCHARA(array, 16),
                                              PU16(StdBoardInfo_GetHardwareFunction()),
                                              PU8(StdBoardInfo_GetHardwareVersion()),
                                              PU8(StdBoardInfo_GetHardwareRevision()));
    StdBoardInfo_GetHardwareNumber(array);
    LOG_TRM("Hardware number:   %c", PCHARA(board_info_ptr->hardware_number, 10));
    StdBoardInfo_GetSerialNumber(array);
    LOG_TRM("Serial:            %c", PCHARA(board_info_ptr->serial_number, 12));
    LOG_TRM("Family:            %d", PU16(StdBoardInfo_GetBoardFamilyNumber()));
    LOG_TRM("Compatibility:     %d", PU16(StdBoardInfo_GetCompatibilityNumber()));
    LOG_TRM("Reserved:          %d", PU32(StdBoardInfo_GetReservedField()));
    
    // BOOT INFO
    StdBoardInfo_GetBootSwVersion(&boot_version);
    LOG_TRM("BootSW:            %d.%d.%d.%d", PU16(boot_version.product_number),
                                              PU16(boot_version.major_revision),
                                              PU16(boot_version.minor_revision),
                                              PU16(boot_version.test_revision));
    
    CoreTerm_PrintAcknowledge();
    CoreLog_Flush();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdBoardInfo_Init(void)
{
    MODULE_INIT_ONCE();
    
    boot_version_ptr = (PRODUCT_VERSION*)STDBOARDINFO_BOOT_VERSION_ADDRESS;
    board_info_ptr   = (BOARD_INFO_BLOCK*)STDBOARDINFO_INFO_BLOCK_ADDRESS;
    
#if (TERM_LEVEL > TERM_LEVEL_NONE)
    CoreTerm_RegisterCommand("BoardInfo", "BOARD info", 0, Command_BoardInfo, TRUE);
#endif
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void StdBoardInfo_GetHardwareNumber(U8* data_ptr)
{
    if(data_ptr != NULL)
    {
        MEMCPY(data_ptr, board_info_ptr->hardware_number, BOARD_INFO_HARDWARE_NUMBER_LENGTH);
    }
}
//------------------------------------------------------------------------------------------------//
void StdBoardInfo_GetSerialNumber(U8* data_ptr)
{
    if(data_ptr != NULL)
    {
        MEMCPY(data_ptr, board_info_ptr->serial_number, BOARD_INFO_SERIAL_NUMBER_LENGTH);
    }
}
//------------------------------------------------------------------------------------------------//
void StdBoardInfo_GetHardwareName(STRING name)
{
    if(name != NULL)
    {
        MEMCPY(name, board_info_ptr->hardware_name, BOARD_INFO_HARDWARE_NAME_LENGTH - 1);
        name[BOARD_INFO_HARDWARE_NAME_LENGTH - 1] = 0;
    }
}
//------------------------------------------------------------------------------------------------//
U16 StdBoardInfo_GetHardwareFunction(void)
{
    return board_info_ptr->hardware_function;
}
//------------------------------------------------------------------------------------------------//
U8 StdBoardInfo_GetHardwareVersion(void)
{
    return board_info_ptr->hardware_version;
}
//------------------------------------------------------------------------------------------------//
U8 StdBoardInfo_GetHardwareRevision(void)
{
    return board_info_ptr->hardware_revision;
}
//------------------------------------------------------------------------------------------------//
U16 StdBoardInfo_GetBoardFamilyNumber(void)
{
    return board_info_ptr->board_family_number;
}
//------------------------------------------------------------------------------------------------//
U16 StdBoardInfo_GetCompatibilityNumber(void)
{
    return board_info_ptr->compatibility_number;
}
//------------------------------------------------------------------------------------------------//
U32  StdBoardInfo_GetReservedField(void)
{
    return board_info_ptr->reserved;
}
//------------------------------------------------------------------------------------------------//
void StdBoardInfo_GetBootSwVersion(PRODUCT_VERSION* version_ptr)
{
    if(version_ptr != NULL)
    {
        *version_ptr = *boot_version_ptr;
    }
}
//================================================================================================//