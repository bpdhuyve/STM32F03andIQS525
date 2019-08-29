//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the Data Link Layer of the MODBUS protocol implemented as SLAVE
//
//
// <------------------------ MODBUS TCP/IP ADU(1) ------------------------->
//              <----------- MODBUS PDU (1') ---------------->
//  +-----------+---------------+------------------------------------------+
//  | TID | PID | Length | UID  |Code | Data                               |
//  +-----------+---------------+------------------------------------------+
//  |     |     |        |      |                                           
// (2)   (3)   (4)      (5)    (6)                                          
//
// (2)  ... MB_TCP_TID          = 0 (Transaction Identifier - 2 Byte) 
// (3)  ... MB_TCP_PID          = 2 (Protocol Identifier - 2 Byte)
// (4)  ... MB_TCP_LEN          = 4 (Number of bytes - 2 Byte)
// (5)  ... MB_TCP_UID          = 6 (Unit Identifier - 1 Byte)
// (6)  ... MB_TCP_FUNC         = 7 (Modbus Function Code)
//
// (1)  ... Modbus TCP/IP Application Data Unit
// (1') ... Modbus Protocol Data Unit

// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MODBUS__COMMNLTCPIPMODBUSSLAVE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MODBUS__COMMDLLMODBUSSLAVE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               MODBUS__COMMDLLMODBUSSLAVE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
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

// COMM
#include "CommNlTcpIpModbusSlave.h"
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
    MESSAGE_SIZE_COMPLEET                               = 0,
    MESSAGE_SIZE_BIGGER_THAN_MODBUS_FRAME               = 1,
    MESSAGE_SIZE_MORE_RECIEVED_THAN_LENGTH_IN_HEADER    = 2,
    MESSAGE_NOT_COMPLEET                                = 3
}
MESSAGE_SIZE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static MESSAGE_SIZE CommNlTcpIpModbusSlave_IsMsgCompleet(U8* data_pntr, U16 length);
static void CommNlTcpIpModbusSlave_DoAction(U8* frame_ptr,U16  length, U8** reply_ptr, U16* reply_length_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static NLTCPIPMODBUSSLAVE_RECEIVED              commnlmodbus_request_hook;
static U8                                       commnlmodbus_frame_bytes[MB_TCP_BUF_SIZE + 10];
static U8                                       commnlmodbus_reply_data[MB_TCP_BUF_SIZE];
static U8                                       commnlmodbus_reply_length;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static MESSAGE_SIZE CommNlTcpIpModbusSlave_IsMsgCompleet(U8* frame_ptr, U16 length)
{
    U16 length_mb_msg;
    MESSAGE_SIZE msg_size_enum;
    
    if(length > MB_TCP_HEADER_LENGHT)
    {
        //modbus header received
        length_mb_msg = CoreConvert_U8ArrayToU16(&frame_ptr[MB_TCP_LEN]) + MB_TCP_HEADER_LENGHT - 1; //length of the TCP data   
        
        if(length_mb_msg > MB_TCP_BUF_SIZE)
        {
            //this can't be correct...
            msg_size_enum = MESSAGE_SIZE_BIGGER_THAN_MODBUS_FRAME;
        }
        
        if(length == length_mb_msg)
        {
           //message compleet...
           //ok, let's go further
            msg_size_enum = MESSAGE_SIZE_COMPLEET;
        }
        else if(length > length_mb_msg)
        {
           //message must be corrupt. 
           //modbus master sends a message but mus wait for response
            msg_size_enum = MESSAGE_SIZE_MORE_RECIEVED_THAN_LENGTH_IN_HEADER;
        }
        else
        {
            msg_size_enum = MESSAGE_NOT_COMPLEET;
        }
    }
    else
    {
        msg_size_enum = MESSAGE_NOT_COMPLEET;
    }
    return msg_size_enum;
}
//------------------------------------------------------------------------------------------------//
static void CommNlTcpIpModbusSlave_DoAction(U8* frame_ptr, U16  length, U8** reply_ptr, U16* reply_length_ptr)
{
    U16 pid, length_mb_msg;
    U8* modbus_data_pntr;
    
    switch(CommNlTcpIpModbusSlave_IsMsgCompleet(frame_ptr, length))
    {
        case MESSAGE_SIZE_COMPLEET:
            //OK 
            pid = CoreConvert_U8ArrayToU16(&commnlmodbus_frame_bytes[MB_TCP_PID]);
            
            if(pid == MB_TCP_PROTOCOL_ID) //must be zero
            {
                //valid
                
                modbus_data_pntr = &frame_ptr[MB_TCP_HEADER_LENGHT]; //start of function code
                length_mb_msg = CoreConvert_U8ArrayToU16(&frame_ptr[MB_TCP_LEN]);
            
                if(commnlmodbus_request_hook != NULL ) 
                {
                    if(modbus_data_pntr[0] != 0x00)//function code is not 0
                    {
                        LOG_DEV("DLL to NL: %02h", PU8A(frame_ptr, length));
                        LOG_DEV("NL to AL:  %02h", PU8A(modbus_data_pntr, length_mb_msg-1));
                        commnlmodbus_request_hook(modbus_data_pntr ,
                                                length_mb_msg,
                                                &commnlmodbus_reply_data[MB_TCP_HEADER_LENGHT],
                                                &commnlmodbus_reply_length);
                        
                        memcpy( &commnlmodbus_reply_data[0], frame_ptr, MB_TCP_HEADER_LENGHT); //copy MBAP header + function code
                        
                        LOG_DEV("AL to NL:  %02h", PU8A(&commnlmodbus_reply_data[MB_TCP_HEADER_LENGHT], commnlmodbus_reply_length));
                                            
                        commnlmodbus_reply_length += MB_TCP_HEADER_LENGHT;
                        commnlmodbus_reply_data[MB_TCP_LEN]       = (commnlmodbus_reply_length >> 8) & 0x00FF;
                        commnlmodbus_reply_data[MB_TCP_LEN + 1]   = commnlmodbus_reply_length & 0x00FF;
            
                        LOG_DEV("NL to DLL: %02h", PU8A(&commnlmodbus_reply_data[0], commnlmodbus_reply_length));
                        
                        *reply_ptr = &commnlmodbus_reply_data[0];
                        *reply_length_ptr = commnlmodbus_reply_length;
                    }
                    else
                    {
                        LOG_WRN("Message received with function code = 0!!");
                    }
                }
                else
                {
                    LOG_WRN("NL rx hook is NULL!");
                }
            }
            break;
        case MESSAGE_SIZE_BIGGER_THAN_MODBUS_FRAME:
        case MESSAGE_SIZE_MORE_RECIEVED_THAN_LENGTH_IN_HEADER:
            reply_length_ptr = 0;
            *reply_ptr = &commnlmodbus_reply_data[0];
            break;
        case MESSAGE_NOT_COMPLEET:
            break;
    }       
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommNlTcpIpModbusSlave_Init(void)
{
    MODULE_INIT_ONCE();
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommNlTcpIpModbusSlave_Handler(U8* frame_ptr,U16  length, U8** reply_ptr, U16* reply_length_ptr)
{
    U16 temp;
    
    MODULE_CHECK();
    
    CommNlTcpIpModbusSlave_DoAction(frame_ptr, length, reply_ptr, reply_length_ptr);
}
//------------------------------------------------------------------------------------------------//
void CommNlTcpIpModbusSlave_RegisterRequestHook(NLTCPIPMODBUSSLAVE_RECEIVED frame_request_hook)
{
    commnlmodbus_request_hook = frame_request_hook;
}
//================================================================================================//
