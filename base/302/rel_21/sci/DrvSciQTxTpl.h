//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Template header file for generating a simple SCI TX Q implementation.
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define Q_STORAGE_TYPE                      U8
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static Q_HNDL                           Q_PREFIX(_tx_q_hndl) = INVALID_Q_HNDL;
static SCI_CHANNEL_HNDL                 Q_PREFIX(_tx_channel_hndl) = NULL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(SciQTx_Create)(SCI_CHANNEL_HNDL channel_hndl);
static void Q_PREFIX(SciQTx_SwitchSciChannel)(SCI_CHANNEL_HNDL channel_hndl);
static U8 Q_PREFIX(SciQTx_GetNextByteHook)(U8* byte_ptr, U8 length);
static BOOL Q_PREFIX(SciQTx_Write)(U8* data_ptr, U8 count);
static void Q_PREFIX(SciQTx_NotifyTxDataReady)(void);
static U16 Q_PREFIX(SciQTx_GetSpace)(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(SciQTx_Create)(SCI_CHANNEL_HNDL channel_hndl)
{
    Q_PREFIX(_tx_q_hndl) = CoreQ_Register(Q_SIZE, SIZEOF(Q_STORAGE_TYPE), CORE_CONVERT_TO_STRING(Q_PREFIX(SciQTx)));
    Q_PREFIX(SciQTx_SwitchSciChannel)(channel_hndl);
    return (BOOL)(Q_PREFIX(_tx_q_hndl) != INVALID_Q_HNDL);
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(SciQTx_SwitchSciChannel)(SCI_CHANNEL_HNDL channel_hndl)
{
    if(Q_PREFIX(_tx_channel_hndl) != NULL)
    {
        DrvSciChannel_RegisterTxHook(Q_PREFIX(_tx_channel_hndl), NULL);
    }
    Q_PREFIX(_tx_channel_hndl) = channel_hndl;
    if(Q_PREFIX(_tx_channel_hndl) != NULL)
    {
        DrvSciChannel_RegisterTxHook(Q_PREFIX(_tx_channel_hndl), Q_PREFIX(SciQTx_GetNextByteHook));
        if(CoreQ_GetSpace(Q_PREFIX(_tx_q_hndl)) < Q_SIZE)
        {
            Q_PREFIX(SciQTx_NotifyTxDataReady)();
        }
    }
}
//------------------------------------------------------------------------------------------------//
static U8 Q_PREFIX(SciQTx_GetNextByteHook)(U8* byte_ptr, U8 length)
{
    U16 count = CoreQ_GetCount(Q_PREFIX(_tx_q_hndl));
    if(count > 0)
    {
        if(count > length)
        {
            count = length;
        }
        CoreQ_Read(Q_PREFIX(_tx_q_hndl), (VPTR)byte_ptr, (U16)count);
    }
    return count;
}
//------------------------------------------------------------------------------------------------//
static BOOL Q_PREFIX(SciQTx_Write)(U8* data_ptr, U8 length)
{
    if(CoreQ_Write(Q_PREFIX(_tx_q_hndl), (VPTR)data_ptr, (U16)length))
    {
        Q_PREFIX(SciQTx_NotifyTxDataReady)();
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void Q_PREFIX(SciQTx_NotifyTxDataReady)(void)
{
    if(Q_PREFIX(_tx_channel_hndl) != NULL)
    {
        DrvSciChannel_NotifyTxDataReady(Q_PREFIX(_tx_channel_hndl));
    }
}
//------------------------------------------------------------------------------------------------//
static U16 Q_PREFIX(SciQTx_GetSpace)(void)
{
    return CoreQ_GetSpace(Q_PREFIX(_tx_q_hndl));
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
