//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of data manager for saving system settings/data structs on the internal flash 
// of the chip. This module requires that internal flash can be read directly by using
// its memory address.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STDDATAMNGR_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDDATAMNGR_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STDDATAMNGR_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of modules that can save its data structure
#ifndef DATAMNGR_MODULE_COUNT
    #error "DATAMNGR_MODULE_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DATA_ADDRESS
    #error "DATA_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef BACKUP_ADDRESS
    #error "BACKUP_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DATA_SIZE
    #error "DATA_SIZE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SECTOR_SIZE
    #error "SECTOR_SIZE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of modules that can save its data structure
#ifndef DELAYED_BACKUP
    #define DELAYED_BACKUP              0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// STD
#include "crc\StdCrc.h"
#include "StdDataMngr.h"

// DRV
#include "mem\DrvMem.h"
#include "mem\DrvMemSysFlash.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define FLASH_STRUCT_VERSION                        1
#define SIZE_IN_8BIT(x)                             ((x + 1) & 0xFFFE)  // round up to align 16bit
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
	U16	software_number;
	U8	flash_struct_version;
	U8	module_count;
}
DATAMNGR_HEADER;

typedef struct
{
	U16	data_len;
	U8	data_version;
	U8	module_id;
}
DATAMNGR_MODULE_HEADER;

typedef struct
{
    DATAMNGR_CONVERT_DATA   convert_data_hook;
	DATAMNGR_LOAD_DEFAULTS  load_defaults_hook;
	VPTR                     data_ptr;
}
DATAMNGR_MODULE_POINTERS;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_DataInfo(void);
static void Command_DataClear(void);
static void Command_DataSave(void);
static void Command_DataDefaults(void);
static void Command_DataRaw(void);
#endif
static void DataMngrVerifyFlash(void);
static BOOL DataMngrGetFlashDataPtr(U8 module_id, DATAMNGR_MODULE_HEADER* flash_module_header_ptr, U8** flash_data_start_ptr_ptr);
static BOOL DataMngr_PrintSaveFeedback(BOOL is_success, STRING action_string);
static BOOL DataMngr_SaveBackup(void);
static BOOL DataMngr_RestoreBackup(void);
#if DELAYED_BACKUP == 1
static void DataMngr_DelayedBackup(VPTR data_ptr);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static DATAMNGR_HEADER                  datamngr_header;
static DATAMNGR_MODULE_HEADER           datamngr_module_header[DATAMNGR_MODULE_COUNT];
static DATAMNGR_MODULE_POINTERS         datamngr_module_pointers[DATAMNGR_MODULE_COUNT];

static BOOL                             datamngr_flashdata_healthy;
static U8                               datamngr_module_count;

static MEM_HNDL                         datamngr_mem_hndl;
static MEM_HNDL                         datamngr_backup_hndl;

static const STRING                     datamngr_string = "[DATAMNGR] ";

#if DELAYED_BACKUP == 1
TASK_HNDL                               datamngr_backup_delay_task;
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
static void Command_DataInfo(void)
{
    U8                      j;
    U8*                     data_ptr;
    DATAMNGR_MODULE_HEADER  flash_module_header;
    DATAMNGR_MODULE_HEADER* ram_module_header_ptr;
    const STRING            oknok[] = {"NOK", "OK", "NOK : LEN", "NOK : VERS", "NOK : N/A"};
    
    LOG_TRM("%sINFO", PCSTR(datamngr_string));
    LOG_TRM("SW%d - [v%d] - %s", PU16(datamngr_header.software_number),
                            PU8(datamngr_header.flash_struct_version),
                            PCSTR(oknok[datamngr_flashdata_healthy]));
    LOG_TRM("Modules");
    for(ram_module_header_ptr = datamngr_module_header; ram_module_header_ptr < &datamngr_module_header[DATAMNGR_MODULE_COUNT]; ram_module_header_ptr++)
    {
        if(ram_module_header_ptr->data_len > 0)
        {
            j = 4;
            if(DataMngrGetFlashDataPtr(ram_module_header_ptr->module_id, &flash_module_header, &data_ptr))
            {
                j--;
                if(flash_module_header.data_version == ram_module_header_ptr->data_version)
                {
                    j--;
                    if(flash_module_header.data_len == ram_module_header_ptr->data_len)
                    {
                        j--;
                    }
                }
            }
            
            LOG_TRM("- ID %-2d - %4db [v%d] - %s",
                    PU8(ram_module_header_ptr->module_id),
                    PU16(ram_module_header_ptr->data_len),
                    PU8(ram_module_header_ptr->data_version),
                    PCSTR(oknok[j]));
            CoreLog_Flush();
        }
    }
    LOG_TRM("Use: %d/%d", PU8(datamngr_module_count), PU8(datamngr_header.module_count));
}
//------------------------------------------------------------------------------------------------//
static void Command_DataClear(void)
{
    switch(CoreTerm_GetArgumentAsU32(0))
    {
    case 0:
        CoreTerm_PrintFeedback(DrvMem_Clear(datamngr_mem_hndl));
        break;
    case 1:
        CoreTerm_PrintFeedback(DrvMem_Clear(datamngr_backup_hndl));
        break;
    default:
        CoreTerm_PrintFailed();
        break;
    }
    DataMngrVerifyFlash();
}
//------------------------------------------------------------------------------------------------//
static void Command_DataSave(void)
{
    StdDataMngr_SaveData();
    CoreTerm_PrintFeedback(datamngr_flashdata_healthy);
}
//------------------------------------------------------------------------------------------------//
static void Command_DataDefaults(void)
{
    StdDataMngr_LoadDefaults();
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_DataRaw(void)
{
    MEM_HNDL    mem_hndl;
    U8*         start_address;
    U32         memdata_length;
    U8*         data_ptr;
    
    switch(CoreTerm_GetArgumentAsU32(0))
    {
    case 0:
        mem_hndl = datamngr_mem_hndl;
        break;
    case 1:
        mem_hndl = datamngr_backup_hndl;
        break;
    default:
        CoreTerm_PrintFailed();
        return;
    }
    
    if(DrvMemSysFlash_GetInfo(mem_hndl, (U32*)&start_address, &memdata_length))
    {
        for(data_ptr = start_address; data_ptr < (start_address + memdata_length); data_ptr += 32)
        {
            LOG_TRM("0x%08h: %02h", PU32(data_ptr), PU8A(data_ptr, 32));
            CoreLog_Flush();
        }
        CoreTerm_PrintAcknowledge();
    }
    else
    {
        CoreTerm_PrintFailed();
    }
}
#endif
//------------------------------------------------------------------------------------------------//
static void DataMngrVerifyFlash(void)
{
    U8*             data_ptr;
    U8*             flash_end_ptr;
    U16             crc;
    DATAMNGR_HEADER flash_data_hdr;
    U8              i;
    
    U8*             data_start_ptr;
    U32             memdata_length;
    
    datamngr_flashdata_healthy = FALSE;
    
    if(DrvMemSysFlash_GetInfo(datamngr_mem_hndl, (U32*)&data_start_ptr, &memdata_length) == FALSE)
    {
        LOG_DBG("%sMem hdnl problem", PCSTR(datamngr_string));
        return;
    }
    
    data_ptr      = data_start_ptr;
    flash_end_ptr = data_start_ptr;
    
    // CHECK HEADER
    MEMCPY((VPTR)&flash_data_hdr, (VPTR)data_ptr, sizeof(DATAMNGR_HEADER));
    if(flash_data_hdr.software_number != PRODUCT_ID_NUMBER)
    {
        LOG_DBG("%sWrong Product ID", PCSTR(datamngr_string));
        return;
    }
    if(flash_data_hdr.flash_struct_version != FLASH_STRUCT_VERSION)
    {
        LOG_DBG("%sUnknown Flash Version", PCSTR(datamngr_string));
        return;
    }
    
    // CHECK OUT OF BOUNDS
    data_ptr += SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER));
    flash_end_ptr += SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER)) + (flash_data_hdr.module_count * SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER)));
    if(flash_end_ptr > (data_start_ptr + memdata_length))
    {
        LOG_DBG("%sData pointer out of bounds", PCSTR(datamngr_string));
        return;
    }
    
    // CHECK EACH MODULE HEADER
    for(i = 0; i < flash_data_hdr.module_count; i++)
    {
        flash_end_ptr += SIZE_IN_8BIT(((DATAMNGR_MODULE_HEADER*)data_ptr)->data_len);
        data_ptr += SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER));
        if(flash_end_ptr > (data_start_ptr + memdata_length))
        {
            LOG_DBG("%sData pointer out of bounds", PCSTR(datamngr_string));
            return;
        }
    }
    
    // CHECK CRC
    crc = StdCrcGenerateCrc16IBM(data_start_ptr, (U32)flash_end_ptr - (U32)data_start_ptr);
    if(crc != *(U16*)((data_start_ptr + memdata_length) - SIZE_IN_8BIT(SIZEOF(crc))))
    {
        LOG_DBG("%sCRC failed: 0x%04h - 0x%04h", PCSTR(datamngr_string), PU16(crc), PU16(*(U16*)((data_start_ptr + memdata_length) - SIZE_IN_8BIT(SIZEOF(crc)))));
        return;
    }
    
    // DATA VERIFIED
    LOG_DBG("%sFlash Data Verified", PCSTR(datamngr_string));
    
    datamngr_flashdata_healthy = TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DataMngrGetFlashDataPtr(U8 module_id, DATAMNGR_MODULE_HEADER* flash_module_header_ptr, U8** flash_data_start_ptr_ptr)
{
    U8* data_ptr;
    U8  flash_module_count;
    U8  i;
    U32 memdata_length;
    
    DrvMemSysFlash_GetInfo(datamngr_mem_hndl, (U32*)&data_ptr, &memdata_length);
    
    // LOAD HEADER
    flash_module_count = ((DATAMNGR_HEADER*)data_ptr)->module_count;
    data_ptr += SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER));
    *flash_data_start_ptr_ptr = data_ptr + (flash_module_count * SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER)));
    
    // SEARCH IF MODULE KNOWN IN FLASH
    for(i=0; i<flash_module_count; i++)
    {
        MEMCPY((VPTR)flash_module_header_ptr, (VPTR)data_ptr, sizeof(DATAMNGR_MODULE_HEADER));
        if(flash_module_header_ptr->module_id == module_id)
        {
            // FOUND, RETURN MODULE HEADER DATA & FLASH DATA PTR
            return TRUE;
        }
        data_ptr += SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER));
        *flash_data_start_ptr_ptr += SIZE_IN_8BIT(flash_module_header_ptr->data_len);
    }
    
    // NOT FOUND
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DataMngr_PrintSaveFeedback(BOOL is_success, STRING action_string)
{
    if(is_success)
    {
        LOG_DBG("[DATAMNGR] SAVE - %s OK", PCSTR(action_string));
    }
    else
    {
        LOG_WRN("[DATAMNGR] SAVE - %s FAILED", PCSTR(action_string));
    }
    return is_success;
}
//------------------------------------------------------------------------------------------------//
static BOOL DataMngr_SaveBackup(void)
{
    U8* data_ptr;
    U8* backup_ptr;
    U32 memdata_length;
    U8* data_start_ptr;
    
    if((datamngr_flashdata_healthy == TRUE) &&
       (datamngr_backup_hndl != NULL) &&
       (DrvMemSysFlash_GetInfo(datamngr_backup_hndl, (U32*)&backup_ptr, &memdata_length)) &&
       (DrvMemSysFlash_GetInfo(datamngr_mem_hndl, (U32*)&data_ptr, &memdata_length)))
    {
        data_start_ptr = data_ptr;
        while(data_ptr < (data_start_ptr + memdata_length))
        {
            if(*data_ptr != *backup_ptr)
            {
                break;
            }
            data_ptr++;
            backup_ptr++;
        }
        
        if(data_ptr == (data_start_ptr + memdata_length))
        {
            DataMngr_PrintSaveFeedback(TRUE, "backup");
        }
        else
        {
            if(DataMngr_PrintSaveFeedback(DrvMem_Clear(datamngr_backup_hndl), "clear backup") == FALSE)
            {
                return FALSE;
            }
            if(DataMngr_PrintSaveFeedback(DrvMem_WriteData(datamngr_backup_hndl, 0, data_start_ptr, memdata_length), "make backup") == FALSE)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DataMngr_RestoreBackup(void)
{
    U8* backup_ptr;
    U32 memdata_length;
    
    if((datamngr_backup_hndl != NULL) && DrvMemSysFlash_GetInfo(datamngr_backup_hndl, (U32*)&backup_ptr, &memdata_length))
    {
        if(DataMngr_PrintSaveFeedback(DrvMem_Clear(datamngr_mem_hndl), "clear data") == FALSE)
        {
            return FALSE;
        }
        if(DataMngr_PrintSaveFeedback(DrvMem_WriteData(datamngr_mem_hndl, 0, backup_ptr, memdata_length), "restore backup") == FALSE)
        {
            return FALSE;
        }
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
#if DELAYED_BACKUP == 1
static void DataMngr_DelayedBackup(VPTR data_ptr)
{
    CoreTask_Stop(datamngr_backup_delay_task);
    DataMngr_SaveBackup();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdDataMngr_Init(void)
{
    MODULE_INIT_ONCE();
    
    datamngr_mem_hndl    = DrvMemSysFlash_Register(DATA_ADDRESS, DATA_SIZE, SECTOR_SIZE);
    datamngr_backup_hndl = DrvMemSysFlash_Register(BACKUP_ADDRESS, DATA_SIZE, SECTOR_SIZE);
    
    datamngr_header.software_number      = PRODUCT_ID_NUMBER,
    datamngr_header.flash_struct_version = FLASH_STRUCT_VERSION,
    datamngr_header.module_count         = DATAMNGR_MODULE_COUNT,
    
    datamngr_module_count = 0;
    
    MEMSET((VPTR)datamngr_module_header,   0, sizeof(datamngr_module_header));
    MEMSET((VPTR)datamngr_module_pointers, 0, sizeof(datamngr_module_pointers));
    
    CoreTerm_RegisterCommand("DataInfo", "DATAMNGR info", 0, Command_DataInfo, TRUE);
    CoreTerm_RegisterCommand("DataClear", "DATAMNGR clear data", 1, Command_DataClear, TRUE);
    CoreTerm_RegisterCommand("DataSave", "DATAMNGR save data", 0, Command_DataSave, TRUE);
    CoreTerm_RegisterCommand("DataDefaults", "DATAMNGR load default data in RAM", 0, Command_DataDefaults, TRUE);
    CoreTerm_RegisterCommand("DataRaw", "DATAMNGR print raw data", 1, Command_DataRaw, FALSE);
    
    DataMngrVerifyFlash();
    
    if(datamngr_flashdata_healthy == TRUE)
    {
        // check if backup is needed
        DataMngr_SaveBackup();
    }
    else
    {
        // restore backup and check again
        DataMngr_RestoreBackup();
        DataMngrVerifyFlash();
    }
    
#if DELAYED_BACKUP == 1
    datamngr_backup_delay_task = CoreTask_RegisterTask(400e3, DataMngr_DelayedBackup, NULL, 128, "Backup");
#endif
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_Register(DATAMNGR_MODULE_ID module_id,
                          DATAMNGR_CONVERT_DATA convert_data_hook,
                          DATAMNGR_LOAD_DEFAULTS load_defaults_hook,
                          VPTR data_ptr,
                          U16 data_len,
                          U8 data_version)
{
    U8*             flash_data_start_ptr;
    DATAMNGR_MODULE_HEADER      flash_module_header;
    DATAMNGR_MODULE_HEADER*     ram_module_header_ptr;
    DATAMNGR_MODULE_POINTERS*   ram_module_ptr_ptr;
    U32             offset;
    
    U8*             data_start_ptr;
    U32             memdata_length;
    
#if (CORELOG_LEVEL & LOG_LEVEL_WARN)
    const STRING    error_string = "REGISTER ERROR - ";
#endif
    
    MODULE_CHECK();
    
    if(DrvMemSysFlash_GetInfo(datamngr_mem_hndl, (U32*)&data_start_ptr, &memdata_length) == FALSE)
    {
        LOG_WRN("%s%sMem hdnl problem", PCSTR(datamngr_string), PCSTR(error_string));
        return;
    }
    
    // VERIFY REGISTRATION
    if(datamngr_module_count >= DATAMNGR_MODULE_COUNT)
    {
        LOG_WRN("%s%smax modules reached", PCSTR(datamngr_string), PCSTR(error_string));
        return;
    }
    if(data_ptr == NULL)
    {
        LOG_WRN("%s%sdata ptr is NULL", PCSTR(datamngr_string), PCSTR(error_string));
        return;
    }
    if(data_len == 0)
    {
        LOG_WRN("%s%sdata length is 0", PCSTR(datamngr_string), PCSTR(error_string));
        return;
    }
    offset = SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER)) + DATAMNGR_MODULE_COUNT * SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER));  // HDR
    for(ram_module_header_ptr = datamngr_module_header; ram_module_header_ptr < &datamngr_module_header[datamngr_module_count]; ram_module_header_ptr++)
    {
        if(ram_module_header_ptr->module_id == (U8)module_id)
        {
            LOG_WRN("%s%sdouble registration of module 0x%02h", PCSTR(datamngr_string), PCSTR(error_string), PU8(module_id));
            return;
        }
        // data size of registered modules
        offset += SIZE_IN_8BIT(ram_module_header_ptr->data_len);
    }
    offset += SIZE_IN_8BIT(data_len);       // data size of new module
    offset += 2;                            // crc
    if(offset > memdata_length)
    {
        LOG_WRN("%s%sout of bounds", PCSTR(error_string), PU8(module_id));
        return;
    }
    
    // REGISTER DATA
    ram_module_header_ptr->data_len = data_len;
    ram_module_header_ptr->data_version = data_version;
    ram_module_header_ptr->module_id = (U8)module_id;
    
    ram_module_ptr_ptr = &datamngr_module_pointers[datamngr_module_count];
    ram_module_ptr_ptr->convert_data_hook = convert_data_hook;
    ram_module_ptr_ptr->load_defaults_hook = load_defaults_hook;
    ram_module_ptr_ptr->data_ptr = data_ptr;
    
    // LOAD DEFAULTS
    if(load_defaults_hook != NULL)
    {
        load_defaults_hook();
    }
    
    // TRY TO RETRIEVE FLASH DATA
    if((datamngr_flashdata_healthy == TRUE) && DataMngrGetFlashDataPtr(module_id, &flash_module_header, &flash_data_start_ptr))
    {
        // COPY DATA IF CONSISTENT
        if((flash_module_header.data_len == data_len) && (flash_module_header.data_version == data_version))
        {
            MEMCPY(data_ptr, (VPTR)flash_data_start_ptr, data_len);
        }
        // OTHERWISE, TRY TO CONVERT
        else if(convert_data_hook != NULL)
        {
            convert_data_hook(flash_data_start_ptr, flash_module_header.data_len, flash_module_header.data_version);
        }
    }
    
    datamngr_module_count++;
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_SaveData(void)
{
    U32     offset = 0;
    U16     crc;
    DATAMNGR_MODULE_HEADER*     ram_module_header_ptr;
    DATAMNGR_MODULE_POINTERS*   ram_module_ptr_ptr;
    
    U8*             data_start_ptr;
    U32             memdata_length;
    
    MODULE_CHECK();
    
    if(DataMngr_PrintSaveFeedback(DrvMemSysFlash_GetInfo(datamngr_mem_hndl, (U32*)&data_start_ptr, &memdata_length), "mem hndl") == FALSE)
    {
        return;
    }
    
    if(DataMngr_SaveBackup() == FALSE)
    {
        return;
    }
    
    // CLEAR SECTOR
    if(DataMngr_PrintSaveFeedback(DrvMem_Clear(datamngr_mem_hndl), "clear data") == FALSE)
    {
        return;
    }
    
    // WRITE HEADER
    if(DataMngr_PrintSaveFeedback(DrvMem_WriteData(datamngr_mem_hndl, offset, (U8*)&datamngr_header, SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER))), "write header") == FALSE)
    {
        return;
    }
    offset += SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER));
    
    // WRITE MODULE HEADERS
    for(ram_module_header_ptr = datamngr_module_header; ram_module_header_ptr < &datamngr_module_header[DATAMNGR_MODULE_COUNT]; ram_module_header_ptr++)
    {
        if(DataMngr_PrintSaveFeedback(DrvMem_WriteData(datamngr_mem_hndl, offset, (U8*)ram_module_header_ptr, SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER))), "write module header") == FALSE)
        {
            return;
        }
        offset += SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER));
    }
    
    // WRITE MODULE DATA
    for(ram_module_header_ptr = datamngr_module_header, ram_module_ptr_ptr = datamngr_module_pointers;
        ram_module_header_ptr < &datamngr_module_header[datamngr_module_count];
        ram_module_header_ptr++, ram_module_ptr_ptr++)
    {
        if((ram_module_header_ptr->data_len > 0) && (ram_module_ptr_ptr->data_ptr != NULL))
        {
            if(DataMngr_PrintSaveFeedback(DrvMem_WriteData(datamngr_mem_hndl, offset, (U8*)ram_module_ptr_ptr->data_ptr, SIZE_IN_8BIT(ram_module_header_ptr->data_len)), "write module data") == FALSE)
            {
                return;
            }
            offset += SIZE_IN_8BIT(ram_module_header_ptr->data_len);
        }
    }
    
    if(DataMngr_PrintSaveFeedback(DrvMem_Flush(datamngr_mem_hndl), "flush data") ==  FALSE)
    {
        return;
    }
    
    // APPEND CRC
    crc = StdCrcGenerateCrc16IBM(data_start_ptr, offset);
    if(DataMngr_PrintSaveFeedback(DrvMem_WriteData(datamngr_mem_hndl, memdata_length - SIZE_IN_8BIT(SIZEOF(crc)), (U8*)&crc, SIZE_IN_8BIT(SIZEOF(crc))), "write CRC") == FALSE)
    {
        return;
    }
       
    if(DataMngr_PrintSaveFeedback(DrvMem_Flush(datamngr_mem_hndl), "flush CRC") == FALSE)
    {
        return;
    }
    
    LOG_TRM("%sSAVE - CRC 0x%04h", PCSTR(datamngr_string), PU16(crc));

    DataMngrVerifyFlash();
    
#if DELAYED_BACKUP == 1
    CoreTask_Start(datamngr_backup_delay_task);
#else
    DataMngr_SaveBackup();
#endif
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_LoadDefaults(void)
{
    U8      i;
    
    for(i = 0; i < datamngr_module_count; i++)
    {
        if(datamngr_module_pointers[i].load_defaults_hook != NULL)
        {
            datamngr_module_pointers[i].load_defaults_hook();
        }
    }
}
//================================================================================================//