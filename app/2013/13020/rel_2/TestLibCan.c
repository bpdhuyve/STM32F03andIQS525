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

// DRV
#include "Can\DrvCanChannel.h"
#include "Can\DrvCanChannelSysInt.h"
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
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

CAN_CHANNEL test_channel;
CAN_CONFIG_STRUCT can_config;
CAN_CHANNEL_HNDL test_can_channel;

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
static void Command_CanInitPort(void)
{  
    //register can channel
    if (CoreTerm_GetArgumentAsU32(0) == 1)
    {
        test_channel = CAN_CHANNEL_1;
    }
    else
    {
        CoreTerm_PrintFailed();
        return;
    }

    test_can_channel = DrvCanChannelSysInt_Register(test_channel);
    
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

    if((DrvCanChannel_Config(test_can_channel, &can_config)) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    else
    {
    CoreTerm_PrintAcknowledge();
    }
}

static void Command_CanConfigMailbox(void)
{
    if((DrvCanChannel_ConfigMailboxes(test_can_channel, PCMS_TX_RX_BALANCED, 0x08000000)) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    else
    {
        CoreTerm_PrintAcknowledge();
    }
    return;
}

static void Command_CanSendMessage(void)
{
    CAN_MSSG_STRUCT* msg_ptr;
    U8* data_ptr;
    *(data_ptr[0]) = 40;
    DrvCanChannel_FillExtDataFrameMssgStruct(msg_ptr, 0x05000000, 1, data_ptr);

    
        
    if((DrvCanChannel_RegisterTxHook(test_can_channel, CanTXHook)) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    else
    {
        CoreTerm_PrintAcknowledge();
    }
    
    
    if((BOOL DrvCanChannelSysInt_NotityTxMessageReady(CAN_CHANNEL_1)) == FALSE)
    {
        CoreTerm_PrintFailed();
    }
    else
    {
        CoreTerm_PrintAcknowledge();
    }  
    
    return;
}

static BOOL CanTXHook(CAN_MSSG_STRUCT* mssg_ptr)
{
    CAN_MSSG_STRUCT* msg_ptr;
    U8* data_ptr;
    *(data_ptr[0]) = 40;
    DrvCanChannel_FillExtDataFrameMssgStruct(msg_ptr, 0x05000000, 1, data_ptr);
}
//--------------------------------------------------------------------------------------------------------------------//
/*static void Command_CanSet(void)
{
    Can_PORT port;
    CommandAid_GetPort(0,&port);
    U32 pinmask = (1<<(CoreTerm_GetArgumentAsU32(1)));

    SysCan_SetPinMask(port, pinmask, (BOOL)CoreTerm_GetArgumentAsU32(2));
    CoreTerm_PrintAcknowledge();
}
//--------------------------------------------------------------------------------------------------------------------//
static void Command_CanGet(void)
{
    Can_PORT port;
    CommandAid_GetPort(0,&port);
    U32 pinmask = (1<<(CoreTerm_GetArgumentAsU32(1)));

    if ((SysCan_GetPinMask(port,pinmask)) == pinmask)
    {
        CoreTerm_PrintBool(TRUE);
    }
    else
    {
        CoreTerm_PrintBool(FALSE);
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_CanInitMask(void)
{
    SYS_PIN_FUNC    tempFunction;
    U8              argnb = 1;
    Can_PORT       Can_port;

    if(CoreTerm_GetArgumentAsU32(0) == 0)
    {
        tempFunction = PIN_INPUT;
    }
    else
    {
        tempFunction = PIN_OUTPUT;
    }

    LOG_TRM("Can INIT %d", PU8(CoreTerm_GetArgumentAsU32(0)));
    while(CommandAid_GetPort(argnb, &Can_port))
    {
        argnb++;
        LOG_TRM("PORT %c : 0x%04h", PCHAR('A' + Can_port), PU32(CoreTerm_GetArgumentAsU32(argnb)));
        SysPin_InitPinMask(Can_port, CoreTerm_GetArgumentAsU32(argnb), tempFunction);
        argnb++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_CanSetMask(void)
{
    BOOL            set_high = (BOOL)(CoreTerm_GetArgumentAsU32(0) > 0);
    U8              argnb = 1;
    Can_PORT       Can_port;

    LOG_TRM("Can SET %d", PU8(set_high));
    while(CommandAid_GetPort(argnb, &Can_port))
    {
        argnb++;
        LOG_TRM("PORT %c : 0x%04h", PCHAR('A' + Can_port), PU32(CoreTerm_GetArgumentAsU32(argnb)));
        SysCan_SetPinMask(Can_port, CoreTerm_GetArgumentAsU32(argnb), set_high);
        argnb++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_CanGetMask(void)
{
    U8              argnb = 0;
    Can_PORT       Can_port;

    LOG_TRM("Can GET");
    while(CommandAid_GetPort(argnb, &Can_port))
    {
        LOG_TRM("PORT %c : 0x%04h", PCHAR('A' + Can_port), PU32(SysCan_GetPinMask(Can_port, 0xFFFF)));
        argnb++;
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static BOOL CommandAid_GetPort(U8 argumentNumber, Can_PORT* port_ptr)
{
    CHAR tempstring[20];

    CoreTerm_GetArgumentAsString(argumentNumber, tempstring);

    if(tempstring[0] == 0)
    {
        return FALSE;
    }

    if (tempstring[0] >= 'a')   //port argument is supplied as letter
    {
        *port_ptr = (Can_PORT)(tempstring[0] - 'a');
    }
    else    //argument is supplied as number, certain processors indiacte by numbers (nxp)
    {
        *port_ptr = (Can_PORT)CoreTerm_GetArgumentAsU32(0);
    }
    return TRUE;
}
*/
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibCan_Init(void)
{
    MODULE_INIT_ONCE();

    CoreTerm_RegisterCommand("CanInitPort", "init a can port on a specific speed (a = portnr, b= speed)",2, Command_CanInitPort, TRUE);
    CoreTerm_RegisterCommand("CanConfigMailbox", "config a mailbox( a = Channel Handle, b = PiCan Arbitration field, c = Node ID field",0, Command_CanConfigMailbox, TRUE);
    CoreTerm_RegisterCommand("CanSendMessage", "Send one message with data (U32) (a = Channel Handle, b= data)",2, Command_CanSendMessage, TRUE);
    
    /*CoreTerm_RegisterCommand("CanSetMasks", "sets mask c on port b to a (a=0:low,1:high), supply var args as 'a1 b1 c1 b2 c2 ...'", CMD_VAR_ARGUMENTS, Command_CanSetMask, TRUE);
    CoreTerm_RegisterCommand("CanGetMasks", "get the complete port mask (a=port), supply var arg as 'a1 a2 ...'", CMD_VAR_ARGUMENTS, Command_CanGetMask, TRUE);
    */
    MODULE_INIT_DONE();
}
//================================================================================================//

