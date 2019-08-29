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
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max length of one message handled in the MODBUS network layer
#ifndef STDNL_FRAME_LENGTH
    #error "STDNL_FRAME_LENGTH not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//COMM lib include section
#include "modbus\CommDllModbusSlave.h"
#include "modbus\CommNlModbusSlave.h"
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
static void CommNlModbusSlave_HandleMssg(U8* data_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static U8                               commnlmodbus_addr;
static U8                               commnlmodbus_reply_data[STDNL_FRAME_LENGTH];
static U8                               commnlmodbus_reply_length;

static NLHOOK_REQUEST_RECEIVED          commnlmodbus_request_hook;
static NLHOOK_UNC_EVENT_RECEIVED        commnlmodbus_unc_event_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CommNlModbusSlave_HandleMssg(U8* data_ptr, U8 length)
{
    if(length > STDNL_FRAME_LENGTH)
    {
        LOG_WRN("Data length is too long.");
        return;
    }
    else if((data_ptr[0] != commnlmodbus_addr) && (data_ptr[0] != 0x00))
    {
        LOG_WRN("Wrong slave device address.");
        return;
    }
    else
    {
        if((commnlmodbus_request_hook != NULL) && (data_ptr[0] != 0x00))
        {
            LOG_DEV("NL to AL:  %02h", PU8A(data_ptr+1, length-1));
            commnlmodbus_request_hook(data_ptr + 1,
                                      length - 1,
                                      &commnlmodbus_reply_data[1],
                                      &commnlmodbus_reply_length);
            commnlmodbus_reply_data[0] = *data_ptr;

            LOG_DEV("AL to NL:  %02h", PU8A(&commnlmodbus_reply_data[1], commnlmodbus_reply_length));
            LOG_DEV("NL to DLL: %02h", PU8A(&commnlmodbus_reply_data[0], commnlmodbus_reply_length + 1));

            CommDllModbusSlave_SendFrame(&commnlmodbus_reply_data[0], commnlmodbus_reply_length + 1);
        }
        else if((commnlmodbus_unc_event_hook != NULL) && (data_ptr[0] == 0x00))
        {
            LOG_DEV("NL to AL:  %02h", PU8A(data_ptr+1, length-1));
            commnlmodbus_unc_event_hook(data_ptr + 1, length - 1);
        }
        else
        {
            LOG_WRN("NL rx hook is NULL!");
        }
    }
    return;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_Init(U8 address)
{
    MODULE_INIT_ONCE();
    
    commnlmodbus_addr = address;

    CommDllModbusSlave_RegisterFrameHook(CommNlModbusSlave_HandleMssg);

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_RegisterUncEventHook(NLHOOK_UNC_EVENT_RECEIVED frame_unc_event_hook)
{
    commnlmodbus_unc_event_hook = frame_unc_event_hook;
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_RegisterRequestHook(NLHOOK_REQUEST_RECEIVED frame_request_hook)
{
    commnlmodbus_request_hook = frame_request_hook;
}
//------------------------------------------------------------------------------------------------//
U8 CommNlModbusSlave_GetAddress(void)
{
    return commnlmodbus_addr;
}
//------------------------------------------------------------------------------------------------//
void CommNlModbusSlave_SetAddress(U8 address)
{
    commnlmodbus_addr = address;
}
//================================================================================================//
