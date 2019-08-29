//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Header file for the Application Layer of the MODBUS-protocol.
// The MODBUS protocol defines a simple protocol data unit (PDU) independent of the underlying communication layers.
// The mapping of MODBUS protocol on specific buses or network can introduce some additional fields on the application
// data unit (ADU).
// The MODBUS application data unit is built by the client that initiates a MODBUS transaction. The function indicates
// to the server what kind of action to perform. The MODBUS application establishes the format of a request initated by
// a client.
// The MODBUS application protocol is described in following document found under the Oxi project directory:\n
// "\Oxi\Specifications\modbus\Modbus_Application_Protocol_V1_1a.pdf"
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MODBUS__COMMALMODBUSSLAVE_H
#define MODBUS__COMMALMODBUSSLAVE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
#include "CommDllModbus.h"

//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Enum of MODBUS function codes
typedef enum
{
    READ_COILS                                 = 1,
    READ_DISCRETE_INPUTS                       = 2,
    READ_HOLDING_REGISTERS                     = 3,
    READ_INPUT_REGISTERS                       = 4,
    WRITE_SINGLE_COIL                          = 5,
    WRITE_SINGLE_REGISTER                      = 6,
    READ_EXCEPTION_STATUS                      = 7,
    DIAGNOSTICS                                = 8,
    GET_COMM_EVENT_COUNTER                     = 11,
    GET_COMM_EVENT_LOG                         = 12,
    WRITE_MULTIPLE_COILS                       = 15,
    WRITE_MULTIPLE_REGISTERS                   = 16,
    REPORT_SLAVE_ID                            = 17,
    READ_FILE_RECORD                           = 20,
    WRITE_FILE_RECORD                          = 21,
    MASK_WRITE_REGISTER                        = 22,
    RD_WR_MULTIPLE_REGISTERS                   = 23,
    READ_FIFO_QUEUE                            = 24,
    ENCAP_INTERF_TRANSPORT                     = 43,
    UPLOADER_COMMAND_SWITCH_TO_BOOT            = 0x65,
    UPLOADER_COMMAND_INIT_UPLOAD               = 0x66,
    TIME_REFERENCE_BROADCAST                   = 0x67,
    UPLOADER_COMMAND_UPLOAD_EXCEPTION_STATUS   = 0x69,
    UPLOADER_COMMAND_UPLOAD_DATA_BLOCK         = 0x67,
    UPLOADER_COMMAND_READ_VERSION              = 0x6A,
    ASSIGN_ADDRESS_DAISY_CHAIN_BROADCAST       = 0x6B,
    DAISY_CHAIN_OUTPUT_PIN_CONTROL             = 0x6C,
    
    UPLOADER_COMMAND_GET_DEVICE_IDENTIFICATION = ENCAP_INTERF_TRANSPORT,
    
}
MODBUS_FUNCTIONCODE;

// @brief   Prototype of the function to read coils -> function code 0x01
typedef BOOL (*ALHOOK_READ_COILS)(U16 addr, U16 quantity, U8* coil_stat_ptr);

// @brief   Prototype of the function to read discrete inputs -> function code 0x02
typedef BOOL (*ALHOOK_READ_DISCR_INP)(U16 addr, U16 quantity, U8* inp_stat_ptr);

// @brief   Prototype of the function to read holding registers -> function code 0x03
// @param   reg_value_ptr_ptr : must point to a static array filled with the register values
typedef BOOL (*ALHOOK_READ_HOLD_REG)(U16 addr, U16 quantity, U16** reg_value_ptr_ptr);

// @brief   Prototype of the function to read input registers -> function code 0x04
typedef BOOL (*ALHOOK_READ_INP_REG)(U16 addr, U16 quantity, U16** inp_reg_ptr_ptr);

// @brief   Prototype of the function to write single coil -> function code 0x05
typedef BOOL (*ALHOOK_WR_SINGLE_COIL)(U16 addr, BOOL value);

// @brief   Prototype of the function to write single holding registers -> function code 0x06
typedef BOOL (*ALHOOK_WR_SINGLE_HOLD_REG)(U16 addr, U16 value);

// @brief   Prototype of the function to read exception status -> function code 0x07
typedef BOOL (*ALHOOK_READ_EXC_STATUS)(U8* output_data_ptr);

// @brief   Prototype of the function to write multiple coils -> function code 0x0F
typedef BOOL (*ALHOOK_WR_MULT_COIL)(U16 addr, U16 quantity, U8* value_ptr);

// @brief   Prototype of the function to write multiple holding registers -> function code 0x10
typedef BOOL (*ALHOOK_WR_MULT_HOLD_REG)(U16 addr, U16 quantity, U16* value_ptr);

// @brief   Prototype of the function to read file record type 0x06 -> function code 0x14
typedef BOOL (*ALHOOK_READ_FILE_REC)(U16  file_number,
                                     U16  rec_number,
                                     U16  rec_length,
                                     U8*  rec_data_ptr);

// @brief   Prototype of the function to write file record type 0x06 -> function code 0x15
typedef BOOL (*ALHOOK_WR_FILE_REC)(U16  file_number,
                                   U16  rec_number,
                                   U16  rec_length,
                                   U8*  rec_data_ptr);

// @brief   Prototype of the function to read device identification -> function code 0x2B
typedef BOOL (*ALHOOK_DEVICE_ID)(STRING* str_ptr);

// @brief   Prototype of the function to implement user defined function codes -> codes between 0x41-0x48 & 0x64-0x6E
typedef BOOL (*ALHOOK_SPECIFIC)(U8  func_type,
                                U8* frame_ptr,
                                U8  length,
                                U8* reply_ptr,
                                U8* reply_length_ptr);

typedef void (*HOOK_REQUEST_RECEIVED)(  MODBUS_CHANNEL modbus_channel,
                                        U8* frame_ptr,
                                        U8  length,
                                        U8* reply_ptr,
                                        U8* reply_length_ptr);


//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the Application layer of MODBUS protocol entity
// Initialises Application layer of MODBUS protocol and registers this entity to the Module Manager.\n
void CommAlModbusSlave_Init(void);

// @brief   Initialiser for the Function Code 0x01 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the funtion to read coils
void CommAlModbusSlave_InstallHookReadCoils(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_COILS hook);

// @brief   Initialiser for the Function Code 0x02 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read discrete inputs
void CommAlModbusSlave_InstallHookDiscrInp(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_DISCR_INP hook);

// @brief   Initialiser for the Function Code 0x03 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read holding registers
void CommAlModbusSlave_InstallHookReadHoldReg(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_HOLD_REG hook);

// @brief   Initialiser for the Function Code 0x04 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read input registers
void CommAlModbusSlave_InstallHookReadInpReg(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_INP_REG hook);

// @brief   Initialiser for the Function Code 0x05 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write single coil
void CommAlModbusSlave_InstallHookWrSingleCoils(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_SINGLE_COIL hook);

// @brief   Initialiser for the Function Code 0x06 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write single holding registers
void CommAlModbusSlave_InstallHookWrSingleHoldReg(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_SINGLE_HOLD_REG hook);

// @brief   Initialiser for the Function Code 0x07 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read exception status
void CommAlModbusSlave_InstallHookReadExcStatus(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_EXC_STATUS hook);

// @brief   Initialiser for the Function Code 0x0F in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write multiple coils
void CommAlModbusSlave_InstallHookWrMultCoils(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_MULT_COIL hook);

// @brief   Initialiser for the Function Code 0x10 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write multiple holding registers
void CommAlModbusSlave_InstallHookWrMultHoldReg(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_MULT_HOLD_REG hook);

// @brief   Initialiser for the Function Code 0x14 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read file record type 0x06
void CommAlModbusSlave_InstallHookReadFileRec(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_FILE_REC hook);

// @brief   Initialiser for the Function Code 0x15 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write file record type 0x06
void CommAlModbusSlave_InstallHookWrFileRec(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_FILE_REC hook);

// @brief   Initialiser for the Function Code 0x2B in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read device identification
void CommAlModbusSlave_InstallHookDeviceId(MODBUS_CHANNEL modbus_channel, ALHOOK_DEVICE_ID hook);

// @brief   Initialiser for the "User Defined" Function in the Application layer of MODBUS protocol entity
// Initialises extra "User Defined" function codes in the Application layer of MODBUS protocol.\n
// Here, the hooks you want to use when you received this function code, will be defined.
// @param   func_code :   The function code
// @param   hook :        Pointer to the function which handles "User Defined" function
void CommAlModbusSlave_RegisterFunctionCode(MODBUS_CHANNEL modbus_channel, MODBUS_FUNCTIONCODE func_code, ALHOOK_SPECIFIC hook);

// @remark none
HOOK_REQUEST_RECEIVED CommAlModbusSlave_GetRequestHook(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* MODBUS__COMMALMODBUSSLAVE_H */
