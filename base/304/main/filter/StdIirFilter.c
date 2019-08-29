//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the implementation of an IIR Filter.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define FILTER__STDIIRFILTER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef FILTER__STDIIRFILTER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               FILTER__STDIIRFILTER_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "filter/StdIirFilter.h"
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
void StdIirFilter_U16(U16* curr_val_ptr, U16 meas_val, U8 order, U16 delta_limit)
{
	U16     delta_val;

	if(meas_val == *curr_val_ptr)
	{
		return;
	}
	else if(meas_val > *curr_val_ptr)
	{
		delta_val = ((meas_val - *curr_val_ptr - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr += delta_val;
	}
	else
	{
		delta_val = ((*curr_val_ptr - meas_val - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr -= delta_val;
	}
}
//------------------------------------------------------------------------------------------------//
void StdIirFilter_S16(S16* curr_val_ptr, S16 meas_val, U8 order, S16 delta_limit)
{
	S16       delta_val;

	if(meas_val == *curr_val_ptr)
	{
		return;
	}
	else if(meas_val > *curr_val_ptr)
	{
		delta_val = ((meas_val - *curr_val_ptr - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr += delta_val;
	}
	else
	{
		delta_val = ((*curr_val_ptr - meas_val - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr -= delta_val;
	}
}
//------------------------------------------------------------------------------------------------//
void StdIirFilter_U32(U32* curr_val_ptr, U32 meas_val, U8 order, U32 delta_limit)
{
	U32     delta_val;

	if(meas_val == *curr_val_ptr)
	{
		return;
	}
	else if(meas_val > *curr_val_ptr)
	{
		delta_val = ((meas_val - *curr_val_ptr - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr += delta_val;
	}
	else
	{
		delta_val = ((*curr_val_ptr - meas_val - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr -= delta_val;
	}
}
//------------------------------------------------------------------------------------------------//
void StdIirFilter_S32(S32* curr_val_ptr, S32 meas_val, U8 order, S32 delta_limit)
{
	S32     delta_val;

	if(meas_val == *curr_val_ptr)
	{
		return;
	}
	else if(meas_val > *curr_val_ptr)
	{
		delta_val = ((meas_val - *curr_val_ptr - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr += delta_val;
	}
	else
	{
		delta_val = ((*curr_val_ptr - meas_val - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr -= delta_val;
	}
}
//------------------------------------------------------------------------------------------------//
void StdIirFilter_Multiplication_U32(U32* curr_val_ptr, U32 meas_val, U32 alpha, U8 order, U32 delta_limit)
{
	U32     delta_val;

	if(meas_val == *curr_val_ptr)
	{
		return;
	}
	else if(meas_val > *curr_val_ptr)
	{
		delta_val = ((((meas_val - *curr_val_ptr) * alpha) - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr += delta_val;
	}
	else
	{
		delta_val = ((((*curr_val_ptr - meas_val) * alpha) - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr -= delta_val;
	}
}
//------------------------------------------------------------------------------------------------//
void StdIirFilter_Multiplication_S32(S32* curr_val_ptr, S32 meas_val, U16 alpha, U8 order, S32 delta_limit)
{
	S32     delta_val;

	if(meas_val == *curr_val_ptr)
	{
		return;
	}
	else if(meas_val > *curr_val_ptr)
	{
		delta_val = ((((meas_val - *curr_val_ptr) * alpha) - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr += delta_val;
	}
	else
	{
		delta_val = ((((*curr_val_ptr - meas_val) * alpha) - 1) >> order) + 1;
		if((delta_limit > 0) && (delta_val > delta_limit))
		{
			delta_val = delta_limit;
		}
		*curr_val_ptr -= delta_val;
	}
}
//================================================================================================//
