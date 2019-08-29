//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module that handles the communication of a LIN slave
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef LIN_COMMDLLLIN_H
#define LIN_COMMDLLLIN_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
// DRV
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// LIN v2.x devices use enhanced checksum
// LIN v1.x devices use classic checksum
// PID 60 and 61 are always using classic checksum
typedef enum
{
    CHECKSUM_CLASSIC            = 0,
    CHECKSUM_ENHANCED           = 1,
}
CHECKSUM_METHOD;

typedef BOOL (*COMMDLLLIN_CHECK_FOR_RESPONSE_HOOK)(U8 pid, U8* buffer_ptr, U8* data_len_ptr, CHECKSUM_METHOD* checksum_method_ptr);

typedef BOOL (*COMMDLLLIN_GET_EXPECTED_RESPONSE_HOOK)(U8 pid, U8* expected_len_ptr, CHECKSUM_METHOD* checksum_method_ptr);

typedef void (*COMMDLLLIN_REPORT_RESPONSE_HOOK)(U8 pid, U8* recv_data_ptr, U8 recv_data_len);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommDllLin_Init(SCI_CHANNEL_HNDL sci_channel_hndl);

// @remark  Function to register the function that checks if the application has a response to the received header
BOOL CommDllLin_Register_CheckForResponseHook(COMMDLLLIN_CHECK_FOR_RESPONSE_HOOK check_for_response_hook);

// @remark  Function to register the function that checks if the application is interested in the response of the received header and what its length and checksum method should be
BOOL CommDllLin_Register_GetExpectedResponseHook(COMMDLLLIN_GET_EXPECTED_RESPONSE_HOOK get_expected_response_hook);

// @remark  Function to register the function that actually reports the accepted response to the application
BOOL CommDllLin_Register_ReportResponseHook(COMMDLLLIN_REPORT_RESPONSE_HOOK report_response_hook);

// @remark  Function to send out a master request on a given PID
BOOL CommDllLin_MasterRequest(U8 pid);
//================================================================================================//



#endif /* LIN_COMMDLLLIN_H */
