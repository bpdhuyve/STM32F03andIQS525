//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Test library for the Real Time Clock
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBRTC_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBRTC_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBRTC_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// TEST LIB
#include "TestLibRtc.h"
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
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_RtcSetTime(void);
static void Command_RtcGetTime(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static RTC_HNDL test_lib_rtc_hndl;
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static const STRING day_of_week_string[7] = {"MONDAY", 
                                             "TUESDAY", 
                                             "WEDNESDAY", 
                                             "THURSDAY", 
                                             "FRIDAY", 
                                             "SATURDAY", 
                                             "SUNDAY"};
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_RtcSetTime(void)
{
    RTC_TIME rtc_time_temp;
    
    rtc_time_temp.day_of_month    = CoreTerm_GetArgumentAsU32(0);
    rtc_time_temp.month           = (MONTH)CoreTerm_GetArgumentAsU32(1);
    rtc_time_temp.year            = CoreTerm_GetArgumentAsU32(2);
    rtc_time_temp.hour            = CoreTerm_GetArgumentAsU32(3);
    rtc_time_temp.minute          = CoreTerm_GetArgumentAsU32(4);
    rtc_time_temp.second          = CoreTerm_GetArgumentAsU32(5);
    
    if(DrvRtc_DetermineDayOfWeek(&rtc_time_temp) == FALSE)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    CoreTerm_PrintFeedback(DrvRtc_SetTime(test_lib_rtc_hndl, &rtc_time_temp));
}
//------------------------------------------------------------------------------------------------//
static void Command_RtcGetTime(void)
{
    RTC_TIME rtc_time_temp;
    
    if(DrvRtc_GetTime(test_lib_rtc_hndl, &rtc_time_temp) == TRUE)
    {
        LOG_TRM("[RTC] %s %02d/%02d/%04d %02d:%02d:%02d", PCSTR(day_of_week_string[rtc_time_temp.day_of_week]),
                                                          PU8(rtc_time_temp.day_of_month),
                                                          PU8(rtc_time_temp.month),
                                                          PU16(rtc_time_temp.year),
                                                          PU8(rtc_time_temp.hour),
                                                          PU8(rtc_time_temp.minute),
                                                          PU8(rtc_time_temp.second));
        CoreTerm_PrintAcknowledge();
    }
    else
    {
        CoreTerm_PrintFailed();
    }
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibRtc_Init(RTC_HNDL rtc_hndl)
{
    MODULE_INIT_ONCE();
    
    test_lib_rtc_hndl = rtc_hndl;
    
    CoreTerm_RegisterCommand("RtcSetTime", "Set time (a=day of month, b=month(1-12), c=year, d=hour, e=minute, f=second)", 6, Command_RtcSetTime, TRUE);
    CoreTerm_RegisterCommand("RtcGetTime", "Get time", 0, Command_RtcGetTime, TRUE);
    
    MODULE_INIT_DONE();
}
//================================================================================================//
