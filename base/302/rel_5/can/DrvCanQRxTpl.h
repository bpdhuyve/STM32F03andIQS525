//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Template header file for generating a simple CAN RX Q implementation.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#ifndef Q_PREFIX
    #error "Q_PREFIX not defined"
#endif
//------------------------------------------------------------------------------------------------//
#if (Q_SIZE < 2)
    #error "Q_SIZE must be 2 or larger"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "can\ISysCanInt.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define Q_STORAGE_TYPE     CAN_MSSG_STRUCT
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static Q_HNDL                           Q_PREFIX(_rx_q_hndl) = INVALID_Q_HNDL;
static CAN_CHANNEL_HNDL                 Q_PREFIX(_rx_channel_hndl) = NULL;
static BOOL                             Q_PREFIX(_rx_log_mssgs);
//================================================================================================//


 
//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(CanQRx_Create)(CAN_CHANNEL_HNDL channel_hndl, BOOL log_mssgs);
static void Q_PREFIX(CanQRx_SwitchCanChannel)(CAN_CHANNEL_HNDL channel_hndl);
static void Q_PREFIX(CanQRx_NewMssgHook)(CAN_MSSG_STRUCT* mssg_ptr);
static BOOL Q_PREFIX(CanQRx_Read)(CAN_MSSG_STRUCT* mssg_ptr);
static U16 Q_PREFIX(CanQRx_GetCount)(void);
static U16 Q_PREFIX(CanQRx_DropAll)(void);
static void Q_PREFIX(CanQRx_EnableLogging)(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(CanQRx_Create)(CAN_CHANNEL_HNDL channel_hndl, BOOL log_mssgs)
{
    Q_PREFIX(_rx_q_hndl) = CoreQ_Register(Q_SIZE, SIZEOF(Q_STORAGE_TYPE), CORE_CONVERT_TO_STRING(Q_PREFIX(CanQRx)));
    Q_PREFIX(_rx_log_mssgs) = log_mssgs;
    Q_PREFIX(CanQRx_SwitchCanChannel)(channel_hndl);
    CoreTerm_RegisterCommand(CORE_CONVERT_TO_STRING(Q_PREFIX(LogRx)), "Enable CAN RX logging", 1, Q_PREFIX(CanQRx_EnableLogging) , TRUE);
    return (BOOL)(Q_PREFIX(_rx_q_hndl) != INVALID_Q_HNDL);
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(CanQRx_SwitchCanChannel)(CAN_CHANNEL_HNDL channel_hndl)
{
    if(Q_PREFIX(_rx_channel_hndl) != NULL)
    {
        DrvCanChannel_RegisterRxHook(Q_PREFIX(_rx_channel_hndl), NULL);
    }
    if(channel_hndl != NULL)
    {
        DrvCanChannel_RegisterRxHook(channel_hndl, Q_PREFIX(CanQRx_NewMssgHook));
    }
    Q_PREFIX(_rx_channel_hndl) = channel_hndl;
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(CanQRx_NewMssgHook)(CAN_MSSG_STRUCT* mssg_ptr)
{
    if(CoreQ_Write(Q_PREFIX(_rx_q_hndl), (VPTR)mssg_ptr, 1) == FALSE)
    {
        LOG_WRN("%sRX: 0x%08X [%d] %02X : mssg lost", PCSTR(CORE_CONVERT_TO_STRING(Q_PREFIX())), PU32(mssg_ptr->identifier), PU8(mssg_ptr->dlc), PU8A(mssg_ptr->data, mssg_ptr->dlc));
    }
    else if(Q_PREFIX(_rx_log_mssgs) == TRUE) //log the mssg
    {
        LOG_TRM("%sRX: 0x%08X [%d] %02X", PCSTR(CORE_CONVERT_TO_STRING(Q_PREFIX())), PU32(mssg_ptr->identifier), PU8(mssg_ptr->dlc), PU8A(mssg_ptr->data, mssg_ptr->dlc));
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(CanQRx_Read)(CAN_MSSG_STRUCT* mssg_ptr)
{
    return CoreQ_Read(Q_PREFIX(_rx_q_hndl), (VPTR)mssg_ptr, 1);
}
//------------------------------------------------------------------------------------------------//
static U16 Q_PREFIX(CanQRx_GetCount)(void)
{
    return CoreQ_GetCount(Q_PREFIX(_rx_q_hndl));
}
//------------------------------------------------------------------------------------------------//
static U16 Q_PREFIX(CanQRx_DropAll)(void)
{
    return CoreQ_DropAll(Q_PREFIX(_rx_q_hndl));
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(CanQRx_EnableLogging)(void)
{
    Q_PREFIX(_rx_log_mssgs) = CoreTerm_GetArgumentAsBool(0);
    CoreTerm_PrintAcknowledge();
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//




//================================================================================================//
// C L E A R / U N D E F    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#undef Q_PREFIX
#undef Q_SIZE
#undef Q_STORAGE_TYPE
//================================================================================================//
