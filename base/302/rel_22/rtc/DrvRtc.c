//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// // Implementation of the common part of the RTCl driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define RTC__DRVRTC_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef RTC__DRVRTC_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               RTC__DRVRTC_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "rtc\DrvRtc.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    S8      timezone;
    BOOL    auto_daylight_saving_time;
}
RTC_PARA_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Rtc_VerifyTime(RTC_TIME* rtc_time_ptr);

static BOOL Rtc_IsInSummerTime(EPOCH_TIME utc_time);
static EPOCH_TIME Rtc_ConvertLocalToUtcTime(EPOCH_TIME local_time);
static EPOCH_TIME Rtc_ConvertUtcToLocalTime(EPOCH_TIME utc_time);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static RTC_PARA_STRUCT              rtc_para_struct;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL Rtc_VerifyTime(RTC_TIME* rtc_time_ptr)
{
    const U8    month_len[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    BOOL        time_ok = TRUE;
    
    if(rtc_time_ptr->day_of_week >= 7)
    {
        LOG_DBG("Illegal day_of_week");
        time_ok = FALSE;
    }
    if((rtc_time_ptr->year < 1970) || (rtc_time_ptr->year > 2199))
    {
        LOG_DBG("Illegal year");
        time_ok = FALSE;
    }
    else if((rtc_time_ptr->month < 1) || (rtc_time_ptr->month > 12))
    {
        LOG_DBG("Illegal month");
        time_ok = FALSE;
    }
    else if((rtc_time_ptr->day_of_month < 1) || (rtc_time_ptr->day_of_month > month_len[rtc_time_ptr->month - 1]))
    {
        if((rtc_time_ptr->month == FEBRUARY) &&
           (rtc_time_ptr->day_of_month == 29) &&
           ((rtc_time_ptr->year & 0x03) == 0) &&
           (((rtc_time_ptr->year % 100) != 0) || ((rtc_time_ptr->year % 400) == 0)))
        {
            // leapday :-)
        }
        else
        {
            LOG_DBG("Illegal day");
            time_ok = FALSE;
        }
    }
    if(rtc_time_ptr->hour > 23)
    {
        LOG_DBG("Illegal hour");
        time_ok = FALSE;
    }
    if(rtc_time_ptr->minute > 59)
    {
        LOG_DBG("Illegal minute");
        time_ok = FALSE;
    }
    if(rtc_time_ptr->second > 59)
    {
        LOG_DBG("Illegal second");
        time_ok = FALSE;
    }
    
    return time_ok;
}
//------------------------------------------------------------------------------------------------//
static BOOL Rtc_IsInSummerTime(EPOCH_TIME utc_time)
{
    RTC_TIME    utc_rtc_time;
        
    if(DrvRtc_ConvertEpochTimetoRtcTime(utc_time, &utc_rtc_time) == FALSE)
    {
        return FALSE;
    }
    
    // daylight saving time is active between the last sunday of march till the last sunday of october
    // so it is active:
    // - from april through september
    // - in march, the days after the last sunday
    //             and on the last sunday after 1am
    // - in october, the days before the last sunday
    //             and on the last sunday before 1am
    return (BOOL)(((utc_rtc_time.month >= APRIL) && (utc_rtc_time.month <= SEPTEMBER)) ||
                  ((utc_rtc_time.month == MARCH) && (((utc_rtc_time.day_of_month + 6 - utc_rtc_time.day_of_week) > 31) ||
                                                     ((utc_rtc_time.day_of_week == SUNDAY) && ((utc_rtc_time.day_of_month + 7) > 31) && (utc_rtc_time.hour >= 1)))) ||
                  ((utc_rtc_time.month == OCTOBER) && (((utc_rtc_time.day_of_month + 6 - utc_rtc_time.day_of_week) <= 31) &&
                                                       ((utc_rtc_time.day_of_week != SUNDAY) || ((utc_rtc_time.day_of_month + 7) <= 31) || (utc_rtc_time.hour < 1)))));
}
//------------------------------------------------------------------------------------------------//
static EPOCH_TIME Rtc_ConvertLocalToUtcTime(EPOCH_TIME local_time)
{
    EPOCH_TIME  utc_time = local_time - ((EPOCH_TIME)rtc_para_struct.timezone * 3600);
    
    if((rtc_para_struct.auto_daylight_saving_time) && (Rtc_IsInSummerTime(utc_time)))
    {
        utc_time -= 3600;
    }
    
    return utc_time;
}
//------------------------------------------------------------------------------------------------//
static EPOCH_TIME Rtc_ConvertUtcToLocalTime(EPOCH_TIME utc_time)
{
    EPOCH_TIME  local_time = utc_time + ((EPOCH_TIME)rtc_para_struct.timezone * 3600);
    
    if((rtc_para_struct.auto_daylight_saving_time) && (Rtc_IsInSummerTime(utc_time)))
    {
        local_time += 3600;
    }
    
    return local_time;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRtc_Init(void)
{
    rtc_para_struct.timezone = 0;
    rtc_para_struct.auto_daylight_saving_time = FALSE;
}
//------------------------------------------------------------------------------------------------//
// SET/GET LOCAL TIME (taking into account the timezone and the daylight saving time )
BOOL DrvRtc_SetLocalTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr)
{
    EPOCH_TIME local_epoch_time;
    
    if(DrvRtc_ConvertRtcTimetoEpochTime(*rtc_time_ptr, &local_epoch_time))
    {
        return DrvRtc_SetLocalEpochTime(rtc_hndl, local_epoch_time);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_GetLocalTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr)
{
    EPOCH_TIME local_epoch_time;
    
    if(DrvRtc_GetLocalEpochTime(rtc_hndl, &local_epoch_time))
    {
        return DrvRtc_ConvertEpochTimetoRtcTime(local_epoch_time, rtc_time_ptr);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_SetLocalEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME epoch_time)
{
    return DrvRtc_SetUtcEpochTime(rtc_hndl, Rtc_ConvertLocalToUtcTime(epoch_time));
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_GetLocalEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME* epoch_time_ptr)
{
    if(DrvRtc_GetUtcEpochTime(rtc_hndl, epoch_time_ptr))
    {
        *epoch_time_ptr = Rtc_ConvertUtcToLocalTime(*epoch_time_ptr);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// GET/SET UTC (or GMT) (not taking into account the timezone and the daylight saving time )
BOOL DrvRtc_SetUtcTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr)
{
    if((rtc_hndl != NULL) && (rtc_hndl->hook_list_ptr != NULL) && (rtc_hndl->hook_list_ptr->set_time_hook != NULL) && Rtc_VerifyTime(rtc_time_ptr))
    {
        return rtc_hndl->hook_list_ptr->set_time_hook(rtc_hndl->rtc_id, rtc_time_ptr);
    }
    LOG_WRN("RTC set time function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_GetUtcTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr)
{
    if((rtc_hndl != NULL) && (rtc_hndl->hook_list_ptr != NULL) && (rtc_hndl->hook_list_ptr->get_time_hook != NULL))
    {
        return rtc_hndl->hook_list_ptr->get_time_hook(rtc_hndl->rtc_id, rtc_time_ptr);
    }
    LOG_WRN("RTC get time function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_SetUtcEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME epoch_time)
{
    RTC_TIME    rtc_time_temp;
    
    if(DrvRtc_ConvertEpochTimetoRtcTime(epoch_time, &rtc_time_temp))
    {
        return DrvRtc_SetUtcTime(rtc_hndl, &rtc_time_temp);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_GetUtcEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME* epoch_time_ptr)
{
    RTC_TIME    rtc_time_temp;
    
    if(DrvRtc_GetUtcTime(rtc_hndl, &rtc_time_temp))
    {
        return DrvRtc_ConvertRtcTimetoEpochTime(rtc_time_temp, epoch_time_ptr);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// Convert RTC to Epoch and back
BOOL DrvRtc_ConvertRtcTimetoEpochTime(RTC_TIME rtc_time, EPOCH_TIME* epoch_time_ptr)
{
    static const U16    monthdays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    
    if(rtc_time.year < 1970)
    {
        *epoch_time_ptr = 0;
        return FALSE;
    }
    
    // add days since 01/01/1970, ignoring leapdays
    *epoch_time_ptr = (365 * (rtc_time.year - 1970)) + monthdays[rtc_time.month - 1] + rtc_time.day_of_month - 1;
    
    // add leapdays
    if(rtc_time.month < 3)
    {
        rtc_time.year--;
    }
    *epoch_time_ptr += ((rtc_time.year - 1968) >> 2) - ((rtc_time.year - 1900) / 100)  + ((rtc_time.year - 1600) / 400);
    
    // convert days to seconds
    *epoch_time_ptr *= 86400;
    
    // add hours-minutes-seconds
    *epoch_time_ptr += ((U32)rtc_time.hour * 3600) + ((U32)rtc_time.minute * 60) + rtc_time.second;
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_ConvertEpochTimetoRtcTime(EPOCH_TIME epoch_time, RTC_TIME* rtc_time_ptr)
{
    EPOCH_TIME epoch_temp = epoch_time;
    
    // trim hours-minutes-seconds
    rtc_time_ptr->second = epoch_temp % 60;
    epoch_temp /= 60;
    rtc_time_ptr->minute = epoch_temp % 60;
    epoch_temp /= 60;
    rtc_time_ptr->hour = epoch_temp % 24;
    epoch_temp /= 24;
    
    // day_of_week
    rtc_time_ptr->day_of_week = (DAY_OF_WEEK)((epoch_temp + 3) % 7);
    
    // search year
    rtc_time_ptr->day_of_month = 1;
    rtc_time_ptr->month = (MONTH)1;
    rtc_time_ptr->year = 1971 + epoch_temp / 366;
    LOG_DEV("estim year  %d", PU16(rtc_time_ptr->year - 1));
    while(DrvRtc_ConvertRtcTimetoEpochTime(*rtc_time_ptr, &epoch_temp) && (epoch_temp <= epoch_time))
    {
        rtc_time_ptr->year++;
    }
    rtc_time_ptr->year--;
    
    // search month
    rtc_time_ptr->month = (MONTH)(13 - ((epoch_temp - epoch_time) / (30 * 86400)));
    LOG_DEV("estim month %d", PU16(rtc_time_ptr->month - 1));
    while((rtc_time_ptr->month < 13) && DrvRtc_ConvertRtcTimetoEpochTime(*rtc_time_ptr, &epoch_temp) && (epoch_temp <= epoch_time))
    {
        rtc_time_ptr->month++;
    }
    rtc_time_ptr->month--;
    
    // determine day
    DrvRtc_ConvertRtcTimetoEpochTime(*rtc_time_ptr, &epoch_temp);
    rtc_time_ptr->day_of_month = (epoch_time - epoch_temp) / 86400 + 1;

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_ConvertEpochTimetoLocalRtcTime(EPOCH_TIME epoch_time_utc, RTC_TIME* rtc_time_ptr)
{
	EPOCH_TIME epoch_time = Rtc_ConvertUtcToLocalTime(epoch_time_utc);
	return DrvRtc_ConvertEpochTimetoRtcTime(epoch_time, rtc_time_ptr);
}
//------------------------------------------------------------------------------------------------//
// @remark  fills in the day of the week in the supplied date struct
BOOL DrvRtc_DetermineDayOfWeek(RTC_TIME* rtc_time_ptr)
{
    EPOCH_TIME  epoch_temp;
    
    if(DrvRtc_ConvertRtcTimetoEpochTime(*rtc_time_ptr, &epoch_temp))
    {
        epoch_temp /= 86400;
        rtc_time_ptr->day_of_week = (DAY_OF_WEEK)((epoch_temp + 3) % 7);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// GET/SET RTC parameters
// @remark  parameters are not saved within this module. Retaining must be handled in application
BOOL DrvRtc_GetParameter(RTC_PARA rtc_para, S16* value_ptr)
{
    switch(rtc_para)
    {
    case RTC_PARA_TIMEZONE:
        *value_ptr = (S16)rtc_para_struct.timezone;
        break;
        
    case RTC_PARA_AUTODAYLIGHTSAVINGTIME:
        *value_ptr = (S16)rtc_para_struct.auto_daylight_saving_time;
        break;
        
    default:
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRtc_SetParameter(RTC_PARA rtc_para, S16 value)
{
    switch(rtc_para)
    {
    case RTC_PARA_TIMEZONE:
        if((value < -11) || (value > 12))
        {
            return FALSE;
        }
        rtc_para_struct.timezone = (S8)value;
        break;
        
    case RTC_PARA_AUTODAYLIGHTSAVINGTIME:
        if((value < 0) || (value > 1))
        {
            return FALSE;
        }
        rtc_para_struct.auto_daylight_saving_time = (BOOL)value;
        break;
        
    default:
        return FALSE;
    }
    return TRUE;
}
//================================================================================================//
