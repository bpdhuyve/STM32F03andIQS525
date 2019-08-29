//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Source file for the Modbus Network Layer (Slave).
// This file is the source file of MODBUS Network Layer. Here the address will be checked to be the right one.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MODBUS__COMMNLMODBUSSLAVE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MODBUS__COMMNLMODBUSSLAVE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               MODBUS__COMMNLMODBUSSLAVE_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//COMM lib include section
#include "modbus\CommDllModbus.h"
#include "modbus\CommNlModbusSlave.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
#define VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, check_registered)     \
    if(((MODBUS_CHANNEL)(modbus_channel) == INVALID_MODBUS_CHANNEL) || (modbus_channel > MODBUS_CHANNEL_COUNT)) {LOG_ERR("[MODBUS NL SLAVE] Invalid MODBUS channel %d", PU8(modbus_channel));} \
    MODBUS_NL_SLAVE_STRUCT* modbus_nl_channel_ptr = &nlmodbus_slave_struct[(MODBUS_CHANNEL)modbus_channel - 1]; \
    if (check_registered && (modbus_nl_channel_ptr->commnlmodbus_channel != modbus_channel)) {LOG_ERR("[MODBUS NL SLAVE] MODBUS channel %d not properly registered", PU8(modbus_channel));}
    
//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
typedef struct
{
    MODBUS_CHANNEL              commnlmodbus_channel;
    U8                          commnlmodbus_addr;
    U8                          commnlmodbus_reply_data[MODBUS_MAX_FRAME_LENGTH];
    U8                          commnlmodbus_reply_length;
    NLHOOK_REQUEST_RECEIVED     commnlmodbus_request_hook;
    NLHOOK_UNC_EVENT_RECEIVED   commnlmodbus_unc_event_hook;
}
MODBUS_NL_SLAVE_STRUCT;


//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void CommNlModbusSlave_HandleMsg(MODBUS_CHANNEL modbus_channel, U8* data_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static MODBUS_NL_SLAVE_STRUCT   nlmodbus_slave_struct[MODBUS_CHANNEL_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CommNlModbusSlave_HandleMsg(MODBUS_CHANNEL modbus_channel, U8* data_ptr, U8 length)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE); 
    
    if(length > MODBUS_MAX_FRAME_LENGTH)
    {
        LOG_WRN("Data length is too long.");
        return;
    }
    else if((data_ptr[0] != modbus_nl_channel_ptr->commnlmodbus_addr) && (data_ptr[0] != 0x00))
    {
        LOG_WRN("Wrong slave device address: %d", PU8(modbus_nl_channel_ptr->commnlmodbus_addr));
        return;
    }
    else
    {
        if((modbus_nl_channel_ptr->commnlmodbus_request_hook != NULL) && (data_ptr[0] != 0x00))
        {
            LOG_DEV("NL to AL:  %02h", PU8A(data_ptr+1, length-1));
                        
            modbus_nl_channel_ptr->commnlmodbus_request_hook(
                                      modbus_channel,
                                      data_ptr + 1,
                                      length - 1,
                                      &(modbus_nl_channel_ptr->commnlmodbus_reply_data[1]),
                                      &(modbus_nl_channel_ptr->commnlmodbus_reply_length));
            modbus_nl_channel_ptr->commnlmodbus_reply_data[0] = *data_ptr;
            

            LOG_DEV("AL to NL:  %02h", PU8A(&(modbus_nl_channel_ptr->commnlmodbus_reply_data[1]), modbus_nl_channel_ptr->commnlmodbus_reply_length));
            LOG_DEV("NL to DLL: %02h", PU8A(&(modbus_nl_channel_ptr->commnlmodbus_reply_data[0]), modbus_nl_channel_ptr->commnlmodbus_reply_length + 1));
            if (!CommDllModbus_SendFrame(modbus_channel, &(modbus_nl_channel_ptr->commnlmodbus_reply_data[0]), modbus_nl_channel_ptr->commnlmodbus_reply_length + 1))
            {
                LOG_WRN("[MODBUS NL SLAVE] Failed to send frame");
            }
        }
        else if((modbus_nl_channel_ptr->commnlmodbus_unc_event_hook != NULL) && (data_ptr[0] == 0x00))
        {
            LOG_DEV("NL to AL:  %02h", PU8A(data_ptr+1, length-1));
            modbus_nl_channel_ptr->commnlmodbus_unc_event_hook(data_ptr + 1, length - 1);
        }
        else
        {
            LOG_WRN("NL rx hook is NULL!");
        }
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)nlmodbus_slave_struct, 0, sizeof(nlmodbus_slave_struct));
   
    LOG_DBG("[MODBUS NL SLAVE] initialised");
    
    MODULE_INIT_DONE();
} 
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_RegisterModbusChannel(MODBUS_CHANNEL modbus_channel, U8 address)
{
    MODULE_CHECK();

    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, FALSE);
    
    modbus_nl_channel_ptr->commnlmodbus_channel = modbus_channel;
    modbus_nl_channel_ptr->commnlmodbus_addr = address;
  
    CommDllModbus_RegisterFrameHook(modbus_channel, CommNlModbusSlave_HandleMsg);
    
    LOG_DBG("[MODBUS NL SLAVE] channel %d registered on MODBUS address %d", PU8(modbus_channel), PU8(address));
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_RegisterUncEventHook(MODBUS_CHANNEL modbus_channel, NLHOOK_UNC_EVENT_RECEIVED frame_unc_event_hook)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
    
    modbus_nl_channel_ptr->commnlmodbus_unc_event_hook = frame_unc_event_hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_RegisterRequestHook(MODBUS_CHANNEL modbus_channel, NLHOOK_REQUEST_RECEIVED frame_request_hook)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
    
    modbus_nl_channel_ptr->commnlmodbus_request_hook = frame_request_hook;
}
//------------------------------------------------------------------------------------------------//
U8 CommNlModbusSlave_GetAddress(MODBUS_CHANNEL modbus_channel)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
    
    return modbus_nl_channel_ptr->commnlmodbus_addr;
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_SetAddress(MODBUS_CHANNEL modbus_channel, U8 address)
{
    VALIDATE_AND_GET_MODBUS_NL_CHANNEL_PTR(modbus_channel, TRUE);
    
    modbus_nl_channel_ptr->commnlmodbus_addr = address;
}
//================================================================================================//
