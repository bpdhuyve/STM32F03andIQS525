//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the DLL layer in a P2P network
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef P2P__COMMDLLP2P_H
#define P2P__COMMDLLP2P_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Prototype of the rx frame handling function
typedef void (*COMMDLLP2P_RECV_FRAME_HOOK)(U8* frame_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommDllP2P_Init(SCI_CHANNEL_HNDL channel_hndl);

// @brief   Function to be called in the background loop
// This function will be called in background. If a complete, correct message was received
// the frame_received_hook will be called with the content of that frame (and length) as arguments
void CommDllP2P_Handler(void);

// @remark hook will be called when the a frame has been received, the frame will be passed to the hook (length and data included)
void CommDllP2P_RegisterFrameReceivedHook(COMMDLLP2P_RECV_FRAME_HOOK frame_hook);

// @remark  Function to send a data frame, a crc check will be added @ the end of this frame
// @param   frame_ptr: pointer to the data frame
// @param   length: length of the data frame
// @return  returns if sending frame was accepted or not
BOOL CommDllP2P_SendFrame(U8* frame_ptr, U8 frame_length);
//================================================================================================//



#endif /* P2P__COMMDLLP2P_H */
