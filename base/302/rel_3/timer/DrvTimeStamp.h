//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// easy access methods for the TimeStamp module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef TIMER__DRVTIMESTAMP_H
#define TIMER__DRVTIMESTAMP_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          TIMESTAMP_ID;

typedef TIMESTAMP (*GET_TIMESTAMP_HOOK)(TIMESTAMP_ID id);
typedef U32 (*SETUP_HOOK)(TIMESTAMP_ID id, U32 frequency);


typedef struct
{
    GET_TIMESTAMP_HOOK                   get_timestamp_hook;
    SETUP_HOOK                           setup_hook;
}
TIMESTAMP_HOOK_LIST;

typedef struct
{
    TIMESTAMP_ID                    timestamp_id;
    TIMESTAMP                       configured_unit_in_ns;
    TIMESTAMP                       configured_unit_scaled;
    TIMESTAMP                       configured_unit_shift;
    TIMESTAMP_HOOK_LIST*	        hook_list_ptr;
}
TIMESTAMP_STRUCT;

typedef TIMESTAMP_STRUCT*         TIMESTAMP_HNDL;
//================================================================================================//


//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define TIMER__DRVTIMESTAMP_SYSTICK    1
#define TIMER__DRVTIMESTAMP_SYSTIMER   2

#if TIMER__DRVTIMESTAMP_DEFAULT_TIMER == TIMER__DRVTIMESTAMP_SYSTIMER
    #include "timer\DrvTimeStampSys.h"
    #define GET_TIMESTAMP()                                 DrvTimeStampSys_GetDefaultTimeStamp()
    #define GET_DEFAULT_TIMESTAMP_HANDLE()                  DrvTimeStampSys_GetDefaultHandle()
#elif TIMER__DRVTIMESTAMP_DEFAULT_TIMER == TIMER__DRVTIMESTAMP_SYSTICK
    #include "timer\DrvTimeStampSysTick.h"
    #define GET_TIMESTAMP()                                 DrvTimeStampSysTick_GetDefaultTimeStamp()
    #define GET_DEFAULT_TIMESTAMP_HANDLE()                  DrvTimeStampSysTick_GetDefaultHandle()
#else
    #error no default timestamp defined!
    #define GET_TIMESTAMP()                                 0
    #define GET_DEFAULT_TIMESTAMP_HANDLE()                  NULL
#endif

#define BLOCKING_WAIT(waittime_in_ns)                       DrvTimeStamp_Wait(GET_DEFAULT_TIMESTAMP_HANDLE(), waittime_in_ns)
#define TIMEOUT_STILL_PENDING(timestamp, waittime_in_ns)    DrvTimeStamp_TimeOutStillPending(GET_DEFAULT_TIMESTAMP_HANDLE(), timestamp, waittime_in_ns)
//================================================================================================//


//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void DrvTimeStamp_Init(void);

// @remark  redirect to the actual timestamp get
TIMESTAMP DrvTimeStamp_GetTimeStamp(TIMESTAMP_HNDL handle);

// @remark  redirect to the actual timestamp setup
U32 DrvTimeStamp_Setup(TIMESTAMP_HNDL handle, U32 frequency);

// @remark	returns TRUE as long as the timeout has not yet happened
BOOL DrvTimeStamp_TimeOutStillPending(TIMESTAMP_HNDL handle, TIMESTAMP timestamp, U32 waittime_in_ns);

// @remark  Blocking wait; Accuracy is around 0.002%
void DrvTimeStamp_Wait(TIMESTAMP_HNDL handle, U32 waittime_in_ns);

// @remark: calculates the unit and scale to keep the accuracy as high as possible
void CalculateScaleAndUnit(TIMESTAMP_HNDL handle, U32 configured_frequency);
//================================================================================================//



#endif /* DRV_TIMER_TIMESTAMP_H */
