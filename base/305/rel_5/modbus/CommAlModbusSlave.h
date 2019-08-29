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



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Prototype of the function to read coils -> function code 0x01
typedef BOOL (*ALHOOK_READ_COILS)(U16 addr, U16 quantity, U8* coil_stat_ptr);

// @brief   Prototype of the function to read discrete inputs -> function code 0x02
typedef BOOL (*ALHOOK_READ_DISCR_INP)(U16 addr, U16 quantity, U8* inp_stat_ptr);

// @brief   Prototype of the function to read holding registers -> function code 0x03
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
void CommAlModbusSlave_InstallHookReadCoils(ALHOOK_READ_COILS hook);

// @brief   Initialiser for the Function Code 0x02 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read discrete inputs
void CommAlModbusSlave_InstallHookDiscrInp(ALHOOK_READ_DISCR_INP hook);

// @brief   Initialiser for the Function Code 0x03 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read holding registers
void CommAlModbusSlave_InstallHookReadHoldReg(ALHOOK_READ_HOLD_REG hook);

// @brief   Initialiser for the Function Code 0x04 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read input registers
void CommAlModbusSlave_InstallHookReadInpReg(ALHOOK_READ_INP_REG hook);

// @brief   Initialiser for the Function Code 0x05 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write single coil
void CommAlModbusSlave_InstallHookWrSingleCoils(ALHOOK_WR_SINGLE_COIL hook);

// @brief   Initialiser for the Function Code 0x06 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write single holding registers
void CommAlModbusSlave_InstallHookWrSingleHoldReg(ALHOOK_WR_SINGLE_HOLD_REG hook);

// @brief   Initialiser for the Function Code 0x07 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read exception status
void CommAlModbusSlave_InstallHookReadExcStatus(ALHOOK_READ_EXC_STATUS hook);

// @brief   Initialiser for the Function Code 0x0F in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write multiple coils
void CommAlModbusSlave_InstallHookWrMultCoils(ALHOOK_WR_MULT_COIL hook);

// @brief   Initialiser for the Function Code 0x10 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write multiple holding registers
void CommAlModbusSlave_InstallHookWrMultHoldReg(ALHOOK_WR_MULT_HOLD_REG hook);

// @brief   Initialiser for the Function Code 0x14 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read file record type 0x06
void CommAlModbusSlave_InstallHookReadFileRec(ALHOOK_READ_FILE_REC hook);

// @brief   Initialiser for the Function Code 0x15 in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to write file record type 0x06
void CommAlModbusSlave_InstallHookWrFileRec(ALHOOK_WR_FILE_REC hook);

// @brief   Initialiser for the Function Code 0x2B in the Application layer of MODBUS protocol entity
// @param   hook :        Pointer to the function which handles the function to read device identification
void CommAlModbusSlave_InstallHookDeviceId(ALHOOK_DEVICE_ID hook);

// @brief   Initialiser for the "User Defined" Function in the Application layer of MODBUS protocol entity
// Initialises extra "User Defined" function codes in the Application layer of MODBUS protocol.\n
// Here, the hooks you want to use when you received this function code, will be defined.
// @param   func_code :   The function code
// @param   hook :        Pointer to the function which handles "User Defined" function
void CommAlModbusSlave_RegisterFunctionCode(U8 func_code, ALHOOK_SPECIFIC hook);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* MODBUS__COMMALMODBUSSLAVE_H */
