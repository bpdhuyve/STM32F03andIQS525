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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvRtc_Init(void);

BOOL DrvRtc_SetTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr);

BOOL DrvRtc_GetTime(RTC_HNDL rtc_hndl, RTC_TIME* rtc_time_ptr);

BOOL DrvRtc_SetEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME epoch_time);

BOOL DrvRtc_GetEpochTime(RTC_HNDL rtc_hndl, EPOCH_TIME* epoch_time_ptr);

BOOL DrvRtc_ConvertRtcTimetoEpochTime(RTC_TIME rtc_time, EPOCH_TIME* epoch_time_ptr);

BOOL DrvRtc_ConvertEpochTimetoRtcTime(EPOCH_TIME epoch_time, RTC_TIME* rtc_time_ptr);

// @remark  fills in the day of the week in the supplied date struct
BOOL DrvRtc_DetermineDayOfWeek(RTC_TIME* rtc_time_ptr);
//================================================================================================//



#endif /* RTC__DRVRTC_H */