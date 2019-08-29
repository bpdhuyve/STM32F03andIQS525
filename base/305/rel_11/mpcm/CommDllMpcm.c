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
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the undefined address
#ifndef STDDLL_UNKNOWN_ADDRESS
    #define STDDLL_UNKNOWN_ADDRESS          0x00
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the broadcast address
#ifndef STDDLL_BROADCAST_ADDRESS
    #define STDDLL_BROADCAST_ADDRESS        0xFF
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
#endif
#if STDDLL_LENGTH_INFO_IN_BYTE > 1
    #error "STDDLL_LENGTH_INFO_IN_BYTE must be 0 or 1"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the address mask
#ifndef STDDLL_LENGTH_MASK
    #define STDDLL_LENGTH_MASK     0xFF
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
//------------------------------------------------------------------------------------------------//
// @brief  Defines if RX messages have to be handled on interrupt (1) or in background (0)
#ifndef STDDLL_HANDLE_RX_ON_ISR
    #define STDDLL_HANDLE_RX_ON_ISR        0
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if TX messages have to be handled on interrupt (1) or in background (0)
#ifndef STDDLL_HANDLE_TX_ON_ISR
    #define STDDLL_HANDLE_TX_ON_ISR        0
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

typedef enum
{
    DLL_TX_START_FRAME,
    DLL_TX_DATA_BYTE,
    DLL_TX_BCC,
    DLL_TX_END,
}
DLL_TX_STATE;

typedef enum
{
    DLL_RX_START_FRAME,
    DLL_RX_INFO,
    DLL_RX_DATA,
    DLL_RX_BCC
}
DLL_RX_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U8 CommDllMpcm_TxGetNextByte(U8* byte_ptr, U8 length);
static void CommDllMpcm_RxNewByte(U8* byte_ptr, U8 length);
static void CommDllMpcm_HandleReceivedFrame(void);
static void CommDllMpcm_HandleSendingFrame(void);
static void CommDllMpcm_OnError(void);
static void CommDllMpcm_MinReplyTimeoutTask(VPTR data_ptr);
static void CommDllMpcm_MaxReplyTimeoutTask(VPTR data_ptr);
#if STDDLL_T_RESPONSE_TIMEOUT > 0
static void CommDllMpcm_ResponseTimeoutTask(VPTR data_ptr);
#endif
#if (TERM_LEVEL > TERM_LEVEL_NONE)
void Command_CommDllMpcm(void);
#endif
static void CommDllMpcm_Handler(void);
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

static DLL_TX_STATE                     commdllmpcm_enc_state;
static DLL_RX_STATE                     commdllmpcm_dec_state;

static DLL_MPCM_STATE                   commdllmpcm_state;
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
    case DLL_TX_START_FRAME:
        output_byte_ptr = commdllmpcm_frame_bytes;
        output_length = 0;
        output_BCC = 0;
        commdllmpcm_enc_state = DLL_TX_DATA_BYTE;
        LOG_DEV("TX DLL: start");
        // no break;

    case DLL_TX_DATA_BYTE:
        *byte_ptr = *output_byte_ptr;
        output_BCC += *byte_ptr;
        output_length++;
        output_byte_ptr++;
        LOG_DEV("TX DLL: 0x%02h", PU8(*byte_ptr));
        if(output_length >= commdllmpcm_length)
        {
            commdllmpcm_enc_state = DLL_TX_BCC;
        }
        break;

    case DLL_TX_BCC:
        *byte_ptr = output_BCC;
        LOG_DEV("TX BCC: 0x%02h", PU8(*byte_ptr));
        commdllmpcm_enc_state = DLL_TX_END;
        break;

    case DLL_TX_END:
        commdllmpcm_enc_state = DLL_TX_START_FRAME;
        commdllmpcm_state = DLL_MPCM_STATE_RX;
        LOG_DEV("TX DLL: done");
        #if STDDLL_T_RESPONSE_TIMEOUT > 0
        {
            if(commdllmpcm_enable_response_timeout)
            {
                CoreTask_Start(commdllmpcm_response_timeout_task);
            }
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
    static U8  recv_frame_length;
    U8 address;

    //Protection: Half Duplex
    if(commdllmpcm_state != DLL_MPCM_STATE_RX)
    {
        LOG_WRN("MPCM RX while not in RX!");
        return;
    }

    while(length > 0)
    {
        LOG_DEV("RX DLL: 0x%02h", PU8(*byte_ptr));
        switch(commdllmpcm_dec_state)
        {
        case DLL_RX_START_FRAME:
            address = (*byte_ptr & STDDLL_ADDRESS_MASK) >> STDDLL_ADDRESS_SHIFT;
            if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
            {
                //slaves: check if address is meant for you
                if((address == commdllmpcm_device_address) || (address == STDDLL_BROADCAST_ADDRESS))
                {
                    //valid Device address: disable MPCM, so all databytes will be received
                    DrvSciChannel_SetMpcmFilter(commdllmpcm_sci_channel_hndl, FALSE);
                }
                else
                {
                    //if message is not meant for this Device: MPCM filter is kept enabled
                    return;
                }
            }
            else
            {
                #if STDDLL_CHECK_RX_MASTER_ADDRESS > 0
                {
                    if(address != STDDLL_MASTER_ADDRESS)
                    {
                        // master received a message not starting with the master address
                        // this should not occur, but is possible when a receive byte was lost.
                        // therefore the byte is rejected
                        return;
                    }
                }
                #endif
            }

            input_data_ptr = commdllmpcm_frame_bytes;

            *input_data_ptr = *byte_ptr;
            input_data_ptr++;
            input_length = 1;
            input_BCC = *byte_ptr;

            #if STDDLL_LENGTH_INFO_IN_BYTE > 0
            {
                commdllmpcm_dec_state = DLL_RX_INFO;
                break;
            }
            #endif

        case DLL_RX_INFO:
            #if STDDLL_LENGTH_INFO_IN_BYTE > 0
            {
                *input_data_ptr = *byte_ptr;
                input_data_ptr++;
                input_length++;
                input_BCC += *byte_ptr;
            }
            #endif

            recv_frame_length = (*byte_ptr & STDDLL_LENGTH_MASK) >> STDDLL_LENGTH_SHIFT;
            if(recv_frame_length > (STDDLL_FRAME_LENGTH - STDDLL_LENGTH_INFO_IN_BYTE - 1))
            {
                LOG_WRN("error frame length too long");
                CommDllMpcm_OnError();
                return;
            }
            else if(recv_frame_length == 0)
            {
                commdllmpcm_dec_state = DLL_RX_BCC;
            }
            else
            {
                commdllmpcm_dec_state = DLL_RX_DATA;
            }
            break;

        case DLL_RX_DATA:
            if(input_length > STDDLL_FRAME_LENGTH)
            {
                LOG_WRN("error frame length too long");
                CommDllMpcm_OnError();
                return;
            }

            input_BCC += *byte_ptr;
            *input_data_ptr = *byte_ptr;
            input_length++;
            input_data_ptr++;

            if(--recv_frame_length == 0)
            {
                commdllmpcm_dec_state = DLL_RX_BCC;
            }
            break;

        case DLL_RX_BCC:
            if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
            {
                DrvSciChannel_SetMpcmFilter(commdllmpcm_sci_channel_hndl, TRUE);
            }

            if(*byte_ptr != input_BCC)
            {
                LOG_WRN("BCC NOK ");
                CommDllMpcm_OnError();
                return;
            }

            commdllmpcm_length = input_length;

            CoreTask_Start(commdllmpcm_min_reply_timeout_task);
            if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
            {
                CoreTask_Start(commdllmpcm_max_reply_timeout_task);
            }

            #if STDDLL_T_RESPONSE_TIMEOUT > 0
            {
                if(commdllmpcm_enable_response_timeout)
                {
                    CoreTask_Stop(commdllmpcm_response_timeout_task);
                    commdllmpcm_enable_response_timeout = FALSE;
                }
            }
            #endif

            #if STDDLL_HANDLE_RX_ON_ISR > 0
            {
                CommDllMpcm_HandleReceivedFrame();
            }
            #else
            {
                commdllmpcm_state = DLL_MPCM_STATE_RX_MSG_RECEIVED;
            }
            #endif

            commdllmpcm_dec_state = DLL_RX_START_FRAME;
            break;

        default:
            LOG_ERR("Unknown dec state");
            break;
        }

        byte_ptr++;
        length--;
    }
}
//------------------------------------------------------------------------------------------------//
static void CommDllMpcm_HandleReceivedFrame(void)
{
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
//------------------------------------------------------------------------------------------------//
static void CommDllMpcm_HandleSendingFrame(void)
{
    CoreTask_Stop(commdllmpcm_max_reply_timeout_task);

    if(commdllmpcm_state == DLL_MPCM_STATE_WAIT_BEFORE_TX)
    {
        commdllmpcm_state = DLL_MPCM_STATE_TX;
        DrvSciChannel_NotifyTxDataReady(commdllmpcm_sci_channel_hndl);
    }
}
//------------------------------------------------------------------------------------------------//
static void CommDllMpcm_OnError(void)
{
    if(commdllmpcm_device_address != STDDLL_MASTER_ADDRESS)
    {
        DrvSciChannel_SetMpcmFilter(commdllmpcm_sci_channel_hndl, TRUE);
    }
    commdllmpcm_dec_state = DLL_RX_START_FRAME;
    commdllmpcm_state = DLL_MPCM_STATE_RX;
    LOG_WRN("frame : %02x", PU8A(commdllmpcm_frame_bytes, STDDLL_FRAME_LENGTH));
}
//------------------------------------------------------------------------------------------------//
// @brief   This function is used to handle the minimum reply time.
// @param   data_ptr :      not used here
static void CommDllMpcm_MinReplyTimeoutTask(VPTR data_ptr)
{
    CoreTask_Stop(commdllmpcm_min_reply_timeout_task);
    #if STDDLL_HANDLE_TX_ON_ISR > 0
    {
        CommDllMpcm_HandleSendingFrame();
    }
    #endif
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
    commdllmpcm_dec_state = DLL_RX_START_FRAME;
}
#endif
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
void Command_CommDllMpcm(void)
{
    LOG_TRM("SCI channel    : 0x%8h", PU32(commdllmpcm_sci_channel_hndl));
    LOG_TRM("Frame          : %d - %02h", PU8(commdllmpcm_length), PU8A(commdllmpcm_frame_bytes, commdllmpcm_length));
    LOG_TRM("State          : %d", PU8(commdllmpcm_state));
    LOG_TRM("enc_state      : %d", PU8(commdllmpcm_enc_state));
    LOG_TRM("dec_state      : %d", PU8(commdllmpcm_dec_state));
}
#endif
//------------------------------------------------------------------------------------------------//
static void CommDllMpcm_Handler(void)
{
    #if STDDLL_HANDLE_RX_ON_ISR == 0
    {
        if(commdllmpcm_state == DLL_MPCM_STATE_RX_MSG_RECEIVED)
        {
            CommDllMpcm_HandleReceivedFrame();
        }
    }
    #endif

    #if STDDLL_HANDLE_TX_ON_ISR == 0
    {
        if((commdllmpcm_state == DLL_MPCM_STATE_WAIT_BEFORE_TX) && (CoreTask_IsTaskRunning(commdllmpcm_min_reply_timeout_task) == FALSE))
        {
            CommDllMpcm_HandleSendingFrame();
        }
    }
    #endif

    if(commdllmpcm_state > DLL_MPCM_STATE_TX)
    {
        LOG_ERR("DLL_MPCM state error!");
        commdllmpcm_enc_state = DLL_TX_START_FRAME;
        commdllmpcm_dec_state = DLL_RX_START_FRAME;
        commdllmpcm_state = DLL_MPCM_STATE_RX;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_Init(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED sci_speed)
{    
    
    MODULE_INIT_ONCE();

    commdllmpcm_min_reply_timeout_task = CoreTask_RegisterTask(STDDLL_T_MIN_REPLY_TIMEOUT, CommDllMpcm_MinReplyTimeoutTask, NULL, 127, "MpcmMinReplyTo");
    commdllmpcm_max_reply_timeout_task = CoreTask_RegisterTask(STDDLL_T_MAX_REPLY_TIMEOUT, CommDllMpcm_MaxReplyTimeoutTask, NULL, 126, "MpcmMaxReplyTo");

#if STDDLL_T_RESPONSE_TIMEOUT > 0
    commdllmpcm_response_timeout_hook = NULL;
    commdllmpcm_response_timeout_task = CoreTask_RegisterTask(STDDLL_T_RESPONSE_TIMEOUT, CommDllMpcm_ResponseTimeoutTask, NULL, 255, "MpcmReponseTo");
    commdllmpcm_enable_response_timeout = FALSE;
#endif

    CoreTerm_RegisterCommand("CommDllMpcm", "COMM DLL mpcm", 0, Command_CommDllMpcm, TRUE);
    
    Core_RegisterModuleHandler(CommDllMpcm_Handler);

    MODULE_INIT_DONE();
    
    // stop tasks on reinit to prevent illegal states
    CoreTask_Stop(commdllmpcm_min_reply_timeout_task);
    CoreTask_Stop(commdllmpcm_max_reply_timeout_task);
#if STDDLL_T_RESPONSE_TIMEOUT > 0
    CoreTask_Stop(commdllmpcm_response_timeout_task);
#endif
    
    commdllmpcm_sci_channel_hndl = channel_hndl;
    commdllmpcm_sci_speed =  sci_speed;
    commdllmpcm_frame_hook = NULL;
    commdllmpcm_tx_complete_hook = NULL;

    CommDllMpcm_SetDeviceAddress(STDDLL_UNKNOWN_ADDRESS);


    commdllmpcm_enc_state = DLL_TX_START_FRAME;
    commdllmpcm_dec_state = DLL_RX_START_FRAME;

    commdllmpcm_state = DLL_MPCM_STATE_RX;
    DrvSciChannel_RegisterRxHook(commdllmpcm_sci_channel_hndl, CommDllMpcm_RxNewByte);
    DrvSciChannel_RegisterTxHook(commdllmpcm_sci_channel_hndl, CommDllMpcm_TxGetNextByte);
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_RegisterFrameReceivedHook(DLLMPCM_RECV_FRAME_HOOK frame_hook)
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
BOOL CommDllMpcm_SendFrame(U8* frame_ptr, U8 frame_length)
{
    if(commdllmpcm_state != DLL_MPCM_STATE_RX)
    {
        LOG_WRN("DLL not ready for tx frame, frame not send");
        return FALSE;
    }

    if(frame_length > STDDLL_FRAME_LENGTH)
    {
        LOG_WRN("DLL TX msg too long, frame not send");
        return FALSE;
    }

    commdllmpcm_length = frame_length;
    MEMCPY((VPTR)commdllmpcm_frame_bytes, (VPTR)frame_ptr, frame_length);
    LOG_DBG("AL to DLL: len = %d - data : %02h", PU8(frame_length), PU8A(frame_ptr, frame_length));

    commdllmpcm_state = DLL_MPCM_STATE_WAIT_BEFORE_TX;

    #if STDDLL_HANDLE_TX_ON_ISR > 0
    {
        if((CoreTask_IsTaskRunning(commdllmpcm_min_reply_timeout_task) == FALSE) && (commdllmpcm_state == DLL_MPCM_STATE_WAIT_BEFORE_TX))
        {
            CommDllMpcm_HandleSendingFrame();
        }
    }
    #endif

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
void CommDllMpcm_SetDeviceAddress(U8 address)
{
    if(address == STDDLL_BROADCAST_ADDRESS)
    {
        address = STDDLL_UNKNOWN_ADDRESS;
    }
    commdllmpcm_device_address = address;
    DrvSciChannel_ConfigAsMpcm(commdllmpcm_sci_channel_hndl, commdllmpcm_sci_speed, (BOOL)(address == STDDLL_MASTER_ADDRESS));
}
//------------------------------------------------------------------------------------------------//
U8 CommDllMpcm_GetDeviceAddress(void)
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

