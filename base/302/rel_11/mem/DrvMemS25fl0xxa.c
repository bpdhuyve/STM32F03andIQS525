//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the SPI memory S25FL0xxA
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MEM__DRVMEMS25FL0XXA_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__DRVMEMS25FL0XXA_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MEM__DRVMEMS25FL0XXA_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef DRVMEMS25FL0XXA_COUNT
	#define DRVMEMS25FL0XXA_COUNT		1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMS25FL0XXA_IGNORE_FLASH_CLEAR_FAIL
	#define DRVMEMS25FL0XXA_IGNORE_FLASH_CLEAR_FAIL		0
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMS25FL0XXA_NUMBER_OF_SECTORS
    #define DRVMEMS25FL0XXA_NUMBER_OF_SECTORS               128
#endif
//------------------------------------------------------------------------------------------------//
/// @brief   Defines the number of wait cycles until the device needs to be ready (processor speed dependent)
#ifndef MAX_WAIT_COUNT
    #define MAX_WAIT_COUNT                              50000
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMS25FL0XXA_SPI_CLOCK_SPEED
    #define DRVMEMS25FL0XXA_SPI_CLOCK_SPEED             20000000
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "mem\DrvMemS25fl0xxa.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    SPI_DEVICE_ID   spi_device_id;
    U32             address;
    U32             length;
}
MEM_CTRL_STRUCT;

typedef enum
{                                   // Description                          address bytes    - dummy bytes   - data bytes
    COMMAND_READ            = 0x03, // Read Data Bytes                          3                   0           1 to inf
    COMMAND_FAST_READ       = 0x0B, // Read Data Bytes at Higher Speed          3                   1           1 to inf
    COMMAND_RDID            = 0x9F, // Read Identification                      0                   0           1 to 3
    COMMAND_WREN            = 0x06, // Write Enable                             0                   0               0
    COMMAND_WRDI            = 0x04, // Write Disable                            0                   0               0
    COMMAND_SE              = 0xD8, // Sector Erase                             3                   0               0
    COMMAND_BE              = 0xC7, // Bulk (Chip) Erase                        0                   0               0
    COMMAND_PP              = 0x02, // Page Program                             3                   0           1 to 256
    COMMAND_RDSR            = 0x05, // Read from status register                0                   0           1 to inf
    COMMAND_WRSR            = 0x01, // Write to status register                 0                   0               1
    COMMAND_DP              = 0xB9, // Deep Power Down                          0                   0               0
    COMMAND_RES             = 0xAB, // Release from deep power down             0                   0               0
}
COMMAND;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_S25flReadId(void);
static void Command_S25flReadStatus(void);
static void Command_S25flClear(void);
static void Command_S25flRead(void);
static void Command_S25flWrite(void);
#endif

static BOOL DrvMemS25fl0xxa_WaitUntilReady(MEM_ID mem_id);
static BOOL DrvMemS25fl0xxa_IsReady(MEM_ID mem_id);
static U8 DrvMemS25fl0xxa_ReadStatus(MEM_ID mem_id);

static BOOL DrvMemS25fl0xxa_Clear(MEM_ID mem_id);
static BOOL DrvMemS25fl0xxa_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemS25fl0xxa_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemS25fl0xxa_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length);
static BOOL DrvMemS25fl0xxa_Flush(MEM_ID mem_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const MEM_HOOK_LIST                  mem_hook_list = {DrvMemS25fl0xxa_Clear, DrvMemS25fl0xxa_Read, DrvMemS25fl0xxa_Write, DrvMemS25fl0xxa_Verify, DrvMemS25fl0xxa_Flush};
static MEM_STRUCT                           mem_struct[DRVMEMS25FL0XXA_COUNT];
static MEM_CTRL_STRUCT                      mem_ctrl_struct[DRVMEMS25FL0XXA_COUNT];
static U8                                   mem_count;
static U8                                   spi_data[4];

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static U8                                   test_buffer[1024];
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_S25flReadId(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);
    SPI_DEVICE_ID   spi_device_id = mem_ctrl_struct[mem_id].spi_device_id;

    if(mem_id >= mem_count)
    {
        CoreTerm_PrintFailed();
        return;
    }

    spi_data[0] = (U8)COMMAND_RDID;
    DrvSpiMasterDevice_Select(spi_device_id);
    DrvSpiMasterDevice_WriteData(spi_device_id, spi_data, 1);
    DrvSpiMasterDevice_ReadData(spi_device_id, spi_data, 3);
    DrvSpiMasterDevice_Deselect(spi_device_id);

    LOG_TRM("%02x", PU8A(spi_data, 3));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_S25flReadStatus(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);

    if(mem_id >= mem_count)
    {
        CoreTerm_PrintFailed();
        return;
    }

    LOG_TRM("%02x", PU8(DrvMemS25fl0xxa_ReadStatus(mem_id)));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_S25flClear(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);

    if(mem_id >= mem_count)
    {
        CoreTerm_PrintFailed();
        return;
    }

    CoreTerm_PrintFeedback(DrvMemS25fl0xxa_Clear(mem_id));
}
//------------------------------------------------------------------------------------------------//
static void Command_S25flRead(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);
    U16             length = CoreTerm_GetArgumentAsU32(2);
    U16             part_len;
    U8*             data_ptr;

    if((mem_id >= mem_count) || (length > 1024))
    {
        CoreTerm_PrintFailed();
        return;
    }

    if(DrvMemS25fl0xxa_Read(mem_id, CoreTerm_GetArgumentAsU32(1), test_buffer, length))
    {
        data_ptr = test_buffer;
        while(length > 0)
        {
            part_len = MIN(32, length);
            LOG_TRM("%02x", PU8A(data_ptr, part_len));
            length -= part_len;
            data_ptr += part_len;
            CoreLog_Flush();
        }
        CoreTerm_PrintAcknowledge();
        return;
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void Command_S25flWrite(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);
    U16             length = CoreTerm_GetArgumentAsU32(2);
    U8              byte = CoreTerm_GetArgumentAsU32(3);

    if((mem_id >= mem_count) || (length > 1024))
    {
        CoreTerm_PrintFailed();
        return;
    }

    MEMSET((VPTR)test_buffer, byte, length);

    CoreTerm_PrintFeedback(DrvMemS25fl0xxa_Write(mem_id, CoreTerm_GetArgumentAsU32(1), test_buffer, length));
}
#endif
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemS25fl0xxa_WaitUntilReady(MEM_ID mem_id)
{
    U16                 count = 0;

    while(DrvMemS25fl0xxa_IsReady(mem_id) == FALSE)
    {
        System_KickDog();
        if(++count > MAX_WAIT_COUNT)
        {
            LOG_WRN("timeout");
            return FALSE;
        }
    }

    LOG_DBG("ready @%d", PU16(count));
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemS25fl0xxa_IsReady(MEM_ID mem_id)
{
    return (BOOL)((DrvMemS25fl0xxa_ReadStatus(mem_id) & 0x01) == 0x00);
}
//------------------------------------------------------------------------------------------------//
static U8 DrvMemS25fl0xxa_ReadStatus(MEM_ID mem_id)
{
    SPI_DEVICE_ID       spi_device_id = mem_ctrl_struct[mem_id].spi_device_id;

    spi_data[0] = (U8)COMMAND_RDSR;

    DrvSpiMasterDevice_Select(spi_device_id);
    DrvSpiMasterDevice_WriteData(spi_device_id, spi_data, 1);
    DrvSpiMasterDevice_ReadData(spi_device_id, spi_data, 1);
    DrvSpiMasterDevice_Deselect(spi_device_id);

    return spi_data[0];
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemS25fl0xxa_Clear(MEM_ID mem_id)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32                 offset;

    for(offset = 0; offset < mem_ctrl_struct_ptr->length; offset += DRVMEMS25FL0XXA_SECTOR_SIZE)
    {
        System_KickDog();

        // check if ready
        if(DrvMemS25fl0xxa_WaitUntilReady(mem_id) == FALSE)
        {
            return FALSE;
        }

        // enable writing
        spi_data[0] = (U8)COMMAND_WREN;
        DrvSpiMasterDevice_SelectWriteData(mem_ctrl_struct_ptr->spi_device_id, spi_data, 1);

        // erase sector
        spi_data[0] = (U8)COMMAND_SE;
        spi_data[1] = (U8)(((mem_ctrl_struct_ptr->address + offset) >> 16) & 0xFF);
        spi_data[2] = (U8)(((mem_ctrl_struct_ptr->address + offset) >> 8) & 0xFF);
        spi_data[3] = (U8)(((mem_ctrl_struct_ptr->address + offset) >> 0) & 0xFF);

        DrvSpiMasterDevice_SelectWriteData(mem_ctrl_struct_ptr->spi_device_id, spi_data, 4);
    }

    // check if ready
    if(DrvMemS25fl0xxa_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemS25fl0xxa_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    BOOL                success;

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }

    // check if ready
    if(DrvMemS25fl0xxa_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }

    spi_data[0] = (U8)COMMAND_READ;
    spi_data[1] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 16) & 0xFF);
    spi_data[2] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 8) & 0xFF);
    spi_data[3] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 0) & 0xFF);

    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, spi_data, 4);
    success = DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, data_ptr, length);
    DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);

    return success;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemS25fl0xxa_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32                 page_address = mem_ctrl_struct_ptr->address + address_offset;
    U16                 page_length;
    BOOL                success;

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }

    // check if ready
    if(DrvMemS25fl0xxa_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }

    while(length > 0)
    {
        System_KickDog();

        // determine (remaining)
        page_length = DRVMEMS25FL0XXA_PAGE_SIZE - (page_address & 0x000000FF);
        if(length < page_length)
        {
            page_length = length;
        }

        // check if ready
        if(DrvMemS25fl0xxa_WaitUntilReady(mem_id) == FALSE)
        {
            return FALSE;
        }

        // enable writing
        spi_data[0] = (U8)COMMAND_WREN;
        DrvSpiMasterDevice_SelectWriteData(mem_ctrl_struct_ptr->spi_device_id, spi_data, 1);

        // program page
        spi_data[0] = (U8)COMMAND_PP;
        spi_data[1] = (U8)((page_address >> 16) & 0xFF);
        spi_data[2] = (U8)((page_address >> 8) & 0xFF);
        spi_data[3] = (U8)((page_address >> 0) & 0xFF);

        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, spi_data, 4);
        success = DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, data_ptr, page_length);
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);

        length -= page_length;
        data_ptr += page_length;
        page_address += page_length;

        if(success == FALSE)
        {
            return FALSE;
        }
    }


    // check if ready
    if(DrvMemS25fl0xxa_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemS25fl0xxa_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U8                  flash_byte;

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }

    // check if ready
    if(DrvMemS25fl0xxa_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }

    spi_data[0] = (U8)COMMAND_READ;
    spi_data[1] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 16) & 0xFF);
    spi_data[2] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 8) & 0xFF);
    spi_data[3] = (U8)(((mem_ctrl_struct_ptr->address + address_offset) >> 0) & 0xFF);

    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, spi_data, 4);

    while(length > 0)
    {
        if(DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, &flash_byte, 1) == FALSE)
        {
            break;
        }
        if(flash_byte != *data_ptr)
        {
            break;
        }
        data_ptr++;
        length--;
    }

    DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);

    return (BOOL)(length == 0);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemS25fl0xxa_Flush(MEM_ID mem_id)
{
    // no flushing needed, system can write single bytes.
    return TRUE;
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvMemS25fl0xxa_Init(void)
{
    MEMSET((VPTR)mem_struct, 0, SIZEOF(mem_struct));
    MEMSET((VPTR)mem_ctrl_struct, 0, SIZEOF(mem_ctrl_struct));

    mem_count = 0;

    CoreTerm_RegisterCommand("S25flReadId", "S25FL Read ID (a: mem_id)", 1, Command_S25flReadId, TRUE);
    CoreTerm_RegisterCommand("S25flReadStatus", "S25FL Read Status (a: mem_id)", 1, Command_S25flReadStatus, TRUE);
    CoreTerm_RegisterCommand("S25flClear", "S25FL Clear (a: mem_id)", 1, Command_S25flClear, TRUE);
    CoreTerm_RegisterCommand("S25flRead", "S25FL Read (a: mem_id, b:addr_offset, c:length)", 3, Command_S25flRead, TRUE);
    CoreTerm_RegisterCommand("S25flWrite", "S25FL Write (a: mem_id, b:addr_offset, c:length, d:data_byte)", 4, Command_S25flWrite, TRUE);
}
//------------------------------------------------------------------------------------------------//
MEM_HNDL DrvMemS25fl0xxa_Register(SPI_DEVICE_ID spi_device_id, U32 address, U16 sector_count)
{
    MEM_HNDL            mem_hndl;
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr;
    U32                 length = sector_count * DRVMEMS25FL0XXA_SECTOR_SIZE;
    SPI_CONFIG_STRUCT   config_struct;

    if((address & 0x0000FFFF) != 0)
    {
        LOG_WRN("Illegal start address");
        return NULL;
    }
    if(((address >> 16) + sector_count) > DRVMEMS25FL0XXA_NUMBER_OF_SECTORS)
    {
        LOG_WRN("Illegal length");
        return NULL;
    }

    for(mem_hndl = mem_struct, mem_ctrl_struct_ptr = mem_ctrl_struct; mem_hndl < &mem_struct[mem_count]; mem_hndl++, mem_ctrl_struct_ptr++)
    {
        if((mem_ctrl_struct_ptr->spi_device_id == spi_device_id) && (mem_ctrl_struct_ptr->address == address) && (mem_ctrl_struct_ptr->length == length))
        {
            return mem_hndl;
        }
        if(!((address + length <= mem_ctrl_struct_ptr->address) || (address >= mem_ctrl_struct_ptr->address + mem_ctrl_struct_ptr->length)))
        {
            LOG_WRN("Overlap");
            return NULL;
        }
    }

    if(mem_count < DRVMEMS25FL0XXA_COUNT)
    {
        config_struct.speed     = DRVMEMS25FL0XXA_SPI_CLOCK_SPEED;
        config_struct.mode      = MODE_3;
        config_struct.bitcount  = 8;
        config_struct.lsb_first = FALSE;

        DrvSpiMasterDevice_Config(spi_device_id, &config_struct);

        mem_ctrl_struct_ptr->spi_device_id = spi_device_id;
        mem_ctrl_struct_ptr->address = address;
        mem_ctrl_struct_ptr->length = length;

        mem_hndl->hook_list_ptr = (MEM_HOOK_LIST*)&mem_hook_list;
        mem_hndl->mem_id = mem_count;
        mem_count++;

        return mem_hndl;
    }
    return NULL;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvMemS25fl0xxa_GetInfo(MEM_HNDL mem_hndl, U32* address_ptr, U32* length_ptr)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_hndl->mem_id];

    if(mem_hndl->hook_list_ptr != &mem_hook_list)
    {
        return FALSE;
    }

    *address_ptr = mem_ctrl_struct_ptr->address;
    *length_ptr = mem_ctrl_struct_ptr->length;
    return TRUE;
}
//================================================================================================//
