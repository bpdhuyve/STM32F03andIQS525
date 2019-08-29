//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module containing utils on the PsiRF network
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef PSIRF__COMMNLPSIRFUTIL_H
#define PSIRF__COMMNLPSIRFUTIL_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the protocol version to be used
#ifndef STDNLPSIRF_PROTOCOL_VERSION
    #error "STDNLPSIRF_PROTOCOL_VERSION not defined in AppConfig.h"
#else
    #if STDNLPSIRF_PROTOCOL_VERSION == 0
        #define STDNLPSIRF_HEADER_LENGTH            10
    #else
        #error "STDNLPSIRF_PROTOCOL_VERSION defined in AppConfig.h is unkown"
    #endif
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of nodes
#ifndef STDNLPSIRF_MAX_NODES
    #error "STDNLPSIRF_MAX_NODES not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the broadcast delay timeout in ms
#ifndef STDNLPSIRF_BC_DELAY_TIMEOUT
    #error "STDNLPSIRF_BC_DELAY_TIMEOUT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the p2p timeout in ms
#ifndef STDNLPSIRF_P2P_TIMEOUT
    #error "STDNLPSIRF_P2P_TIMEOUT not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "psirf\CommDllPsiRf.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define STDNLPSIRF_NETWORK_MASTER_ADDR      1
#define STDNLPSIRF_P2P_TIMEOUT_US(x)        (((U32)STDNLPSIRF_P2P_TIMEOUT * 1000L) +((U32)x << 7))
#define STDNLPSIRF_BC_TIMEOUT_US            ((U32)STDNLPSIRF_MAX_NODES * (U32)STDNLPSIRF_BC_DELAY_TIMEOUT * 1000L)
#define STDNLPSIRF_BC_WAIT_TIMEOUT(x)       ((U32)(x) * (U32)STDNLPSIRF_BC_DELAY_TIMEOUT * 1000L)

#define STDNLPSIRF_DATA_LENGTH              (STDDLLPSIRF_FRAME_LENGTH-STDNLPSIRF_HEADER_LENGTH)
#define STDNLPSIRF_HEADER_LENGTH_BASE       (STDNLPSIRF_HEADER_LENGTH-1)
#define STDNLPSIRF_HEADER_EXTRA_INCL        4
#define STDNLPSIRF_HEADER_LENGTH_INCL       (STDNLPSIRF_HEADER_LENGTH_BASE+STDNLPSIRF_HEADER_EXTRA_INCL)
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    MSG_VERSION_V0          = 0,
}
NL_PSIRF_MSG_VERSION;

typedef enum
{
    MSG_TYPE_V0_LINK        = 0,
    MSG_TYPE_V0_EXPLORE     = 1,
    MSG_TYPE_V0_REVEAL      = 2,
    MSG_TYPE_V0_REQ_NID     = 3,
    MSG_TYPE_V0_SET_NID     = 4,
    MSG_TYPE_V0_REM_NID     = 5,
    MSG_TYPE_V0_ACK         = 6,
    MSG_TYPE_V0_APPLIC      = 7,
}
NL_PSIRF_MSG_TYPE_V0;

typedef union
{
    U8                      type;
    NL_PSIRF_MSG_TYPE_V0    v0;
}
NL_PSIRF_MSG_TYPE;

typedef struct
{
    U8                      length;
    NL_PSIRF_MSG_VERSION    version;
    NL_PSIRF_MSG_TYPE       msg_type;
    U32                     hid;
    U8                      snid;
    U8                      tnid;
    U8                      rsnid;
    U8                      rtnid;
    U8                      fid;
    U8                      rssi;
    U8                      data[STDNLPSIRF_DATA_LENGTH];
}
NL_PSIRF_MSG;

typedef enum
{
    MSG_CATEGORY_BC_NEW,
    MSG_CATEGORY_P2P_SELF,
    MSG_CATEGORY_P2P_FWD,
    MSG_CATEGORY_NETW,
    MSG_CATEGORY_EXPLORE,
    MSG_CATEGORY_NETW_CLEAR,
    MSG_CATEGORY_NOTHING,
}
NL_PSIRF_MSG_CATEGORY;

typedef struct
{
    BOOL    linking_active;
    U8      network_address;
    U8      broadcast_msg_fid;
    U32     network_hid;
    U32     device_hid;
}
NL_PSIRF_MSG_CATEGORY_INFO;

typedef enum
{
    MSG_DIR_RECV            = 0,
    MSG_DIR_SEND            = 1,
    MSG_DIR_REPLY           = 2,
}
NL_PSIRF_MSG_DIR;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommNlPsiRfUtil_Init(void);

// @brief   Function to convert data array to PsiNL message struct
BOOL CommNlPsiRfUtil_ConvertMsgStructToArray(NL_PSIRF_MSG* msg_ptr, U8* data_ptr);
BOOL CommNlPsiRfUtil_ConvertMsgArrayToStruct(U8* data_ptr, NL_PSIRF_MSG* msg_ptr);

// @brief   Function to print the message to the terminal (debug level)
void CommNlPsiRfUtil_DebugPrintMessage(NL_PSIRF_MSG_DIR dir, NL_PSIRF_MSG* msg_ptr, U8 rssi);
// @brief   Function to get the message type name
STRING CommNlPsiRfUtil_GetTypeName(NL_PSIRF_MSG* msg_ptr);

// @brief   Function to categorize message
NL_PSIRF_MSG_CATEGORY CommNlPsiRfUtil_CategorizeMsg(NL_PSIRF_MSG* msg_ptr, NL_PSIRF_MSG_CATEGORY_INFO* info_ptr);
//================================================================================================//



#endif /* PSIRF__COMMNLPSIRFUTIL_H */
