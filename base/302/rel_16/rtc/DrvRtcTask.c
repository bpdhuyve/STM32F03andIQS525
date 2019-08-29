//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of RTC with use of  CoreTask 
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define RTC__DRVRTCTASK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef RTC__DRVRTCTASK_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               RTC__DRVRTCTASK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the time refreesh rate to be used when an external RTC is used
#ifndef TIME_REFRESH_RATE_IN_S
	#define TIME_REFRESH_RATE_IN_S              60
#elif TIME_REFRESH_RATE_IN_S < 1
    #error "TIME_REFRESH_RATE_IN_S must be at lease 1"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "DrvRtcTask.h"
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
static BOOL RtcTask_SetTime(RTC_ID rtc_task_id, RTC_TIME* rtc_time_ptr);
static BOOL RtcTask_GetTime(RTC_ID rtc_task_id, RTC_TIME* rtc_time_ptr);
static void RtcTask_Tick(VPTR data_ptr);
static void RtcTask_Update(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const RTC_HOOK_LIST                  rtc_task_hook_list = {RtcTask_SetTime, RtcTask_GetTime};
static RTC_STRUCT                           rtc_task_struct;
static EPOCH_TIME                           rtc_task_epoch_time;

static RTC_HNDL                             rtc_task_source_rtc_hndl;
static BOOL                                 rtc_task_update_source_rtc;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL RtcTask_SetTime(RTC_ID rtc_task_id, RTC_TIME* rtc_time_ptr)
{
    EPOCH_TIME  new_time;
    
    if(DrvRtc_ConvertRtcTimetoEpochTime(*rtc_time_ptr, &new_time))
    {
        rtc_task_epoch_time = new_time;
        rtc_task_update_source_rtc = TRUE;
        return TRUE;
    }
    
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL RtcTask_GetTime(RTC_ID rtc_task_id, RTC_TIME* rtc_time_ptr)
{
    return DrvRtc_ConvertEpochTimetoRtcTime(rtc_task_epoch_time, rtc_time_ptr);
}
//------------------------------------------------------------------------------------------------//
static void RtcTask_Tick(VPTR data_ptr)
{
    rtc_task_epoch_time++;
}
//------------------------------------------------------------------------------------------------//
static void RtcTask_Update(VPTR data_ptr)
{
    static U16  count = TIME_REFRESH_RATE_IN_S;
    EPOCH_TIME  new_time;
    
    if(rtc_task_update_source_rtc)
    {
        if(DrvRtc_SetUtcEpochTime(rtc_task_source_rtc_hndl, rtc_task_epoch_time))
        {
            rtc_task_update_source_rtc = FALSE;
        }
    }
    else if(++count >= TIME_REFRESH_RATE_IN_S)
    {
        count--;
        if(DrvRtc_GetUtcEpochTime(rtc_task_source_rtc_hndl, &new_time))
        {
            if(new_time != rtc_task_epoch_time)
            {
                LOG_DBG("RTC update %d --> %d", PU32(rtc_task_epoch_time), PU32(new_time));
                rtc_task_epoch_time = new_time;
            }
            count = 0;
        }
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRtcTask_Init(void)
{
    MODULE_INIT_ONCE();
    
    rtc_task_struct.hook_list_ptr   = (RTC_HOOK_LIST*)&rtc_task_hook_list;
    rtc_task_struct.rtc_id          = 0;
    rtc_task_epoch_time             = 0;
    
    rtc_task_source_rtc_hndl        = NULL;
    rtc_task_update_source_rtc      = FALSE;
    
    CoreTask_Start(CoreTask_RegisterTask(1e6, RtcTask_Tick, NULL, 100, "RTC task tick"));
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
RTC_HNDL DrvRtcTask_Register(RTC_HNDL source_rtc_hndl)
{
    MODULE_CHECK();
    
    if(source_rtc_hndl != NULL)
    {
        if(rtc_task_source_rtc_hndl == NULL)
        {
            CoreTask_Start(CoreTask_RegisterTask(1e6, RtcTask_Update, NULL, 128, "RTC task update"));
        }
        rtc_task_source_rtc_hndl = source_rtc_hndl;
    }
    
    return (RTC_HNDL)&rtc_task_struct;
}
//================================================================================================//
