//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module that handles the application layer of a LIN slave
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef LIN_COMMALLINNSLAVE_H
#define LIN_COMMALLINNSLAVE_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
// COM
#include "lin\CommDllLin.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef void (*COMMALLINSLAVE_REPORT_RESPONSE_HOOK)(U8* recv_data_ptr, U8 recv_data_len);

typedef BOOL (*COMMALLINSLAVE_REQUEST_RESPONSE_HOOK)(U8* data_ptr, U8* data_len_ptr);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommAlLinSlave_Init(SCI_CHANNEL_HNDL sci_channel_hndl);

// functions to get/set the nad of the module
BOOL CommAlLinSlave_SetNad(U8 nad);
U8 CommAlLinSlave_GetNad(void);

// function to subscribe to a signal carrying frame (pid < 60)
BOOL CommAlLinSlave_SubscribeTo(U8 pid, U8 data_len, CHECKSUM_METHOD method, COMMALLINSLAVE_REPORT_RESPONSE_HOOK report_hook);

// function to indicate the node will reply to a signal carrying frame (pid < 60)
BOOL CommAlLinSlave_ReplyToRequest(U8 pid, CHECKSUM_METHOD method, COMMALLINSLAVE_REQUEST_RESPONSE_HOOK request_hook);
//================================================================================================//



#endif /* LIN_COMMALLINNSLAVE_H */

