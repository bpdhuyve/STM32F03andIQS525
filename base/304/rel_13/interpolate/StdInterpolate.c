//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to perform interpolations
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define INTERPOLATE_STDINTERPOLATE_H_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef INTERPOLATE_STDINTERPOLATE_H_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               INTERPOLATE_STDINTERPOLATE_H_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STD lib include section
#include "StdInterpolate.h"
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
void StdInterpolate_Init(void)
{
    // nothing to be done
}
//------------------------------------------------------------------------------------------------//
BOOL StdInterpolate_EquidistantY(U16 x_value, INTERP_EQUIDISTANT_Y* interp_data, S16* y_value_ptr)
{
    U16     index;
    S32     return_val;
    
    // first some checks if data is valuable
    if(interp_data->y_step == 0)
    {
        LOG_ERR("interpolation y-step = 0");
        return FALSE;
    }
    if(interp_data->table_length <= 1)
    {
        LOG_ERR("interpolation table length <= 1");
        return FALSE;
    }
    for(index = 1; index < interp_data->table_length; index++)
    {
        if(interp_data->x_values_table_ptr[index] == interp_data->x_values_table_ptr[index - 1])
        {
            LOG_ERR("x-curve is not monotonous");
            return FALSE;
        }
    }
    
    // do interpolation
    if(interp_data->x_values_table_ptr[0] < interp_data->x_values_table_ptr[interp_data->table_length - 1])
    {
        // ascending x-table
        if(x_value <= interp_data->x_values_table_ptr[0])
        {
            *y_value_ptr = interp_data->y_first;
            return (BOOL)(x_value == interp_data->x_values_table_ptr[0]);
        }
        else if(x_value >= interp_data->x_values_table_ptr[interp_data->table_length - 1])
        {
            *y_value_ptr = interp_data->y_first + ((S16)(interp_data->table_length - 1) * interp_data->y_step);
            return (BOOL)(x_value == interp_data->x_values_table_ptr[interp_data->table_length - 1]);
        }
        else
        {
            index = 1;
            while(x_value > interp_data->x_values_table_ptr[index])
            {
                index++;
            }
        }
    }
    else
    {
        // descending x-table
        if(x_value >= interp_data->x_values_table_ptr[0])
        {
            *y_value_ptr = interp_data->y_first;
            return (BOOL)(x_value == interp_data->x_values_table_ptr[0]);
        }
        else if(x_value <= interp_data->x_values_table_ptr[interp_data->table_length - 1])
        {
            *y_value_ptr = interp_data->y_first + ((S16)(interp_data->table_length - 1) * interp_data->y_step);
            return (BOOL)(x_value == interp_data->x_values_table_ptr[interp_data->table_length - 1]);
        }
        else
        {
            index = 1;
            while(x_value < interp_data->x_values_table_ptr[index])
            {
                index++;
            }
        }
    }
    
    //           y1 - y0                                                y_step
    // y = y0 + --------- * (x - x0)  =  y_first + (index-1)*y_step + --------- * (x - x0)
    //           x1 - x0                                               x1 - x0
    return_val = (S32)(x_value - interp_data->x_values_table_ptr[index - 1]);
    return_val *= (S32)interp_data->y_step;
    return_val += ((S32)(interp_data->x_values_table_ptr[index] - interp_data->x_values_table_ptr[index - 1]) >> 1);
    return_val /= (S32)(interp_data->x_values_table_ptr[index] - interp_data->x_values_table_ptr[index - 1]);
    return_val += (S32)interp_data->y_first + ((S32)(index - 1) * (S32)interp_data->y_step);
    
    *y_value_ptr = (S16)return_val;
    
    return TRUE;
}
//================================================================================================//

