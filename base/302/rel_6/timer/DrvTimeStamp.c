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
    if (handle != NULL && handle->hook_list_ptr != NULL && handle->hook_list_ptr->setup_hook != NULL)
    {
        return handle->hook_list_ptr->setup_hook(handle->timestamp_id, frequency);
    }    
    
    LOG_WRN("TimeStamp setup function is NULL");
    return 0;    
}
//------------------------------------------------------------------------------------------------//
BOOL DrvTimeStamp_TimeOutStillPending(TIMESTAMP_HNDL handle,TIMESTAMP timestamp, U32 waittime_in_ns)
{
    TIMESTAMP diff = DrvTimeStamp_GetTimeStamp(handle) - timestamp;
    if(((diff >> (handle->configured_unit_shift +2)) * handle->configured_unit_scaled) < waittime_in_ns) return TRUE; //still pending, timeout not yet passed
    return FALSE; //timeout has happened
}
//------------------------------------------------------------------------------------------------//
void DrvTimeStamp_Wait(TIMESTAMP_HNDL handle, U32 waittime_in_ns)
{
    TIMESTAMP start_value = DrvTimeStamp_GetTimeStamp(handle);
    //TIMESTAMP s = start_value; // for debugging purposes
    //TIMESTAMP d; // for debugging purposes
    TIMESTAMP diff = 0; 

    // no use waiting less than 1 microsecond, we're never that accurate...
    if (waittime_in_ns < 1000)
        return;
    
    // longer than this will always give overflow problems
    if (waittime_in_ns > (((TIMESTAMP)0 -1) >> 2))
        LOG_ERR("waiting more than 0x%x ns is impossible (0x%x received)", PU32((((TIMESTAMP)0 -1) >> 2)), PU32(waittime_in_ns));
    
    
    // 1: rough wait, which counts till (floor(waittime_in_ns/configured_unit_scaled)) << (configured_unit_shift +2)
    if (waittime_in_ns > handle->configured_unit_scaled)
    {        
        do
        {
            diff = DrvTimeStamp_GetTimeStamp(handle) - start_value;
        }
        while(((diff >> (handle->configured_unit_shift +2))) < (waittime_in_ns/handle->configured_unit_scaled));
            
        // here we make an inevitable rounding error... suggestions for improvement welcome! (BNRN)
        waittime_in_ns -= (((diff >> (handle->configured_unit_shift +2))) * handle->configured_unit_scaled);           
        
    }
                
    // 2 detailed last bit. Note that we first decrease diff, so we can multiply with configure_unit_scaled without fear for overflow    
    start_value = start_value + diff;
    
    do
    {
        diff = DrvTimeStamp_GetTimeStamp(handle) - start_value;
    }
    while((diff * (handle->configured_unit_scaled)) < (waittime_in_ns << (handle->configured_unit_shift +2)));
    
    // d = DrvTimeStamp_GetTimeStamp(handle) - s; // for debugging purposes
                
}
//------------------------------------------------------------------------------------------------//
void CalculateScaleAndUnit(TIMESTAMP_HNDL handle, U32 configured_frequency)
{
    if (configured_frequency == 0)
    {
        LOG_ERR("Frequency cannot be zero");
    }
    else
    {        
        U32 temp = configured_frequency;
        handle->configured_unit_shift = 0;
        // check how much we can shift before we lose accuracy
        while ((temp & 0x1) == 0x0)
        {
            temp >>= 1;
            (handle->configured_unit_shift)++;
        }
        
        // express the unit in scaled 250ps ticks, keeping the accuracy as high as possible
        handle->configured_unit_scaled = (4000000000 + temp -1) / temp;    
        
        // indication of the unit in ns, +2 because "configured_unit_scaled" is in 250ps ticks
        handle->configured_unit_in_ns = handle->configured_unit_scaled >> (handle->configured_unit_shift + 2);
    }
}
//================================================================================================//
