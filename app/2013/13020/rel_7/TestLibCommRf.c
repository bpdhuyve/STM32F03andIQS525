//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This module determines the board type
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBCOMMRF_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBCOMMRF_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBCOMMRF_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DUCO LIB
#include "TestLibCommRf.h"

// COMM
#include "psirf\CommDllPsiRf.h"
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
static BOOL TestLibCommRf_RecvFrame(U8* frame_ptr, U8 rssi);
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CommRfSendFrame(void);
static void Command_CommRfSetPa(void);
static void Command_CommRfInfo(void);
static void Command_CommRfReset(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static U16          testlibcommrf_send_count;
static U16          testlibcommrf_recv_count;

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
static BOOL TestLibCommRf_RecvFrame(U8* frame_ptr, U8 rssi)
{
    LOG_TRM("[MSG RCV] %02h (rssi = %d)", PU8A(frame_ptr, frame_ptr[0]+1), PU8(rssi));
    testlibcommrf_recv_count++;
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CommRfSendFrame(void)
{
    U8 buffer[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
    U8 length = (U8)CoreTerm_GetArgumentAsU32(0);

    if(length < 32)
    {
        buffer[0] = length;
        LOG_TRM("[MSG SND] %02h", PU8A(buffer, buffer[0]+1));
        testlibcommrf_send_count++;
        CommDllPsiRf_SendFrame(buffer);
        CoreTerm_PrintAcknowledge();
    }
    else
    {
        CoreTerm_PrintFailed();
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_CommRfSetPa(void)
{
    CommDllPsiRf_UpdatePaTable(0, (U8)CoreTerm_GetArgumentAsU32(0));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_CommRfInfo(void)
{
    LOG_TRM("Send count: %5d", PU16(testlibcommrf_send_count));
    LOG_TRM("Recv count: %5d", PU16(testlibcommrf_recv_count));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_CommRfReset(void)
{
    testlibcommrf_send_count = 0;
    testlibcommrf_recv_count = 0;
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibCommRf_Init(SPI_CHANNEL_HNDL channel_hndl,
                        DRVGPIO_PIN_HNDL cs_pin_hndl,
                        DRVGPIO_PIN_HNDL gdo0_pin_hndl,
                        DRVGPIO_PIN_HNDL gdo2_pin_hndl,
                        U8 freq0_value)
{
    MODULE_INIT_ONCE();

    CommDllPsiRf_Init(channel_hndl, cs_pin_hndl, gdo0_pin_hndl, gdo2_pin_hndl);
    CommDllPsiRf_UpdateRegister(CC1101_FREQ0 ,freq0_value);
    CommDllPsiRf_RegisterFrameHook(TestLibCommRf_RecvFrame);

    CoreTerm_RegisterCommand("CommRfSetPa", "COMM RF sets the power output to a (default = 0xC5)", 1, Command_CommRfSetPa, TRUE);
    CoreTerm_RegisterCommand("CommRfSendFrame", "COMM RF sends a frame with length a (max 31)", 1, Command_CommRfSendFrame, TRUE);
    CoreTerm_RegisterCommand("CommRfInfo", "COMM RF print send/receive counters", 0, Command_CommRfInfo, TRUE);
    CoreTerm_RegisterCommand("CommRfReset", "COMM RF reset send/receive counters", 0, Command_CommRfReset, TRUE);
    CoreTerm_RegisterCommand("CommRfStatus", "COMM RF get DLL status", 0, CommDllPsiRf_PrintStatus, TRUE);
    
    testlibcommrf_send_count = 0;
    testlibcommrf_recv_count = 0;

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void TestLibCommRf_Handler(void)
{
    MODULE_CHECK();

    CommDllPsiRf_Handler();
}
//================================================================================================//

