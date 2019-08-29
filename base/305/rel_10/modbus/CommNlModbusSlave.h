//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Header file for the Network Layer(Slave).
// The main function of this layer is to detect if a message is one to interpret further on or not. This check is done
// by checking the address of the device.
// The layer serves as a communication medium between the datalink layer and the application layer.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MODBUS__COMMNLMODBUSSLAVE_H
#define MODBUS__COMMNLMODBUSSLAVE_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
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
// @brief   Prototype of the frame handling function when it's a request/reply message.
typedef void (*NLHOOK_REQUEST_RECEIVED)(U8* frame_ptr,
                                        U8  length,
                                        U8* reply_ptr,
                                        U8* reply_length_ptr);

// @brief   Prototype of the frame handling function when it's an uncontrolled event message.
typedef void (*NLHOOK_UNC_EVENT_RECEIVED)(U8* frame_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the Network layer entity
void CommNlModbusSlave_Init(U8 address);

// @remark none
void CommNlModbusSlave_RegisterUncEventHook(NLHOOK_UNC_EVENT_RECEIVED frame_unc_event_hook);

// @remark none
void CommNlModbusSlave_RegisterRequestHook(NLHOOK_REQUEST_RECEIVED frame_request_hook);

// @remark none
U8 CommNlModbusSlave_GetAddress(void);

// @remark none
void CommNlModbusSlave_SetAddress(U8 address);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* MODBUS__COMMNLMODBUSSLAVE_H */
