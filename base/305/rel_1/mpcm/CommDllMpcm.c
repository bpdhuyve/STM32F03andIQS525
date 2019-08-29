//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the Data Link Layer of the MPCM-protocol.
// The main function of this layer is to transmit frames of characters between Master and slave equipment.
// The layer serves as a communication medium to the network layer.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MPCM__COMMDLLMPCM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MPCM__COMMDLLMPCM_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               MPCM__COMMDLLMPCM_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum time in µs before before a received message should start to be handled
//         (formerly STDDLL_T_TIMEOUT)
#ifndef STDDLL_T_MAX_REPLY_TIMEOUT
    #error "STDDLL_T_MAX_REPLY_TIMEOUT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum time in µs before before a response to a message should be received (when expected).
//         (formerly STDDLL_T_TIMEOUT)
// @remark  if set to 0, the response timeout is completely disabled.
#ifndef STDDLL_T_RESPONSE_TIMEOUT
    #error "STDDLL_T_RESPONSE_TIMEOUT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the master address
#ifndef STDDLL_MASTER_ADDRESS
    #define STDDLL_MASTER_ADDRESS           0x01
//    #error "STDDLL_MASTER_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the undefined address
#ifndef STDDLL_UNKNOWN_ADDRESS
    #define STDDLL_UNKNOWN_ADDRESS          0x00
//    #error "STDDLL_UNKNOWN_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the broadcast address
#ifndef STDDLL_BROADCAST_ADDRESS
    #define STDDLL_BROADCAST_ADDRESS        0xFF
//    #error "STDDLL_BROADCAST_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines whether the master has to check if a message is addressed to him (1) or not (0)
#ifndef STDDLL_CHECK_RX_MASTER_ADDRESS
    #error "STDDLL_CHECK_RX_MASTER_ADDRESS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the address mask
#ifndef STDDLL_ADDRESS_MASK
    #define STDDLL_ADDRESS_MASK     0xFF
//    #error "STDDLL_ADDRESS_MASK not defined in AppConfig.h"
#endif
#if STDDLL_ADDRESS_MASK & 0x01
    #define STDDLL_ADDRESS_SHIFT            0
#elif STDDLL_ADDRESS_MASK & 0x02
    #define STDDLL_ADDRESS_SHIFT            1
#elif STDDLL_ADDRESS_MASK & 0x04
    #define STDDLL_ADDRESS_SHIFT            2
#elif STDDLL_ADDRESS_MASK & 0x08
    #define STDDLL_ADDRESS_SHIFT            3
#elif STDDLL_ADDRESS_MASK & 0x10
    #define STDDLL_ADDRESS_SHIFT            4
#elif STDDLL_ADDRESS_MASK & 0x20
    #define STDDLL_ADDRESS_SHIFT            5
#elif STDDLL_ADDRESS_MASK & 0x40
    #define STDDLL_ADDRESS_SHIFT            6
#elif STDDLL_ADDRESS_MASK & 0x80
    #define STDDLL_ADDRESS_SHIFT            7
#else
    #error "ILLEGAL ADDRESS MASK"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the address mask
#ifndef STDDLL_LENGTH_INFO_IN_BYTE
    #define STDDLL_LENGTH_INFO_IN_BYTE     1
//    #error "STDDLL_LENGTH_INFO_IN_BYTE not defined in AppConfig.h"
#endif
#if STDDLL_LENGTH_INFO_IN_BYTE > 1
    #error "STDDLL_LENGTH_INFO_IN_BYTE must be 0 or 1"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the address mask
#ifndef STDDLL_LENGTH_MASK
    #define STDDLL_LENGTH_MASK     0xFF
//    #error "STDDLL_ADDRESS_MASK not defined in AppConfig.h"
#endif
#if STDDLL_LENGTH_MASK & 0x01
    #define STDDLL_LENGTH_SHIFT            0
#elif STDDLL_LENGTH_MASK & 0x02
    #define STDDLL_LENGTH_SHIFT            1
#elif STDDLL_LENGTH_MASK & 0x04
    #define STDDLL_LENGTH_SHIFT            2
#elif STDDLL_LENGTH_MASK & 0x08
    #define STDDLL_LENGTH_SHIFT            3
#elif STDDLL_LENGTH_MASK & 0x10
    #define STDDLL_LENGTH_SHIFT            4
#elif STDDLL_LENGTH_MASK & 0x20
    #define STDDLL_LENGTH_SHIFT            5
#elif STDDLL_LENGTH_MASK & 0x40
    #define STDDLL_LENGTH_SHIFT            6
#elif STDDLL_LENGTH_MASK & 0x80
    #define STDDLL_LENGTH_SHIFT            7
#else
    #error "ILLEGAL LENGTH MASK"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//COMM lib include section
#include "mpcm\CommDllMpcm.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   enumeration typedef for the DLE data link layer <em>state</em>
typedef enum
{
    DLL_MPCM_STATE_RX,
    DLL_MPCM_STATE_RX_MSG_RECEIVED,
    DLL_MPCM_STATE_WAIT_BEFORE_TX,
    DLL_MPCM_STATE_TX
}
DLL_MPCM_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U8 CommDllMpcm_TxGetNextByte(U8* byte_ptr, U8 length);
static void CommDllMpcm_RxNewByte(U8* byte_ptr, U8 length);
static void CommDllMpcm_OnError(void);
static void CommDllMpcm_MinReplyTimeoutTask(VPTR data_ptr);
static void CommDllMpcm_MaxReplyTimeoutTask(VPTR data_ptr);
#if STDDLL_T_RESPONSE_TIMEOUT > 0
static void CommDllMpcm_ResponseTimeoutTask(VPTR data_ptr);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SCI_CHANNEL_HNDL                 commdllmpcm_sci_channel_hndl;
static SCI_SPEED                        commdllmpcm_sci_speed;
static DLLMPCM_RECV_FRAME_HOOK          commdllmpcm_frame_hook;
static EVENT_CALLBACK                   commdllmpcm_tx_complete_hook;

static U8                               commdllmpcm_length;
static U8                               commdllmpcm_frame_bytes[STDDLL_FRAME_LENGTH];

static U8                               commdllmpcm_enc_state;
static U8                               commdllmpcm_dec_state;

static DLL_MPCM_STATE                   commdllmpcm_state;
static BOOL                             commdllmpcm_tx_allowed;  //flag which controls the timing issue
                                                                    //between RX and TX frames
static TASK_HNDL                        commdllmpcm_min_reply_timeout_task;
static TASK_HNDL                        commdllmpcm_max_reply_timeout_task;
static U8                               commdllmpcm_device_address;

#if STDDLL_T_RESPONSE_TIMEOUT > 0
static EVENT_CALLBACK                   commdllmpcm_response_timeout_hook;
static TASK_HNDL                        commdllmpcm_response_timeout_task;
static BOOL                             commdllmpcm_enable_response_timeout;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief   Function called by the SCI driver when he's ready to send the next character
// It fetches a new character (if there is one available) from the character array and paste the value into \p byte_ptr
// @param   byte_ptr:   a pointer to the U8 variable where the character has to be filled in
// @return  TRUE if \p byte_ptr was filled in with a valid value which should be transmitted, otherwise returns FALSE
static U8 CommDllMpcm_TxGetNextByte(U8* byte_ptr, U8 length)
{
    static U8* output_byte_ptr;
    static U8  output_length;
    static U8  output_BCC;

    if(commdllmpcm_state != DLL_MPCM_STATE_TX)
    {
        LOG_WRN("MPCM TX while not in TX!");
        return 0;
    }

    switch(commdllmpcm_enc_state)
    {
    case 0: //start of frame
        output_byte_ptr = commdllmpcm_frame_bytes;
        output_length = 0;
        output_BCC = 0;
        commdllmpcm_enc_state = 1;
        LOG_DEV("TX DLL: start");
    case 1: //data frame
        *byte_ptr = *output_byte_ptr;
        output_BCC += *byte_ptr;
        output_length++;
        output_byte_ptr++;
        LOG_DEV("TX DLL: 0x%02h", PU8(*byte_ptr));
        if(output_length >= commdllmpcm_length)
        {
            commdllmpcm_enc_state = 2;    //end of data frame
        }
        break;
    case 2: //end of frame: send BCC
        *byte_ptr = output_BCC;
        LOG_DEV("TX BCC: 0x%02h", PU8(*byte_ptr));
        commdllmpcm_enc_state = 3;
        break;
    case 3: //ready with sending
        commdllmpcm_enc_state = 0;
        commdllmpcm_state = DLL_MPCM_STATE_RX;
        LOG_DEV("TX DLL: done");
#if STDDLL_T_RESPONSE_TIMEOUT > 0
        if(commdllmpcm_enable_response_timeout)
        {
            CoreTask_Start(commdllmpcm_response_timeout_task);
        }
#endif
        if(commdllmpcm_tx_complete_hook != NULL)
        {
            commdllmpcm_tx_complete_hook();
        }
        return 0;
    default:
        LOG_ERR("Unknown enc state");
        return 0;
    }
    return 1;
}
//------------------------------------------------------------------------------------------------//
// @brief   function for adding a byte to a Receive buffer array using the frame decoding mechanism.
// @param   byte:             the byte that should be decoded
static void CommDllMpcm_RxNewByte(U8* byte_ptr, U8 length)
{
    static U8  input_length;
    static U8* input_data_ptr;
    static U8  input_BCC;
    static U8  input_dlc;
    U8 address;

    //Protection: Half Duplex
    if(commdllmpcm_state != DLL_MPCM_STATE_RX)
    {
        LOG_WRN("MPCM RX while not in RX!");
    }
    else
    {
        while(length > 0)
        {
            LOG_DEV("RX DLL: 0x%02h", PU8(*byte_ptr));
            //Frame decoding mechanism
            switch(commdllmpcm_dec_state)
            {
            case 0: //start of frame
                address = (*byte_ptr & STDDLL_ADDRESS_MASK) >> STDDLL_ADDRESS_SHIFT;
                if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
                {
                    //slaves: check if address is meant for you
                    if((address == commdllmpcm_device_address) || (address == STDDLL_BROADCAST_ADDRESS))
                    {
                        //valid slave address: disable MPCM, so all databytes will be received
                        DrvSciChannel_SetMpcmFilter(commdllmpcm_sci_channel_hndl, FALSE);
                    }
                    else
                    {
                        //if message is not meant for this slave: MPCM mode is kept enabled
                        return;
                    }
                }
#if STDDLL_CHECK_RX_MASTER_ADDRESS > 0
                else if(address != STDDLL_MASTER_ADDRESS)
                {
                    // master received a message not starting with the master address
                    // this should not occur, but is possible when a receive byte was lost.
                    // therefore the byte is rejected
                    return;
                }
#endif
                
                input_data_ptr = commdllmpcm_frame_bytes;
                *input_data_ptr = *byte_ptr;
                input_data_ptr++;
                input_length = 1;
                input_BCC = *byte_ptr;
                
#if STDDLL_LENGTH_INFO_IN_BYTE == 0
                input_dlc = (*byte_ptr & STDDLL_ADDRESS_MASK) >> STDDLL_ADDRESS_SHIFT;
                if(input_dlc >= STDDLL_FRAME_LENGTH - 1)
                {
                    LOG_WRN("error frame length too long");
                    CommDllMpcm_OnError();
                    return;
                }
                else if(input_dlc == 0)
                {
                    commdllmpcm_dec_state = 3;
                }
                else
                {
                    commdllmpcm_dec_state = 2;
                }
#else
                commdllmpcm_dec_state = 1;
#endif
                break;
#if STDDLL_LENGTH_INFO_IN_BYTE == 1
            case 1: // get second byte in case length information is in second byte
                *input_data_ptr = *byte_ptr;
                input_data_ptr++;
                input_length++;
                input_BCC += *byte_ptr;
                input_dlc = (*byte_ptr & STDDLL_ADDRESS_MASK) >> STDDLL_ADDRESS_SHIFT;
                if(input_dlc > STDDLL_FRAME_LENGTH - 2)
                {
                    LOG_WRN("error frame length too long");
                    CommDllMpcm_OnError();
                    return;
                }
                else if(input_dlc == 0)
                {
                    commdllmpcm_dec_state = 3;
                }
                else
                {
                    commdllmpcm_dec_state = 2;
                }
                break;
#endif
            case 2: // get data
                if(input_length <= STDDLL_FRAME_LENGTH)
                {
                    input_BCC += *byte_ptr;
                    *input_data_ptr = *byte_ptr;
                    input_length++;
                    input_data_ptr++;
                    if(--input_dlc == 0)
                    {
                        commdllmpcm_dec_state = 3;
                    }
                }
                else
                {
                    LOG_WRN("error frame length too long");
                    CommDllMpcm_OnError();
                }
                break;
            case 3: //end of frame: receive BCC
                if(*byte_ptr == input_BCC)
                {
                    commdllmpcm_length = input_length;
                    LOG_DEV("RX DLL : %02x", PU8A(commdllmpcm_frame_bytes, commdllmpcm_length));
                    
                    // received frame: set minimal reply timeout before allowed to send again
                    commdllmpcm_tx_allowed = FALSE;
                    CoreTask_Start(commdllmpcm_min_reply_timeout_task);
                    
                    // if slave: set max reply timeout
                    if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
                    {
                        CoreTask_Start(commdllmpcm_max_reply_timeout_task);
                    }
                    
#if STDDLL_T_RESPONSE_TIMEOUT > 0
                    if(commdllmpcm_enable_response_timeout)
                    {
                        CoreTask_Stop(commdllmpcm_response_timeout_task);
                        commdllmpcm_enable_response_timeout = FALSE;
                    }
#endif
                    commdllmpcm_state = DLL_MPCM_STATE_RX_MSG_RECEIVED;
                }
                else
                {
                    LOG_WRN("BCC NOK ");
                    CommDllMpcm_OnError();
                }
                if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
                {
                    DrvSciChannel_SetMpcmFilter(commdllmpcm_sci_channel_hndl, TRUE);
                }
    
                commdllmpcm_dec_state = 0;
                break;
            default:
                LOG_ERR("Unknown dec state");
            }
            
            byte_ptr++;
            length--;
        }
    }
    return;
}
//------------------------------------------------------------------------------------------------//
static void CommDllMpcm_OnError(void)
{
    if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
    {
        DrvSciChannel_SetMpcmFilter(commdllmpcm_sci_channel_hndl, TRUE);
    }
    commdllmpcm_dec_state = 0;
    commdllmpcm_state = DLL_MPCM_STATE_RX;
    LOG_WRN("frame : %02x", PU8A(commdllmpcm_frame_bytes, STDDLL_FRAME_LENGTH));
}
//------------------------------------------------------------------------------------------------//
// @brief   This function is used to handle the minimum reply time.
// @param   data_ptr :      not used here
static void CommDllMpcm_MinReplyTimeoutTask(VPTR data_ptr)
{
    CoreTask_Stop(commdllmpcm_min_reply_timeout_task);
    commdllmpcm_tx_allowed = TRUE;
}
//------------------------------------------------------------------------------------------------//
// @brief   Callback function which is called when this device is too busy to send a reply on a master's request in
//          time. If this is the case then the reply is cancelled.
// @param   data_ptr :           not used here.
static void CommDllMpcm_MaxReplyTimeoutTask(VPTR data_ptr)
{
    CoreTask_Stop(commdllmpcm_max_reply_timeout_task);
    LOG_WRN("DLL Max reply time-out.");
    commdllmpcm_state = DLL_MPCM_STATE_RX;
}
//------------------------------------------------------------------------------------------------//
#if STDDLL_T_RESPONSE_TIMEOUT > 0
// @brief   Callback function which is called when the slave does not reply a request in time.
// @param   data_ptr :           not used here.
static void CommDllMpcm_ResponseTimeoutTask(VPTR data_ptr)
{
    CoreTask_Stop(commdllmpcm_response_timeout_task);
    commdllmpcm_enable_response_timeout = FALSE;
    LOG_WRN("DLL Comm time-out.");
    commdllmpcm_state = DLL_MPCM_STATE_RX;
    if(commdllmpcm_response_timeout_hook != NULL)
    {
        commdllmpcm_response_timeout_hook();
    }
    commdllmpcm_dec_state = 0;
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_Init(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED sci_speed)
{
    MODULE_INIT_ONCE();

    commdllmpcm_sci_channel_hndl = channel_hndl;
    commdllmpcm_sci_speed =  sci_speed;
    commdllmpcm_frame_hook = NULL;
    commdllmpcm_tx_complete_hook = NULL;

    CommDllMpcm_SetSlaveAddress(STDDLL_UNKNOWN_ADDRESS);
    
    DrvSciChannel_RegisterRxHook(commdllmpcm_sci_channel_hndl, CommDllMpcm_RxNewByte);
    DrvSciChannel_RegisterTxHook(commdllmpcm_sci_channel_hndl, CommDllMpcm_TxGetNextByte);

    commdllmpcm_enc_state = 0;
    commdllmpcm_dec_state = 0;

    commdllmpcm_state = DLL_MPCM_STATE_RX;
    commdllmpcm_tx_allowed = TRUE;
    
    commdllmpcm_min_reply_timeout_task = CoreTask_RegisterTask(STDDLL_T_MIN_REPLY_TIMEOUT,
                                                            CommDllMpcm_MinReplyTimeoutTask,
                                                            NULL,
                                                            255,
                                                            "MpcmMinReplyTo");
    commdllmpcm_max_reply_timeout_task = CoreTask_RegisterTask(STDDLL_T_MAX_REPLY_TIMEOUT,
                                                            CommDllMpcm_MaxReplyTimeoutTask,
                                                            NULL,
                                                            255,
                                                            "MpcmMaxReplyTo");

#if STDDLL_T_RESPONSE_TIMEOUT > 0
    commdllmpcm_response_timeout_hook = NULL;
    commdllmpcm_response_timeout_task = CoreTask_RegisterTask(STDDLL_T_RESPONSE_TIMEOUT,
                                                           CommDllMpcm_ResponseTimeoutTask,
                                                           NULL,
                                                           255,
                                                           "MpcmReponseTo");
    commdllmpcm_enable_response_timeout = FALSE;
#endif
    
    CoreTerm_RegisterCommand("CommDllMpcm", "COMM DLL mpcm", 0, CommDllMpcm_PrintStatus, TRUE);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_RegisterFrameHook(DLLMPCM_RECV_FRAME_HOOK frame_hook)
{
    commdllmpcm_frame_hook = frame_hook;
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_RegisterTimeoutHook(EVENT_CALLBACK timeout_hook)
{
#if STDDLL_T_RESPONSE_TIMEOUT > 0
    commdllmpcm_response_timeout_hook = timeout_hook;
#endif
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_RegisterTxCompleteHook(EVENT_CALLBACK tx_complete_hook)
{
    commdllmpcm_tx_complete_hook = tx_complete_hook;
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_Handler(void)
{
    if(commdllmpcm_state == DLL_MPCM_STATE_RX_MSG_RECEIVED)
    {
        // stop max reply timeout
        CoreTask_Stop(commdllmpcm_max_reply_timeout_task);

        commdllmpcm_state = DLL_MPCM_STATE_RX;
        
        LOG_DBG("DLL to AL: len = %d - data : %02h", PU8(commdllmpcm_length), PU8A(commdllmpcm_frame_bytes, commdllmpcm_length));
        if(commdllmpcm_frame_hook != NULL)
        {
            commdllmpcm_frame_hook(&commdllmpcm_frame_bytes[0], commdllmpcm_length);
        }
        else
        {
            LOG_WRN("DLL rx hook is NULL!");
        }
    }
    
    if(commdllmpcm_state == DLL_MPCM_STATE_WAIT_BEFORE_TX)
    {
        if(commdllmpcm_tx_allowed)
        {
            commdllmpcm_state = DLL_MPCM_STATE_TX;
            DrvSciChannel_NotifyTxDataReady(commdllmpcm_sci_channel_hndl);
        }
    }
    
    if(commdllmpcm_state > (U8)DLL_MPCM_STATE_TX)
    {
        LOG_ERR("DLL_MPCM state error!");
        commdllmpcm_enc_state = 0;
        commdllmpcm_dec_state = 0;
        commdllmpcm_state = DLL_MPCM_STATE_RX;
    }
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_SendFrame(U8* ptr_ptr, U8 length)
{
    if(commdllmpcm_state == DLL_MPCM_STATE_RX)
    {
        if(length <= STDDLL_FRAME_LENGTH)
        {
            commdllmpcm_length = length;
            MEMCPY((VPTR)commdllmpcm_frame_bytes, (VPTR)ptr_ptr, length);
            LOG_DBG("AL to DLL: len = %d - data : %02h", PU8(length), PU8A(ptr_ptr, length));

            commdllmpcm_state = DLL_MPCM_STATE_WAIT_BEFORE_TX;
        }
        else
        {
            LOG_WRN("DLL TX msg too long");
        }
    }
    else
    {
        LOG_WRN("DLL TX tx-mode is not queued!");
    }
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_PrintStatus(void)
{
    LOG_TRM("SCI channel    : 0x%8h", PU32(commdllmpcm_sci_channel_hndl));
    LOG_TRM("Frame          : %d - %02h", PU8(commdllmpcm_length), PU8A(commdllmpcm_frame_bytes, commdllmpcm_length));
    LOG_TRM("State          : %d", PU8(commdllmpcm_state));
    LOG_TRM("enc_state      : %d", PU8(commdllmpcm_enc_state));
    LOG_TRM("dec_state      : %d", PU8(commdllmpcm_dec_state));
    LOG_TRM("tx_allowed     : %d", PU8(commdllmpcm_tx_allowed));
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_SetSlaveAddress(U8 address)
{
    if(address == STDDLL_BROADCAST_ADDRESS)
    {
        address = STDDLL_UNKNOWN_ADDRESS;
    }
    commdllmpcm_device_address = address;
    DrvSciChannel_ConfigAsMpcm(commdllmpcm_sci_channel_hndl, commdllmpcm_sci_speed, (BOOL)(address == STDDLL_MASTER_ADDRESS));
}
//------------------------------------------------------------------------------------------------//
U8 CommDllMpcm_GetSlaveAddress(void)
{
    return commdllmpcm_device_address;
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_EnableTimeout(BOOL state)
{
#if STDDLL_T_RESPONSE_TIMEOUT > 0
    commdllmpcm_enable_response_timeout = state;
#endif
}
//================================================================================================//

