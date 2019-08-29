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
    U8  position;
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
STDFIRFILTER_HNDL StdFirFilter_CreateMovingAverage_U16(U8 order, U16 delta_limit)
{
    if (stdfirfilter_nb_assigned < FILTER_STDFIRFILTER_MAX_FILTERS)
    {
        STDFIRFILTER_STRUCT* current = &stdfirfilter_filters[stdfirfilter_nb_assigned];
        
        current->order = order;
        current->size = 1 << order;
        current->circ_buffer = CoreBuffer_CreateStaticU16(current->size, "StdFirFilter");
        memset((VPTR)current->circ_buffer, 0, current->size * sizeof(U16));
        current->delta_limit = (delta_limit == 0) ? 0xFFFF : delta_limit;
        current->position = 0;
        current->current_sum = 0;
        current->current_avg = 0;
        current->num_delta_abnormalities = 0xFF; // the filter is in an abnormal state at the beginning
        
        return stdfirfilter_nb_assigned++;
    }
    
    LOG_ERR("Max filter handles reached!");
    return STDFIRFILTER_INVALID_HNDL; 
}
//------------------------------------------------------------------------------------------------//
U16 StdFirFilter_UpdateMovingAverage_U16(STDFIRFILTER_HNDL filter_hndl, U16 data_point)
{
    if (filter_hndl < FILTER_STDFIRFILTER_MAX_FILTERS)
    {        
        STDFIRFILTER_STRUCT* filter = &stdfirfilter_filters[filter_hndl];
        
        BOOL is_abnormal_sample = (BOOL)(ABS(data_point - filter->current_avg) > filter->delta_limit);
        
        if (is_abnormal_sample)
        {
            // increase abnormality counter
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
    
    LOG_ERR("Invalid Filter handle: %d", PU16(filter_hndl));
    return 0;
}
//================================================================================================//
