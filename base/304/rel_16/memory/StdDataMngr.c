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
#ifndef DATA_SIZE
    #error "DATA_SIZE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DATA_SECTOR_SIZE
    #error "DATA_SECTOR_SIZE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef BACKUP_ADDRESS
    #error "BACKUP_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef BACKUP_SECTOR_SIZE
    #define BACKUP_SECTOR_SIZE              DATA_SECTOR_SIZE
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the delay time in case delayed actions are required
#ifndef DELAY_TIME
    #define DELAY_TIME                      400000
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines is the back should be executed immediately or after 400ms (fixed for now)
#ifndef DELAYED_BACKUP
    #define DELAYED_BACKUP                  0
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the size of the RAM to be used to copy backup (if 0, no RAM is used)
// @remark This might be needed if it is not poossible to read from the flash while writing
#ifndef RAM_SIZE_FOR_BACKUP
    #define RAM_SIZE_FOR_BACKUP             0
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the datasave needs to be called after loading defaults (if 0, no automatic saving)
#ifndef AUTOSAVE_AFTER_LOADDEFAULTS
    #define AUTOSAVE_AFTER_LOADDEFAULTS     0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// STD
#include "crc\StdCrc.h"
#include "memory/StdDataMngr.h"

// DRV
#include "mem\DrvMem.h"
#include "mem\DrvMemSysFlash.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define FLASH_STRUCT_VERSION                        1
#define SIZE_IN_8BIT(x)                             ((x + 1) & 0xFFFE)  // round up to align 16bit

// check macro's
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & (LOG_LEVEL_DEVELOP)
    #define REGISTER_CHECK(x, y)        if(DataMngr_PrintFeedback((BOOL)(x), DATAMNGR_ACTION_REGISTER, y) == FALSE) {return;}
    #define VERIFY_CHECK(x,y)           if(DataMngr_PrintFeedback((BOOL)(x), DATAMNGR_ACTION_VERIFY, y) == FALSE) {return;}
    #define SAVE_CHECK(x,y)             if(DataMngr_PrintFeedback((BOOL)(x), DATAMNGR_ACTION_SAVE, y) == FALSE) {return;}
    #define DUMP_CHECK(x,y)             if(DataMngr_PrintFeedback((BOOL)(x), DATAMNGR_ACTION_DUMP, y) == FALSE) {return FALSE;}
    #define ACTION_CHECK(a,x,y)         if(DataMngr_PrintFeedback((BOOL)(x), a, y) == FALSE) {return FALSE;}
#else
    #define REGISTER_CHECK(x, y)        if((BOOL)(x) == FALSE) {return;}
    #define VERIFY_CHECK(x,y)           if((BOOL)(x) == FALSE) {return;}
    #define SAVE_CHECK(x,y)             if((BOOL)(x) == FALSE) {return;}
    #define DUMP_CHECK(x,y)             if((BOOL)(x) == FALSE) {return FALSE;}
    #define ACTION_CHECK(a,x,y)         if((BOOL)(x) == FALSE) {return FALSE;}
#endif
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
	VPTR                    data_ptr;
#if (INCLUDE_INFO_STRING == 1)
    STRING                  module_name;
#endif
}
DATAMNGR_MODULE_POINTERS;

typedef enum
{
    DATAMNGR_STATE_IDLE                 = 0,
    DATAMNGR_STATE_WAIT_TO_SAVE         = 1,
    DATAMNGR_STATE_SAVE_ACTIVE          = 2,
    DATAMNGR_STATE_WAIT_TO_BACKUP       = 3,
    DATAMNGR_STATE_BACKUP_ACTIVE        = 4,
}
DATAMNGR_STATE;

typedef enum
{
    DATAMNGR_ACTION_REGISTER            = 0,
    DATAMNGR_ACTION_VERIFY              = 1,
    DATAMNGR_ACTION_DUMP                = 2,
    DATAMNGR_ACTION_SAVE                = 3,
    DATAMNGR_ACTION_BACKUP              = 4,
    DATAMNGR_ACTION_RESTORE             = 5,
}
DATAMNGR_ACTION;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// state machine handling
static void DataMngr_EnterState(DATAMNGR_STATE state);
static void DataMngr_Handler(void);
#if (DELAY_TIME > 0)
static void DataMngr_DelayTask(VPTR data_ptr);
#endif

// data verification
static void DataMngr_VerifyFlash(void);

// data save, backup & restore
static void DataMngr_SaveData(void);
static BOOL DataMngr_DataAndBackupEqual(void);
static BOOL DataMngr_BackupAction(DATAMNGR_ACTION action);

// data register
static BOOL DataMngr_GetFlashDataPtr(U8 module_id, DATAMNGR_MODULE_HEADER* flash_module_header_ptr, U8** flash_data_start_ptr_ptr);

// feedback
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & (LOG_LEVEL_DEVELOP)
static BOOL DataMngr_PrintFeedback(BOOL is_success, DATAMNGR_ACTION action, STRING action_string);
#endif

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_DataInfo(void);
static void Command_DataClear(void);
static void Command_DataSave(void);
static void Command_DataDefaults(void);
static void Command_DataRaw(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static DATAMNGR_HEADER                  datamngr_header;
static DATAMNGR_MODULE_HEADER           datamngr_module_header[DATAMNGR_MODULE_COUNT];
static DATAMNGR_MODULE_POINTERS         datamngr_module_pointers[DATAMNGR_MODULE_COUNT];
static U8                               datamngr_module_count;

static DATAMNGR_STATE                   datamngr_state;
#if (DELAY_TIME > 0)
static TASK_HNDL                        datamngr_delay_task;
#endif
static BOOL                             datamngr_save_requested;
static BOOL                             datamngr_delayed_save_requested;
static BOOL                             datamngr_backup_requested;
static BOOL                             datamngr_save_allowed;
static BOOL                             datamngr_flashdata_healthy;

static MEM_HNDL                         datamngr_data_mem_hndl;
static MEM_HNDL                         datamngr_backup_mem_hndl;

static EVENT_CALLBACK                   datamngr_ondefaultsloaded_hook;

static const STRING                     datamngr_string = "[DATAMNGR] ";

#if (RAM_SIZE_FOR_BACKUP > 0)
static U8                               datamngr_backup_ram[RAM_SIZE_FOR_BACKUP];
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DataMngr_EnterState(DATAMNGR_STATE state)
{
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & (LOG_LEVEL_DEVELOP)
    const STRING state_names[] = {"IDLE", "WAIT TO SAVE", "SAVE ACTIVE", "WAIT TO BACKUP", "BACKUP ACTIVE"};
    LOG_DEV("%s-> %s", PCSTR(datamngr_string), PCSTR(state_names[state]));
#endif
    
    datamngr_state = state;
}
//------------------------------------------------------------------------------------------------//
static void DataMngr_Handler(void)
{
    // HANDLE delayed saving
    if(datamngr_delayed_save_requested)
    {
        datamngr_delayed_save_requested     = FALSE;    // clear delayed save request flag
        
        #if (DELAY_TIME > 0)
        {
            // enter wait state
            DataMngr_EnterState(DATAMNGR_STATE_WAIT_TO_SAVE);
            
            // clear other pending actions (possible save request is not cleared)
            datamngr_backup_requested       = FALSE;    // clear pending backup request
            
            // initiate delayed saving
            CoreTask_SetDataPtr(datamngr_delay_task, (VPTR)&datamngr_save_requested);
            CoreTask_Start(datamngr_delay_task);
        }
        #else
        {
            datamngr_save_requested         = TRUE;     // take over to save directly
        }
        #endif
    }
    
    // HANDLE save
    if(datamngr_save_requested && datamngr_save_allowed)
    {
        // enter state and clear save request flag
        DataMngr_EnterState(DATAMNGR_STATE_SAVE_ACTIVE);
        datamngr_save_requested             = FALSE;
        
        // clear other pending actions
        #if (DELAY_TIME > 0)
        {
            CoreTask_Stop(datamngr_delay_task);         // stop pending delayed actions
        }
        #endif
        datamngr_delayed_save_requested     = FALSE;    // clear pending delayed saving request
        datamngr_backup_requested           = FALSE;    // clear pending backup request
        
        // save data
        DataMngr_SaveData();
        
        // initiate backup (if possible delayed, otherwise request backup immediately)
        #if ((DELAY_TIME > 0) && (DELAYED_BACKUP == 1))
        {
            DataMngr_EnterState(DATAMNGR_STATE_WAIT_TO_BACKUP);
            
            CoreTask_SetDataPtr(datamngr_delay_task, (VPTR)&datamngr_backup_requested);
            CoreTask_Start(datamngr_delay_task);
        }
        #else
        {
            datamngr_backup_requested   = TRUE;
        }
        #endif
    }
    
    // HANDLE backup
    if(datamngr_backup_requested && datamngr_save_allowed)
    {
        // enter state and clear backup request flag
        DataMngr_EnterState(DATAMNGR_STATE_BACKUP_ACTIVE);
        datamngr_backup_requested = FALSE;
        
        // save backup
        DataMngr_BackupAction(DATAMNGR_ACTION_BACKUP);
        
        // indicate saving done
        DataMngr_EnterState(DATAMNGR_STATE_IDLE);
    }
}
//------------------------------------------------------------------------------------------------//
#if (DELAY_TIME > 0)
static void DataMngr_DelayTask(VPTR data_ptr)
{
    CoreTask_Stop(datamngr_delay_task);
    *((BOOL*)data_ptr) = TRUE;
}
#endif
//------------------------------------------------------------------------------------------------//
static void DataMngr_VerifyFlash(void)
{
    DATAMNGR_HEADER     flash_data_hdr;
    U8*                 data_ptr;
    U32                 data_size;
    U16                 crc;
    U8                  i;
    
     // INIT AS NOT OK
    datamngr_flashdata_healthy = FALSE;
    
    // CHECK HEADER
    data_ptr = (U8*)DATA_ADDRESS;
    MEMCPY((VPTR)&flash_data_hdr, (VPTR)data_ptr, sizeof(DATAMNGR_HEADER));
    VERIFY_CHECK(flash_data_hdr.software_number == PRODUCT_ID_NUMBER, "Product ID");
    VERIFY_CHECK(flash_data_hdr.flash_struct_version == FLASH_STRUCT_VERSION, "Flash version");
    
    // CHECK OUT OF BOUNDS
    data_size = SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER)) + (flash_data_hdr.module_count * SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER)));
    data_ptr += SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER));
    for(i = 0; i < flash_data_hdr.module_count; i++)
    {
        data_size += SIZE_IN_8BIT(((DATAMNGR_MODULE_HEADER*)data_ptr)->data_len);
        data_ptr  += SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER));
    }
    VERIFY_CHECK(data_size <= DATA_SIZE, "bounds");
    
    // CHECK CRC
    crc = StdCrcGenerateCrc16IBM((U8*)DATA_ADDRESS, data_size);
    VERIFY_CHECK(crc == *(U16*)(((U8*)DATA_ADDRESS + DATA_SIZE) - SIZE_IN_8BIT(SIZEOF(crc))), "CRC");
    
    // INDICATE OK
    datamngr_flashdata_healthy = TRUE;
}
//------------------------------------------------------------------------------------------------//
static void DataMngr_SaveData(void)
{
    DATAMNGR_MODULE_HEADER*     ram_module_header_ptr;
    DATAMNGR_MODULE_POINTERS*   ram_module_ptr_ptr;
    U32                         offset = 0;
    U16                         crc;
    
    // BACKUP CURRENT DATA (if not done before)
    DataMngr_BackupAction(DATAMNGR_ACTION_BACKUP);
    
    // CLEAR SECTOR
    SAVE_CHECK(DrvMem_Clear(datamngr_data_mem_hndl), "clear");
    
    // WRITE HEADER
    SAVE_CHECK(DrvMem_WriteData(datamngr_data_mem_hndl, offset, (U8*)&datamngr_header, SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER))), "write hdr");
    offset += SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER));
    
    // WRITE MODULE HEADERS
    for(ram_module_header_ptr = datamngr_module_header; ram_module_header_ptr < &datamngr_module_header[DATAMNGR_MODULE_COUNT]; ram_module_header_ptr++)
    {
        SAVE_CHECK(DrvMem_WriteData(datamngr_data_mem_hndl, offset, (U8*)ram_module_header_ptr, SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER))), "write mod hdr");
        offset += SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER));
    }
    
    // WRITE MODULE DATA
    for(ram_module_header_ptr = datamngr_module_header, ram_module_ptr_ptr = datamngr_module_pointers;
        ram_module_header_ptr < &datamngr_module_header[datamngr_module_count];
        ram_module_header_ptr++, ram_module_ptr_ptr++)
    {
        if((ram_module_header_ptr->data_len > 0) && (ram_module_ptr_ptr->data_ptr != NULL))
        {
            SAVE_CHECK(DrvMem_WriteData(datamngr_data_mem_hndl, offset, (U8*)ram_module_ptr_ptr->data_ptr, SIZE_IN_8BIT(ram_module_header_ptr->data_len)), "write mod data");
            offset += SIZE_IN_8BIT(ram_module_header_ptr->data_len);
        }
    }
    
    // FLASH DATA
    SAVE_CHECK(DrvMem_Flush(datamngr_data_mem_hndl), "flush data");
    
    // APPEND CRC
    crc = StdCrcGenerateCrc16IBM((U8*)DATA_ADDRESS, offset);
    SAVE_CHECK(DrvMem_WriteData(datamngr_data_mem_hndl, DATA_SIZE - SIZE_IN_8BIT(SIZEOF(crc)), (U8*)&crc, SIZE_IN_8BIT(SIZEOF(crc))), "write CRC");
    
    // FLUSH CRC
    SAVE_CHECK(DrvMem_Flush(datamngr_data_mem_hndl), "flush CRC");
    
    // VERIFY
    DataMngr_VerifyFlash();
    SAVE_CHECK(datamngr_flashdata_healthy, "verify");
    
    // PRINT CRC
    LOG_TRM("%sSAVE - CRC 0x%04h", PCSTR(datamngr_string), PU16(crc));
}
//------------------------------------------------------------------------------------------------//
static BOOL DataMngr_DataAndBackupEqual(void)
{
    U8* data_ptr    = (U8*)DATA_ADDRESS;
    U8* backup_ptr  = (U8*)BACKUP_ADDRESS;
    
    while(data_ptr < ((U8*)DATA_ADDRESS + DATA_SIZE))
    {
        if(*data_ptr != *backup_ptr)
        {
            return FALSE;
        }
        data_ptr++;
        backup_ptr++;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DataMngr_BackupAction(DATAMNGR_ACTION action)
{
    MEM_HNDL    target_mem_hndl;
    U8*         source_ptr;
#if (RAM_SIZE_FOR_BACKUP > 0)
    U32         data_len    = DATA_SIZE;
    U32         copy_len;
#endif
    
    // CHECK ACTION
    switch(action)
    {
    case DATAMNGR_ACTION_BACKUP:
        // CHECK IF DATA IS HEALTHY BEFORE BACKUP
        ACTION_CHECK(action, datamngr_flashdata_healthy, "data healthy");
        
        source_ptr      = (U8*)DATA_ADDRESS;
        target_mem_hndl = datamngr_backup_mem_hndl;
        break;
        
    case DATAMNGR_ACTION_RESTORE:
        source_ptr      = (U8*)BACKUP_ADDRESS;
        target_mem_hndl = datamngr_data_mem_hndl;
        break;
        
    default:
        // unsupported action for, this function
        return FALSE;
    }
    
    // CHECK BACKUP HANDLE
    ACTION_CHECK(action, datamngr_backup_mem_hndl != NULL, "backup hndl");
    
    // CHECK IF EQUAL ALREADY (NO COPY NEEDED)
    if(DataMngr_DataAndBackupEqual() == TRUE)
    {
        ACTION_CHECK(action, TRUE, "data equal");
        return TRUE;
    }
    
    // CLEAR TARGET
    ACTION_CHECK(action, DrvMem_Clear(target_mem_hndl), "clear target");
    
    // COPY DATA
    #if (RAM_SIZE_FOR_BACKUP > 0)
    {
        while(data_len)
        {
            // copy source -> RAM
            copy_len = MIN(data_len, RAM_SIZE_FOR_BACKUP);
            MEMCPY((VPTR)datamngr_backup_ram, source_ptr, copy_len);
            
            // write RAM -> target
            ACTION_CHECK(action, DrvMem_WriteData(target_mem_hndl, (DATA_SIZE - data_len), datamngr_backup_ram, copy_len), "copy part");
            
            // update vars
            data_len    -= copy_len;
            source_ptr  += copy_len;
        }
    }
    #else
    {
        ACTION_CHECK(action, DrvMem_WriteData(target_mem_hndl, 0, source_ptr, DATA_SIZE), "copy all");
    }
    #endif
    
    // FLUSH TARGET
    ACTION_CHECK(action, DrvMem_Flush(target_mem_hndl), "flush target");
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DataMngr_GetFlashDataPtr(U8 module_id, DATAMNGR_MODULE_HEADER* flash_module_header_ptr, U8** flash_data_start_ptr_ptr)
{
    U8* data_ptr;
    U8  flash_module_count;
    U8  i;
    U32 memdata_length;
    
    DrvMemSysFlash_GetInfo(datamngr_data_mem_hndl, (U32*)&data_ptr, &memdata_length);
    
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
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & (LOG_LEVEL_DEVELOP)
static BOOL DataMngr_PrintFeedback(BOOL is_success, DATAMNGR_ACTION action, STRING action_string)
{
    const STRING action_names[] = {"REGISTER", "VERIFY", "DUMP", "SAVE", "BACKUP", "RESTORE"};
    const STRING success_names[] = {"FAILED", "OK"};
    
    LOG_DEV("%s%-8s : %-16s - %s", PCSTR(datamngr_string), PCSTR(action_names[action]), PCSTR(action_string), PCSTR(success_names[is_success]));
    CoreLog_Flush();
    
    return is_success;
}
#endif
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_DataInfo(void)
{
    U8                      i;
    U8                      j;
    U8*                     data_ptr;
    DATAMNGR_MODULE_HEADER  flash_module_header;
    DATAMNGR_MODULE_HEADER* ram_module_header_ptr;
    const STRING            oknok[] = {"NOK", "OK", "NOK : LEN", "NOK : VERS", "NOK : N/A"};
    const STRING            noyes[] = {"no", "yes"};
    
    LOG_TRM("%sINFO (sw%d - v%d - %s)", PCSTR(datamngr_string), PU16(datamngr_header.software_number), PU8(datamngr_header.flash_struct_version), PCSTR(oknok[datamngr_flashdata_healthy]));
    LOG_TRM("idx|name      |mid|size|ver|l2d|state");
    for(i= 0; i < datamngr_module_count; i++)
    {
        ram_module_header_ptr = &datamngr_module_header[i];
        if(ram_module_header_ptr->data_len > 0)
        {
            j = 4;
            if(DataMngr_GetFlashDataPtr(ram_module_header_ptr->module_id, &flash_module_header, &data_ptr))
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
            
            LOG_TRM(" %-2d|%-10s|%3d|%4d|%3d|%3s|%s",
                    PU8(i),
#if (INCLUDE_INFO_STRING == 1)
                    PCSTR(datamngr_module_pointers[i].module_name),
#else
                    PCSTR("n/a"),
#endif
                    PU8(ram_module_header_ptr->module_id),
                    PU16(ram_module_header_ptr->data_len),
                    PU8(ram_module_header_ptr->data_version),
                    PCSTR(noyes[(BOOL)(datamngr_module_pointers[i].load_defaults_hook != NULL)]),
                    PCSTR(oknok[j]));
            CoreLog_Flush();
        }
    }
    LOG_TRM("Use: %d/%d", PU8(datamngr_module_count), PU8(datamngr_header.module_count));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_DataClear(void)
{
    switch(CoreTerm_GetArgumentAsU32(0))
    {
    case 0:
        CoreTerm_PrintFeedback(DrvMem_Clear(datamngr_data_mem_hndl));
        break;
    case 1:
        CoreTerm_PrintFeedback(DrvMem_Clear(datamngr_backup_mem_hndl));
        break;
    default:
        CoreTerm_PrintFailed();
        break;
    }
    DataMngr_VerifyFlash();
}
//------------------------------------------------------------------------------------------------//
static void Command_DataSave(void)
{
    StdDataMngr_SaveData();
    CoreTerm_PrintAcknowledge();
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
        mem_hndl = datamngr_data_mem_hndl;
        break;
    case 1:
        mem_hndl = datamngr_backup_mem_hndl;
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdDataMngr_Init(void)
{
    MODULE_INIT_ONCE();
    
    datamngr_data_mem_hndl      = DrvMemSysFlash_Register(DATA_ADDRESS,   DATA_SIZE, DATA_SECTOR_SIZE);
    datamngr_backup_mem_hndl    = DrvMemSysFlash_Register(BACKUP_ADDRESS, DATA_SIZE, BACKUP_SECTOR_SIZE);
    if(datamngr_data_mem_hndl == NULL)
    {
        LOG_ERR("Illegal memory handle");
    }
    
    datamngr_header.software_number         = PRODUCT_ID_NUMBER,
    datamngr_header.flash_struct_version    = FLASH_STRUCT_VERSION,
    datamngr_header.module_count            = DATAMNGR_MODULE_COUNT,
    
    datamngr_module_count       = 0;
    
    datamngr_state              = DATAMNGR_STATE_IDLE;
    #if (DELAY_TIME > 0)
    {
        datamngr_delay_task     = CoreTask_RegisterTask(DELAY_TIME, DataMngr_DelayTask, (VPTR)&datamngr_save_requested, 128, "DataMngr");
    }
    #endif
    datamngr_save_requested     = FALSE;
    datamngr_delayed_save_requested = FALSE;
    datamngr_backup_requested   = FALSE;
    datamngr_save_allowed       = TRUE;
    
    MEMSET((VPTR)datamngr_module_header,   0, sizeof(datamngr_module_header));
    MEMSET((VPTR)datamngr_module_pointers, 0, sizeof(datamngr_module_pointers));
    
    CoreTerm_RegisterCommand("DataInfo", "DATAMNGR info", 0, Command_DataInfo, TRUE);
    CoreTerm_RegisterCommand("DataClear", "DATAMNGR clear data", 1, Command_DataClear, TRUE);
    CoreTerm_RegisterCommand("DataSave", "DATAMNGR save data", 0, Command_DataSave, TRUE);
    CoreTerm_RegisterCommand("DataDefaults", "DATAMNGR load default data in RAM", 0, Command_DataDefaults, TRUE);
    CoreTerm_RegisterCommand("DataRaw", "DATAMNGR print raw data", 1, Command_DataRaw, FALSE);
    
    // VERIFY DATA
    DataMngr_VerifyFlash();
    if(datamngr_flashdata_healthy == TRUE)
    {
        // backup if needed
        DataMngr_BackupAction(DATAMNGR_ACTION_BACKUP);
    }
    else
    {
        // restore backup and run check again
        DataMngr_BackupAction(DATAMNGR_ACTION_RESTORE);
        DataMngr_VerifyFlash();
    }
    
    datamngr_ondefaultsloaded_hook = NULL;
    
    Core_RegisterModuleHandler(DataMngr_Handler);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_RegisterOnDefaultsLoadedHook(EVENT_CALLBACK ondefaultsloaded_hook)
{
    MODULE_CHECK();
    
    datamngr_ondefaultsloaded_hook = ondefaultsloaded_hook;
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_Register(DATAMNGR_MODULE_ID    module_id,
                          DATAMNGR_CONVERT_DATA convert_data_hook,
                          DATAMNGR_LOAD_DEFAULTS load_defaults_hook,
                          VPTR  data_ptr,
                          U16   data_len,
                          U8    data_version,
                          BOOL  allow_loaddefaults,
                          STRING module_name)
{
    U8*                         flash_data_start_ptr;
    DATAMNGR_MODULE_HEADER      flash_module_header;
    DATAMNGR_MODULE_HEADER*     ram_module_header_ptr;
    DATAMNGR_MODULE_POINTERS*   ram_module_ptr_ptr;
    U32                         offset;
    
    MODULE_CHECK();
    
    LOG_DEV("%sREGISTER module %d", PCSTR(datamngr_string),PU8(module_id));
    
    // VERIFY REGISTRATION
    REGISTER_CHECK(datamngr_module_count < DATAMNGR_MODULE_COUNT, "overrun check");
    REGISTER_CHECK((data_ptr != NULL) && (data_len != 0), "data info");
    
    // CHECK FOR DOUBLE REGISTRATION + CALCULATE REGISTERED SIZE
    offset = SIZE_IN_8BIT(SIZEOF(DATAMNGR_HEADER)) + DATAMNGR_MODULE_COUNT * SIZE_IN_8BIT(SIZEOF(DATAMNGR_MODULE_HEADER));  // HDR
    for(ram_module_header_ptr = datamngr_module_header; ram_module_header_ptr < &datamngr_module_header[datamngr_module_count]; ram_module_header_ptr++)
    {
        if(ram_module_header_ptr->module_id == (U8)module_id)
        {
            REGISTER_CHECK(FALSE, "unique id");
        }
        // data size of registered modules
        offset += SIZE_IN_8BIT(ram_module_header_ptr->data_len);
    }
    REGISTER_CHECK(TRUE, "unique id");
    offset += SIZE_IN_8BIT(data_len) + 2;   // data size of new module + crc
    REGISTER_CHECK(offset <= DATA_SIZE, "bounds");
    
    // REGISTER DATA
    ram_module_header_ptr->data_len         = data_len;
    ram_module_header_ptr->data_version     = data_version;
    ram_module_header_ptr->module_id        = (U8)module_id;
    
    ram_module_ptr_ptr = &datamngr_module_pointers[datamngr_module_count];
    ram_module_ptr_ptr->data_ptr            = data_ptr;
    ram_module_ptr_ptr->convert_data_hook   = convert_data_hook;
    ram_module_ptr_ptr->load_defaults_hook  = NULL;
    if(allow_loaddefaults == TRUE)
    {
        ram_module_ptr_ptr->load_defaults_hook  = load_defaults_hook;
    }
    #if (INCLUDE_INFO_STRING == 1)
    {
        ram_module_ptr_ptr->module_name         = module_name;
    }
    #endif
    
    // LOAD DEFAULTS
    if(load_defaults_hook != NULL)
    {
        load_defaults_hook();
    }
    
    // TRY TO RETRIEVE FLASH DATA
    if((datamngr_flashdata_healthy == TRUE) && DataMngr_GetFlashDataPtr(module_id, &flash_module_header, &flash_data_start_ptr))
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
    MODULE_CHECK();
    
    datamngr_save_requested = TRUE;
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_SaveDataDelayed(void)
{
    MODULE_CHECK();
    
    datamngr_delayed_save_requested = TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdDataMngr_IsDataSaved(void)
{
    MODULE_CHECK();

    // data is saved if state is IDLE and no pending requests
    return (BOOL)((datamngr_save_requested == FALSE) &&
                  (datamngr_delayed_save_requested == FALSE) &&
                  (datamngr_state == DATAMNGR_STATE_IDLE));
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_LoadDefaults(void)
{
    U8      i;
    
    MODULE_CHECK();
    
    for(i = 0; i < datamngr_module_count; i++)
    {
        if(datamngr_module_pointers[i].load_defaults_hook != NULL)
        {
            datamngr_module_pointers[i].load_defaults_hook();
        }
    }
    
    if(datamngr_ondefaultsloaded_hook != NULL)
    {
        datamngr_ondefaultsloaded_hook();
    }
    
#if (AUTOSAVE_AFTER_LOADDEFAULTS == 1)
    StdDataMngr_SaveData();
#endif
}
//------------------------------------------------------------------------------------------------//
void StdDataMngr_AllowSaveData(BOOL allow)
{
    MODULE_CHECK();
    
    datamngr_save_allowed = allow;
}
//------------------------------------------------------------------------------------------------//
U16 StdDatMngr_GetRawDataPart(U16 offset, U8* buffer_ptr, U16 max_length)
{
    U16 copy_len;
    
    if(offset >= DATA_SIZE)
    {
        return 0;
    }
    
    copy_len = MIN(max_length, DATA_SIZE - offset);
    MEMCPY(buffer_ptr, (U8*)DATA_ADDRESS + offset, copy_len);
    
    return copy_len;
}
//------------------------------------------------------------------------------------------------//
BOOL StdDatMngr_SetRawDataPart(U16 offset, U8* buffer_ptr, U16 buffer_len)
{
    // CHECK BOUNDARIES
    DUMP_CHECK((offset + buffer_len) <= DATA_SIZE, "bounds");
    
    // IF FIRST : CLEAR
    if(offset == 0)
    {
        datamngr_flashdata_healthy = FALSE;
        DUMP_CHECK(DrvMem_Clear(datamngr_data_mem_hndl), "clear data");
    }
    
    // WRITE PART
    DUMP_CHECK(DrvMem_WriteData(datamngr_data_mem_hndl, offset, buffer_ptr, buffer_len), "write part");

    // IF LAST: FLUSH & VERIFY
    if((offset + buffer_len) == DATA_SIZE)
    {
        DUMP_CHECK(DrvMem_Flush(datamngr_data_mem_hndl), "flush data");
        DataMngr_VerifyFlash();
        DUMP_CHECK(datamngr_flashdata_healthy, "verify");
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdDataMngr_GetInfo(DATAMNGR_INFO info, U32* value_ptr)
{
    MODULE_CHECK();
    
    switch(info)
    {
    case DATAMNGR_INFO_DATA_ADDRESS:
        *value_ptr = DATA_ADDRESS;
        break;
        
    case DATAMNGR_INFO_DATA_SIZE:
        *value_ptr = DATA_SIZE;
        break;
        
    default:
        return FALSE;
    }
    return TRUE;
}
//================================================================================================//
