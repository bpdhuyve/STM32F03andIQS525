//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// easy access methods for the TimeStamp module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TIMER__DRVTIMESTAMP_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TIMER__DRVTIMESTAMP_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TIMER__DRVTIMESTAMP_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section

//COM lib include section

//APP include section
#include "timer\DrvTimeStamp.h"
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

//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static TIMESTAMP_HNDL           drvtimestamp_defaulthandle;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvTimeStamp_Init(void)
{
    MODULE_INIT_ONCE();
    
    drvtimestamp_defaulthandle = NULL;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void DrvTimeStamp_RegisterDefault(TIMESTAMP_HNDL handle)
{
    MODULE_CHECK();
    
    drvtimestamp_defaulthandle = handle;
}
//------------------------------------------------------------------------------------------------//
TIMESTAMP_HNDL DrvTimeStamp_GetDefaultHandle(void)
{
    MODULE_CHECK();
    
    return drvtimestamp_defaulthandle;
}
//------------------------------------------------------------------------------------------------//
TIMESTAMP DrvTimeStamp_GetTimeStamp(TIMESTAMP_HNDL handle)
{
    if (handle != NULL && handle->hook_list_ptr != NULL && handle->hook_list_ptr->get_timestamp_hook != NULL)
    {
        return handle->hook_list_ptr->get_timestamp_hook(handle->timestamp_id);
    }    
    
    LOG_WRN("TimeStamp get timestamp function is NULL");
    return 0;    
}
//------------------------------------------------------------------------------------------------//
U32 DrvTimeStamp_Setup(TIMESTAMP_HNDL handle, U32 frequency)
{
    U32 config_freq;
    
    if (handle != NULL && handle->hook_list_ptr != NULL && handle->hook_list_ptr->setup_hook != NULL)
    {
        config_freq = handle->hook_list_ptr->setup_hook(handle->timestamp_id, frequency);
        if(config_freq > 0)
        {
            handle->frequency = config_freq;
        }
        return config_freq;
    }    
    
    LOG_WRN("TimeStamp setup function is NULL");
    return 0;    
}
//------------------------------------------------------------------------------------------------//
BOOL DrvTimeStamp_TimeOutStillPending(TIMESTAMP_HNDL handle, TIMESTAMP timestamp, U32 waittime_in_ns)
{
    U32         wait_time_in_ticks = (U32)(((U64)waittime_in_ns * (U64)handle->frequency) / (U64)1e9);
    TIMESTAMP   diff_in_ticks;
    
    // longer than this will always give overflow problems
    if(((U32)(-1) != (U32)((TIMESTAMP)(-1))) && (wait_time_in_ticks > (U32)((TIMESTAMP)(-1))))
    {
        LOG_ERR("waiting more than 0x%x ticks is impossible (0x%x received)", PU32((TIMESTAMP)(-1)), PU32(wait_time_in_ticks));
    }
    
    diff_in_ticks = DrvTimeStamp_GetTimeStamp(handle) - timestamp;
    return (BOOL)(diff_in_ticks < wait_time_in_ticks);
}
//------------------------------------------------------------------------------------------------//
void DrvTimeStamp_Wait(TIMESTAMP_HNDL handle, U32 waittime_in_ns)
{
    TIMESTAMP   start_value_in_ticks = DrvTimeStamp_GetTimeStamp(handle);
    TIMESTAMP   diff_in_ticks; 
    U32         wait_time_in_ticks = (U32)(((U64)waittime_in_ns * (U64)handle->frequency) / (U64)1e9);

    // no use waiting less than 1 microsecond, we're never that accurate...
    if(waittime_in_ns < 1000)
    {
        return;
    }
    
    // longer than this will always give overflow problems
    if(((U32)(-1) != (U32)((TIMESTAMP)(-1))) && (wait_time_in_ticks > (U32)((TIMESTAMP)(-1))))
    {
        LOG_ERR("waiting more than 0x%x ticks is impossible (0x%x received)", PU32((TIMESTAMP)(-1)), PU32(wait_time_in_ticks));
    }
    
    do
    {
        diff_in_ticks = DrvTimeStamp_GetTimeStamp(handle) - start_value_in_ticks;
    }
    while(diff_in_ticks < wait_time_in_ticks);
}
//================================================================================================//
