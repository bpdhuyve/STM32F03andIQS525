//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module containing utils on the PsiRF network
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define PSIRF__COMMNLPSIRFUTIL_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef PSIRF__COMMNLPSIRFUTIL_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_ALL
#else
    #define CORELOG_LEVEL               PSIRF__COMMNLPSIRFUTIL_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD

// COM
#include "CommNlPsiRfUtil.h"

//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
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
void CommNlPsiRfUtil_Init(void)
{
    MODULE_INIT_ONCE();
    //place init code which must only be executed once here
    
    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here
    
}
//------------------------------------------------------------------------------------------------//
BOOL CommNlPsiRfUtil_ConvertMsgStructToArray(NL_PSIRF_MSG* msg_ptr, U8* data_ptr)
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
BOOL CommNlPsiRfUtil_ConvertMsgArrayToStruct(U8* data_ptr, NL_PSIRF_MSG* msg_ptr)
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
void CommNlPsiRfUtil_DebugPrintMessage(NL_PSIRF_MSG_DIR dir, NL_PSIRF_MSG* msg_ptr, U8 rssi)
{
    const STRING dir_names[] = {"RCV <-", "SND ->", "RPL +>"};
    
    if(msg_ptr->version == MSG_VERSION_V0)
    {
        LOG_DBG("%s [0x%02x] %s - %08x : %02d->%02d (%02d->%02d) - %2d 0x%02x - (%d) %02x",
                PCSTR(dir_names[dir]),
                PU8(rssi),
                PCSTR(CommNlPsiRfUtil_GetTypeName(msg_ptr)),
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
STRING CommNlPsiRfUtil_GetTypeName(NL_PSIRF_MSG* msg_ptr)
{
    const STRING type_names[] = {"LINK", "EXPL", "REVL", "REQN", "SETN", "REMN", "ACKN", "APPL"};
    
    if((msg_ptr->version != MSG_VERSION_V0) || (msg_ptr->msg_type.type >= 8))
    {
        return "UNKN";
    }
    return type_names[msg_ptr->msg_type.type];
}
//------------------------------------------------------------------------------------------------//
NL_PSIRF_MSG_CATEGORY CommNlPsiRfUtil_CategorizeMsg(NL_PSIRF_MSG* msg_ptr, NL_PSIRF_MSG_CATEGORY_INFO* info_ptr)
{
    if(msg_ptr->msg_type.v0 == MSG_TYPE_V0_EXPLORE)
    {
        if((info_ptr->network_hid > 0) &&
           (((info_ptr->linking_active == TRUE) && (msg_ptr->snid == 1)) ||     // explore msg from new device that wants to be included
            ((info_ptr->network_hid == msg_ptr->hid) && (msg_ptr->snid > 1))))  // explore msg from device in network that wants to find back the network master
        {
            return MSG_CATEGORY_EXPLORE;
        }
    }
    else if(msg_ptr->hid == info_ptr->network_hid)
    {
        if(info_ptr->network_address == STDNLPSIRF_NETWORK_MASTER_ADDR)
        {
            if(msg_ptr->tnid == STDNLPSIRF_NETWORK_MASTER_ADDR)
            {
                return MSG_CATEGORY_P2P_SELF;
            }
        }
        else if((msg_ptr->tnid == 0) && (msg_ptr->rtnid == 0))
        {
            if((msg_ptr->msg_type.v0 == MSG_TYPE_V0_LINK) || (msg_ptr->msg_type.v0 == MSG_TYPE_V0_APPLIC))
            {
                if(msg_ptr->fid != info_ptr->broadcast_msg_fid)
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
                else if(CoreConvert_U8ArrayToU32(msg_ptr->data) == info_ptr->device_hid)
                {
                    return MSG_CATEGORY_P2P_SELF;
                }
            }
        }
        else if(msg_ptr->tnid == info_ptr->network_address)
        {
            if(msg_ptr->rtnid == info_ptr->network_address)
            {
                return MSG_CATEGORY_P2P_SELF;
            }
            return MSG_CATEGORY_P2P_FWD;
        }
        return MSG_CATEGORY_NETW;
    }
    else if(info_ptr->network_hid == 0)
    {
        if(((msg_ptr->msg_type.v0 == MSG_TYPE_V0_REVEAL) || (msg_ptr->msg_type.v0 == MSG_TYPE_V0_SET_NID)) &&
           (msg_ptr->tnid == 0) && (msg_ptr->rtnid == 0) &&
           (CoreConvert_U8ArrayToU32(msg_ptr->data) == info_ptr->device_hid))
        {
            return MSG_CATEGORY_P2P_SELF;
        }
    }
    return MSG_CATEGORY_NOTHING;
}
//================================================================================================//
