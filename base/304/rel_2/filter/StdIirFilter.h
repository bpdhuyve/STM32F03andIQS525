//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the implementation of an IIR Filter.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef FILTER__STDIIRFILTER_H
#define FILTER__STDIIRFILTER_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Function to filter U16 values
// This function filters the U16 input data with an IIR filter of a certain order.\n
// @param   curr_val_ptr: pointer to output value
// @param   meas_val:     new data point
// @param   order:        order of the IIR-filter
// @param	delta_limit:  maximum jump in output value (0 = no limit)
void StdIirFilter_U16(U16* curr_val_ptr, U16 meas_val, U8 order, U16 delta_limit);

// @brief   Function to filter S16 values
// This function filters the S16 input data with an IIR filter of a certain order.\n
// @param   curr_val_ptr: pointer to output value
// @param   meas_val:     new data point
// @param   order:        order of the IIR-filter
// @param	delta_limit:  maximum jump in output value (0 = no limit)
void StdIirFilter_S16(S16* curr_val_ptr, S16 meas_val, U8 order, S16 delta_limit);

// @brief   Function to filter U32 values
// This function filters the U32 input data with an IIR filter of a certain order.\n
// @param   curr_val_ptr: pointer to output value
// @param   meas_val:     new data point
// @param   order:        order of the IIR-filter
// @param	delta_limit:  maximum jump in output value (0 = no limit)
void StdIirFilter_U32(U32* curr_val_ptr, U32 meas_val, U8 order, U32 delta_limit);

// @brief   Function to filter S32 values
// This function filters the S32 input data with an IIR filter of a certain order.\n
// @param   curr_val_ptr: pointer to output value
// @param   meas_val:     new data point
// @param   order:        order of the IIR-filter
// @param	delta_limit:  maximum jump in output value (0 = no limit)
void StdIirFilter_S32(S32* curr_val_ptr, S32 meas_val, U8 order, S32 delta_limit);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* FILTER__STDIIRFILTER_H */
