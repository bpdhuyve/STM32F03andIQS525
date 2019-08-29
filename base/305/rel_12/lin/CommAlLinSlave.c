//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module that handles the application layer of a LIN slave
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define LIN_COMMALLINNSLAVE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef LIN_COMMALLINNSLAVE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               LIN_COMMALLINNSLAVE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SUPPLIER_ID
    #define SUPPLIER_ID                 0x7FFF
#endif
//------------------------------------------------------------------------------------------------//
#ifndef FUNCTION_ID
    #define FUNCTION_ID                 0xFFFF
#endif
//------------------------------------------------------------------------------------------------//
#ifndef VARIANT
    #define VARIANT                     0
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SUBSCRIBE_COUNT
    #define SUBSCRIBE_COUNT             5
#endif
//------------------------------------------------------------------------------------------------//
#ifndef REQUEST_COUNT
    #define REQUEST_COUNT               5
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD

// COM
#include "lin\CommAlLinSlave.h"
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
    U8                                      pid;
    U8                                      expected_length;
    CHECKSUM_METHOD                         checksum_method;
    COMMALLINSLAVE_REPORT_RESPONSE_HOOK     report_response_hook;
}
SUBSCRIBE_STRUCT;

typedef struct
{
    U8                                      pid;
    CHECKSUM_METHOD                         checksum_method;
    COMMALLINSLAVE_REQUEST_RESPONSE_HOOK    request_response_hook;
}
REQUEST_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL CommAlLinSlave_CheckForResponse(U8 pid, U8* buffer_ptr, U8* data_len_ptr, CHECKSUM_METHOD* checksum_method_ptr);

static BOOL CommAlLinSlave_GetExpectedResponse(U8 pid, U8* expected_len_ptr, CHECKSUM_METHOD* checksum_method_ptr);
static void CommAlLinSlave_ReportResponse(U8 pid, U8* recv_data_ptr, U8 recv_data_len);

static void CommAlLinSlave_HandleMasterRequestFrame(U8* recv_data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SUBSCRIBE_STRUCT                     commallin_subscribe_struct[SUBSCRIBE_COUNT];
static U8                                   commallin_subscribe_count;

static REQUEST_STRUCT                       commallin_request_struct[REQUEST_COUNT];
static U8                                   commallin_request_count;

static U8                                   commallin_nad;
static BOOL                                 commallin_slave_response_available;
static U8                                   commallin_slave_response[8];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL CommAlLinSlave_CheckForResponse(U8 pid, U8* buffer_ptr, U8* data_len_ptr, CHECKSUM_METHOD* checksum_method_ptr)
{
    REQUEST_STRUCT*     request_ptr;
    
    if(pid == 61)
    {
        if(commallin_slave_response_available)
        {
            MEMCPY((VPTR)buffer_ptr, (VPTR)commallin_slave_response, 8);
            *data_len_ptr        = 8;
            *checksum_method_ptr = CHECKSUM_CLASSIC;
            return TRUE;
        }
        return FALSE;
    }
    
    for(request_ptr = commallin_request_struct; request_ptr < &commallin_request_struct[commallin_request_count]; request_ptr++)
    {
        if(request_ptr->pid == pid)
        {
            *checksum_method_ptr = request_ptr->checksum_method;
            if(request_ptr->request_response_hook != NULL)
            {
                return request_ptr->request_response_hook(buffer_ptr, data_len_ptr);
            }
            return FALSE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlLinSlave_GetExpectedResponse(U8 pid, U8* expected_len_ptr, CHECKSUM_METHOD* checksum_method_ptr)
{
    SUBSCRIBE_STRUCT*   subscribe_ptr;
    
    if(pid == 60)
    {
        *expected_len_ptr    = 8;
        *checksum_method_ptr = CHECKSUM_CLASSIC;
        return TRUE;
    }
    
    for(subscribe_ptr = commallin_subscribe_struct; subscribe_ptr < &commallin_subscribe_struct[commallin_subscribe_count]; subscribe_ptr++)
    {
        if(subscribe_ptr->pid == pid)
        {
            *expected_len_ptr    = subscribe_ptr->expected_length;
            *checksum_method_ptr = subscribe_ptr->checksum_method;
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void CommAlLinSlave_ReportResponse(U8 pid, U8* recv_data_ptr, U8 recv_data_len)
{
    SUBSCRIBE_STRUCT*   subscribe_ptr;
    
    if(pid == 60)
    {
        CommAlLinSlave_HandleMasterRequestFrame(recv_data_ptr);
        return;
    }
    
    for(subscribe_ptr = commallin_subscribe_struct; subscribe_ptr < &commallin_subscribe_struct[commallin_subscribe_count]; subscribe_ptr++)
    {
        if(subscribe_ptr->pid == pid)
        {
            if(subscribe_ptr->report_response_hook != NULL)
            {
                subscribe_ptr->report_response_hook(recv_data_ptr, recv_data_len);
            }
            return;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void CommAlLinSlave_HandleMasterRequestFrame(U8* recv_data_ptr)
{
    U16     supplier_id;
    U16     function_id;
    
    // clear response
    commallin_slave_response_available = FALSE;
    MEMSET((VPTR)commallin_slave_response, 0xFF, SIZEOF(commallin_slave_response));
    
    // check if request if for us
    if((recv_data_ptr[0] != 0x7F) && (recv_data_ptr[0] != commallin_nad))
    {
        return;
    }
    
    // check PCI (protocol control information) (only single frames with 6 data bytes are supported)
    if(recv_data_ptr[1] != 0x06)
    {
        return;
    }
    
    // check SID (service identifier)
    switch(recv_data_ptr[2])
    {
    case 0xB0:  // Assign NAD
        supplier_id = BYTES2WORD(recv_data_ptr[4], recv_data_ptr[3]);
        function_id = BYTES2WORD(recv_data_ptr[6], recv_data_ptr[5]);
        
        if((supplier_id == SUPPLIER_ID) && (function_id == FUNCTION_ID))
        {
            commallin_slave_response[0] = commallin_nad;
            commallin_slave_response[1] = 0x01;
            commallin_slave_response[2] = 0xF0;
            
            commallin_slave_response_available = TRUE;
            
            // update to new NAD
            commallin_nad = recv_data_ptr[7];
        }
        break;
        
    case 0xB2:  // Read identifier
        supplier_id = BYTES2WORD(recv_data_ptr[5], recv_data_ptr[4]);
        function_id = BYTES2WORD(recv_data_ptr[7], recv_data_ptr[6]);
        
        if((supplier_id == SUPPLIER_ID) && (function_id == FUNCTION_ID))
        {
            switch(recv_data_ptr[3])
            {
            case 0: // LIN product identification
                commallin_slave_response[0] = commallin_nad;
                commallin_slave_response[1] = 0x06;
                commallin_slave_response[2] = 0xF2;
                commallin_slave_response[3] = LSB(SUPPLIER_ID);
                commallin_slave_response[4] = MSB(SUPPLIER_ID);
                commallin_slave_response[5] = LSB(FUNCTION_ID);
                commallin_slave_response[6] = MSB(FUNCTION_ID);
                commallin_slave_response[7] = VARIANT;
                
                commallin_slave_response_available = TRUE;
                break;
                
            default:
                commallin_slave_response[0] = commallin_nad;
                commallin_slave_response[1] = 0x03;
                commallin_slave_response[2] = 0x7F;
                commallin_slave_response[3] = recv_data_ptr[3];
                commallin_slave_response[4] = 0x12; // error code
                
                commallin_slave_response_available = TRUE;
                break;
            }
        }
        break;
    }
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommAlLinSlave_Init(SCI_CHANNEL_HNDL sci_channel_hndl)
{
    MODULE_INIT_ONCE();
    
    CommDllLin_Init(sci_channel_hndl);
    CommDllLin_Register_CheckForResponseHook(CommAlLinSlave_CheckForResponse);
    CommDllLin_Register_GetExpectedResponseHook(CommAlLinSlave_GetExpectedResponse);
    CommDllLin_Register_ReportResponseHook(CommAlLinSlave_ReportResponse);
    
    commallin_subscribe_count = 0;
    MEMSET((VPTR)commallin_subscribe_struct, 0, SIZEOF(commallin_subscribe_struct));
    
    commallin_request_count = 0;
    MEMSET((VPTR)commallin_request_struct, 0, SIZEOF(commallin_request_struct));
    
    commallin_nad = 0x7F;
    commallin_slave_response_available = FALSE;
    MEMSET((VPTR)commallin_slave_response, 0, SIZEOF(commallin_slave_response));
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
BOOL CommAlLinSlave_SetNad(U8 nad)
{
    if((nad > 0) && (nad < 0x7F))
    {
        commallin_nad = nad;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U8 CommAlLinSlave_GetNad(void)
{
    return commallin_nad;
}
//------------------------------------------------------------------------------------------------//
BOOL CommAlLinSlave_SubscribeTo(U8 pid, U8 data_len, CHECKSUM_METHOD method, COMMALLINSLAVE_REPORT_RESPONSE_HOOK report_hook)
{
    SUBSCRIBE_STRUCT*   subscribe_ptr = &commallin_subscribe_struct[commallin_subscribe_count];
    
    if((commallin_subscribe_count < SUBSCRIBE_COUNT) && (pid < 60))
    {
        subscribe_ptr->pid                  = pid;
        subscribe_ptr->expected_length      = data_len;
        subscribe_ptr->checksum_method      = method;
        subscribe_ptr->report_response_hook = report_hook;
        
        commallin_subscribe_count++;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommAlLinSlave_ReplyToRequest(U8 pid, CHECKSUM_METHOD method, COMMALLINSLAVE_REQUEST_RESPONSE_HOOK request_hook)
{
    REQUEST_STRUCT*     request_ptr = &commallin_request_struct[commallin_request_count];
    
    if((commallin_request_count < REQUEST_COUNT) && (pid < 60))
    {
        request_ptr->pid                    = pid;
        request_ptr->checksum_method        = method;
        request_ptr->request_response_hook  = request_hook;
        
        commallin_request_count++;
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//
