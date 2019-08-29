//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of chip independent layer for the Bluetooth LE upload service
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMMBTLEBOOTLOADERSERVICE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMBTLEBOOTLOADERSERVICE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMMBTLEBOOTLOADERSERVICE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMBTLEBOOTLOADER_PRODUCT_NUMBER
    #error "COMMBTLEBOOTLOADER_PRODUCT_NUMBER not defined in AppConfig"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMBTLEBOOTLOADER_MAX_IMAGE_LENGTH
    #error "COMMBTLEBOOTLOADER_MAX_IMAGE_LENGTH not defined in AppConfig"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMBTLEBOOTLOADER_DESTINATION_ADDRESS
    #error "COMMBTLEBOOTLOADER_DESTINATION_ADDRESS not defined in AppConfig"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMBTLEBOOTLOADER_RX_Q_SIZE
	#error "COMMBTLEBOOTLOADER_RX_Q_SIZE not defined in AppConfig"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// STD
#include "crc\StdCrc.h"
#include "boot\StdBootPsiLdr.h"
#include "encrypt\StdEncrypt.h"

// COM
#include "CommBtleBootloaderService.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define PSILDR_BUFFER_SIZE              20
//------------------------------------------------------------------------------------------------//
#define UPLOAD_DATA_BUFFER_SIZE         60
#define CTRL_DATA_BUFFER_SIZE           20
#define STATUS_DATA_BUFFER_SIZE         20
//------------------------------------------------------------------------------------------------//
#define INVALID_SW                      0xFFFF
#define INVALID_RESPONSE                0xFF
#define STATUS_DATA_LENGTH              20
//------------------------------------------------------------------------------------------------//
#define STATUS_RESPONS_OK               0x01
#define STATUS_RESPONS_NOT_OK           0x02
#define STATUS_RESPONS_OTHER            0x03
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    UPLOAD_IDLE                 = 0,
    UPLOAD_BUSY                 = 1,
    UPLOAD_DONE                 = 2
}
UPLOAD_STATE;

typedef enum
{
    OPCODE_START_DFU           = 1,
    OPCODE_RECEIVE_IMAGE_DATA  = 2,
    OPCODE_VALIDATE_DATA       = 3,
    OPCODE_ACTIVATE_IMAGE      = 4,
    OPCODE_INVALID             = 0xFF
}
UPLOAD_PROCEDURE;

typedef struct
{
    PRODUCT_VERSION             version;
    U32                         total_length;
    U32                         received_length;
}
NEW_SOFTWARE_STRUCT;

typedef enum
{
    VALIDATION_NOT_DONE         = 0,
    VALIDATION_NOT_OK           = 1,
    VALIDATION_OK               = 2
}
VALIDATION_STATE;

typedef struct
{
    UPLOAD_STATE                state;
    UPLOAD_PROCEDURE            last_procedure;
    NEW_SOFTWARE_STRUCT         new_software;
    MEM_HNDL                    mem_hndl;
    VALIDATION_STATE            validation_state;
}
UPLOAD_CTRL_STRUCT;

typedef enum
{
    CTRL_DATA_IDLE             = 0,
    CTRL_DATA_PENDING          = 1
}
CTRL_DATA_STATE;

typedef struct
{
    PRODUCT_VERSION             version;
    U16                         crc;
}
SWHEADER;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_BtleUploadInfo(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
// Upload characteristic
static Q_HNDL                   comm_upload_data_q_hndl;
static U8*                      comm_upload_data_buffer_ptr;
static BOOL                     comm_upload_data_allowed;
// Ctrl characteristic
static U8*                      comm_ctrl_data_buffer_ptr;
static U8                       comm_ctrl_data_length;
static CTRL_DATA_STATE          comm_ctrl_data_state;
// Status characteristic
static U8*                      comm_status_data_buffer_ptr;
static U8                       comm_status_data_length;
// Hooks
static COMM_IS_TX_FREE_HOOK     comm_is_tx_free_hook;
static COMM_NOTIFY_CTRL_HOOK    comm_notify_ctrl_hook;
static EVENT_CALLBACK           comm_activate_image_hook;

// Misc
static UPLOAD_CTRL_STRUCT       comm_upload_ctrl;
static U8*                      comm_psildr_buffer;
static BOOL                     comm_cancel_upload;

// Strings
static const STRING             module_name = "[DFU] ";
static const STRING             procedure_names[] = {"START_DFU",
                                                     "RECEIVE_IMAGE_DATA",
                                                     "VALIDATE_IMAGE_DATA",
                                                     "ACTIVATE_IMAGE",
                                                     "INVALID"};
static const STRING             state_names[] = {"UPLOAD_IDLE", 
                                                 "UPLOAD_BUSY",
                                                 "UPLOAD_DONE"};
static const STRING             validation_names[] = {"NOT_DONE",
                                                      "NOT_OK",
                                                      "OK"};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static STRING GetProcedureString(UPLOAD_PROCEDURE procedure)
{
    switch(procedure)
    {
    case OPCODE_START_DFU:
        return procedure_names[0];
    case OPCODE_RECEIVE_IMAGE_DATA:
        return procedure_names[1];
    case OPCODE_VALIDATE_DATA:
        return procedure_names[2];
    case OPCODE_ACTIVATE_IMAGE:
        return procedure_names[3];
    case OPCODE_INVALID:
    default:
        return procedure_names[4];
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL AreSoftwaresEqual(PRODUCT_VERSION first_sw, PRODUCT_VERSION second_sw)
{
    if((first_sw.product_number == second_sw.product_number) &&
       (first_sw.major_revision == second_sw.major_revision) &&
       (first_sw.minor_revision == second_sw.minor_revision) &&
       (first_sw.test_revision  == second_sw.test_revision))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL IsPsiLdrHeaderValid(BOOTLDR_APP_RAW_HEADER* header_ptr)
{
    if((header_ptr->start_of_header == 0x48) &&
       (header_ptr->header_crc == StdCrcGenerateCrc16IBM((U8*)header_ptr, PSILDR_HEADER_SIZE - 2)) &&
       (header_ptr->header_version == 1) &&
       (header_ptr->destination_address == COMMBTLEBOOTLOADER_DESTINATION_ADDRESS) &&
       (header_ptr->data_length <= COMMBTLEBOOTLOADER_MAX_IMAGE_LENGTH))
    {
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void SetUploadDataNotAllowed(void)
{
    comm_upload_data_allowed = FALSE;
    CoreQ_DropAll(comm_upload_data_q_hndl);
}
//------------------------------------------------------------------------------------------------//
static void EnterState(UPLOAD_STATE state)
{
    LOG_TRM("%sEnter state %s", PCSTR(module_name), PCSTR(state_names[state]));
    comm_upload_ctrl.state = state;
}
//------------------------------------------------------------------------------------------------//
static void OnStatusMessageSent(U8* data_ptr, U8 length)
{
    LOG_TRM("%sStatus message sent %02h", PCSTR(module_name), PU8A(data_ptr, length));
    
    if(data_ptr[0] == STATUS_RESPONS_OK)
    {
        switch(data_ptr[1])
        {
        case OPCODE_START_DFU:
            EnterState(UPLOAD_BUSY);
            break;
            
        case OPCODE_RECEIVE_IMAGE_DATA:
            comm_upload_data_allowed = TRUE;
            break;
            
        case OPCODE_VALIDATE_DATA:
            break;
            
        case OPCODE_ACTIVATE_IMAGE:
            if(comm_activate_image_hook != NULL)
            {
                comm_activate_image_hook();
                return;
            }
            break;
            
        case OPCODE_INVALID:
            break;
            
        default:
            return;
        }
        
        comm_upload_ctrl.last_procedure = (UPLOAD_PROCEDURE)data_ptr[1];
    }
    else if(data_ptr[0] == STATUS_RESPONS_OTHER)
    {
        if((data_ptr[1] != OPCODE_START_DFU) && (data_ptr[1] != OPCODE_RECEIVE_IMAGE_DATA))
        {
            comm_upload_ctrl.last_procedure = OPCODE_INVALID;
            return;
        }
        comm_upload_ctrl.last_procedure = (UPLOAD_PROCEDURE)data_ptr[1];
    }
}
//------------------------------------------------------------------------------------------------//
static void RespondOk(UPLOAD_PROCEDURE opcode)
{
    LOG_DBG("%sResponse OK", PCSTR(module_name));
    
    comm_status_data_buffer_ptr[0] = STATUS_RESPONS_OK;
    comm_status_data_buffer_ptr[1] = opcode;
    comm_status_data_length = 2;
}
//------------------------------------------------------------------------------------------------//
static void RespondNotOk(UPLOAD_PROCEDURE opcode)
{
    LOG_DBG("%sResponse NOT_OK", PCSTR(module_name));
    
    comm_status_data_buffer_ptr[0] = STATUS_RESPONS_NOT_OK;
    comm_status_data_buffer_ptr[1] = opcode;
    comm_status_data_length = 2;
    
    comm_upload_ctrl.last_procedure = OPCODE_INVALID;
}
//------------------------------------------------------------------------------------------------//
static void RespondReceivedLength(void)
{
    LOG_DBG("%sResponse RECEIVED_LENGTH", PCSTR(module_name));
    
    comm_status_data_buffer_ptr[0] = 0x03;
    comm_status_data_buffer_ptr[1] = OPCODE_START_DFU;
    CoreConvert_U32ToU8Array(comm_upload_ctrl.new_software.received_length, &comm_status_data_buffer_ptr[2]);
    comm_status_data_length = 6;
}
//------------------------------------------------------------------------------------------------//
static void RespondImageReceived(void)
{
    LOG_DBG("%sImage received", PCSTR(module_name));
    
    comm_status_data_buffer_ptr[0] = 0x03;
    comm_status_data_buffer_ptr[1] = OPCODE_RECEIVE_IMAGE_DATA;
    comm_status_data_length        = 2;
}
//------------------------------------------------------------------------------------------------//
static void HandleStartDfuMessage(U8* data_ptr, U8 length)
{
    PRODUCT_VERSION new_software;
    U32 new_software_length;
    
    LOG_DBG("%sHandleStartDfuMessage - %02h", PCSTR(module_name), PU8A(data_ptr, length));
    
    if((data_ptr == NULL) || (length != 13))
    {
        LOG_WRN("Invalid input params");
        RespondNotOk(OPCODE_START_DFU);
        return;
    }
    
    new_software.test_revision  = CoreConvert_U8ArrayToU16(&data_ptr[1]);
    new_software.minor_revision = CoreConvert_U8ArrayToU16(&data_ptr[3]);
    new_software.major_revision = CoreConvert_U8ArrayToU16(&data_ptr[5]);
    new_software.product_number = CoreConvert_U8ArrayToU16(&data_ptr[7]);
    new_software_length         = CoreConvert_U8ArrayToU32(&data_ptr[9]);
    
    LOG_TRM("%s%s %d.%d.%d.%d - %d bytes", PCSTR(module_name),
                                           PCSTR(GetProcedureString(OPCODE_START_DFU)),
                                           PU16(new_software.product_number),
                                           PU16(new_software.major_revision),
                                           PU16(new_software.minor_revision),
                                           PU16(new_software.test_revision),
                                           PU32(new_software_length));
    
    // Check if inputs are valid (correct SW product & length < max_length)
    if((new_software.product_number != COMMBTLEBOOTLOADER_PRODUCT_NUMBER) ||
       (new_software_length > COMMBTLEBOOTLOADER_MAX_IMAGE_LENGTH))
    {
        RespondNotOk(OPCODE_START_DFU);
        return;
    }
    
    comm_upload_ctrl.validation_state = VALIDATION_NOT_DONE;
    
    // Has downloading this version already been started?
    if((comm_upload_ctrl.state == UPLOAD_BUSY) && AreSoftwaresEqual(new_software, comm_upload_ctrl.new_software.version))
    {
        LOG_TRM("%s%s Upload already started", PCSTR(module_name), PCSTR(GetProcedureString(OPCODE_START_DFU)));
        
        if(new_software_length != comm_upload_ctrl.new_software.total_length)
        {
            LOG_TRM("%s%s lengths differ: %d - %d", PCSTR(module_name),
                                                    PCSTR(GetProcedureString(OPCODE_START_DFU)),
                                                    PU32(new_software_length),
                                                    PU32(comm_upload_ctrl.new_software.total_length));
            RespondNotOk(OPCODE_START_DFU);
        }
        else
        {
            // SW version and length are OK, respond with received length
            LOG_TRM("%s%s Already received: %d bytes", PCSTR(module_name),
                                                       PCSTR(GetProcedureString(OPCODE_START_DFU)),
                                                       PU32(comm_upload_ctrl.new_software.received_length));
            RespondReceivedLength();
        }
        return;
    }
    
    // Start new upload
    comm_upload_ctrl.new_software.version         = new_software;
    comm_upload_ctrl.new_software.total_length    = new_software_length;
    comm_upload_ctrl.new_software.received_length = 0;
    
    // Clear backup space
    DrvMem_Clear(comm_upload_ctrl.mem_hndl);
    
    RespondOk(OPCODE_START_DFU);
}
//------------------------------------------------------------------------------------------------//
static void HandleReceiveImageDataMessage(U8* data_ptr, U8 length)
{
    LOG_DBG("%sHandleReceiveImageDataMessage - %02h", PCSTR(module_name), PU8A(data_ptr, length));
    
    if((data_ptr == NULL) || (length != 5))
    {
        LOG_WRN("Invalid input params");
        RespondNotOk(OPCODE_RECEIVE_IMAGE_DATA);
        return;
    }
    
    // Is sequence of procedures respected?
    if((comm_upload_ctrl.state != UPLOAD_BUSY) || (comm_upload_ctrl.last_procedure != OPCODE_START_DFU))
    {
        LOG_TRM("%sInvalid state/last procedure: %s, %s", PCSTR(module_name),
                                                          PCSTR(state_names[comm_upload_ctrl.state]),
                                                          PCSTR(GetProcedureString(comm_upload_ctrl.last_procedure)));
        RespondNotOk(OPCODE_RECEIVE_IMAGE_DATA);
        return;
    }
    
    // Is the received start adress equal to the expected start address?
    if(comm_upload_ctrl.new_software.received_length != CoreConvert_U8ArrayToU32(&data_ptr[1]))
    {
        LOG_TRM("%s%s lengths differ", PCSTR(module_name), PCSTR(GetProcedureString(OPCODE_RECEIVE_IMAGE_DATA)));
        RespondNotOk(OPCODE_RECEIVE_IMAGE_DATA);
        return;
    }
    
    RespondOk(OPCODE_RECEIVE_IMAGE_DATA);
}
//------------------------------------------------------------------------------------------------//
static void HandleValidateDataMessage(U8* data_ptr, U8 msg_length)
{
    BOOTLDR_APP_RAW_HEADER psildr_header;
    SWHEADER flash_header;
    U32 offset;
    U16 crc_encrypted;
    U16 crc_decrypted;
    U8  buffer_length;
    U8  i;
    
    LOG_DBG("%sHandleValidateDataMessage - %02h", PCSTR(module_name), PU8A(data_ptr, msg_length));
    
    if(msg_length > 1)
    {
        LOG_WRN("Invalid input params");
        RespondNotOk(OPCODE_VALIDATE_DATA);
        return;
    }
    
    // Is sequence of procedures respected?
    if((comm_upload_ctrl.new_software.received_length != comm_upload_ctrl.new_software.total_length) ||
       (comm_upload_ctrl.last_procedure != OPCODE_RECEIVE_IMAGE_DATA))
    {
        LOG_TRM("%sInvalid state/last procedure: %s, %s", PCSTR(module_name),
                                                          PCSTR(state_names[comm_upload_ctrl.state]),
                                                          PCSTR(GetProcedureString(comm_upload_ctrl.last_procedure)));
        RespondNotOk(OPCODE_VALIDATE_DATA);
        return;
    }
    
    // If validated, write flash header
    DrvMem_ReadData(comm_upload_ctrl.mem_hndl, BTLE_BOOTLOADER_PSILDR_OFFSET, (U8*)&psildr_header, PSILDR_HEADER_SIZE);
    if(IsPsiLdrHeaderValid(&psildr_header) == FALSE)
    {
        LOG_TRM("%sInvalid header", PCSTR(module_name));
        RespondNotOk(OPCODE_VALIDATE_DATA);
        return;
    }
    
    crc_encrypted = 0xFFFF;
    crc_decrypted = 0xFFFF;
    for(offset = 0; offset < psildr_header.data_length; offset += buffer_length)
    {
        buffer_length = MIN(PSILDR_BUFFER_SIZE, (psildr_header.data_length - offset));
        
        System_KickDog();
        DrvMem_ReadData(comm_upload_ctrl.mem_hndl, PSILDR_HEADER_SIZE + BTLE_BOOTLOADER_PSILDR_OFFSET + offset, comm_psildr_buffer, buffer_length);
        
        crc_encrypted = StdCrcGenerateCrc16IBMPart(comm_psildr_buffer, (U32)buffer_length, crc_encrypted);
        
        for(i = 0; i < buffer_length; i++)
        {
            comm_psildr_buffer[i] = StdEncryptDecodeData((U16)comm_psildr_buffer[i]);
        }
        
        crc_decrypted = StdCrcGenerateCrc16IBMPart(comm_psildr_buffer, (U32)buffer_length, crc_decrypted);
    }
    
    // Check on decoded crc
    if((crc_encrypted == psildr_header.data_crc) &&
       (crc_decrypted == psildr_header.reserved_bytes))
    {
        flash_header.version = comm_upload_ctrl.new_software.version;
        flash_header.crc     = StdCrcGenerateCrc16IBM((U8*)&flash_header, SIZEOF(SWHEADER) - 2);
        if(DrvMem_WriteData(comm_upload_ctrl.mem_hndl, 0, (U8*)&flash_header, SIZEOF(SWHEADER)) &&
           DrvMem_Flush(comm_upload_ctrl.mem_hndl) &&
           DrvMem_VerifyData(comm_upload_ctrl.mem_hndl, 0, (U8*)&flash_header, SIZEOF(SWHEADER)))
        {
            LOG_DBG("%sValidation OK", PCSTR(module_name));
            comm_upload_ctrl.validation_state = VALIDATION_OK;
            EnterState(UPLOAD_DONE);
        }
    }
    else
    {
        LOG_DBG("%sValidation NOT_OK", PCSTR(module_name));
        comm_upload_ctrl.validation_state = VALIDATION_NOT_OK;
        
        MEMSET(&comm_upload_ctrl.new_software.version, 0xFF, SIZEOF(comm_upload_ctrl.new_software.version));
        comm_upload_ctrl.new_software.received_length = 0;
        comm_upload_ctrl.new_software.total_length    = 0;
    }
    
    if(comm_upload_ctrl.validation_state == VALIDATION_OK)
    {
        RespondOk(OPCODE_VALIDATE_DATA);
    }
    else
    {
        RespondNotOk(OPCODE_VALIDATE_DATA);
    }
}
//------------------------------------------------------------------------------------------------//
static void HandleActivateImageMessage(U8* data_ptr, U8 length)
{
    LOG_DBG("[DFU] HandleActivateImageMessage - %02h", PU8A(data_ptr, length));
    
    if(length > 1)
    {
        LOG_WRN("Invalid input params");
        RespondNotOk(OPCODE_ACTIVATE_IMAGE);
        return;
    }
    
    if((comm_upload_ctrl.state != UPLOAD_DONE) && 
       (comm_upload_ctrl.last_procedure != OPCODE_VALIDATE_DATA) && 
       (comm_upload_ctrl.validation_state != VALIDATION_OK))
    {
        RespondNotOk(OPCODE_ACTIVATE_IMAGE);
        return;
    }
    
    LOG_TRM("%s%s", PCSTR(module_name), PCSTR(GetProcedureString(OPCODE_ACTIVATE_IMAGE)));
    RespondOk(OPCODE_ACTIVATE_IMAGE);
}
//------------------------------------------------------------------------------------------------//
static void HandleCtrlMessage(U8* data_ptr, U8 length)
{
    if((data_ptr == NULL) || (length == 0))
    {
        LOG_WRN("Invalid inputs");
        return;
    }
    
    switch(data_ptr[0])
    {
    case OPCODE_START_DFU:
        HandleStartDfuMessage(data_ptr, length);
        break;
        
    case OPCODE_RECEIVE_IMAGE_DATA:
        HandleReceiveImageDataMessage(data_ptr, length);
        break;
        
    case OPCODE_VALIDATE_DATA:
        HandleValidateDataMessage(data_ptr, length);
        break;
        
    case OPCODE_ACTIVATE_IMAGE:
        HandleActivateImageMessage(data_ptr, length);
        break;
        
    case OPCODE_INVALID:
    default:
        LOG_TRM("%sInvalid opcode", PCSTR(module_name));
        return;
    }
}
//------------------------------------------------------------------------------------------------//
static void HandleUploadDataMessage(U8* data_ptr, U8 length)
{
    LOG_DEV("%sData: %02h", PCSTR(module_name), PU8A(data_ptr, length));
    
    if(DrvMem_WriteData(comm_upload_ctrl.mem_hndl, BTLE_BOOTLOADER_PSILDR_OFFSET + comm_upload_ctrl.new_software.received_length, data_ptr, length))
    {
        comm_upload_ctrl.new_software.received_length += length;
    }
    else
    {
        LOG_TRM("%sWriting upload data failed", PCSTR(module_name));
        RespondNotOk(OPCODE_RECEIVE_IMAGE_DATA);
        SetUploadDataNotAllowed();
        return;
    }
    
    if(comm_upload_ctrl.new_software.received_length >= comm_upload_ctrl.new_software.total_length)
    {
        // All bytes received
        LOG_TRM("[DFU] All bytes received %d - %d", PU32(comm_upload_ctrl.new_software.received_length),
                                                    PU32(comm_upload_ctrl.new_software.total_length));
        DrvMem_Flush(comm_upload_ctrl.mem_hndl);
        SetUploadDataNotAllowed();
        RespondImageReceived();
    }
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_BtleUploadInfo(void)
{
    LOG_TRM("State:                 %s", PCSTR(state_names[comm_upload_ctrl.state]));
    if(comm_upload_ctrl.last_procedure <= 4)
    {
        LOG_TRM("Last procedure:        %s", PCSTR(procedure_names[comm_upload_ctrl.last_procedure - 1]));
    }
    else
    {
        LOG_TRM("Last procedure:        %s", PCSTR(procedure_names[4]));
    }
    LOG_TRM("New software:");       
    LOG_TRM(" - version:            %d.%d.%d.%d", PU16(comm_upload_ctrl.new_software.version.product_number),
                                                  PU16(comm_upload_ctrl.new_software.version.major_revision),
                                                  PU16(comm_upload_ctrl.new_software.version.minor_revision),
                                                  PU16(comm_upload_ctrl.new_software.version.test_revision));
    LOG_TRM(" - total length:       %d", PU32(comm_upload_ctrl.new_software.total_length));
    LOG_TRM(" - received length:    %d [%d %%]", PU32(comm_upload_ctrl.new_software.received_length), PU8((comm_upload_ctrl.new_software.received_length * 100)/comm_upload_ctrl.new_software.total_length));
    LOG_TRM("Validation state:      %s", PCSTR(validation_names[comm_upload_ctrl.validation_state]));
    LOG_TRM("Mem id:                %d", PU8(comm_upload_ctrl.mem_hndl->mem_id));
    LOG_TRM("Upload busy:           %d", PU8(CommBtleBootloaderService_IsUploadBusy()));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommBtleBootloaderService_Init(MEM_HNDL mem_hndl, const U16* encryption_table_ptr)
{
    StdEncryptInitDecoder((U16*)encryption_table_ptr);
    
    comm_upload_data_q_hndl           = CoreQ_Register(COMMBTLEBOOTLOADER_RX_Q_SIZE, 1, "Upload data q");
    comm_upload_data_buffer_ptr       = CoreBuffer_CreateStaticU8(UPLOAD_DATA_BUFFER_SIZE, "Btle upload");
    comm_upload_data_allowed          = FALSE;
    comm_ctrl_data_buffer_ptr         = CoreBuffer_CreateStaticU8(CTRL_DATA_BUFFER_SIZE, "Btle ctrl");
    comm_ctrl_data_length             = 0;
    comm_ctrl_data_state              = CTRL_DATA_IDLE;
    comm_status_data_buffer_ptr       = CoreBuffer_CreateStaticU8(STATUS_DATA_BUFFER_SIZE, "Btle status");
    comm_status_data_length           = 0;
    comm_psildr_buffer                = CoreBuffer_CreateStaticU8(PSILDR_BUFFER_SIZE, "psildr data buffer");
    comm_cancel_upload                = FALSE;
    
    MEMSET(&comm_upload_ctrl, 0 , SIZEOF(UPLOAD_CTRL_STRUCT));
    comm_upload_ctrl.mem_hndl         = mem_hndl;
    comm_upload_ctrl.last_procedure   = OPCODE_INVALID;
    comm_upload_ctrl.validation_state = VALIDATION_NOT_DONE;
    
    comm_is_tx_free_hook              = NULL;
    comm_notify_ctrl_hook             = NULL;
    comm_activate_image_hook          = NULL;
    
    EnterState(UPLOAD_IDLE);
    
#if (TERM_LEVEL > TERM_LEVEL_NONE)
    CoreTerm_RegisterCommand("BtleUploadInfo", "Print version", 0, Command_BtleUploadInfo, TRUE);
#endif
}
//------------------------------------------------------------------------------------------------//
void CommBtleBootloaderService_Handler(void)
{
    U8 length;
    
    if(comm_cancel_upload)
    {
        LOG_WRN("%sCancel upload", PCSTR(module_name));
        RespondNotOk(OPCODE_RECEIVE_IMAGE_DATA);
        SetUploadDataNotAllowed();
        comm_ctrl_data_state     = CTRL_DATA_IDLE;
        comm_cancel_upload       = FALSE;
    }
    
    // Check if incoming message is still pending
    if(comm_ctrl_data_state == CTRL_DATA_PENDING)
    {
        if(comm_status_data_length == 0)
        {
            // No outgoing message is pending, process the new ctrl message
            HandleCtrlMessage(comm_ctrl_data_buffer_ptr, comm_ctrl_data_length);
        }
    }
    
    if(comm_status_data_length > 0)
    {
        if(((comm_is_tx_free_hook != NULL) && (comm_is_tx_free_hook() == TRUE)) &&
           ((comm_notify_ctrl_hook != NULL) && comm_notify_ctrl_hook(comm_status_data_buffer_ptr, comm_status_data_length)))
        {
            OnStatusMessageSent(comm_status_data_buffer_ptr, comm_status_data_length);
            comm_status_data_length = 0;
            comm_ctrl_data_state    = CTRL_DATA_IDLE;
        }
    }
    
    while(comm_upload_data_allowed && (CoreQ_GetCount(comm_upload_data_q_hndl) > 0))
    {
        System_KickDog();
        
        length = MIN(UPLOAD_DATA_BUFFER_SIZE, CoreQ_GetCount(comm_upload_data_q_hndl));
        CoreQ_Read(comm_upload_data_q_hndl, comm_upload_data_buffer_ptr, length);
        HandleUploadDataMessage(comm_upload_data_buffer_ptr, length);
    }
}
//------------------------------------------------------------------------------------------------//
void CommBtleBootloaderService_RegisterCtrlPointData(U8* data_ptr, U16 length)
{
    // When new ctrl point information is received,
    // receiving upload data isn't allowed any more
    SetUploadDataNotAllowed();
    
    switch(comm_ctrl_data_state)
    {
    case CTRL_DATA_IDLE:
        if(length <= CTRL_DATA_BUFFER_SIZE)
        {
            comm_ctrl_data_state = CTRL_DATA_PENDING;
            // Assumption is made that every ctrl command is transmitted in 1 packet
            MEMCPY(comm_ctrl_data_buffer_ptr, data_ptr, length);
            comm_ctrl_data_length = length;
        }
        break;
        
    case CTRL_DATA_PENDING:
        LOG_TRM("%sPrevious ctrl data still pending", PCSTR(module_name));
        break;
        
    default:
        LOG_WRN("Invalid state");
        return;
    }
}
//------------------------------------------------------------------------------------------------//
void CommBtleBootloaderService_RegisterUploadData(U8* data_ptr, U16 length)
{
    if((comm_ctrl_data_state == CTRL_DATA_PENDING) || 
       (comm_upload_data_allowed == FALSE) || 
       (comm_cancel_upload == TRUE) ||
       (comm_upload_ctrl.state != UPLOAD_BUSY))
    {
        LOG_DBG("%sData while in %s", PCSTR(module_name), PCSTR(state_names[comm_upload_ctrl.state]));
        return;
    }
    
    if(CoreQ_Write(comm_upload_data_q_hndl, data_ptr, length) == FALSE)
    {
        // Notify the app that you've lost control of data
        comm_cancel_upload = TRUE;
        LOG_WRN("Not enough space");
    }
}
//------------------------------------------------------------------------------------------------//
void CommBtleBootloaderService_RegisterIsTxFreeHook(COMM_IS_TX_FREE_HOOK hook)
{
    comm_is_tx_free_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommBtleBootloaderService_RegisterNotifyCtrlHook(COMM_NOTIFY_CTRL_HOOK hook)
{
    comm_notify_ctrl_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommBtleBootloaderService_RegisterActivateImageHook(EVENT_CALLBACK hook)
{
    comm_activate_image_hook = hook;
}
//------------------------------------------------------------------------------------------------//
BOOL CommBtleBootloaderService_IsUploadBusy(void)
{
	return (BOOL)(comm_upload_ctrl.state != UPLOAD_IDLE);
}
//================================================================================================//