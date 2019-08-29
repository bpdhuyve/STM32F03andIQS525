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
//frame pointer contains also the address byte !
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

// @brief   Sets the slave of this device
void CommDllMpcm_SetDeviceAddress(U8 address);

// @brief   returns the  address of this device.
U8 CommDllMpcm_GetDeviceAddress(void);

// @remark  Function to send a frame, a bcc check will be added @ the end of this frame
// @param   frame_ptr: pointer to frame (contains the whole frame inluding the destination address)
// @param   length: length of the enitire frame (address included, bcc excluded)
// @return  returns if sending frame was accepted or not
BOOL CommDllMpcm_SendFrame(U8* frame_ptr, U8 length);

// @remark hook will be called when the frame has been send by CommDllMpcm_SendFrame function
void CommDllMpcm_RegisterTxCompleteHook(EVENT_CALLBACK tx_complete_hook);

// @remark hook will be called when the a frame has been received, the entire frame will be passed to the hook (address included, bcc excluded, the length is the length of the entire frame
void CommDllMpcm_RegisterFrameReceivedHook(DLLMPCM_RECV_FRAME_HOOK frame_hook);

// @brief   Turns on or off the timeout task to time-out the reception of an answer to a single message. default the timeout is off, to use this feature you must set the STDDLL_T_RESPONSE_TIMEOUT
// @param   state :         TRUE to turn ON, FALSE to turn OFF
void CommDllMpcm_EnableTimeout(BOOL state);

// @remark none
void CommDllMpcm_RegisterTimeoutHook(EVENT_CALLBACK timeout_hook);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//defines to be backwards compatible with old names
#define CommDllMpcm_SetSlaveAddress(address)                        CommDllMpcm_SetDeviceAddress(address)
#define CommDllMpcm_GetSlaveAddress()                               CommDllMpcm_GetDeviceAddress()
#define CommDllMpcm_RegisterFrameHook(frame_hook)                   CommDllMpcm_RegisterFrameReceivedHook(frame_hook)
//================================================================================================//



#endif // MPCM__COMMDLLMPCM_H

