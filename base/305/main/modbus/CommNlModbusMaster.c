//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Network layer for MODBUS Master
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MODBUS__COMMNLMODBUSMASTER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MODBUS__COMMNLMODBUSMASTER_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               MODBUS__COMMNLMODBUSMASTER_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD

// COM
#include "CommDllModbus.h"
#include "CommNlModbusMaster.h"

// APP

//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
#define VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, check_registered)     \
    if(((MODBUS_CHANNEL)(modbus_channel) == INVALID_MODBUS_CHANNEL) || (modbus_channel > MODBUS_CHANNEL_COUNT)) {LOG_ERR("[MODBUS NL MASTER] Invalid MODBUS channel %d", PU8(modbus_channel));} \
    MODBUS_NL_MASTER_STRUCT* modbus_nl_channel_ptr = &nlmodbus_master_struct[(MODBUS_CHANNEL)modbus_channel - 1]; \
    if (check_registered && (modbus_nl_channel_ptr->commnlmodbus_channel != modbus_channel)) {LOG_ERR("[MODBUS NL MASTER] MODBUS channel %d not properly registered", PU8(modbus_channel));}
    

//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
typedef struct
{
    MODBUS_CHANNEL      commnlmodbus_channel;
    TASK_HNDL           commnlmodbus_timeout_task;
    EVENT_CALLBACK      commnlmodbus_timeout_hook; 
    EVENT_VPTR_CALLBACK commnlmodbus_response_hook;
}
MODBUS_NL_MASTER_STRUCT;


//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void CommNlModbusMaster_HandleMsg(MODBUS_CHANNEL modbus_channel, U8* data_ptr, U8 length);
static void CommNlModbusMaster_Timeout(VPTR data_ptr);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static MODBUS_NL_MASTER_STRUCT   nlmodbus_master_struct[MODBUS_CHANNEL_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CommNlModbusMaster_HandleMsg(MODBUS_CHANNEL modbus_channel, U8* data_ptr, U8 length)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
    
    LOG_TRM("[MODBUS NL MASTER] Frame received: %02h", PU8A(data_ptr, length));
    
    CoreTask_Stop(modbus_nl_channel_ptr->commnlmodbus_timeout_task);
    
    if (modbus_nl_channel_ptr->commnlmodbus_response_hook != NULL)
    {
        modbus_nl_channel_ptr->commnlmodbus_response_hook(data_ptr);
    }
}
//------------------------------------------------------------------------------------------------//
static void CommNlModbusMaster_Timeout(VPTR data_ptr)
{
    MODBUS_NL_MASTER_STRUCT* modbus_nl_channel_ptr = (MODBUS_NL_MASTER_STRUCT*)data_ptr;
    
    CoreTask_Stop(modbus_nl_channel_ptr->commnlmodbus_timeout_task);
    
    if (modbus_nl_channel_ptr->commnlmodbus_timeout_hook != NULL)
    {
        modbus_nl_channel_ptr->commnlmodbus_timeout_hook();
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
void CommNlModbusMaster_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)nlmodbus_master_struct, 0, sizeof(nlmodbus_master_struct));
   
    LOG_DBG("[MODBUS NL MASTER] initialised");
    
    MODULE_INIT_DONE();    
}

//------------------------------------------------------------------------------------------------//
void CommNlModbusMaster_RegisterModbusChannel(MODBUS_CHANNEL modbus_channel)
{
    MODULE_CHECK();
    
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, FALSE);
      
    modbus_nl_channel_ptr->commnlmodbus_channel = modbus_channel;
    modbus_nl_channel_ptr->commnlmodbus_timeout_task = CoreTask_RegisterTask(T_TIMEOUT_MASTER, CommNlModbusMaster_Timeout, modbus_nl_channel_ptr, 255, "ModbusTimeOut");
  
    CommDllModbus_RegisterFrameHook(modbus_channel, CommNlModbusMaster_HandleMsg);
    
    LOG_DBG("[MODBUS NL MASTER] channel %d registered", PU8(modbus_channel));
  
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusMaster_RegisterTimeoutHook(MODBUS_CHANNEL modbus_channel, EVENT_CALLBACK timeout_hook)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
      
    modbus_nl_channel_ptr->commnlmodbus_timeout_hook = timeout_hook;
    LOG_DBG("[MODBUS NL MASTER] timeout hook registered for channel %d", PU8(modbus_channel));    
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusMaster_RegisterReceiveHook(MODBUS_CHANNEL modbus_channel, EVENT_VPTR_CALLBACK receive_hook)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
    
    modbus_nl_channel_ptr->commnlmodbus_response_hook = receive_hook;
    LOG_DBG("[MODBUS NL MASTER] response hook registered for channel %d", PU8(modbus_channel));
}
//------------------------------------------------------------------------------------------------//
BOOL CommNlModbusMaster_SendFrame(MODBUS_CHANNEL modbus_channel, U8* frame_ptr, U8 length)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
        
    if (CommDllModbus_SendFrame(modbus_channel, frame_ptr, length) == TRUE)
    {
        CoreTask_Stop(modbus_nl_channel_ptr->commnlmodbus_timeout_task);
        CoreTask_Start(modbus_nl_channel_ptr->commnlmodbus_timeout_task);
        
        return TRUE;
    }
    else
    {
        LOG_WRN("[MODBUS NL MASTER] Failed to send frame");
        return FALSE;
    } 
}
//================================================================================================//
