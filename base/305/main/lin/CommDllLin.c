//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module that handles the communication of a LIN slave
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define LIN_COMMDLLLIN_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef LIN_COMMDLLLIN_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               LIN_COMMDLLLIN_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD

// COM
#include "lin\CommDllLin.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    LIN_RECV_WAIT_FOR_BREAK           = 0,
    LIN_RECV_WAIT_FOR_SYNC            = 1,
    LIN_RECV_WAIT_FOR_PID             = 2,
    LIN_RECV_WAIT_FOR_DATA            = 3,
    LIN_RECV_WAIT_FOR_CRC             = 4,
}
LIN_RECV_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void CommDllLin_LinBreakDetected(void);
static BOOL CommDllLin_IsLegalPid(void);
static U8 CommDllLin_CalculateChecksum(U8* data_ptr, U8 data_len, CHECKSUM_METHOD checksum_method);

static void CommDllLin_RxNewByte(U8* byte_ptr, U8 length);
static U8 CommDllLin_TxGetNextByte(U8* byte_ptr, U8 length);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SCI_CHANNEL_HNDL                     commdlllin_sci_hndl;

static COMMDLLLIN_CHECK_FOR_RESPONSE_HOOK      commdlllin_check_for_response_hook;
static COMMDLLLIN_GET_EXPECTED_RESPONSE_HOOK   commdlllin_get_expected_response_hook;
static COMMDLLLIN_REPORT_RESPONSE_HOOK         commdlllin_report_response_hook;

static LIN_RECV_STATE                       commdlllin_recv_state;
static U8                                   commdlllin_recv_pid;

static U8                                   commdlllin_recv_buffer[8];
static U8                                   commdlllin_recv_len;
static CHECKSUM_METHOD                      commdlllin_recv_checksum_method;
static U8*                                  commdlllin_recv_buffer_ptr;

static U8                                   commdlllin_send_buffer[9];
static U8                                   commdlllin_send_len;
static CHECKSUM_METHOD                      commdlllin_send_checksum_method;
static U8*                                  commdlllin_send_buffer_ptr;

const U8 allowed_pid[] = {0x80, 0xC1, 0x42, 0x03, 0xC4, 0x85, 0x06, 0x47, 0x08, 0x49, 0xCA, 0x8B, 0x4C, 0x0D, 0x8E, 0xCF,
                          0x50, 0x11, 0x92, 0xD3, 0x14, 0x55, 0xD6, 0x97, 0xD8, 0x99, 0x1A, 0x5B, 0x9C, 0xDD, 0x5E, 0x1F,
                          0x20, 0x61, 0xE2, 0xA3, 0x64, 0x25, 0xA6, 0xE7, 0xA8, 0xE9, 0x6A, 0x2B, 0xEC, 0xAD, 0x2E, 0x6F,
                          0xF0, 0xB1, 0x32, 0x73, 0xB4, 0xF5, 0x76, 0x37, 0x78, 0x39, 0xBA, 0xFB, 0x3C, 0x7D, 0xFE, 0xBF};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CommDllLin_LinBreakDetected(void)
{
    LOG_DEV("BREAK");
    
    commdlllin_recv_state = LIN_RECV_WAIT_FOR_SYNC;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommDllLin_IsLegalPid(void)
{
    return (BOOL)(commdlllin_recv_pid = allowed_pid[commdlllin_recv_pid & 0x3F]);
}
//------------------------------------------------------------------------------------------------//
static U8 CommDllLin_CalculateChecksum(U8* data_ptr, U8 data_len, CHECKSUM_METHOD checksum_method)
{
    U16 sum = 0;
    
    if(checksum_method == CHECKSUM_ENHANCED)
    {
        sum = commdlllin_recv_pid;
    }
    
    while(data_len)
    {
        sum += *data_ptr;
        sum += (sum >> 8);
        sum &= 0x00FF;
        data_ptr++;
        data_len--;
    }
    
    return (U8)(0xFF - sum);
}
//------------------------------------------------------------------------------------------------//
static void CommDllLin_RxNewByte(U8* byte_ptr, U8 length)
{
    LOG_DEV("LIN RX %02x - DS %d", PU8A(byte_ptr, length), PU8(commdlllin_recv_state));
    
    while(length)
    {
        switch(commdlllin_recv_state)
        {
        case LIN_RECV_WAIT_FOR_SYNC:
            // check if legal SYNC
            if(*byte_ptr != 0x55)
            {
                commdlllin_recv_state = LIN_RECV_WAIT_FOR_BREAK;
                break;
            }
            
            commdlllin_recv_state = LIN_RECV_WAIT_FOR_PID;
            break;
            
        case LIN_RECV_WAIT_FOR_PID:
            commdlllin_recv_pid = *byte_ptr;
            
            // check if legal PID
            if(CommDllLin_IsLegalPid() == FALSE)
            {
                commdlllin_recv_state = LIN_RECV_WAIT_FOR_BREAK;
                break;
            }
            
            // check if response needs to be given
            if((commdlllin_check_for_response_hook != NULL) &&
               (commdlllin_check_for_response_hook(commdlllin_recv_pid & 0x3F,
                                                        commdlllin_send_buffer,
                                                        &commdlllin_send_len,
                                                        &commdlllin_send_checksum_method)) &&
               (commdlllin_send_len > 0) && (commdlllin_send_len <= 8))
            {
                // add checksum to outgoing buffer
                commdlllin_send_buffer[commdlllin_send_len] = CommDllLin_CalculateChecksum(commdlllin_send_buffer,
                                                                                                          commdlllin_send_len,
                                                                                                          commdlllin_send_checksum_method);
                commdlllin_send_len++;
                
                // init pointer
                commdlllin_send_buffer_ptr = commdlllin_send_buffer;
                
                // indicate data ready
                DrvSciChannel_NotifyTxDataReady(commdlllin_sci_hndl);
                
                // wait for next break
                //commdlllin_recv_state = LIN_RECV_WAIT_FOR_BREAK;
                //break;
            }
            
            // check if response is expected
            if((commdlllin_get_expected_response_hook != NULL) &&
               (commdlllin_get_expected_response_hook(commdlllin_recv_pid & 0x3F,
                                                           &commdlllin_recv_len,
                                                           &commdlllin_recv_checksum_method)) &&
               (commdlllin_recv_len > 0) && (commdlllin_recv_len <= 8))
            {
                commdlllin_recv_buffer_ptr = commdlllin_recv_buffer;
                commdlllin_recv_state = LIN_RECV_WAIT_FOR_DATA;
                break;
            }
            
            // otherwise discard and wait for next
            commdlllin_recv_state = LIN_RECV_WAIT_FOR_BREAK;
            break;
            
        case LIN_RECV_WAIT_FOR_DATA:
            *commdlllin_recv_buffer_ptr = *byte_ptr;
            commdlllin_recv_buffer_ptr++;
            if(commdlllin_recv_buffer_ptr == &commdlllin_recv_buffer[commdlllin_recv_len])
            {
                commdlllin_recv_state = LIN_RECV_WAIT_FOR_CRC;
            }
            break;
            
        case LIN_RECV_WAIT_FOR_CRC:
            if((*byte_ptr == CommDllLin_CalculateChecksum(commdlllin_recv_buffer,
                                                               commdlllin_recv_len,
                                                               commdlllin_recv_checksum_method)) &&
               (commdlllin_report_response_hook != NULL))
            {
                commdlllin_report_response_hook(commdlllin_recv_pid & 0x3F,
                                                     commdlllin_recv_buffer,
                                                     commdlllin_recv_len);
            }
            commdlllin_recv_state = LIN_RECV_WAIT_FOR_BREAK;
            break;
            
        case LIN_RECV_WAIT_FOR_BREAK:
        default:
            // do nothing, just wait for break
            break;
        }
        
        byte_ptr++;
        length--;
    }
}
//------------------------------------------------------------------------------------------------//
static U8 CommDllLin_TxGetNextByte(U8* byte_ptr, U8 length)
{
    U8  out_len = MIN(length, commdlllin_send_len);
    
    if(out_len > 0)
    {
        MEMCPY((VPTR)byte_ptr, commdlllin_send_buffer_ptr, out_len);
        commdlllin_send_buffer_ptr += out_len;
        commdlllin_send_len -= out_len;
        
        LOG_DEV("LIN TX %02x", PU8A(byte_ptr, out_len));
    }
    
    return out_len;
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
void CommDllLin_Init(SCI_CHANNEL_HNDL sci_channel_hndl)
{
    SCI_CONFIG_STRUCT                sci_cfg_struct;
    
    MODULE_INIT_ONCE();
    
    commdlllin_sci_hndl = sci_channel_hndl;
    
    sci_cfg_struct.speed        = SCI_SPEED_19200_BPS;
    sci_cfg_struct.parity       = SCI_PARITY_NONE;
    sci_cfg_struct.stopbit      = SCI_STOPBIT_1;
    sci_cfg_struct.data_length  = SCI_DATA_LENGTH_8_BITS;
    DrvSciChannel_ConfigAsLin(commdlllin_sci_hndl, &sci_cfg_struct);
    
    DrvSciChannel_RegisterEvent_RxDataReceived(commdlllin_sci_hndl, CommDllLin_RxNewByte);
    DrvSciChannel_RegisterEvent_TxDataNeeded(commdlllin_sci_hndl, CommDllLin_TxGetNextByte);
    DrvSciChannel_RegisterEvent_LinBreakDetected(commdlllin_sci_hndl, CommDllLin_LinBreakDetected);
    
    commdlllin_check_for_response_hook     = NULL;
    commdlllin_get_expected_response_hook  = NULL;
    commdlllin_report_response_hook        = NULL;
    
    commdlllin_recv_state = LIN_RECV_WAIT_FOR_BREAK;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllLin_Register_CheckForResponseHook(COMMDLLLIN_CHECK_FOR_RESPONSE_HOOK check_for_response_hook)
{
    commdlllin_check_for_response_hook = check_for_response_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllLin_Register_GetExpectedResponseHook(COMMDLLLIN_GET_EXPECTED_RESPONSE_HOOK get_expected_response_hook)
{
    commdlllin_get_expected_response_hook = get_expected_response_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllLin_Register_ReportResponseHook(COMMDLLLIN_REPORT_RESPONSE_HOOK report_response_hook)
{
    commdlllin_report_response_hook = report_response_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllLin_MasterRequest(U8 pid)
{
    commdlllin_send_buffer[0] = 0x55;
    commdlllin_send_buffer[1] = allowed_pid[pid];
    
    commdlllin_send_len = 2;
    
    commdlllin_send_buffer_ptr = commdlllin_send_buffer;
    
    DrvSciChannel_SendLinBreak(commdlllin_sci_hndl);
    return DrvSciChannel_NotifyTxDataReady(commdlllin_sci_hndl);
}
//================================================================================================//

