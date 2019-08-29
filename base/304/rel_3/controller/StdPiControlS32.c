//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains an alternative implementation of the PI control std lib module
// Ported from platform 1 (StdPiControl2.c)
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CONTROLLER__STDPICONTROL_C
//================================================================================================//

//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CONTROLLER__STDPICONTROL_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               CONTROLLER__STDPICONTROL_LOG_LEVEL
#endif
#ifndef PI_CONTROLLER_COUNT
    #error "PI_CONTROLLER_COUNT not defined in AppConfig.h"
#endif
//================================================================================================//


//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "controller/StdPiControlS32.h"
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
    S32     kp;
    S32     kp_scale;
    S32     ki;
    S32     ki_scale;
    S32     i_action;
    S32     prev_action;
    S32     pos_limit;
    S32     neg_limit;
    S32		pos_error_limit;
    S32		neg_error_limit;
}
STD_PI_PARAMS;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static S32 ConvertAwLimitToMaxErrorLimit(S32 aw_limit, S32 ki, S32 ki_scale);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static STD_PI_PARAMS      	std_pi_params[PI_CONTROLLER_COUNT];
static STD_PI_CTRL_HNDL 	std_pi_registered_controllers;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
/**
 * @brief   Conversion of anti-windup limit together with the Ki to the maximum sum or errors
 *
 * @param 	aw_limit : 	anti windup limit (-32768...32767)
 * @param	ki :		the Ki mantisse (0...32767)
 * @param	ki_scale : 	the Ki scale (0..16)
 *
 * @return 	the maximum (pos/neg) sum of errors
 */
static S32 ConvertAwLimitToMaxErrorLimit(S32 aw_limit, S32 ki, S32 ki_scale)
{
	if(ki == 0)
	{
		return (((S32)aw_limit) << ki_scale);
	}
	else
	{
		return ((((S32)aw_limit) << ki_scale) / (S32)ki);
	}
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdPiControlInit(void)
{
    MODULE_INIT_ONCE();
    STD_PI_PARAMS* ptr;

    ptr = std_pi_params;
    do
    {
        ptr->kp         		= 0;
        ptr->kp_scale   		= 10;
        ptr->ki         		= 0;
        ptr->ki_scale   		= 10;
        ptr->i_action   		= 0;
        ptr->pos_limit  		= 0;
        ptr->neg_limit  		= 0;
        ptr->pos_error_limit	= 0;
        ptr->neg_error_limit	= 0;
        ptr++;
    }
    while(ptr < &std_pi_params[PI_CONTROLLER_COUNT]);

    std_pi_registered_controllers = 0;

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
STD_PI_CTRL_HNDL StdPiRegisterController(S32 kp,
                                         S32 kp_scale,
                                         S32 ki,
                                         S32 ki_scale,
                                         S32 pos_limit,
                                         S32 neg_limit)
{
    STD_PI_PARAMS* ptr;

    if((kp_scale < 0) || (ki_scale < 0) || (kp_scale > 16) || (ki_scale > 16) || (kp < 0) || (ki < 0))
    {
        LOG_ERR("Invalid PI parameter");
        return INVALID_STD_PI_CTRL_HNDL;
    }

    if(std_pi_registered_controllers >= PI_CONTROLLER_COUNT)
    {
        LOG_ERR("max nb of PI controllers exceeded");
        return INVALID_STD_PI_CTRL_HNDL;
    }

    ptr = &std_pi_params[std_pi_registered_controllers];

    ptr->kp         		= kp;
    ptr->kp_scale   		= kp_scale;
    ptr->ki         		= ki;
    ptr->ki_scale   		= ki_scale;
    ptr->pos_limit  		= pos_limit;
    if(neg_limit == 0x8000)
    {
    	ptr->neg_limit  	= 0x8001;
    }
    else
    {
    	ptr->neg_limit  	= neg_limit;
    }
    ptr->pos_error_limit	= ConvertAwLimitToMaxErrorLimit(ptr->pos_limit, ki, ki_scale);
    ptr->neg_error_limit	= ConvertAwLimitToMaxErrorLimit(ptr->neg_limit, ki, ki_scale);

    std_pi_registered_controllers++;

    return (std_pi_registered_controllers - 1);
}
//------------------------------------------------------------------------------------------------//
S32 StdPiController(STD_PI_CTRL_HNDL hndl, S32 set_val, S32 meas_val)
{
	S32       input_error;
    S32       p_term;
    S32       i_term;
    S32       output;
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
        return(0);
    }

    ptr = &std_pi_params[hndl];

	//calculate input error
	//---------------------
	//remark: because input params are signed 16: error = [-65535 ... +65535]
	//so: this error * 32767 (max mantisse) can't give a 32 bit overflow
    input_error = (S32)set_val - (S32)meas_val;

	//calculate P term
	//----------------
    p_term = ((S32)(ptr->kp) * input_error) >> (ptr->kp_scale);

	//calculate I term + limit at anti-windup limits
	//----------------------------------------------
	if((ptr->ki) == 0)
	{
		i_term = (ptr->i_action) >> (ptr->ki_scale);
	}
	else
	{
		i_term = ((S32)(ptr->ki) * (ptr->i_action)) >> (ptr->ki_scale);
	}

	//calculate P+I term + limit at anti-windup limits
	//------------------------------------------------
	output = p_term + i_term;

	if(output > (S32)(ptr->pos_limit))
    {
        output = (S32)(ptr->pos_limit);
    }
    else if(output < (S32)(ptr->neg_limit))
    {
    	output = (S32)(ptr->neg_limit);
    }
    else
    {
    	ptr->i_action += input_error;
    	//recalculate I term with actual contribution included
    	if((ptr->ki) == 0)
		{
			i_term = (ptr->i_action) >> (ptr->ki_scale);
		}
		else
		{
			i_term = ((S32)(ptr->ki) * (ptr->i_action)) >> (ptr->ki_scale);
		}

    	output = p_term + i_term;

    	if(output > (S32)(ptr->pos_limit))
	    {
	        output = (S32)(ptr->pos_limit);
	    }
	    else if(output < (S32)(ptr->neg_limit))
	    {
	    	output = (S32)(ptr->neg_limit);
	    }
    }

    return((S32)output);
}
//------------------------------------------------------------------------------------------------//
S32 StdPiControllerVelocityForm(STD_PI_CTRL_HNDL hndl, S32 prev_output, S32 set_val, S32 meas_val)
{
    // alternate form of PI controller using the velocity form. Only the prev_error is stored in i_action.
	S32       input_error;
    S32       output;
    S32       p_term;
    S32       i_term;
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
        return(0);
    }

    ptr = &std_pi_params[hndl];

	//calculate input error
	//---------------------
	//remark: because input params are signed 16: error = [-65535 ... +65535]
	//so: this error * 32767 (max mantisse) can't give a 32 bit overflow
    input_error = (S32)set_val - (S32)meas_val;

	//calculate P term
	//----------------
    p_term = ((S32)(ptr->kp) * (input_error - ptr->prev_action)) >> (ptr->kp_scale);

	//calculate I term
	//----------------
	if((ptr->ki) == 0)
	{
		i_term = 0;
	}
	else
	{
		i_term = ((S32)(ptr->ki) * (input_error)) >> (ptr->ki_scale);
	}

	//calculate prev + P + I term + limit at anti-windup limits
	//---------------------------------------------------------
	output = prev_output + p_term + i_term;

	if(output > (S32)(ptr->pos_limit))
    {
        output = (S32)(ptr->pos_limit);
    }
    else if(output < (S32)(ptr->neg_limit))
    {
    	output = (S32)(ptr->neg_limit);
    }

    ptr->prev_action = input_error;

    return((S32)output);
}
//------------------------------------------------------------------------------------------------//
void StdPiControlChangeKpParams(STD_PI_CTRL_HNDL hndl, S32 kp, S32 kp_scale)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    ptr->kp         = kp;
    ptr->kp_scale   = kp_scale;
}
//------------------------------------------------------------------------------------------------//
void StdPiControlChangeKiParams(STD_PI_CTRL_HNDL hndl, S32 ki, S32 ki_scale)
{
    STD_PI_PARAMS*  ptr;
    S32		new_ki;
    S32		old_ki;
    S32 		i_action;
    S32 		i_action_scale;
    S32		max_shift;
	S32		shift;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];

    if((ptr->ki != ki) || (ptr->ki_scale != ki_scale))
    {
    	//recalculate new I_action: Ki_old * Iaction_old = Ki_new * Iaction_new
    	if(ki == 0)
    	{
			new_ki = 1;
		}
		else
		{
			new_ki = ki;
		}
		if((ptr->ki) == 0)
		{
			old_ki = 1;
		}
		else
		{
			old_ki = ptr->ki;
		}

		i_action = ptr->i_action;
		i_action_scale = 0;

		while((i_action > 65538) || (i_action < -65538))
		{
			i_action >>= 1;
			i_action_scale++;
		}

		i_action = (S32)old_ki * i_action;
		if(new_ki != 1)
		{
			i_action = i_action / (S32)new_ki;
		}
		shift = (ptr->ki_scale) - i_action_scale - ki_scale;
		if(shift >= 0)
		{
			i_action >>= shift;
		}
		else	//shift left as much as possible (just until overflow would happen)
		{
			max_shift = -shift;
			shift = 0;
			if(i_action >= 0)
			{
				while((shift < max_shift) && ((i_action & 0xC0000000) == 0))
				{
					i_action <<= 1;
					shift++;
				}
			}
			else
			{
				while((shift < max_shift) && ((i_action & 0xC0000000) == 0xC0000000))
				{
					i_action <<= 1;
					shift++;
				}
			}
		}

		//limit new I action to limits
		ptr->pos_error_limit = ConvertAwLimitToMaxErrorLimit(ptr->pos_limit, ki, ki_scale);
    	ptr->neg_error_limit = ConvertAwLimitToMaxErrorLimit(ptr->neg_limit, ki, ki_scale);

		if(i_action > ptr->pos_error_limit)
		{
			ptr->i_action = ptr->pos_error_limit;
		}
		else if(i_action < ptr->neg_error_limit)
		{
			ptr->i_action = ptr->neg_error_limit;
		}
		else
		{
			ptr->i_action = i_action;
		}

		//use new Ki
		ptr->ki         = ki;
    	ptr->ki_scale   = ki_scale;
	}
}
//------------------------------------------------------------------------------------------------//
void StdPiControlSetKpAndKi(STD_PI_CTRL_HNDL hndl, S32 kp, S32 ki)
{
	StdPiControlSetKp(hndl, kp);
	StdPiControlSetKi(hndl, ki);
}
//------------------------------------------------------------------------------------------------//
void StdPiControlSetKp(STD_PI_CTRL_HNDL hndl, S32 kp)
{
	STD_PI_PARAMS*  ptr;

	if(hndl >= std_pi_registered_controllers)
	{
		LOG_ERR("Invalid PI_CTRL_HNDL");
	}

	ptr = &std_pi_params[hndl];
	ptr->kp = kp;
}
//------------------------------------------------------------------------------------------------//
void StdPiControlSetKi(STD_PI_CTRL_HNDL hndl, S32 ki)
{
	STD_PI_PARAMS*  ptr;
    S32		new_ki;
    S32		old_ki;
    S32 		i_action;
    S32 		i_action_scale;
    S32		shift;

	if(hndl >= std_pi_registered_controllers)
	{
		LOG_ERR("Invalid PI_CTRL_HNDL");
	}

	ptr = &std_pi_params[hndl];
	if(ptr->ki != ki)
	{
		//recalculate new I_action: Ki_old * Iaction_old = Ki_new * Iaction_new
		if(ki == 0)
		{
			new_ki = 1;
		}
		else
		{
			new_ki = ki;
		}
		if((ptr->ki) == 0)
		{
			old_ki = 1;
		}
		else
		{
			old_ki = ptr->ki;
		}

		i_action = ptr->i_action;
		i_action_scale = 0;

		while((i_action > 65538) || (i_action < -65538))
		{
			i_action >>= 1;
			i_action_scale++;
		}

		i_action = (S32)old_ki * i_action;
		if(new_ki != 1)
		{
			i_action = i_action / (S32)new_ki;
		}

		//shift left as much as possible (just until overflow would happen)
		{
			shift = 0;
			if(i_action >= 0)
			{
				while((shift < i_action_scale) && ((i_action & 0xC0000000) == 0))
				{
					i_action <<= 1;
					shift++;
				}
			}
			else
			{
				while((shift < i_action_scale) && ((i_action & 0xC0000000) == 0xC0000000))
				{
					i_action <<= 1;
					shift++;
				}
			}
		}

		//limit new I action to limits
		ptr->pos_error_limit = ConvertAwLimitToMaxErrorLimit(ptr->pos_limit, ki, ptr->ki_scale);
		ptr->neg_error_limit = ConvertAwLimitToMaxErrorLimit(ptr->neg_limit, ki, ptr->ki_scale);

		if(i_action > ptr->pos_error_limit)
		{
			ptr->i_action = ptr->pos_error_limit;
		}
		else if(i_action < ptr->neg_error_limit)
		{
			ptr->i_action = ptr->neg_error_limit;
		}
		else
		{
			ptr->i_action = i_action;
		}

		//use new Ki
		ptr->ki = ki;
	}
}
//------------------------------------------------------------------------------------------------//
void StdPiControlChangeAwParams(STD_PI_CTRL_HNDL hndl, S32 pos_limit, S32 neg_limit)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    ptr->pos_limit = pos_limit;
    if(neg_limit == 0x8000)
    {
    	ptr->neg_limit = 0x8001;
    }
    else
    {
    	ptr->neg_limit = neg_limit;
    }
	ptr->pos_error_limit = ConvertAwLimitToMaxErrorLimit(ptr->pos_limit, ptr->ki, ptr->ki_scale);
	ptr->neg_error_limit = ConvertAwLimitToMaxErrorLimit(ptr->neg_limit, ptr->ki, ptr->ki_scale);

	if(ptr->i_action > ptr->pos_error_limit)
	{
		ptr->i_action = ptr->pos_error_limit;
	}
	else if (ptr->i_action < ptr->neg_error_limit)
	{
		ptr->i_action = ptr->neg_error_limit;
	}
}
//------------------------------------------------------------------------------------------------//
void StdPiControlResetIaction(STD_PI_CTRL_HNDL hndl)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    ptr->i_action = 0;
}
//------------------------------------------------------------------------------------------------//
void StdPiControlInitIaction(STD_PI_CTRL_HNDL hndl, S32 value)
{
	STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    ptr->i_action = value;
}
//------------------------------------------------------------------------------------------------//
S32 StdPiControlGetIaction(STD_PI_CTRL_HNDL hndl)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    return(ptr->i_action);
}
//------------------------------------------------------------------------------------------------//
S32 StdPiControlGetKp(STD_PI_CTRL_HNDL hndl)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    return(ptr->kp);
}
//------------------------------------------------------------------------------------------------//
S32 StdPiControlGetKpScale(STD_PI_CTRL_HNDL hndl)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    return(ptr->kp_scale);
}
//------------------------------------------------------------------------------------------------//
S32 StdPiControlGetKi(STD_PI_CTRL_HNDL hndl)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    return(ptr->ki);
}
//------------------------------------------------------------------------------------------------//
S32 StdPiControlGetKiScale(STD_PI_CTRL_HNDL hndl)
{
    STD_PI_PARAMS*  ptr;

    if(hndl >= std_pi_registered_controllers)
    {
        LOG_ERR("Invalid PI_CTRL_HNDL");
    }

    ptr = &std_pi_params[hndl];
    return(ptr->ki_scale);
}
//================================================================================================//
