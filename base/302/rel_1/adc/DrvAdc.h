//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the ADC driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ADC__DRVADC_H
#define ADC__DRVADC_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "adc\ISysAdc.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          ADC_CHANNEL_ID;

typedef BOOL (*ADC_START_CONVERSION)(ADC_CHANNEL_ID channel_id, U16* value_ptr);

typedef struct
{
    ADC_START_CONVERSION            start_conversion_hook;
}
ADC_CHANNEL_HOOK_LIST;

typedef struct
{
    ADC_CHANNEL_HOOK_LIST*	        hook_list_ptr;
    ADC_CHANNEL_ID                  channel_id;
    EVENT_CALLBACK                  conversion_complete_callback;
}
ADC_CHANNEL_STRUCT;

typedef ADC_CHANNEL_STRUCT*         ADC_CHANNEL_HNDL;

typedef void (*DRVADC_CONVERSION_COMPLETE)(ADC_CHANNEL_HNDL channel_hndl);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark none
void DrvAdc_Init(void);

// @remark none
BOOL DrvAdc_RegisterConversionComplete(ADC_CHANNEL_HNDL channel_hndl, EVENT_CALLBACK conversion_complete_callback);

// @remark none
BOOL DrvAdc_StartConversion(ADC_CHANNEL_HNDL channel_hndl, U16* value_ptr, BOOL wait_for_result);

// @remark function is only ment to be called from a specific sysAdc file !
void DrvAdc_ConversionComplete(ADC_CHANNEL_HNDL channel_hndl);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* ADC__DRVADC_H */
