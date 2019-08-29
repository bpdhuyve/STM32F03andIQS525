//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// // Implementation of the common part of the RTCl driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef RTC__DRVRTC_H
#define RTC__DRVRTC_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "rtc\ISysRtc.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          RTC_ID;

typedef BOOL (*RTC_SET_TIME_HOOK)(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr);

typedef BOOL (*RTC_GET_TIME_HOOK)(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr);

typedef struct
{
    RTC_SET_TIME_HOOK               set_time_hook;
    RTC_GET_TIME_HOOK               get_time_hook;
}
RTC_HOOK_LIST;

typedef struct
{
    RTC_HOOK_LIST*	                hook_list_ptr;
    RTC_ID                          rtc_id;
}
RTC_STRUCT;

typedef RTC_STRUCT*                 RTC_HNDL;

typedef U64                         EPOCH_TIME;

typedef enum
{
    RTC_PARA_TIMEZONE               = 0,
    RTC_PARA_AUTODAYLIGHTSAVINGTIME = 1,
}
RTC_PARA;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvRtc_Init(void);

// SET/GET LOCAL TIME (taking into account the timezone and the daylight saving time )
BOOL DrvRtc_SetLocalTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr);
BOOL DrvRtc_GetLocalTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr);
BOOL DrvRtc_SetLocalEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME epoch_time);
BOOL DrvRtc_GetLocalEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME* epoch_time_ptr);

// GET/SET UTC (or GMT) (not taking into account the timezone and the daylight saving time )
BOOL DrvRtc_SetUtcTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr);
BOOL DrvRtc_GetUtcTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr);
BOOL DrvRtc_SetUtcEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME epoch_time);
BOOL DrvRtc_GetUtcEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME* epoch_time_ptr);

// Convert RTC to Epoch and back
BOOL DrvRtc_ConvertRtcTimetoEpochTime(RTC_TIME rtc_time, EPOCH_TIME* epoch_time_ptr);
BOOL DrvRtc_ConvertEpochTimetoRtcTime(EPOCH_TIME epoch_time, RTC_TIME* rtc_time_ptr);

// @remark  fills in the day of the week in the supplied date struct
BOOL DrvRtc_DetermineDayOfWeek(RTC_TIME* rtc_time_ptr);

// GET/SET RTC parameters
// @remark  parameters are not saved within this module. Retaining must be handled in application
BOOL DrvRtc_GetParameter(RTC_PARA rtc_para, S16* value_ptr);
BOOL DrvRtc_SetParameter(RTC_PARA rtc_para, S16 value);
//================================================================================================//



//================================================================================================//
// REVERSE COMPATIBLE DEFINITION LIST
//------------------------------------------------------------------------------------------------//
#define DrvRtc_SetTime(rtc_hndl, rtc_time_ptr)              DrvRtc_SetUtcTime(rtc_hndl, rtc_time_ptr)
#define DrvRtc_GetTime(rtc_hndl, rtc_time_ptr)              DrvRtc_GetUtcTime(rtc_hndl, rtc_time_ptr)
//================================================================================================//



#endif /* RTC__DRVRTC_H */
