//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the implementation of an FIR Filter.
//
// Copyright (c), PsiControl NV, All rights reserved.
//================================================================================================//
#ifndef FILTER__STDFIRFILTER_H
#define FILTER__STDFIRFILTER_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define STDFIRFILTER_INVALID_HNDL 0xFF
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8      STDFIRFILTER_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Function to init the FirFilter data structures to zero
void StdFirFilter_Init(void);

// @brief   Function to create a filter for U16 values
// This function creates a U16 input data with an IIR filter of a certain order.\n
// @param   order:        order (size) of the FIR-filter
// @param	delta_limit:  maximum jump in output value (0 = no limit)
// @return  handle to the new filter
STDFIRFILTER_HNDL StdFirFilter_CreateMovingAverage_U16(U8 order, U16 delta_limit);

// @brief   Function to reset a filter to a new average
// This function will fill up the whole buffer with a fixed value, to reset the average\n
// This can be used to "jump" to a new steady state, e.g. during startup
// @param   filter:       handle to the FIR filter
// @param	fixed value:  the new average to be set
// @return  new average
void StdFirFilter_ResetMovingAverage_U16(STDFIRFILTER_HNDL filter_hndl, U16 fixed_value);

// @brief   Function to update
// This function takes in a new sample and decides wether to add it based on the delta limit restrictions.\n
// when the new sample differs more than +-delta from the current average, the sample is rejected
// however, when more than size/2 abnormal data points occur, the sample is accepted, to accomodate for a new steady state
// when the sample doesn't differ more than +- delta, the abnormality counter is reset
// @param   filter:       handle to the FIR filter
// @param	data point:   new sample to be added to the filter
// @return  new average
U16 StdFirFilter_UpdateMovingAverage_U16(STDFIRFILTER_HNDL filter_hndl, U16 data_point);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* FILTER__STDFIRFILTER_H */
