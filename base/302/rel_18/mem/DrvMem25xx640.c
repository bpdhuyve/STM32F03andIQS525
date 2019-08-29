//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of 25xx640 SPI Eeprom
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MEM__DRVMEM25xx640_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__DRVMEM25xx640_LOG_LEVEL
	#define CORELOG_LEVEL               TERM_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MEM__DRVMEM25xx640_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of 25xx640 eeprom handles
#ifndef DRVMEM25xx640_COUNT
	#define DRVMEM25xx640_COUNT              1
#endif
//------------------------------------------------------------------------------------------------//
/// @brief   Defines the number of wait cycles until the device needs to be ready (processor speed dependent)
#ifndef MAX_WAIT_COUNT
    #define MAX_WAIT_COUNT                              50000 
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEM25xx640_SPI_CLOCK_SPEED
    #define DRVMEM25xx640_SPI_CLOCK_SPEED             2000000
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
// DRV
#include "DrvMem25xx640.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DRVMEM25xx640_PAGE_SIZE                 32
#define DRVMEM25xx640_NUMBER_OF_PAGES           250    // ((64k bit / 8 )bytes / 32) = 250 pages
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
{                                       // Description                                              -address bytes       - dummy bytes   - data bytes
    COMMAND_READ            = 0x03,     // Read Data Bytes                                                  2                   0           1 to inf
    COMMAND_WRITE           = 0x02,     // Write data to memory array beginning at selected address         2                   0           1 to 32
    COMMAND_WREN            = 0x06,     // Write Enable                                                     0                   0               0
    COMMAND_WRDI            = 0x04,     // Write Disable                                                    0                   0               0
    COMMAND_RDSR            = 0x05,     // Read from status register                                        0                   0               1 
    COMMAND_WRSR            = 0x01,     // Write to status register                                         0                   0               1
}
COMMAND;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (CORELOG_LEVEL > TERM_LEVEL_NONE)
static void Command_25xx640ReadStatus(void);
static void Command_25xx640Clear(void);
static void Command_25xx640Read(void);
static void Command_25xx640Write(void);
static void Command_25xx640MemTest(void);
static void Command_25xx640ReadFlash(void);
#endif

static BOOL DrvMem25xx640_WaitUntilReady(MEM_ID mem_id);
static BOOL DrvMem25xx640_IsReady(MEM_ID mem_id);
static U8   DrvMem25xx640_ReadStatus(MEM_ID mem_id);

static BOOL DrvMem25xx640_Flush(MEM_ID mem_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const MEM_HOOK_LIST          mem_hook_list = {DrvMem25xx640_Clear, DrvMem25xx640_Read, DrvMem25xx640_Write, DrvMem25xx640_Verify, DrvMem25xx640_Flush};
static MEM_CTRL_STRUCT		        mem_ctrl_struct[DRVMEM25xx640_COUNT];
static MEM_STRUCT                   mem_struct[DRVMEM25xx640_COUNT];
static U8							mem_count;
static U8                           mem_buffer[3];

#if (CORELOG_LEVEL > TERM_LEVEL_NONE)
static U8                           test_buffer[DRVMEM25xx640_NUMBER_OF_PAGES * DRVMEM25xx640_PAGE_SIZE];
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvMem25xx640_WaitUntilReady(MEM_ID mem_id)
{
    U32 count = 0;

    while(DrvMem25xx640_IsReady(mem_id) == FALSE)
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
static BOOL DrvMem25xx640_IsReady(MEM_ID mem_id)
{
    return (BOOL)((DrvMem25xx640_ReadStatus(mem_id) & 0x01) == 0x00);
}
//------------------------------------------------------------------------------------------------//
static U8 DrvMem25xx640_ReadStatus(MEM_ID mem_id)
{
    SPI_DEVICE_ID       spi_device_id = mem_ctrl_struct[mem_id].spi_device_id;

    mem_buffer[0] = COMMAND_RDSR;

    DrvSpiMasterDevice_Select(spi_device_id);
    DrvSpiMasterDevice_WriteData(spi_device_id, mem_buffer, 1);
    DrvSpiMasterDevice_ReadData(spi_device_id, mem_buffer, 1);
    DrvSpiMasterDevice_Deselect(spi_device_id);

    return mem_buffer[0];
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMem25xx640_Flush(MEM_ID mem_id)
{
    // no flushing needed, system can write single bytes.
    return TRUE;
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (CORELOG_LEVEL > TERM_LEVEL_NONE)
static void Command_25xx640ReadStatus(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);

    if(mem_id >= mem_count)
    {
        CoreTerm_PrintFailed();
        return;
    }

    LOG_TRM("%02x", PU8(DrvMem25xx640_ReadStatus(mem_id)));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_25xx640Clear(void)
{
    MEM_ID          mem_id = CoreTerm_GetArgumentAsU32(0);

    if(mem_id >= mem_count)
    {
        CoreTerm_PrintFailed();
        return;
    }

    CoreTerm_PrintFeedback(DrvMem25xx640_Clear(mem_id));
}
//------------------------------------------------------------------------------------------------//
static void Command_25xx640Read(void)
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

    if(DrvMem25xx640_Read(mem_id, CoreTerm_GetArgumentAsU32(1), test_buffer, length))
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
static void Command_25xx640Write(void)
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

    CoreTerm_PrintFeedback(DrvMem25xx640_Write(mem_id, CoreTerm_GetArgumentAsU32(1), test_buffer, length));
}
//------------------------------------------------------------------------------------------------//
static void Command_25xx640MemTest(void){    
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[CoreTerm_GetArgumentAsU32(0)];
    U8 data = 0x0;
    U16 flash_length =  (DRVMEM25xx640_NUMBER_OF_PAGES * DRVMEM25xx640_PAGE_SIZE);
    BOOL result = FALSE;
    
    for(U16 i = 0; i < flash_length; i++)
    {
        test_buffer[i] = data;
        data++;        
    }
    
    DrvMem25xx640_Write(mem_ctrl_struct_ptr->spi_device_id,0,test_buffer,flash_length);
    result = DrvMem25xx640_Verify(mem_ctrl_struct_ptr->spi_device_id,0,test_buffer,flash_length);   
    
    CoreTerm_PrintFeedback(result);
}
//------------------------------------------------------------------------------------------------//
static void Command_25xx640ReadFlash(void){    
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[CoreTerm_GetArgumentAsU32(0)];
    U16 start_address = CoreTerm_GetArgumentAsU32(1);
    U16 length = CoreTerm_GetArgumentAsU32(2);   
    U8 column_index = 0;
    U16 row_index = 0;
    
    //defines the number of columns when outputting the eeprom data
    const U8 columns = 8;    
    U8 column_data[8];    
    
    if(length == 0 || (length > (DRVMEM25xx640_NUMBER_OF_PAGES * DRVMEM25xx640_PAGE_SIZE - start_address)))
    {
        length = ( DRVMEM25xx640_NUMBER_OF_PAGES * DRVMEM25xx640_PAGE_SIZE ) - start_address;
    }
        
    DrvMem25xx640_Read(mem_ctrl_struct_ptr->spi_device_id,start_address,test_buffer,length);

    for(row_index = 0; row_index < length; row_index += columns)
    {        
        for(column_index = 0; column_index < columns; column_index++)
        {
            if(row_index + column_index < length)
            {
                column_data[column_index] = test_buffer[row_index + column_index];
            }
            else
            {
                column_data[column_index] = 0xFF;
            }              
        }             
        LOG("Address %4x: ",CORELOG_LEVEL,PU16(row_index));
        for(column_index = 0; column_index < (columns); column_index++)
        {
            LOG("%2x ", CORELOG_LEVEL,PU16(column_data[column_index]));            
        }   
        LOG("      ",CORELOG_LEVEL); 
        for(column_index = 0; column_index < (columns); column_index++)
        {
            LOG("%c ",CORELOG_LEVEL, PCHAR((CHAR) column_data[column_index]));            
        }
        LOG_TRM("");
        CoreLog_Flush();
    }
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvMem25xx640_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)mem_struct, 0, SIZEOF(mem_struct));
    MEMSET((VPTR)mem_ctrl_struct, 0, SIZEOF(mem_ctrl_struct));

    mem_count = 0;
#if (CORELOG_LEVEL > TERM_LEVEL_NONE)
    CoreTerm_RegisterCommand("25xx640ReadStatus", "25xx640 Read Status (a: mem_id)", 1, Command_25xx640ReadStatus, TRUE);
    CoreTerm_RegisterCommand("25xx640Clear", "25xx640 Clear (a: mem_id)", 1, Command_25xx640Clear, TRUE);
    CoreTerm_RegisterCommand("25xx640Read", "25xx640 Read (a: mem_id, b:addr_offset, c:length)", 3, Command_25xx640Read, TRUE);
    CoreTerm_RegisterCommand("25xx640Write", "25xx640 Write (a: mem_id, b:addr_offset, c:length, d:data_byte)", 4, Command_25xx640Write, TRUE);   
    CoreTerm_RegisterCommand("25xx640MemTest", "25xx640 Writes data to all addresses of the EEPROM (a: mem_id)", 1, Command_25xx640MemTest, TRUE);  
    CoreTerm_RegisterCommand("25xx640ShowMemData", "25xx640 Writes data to all addresses of the EEPROM (a: mem_id; b: start address; c: length)", 3, Command_25xx640ReadFlash, TRUE);
#endif    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
MEM_HNDL DrvMem25xx640_Register(SPI_DEVICE_ID spi_device_id, U32 address, U16 sector_count)
{
    MEM_HNDL            mem_hndl;
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr;
    U32                 length = sector_count * DRVMEM25xx640_PAGE_SIZE;
    SPI_CONFIG_STRUCT   config_struct;

    if((address & 0x0000FFFF) != 0)
    {
        LOG_WRN("Illegal start address");
        return NULL;
    }
    if(((address >> 16) + sector_count) > DRVMEM25xx640_NUMBER_OF_PAGES)
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

    if(mem_count < DRVMEM25xx640_COUNT)
    {
        config_struct.speed     = DRVMEM25xx640_SPI_CLOCK_SPEED;
        config_struct.mode      = MODE_0;
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
BOOL DrvMem25xx640_GetInfo(MEM_HNDL mem_hndl, U32* address_ptr, U32* length_ptr)
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
//------------------------------------------------------------------------------------------------//
BOOL DrvMem25xx640_Clear(MEM_ID mem_id)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32                 offset;
    U8                  empty_page[DRVMEM25xx640_PAGE_SIZE];
    
    //initialize array data
    MEMSET(&empty_page[0], 0xFF, DRVMEM25xx640_PAGE_SIZE);

    for(offset = 0; offset < mem_ctrl_struct_ptr->length; offset += DRVMEM25xx640_PAGE_SIZE)
    {
        System_KickDog();               
        
        // erase sector, by sending 32 times 0xFF
        DrvMem25xx640_Write(mem_ctrl_struct_ptr->spi_device_id, offset, empty_page, DRVMEM25xx640_PAGE_SIZE);
    }

    // check if ready
    if(DrvMem25xx640_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvMem25xx640_Read(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    BOOL                success;

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }

    // check if ready
    if(DrvMem25xx640_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }

    mem_buffer[0] = COMMAND_READ;
    mem_buffer[1] = ((mem_ctrl_struct_ptr->address + address_offset) >> 8) & 0xFF;
    mem_buffer[2] = ((mem_ctrl_struct_ptr->address + address_offset) >> 0) & 0xFF;

    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, mem_buffer, 3);
    success = DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, data_ptr, length);
    DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);

    return success;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvMem25xx640_Write(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U32                 page_address = mem_ctrl_struct_ptr->address + address_offset; //start address
    U16                 page_length;
    BOOL                success;

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }

    // check if ready
    if(DrvMem25xx640_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }

    while(length > 0)
    {
        System_KickDog();
        
        // determine (remaining)
        page_length = DRVMEM25xx640_PAGE_SIZE - ( page_address & 0x0000001F ); //number remaining bytes in the current page
        if(length < page_length) //how many bytes do we want to read? is it within the same page?
        {
            page_length = length;
        }

        // check if ready
        if(DrvMem25xx640_WaitUntilReady(mem_id) == FALSE)
        {
            return FALSE;
        }

        // enable writing
        mem_buffer[0] = COMMAND_WREN;
        DrvSpiMasterDevice_SelectWriteData(mem_ctrl_struct_ptr->spi_device_id, mem_buffer, 1);

        // program page
        mem_buffer[0] = COMMAND_WRITE;
        mem_buffer[1] = (page_address >> 8) & 0xFF;
        mem_buffer[2] = (page_address >> 0) & 0xFF;

        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, mem_buffer, 3);
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
    if(DrvMem25xx640_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvMem25xx640_Verify(MEM_ID mem_id, U32 address_offset, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*    mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U8                  flash_byte;

    if((address_offset + length) > mem_ctrl_struct_ptr->length)
    {
        return FALSE;
    }

    // check if ready
    if(DrvMem25xx640_WaitUntilReady(mem_id) == FALSE)
    {
        return FALSE;
    }

    mem_buffer[0] = COMMAND_READ;
    mem_buffer[1] = ((mem_ctrl_struct_ptr->address + address_offset) >> 8) & 0xFF;
    mem_buffer[2] = ((mem_ctrl_struct_ptr->address + address_offset) >> 0) & 0xFF;
    
    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, mem_buffer, 3);

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
//================================================================================================//
