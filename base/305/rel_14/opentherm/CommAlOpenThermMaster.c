//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Master Application Link Layer of the OpenTherm communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define OPENTHERM__COMMALOPENTHERMMASTER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef OPENTHERM__COMMALOPENTHERMMASTER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               OPENTHERM__COMMALOPENTHERMMASTER_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD

// COM
#include "CommAlOpenThermMaster.h"
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
static BOOL AlOpenThermMaster_IsAllowedToRead(OT_DATA_ID data_id);
static BOOL AlOpenThermMaster_IsAllowedToWrite(OT_DATA_ID data_id);

static void AlOpenThermMaster_RecvFrame(OPENTHERM_MSG opentherm_msg);

static void AlOpenThermMaster_MasterTask(VPTR data_ptr);
static BOOL AlOpenThermMaster_NeedToSendReadMessage(void);
static BOOL AlOpenThermMaster_NeedToSendWriteMessage(void);
static void AlOpenThermMaster_SendInvalidMessage(void);
static void AlOpenThermMaster_SendMsg(void);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_OTMasterEnable(void);
static void Command_OTMasterRead(void);
static void Command_OTMasterWrite(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static OPENTHERM_CHANNEL                            alopenthermmaster_channel;
static BOOL                                         alopenthermmaster_enabled;

static U8                                           alopenthermmaster_tick_count;
static U8                                           alopenthermmaster_reply_tick;

static OPENTHERM_MSG                                alopenthermmaster_send_msg;
static OPENTHERMMASTER_DATA_READ_RESPONSE_HOOK      alopenthermmaster_data_read_response_hook;
static OPENTHERMMASTER_DATA_WRITE_RESPONSE_HOOK     alopenthermmaster_data_write_response_hook;

static U8                                           alopenthermmaster_readmask[(OPENTHERM_OBJECT_COUNT + 7) >> 3];
static U8                                           alopenthermmaster_writemask[(OPENTHERM_OBJECT_COUNT + 7) >> 3];

const U8    alopenthermmaster_readallowed[16] = {0x69, // 0b01101001, // 0-7
                                                 0xBE, // 0b10111110, // 8-15
                                                 0x7E, // 0b01111110, // 16-23
                                                 0xFE, // 0b11111110, // 24-31
                                                 0x5F, // 0b01011111, // 32-39
                                                 0x00, // 0b00000000, // 40-47
                                                 0x03, // 0b00000011, // 48-55
                                                 0x03, // 0b00000011, // 56-63
                                                 0x40, // 0b01000000, // 64-71
                                                 0xFF, // 0b11111111, // 72-79
                                                 0xFF, // 0b11111111, // 80-87
                                                 0x0F, // 0b00001111, // 88-95
                                                 0xF8, // 0b11111000, // 96-103
                                                 0xFF, // 0b11111111, // 104-111
                                                 0xFF, // 0b11111111, // 112-119
                                                 0xAF};// 0b10101111};// 120-127

const U8    alopenthermmaster_writeallowed[16]= {0x96, // 0b10010110, // 0-7
                                                 0x49, // 0b01001001, // 8-15
                                                 0xF1, // 0b11110001, // 16-23
                                                 0x09, // 0b00001001, // 24-31
                                                 0x60, // 0b01100000, // 32-39
                                                 0x00, // 0b00000000, // 40-47
                                                 0x00, // 0b00000000, // 48-55
                                                 0x03, // 0b00000011, // 56-63
                                                 0x80, // 0b10000000, // 64-71
                                                 0xC0, // 0b11000000, // 72-79
                                                 0x80, // 0b10000000, // 80-87
                                                 0x02, // 0b00000010, // 88-95
                                                 0x0C, // 0b00001100, // 96-103
                                                 0x64, // 0b01100100, // 104-111
                                                 0xF7, // 0b11110111, // 112-119
                                                 0x5F};// 0b01011111};// 120-127
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
OT_DATA_VALUE                           commalopenthermmaster_value[128];
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @remark  Function to check if message ID can be read
static BOOL AlOpenThermMaster_IsAllowedToRead(OT_DATA_ID data_id)
{
    if(data_id < OPENTHERM_OBJECT_COUNT)
    {
        if(data_id < 128)
        {
            return (BOOL)((alopenthermmaster_readallowed[data_id >> 3] & (1 << (data_id & 0x07))) > 0);
        }
        return TRUE;    // application specific, always allow
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to check if message ID can be written
static BOOL AlOpenThermMaster_IsAllowedToWrite(OT_DATA_ID data_id)
{
    if(data_id < OPENTHERM_OBJECT_COUNT)
    {
        if(data_id < 128)
        {
            return (BOOL)((alopenthermmaster_writeallowed[data_id >> 3] & (1 << (data_id & 0x07))) > 0);
        }
        return TRUE;    // application specific, always allow
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void AlOpenThermMaster_RecvFrame(OPENTHERM_MSG recv_msg)
{
    DATA_READ_RESPONSE      data_read_response;
    DATA_WRITE_RESPONSE     data_write_response;
    
    alopenthermmaster_reply_tick = alopenthermmaster_tick_count;
    
    // if response to read request
    if(alopenthermmaster_send_msg.msg_type == MSG_TYPE_M2S_READ_DATA)
    {
        switch(recv_msg.msg_type)
        {
        case MSG_TYPE_S2M_READ_ACK:
            if(recv_msg.data_id < OPENTHERM_OBJECT_COUNT)
            {
                commalopenthermmaster_value[recv_msg.data_id].u16_value = recv_msg.data_value;
            }
            data_read_response = READ_OK;
            break;
        case MSG_TYPE_S2M_UNKNOWN_DATAID:
            data_read_response = READ_UNKNOWN_DATAID;
            break;
        default:
            data_read_response = READ_DATA_NOT_AVAILABLE;
            break;
        }
        
        // inform application
        if(alopenthermmaster_data_read_response_hook != NULL)
        {
            alopenthermmaster_data_read_response_hook((OT_DATA_ID)recv_msg.data_id, data_read_response);
        }
    }
    // if response to write request
    else if(alopenthermmaster_send_msg.msg_type == MSG_TYPE_M2S_WRITE_DATA)
    {
        switch(recv_msg.msg_type)
        {
        case MSG_TYPE_S2M_WRITE_ACK:
            data_write_response = WRITE_OK;
            break;
        case MSG_TYPE_S2M_UNKNOWN_DATAID:
            data_write_response = WRITE_UNKNOWN_DATAID;
            break;
        default:
            data_write_response = WRITE_DATA_INVALID;
            break;
        }
        
        // inform application
        if(alopenthermmaster_data_write_response_hook != NULL)
        {
            alopenthermmaster_data_write_response_hook((OT_DATA_ID)recv_msg.data_id, data_write_response);
        }
    }
    // otherwise discard response
}
//------------------------------------------------------------------------------------------------//
static void AlOpenThermMaster_MasterTask(VPTR data_ptr)
{
    // if disabled, doe nothing
    if(alopenthermmaster_enabled == FALSE)
    {
        alopenthermmaster_tick_count = 0;
        alopenthermmaster_reply_tick = 0;
    }
    // else check for .5 second timeout
    else if(++alopenthermmaster_tick_count >= 10)
    {
        // check if read or write request needs to be send, otherwise send dummy
        if(AlOpenThermMaster_NeedToSendReadMessage() == TRUE)
        {
            return;
        }
        else if(AlOpenThermMaster_NeedToSendWriteMessage() == TRUE)
        {
            return;
        }
        else
        {
            AlOpenThermMaster_SendInvalidMessage();
        }
    }
    // else if not waiting for reply or reply has been received more than 100ms ago
    else if((alopenthermmaster_reply_tick != 0xFF) && ((alopenthermmaster_tick_count - alopenthermmaster_reply_tick) > 2))
    {
        // check if read or write request needs to be send
        if(AlOpenThermMaster_NeedToSendReadMessage() == TRUE)
        {
            return;
        }
        else if(AlOpenThermMaster_NeedToSendWriteMessage() == TRUE)
        {
            return;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL AlOpenThermMaster_NeedToSendReadMessage(void)
{
    U8  id;
    
    for(id = 0; id < OPENTHERM_OBJECT_COUNT; id++)
    {
        if((alopenthermmaster_readmask[id >> 3] & (1 << (id & 0x07))) > 0)
        {
            alopenthermmaster_readmask[id >> 3] &= ~(1 << (id & 0x07));
            
            alopenthermmaster_send_msg.msg_type   = MSG_TYPE_M2S_READ_DATA;
            alopenthermmaster_send_msg.data_id    = id;
            alopenthermmaster_send_msg.data_value = commalopenthermmaster_value[id].u16_value;
            
            AlOpenThermMaster_SendMsg();
            return TRUE;
        }
    }
    
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL AlOpenThermMaster_NeedToSendWriteMessage(void)
{
    U8  id;
    
    for(id = 0; id < OPENTHERM_OBJECT_COUNT; id++)
    {
        if((alopenthermmaster_writemask[id >> 3] & (1 << (id & 0x07))) > 0)
        {
            alopenthermmaster_writemask[id >> 3] &= ~(1 << (id & 0x07));
            
            alopenthermmaster_send_msg.msg_type   = MSG_TYPE_M2S_WRITE_DATA;
            alopenthermmaster_send_msg.data_id    = id;
            alopenthermmaster_send_msg.data_value = commalopenthermmaster_value[id].u16_value;
            
            AlOpenThermMaster_SendMsg();
            return TRUE;
        }
    }
    
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void AlOpenThermMaster_SendInvalidMessage(void)
{
    static U16      invalid_count = 0;
    
    alopenthermmaster_send_msg.msg_type   = MSG_TYPE_M2S_INVALID_DATA;
    alopenthermmaster_send_msg.data_id    = 0;
    alopenthermmaster_send_msg.data_value = invalid_count++;
    
    AlOpenThermMaster_SendMsg();
}
//------------------------------------------------------------------------------------------------//
static void AlOpenThermMaster_SendMsg(void)
{
    alopenthermmaster_tick_count = 0;
    alopenthermmaster_reply_tick = 0xFF;
    
    CommDllOpenTherm_SendFrame(alopenthermmaster_channel, alopenthermmaster_send_msg);
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_OTMasterEnable(void)
{
    CommAlOpenThermMaster_Enable(CoreTerm_GetArgumentAsBool(0));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_OTMasterRead(void)
{
    CoreTerm_PrintFeedback(CommAlOpenThermMaster_TriggerDataRead((OT_DATA_ID)CoreTerm_GetArgumentAsU32(0)));
}
//------------------------------------------------------------------------------------------------//
static void Command_OTMasterWrite(void)
{
    CoreTerm_PrintFeedback(CommAlOpenThermMaster_TriggerDataWrite((OT_DATA_ID)CoreTerm_GetArgumentAsU32(0)));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommAlOpenThermMaster_Init(OPENTHERM_CHANNEL opentherm_channel)
{
    MODULE_INIT_ONCE();
    
    alopenthermmaster_channel = opentherm_channel;
    CommDllOpenTherm_RegisterFrameHook(opentherm_channel, AlOpenThermMaster_RecvFrame);
    
    alopenthermmaster_enabled = FALSE;
    
    alopenthermmaster_tick_count = 0;
    alopenthermmaster_reply_tick = 0;
    
    alopenthermmaster_data_read_response_hook = NULL;
    alopenthermmaster_data_write_response_hook = NULL;
    
    MEMSET((VPTR)&alopenthermmaster_send_msg, 0, SIZEOF(alopenthermmaster_send_msg));
    MEMSET((VPTR)alopenthermmaster_readmask, 0, SIZEOF(alopenthermmaster_readmask));
    MEMSET((VPTR)alopenthermmaster_writemask, 0, SIZEOF(alopenthermmaster_writemask));
    
    CoreTask_Start(CoreTask_RegisterTask(50e3, AlOpenThermMaster_MasterTask, NULL, 128, "OT MSTR"));
    
    CoreTerm_RegisterCommand("OTMasterEnable", "OT master enable", 1, Command_OTMasterEnable, TRUE);
    CoreTerm_RegisterCommand("OTMasterRead", "OT master read ID ", 1, Command_OTMasterRead, TRUE);
    CoreTerm_RegisterCommand("OTMasterWrite", "OT master write ID ", 1, Command_OTMasterWrite, TRUE);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to enable/disable the OpenTerm Master
void CommAlOpenThermMaster_Enable(BOOL set_enable)
{
    alopenthermmaster_enabled = set_enable;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to trigger the reading of a data ID
BOOL CommAlOpenThermMaster_TriggerDataRead(OT_DATA_ID data_id)
{
    if(AlOpenThermMaster_IsAllowedToRead(data_id))
    {
        alopenthermmaster_readmask[data_id >> 3] |= (1 << (data_id & 0x07));
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to trigger the writing of a data ID
BOOL CommAlOpenThermMaster_TriggerDataWrite(OT_DATA_ID data_id)
{
    if(AlOpenThermMaster_IsAllowedToWrite(data_id))
    {
        alopenthermmaster_writemask[data_id >> 3] |= (1 << (data_id & 0x07));
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to register the function to be called to feedback the data read response
void CommAlOpenThermMaster_RegisterDataReadResponseHook(OPENTHERMMASTER_DATA_READ_RESPONSE_HOOK data_read_response_hook)
{
    alopenthermmaster_data_read_response_hook = data_read_response_hook;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to register the function to be called when data needs to be written
void CommAlOpenThermMaster_RegisterDataWriteResponseHook(OPENTHERMMASTER_DATA_WRITE_RESPONSE_HOOK data_write_response_hook)
{
    alopenthermmaster_data_write_response_hook = data_write_response_hook;
}
//================================================================================================//
