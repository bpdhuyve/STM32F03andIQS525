//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// easy access methods for the TimeStamp module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TIMER__DRVTIMESTAMPSYS_C
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
//------------------------------------------------------------------------------------------------//
// @remark Number of Timers used to create a timestamp
#ifndef TIMER__DRVTIMESTAMP_COUNT
    #define TIMER__DRVTIMESTAMP_COUNT       1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "timer\SysTimeStamp.h"

// DRV
#include "timer\DrvTimeStampSys.h"
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
static TIMESTAMP DrvTimeStampSys_GetTimeStamp(TIMESTAMP_ID id);
static U32 DrvTimeStampSys_Setup(TIMESTAMP_ID id, U32 frequency);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static TIMESTAMP_STRUCT                            timestamp_handles[TIMER__DRVTIMESTAMP_COUNT];
static TIMESTAMP_HOOK_LIST                         timestamp_hook_list;
static U8                                          timestamp_handles_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static TIMESTAMP DrvTimeStampSys_GetTimeStamp(TIMESTAMP_ID id)
{
    return SysTimer_TimeStamp_GetTimeStamp((TIMER) id);
}
//------------------------------------------------------------------------------------------------//
static U32 DrvTimeStampSys_Setup(TIMESTAMP_ID id, U32 frequency)
{
    return SysTimer_TimeStamp_Setup((TIMER) id, frequency);
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvTimeStampSys_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)timestamp_handles, 0xFF, SIZEOF(timestamp_handles));
    
    timestamp_hook_list.get_timestamp_hook = DrvTimeStampSys_GetTimeStamp;
    timestamp_hook_list.setup_hook = DrvTimeStampSys_Setup;    
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
TIMESTAMP_HNDL DrvTimeStampSys_Register(TIMER timer, U32 frequency)
{
    U32 configured_frequency;
    TIMESTAMP_HNDL timestamp_hndl = &timestamp_handles[timestamp_handles_count];
    
    MODULE_CHECK();
    
    LOG_DEV("DrvTimeStampSys_Register");
    if(timestamp_handles_count < TIMER__DRVTIMESTAMP_COUNT)
    {
        configured_frequency = SysTimer_TimeStamp_Setup(timer, frequency);
        if(configured_frequency > 0)
        {
            timestamp_hndl->hook_list_ptr = &timestamp_hook_list;
            timestamp_hndl->timestamp_id = (TIMESTAMP_ID) timer;
            timestamp_hndl->frequency = configured_frequency;
            timestamp_handles_count++;
            return timestamp_hndl;
        }
        LOG_ERR("TimeStamp init failed");
        return NULL;
    }
    
    LOG_ERR("Timestamp count overrun - %d", PU8(timer));
    return NULL;
}
//================================================================================================//
