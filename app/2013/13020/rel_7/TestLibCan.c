//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This module determines the board type
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBCAN_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBCan_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBCan_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

#include "TestLibCan.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void Command_CanInitPort(void);
static void Command_CanConfigMailbox(void);
static BOOL CanTxHook(CAN_MSSG_STRUCT* mssg_ptr);
static void CanRxHook(CAN_MSSG_STRUCT* mssg_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();


static CAN_CHANNEL_HNDL test_can_channel_hndl[CAN_CHANNEL_COUNT];
static CAN_INIT_HOOK    test_can_init_hook;
static CAN_MSSG_STRUCT  can_mssg;
static BOOL send_once_done;

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CanRxHook(CAN_MSSG_STRUCT* mssg_ptr)
{
     LOG_TRM("RX: 0x%08X [%d] %02X", PU32(mssg_ptr->identifier), PU8(mssg_ptr->dlc), PU8A(mssg_ptr->data, mssg_ptr->dlc));
}
//------------------------------------------------------------------------------------------------//
static void Command_CanInitPort(void)
{  
    CAN_CONFIG_STRUCT can_config;
    CAN_CHANNEL channel = (CAN_CHANNEL) CoreTerm_GetArgumentAsU32(0);

    //config can channel
    switch (CoreTerm_GetArgumentAsU32(1))
    {
        case 125:
            can_config.speed = CAN_SPEED_125_Kb;
            break;
        case 250:
            can_config.speed = CAN_SPEED_250_Kb;
            break;
        case 500:
            can_config.speed = CAN_SPEED_500_Kb;
            break;
        case 1000:
            can_config.speed = CAN_SPEED_1_Mb;
            break;
        default:
            CoreTerm_PrintFailed();
            return;
    }
    
    if(channel > CAN_CHANNEL_COUNT)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    test_can_channel_hndl[channel] = DrvCanChannelSysInt_Register(channel);
    
    if((test_can_init_hook == NULL) || (test_can_init_hook(channel, can_config , test_can_channel_hndl[channel]) == FALSE))
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    if((DrvCanChannel_RegisterTxHook(test_can_channel_hndl[channel], CanTxHook)) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    
    if((DrvCanChannel_RegisterRxHook(test_can_channel_hndl[channel], CanRxHook)) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    else
    {
        CoreTerm_PrintAcknowledge();
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_CanConfigMailbox(void)
{
    CAN_CHANNEL channel = (CAN_CHANNEL) CoreTerm_GetArgumentAsU32(0);
    if((DrvCanChannel_ConfigMailboxes(test_can_channel_hndl[channel], (CONFIG_SCHEME) CoreTerm_GetArgumentAsU32(1), CoreTerm_GetArgumentAsU32(2))) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    else
    {
        CoreTerm_PrintAcknowledge();
    }
    return;
}
//------------------------------------------------------------------------------------------------//
static void Command_CanSendMessage(void)
{
    CAN_CHANNEL channel = (CAN_CHANNEL) CoreTerm_GetArgumentAsU32(0);
    U8 data[8];
    
    data[0] = (U8) CoreTerm_GetArgumentAsU32(2);
    
    DrvCanChannel_FillExtDataFrameMssgStruct(&can_mssg, CoreTerm_GetArgumentAsU32(1), 1, &data[0]);
    
    send_once_done = FALSE;
    
    if((DrvCanChannel_NotityTxMessageReady(test_can_channel_hndl[channel])) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    else
    {
        CoreTerm_PrintAcknowledge();
    }  
    return;
}
//--------------------------------------------------------------------------------------------------------------------//
static BOOL CanTxHook(CAN_MSSG_STRUCT* mssg_ptr)
{
    MEMCPY(mssg_ptr, &can_mssg, SIZEOF(CAN_MSSG_STRUCT));
    if(send_once_done == FALSE)
    {
        send_once_done = TRUE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibCan_Init(CAN_INIT_HOOK init_hook)
{
    MODULE_INIT_ONCE();

    test_can_init_hook = init_hook;
    
    CoreTerm_RegisterCommand("CanInitPort", "init a can port on a specific speed (a = portnr, b= speed)",2, Command_CanInitPort, TRUE);
    CoreTerm_RegisterCommand("CanConfigMailbox", "config a mailbox( a = Channel, b = config scheme, c = Node ID field", 3, Command_CanConfigMailbox, TRUE);
    CoreTerm_RegisterCommand("CanSendMessage", "Send one message with data (U32) (a = Channel, b = Identifier, c= data)", 3, Command_CanSendMessage, TRUE);
    
    MODULE_INIT_DONE();
}
//================================================================================================//

