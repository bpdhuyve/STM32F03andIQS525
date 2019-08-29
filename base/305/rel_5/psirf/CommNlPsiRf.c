//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the Network Layer of the PsiRf-protocol.
// The main function of this layer is to transmit frames of characters between Master and slave equipment. The
// layer serves as a communication medium to the application layer.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define PSIRF__COMMNLPSIRF_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef PSIRF__COMMNLPSIRF_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               PSIRF__COMMNLPSIRF_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the queue size
#ifndef STDNLPSIRF_QUEUE_SIZE
    #error "STDNLPSIRF_QUEUE_SIZE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the broadcast delay timeout in ms
#ifndef STDNLPSIRF_BC_DELAY_TIMEOUT
    #error "STDNLPSIRF_BC_DELAY_TIMEOUT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the p2p timeout in ms
#ifndef STDNLPSIRF_P2P_TIMEOUT
    #error "STDNLPSIRF_P2P_TIMEOUT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the retry count for P2P msg
#ifndef STDNLPSIRF_RETRY_COUNT
    #error "STDNLPSIRF_RETRY_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the node can be used as a repeater point
#ifndef STDNLPSIRF_ENABLE_REPEATER
    #error "STDNLPSIRF_ENABLE_REPEATER not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the node can be used as a repeater point
#ifndef INCLUSION_RETRY
    #define INCLUSION_RETRY             2
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the node can be used as a repeater point
#ifndef EXPLORE_RETRY
    #define EXPLORE_RETRY               1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "psirf\CommNlPsiRf.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define STDBLPSIRF_NETWORK_MASTER_ADDR      1
#define STDNLPSIRF_P2P_TIMEOUT_US(x)        (((U32)STDNLPSIRF_P2P_TIMEOUT * 1000L) +((U32)x << 7))
#define STDNLPSIRF_BC_TIMEOUT_US            ((U32)STDNLPSIRF_MAX_NODES * (U32)STDNLPSIRF_BC_DELAY_TIMEOUT * 1000L)
#define STDNLPSIRF_BC_WAIT_TIMEOUT(x)       ((U32)(x) * (U32)STDNLPSIRF_BC_DELAY_TIMEOUT * 1000L)

#define STDNLPSIRF_DATA_LENGTH              (STDDLLPSIRF_FRAME_LENGTH-STDNLPSIRF_HEADER_LENGTH)
#define STDNLPSIRF_HEADER_LENGTH_BASE       (STDNLPSIRF_HEADER_LENGTH-1)
#define STDNLPSIRF_HEADER_EXTRA_INCL        4
#define STDNLPSIRF_HEADER_LENGTH_INCL       (STDNLPSIRF_HEADER_LENGTH_BASE+STDNLPSIRF_HEADER_EXTRA_INCL)
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   enumeration typedef for the DLE data link layer <em>state</em>
typedef enum
{
    NL_PSIRF_NO_ADDRESS,
    NL_PSIRF_IDLE,
    NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT,
    NL_PSIRF_WAIT_FOR_INCLUSION,
    NL_PSIRF_WAIT_FOR_EXCLUSION,
    NL_PSIRF_WAIT_FOR_BROADCAST_FORWARD_TIMEOUT,
    NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT,
    NL_PSIRF_WAIT_FOR_ACK,
    NL_PSIRF_WAIT_FOR_ACK_SET_NID,
}
NL_PSIRF_STATE;

typedef enum
{
    MSG_TYPE_V0_LINK        = 0,
    MSG_TYPE_V0_EXPLORE     = 1,
    MSG_TYPE_V0_REVEAL      = 2,
    MSG_TYPE_V0_REQ_NID     = 3,
    MSG_TYPE_V0_SET_NID     = 4,
    MSG_TYPE_V0_REM_NID     = 5,
    MSG_TYPE_V0_ACK         = 6,
    MSG_TYPE_V0_APPLIC      = 7,
}
NL_PSIRF_MSG_TYPE_V0;

typedef enum
{
    MSG_VERSION_V0          = 0,
}
NL_PSIRF_MSG_VERSION;

typedef union
{
    U8              type;
    NL_PSIRF_MSG_TYPE_V0    v0;
}
NL_PSIRF_MSG_TYPE;

typedef struct
{
    U8              length;
    NL_PSIRF_MSG_VERSION    version;
    NL_PSIRF_MSG_TYPE       msg_type;
    U32             hid;
    U8              snid;
    U8              tnid;
    U8              rsnid;
    U8              rtnid;
    U8              fid;
    U8              rssi;
    U8              data[STDNLPSIRF_DATA_LENGTH];
}
NL_PSIRF_MSG;

typedef struct
{
    U8              msg_id;
    NL_PSIRF_MSG_TYPE       msg_type;
    U8              rtnid;
    U8              data[STDNLPSIRF_DATA_LENGTH];
    U8              data_length;
}
NL_PSIRF_MSG_SEND;

typedef enum
{
    MSG_CATEGORY_BC_NEW,
    MSG_CATEGORY_P2P_SELF,
    MSG_CATEGORY_P2P_FWD,
    MSG_CATEGORY_NETW,
    MSG_CATEGORY_EXPLORE,
    MSG_CATEGORY_NETW_CLEAR,
    MSG_CATEGORY_NOTHING,
}
NL_PSIRF_MSG_CATEGORY;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U8 NlPsiRfGetNextBcFid(void);
static U8 NlPsiRfGetNextP2PFid(void);
static U8 NlPsiRfGetNextMsgToSendId(void);
static void NlPsiRfClearOutgoingMsgId(void);

static void NlPsiRfOnExit(void);
static void NlPsiRfOnEntry(NL_PSIRF_STATE state, U32 delay);

static void NlPsiRfOnTimeOut(VPTR data_ptr);
#if STDNLPSIRF_ENABLE_REPEATER
static void NlPsiRfOnRevealTimeOut(VPTR data_ptr);
#endif

static void NlPsiRfSendMsg(NL_PSIRF_MSG* msg_ptr);

static BOOL NlPsiRfConvertMsgStructToArray(NL_PSIRF_MSG* msg_ptr, U8* data_ptr);
static BOOL NlPsiRfConvertMsgArrayToStruct(U8* data_ptr, NL_PSIRF_MSG* msg_ptr);
static void NlPsiRfPrintMsgStruct(STRING prestring, NL_PSIRF_MSG* msg_ptr, U8 rssi);

static void NlPsiRfRemoveAddress(U8 address);
static void NlPsiRfClearRoutingTable(void);
static void NlPsiRfRegisterRouteInclusion(U8 includant, U8 hop_node, U8 rssi);
static void NlPsiRfRegisterRoute(NL_PSIRF_MSG* msg_ptr, U8 rssi);
static U8 NlPsiRfDetermineRouteTo(U8 rtnid);

static NL_PSIRF_MSG_CATEGORY NlPsiRfCategorizeMsg(NL_PSIRF_MSG* msg_ptr);
static void NlPsiRfComposeAck(NL_PSIRF_MSG* msg_ptr);
static void NlPsiRfComposeSetNid(NL_PSIRF_MSG* msg_ptr);

static BOOL NlPsiRfAllowedToSendMsg(NL_PSIRF_MSG_SEND* outbox_msg_ptr);

static void NlPsiRfSetLinking(BOOL link_state);
static void NlPsiRfReportMsgSuccess(BOOL is_success);

static BOOL NlPsiRfHandleReceivedMsg(U8* frame_ptr, U8 rssi);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static NL_PSIRF_STATE                   nlpsirf_state;
static BOOL                             nlpsirf_in_linking;
static BOOL                             nlpsirf_in_linking_do_broadcast;

static NLPSIRF_DATA*                    nlpsirf_data_ptr;
static U32                              nlpsirf_network_hid_reveal;
static U8                               nlpsirf_inclusion_retry;
static U8                               nlpsirf_removed_nid;

static NLPSIRFHOOK_FRAME_RECEIVED       nlpsirf_frame_hook;
static NLPSIRFHOOK_LINK_STATUS          nlpsirf_link_status_hook;
static NLPSIRFHOOK_SET_NID              nlpsirf_set_nid_hook;
static NLPSIRFHOOK_ADD_REMOVE_NODE      nlpsirf_add_remove_node_hook;
static NLPSIRFHOOK_MSG_FEEDBACK         nlpsirf_msg_feedback_hook;
static NLPSIRFHOOK_SAVE_DATA            nlpsirf_save_data_hook;

static TASK_HNDL                        nlpsirf_timeout_task;

static U8                               nlpsirf_wait_for_p2p_timeout_jitter;

static U8                               nlpsirf_retry_counter;
static U16                              nlpsirf_msg_fail_counter;
static U16                              nlpsirf_conseq_fail_counter;

static NL_PSIRF_MSG                     nlpsirf_broadcast_msg;
static NL_PSIRF_MSG                     nlpsirf_outgoing_msg;
static U8                               nlpsirf_outgoing_msg_id_len;
static U8                               nlpsirf_outgoing_msg_id[4];

static BOOL                             nlpsirf_outbox_msg_unhandled;
static BOOL                             nlpsirf_save_after_keep_silent;

static U8                               nlpsirf_fid_bc;
static U8                               nlpsirf_fid_p2p;
static U8                               nlpsirf_msg_to_send_id;

static Q_HNDL                           nlpsirf_q_hndl;

static BOOL                             nlpsirf_fast_p2p;

#if STDNLPSIRF_ENABLE_REPEATER
static TASK_HNDL                        nlpsirf_timeout_reveal_task;
static NL_PSIRF_MSG                     nlpsirf_reveal_msg;
#endif

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static const STRING                     nlpsirf_states[] = {"NO_ADDRESS",
                                                            "IDLE",
                                                            "WAIT_FOR_EXPLORE_TO",
                                                            "WAIT_FOR_INCLUSION",
                                                            "WAIT_FOR_EXCLUSION",
                                                            "WAIT_FOR_BC_FWD_TO",
                                                            "WAIT_FOR_KEEP_SILENT_TO",
                                                            "WAIT_FOR_ACK",
                                                            "WAIT_FOR_ACK_SET_NID"};
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static U8 NlPsiRfGetNextBcFid(void)
{
    if(nlpsirf_fid_bc >= 0x0F)
    {
        nlpsirf_fid_bc = 1;
    }
    else
    {
        nlpsirf_fid_bc++;
    }
    return nlpsirf_fid_bc;
}
//------------------------------------------------------------------------------------------------//
static U8 NlPsiRfGetNextP2PFid(void)
{
    if(nlpsirf_fid_p2p >= 0x0F)
    {
        nlpsirf_fid_p2p = 1;
    }
    else
    {
        nlpsirf_fid_p2p++;
    }
    return nlpsirf_fid_p2p;
}
//------------------------------------------------------------------------------------------------//
static U8 NlPsiRfGetNextMsgToSendId(void)
{
    if(++nlpsirf_msg_to_send_id == 0)
    {
        nlpsirf_msg_to_send_id++;
    }
    return nlpsirf_msg_to_send_id;
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfClearOutgoingMsgId(void)
{
    MEMSET((VPTR)nlpsirf_outgoing_msg_id, 0, 4);
    nlpsirf_outgoing_msg_id_len = 0;
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfOnExit(void)
{
    CoreTask_Stop(nlpsirf_timeout_task);
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfOnEntry(NL_PSIRF_STATE state, U32 delay)
{
    switch(state)
    {
    case NL_PSIRF_NO_ADDRESS:
        NlPsiRfClearRoutingTable();
        break;
    case NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT:
        delay = (U32)STDNLPSIRF_BC_TIMEOUT_US;
        nlpsirf_network_hid_reveal = 0;
        break;
    case NL_PSIRF_IDLE:
        break;
    case NL_PSIRF_WAIT_FOR_ACK:
        nlpsirf_retry_counter++;
        //no break;
    case NL_PSIRF_WAIT_FOR_ACK_SET_NID:
        delay = (U32)STDNLPSIRF_P2P_TIMEOUT_US(nlpsirf_wait_for_p2p_timeout_jitter);
        break;
    case NL_PSIRF_WAIT_FOR_INCLUSION:
    case NL_PSIRF_WAIT_FOR_EXCLUSION:
        delay = (INCLUSION_RETRY + 1) * (U32)STDNLPSIRF_P2P_TIMEOUT_US(nlpsirf_wait_for_p2p_timeout_jitter);
        break;
    case NL_PSIRF_WAIT_FOR_BROADCAST_FORWARD_TIMEOUT:
    case NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT:
    default:
        //do nothing
        break;
    }
    if(delay > 0)
    {
        LOG_DEV("[NLDELAY] %d", PU32(delay / 1000));
        CoreTask_SetPeriod(nlpsirf_timeout_task, delay);
        CoreTask_Start(nlpsirf_timeout_task);
    }
    nlpsirf_state = state;
    
    LOG_DEV("[NLSTATE] %s", PCSTR(nlpsirf_states[nlpsirf_state]));
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfOnTimeOut(VPTR data_ptr)
{
    U8  i;
    
    switch(nlpsirf_state)
    {
    case NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT:
        //exit current state
        NlPsiRfOnExit();
        if(nlpsirf_network_hid_reveal > 0)
        {
            //side effects
            for(i=nlpsirf_outgoing_msg.length;i>=STDNLPSIRF_HEADER_LENGTH;i--)
            {
                nlpsirf_outgoing_msg.data[i-STDNLPSIRF_HEADER_LENGTH+STDNLPSIRF_HEADER_EXTRA_INCL] = nlpsirf_outgoing_msg.data[i-STDNLPSIRF_HEADER_LENGTH];
            }
            CoreConvert_U32ToU8Array(nlpsirf_data_ptr->device_hid, nlpsirf_outgoing_msg.data);
            nlpsirf_outgoing_msg.length += STDNLPSIRF_HEADER_EXTRA_INCL;
            nlpsirf_outgoing_msg.version = MSG_VERSION_V0;
            nlpsirf_outgoing_msg.msg_type.v0 = MSG_TYPE_V0_REQ_NID;
            nlpsirf_outgoing_msg.hid = nlpsirf_network_hid_reveal;
            nlpsirf_outgoing_msg.snid = 0;
            nlpsirf_outgoing_msg.rsnid = 0;
            nlpsirf_outgoing_msg.tnid = NlPsiRfDetermineRouteTo(STDBLPSIRF_NETWORK_MASTER_ADDR);
            nlpsirf_outgoing_msg.rtnid = STDBLPSIRF_NETWORK_MASTER_ADDR;
            nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
            nlpsirf_outgoing_msg.rssi = 0xFF;
            NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
            //enter state
            nlpsirf_inclusion_retry = INCLUSION_RETRY;
            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_INCLUSION, 0);
        }
        else if(nlpsirf_inclusion_retry > 0)
        {
            nlpsirf_outgoing_msg.fid = NlPsiRfGetNextBcFid();
            NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
            //enter state
            nlpsirf_inclusion_retry--;
            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT, 0);
        }
        else
        {
            //enter state
            NlPsiRfOnEntry(NL_PSIRF_NO_ADDRESS, 0);
            //report failure
            NlPsiRfReportMsgSuccess(FALSE);     // exploration failed
        }
        break;
    case NL_PSIRF_WAIT_FOR_INCLUSION:
    case NL_PSIRF_WAIT_FOR_EXCLUSION:
        //exit current state
        NlPsiRfOnExit();
        if(nlpsirf_inclusion_retry > 0)
        {
            nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
            NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
            //enter state
            nlpsirf_inclusion_retry--;
            NlPsiRfOnEntry(nlpsirf_state, 0);
        }
        else
        {
            //enter state
            if(nlpsirf_state == NL_PSIRF_WAIT_FOR_INCLUSION)
            {
                NlPsiRfOnEntry(NL_PSIRF_NO_ADDRESS, 0);
            }
            else
            {
                NlPsiRfOnEntry(NL_PSIRF_IDLE, 0);
            }
            //report failure
            NlPsiRfReportMsgSuccess(FALSE);     // inclusion/exclusion failed
        }
        break;
    case NL_PSIRF_WAIT_FOR_BROADCAST_FORWARD_TIMEOUT:
        //exit current state
        NlPsiRfOnExit();
        //side effects
        NlPsiRfSendMsg(&nlpsirf_broadcast_msg);
        //enter state
        NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, (U32)STDNLPSIRF_BC_TIMEOUT_US);
        break;
    case NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT:
        //exit current state
        NlPsiRfOnExit();
        //side effect & next state
        nlpsirf_broadcast_msg.fid = 0;               // clear fid of broadacst msg (used to categorize new msg)
        if(nlpsirf_save_after_keep_silent == TRUE)
        {
            nlpsirf_save_after_keep_silent = FALSE;
            if(nlpsirf_save_data_hook != NULL)
            {
                nlpsirf_save_data_hook();
            }
        }
        if(nlpsirf_outbox_msg_unhandled == TRUE)
        {
            //side effects
            nlpsirf_outbox_msg_unhandled = FALSE;
            nlpsirf_retry_counter--;
            NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
            //enter state
            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK, 0);
        }
        else
        {
            //enter state
            NlPsiRfOnEntry(NL_PSIRF_IDLE, 0);
        }
        break;
    case NL_PSIRF_WAIT_FOR_ACK:
        //exit current state
        NlPsiRfOnExit();
        //side effect & next state
        if(nlpsirf_retry_counter < STDNLPSIRF_RETRY_COUNT)
        {
            //side effects
            nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
            NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
            //enter state
            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK, 0);
        }
        else
        {
            //side effects
            nlpsirf_msg_fail_counter++;
            nlpsirf_conseq_fail_counter++;
            if(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR)
            {
                // indicate failed route
                nlpsirf_data_ptr->routing_table[nlpsirf_outgoing_msg.rtnid][0] = STDBLPSIRF_NETWORK_MASTER_ADDR;
            }
            else
            {
                // penalize the used route
                if(nlpsirf_data_ptr->routing_table[nlpsirf_outgoing_msg.tnid][1] > 10)
                {
                    nlpsirf_data_ptr->routing_table[nlpsirf_outgoing_msg.tnid][1] -= 10;
                }
            }
            //enter state
            if(nlpsirf_fast_p2p == TRUE)
            {
                NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, 1);
            }
            else
            {
                NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, STDNLPSIRF_P2P_TIMEOUT_US(nlpsirf_wait_for_p2p_timeout_jitter));
            }
            //report failure
            NlPsiRfReportMsgSuccess(FALSE);     // p2p msg failed
        }
        break;
    case NL_PSIRF_WAIT_FOR_ACK_SET_NID:
        //exit current state
        NlPsiRfOnExit();
        //side effect & next state
        if(nlpsirf_inclusion_retry > 0)
        {
            nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
            NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
            //enter state
            nlpsirf_inclusion_retry--;
            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK_SET_NID, 0);
        }
        else if((nlpsirf_outgoing_msg.length > STDNLPSIRF_HEADER_LENGTH_BASE) && (nlpsirf_outgoing_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL] > 0))
        {
            nlpsirf_outgoing_msg.tnid = NlPsiRfDetermineRouteTo(nlpsirf_outgoing_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL]);
            nlpsirf_outgoing_msg.rtnid = nlpsirf_outgoing_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL];
            nlpsirf_outgoing_msg.length = STDNLPSIRF_HEADER_LENGTH_BASE;
            nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
            NlPsiRfRemoveAddress(nlpsirf_outgoing_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL]);
            NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
            //enter state
            nlpsirf_inclusion_retry = INCLUSION_RETRY;
            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK_SET_NID, 0);
        }
        else
        {
            //no ACK received on remove, search for node
            CommNlPsiRf_SetLink(nlpsirf_removed_nid);
            //enter state
            NlPsiRfOnEntry(NL_PSIRF_IDLE, 0);
        }
        //report failure
        NlPsiRfReportMsgSuccess(FALSE);     // set nid failed
        break;
    case NL_PSIRF_NO_ADDRESS:
    case NL_PSIRF_IDLE:
    default:
        //should never happen
        CoreTask_Stop(nlpsirf_timeout_task);
        LOG_DBG("[NLERROR] unexpected timeout");
        break;
    }
}
//------------------------------------------------------------------------------------------------//
#if STDNLPSIRF_ENABLE_REPEATER
static void NlPsiRfOnRevealTimeOut(VPTR data_ptr)
{
    CoreTask_Stop(nlpsirf_timeout_reveal_task);
    NlPsiRfSendMsg(&nlpsirf_reveal_msg);
}
#endif
//------------------------------------------------------------------------------------------------//
static void NlPsiRfSendMsg(NL_PSIRF_MSG* msg_ptr)
{
    U8  data[STDDLLPSIRF_FRAME_LENGTH];
    
    NlPsiRfPrintMsgStruct("SND ->", msg_ptr, 0xFF);
    
    if(NlPsiRfConvertMsgStructToArray(msg_ptr, data))
    {
        CommDllPsiRf_SendFrame(data);
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL NlPsiRfConvertMsgStructToArray(NL_PSIRF_MSG* msg_ptr, U8* data_ptr)
{
    U32     data;
    
    if((msg_ptr->version == MSG_VERSION_V0) &&
       (msg_ptr->length >= STDNLPSIRF_HEADER_LENGTH_BASE) &&
       (msg_ptr->length < STDDLLPSIRF_FRAME_LENGTH))
    {
        data_ptr[0] = msg_ptr->length;
        data_ptr[1] = (U8)((msg_ptr->version & 0x0F) << 4) | (U8)(msg_ptr->msg_type.v0 & 0x0F);
        CoreConvert_U32ToU8Array(msg_ptr->hid, &data_ptr[2]);
        
        data = ((U32)(msg_ptr->snid & 0x1F) << 27) |
            ((U32)(msg_ptr->tnid & 0x1F) << 22) |
            ((U32)(msg_ptr->rsnid & 0x1F) << 17) |
            ((U32)(msg_ptr->rtnid & 0x1F) << 12) |
            ((U32)(msg_ptr->fid & 0x0F) << 8) |
            ((U32)(msg_ptr->rssi & 0xFF) << 0);
        CoreConvert_U32ToU8Array(data, &data_ptr[6]);
        
        MEMCPY(&data_ptr[STDNLPSIRF_HEADER_LENGTH], msg_ptr->data, msg_ptr->length-STDNLPSIRF_HEADER_LENGTH_BASE);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL NlPsiRfConvertMsgArrayToStruct(U8* data_ptr, NL_PSIRF_MSG* msg_ptr)
{
    U32     data;
    
    msg_ptr->length = data_ptr[0];
    if((msg_ptr->length < STDNLPSIRF_HEADER_LENGTH_BASE) || (msg_ptr->length >= STDDLLPSIRF_FRAME_LENGTH))
    {
        return FALSE;
    }
    
    msg_ptr->version = (NL_PSIRF_MSG_VERSION)(data_ptr[1] >> 4);
    if(msg_ptr->version == MSG_VERSION_V0)
    {
        msg_ptr->version = MSG_VERSION_V0;
        msg_ptr->msg_type.v0 = (NL_PSIRF_MSG_TYPE_V0)(data_ptr[1] & 0x0F);
        msg_ptr->hid = CoreConvert_U8ArrayToU32(&data_ptr[2]);
        
        data = CoreConvert_U8ArrayToU32(&data_ptr[6]);
        msg_ptr->snid = ((data >> 27) & 0x1F);
        msg_ptr->tnid = ((data >> 22) & 0x1F);
        msg_ptr->rsnid = ((data >> 17) & 0x1F);
        msg_ptr->rtnid = ((data >> 12) & 0x1F);
        msg_ptr->fid = ((data >> 8) & 0x0F);
        msg_ptr->rssi = ((data >> 0) & 0xFF);
        
        MEMCPY(msg_ptr->data, &data_ptr[STDNLPSIRF_HEADER_LENGTH], msg_ptr->length-STDNLPSIRF_HEADER_LENGTH_BASE);
        
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfPrintMsgStruct(STRING prestring, NL_PSIRF_MSG* msg_ptr, U8 rssi)
{
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & LOG_LEVEL_DEBUG
    const STRING  types[] = {"LINK", "EXPL", "REVL", "REQN", "SETN", "REMN", "ACKN", "APPL"};
#endif
    
    if(msg_ptr->version == MSG_VERSION_V0)
    {
        LOG_DBG("%s [0x%02x] %s - %08x : %02d->%02d (%02d->%02d) - %2d 0x%02x - (%d) %02x",
                PCSTR(prestring),
                PU8(rssi),
                PCSTR(types[msg_ptr->msg_type.type]),
                PU32(msg_ptr->hid),
                PU8(msg_ptr->snid),
                PU8(msg_ptr->tnid),
                PU8(msg_ptr->rsnid),
                PU8(msg_ptr->rtnid),
                PU8(msg_ptr->fid),
                PU8(msg_ptr->rssi),
                PU8(msg_ptr->length - STDNLPSIRF_HEADER_LENGTH_BASE),
                PU8A(msg_ptr->data, msg_ptr->length - STDNLPSIRF_HEADER_LENGTH_BASE));
    }
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfRemoveAddress(U8 address)
{
    U8  i;
    
    if(address > STDBLPSIRF_NETWORK_MASTER_ADDR)
    {
        if((nlpsirf_add_remove_node_hook == NULL) ||
           (nlpsirf_add_remove_node_hook(address,
                                            FALSE,
                                            NULL,
                                            NULL) == FALSE))
        {
            return;
        }
        nlpsirf_removed_nid = address;
        nlpsirf_data_ptr->routing_table[address][0] = 0;
        nlpsirf_data_ptr->routing_table[address][1] = 0;
        for(i=STDBLPSIRF_NETWORK_MASTER_ADDR+1; i<STDNLPSIRF_MAX_NODES; i++)
        {
            if(nlpsirf_data_ptr->routing_table[i][0] == address)
            {
                nlpsirf_data_ptr->routing_table[i][0] = STDBLPSIRF_NETWORK_MASTER_ADDR;      // indicate node exists, no route
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfClearRoutingTable(void)
{
    MEMSET((VPTR)nlpsirf_data_ptr->routing_table, 0, sizeof(nlpsirf_data_ptr->routing_table));
}        
//------------------------------------------------------------------------------------------------//
static void NlPsiRfRegisterRouteInclusion(U8 includant, U8 hop_node, U8 rssi)
{
    if(hop_node == 0)
    {
        nlpsirf_data_ptr->routing_table[includant][0] = includant;
        nlpsirf_data_ptr->routing_table[includant][1] = rssi;
    }
    else
    {
        nlpsirf_data_ptr->routing_table[includant][0] = hop_node;
        nlpsirf_data_ptr->routing_table[hop_node][1] = rssi;
    }
    
    nlpsirf_wait_for_p2p_timeout_jitter ^= rssi;
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfRegisterRoute(NL_PSIRF_MSG* msg_ptr, U8 rssi)
{
    if((msg_ptr->snid == 0) || (msg_ptr->rssi == 0))
    {
        return;
    }
    
    if(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR)
    {
        if(msg_ptr->rsnid != nlpsirf_removed_nid)
        {
            if(msg_ptr->tnid == STDBLPSIRF_NETWORK_MASTER_ADDR)
            {
                nlpsirf_data_ptr->routing_table[msg_ptr->rsnid][0] = msg_ptr->snid;
            }
        }
        if(msg_ptr->snid != nlpsirf_removed_nid)
        {
            if(nlpsirf_data_ptr->routing_table[msg_ptr->snid][0] == 0)
            {
                nlpsirf_data_ptr->routing_table[msg_ptr->snid][0] = STDBLPSIRF_NETWORK_MASTER_ADDR;
            }
            nlpsirf_data_ptr->routing_table[msg_ptr->snid][1] = rssi;
        }
    }
    else
    {
        if(msg_ptr->rsnid == STDBLPSIRF_NETWORK_MASTER_ADDR)
        {
            nlpsirf_data_ptr->routing_table[msg_ptr->snid][0] = msg_ptr->rssi;
        }
        nlpsirf_data_ptr->routing_table[msg_ptr->snid][1] = rssi;
    }
    
    nlpsirf_wait_for_p2p_timeout_jitter ^= rssi;
}
//------------------------------------------------------------------------------------------------//
static U8 NlPsiRfDetermineRouteTo(U8 rtnid)
{
    U8  i;
    U8  tnid = 0;
    U8  compare = 0;
    
    if((nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR) && (rtnid > STDBLPSIRF_NETWORK_MASTER_ADDR))
    {
        if(nlpsirf_data_ptr->routing_table[rtnid][0] > STDBLPSIRF_NETWORK_MASTER_ADDR)
        {
            return nlpsirf_data_ptr->routing_table[rtnid][0];
        }
    }
    else if((nlpsirf_data_ptr->network_address != STDBLPSIRF_NETWORK_MASTER_ADDR) && (rtnid == STDBLPSIRF_NETWORK_MASTER_ADDR))
    {
        if(nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][1] < 0x80)
        {
            compare = (U16)nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][1] + 5;
            tnid = STDBLPSIRF_NETWORK_MASTER_ADDR;
            for(i=STDBLPSIRF_NETWORK_MASTER_ADDR+1;i<STDNLPSIRF_MAX_NODES;i++)
            {
                if((nlpsirf_data_ptr->routing_table[i][0] > compare) && (nlpsirf_data_ptr->routing_table[i][1] > compare))
                {
                    compare = MIN(nlpsirf_data_ptr->routing_table[i][0], nlpsirf_data_ptr->routing_table[i][1]);
                    tnid = i;
                }
            }
            if(compare < 0x20)
            {
                CommNlPsiRf_SendExplore(NULL, 0);
                return rtnid;
            }
            return tnid;
        }
    }
    return rtnid;
}
//------------------------------------------------------------------------------------------------//
static NL_PSIRF_MSG_CATEGORY NlPsiRfCategorizeMsg(NL_PSIRF_MSG* msg_ptr)
{
    if(msg_ptr->msg_type.v0 == MSG_TYPE_V0_EXPLORE)
    {
        if((nlpsirf_data_ptr->network_hid > 0) &&
           (((nlpsirf_in_linking == TRUE) && (msg_ptr->snid == 1)) ||                   // explore msg from new device that wants to be included
            ((nlpsirf_data_ptr->network_hid == msg_ptr->hid) && (msg_ptr->snid > 1))))  // explore msg from device in network that wants to find back the network master
        {
            return MSG_CATEGORY_EXPLORE;
        }
    }
    else if(msg_ptr->hid == nlpsirf_data_ptr->network_hid)
    {
        if(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR)
        {
            if(msg_ptr->tnid == STDBLPSIRF_NETWORK_MASTER_ADDR)
            {
                return MSG_CATEGORY_P2P_SELF;
            }
        }
        else if((msg_ptr->tnid == 0) && (msg_ptr->rtnid == 0))
        {
            if((msg_ptr->msg_type.v0 == MSG_TYPE_V0_LINK) || (msg_ptr->msg_type.v0 == MSG_TYPE_V0_APPLIC))
            {
                if(msg_ptr->fid != nlpsirf_broadcast_msg.fid)
                {
                    return MSG_CATEGORY_BC_NEW;
                }
            }
            else if(msg_ptr->msg_type.v0 == MSG_TYPE_V0_SET_NID)
            {
                if(msg_ptr->length == STDNLPSIRF_HEADER_LENGTH_BASE)
                {
                    return MSG_CATEGORY_NETW_CLEAR;
                }
                else if(CoreConvert_U8ArrayToU32(msg_ptr->data) == nlpsirf_data_ptr->device_hid)
                {
                    return MSG_CATEGORY_P2P_SELF;
                }
            }
        }
        else if(msg_ptr->tnid == nlpsirf_data_ptr->network_address)
        {
            if(msg_ptr->rtnid == nlpsirf_data_ptr->network_address)
            {
                return MSG_CATEGORY_P2P_SELF;
            }
            return MSG_CATEGORY_P2P_FWD;
        }
        return MSG_CATEGORY_NETW;
    }
    else if(nlpsirf_data_ptr->network_hid == 0)
    {
        if(((msg_ptr->msg_type.v0 == MSG_TYPE_V0_REVEAL) || (msg_ptr->msg_type.v0 == MSG_TYPE_V0_SET_NID)) &&
           (msg_ptr->tnid == 0) && (msg_ptr->rtnid == 0) &&
           (CoreConvert_U8ArrayToU32(msg_ptr->data) == nlpsirf_data_ptr->device_hid))
        {
            return MSG_CATEGORY_P2P_SELF;
        }
    }
    return MSG_CATEGORY_NOTHING;
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfComposeAck(NL_PSIRF_MSG* msg_ptr)
{
    U8  rtnid = msg_ptr->rsnid;
    
    msg_ptr->length = STDNLPSIRF_HEADER_LENGTH_BASE;
    msg_ptr->msg_type.v0 = MSG_TYPE_V0_ACK;
    
    msg_ptr->rsnid = nlpsirf_data_ptr->network_address;
    msg_ptr->snid = nlpsirf_data_ptr->network_address;
    msg_ptr->rtnid = rtnid;
    msg_ptr->tnid = NlPsiRfDetermineRouteTo(rtnid);
    
    msg_ptr->rssi = 0xFF;
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfComposeSetNid(NL_PSIRF_MSG* msg_ptr)
{
    msg_ptr->msg_type.v0 = MSG_TYPE_V0_SET_NID;
    
    msg_ptr->tnid = msg_ptr->snid;
    msg_ptr->rtnid = msg_ptr->rsnid;
    msg_ptr->snid = STDBLPSIRF_NETWORK_MASTER_ADDR;
    msg_ptr->rsnid = STDBLPSIRF_NETWORK_MASTER_ADDR;
    
    msg_ptr->fid = NlPsiRfGetNextP2PFid();
    msg_ptr->rssi = 0xFF;
    
    MEMCPY((VPTR)&nlpsirf_outgoing_msg, (VPTR)msg_ptr, sizeof(NL_PSIRF_MSG));
}
//------------------------------------------------------------------------------------------------//
static BOOL NlPsiRfAllowedToSendMsg(NL_PSIRF_MSG_SEND* outbox_msg_ptr)
{
    if(nlpsirf_state == NL_PSIRF_NO_ADDRESS)
    {
        return (BOOL)(outbox_msg_ptr->msg_type.v0 == MSG_TYPE_V0_EXPLORE);
    }
    else if(nlpsirf_state == NL_PSIRF_IDLE)
    {
        switch(outbox_msg_ptr->msg_type.v0)
        {
        case MSG_TYPE_V0_EXPLORE:
            return TRUE;
        case MSG_TYPE_V0_APPLIC:
        case MSG_TYPE_V0_LINK:
            return (BOOL)((nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR) || (outbox_msg_ptr->rtnid != 0));
        case MSG_TYPE_V0_REM_NID:
            return (BOOL)(nlpsirf_data_ptr->network_address > STDBLPSIRF_NETWORK_MASTER_ADDR);
        case MSG_TYPE_V0_SET_NID:
            return (BOOL)(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR);
        default:
            return FALSE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfSetLinking(BOOL link_state)
{
    nlpsirf_in_linking = link_state;
    if(nlpsirf_link_status_hook != NULL)
    {
        nlpsirf_link_status_hook(link_state);
    }
}
//------------------------------------------------------------------------------------------------//
static void NlPsiRfReportMsgSuccess(BOOL is_success)
{
    U8  i;
    
    if(nlpsirf_msg_feedback_hook != NULL)
    {
        for(i=0; i<nlpsirf_outgoing_msg_id_len; i++)
        {
            nlpsirf_msg_feedback_hook(nlpsirf_outgoing_msg_id[i], is_success);
        }
    }
    NlPsiRfClearOutgoingMsgId();
}
//------------------------------------------------------------------------------------------------//
static BOOL NlPsiRfHandleReceivedMsg(U8* frame_ptr, U8 rssi)
{
    NL_PSIRF_MSG    incoming_msg;
    BOOL            response_available = FALSE;
    U8              len[4];
    U8*             data_ptr;
    U8              i;
    NL_PSIRF_STATE  shadow_prev_state;
    
    if(NlPsiRfConvertMsgArrayToStruct(frame_ptr, &incoming_msg))
    {
        NlPsiRfPrintMsgStruct("RCV <-", &incoming_msg, rssi);
        switch(NlPsiRfCategorizeMsg(&incoming_msg))
        {
        case MSG_CATEGORY_BC_NEW:
            switch(nlpsirf_state)
            {
            case NL_PSIRF_WAIT_FOR_EXCLUSION:
                // waiting for exclusion, don't react on BC
                return FALSE;
            case NL_PSIRF_WAIT_FOR_ACK:
                nlpsirf_outbox_msg_unhandled = TRUE;
                break;
            }
            //exit current state
            NlPsiRfOnExit();
            //clear routing table
            NlPsiRfClearRoutingTable();
            NlPsiRfRegisterRoute(&incoming_msg, rssi);      // slave BC NEW message
            nlpsirf_save_after_keep_silent = TRUE;
            //register BC msg
            MEMCPY((VPTR)&nlpsirf_broadcast_msg, (VPTR)&incoming_msg, sizeof(NL_PSIRF_MSG));
            nlpsirf_broadcast_msg.snid = nlpsirf_data_ptr->network_address;
            nlpsirf_broadcast_msg.rssi = rssi;
            //side effects
            //determine next state
#if STDNLPSIRF_ENABLE_REPEATER
            if(incoming_msg.snid == STDBLPSIRF_NETWORK_MASTER_ADDR)
            {
                NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_BROADCAST_FORWARD_TIMEOUT, STDNLPSIRF_BC_WAIT_TIMEOUT(nlpsirf_data_ptr->network_address - incoming_msg.snid));
            }
            else
#endif
            {
                NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, (U32)STDNLPSIRF_BC_TIMEOUT_US + STDNLPSIRF_BC_WAIT_TIMEOUT(nlpsirf_data_ptr->network_address - incoming_msg.snid));
            }
            if(incoming_msg.msg_type.v0 == MSG_TYPE_V0_LINK)
            {
                if(incoming_msg.data[0] == 0)
                {
                    NlPsiRfSetLinking(FALSE);           // on slave, master BC linking OFF
                }
                else if(incoming_msg.data[0] == 1)
                {
                    NlPsiRfSetLinking(TRUE);            // on slave, master BC linking ON
                }
                else if(incoming_msg.data[0] == nlpsirf_data_ptr->network_address)
                {
                    CommNlPsiRf_SetLink(nlpsirf_data_ptr->network_address);
                }
            }
            else
            {
                if(nlpsirf_frame_hook != NULL)
                {
                    if(incoming_msg.data[0] == 0)
                    {
                        nlpsirf_frame_hook(incoming_msg.rsnid, incoming_msg.rtnid, &incoming_msg.data[1], incoming_msg.length-STDNLPSIRF_HEADER_LENGTH);
                    }
                    else
                    {
                        len[0] = (incoming_msg.data[0] >> 4);
                        len[1] = (incoming_msg.data[0] & 0x0F);
                        len[2] = (incoming_msg.data[1] >> 4);
                        len[3] = (incoming_msg.data[1] & 0x0F);
                        data_ptr = &incoming_msg.data[2];
                        for(i=0;i<4;i++)
                        {
                            if(len[i] > 0)
                            {
                                nlpsirf_frame_hook(incoming_msg.rsnid, incoming_msg.rtnid, data_ptr, len[i]);
                                data_ptr += len[i];
                            }
                        }
                    }
                }
            }
            break;
        case MSG_CATEGORY_P2P_SELF:
            NlPsiRfRegisterRoute(&incoming_msg, rssi);      // master & slave P2P SELF msgs
            switch(incoming_msg.msg_type.v0)
            {
            case MSG_TYPE_V0_REVEAL:
                if(nlpsirf_state == NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT)
                {
                    if((nlpsirf_network_hid_reveal > 0) && (nlpsirf_network_hid_reveal != incoming_msg.hid))
                    {
                        //exit current state
                        NlPsiRfOnExit();
                        //side effects
                        nlpsirf_network_hid_reveal = 0;
                        //enter state
                        NlPsiRfOnEntry(NL_PSIRF_NO_ADDRESS, 0);
                        //report failure
                        NlPsiRfReportMsgSuccess(FALSE);     // explore failed - multiple networks
                    }
                    else
                    {
                        nlpsirf_network_hid_reveal = incoming_msg.hid;
                    }
                }
                break;
            case MSG_TYPE_V0_REQ_NID:
            case MSG_TYPE_V0_REM_NID:
                // master only
                if((nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR) &&
                   ((nlpsirf_state == NL_PSIRF_IDLE) || (nlpsirf_state == NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT)) &&
                   (nlpsirf_in_linking == TRUE))
                {
                    if(incoming_msg.msg_type.v0 == MSG_TYPE_V0_REQ_NID)
                    {
                        incoming_msg.length -= STDNLPSIRF_HEADER_LENGTH_INCL;
                        if((nlpsirf_add_remove_node_hook == NULL) ||
                           (nlpsirf_add_remove_node_hook(incoming_msg.rsnid,
                                                            TRUE,
                                                            &incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL],
                                                            &incoming_msg.length) == FALSE))
                        {
                            break;
                        }
                        incoming_msg.length += STDNLPSIRF_HEADER_LENGTH_INCL;
                        if(nlpsirf_removed_nid == incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL])
                        {
                            nlpsirf_removed_nid = 0;    // clear remove node
                        }
                        NlPsiRfRegisterRouteInclusion(incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL],
                                                      incoming_msg.snid,
                                                      rssi);
                    }
                    else
                    {
                        incoming_msg.length = STDNLPSIRF_HEADER_LENGTH_BASE;
                        NlPsiRfRemoveAddress(incoming_msg.rsnid);
                    }
                    //exit current state
                    NlPsiRfOnExit();
                    //compose SET NID msg
                    NlPsiRfComposeSetNid(&incoming_msg);
                    response_available = TRUE;
                    //enter state
                    nlpsirf_inclusion_retry = INCLUSION_RETRY;
                    NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK_SET_NID, 0);
                    break;
                }
                break;
            case MSG_TYPE_V0_SET_NID:
                // slave only
                if(nlpsirf_data_ptr->network_address != STDBLPSIRF_NETWORK_MASTER_ADDR)
                {
                    if((incoming_msg.length <= STDNLPSIRF_HEADER_LENGTH_INCL) || (incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL] == 0))
                    {
                        //exit current state
                        NlPsiRfOnExit();
                        //side effects
                        //compose acknowledge before clearing network id and address
                        NlPsiRfComposeAck(&incoming_msg);
                        response_available = TRUE;
                        
                        nlpsirf_data_ptr->network_address = 0;
                        nlpsirf_data_ptr->network_hid = 0;
                        if(nlpsirf_set_nid_hook != NULL)
                        {
                            nlpsirf_set_nid_hook(NULL, 0);
                        }
                        
                        //enter state
                        shadow_prev_state = nlpsirf_state;
                        NlPsiRfOnEntry(NL_PSIRF_NO_ADDRESS, 0);
                        
                        //report failure/success
                        switch(shadow_prev_state)
                        {
                        case NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT:
                        case NL_PSIRF_WAIT_FOR_INCLUSION:
                            NlPsiRfReportMsgSuccess(FALSE);     // explore or inclusion failed - set nid 0
                            break;
                        case NL_PSIRF_WAIT_FOR_EXCLUSION:
                            NlPsiRfReportMsgSuccess(TRUE);      // exclusion succeeded
                            break;
                        }
                        break;
                    }
                    else if(incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL] > STDBLPSIRF_NETWORK_MASTER_ADDR)
                    {
                        if(nlpsirf_state == NL_PSIRF_WAIT_FOR_INCLUSION)
                        {
                            //exit current state
                            NlPsiRfOnExit();
                            //side effects
                            nlpsirf_data_ptr->network_address = incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL];
                            nlpsirf_data_ptr->network_hid = incoming_msg.hid;
                            if(nlpsirf_set_nid_hook != NULL)
                            {
                                nlpsirf_set_nid_hook(&incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL], incoming_msg.length-STDNLPSIRF_HEADER_LENGTH_INCL);
                            }
                            
                            //compose acknowledge after registering network id and address
                            NlPsiRfComposeAck(&incoming_msg);
                            response_available = TRUE;
                            
                            //enter state
                            NlPsiRfOnEntry(NL_PSIRF_IDLE, 0);
                            
                            //report success
                            NlPsiRfReportMsgSuccess(TRUE);      // inclusion succeeded
                        }
                        else if((nlpsirf_data_ptr->network_address == incoming_msg.data[STDNLPSIRF_HEADER_EXTRA_INCL]) && (nlpsirf_data_ptr->network_hid == incoming_msg.hid))
                        {
                            //re-acknowledge
                            NlPsiRfComposeAck(&incoming_msg);
                            response_available = TRUE;
                        }
                        break;
                    }
                }
                break;
            case MSG_TYPE_V0_ACK:
                // check if msg is expected ACK
                if((((nlpsirf_state == NL_PSIRF_WAIT_FOR_ACK) && (nlpsirf_outgoing_msg.rtnid == incoming_msg.rsnid)) ||
                    (nlpsirf_state == NL_PSIRF_WAIT_FOR_ACK_SET_NID)) &&
                   (nlpsirf_outgoing_msg.fid == incoming_msg.fid))
                {
                    //exit current state
                    NlPsiRfOnExit();
                    //side effects
                    if(nlpsirf_state == NL_PSIRF_WAIT_FOR_ACK_SET_NID)
                    {
                        if(nlpsirf_removed_nid == incoming_msg.rsnid)
                        {
                            nlpsirf_removed_nid = 0;
                        }
                        //enter state
                        NlPsiRfOnEntry(NL_PSIRF_IDLE, 0);
                    }
                    else
                    {
                        nlpsirf_conseq_fail_counter = 0;
                        //enter state
                        if(nlpsirf_fast_p2p == TRUE)
                        {
                            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, 1);
                        }
                        else
                        {
                            NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, STDNLPSIRF_P2P_TIMEOUT_US(nlpsirf_wait_for_p2p_timeout_jitter));
                        }
                    }
                    //report success
                    NlPsiRfReportMsgSuccess(TRUE);      // p2p msg succeeded
                }
                break;
            case MSG_TYPE_V0_APPLIC:
            case MSG_TYPE_V0_LINK:
                if(incoming_msg.msg_type.v0 == MSG_TYPE_V0_APPLIC)
                {
                    if(nlpsirf_frame_hook != NULL)
                    {
                        if(incoming_msg.data[0] == 0)
                        {
                            nlpsirf_frame_hook(incoming_msg.rsnid, incoming_msg.rtnid, &incoming_msg.data[1], incoming_msg.length-STDNLPSIRF_HEADER_LENGTH);
                        }
                        else
                        {
                            len[0] = (incoming_msg.data[0] >> 4);
                            len[1] = (incoming_msg.data[0] & 0x0F);
                            len[2] = (incoming_msg.data[1] >> 4);
                            len[3] = (incoming_msg.data[1] & 0x0F);
                            data_ptr = &incoming_msg.data[2];
                            for(i=0;i<4;i++)
                            {
                                if(len[i] > 0)
                                {
                                    nlpsirf_frame_hook(incoming_msg.rsnid, incoming_msg.rtnid, data_ptr, len[i]);
                                    data_ptr += len[i];
                                }
                            }
                        }
                    }
                }
                else if(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR)
                {
                    if(incoming_msg.data[0] <= 1)
                    {
                        if(nlpsirf_in_linking != (BOOL)incoming_msg.data[0])
                        {
                            NlPsiRfSetLinking((BOOL)incoming_msg.data[0]);      // on master, received linking request
                            nlpsirf_in_linking_do_broadcast = TRUE;
                        }
                    }
                    else if(nlpsirf_removed_nid == incoming_msg.rsnid)
                    {
                        //node responded while it was expected to be removed, retry removal
                        incoming_msg.length = STDNLPSIRF_HEADER_LENGTH_BASE;
                        NlPsiRfRemoveAddress(incoming_msg.rsnid);
                        //exit current state
                        NlPsiRfOnExit();
                        //compose SET NID msg
                        NlPsiRfComposeSetNid(&incoming_msg);
                        response_available = TRUE;
                        //enter state
                        nlpsirf_inclusion_retry = INCLUSION_RETRY;
                        NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK_SET_NID, 0);
                    }
                }
                //acknowledge
                NlPsiRfComposeAck(&incoming_msg);
                response_available = TRUE;
                break;
            case MSG_TYPE_V0_EXPLORE:
            default:
                //should not happen (if it does: discard)
                break;
            }
            break;
        case MSG_CATEGORY_P2P_FWD:
            NlPsiRfRegisterRoute(&incoming_msg, rssi);      // slave P2P FWD msg
#if STDNLPSIRF_ENABLE_REPEATER
            incoming_msg.tnid = incoming_msg.rtnid;
            incoming_msg.snid = nlpsirf_data_ptr->network_address;
            incoming_msg.rssi = rssi;
            //report response available
            response_available = TRUE;
#endif
            break;
        case MSG_CATEGORY_NETW:
            NlPsiRfRegisterRoute(&incoming_msg, rssi);      // master & slave NETW mag
            break;
        case MSG_CATEGORY_EXPLORE:
#if STDNLPSIRF_ENABLE_REPEATER
            if(CoreTask_IsTaskRunning(nlpsirf_timeout_reveal_task) == TRUE)
            {
                // if reveal task is already running, discard msg
                break;
            }
            //compose reveal msg
            nlpsirf_reveal_msg.length = STDNLPSIRF_HEADER_LENGTH_INCL;
            nlpsirf_reveal_msg.version = MSG_VERSION_V0;
            nlpsirf_reveal_msg.msg_type.v0 = MSG_TYPE_V0_REVEAL;
            nlpsirf_reveal_msg.hid = nlpsirf_data_ptr->network_hid;
            nlpsirf_reveal_msg.snid = nlpsirf_data_ptr->network_address;
            nlpsirf_reveal_msg.tnid = 0;
            nlpsirf_reveal_msg.rsnid = STDBLPSIRF_NETWORK_MASTER_ADDR;
            nlpsirf_reveal_msg.rtnid = 0;
            nlpsirf_reveal_msg.fid = NlPsiRfGetNextBcFid();
            nlpsirf_reveal_msg.rssi = nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][1];
            CoreConvert_U32ToU8Array(incoming_msg.hid, nlpsirf_reveal_msg.data);
            //start reveal timeout
            CoreTask_SetPeriod(nlpsirf_timeout_reveal_task, STDNLPSIRF_BC_WAIT_TIMEOUT(nlpsirf_data_ptr->network_address));
            CoreTask_Start(nlpsirf_timeout_reveal_task);
#endif
            break;
        case MSG_CATEGORY_NETW_CLEAR:
            //exit current state
            NlPsiRfOnExit();
            //side effects
#if STDNLPSIRF_ENABLE_REPEATER
            //abuse reveal msg to propagate network clear msg.
            //Normal broadcast forwarding does not work because node will be removed from network by the time of forwarding
            MEMCPY((VPTR)&nlpsirf_reveal_msg, (VPTR)&incoming_msg, sizeof(NL_PSIRF_MSG));
            nlpsirf_reveal_msg.snid = nlpsirf_data_ptr->network_address;
            nlpsirf_reveal_msg.rssi = 0;
            //start reveal timeout
            CoreTask_SetPeriod(nlpsirf_timeout_reveal_task, STDNLPSIRF_BC_WAIT_TIMEOUT(nlpsirf_data_ptr->network_address));
            CoreTask_Start(nlpsirf_timeout_reveal_task);
#endif
            nlpsirf_data_ptr->network_address = 0;
            nlpsirf_data_ptr->network_hid = 0;
            if(nlpsirf_set_nid_hook != NULL)
            {
                nlpsirf_set_nid_hook(NULL, 0);
            }
            
            //enter state
            shadow_prev_state = nlpsirf_state;
            NlPsiRfOnEntry(NL_PSIRF_NO_ADDRESS, 0);
            
            //report failure/success
            switch(shadow_prev_state)
            {
            case NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT:
            case NL_PSIRF_WAIT_FOR_INCLUSION:
                NlPsiRfReportMsgSuccess(FALSE);     // explore or inclusion failed - set nid 0
                break;
            case NL_PSIRF_WAIT_FOR_EXCLUSION:
                NlPsiRfReportMsgSuccess(TRUE);      // exclusion succeeded
                break;
            }
            break;
        case MSG_CATEGORY_NOTHING:
        default:
            // do nothing
            break;
        }
        
        if(response_available == TRUE)
        {
            NlPsiRfPrintMsgStruct("RPL +>", &incoming_msg, 0xFF);
            if(NlPsiRfConvertMsgStructToArray(&incoming_msg, frame_ptr))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_Init(NLPSIRF_DATA* data_ptr)
{
    U8  i;
    
    MODULE_INIT_ONCE();
    
    nlpsirf_timeout_task = CoreTask_RegisterTask((U32)STDNLPSIRF_BC_TIMEOUT_US,
                                                  NlPsiRfOnTimeOut,
                                                  NULL,
                                                  128,
                                                  "CommNlPsiRfTo");
    
#if STDNLPSIRF_ENABLE_REPEATER
    nlpsirf_timeout_reveal_task = CoreTask_RegisterTask((U32)STDNLPSIRF_BC_TIMEOUT_US,
                                                         NlPsiRfOnRevealTimeOut,
                                                         NULL,
                                                         128,
                                                         "CommNlPsiRfReveal");
#endif
    
    nlpsirf_in_linking = FALSE;
    nlpsirf_in_linking_do_broadcast = FALSE;
    
    nlpsirf_data_ptr = data_ptr;
    
    nlpsirf_wait_for_p2p_timeout_jitter = 0;
    
    if((nlpsirf_data_ptr->network_address == 0) || (nlpsirf_data_ptr->network_address >= STDNLPSIRF_MAX_NODES))
    {
        nlpsirf_data_ptr->network_address = 0;
        nlpsirf_data_ptr->network_hid = 0;
        NlPsiRfOnEntry(NL_PSIRF_NO_ADDRESS, 0);
    }
    else
    {
        if(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR)
        {
            nlpsirf_data_ptr->network_hid = nlpsirf_data_ptr->device_hid;
            nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][0] = 1;
            nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][1] = 0xFF;
        }
        NlPsiRfOnEntry(NL_PSIRF_IDLE, 0);
        
        for(i=0; i<STDNLPSIRF_MAX_NODES; i++)
        {
            nlpsirf_wait_for_p2p_timeout_jitter ^= nlpsirf_data_ptr->routing_table[i][1];
        }
    }
    
    nlpsirf_frame_hook = NULL;
    nlpsirf_link_status_hook = NULL;
    nlpsirf_set_nid_hook = NULL;
    nlpsirf_add_remove_node_hook = NULL;
    nlpsirf_msg_feedback_hook = NULL;
    nlpsirf_save_data_hook = NULL;
    
    CommDllPsiRf_RegisterFrameHook(NlPsiRfHandleReceivedMsg);
    
    nlpsirf_retry_counter = 0;
    nlpsirf_msg_fail_counter = 0;
    nlpsirf_conseq_fail_counter = 0;
    nlpsirf_removed_nid = 0;
    nlpsirf_outbox_msg_unhandled = FALSE;
    nlpsirf_save_after_keep_silent = FALSE;
    
    nlpsirf_fid_bc = 0;
    nlpsirf_fid_p2p = 0;
    nlpsirf_msg_to_send_id = 0;
    
    NlPsiRfClearOutgoingMsgId();
    
    nlpsirf_fast_p2p = FALSE;
    
    nlpsirf_q_hndl = CoreQ_Register(STDNLPSIRF_QUEUE_SIZE, SIZEOF(NL_PSIRF_MSG_SEND), "CommNlPsiRf_Queue");
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_RegisterFrameHook(NLPSIRFHOOK_FRAME_RECEIVED hook)
{
    nlpsirf_frame_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_RegisterLinkStatusHook(NLPSIRFHOOK_LINK_STATUS hook)
{
    nlpsirf_link_status_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_RegisterSetNidHook(NLPSIRFHOOK_SET_NID hook)
{
    nlpsirf_set_nid_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_RegisterAddRemoveNodeHook(NLPSIRFHOOK_ADD_REMOVE_NODE hook)
{
    nlpsirf_add_remove_node_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_RegisterMsgFeedbackHook(NLPSIRFHOOK_MSG_FEEDBACK hook)
{
    nlpsirf_msg_feedback_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_RegisterSaveDataHook(NLPSIRFHOOK_SAVE_DATA hook)
{
    nlpsirf_save_data_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_Handler(void)
{
    static U8           search_index = STDBLPSIRF_NETWORK_MASTER_ADDR+1;
    
    U8                  i;
    NL_PSIRF_MSG_SEND   outbox_msg;
    BOOL                msg_to_handle = FALSE;
    U8*                 data_ptr;
    
    MODULE_CHECK();
    
    if((nlpsirf_state == NL_PSIRF_NO_ADDRESS) || (nlpsirf_state == NL_PSIRF_IDLE))
    {
        if((nlpsirf_data_ptr->network_address == 0) && (nlpsirf_in_linking == FALSE))
        {
            NlPsiRfSetLinking(TRUE);            // on slave, no address, linking ON
        }
        else if(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR)
        {
            if(nlpsirf_in_linking_do_broadcast == TRUE)
            {
                nlpsirf_in_linking_do_broadcast = FALSE;
                outbox_msg.msg_id = NlPsiRfGetNextMsgToSendId();
                outbox_msg.msg_type.v0 = MSG_TYPE_V0_LINK;
                outbox_msg.rtnid = 0;
                outbox_msg.data[0] = (U8)nlpsirf_in_linking;
                outbox_msg.data_length = 1;
                msg_to_handle = TRUE;
            }
            else if(CoreQ_Peek(nlpsirf_q_hndl, (VPTR)&outbox_msg, 1) == FALSE)
            {
                if(nlpsirf_data_ptr->routing_table[search_index][0] == STDBLPSIRF_NETWORK_MASTER_ADDR)      // unknown route to node
                {
                    LOG_DBG("NL - Search Addr %d", PU8(search_index));
                    outbox_msg.msg_id = NlPsiRfGetNextMsgToSendId();
                    outbox_msg.msg_type.v0 = MSG_TYPE_V0_LINK;
                    outbox_msg.rtnid = 0;
                    outbox_msg.data[0] = search_index;
                    outbox_msg.data_length = 1;
                    msg_to_handle = TRUE;
                    nlpsirf_data_ptr->routing_table[search_index][0] = 0;
                }
                if(++search_index >= STDNLPSIRF_MAX_NODES)
                {
                    search_index = STDBLPSIRF_NETWORK_MASTER_ADDR+1;
                }
            }
        }
        else
        {
            if(nlpsirf_conseq_fail_counter >= 5)
            {
                LOG_DBG("NL - Search Neighbours");
                outbox_msg.msg_id = NlPsiRfGetNextMsgToSendId();
                outbox_msg.msg_type.v0 = MSG_TYPE_V0_EXPLORE;
                outbox_msg.rtnid = 0;
                outbox_msg.data_length = 0;
                msg_to_handle = TRUE;
                nlpsirf_conseq_fail_counter = 0;
            }
        }
        
        if((msg_to_handle == TRUE) || (CoreQ_Read(nlpsirf_q_hndl, (VPTR)&outbox_msg, 1)))
        {
            nlpsirf_outgoing_msg_id_len = 1;
            nlpsirf_outgoing_msg_id[0] = outbox_msg.msg_id;
            
            if(NlPsiRfAllowedToSendMsg(&outbox_msg) == FALSE)
            {
                //report failure
                NlPsiRfReportMsgSuccess(FALSE);
                return;
            }
                   
            //exit current state
            NlPsiRfOnExit();
            //side effect
            nlpsirf_outgoing_msg.version = MSG_VERSION_V0;
            nlpsirf_outgoing_msg.msg_type.v0 = outbox_msg.msg_type.v0;
            nlpsirf_outgoing_msg.hid = nlpsirf_data_ptr->network_hid;
            nlpsirf_outgoing_msg.snid = nlpsirf_data_ptr->network_address;
            nlpsirf_outgoing_msg.rsnid = nlpsirf_data_ptr->network_address;
            nlpsirf_outgoing_msg.rssi = 0xFF;
            
            switch(outbox_msg.msg_type.v0)
            {
            case MSG_TYPE_V0_EXPLORE:
                nlpsirf_outgoing_msg.length = STDNLPSIRF_HEADER_LENGTH_BASE + outbox_msg.data_length;
                nlpsirf_outgoing_msg.rsnid = STDBLPSIRF_NETWORK_MASTER_ADDR;
                nlpsirf_outgoing_msg.fid = NlPsiRfGetNextBcFid();
                nlpsirf_outgoing_msg.tnid = 0;
                nlpsirf_outgoing_msg.rtnid = 0;
                MEMCPY(nlpsirf_outgoing_msg.data ,outbox_msg.data, outbox_msg.data_length);
                if(nlpsirf_state == NL_PSIRF_NO_ADDRESS)
                {
                    nlpsirf_outgoing_msg.hid = nlpsirf_data_ptr->device_hid;
                    nlpsirf_outgoing_msg.snid = STDBLPSIRF_NETWORK_MASTER_ADDR;
                    NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
                    //enter state
                    nlpsirf_inclusion_retry = EXPLORE_RETRY;
                    NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_EXPLORE_TIMEOUT, 0);
                }
                else
                {
                    nlpsirf_outgoing_msg.rssi = nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][1];;
                    NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
                    //enter state
                    NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, ((U32)STDNLPSIRF_BC_TIMEOUT_US));
                }
                break;
            case MSG_TYPE_V0_APPLIC:
            case MSG_TYPE_V0_LINK:
                nlpsirf_outgoing_msg.length = STDNLPSIRF_HEADER_LENGTH_BASE + outbox_msg.data_length;
                if(outbox_msg.rtnid == 0)
                {
                    nlpsirf_outgoing_msg.tnid = 0;
                    nlpsirf_outgoing_msg.rtnid = 0;
                    nlpsirf_outgoing_msg.fid = NlPsiRfGetNextBcFid();
                }
                else
                {
                    nlpsirf_outgoing_msg.tnid = NlPsiRfDetermineRouteTo(outbox_msg.rtnid);
                    nlpsirf_outgoing_msg.rtnid = outbox_msg.rtnid;
                    nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
                }
                
                if(outbox_msg.msg_type.v0 == MSG_TYPE_V0_LINK)
                {
                    MEMCPY(nlpsirf_outgoing_msg.data, outbox_msg.data, outbox_msg.data_length);
                }
                else if(outbox_msg.data_length > 0x0F)
                {
                    nlpsirf_outgoing_msg.data[0] = 0;
                    data_ptr = &nlpsirf_outgoing_msg.data[1];
                    nlpsirf_outgoing_msg.length++;
                    MEMCPY(data_ptr, outbox_msg.data, outbox_msg.data_length);
                }
                else
                {
                    nlpsirf_outgoing_msg.data[0] = (outbox_msg.data_length & 0x0F) << 4;
                    nlpsirf_outgoing_msg.data[1] = 0;
                    data_ptr = &nlpsirf_outgoing_msg.data[2];
                    
                    nlpsirf_outgoing_msg.length += 2;
                    MEMCPY(data_ptr, outbox_msg.data, outbox_msg.data_length);
                    data_ptr += outbox_msg.data_length;
                    
                    while((nlpsirf_outgoing_msg_id_len < 4) && CoreQ_Peek(nlpsirf_q_hndl, (VPTR)&outbox_msg, 1))
                    {
                        if((nlpsirf_outgoing_msg.msg_type.v0 != outbox_msg.msg_type.v0) ||
                           (nlpsirf_outgoing_msg.rtnid != outbox_msg.rtnid) ||
                           (outbox_msg.data_length > 0x0F) ||
                           (&nlpsirf_outgoing_msg.data[STDNLPSIRF_DATA_LENGTH] < &data_ptr[outbox_msg.data_length]))
                        {
                            break;
                        }
                        
                        if(nlpsirf_outgoing_msg_id_len == 1)
                        {
                            nlpsirf_outgoing_msg.data[0] |= (outbox_msg.data_length & 0x0F);
                        }
                        else if(nlpsirf_outgoing_msg_id_len == 2)
                        {
                            nlpsirf_outgoing_msg.data[1] |= (outbox_msg.data_length & 0x0F) << 4;
                        }
                        else if(nlpsirf_outgoing_msg_id_len == 3)
                        {
                            nlpsirf_outgoing_msg.data[1] |= (outbox_msg.data_length & 0x0F);
                        }
                        
                        nlpsirf_outgoing_msg.length += outbox_msg.data_length;
                        MEMCPY(data_ptr, outbox_msg.data, outbox_msg.data_length);
                        data_ptr += outbox_msg.data_length;
                        
                        nlpsirf_outgoing_msg_id[nlpsirf_outgoing_msg_id_len] = outbox_msg.msg_id;
                        nlpsirf_outgoing_msg_id_len++;
                        
                        CoreQ_Drop(nlpsirf_q_hndl, 1);
                    }
                }
                
                NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
                
                if(nlpsirf_outgoing_msg.rtnid == 0)
                {
                    //clear RSSI values of all slaves
                    for(i=STDBLPSIRF_NETWORK_MASTER_ADDR+1;i<STDNLPSIRF_MAX_NODES;i++)
                    {
                        nlpsirf_data_ptr->routing_table[i][1] = 0;
                    }
                    //enter state
                    NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, ((U32)STDNLPSIRF_BC_TIMEOUT_US * 2));
                }
                else
                {
                    nlpsirf_retry_counter = 0;
                    //enter state
                    NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK, 0);
                }
                break;
            case MSG_TYPE_V0_REM_NID:
                nlpsirf_outgoing_msg.length = STDNLPSIRF_HEADER_LENGTH_BASE;
                nlpsirf_outgoing_msg.tnid = NlPsiRfDetermineRouteTo(STDBLPSIRF_NETWORK_MASTER_ADDR);
                nlpsirf_outgoing_msg.rtnid = STDBLPSIRF_NETWORK_MASTER_ADDR;
                nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
                NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
                //enter state
                nlpsirf_inclusion_retry = INCLUSION_RETRY;
                NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_EXCLUSION, 0);
                break;
            case MSG_TYPE_V0_SET_NID:
                nlpsirf_outgoing_msg.length = STDNLPSIRF_HEADER_LENGTH_BASE;
                nlpsirf_outgoing_msg.rtnid = outbox_msg.rtnid;
                if(outbox_msg.rtnid == 0)
                {
                    nlpsirf_outgoing_msg.tnid = 0;
                    nlpsirf_outgoing_msg.fid = NlPsiRfGetNextBcFid();
                    NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
                    NlPsiRfClearRoutingTable();
                    nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][0] = 1;
                    nlpsirf_data_ptr->routing_table[STDBLPSIRF_NETWORK_MASTER_ADDR][1] = 0xFF;
                    //enter state
                    NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_KEEP_SILENT_TIMEOUT, ((U32)STDNLPSIRF_BC_TIMEOUT_US * 2));
                }
                else
                {
                    nlpsirf_outgoing_msg.tnid = NlPsiRfDetermineRouteTo(outbox_msg.rtnid);
                    nlpsirf_outgoing_msg.fid = NlPsiRfGetNextP2PFid();
                    NlPsiRfSendMsg(&nlpsirf_outgoing_msg);
                    NlPsiRfRemoveAddress(outbox_msg.rtnid);
                    //enter state
                    nlpsirf_inclusion_retry = INCLUSION_RETRY;
                    NlPsiRfOnEntry(NL_PSIRF_WAIT_FOR_ACK_SET_NID, 0);
                }
                break;
            default:
                //should never happen, discard
                LOG_DBG("[NLERROR] illegal msg type sending");
                //report failure
                NlPsiRfReportMsgSuccess(FALSE);     // illegal msg type
                break;
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
U8 CommNlPsiRf_SetLink(U8 state)
{
    NL_PSIRF_MSG_SEND   msg_to_send;
    
    if((nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR) && (state <= 1))
    {
        NlPsiRfSetLinking((BOOL)state);
        nlpsirf_in_linking_do_broadcast = TRUE;
    }
    else
    {
        msg_to_send.msg_id = NlPsiRfGetNextMsgToSendId();
        msg_to_send.msg_type.v0 = MSG_TYPE_V0_LINK;
        msg_to_send.data_length = 1;
        msg_to_send.data[0] = (U8)state;
        if(nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR)
        {
            msg_to_send.rtnid = 0;
        }
        else
        {
            msg_to_send.rtnid = STDBLPSIRF_NETWORK_MASTER_ADDR;
        }
            
        if((nlpsirf_data_ptr->network_hid > 0) && CoreQ_Write(nlpsirf_q_hndl, (VPTR)&msg_to_send, 1))
        {
            return msg_to_send.msg_id;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U8 CommNlPsiRf_SendApplic(U8 tnid, U8 len, U8* data_ptr)
{
    NL_PSIRF_MSG_SEND   msg_to_send;
    
    if((nlpsirf_data_ptr->network_hid > 0) && (len < STDNLPSIRF_DATA_LENGTH))
    {
        msg_to_send.msg_id = NlPsiRfGetNextMsgToSendId();
        msg_to_send.msg_type.v0 = MSG_TYPE_V0_APPLIC;
        msg_to_send.rtnid = tnid;
        msg_to_send.data_length = len;
        MEMCPY(msg_to_send.data, data_ptr, len);
        
        if(CoreQ_Write(nlpsirf_q_hndl, (VPTR)&msg_to_send, 1))
        {
            return msg_to_send.msg_id;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U8 CommNlPsiRf_SendExplore(U8* data_ptr, U8 data_len)
{
    NL_PSIRF_MSG_SEND   msg_to_send;
    
    if(data_len < (STDNLPSIRF_DATA_LENGTH - STDNLPSIRF_HEADER_EXTRA_INCL))
    {
        msg_to_send.msg_id = NlPsiRfGetNextMsgToSendId();
        msg_to_send.msg_type.v0 = MSG_TYPE_V0_EXPLORE;
        msg_to_send.data_length = data_len;
        MEMCPY(msg_to_send.data, data_ptr, data_len);
        
        if(CoreQ_Write(nlpsirf_q_hndl, (VPTR)&msg_to_send, 1))
        {
            return msg_to_send.msg_id;
        }
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U8 CommNlPsiRf_SendRemNid(void)
{
    NL_PSIRF_MSG_SEND   msg_to_send;
    
    msg_to_send.msg_id = NlPsiRfGetNextMsgToSendId();
    msg_to_send.msg_type.v0 = MSG_TYPE_V0_REM_NID;
    
    if((nlpsirf_data_ptr->network_hid > 0) && CoreQ_Write(nlpsirf_q_hndl, (VPTR)&msg_to_send, 1))
    {
        return msg_to_send.msg_id;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U8 CommNlPsiRf_SendSetNidToZero(U8 nid)
{
    NL_PSIRF_MSG_SEND   msg_to_send;
    
    msg_to_send.msg_id = NlPsiRfGetNextMsgToSendId();
    msg_to_send.msg_type.v0 = MSG_TYPE_V0_SET_NID;
    msg_to_send.rtnid = nid;
    
    if((nlpsirf_data_ptr->network_address == STDBLPSIRF_NETWORK_MASTER_ADDR) && CoreQ_Write(nlpsirf_q_hndl, (VPTR)&msg_to_send, 1))
    {
        return msg_to_send.msg_id;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U8 CommNlPsiRf_GetNid(void)
{
    return nlpsirf_data_ptr->network_address;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_ForceRemoveFromNetwork(void)
{
    if(nlpsirf_data_ptr->network_address != STDBLPSIRF_NETWORK_MASTER_ADDR)
    {
        nlpsirf_data_ptr->network_address = 0;
        nlpsirf_data_ptr->network_hid = 0;
        NlPsiRfOnEntry(NL_PSIRF_NO_ADDRESS, 0);
    }
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_SetFastP2PMessaging(BOOL set_fast)
{
    nlpsirf_fast_p2p = set_fast;
}
//------------------------------------------------------------------------------------------------//
void CommNlPsiRf_PrintStatus(void)
{
#if (TERM_LEVEL > TERM_LEVEL_NONE)
    const STRING true_false[] = {"FALSE", "TRUE"};
    const STRING network_pre[] = {"NL Network     : ", "                 "};
    const STRING star[] = {"", " (*)"};
    U8          i;
    BOOL        space = FALSE;
    U8          hopnode;
    
    LOG_TRM("[NL PSIRF] INFO");
    
    LOG_TRM("NL State       : %s (%d retries)", PCSTR(nlpsirf_states[nlpsirf_state]), PU8(nlpsirf_inclusion_retry));
    CoreLog_Flush();
    LOG_TRM("NL In Link     : %s", PCSTR(true_false[nlpsirf_in_linking]));
    LOG_TRM("NL Address     : %d", PU8(nlpsirf_data_ptr->network_address));
    LOG_TRM("NL Netw HID    : %08h [%08h]", PU32(nlpsirf_data_ptr->network_hid), PU32(nlpsirf_data_ptr->device_hid));
    CoreLog_Flush();
    LOG_TRM("NL Msg Fails   : %d", PU16(nlpsirf_msg_fail_counter));
    LOG_TRM("NL Remove NID  : %d", PU8(nlpsirf_removed_nid));
    CoreLog_Flush();
    
    if(nlpsirf_data_ptr->network_address == 0)
    {
        LOG_TRM("NL Network     : None");
    }
    else
    {
        hopnode = NlPsiRfDetermineRouteTo(STDBLPSIRF_NETWORK_MASTER_ADDR);
        
        for(i=0;i<STDNLPSIRF_MAX_NODES;i++)
        {
            if((nlpsirf_data_ptr->routing_table[i][0] != 0) || (nlpsirf_data_ptr->routing_table[i][1] != 0))
            {
                LOG_TRM("%s%d - %02x %02x %s" ,
                        PCSTR(network_pre[space]),
                        PU8(i),
                        PU8(nlpsirf_data_ptr->routing_table[i][0]),
                        PU8(nlpsirf_data_ptr->routing_table[i][1]),
                        PCSTR(star[(BOOL)(i == hopnode)]));
                space = TRUE;
                CoreLog_Flush();
            }
        }
    }
#endif
}
//================================================================================================//

