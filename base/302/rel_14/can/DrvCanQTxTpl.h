//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Template header file for generating a simple CAN TX Q implementation.
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

//DRV include section
#include "can\DrvCanChannel.h"
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
static Q_HNDL                           Q_PREFIX(_tx_q_hndl) = INVALID_Q_HNDL;
static CAN_CHANNEL_HNDL                 Q_PREFIX(_tx_channel_hndl) = NULL;
static BOOL                             Q_PREFIX(_tx_log_mssgs);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(CanQTx_Create)(CAN_CHANNEL_HNDL channel_hndl, BOOL log_mssgs);
static void Q_PREFIX(CanQTx_SwitchCanChannel)(CAN_CHANNEL_HNDL channel_hndl);
static BOOL Q_PREFIX(CanQTx_GetNextMssgHook)(CAN_MSSG_STRUCT* mssg_ptr);
static BOOL Q_PREFIX(CanQTx_Write)(CAN_MSSG_STRUCT* mssg_ptr);
static void Q_PREFIX(CanQTx_NotifyTxDataReady)(void);
static U16 Q_PREFIX(CanQTx_GetSpace)(void);
static void Q_PREFIX(CanQTx_EnableLogging)(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(CanQTx_Create)(CAN_CHANNEL_HNDL channel_hndl, BOOL log_mssgs)
{
    Q_PREFIX(_tx_q_hndl) = CoreQ_Register(Q_SIZE, SIZEOF(Q_STORAGE_TYPE), CORE_CONVERT_TO_STRING(Q_PREFIX(CanQTx)));
    Q_PREFIX(_tx_log_mssgs) = log_mssgs;
    Q_PREFIX(CanQTx_SwitchCanChannel)(channel_hndl);
    CoreTerm_RegisterCommand(CORE_CONVERT_TO_STRING(Q_PREFIX(LogTx)), "Enable CAN TX logging", 1, Q_PREFIX(CanQTx_EnableLogging) , TRUE);
    return (BOOL)(Q_PREFIX(_tx_q_hndl) != INVALID_Q_HNDL);
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(CanQTx_SwitchCanChannel)(CAN_CHANNEL_HNDL channel_hndl)
{
    if(Q_PREFIX(_tx_channel_hndl) != NULL)
    {
        DrvCanChannel_RegisterTxHook(Q_PREFIX(_tx_channel_hndl), NULL);
    }
    if(channel_hndl != NULL)
    {
        DrvCanChannel_RegisterTxHook(channel_hndl, Q_PREFIX(CanQTx_GetNextMssgHook));
        if(CoreQ_GetSpace(Q_PREFIX(_tx_q_hndl)) < Q_SIZE) //live switching with non-mepty queue
        {
            Q_PREFIX(CanQTx_NotifyTxDataReady)();
        }
    }
    Q_PREFIX(_tx_channel_hndl) = channel_hndl;
}
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(CanQTx_GetNextMssgHook)(CAN_MSSG_STRUCT* mssg_ptr)
{
    Core_CriticalEnter();
    if(CoreQ_Read(Q_PREFIX(_tx_q_hndl), (VPTR)mssg_ptr, 1))
    {
        Core_CriticalExit();
        if(Q_PREFIX(_tx_log_mssgs) == TRUE) //log the mssg
        {
            LOG_TRM("%sTX: 0x%08X [%d] %02X", PCSTR(CORE_CONVERT_TO_STRING(Q_PREFIX())), PU32(mssg_ptr->identifier), PU8(mssg_ptr->dlc), PU8A(mssg_ptr->data, mssg_ptr->dlc));
        }
        return TRUE;
    }
    Core_CriticalExit();
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(CanQTx_Write)(CAN_MSSG_STRUCT* mssg_ptr)
{
    if(CoreQ_Write(Q_PREFIX(_tx_q_hndl), (VPTR)mssg_ptr, 1) == TRUE)
    {
        Q_PREFIX(CanQTx_NotifyTxDataReady)();
        return TRUE;
    }
    
    LOG_WRN("%sTX: 0x%08X [%d] %02X : mssg lost", PCSTR(CORE_CONVERT_TO_STRING(Q_PREFIX())), PU32(mssg_ptr->identifier), PU8(mssg_ptr->dlc), PU8A(mssg_ptr->data, mssg_ptr->dlc));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(CanQTx_NotifyTxDataReady)(void)
{
    if(Q_PREFIX(_tx_channel_hndl) != NULL)
    {
        DrvCanChannel_NotityTxMessageReady(Q_PREFIX(_tx_channel_hndl));
    }
}
//------------------------------------------------------------------------------------------------//
static U16 Q_PREFIX(CanQTx_GetSpace)(void)
{
    return CoreQ_GetSpace(Q_PREFIX(_tx_q_hndl));
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(CanQTx_EnableLogging)(void)
{
    Q_PREFIX(_tx_log_mssgs) = CoreTerm_GetArgumentAsBool(0);
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
#undef Q_STORAGE_TYPE
#undef Q_SIZE
//================================================================================================//
