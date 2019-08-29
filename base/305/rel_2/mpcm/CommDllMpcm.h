//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Header file for the Data Link Layer.
// The main function of this layer is to transmit frames of characters between master and salve equipment. The
// layer serves as a communication medium to the network layer.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef MPCM__COMMDLLMPCM_H
#define MPCM__COMMDLLMPCM_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//DRIVER lib include section
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Prototype of the frame handling function
typedef void (*DLLMPCM_RECV_FRAME_HOOK)(U8* frame_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the DataLink Layer entity
// Initialises the DataLink Layer and registers this entity to the Module Manager.\n
void CommDllMpcm_Init(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED sci_speed);

// @remark none
void CommDllMpcm_RegisterFrameHook(DLLMPCM_RECV_FRAME_HOOK frame_hook);

// @remark none
void CommDllMpcm_RegisterTimeoutHook(EVENT_CALLBACK timeout_hook);

// @remark none
void CommDllMpcm_RegisterTxCompleteHook(EVENT_CALLBACK tx_complete_hook);

// @brief   Function to be called in the background loop
// This function will be called in background. It will call be executed when a complete new message is received.
// The frame handling function will be called and the message will be interpreted.
void CommDllMpcm_Handler(void);

// @remark Function to send a frame, returns if sending frame was accepted or not
BOOL CommDllMpcm_SendFrame(U8* ptr_ptr, U8 length);

// @brief   Sets the slave address of a node.
// @param 	address The new slave address
void CommDllMpcm_SetSlaveAddress(U8 address);

// @brief   returns the slave address of a node.
// @return 	the node's slave address
U8 CommDllMpcm_GetSlaveAddress(void);

// @brief   Turns on or off the timeout task to time-out the reception of an answer to a single message.
// @param   state :         TRUE to turn ON, FALSE to turn OFF
void CommDllMpcm_EnableTimeout(BOOL state);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif // MPCM__COMMDLLMPCM_H

