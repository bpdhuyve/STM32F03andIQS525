//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Slave Application Link Layer of the OpenTherm communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define OPENTHERM__COMMALOPENTHERMSLAVE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef OPENTHERM__COMMALOPENTHERMSLAVE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               OPENTHERM__COMMALOPENTHERMSLAVE_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD

// COM
#include "opentherm/CommAlOpenThermSlave.h"
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
static void AlOpenThermSlave_RecvFrame(OPENTHERM_MSG opentherm_msg);
static void AlOpenThermSlave_ReplyTask(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static OPENTHERM_CHANNEL                        alopenthermslave_channel;
static TASK_HNDL                                alopenthermslave_replytask;
static TASK_HNDL                                alopenthermslave_timeouttask;
static OPENTHERMSLAVE_DATA_READ_REQUEST_HOOK    alopenthermslave_data_read_request_hook;
static OPENTHERMSLAVE_DATA_WRITE_REQUEST_HOOK   alopenthermslave_data_write_request_hook;
static OPENTHERM_MSG                            alopenthermslave_reply_msg;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void AlOpenThermSlave_RecvFrame(OPENTHERM_MSG opentherm_msg)
{
    OT_DATA_VALUE   ot_data_value;
    
    CoreTask_Start(alopenthermslave_timeouttask);
    
    alopenthermslave_reply_msg.msg_type     = MSG_TYPE_S2M_DATA_INVALID;
    alopenthermslave_reply_msg.data_id      = opentherm_msg.data_id;
    alopenthermslave_reply_msg.data_value   = opentherm_msg.data_value;
    
    switch(opentherm_msg.msg_type)
    {
    case MSG_TYPE_M2S_READ_DATA:
        alopenthermslave_reply_msg.data_value   = 0;
        if(alopenthermslave_data_read_request_hook != NULL)
        {
            ot_data_value.u16_value = opentherm_msg.data_value;
            
            switch(alopenthermslave_data_read_request_hook((OT_DATA_ID)opentherm_msg.data_id, &ot_data_value))
            {
            case READ_OK:
                alopenthermslave_reply_msg.msg_type = MSG_TYPE_S2M_READ_ACK;
                break;
            case READ_UNKNOWN_DATAID:
                alopenthermslave_reply_msg.msg_type = MSG_TYPE_S2M_UNKNOWN_DATAID;
                break;
            default:
                // do nothing
                break;
            }
            
            alopenthermslave_reply_msg.data_value = ot_data_value.u16_value;
        }
        break;
        
    case MSG_TYPE_M2S_WRITE_DATA:
        if(alopenthermslave_data_write_request_hook != NULL)
        {
            ot_data_value.u16_value = opentherm_msg.data_value;
            
            switch(alopenthermslave_data_write_request_hook((OT_DATA_ID)opentherm_msg.data_id, ot_data_value))
            {
            case WRITE_OK:
                alopenthermslave_reply_msg.msg_type = MSG_TYPE_S2M_READ_ACK;
                break;
            case WRITE_UNKNOWN_DATAID:
                alopenthermslave_reply_msg.msg_type = MSG_TYPE_S2M_UNKNOWN_DATAID;
                break;
            default:
                // do nothing
                break;
            }
            
            alopenthermslave_reply_msg.data_value = ot_data_value.u16_value;
        }
        break;
        
    default:
        // do nothing
        break;
    }
    
    CoreTask_Start(alopenthermslave_replytask);
}
//------------------------------------------------------------------------------------------------//
static void AlOpenThermSlave_ReplyTask(VPTR data_ptr)
{
    CoreTask_Stop(alopenthermslave_replytask);
    CommDllOpenTherm_SendFrame(alopenthermslave_channel, alopenthermslave_reply_msg);
}
//------------------------------------------------------------------------------------------------//
static void AlOpenThermSlave_TimeoutTask(VPTR data_ptr)
{
    CoreTask_Stop(alopenthermslave_timeouttask);
    CommDllOpenTherm_SetIdleLevel(alopenthermslave_channel, FALSE);
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
void CommAlOpenThermSlave_Init(OPENTHERM_CHANNEL opentherm_channel)
{
    MODULE_INIT_ONCE();
    
    alopenthermslave_channel = opentherm_channel;
    CommDllOpenTherm_RegisterFrameHook(opentherm_channel, AlOpenThermSlave_RecvFrame);
    
    alopenthermslave_replytask = CoreTask_RegisterTask(50e3, AlOpenThermSlave_ReplyTask, NULL, 128, "OT SLAV Reply TO");
    alopenthermslave_timeouttask = CoreTask_RegisterTask(5000e3, AlOpenThermSlave_TimeoutTask, NULL, 128, "OT SLAV Comm TO");
    
    alopenthermslave_data_read_request_hook = NULL;
    alopenthermslave_data_write_request_hook = NULL;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to register the function to be called when data needs to be read
void CommAlOpenThermSlave_RegisterDataReadRequestHook(OPENTHERMSLAVE_DATA_READ_REQUEST_HOOK data_read_request_hook)
{
    alopenthermslave_data_read_request_hook = data_read_request_hook;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to register the function to be called when data needs to be written
void CommAlOpenThermSlave_RegisterDataWriteRequestHook(OPENTHERMSLAVE_DATA_WRITE_REQUEST_HOOK data_write_request_hook)
{
    alopenthermslave_data_write_request_hook = data_write_request_hook;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to check if communication is active
BOOL CommAlOpenThermSlave_IsConnected(void)
{
    return CoreTask_IsTaskRunning(alopenthermslave_timeouttask);
}
//================================================================================================//
