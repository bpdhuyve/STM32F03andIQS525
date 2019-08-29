//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This module determines the board type
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBSCI_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBSCI_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBSCI_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DUCO LIB
#include "TestLibSci.h"

// SYS
#include "sci\SysSciInt.h"
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
static void TestLibSci_Rx(U8* byte_ptr, U8 length);
static U8 TestLibSci_Tx(U8* byte_ptr, U8 length);
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_SciInit(void);
static void Command_SciSend(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static U8           tx_count;

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
static void TestLibSci_Rx(U8* byte_ptr, U8 length)
{
    LOG_TRM("[SCI] RX %c", PCHARA(byte_ptr, length));
}
//------------------------------------------------------------------------------------------------//
static U8 TestLibSci_Tx(U8* byte_ptr, U8 length)
{
    U8  sending = 0;
    U8* base = byte_ptr;
    
    while(length && tx_count)
    {
        tx_count--;
        length--;
        *byte_ptr = 'A' + tx_count;
        while(*byte_ptr > 'Z')
        {
            *byte_ptr -= 26;
        }
        byte_ptr++;
        sending++;
    }
    if(sending)
    {
        LOG_TRM("[SCI] TX %c", PCHARA(base, sending));
    }
    else
    {
        LOG_TRM("[SCI] TX done");
    }
    return sending;
}
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_SciInit(void)
{
    SCI_CHANNEL sci_channel = (SCI_CHANNEL)CoreTerm_GetArgumentAsU32(0);
    SCI_CONFIG_STRUCT   sci_cfg_struct = {SCI_SPEED_9600_BPS, SCI_PARITY_NONE, SCI_STOPBIT_1, SCI_DATA_LENGTH_8_BITS};
    
    if(CoreTerm_GetArgumentAsU32(1) != 0)
    {
        sci_cfg_struct.speed = (SCI_SPEED)CoreTerm_GetArgumentAsU32(1);
    }
    
    if(SysSciInt_Channel_Init(sci_channel) &&
       SysSciInt_Channel_Config(sci_channel, &sci_cfg_struct))
    {
        SysSciInt_Channel_RegisterRxHook(sci_channel, TestLibSci_Rx);
        SysSciInt_Channel_RegisterTxHook(sci_channel, TestLibSci_Tx);
        
        CoreTerm_PrintAcknowledge();
    }
    else
    {
        CoreTerm_PrintFailed();
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_SciSend(void)
{
    SCI_CHANNEL sci_channel = (SCI_CHANNEL)CoreTerm_GetArgumentAsU32(0);
    
    tx_count = (U8)CoreTerm_GetArgumentAsU32(1);
    
    CoreTerm_PrintFeedback(SysSciInt_Channel_NotityTxDataReady(sci_channel));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibSci_Init(void)
{
    MODULE_INIT_ONCE();
    
    tx_count = 0;
    
    CoreTerm_RegisterCommand("SciInit", "SCI init channel a with speed b", 2, Command_SciInit, TRUE);
    CoreTerm_RegisterCommand("SciSend", "SCI send msg on channel a with b bytes", 2, Command_SciSend, TRUE);
    
    MODULE_INIT_DONE();
}
//================================================================================================//

