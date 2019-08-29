//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the implementation of an FIR Filter.
//
// Copyright (c), PsiControl NV, All rights reserved.
//================================================================================================//
#define FILTER__STDFIRFILTER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef FILTER__STDFIRFILTER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               FILTER__STDIIRFILTER_LOG_LEVEL
#endif

#ifndef FILTER_STDFIRFILTER_MAX_FILTERS
	#define FILTER_STDFIRFILTER_MAX_FILTERS               10
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "filter/StdFirFilter.h"
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
    U16* circ_buffer;
    U8  order;
    U16 size;
    U16 delta_limit;
    U16  position;
    U32 current_sum;
    U16 current_avg;
    U8 num_delta_abnormalities;
} STDFIRFILTER_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static STDFIRFILTER_STRUCT stdfirfilter_filters[FILTER_STDFIRFILTER_MAX_FILTERS];

static U8 stdfirfilter_nb_assigned;
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
void StdFirFilter_Init(void)
{
    memset((VPTR)stdfirfilter_filters, 0, sizeof(stdfirfilter_filters));
    stdfirfilter_nb_assigned = 0;
}
//------------------------------------------------------------------------------------------------//
STDFIRFILTER_HNDL StdFirFilter_CreateMovingAverage_U16(U8 order, U16 delta_limit)
{
    
    if (stdfirfilter_nb_assigned >= FILTER_STDFIRFILTER_MAX_FILTERS)
    {    
        LOG_ERR("StdFirFilter: max filter handles reached!");
        return STDFIRFILTER_INVALID_HNDL;         
    }
    
    if (order > 15)
    {
        LOG_ERR("StdFirFilter: cannot create a filter with order bigger than 15, got %d", PU8(order));
        return STDFIRFILTER_INVALID_HNDL;         
    }
    
    STDFIRFILTER_STRUCT* filter = &stdfirfilter_filters[stdfirfilter_nb_assigned];
    
    filter->order = order;
    filter->size = 1 << order;
    filter->circ_buffer = CoreBuffer_CreateStaticU16(filter->size, "StdFirFilter");
    memset((VPTR)filter->circ_buffer, 0, filter->size * sizeof(U16));
    filter->delta_limit = (delta_limit == 0) ? 0xFFFF : delta_limit;
    filter->position = 0;
    filter->current_sum = 0;
    filter->current_avg = 0;
    filter->num_delta_abnormalities = 0xFF; // the filter is in an abnormal state at the beginning
    
    return stdfirfilter_nb_assigned++;
}
//------------------------------------------------------------------------------------------------//
void StdFirFilter_ResetMovingAverage_U16(STDFIRFILTER_HNDL filter_hndl, U16 fixed_value)
{
    if (filter_hndl >= stdfirfilter_nb_assigned)
    {        
        LOG_ERR("Invalid Filter handle: %d", PU16(filter_hndl));       
    }
    
    STDFIRFILTER_STRUCT* filter = &stdfirfilter_filters[filter_hndl];
    U16 i;
    
    // fill the circular buffer with this value
    for (i = 0; i < filter->size; i++)
        filter->circ_buffer[i] = fixed_value;
    
    filter->current_sum = fixed_value << filter->order;
    filter->current_avg = fixed_value;
    filter->num_delta_abnormalities = 0; // when resetting the filter, the abnormality counter is also reset    
    
}
//------------------------------------------------------------------------------------------------//
U16 StdFirFilter_UpdateMovingAverage_U16(STDFIRFILTER_HNDL filter_hndl, U16 data_point)
{
    if (filter_hndl >= stdfirfilter_nb_assigned)
    {        
        LOG_ERR("Invalid Filter handle: %d", PU16(filter_hndl));
        return 0;        
    }
    
    STDFIRFILTER_STRUCT* filter = &stdfirfilter_filters[filter_hndl];
    
    // check wether the new sample is within [avg - delta, avg + delta], if not, it's "abnormal"
    BOOL is_abnormal_sample = (BOOL)(ABS(data_point - filter->current_avg) > filter->delta_limit);
    
    if (is_abnormal_sample)
    {
        // increase abnormality counter, top at 0xFF
        if (filter->num_delta_abnormalities < 0xFF)
            filter->num_delta_abnormalities++;
    }
    else
    {
        // reset abnormality counter
        filter->num_delta_abnormalities = 0;
    }
    
    if (! is_abnormal_sample ||
        filter->num_delta_abnormalities > (filter->size/2))
    {
        // accept sample when the data is not abnormal, or when there have been more than "size/2" abnormal points
        filter->current_sum += ((U32)data_point - filter->circ_buffer[filter->position]);
        filter->current_avg = filter->current_sum >> filter->order;
        
        filter->circ_buffer[filter->position] = data_point;
        
        filter->position++;
        if (filter->position >= filter->size)
            filter->position = 0;
    }
    
    return filter->current_avg;
}
//================================================================================================//
