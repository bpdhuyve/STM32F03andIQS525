//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of PID controller (based on platform 1 version)
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STDPIDCONTROLLER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDPIDCONTROLLER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STDPIDCONTROLLER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef PID_CTRL_COUNT
    #define PID_CTRL_COUNT              1
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
#include "StdPidController.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SIGN(x)                      (S32)(x & 0x80000000)
#define MAX_SIGNED32                 0x7FFFFFFF
#define MIN_SIGNED32                 0x80000001
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U16 kp;
    U8  kp_scale;
    U16 ki;
    U8  ki_scale;
    U16 kd;
    U8  kd_scale;
    S16 pos_limit;
    S16 neg_limit;
    S32 i_action;
    S32 d_action;
    STRING info;
    BOOL print_debug_info;
}
PID_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static PID_CTRL_STRUCT              pid_ctrl[PID_CTRL_COUNT];
static U8                           pid_ctrl_count;
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
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_PidInfo(void)
{
    PID_CTRL_STRUCT* ptr;
    
	if(CoreTerm_GetArgumentAsU32(0) >= pid_ctrl_count)
	{
        CoreTerm_PrintFailed();
		return;
    }
    
    ptr = &pid_ctrl[CoreTerm_GetArgumentAsU32(0)];
    
    LOG_TRM("PID %s", PCSTR(ptr->info));
    LOG_TRM(" 0. KP         : %d", PU16(ptr->kp));
    LOG_TRM(" 1. KP_SCALE   : %d", PU8(ptr->kp_scale));
    LOG_TRM(" 2. KI         : %d", PU16(ptr->ki));
    LOG_TRM(" 3. KI_SCALE   : %d", PU8(ptr->ki_scale));
    LOG_TRM(" 4. KD         : %d", PU16(ptr->kd));
    LOG_TRM(" 5. KD_SCALE   : %d", PU8(ptr->kd_scale));
    LOG_TRM(" 6. POS_LIMIT  : %d", PS16(ptr->pos_limit));
    LOG_TRM(" 7. NEG_LIMIT  : %d", PS16(ptr->neg_limit));
    
    LOG_TRM("Extra");
    LOG_TRM(" -. I_ACTION   : %d", PS32(ptr->i_action));
    LOG_TRM(" -. D_ACTION   : %d", PS32(ptr->d_action));
    
    CoreTerm_PrintAcknowledge();
    CoreLog_Flush();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdPidController_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)pid_ctrl, 0, SIZEOF(pid_ctrl));
    pid_ctrl_count = 0;
    
    CoreTerm_RegisterCommand("PidInfo", "Get info from PID controller a", 1, Command_PidInfo, TRUE);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
STD_PID_CTRL_HNDL StdPidController_Register(U16 kp,
                                            U8  kp_scale,
                                            U16 ki,
                                            U8  ki_scale,
                                            U16 kd,
                                            U8  kd_scale,
                                            S16 pos_limit,
                                            S16 neg_limit,
                                            STRING info,
                                            BOOL print_debug_info)
{
    MODULE_CHECK();
    
    PID_CTRL_STRUCT* ptr = &pid_ctrl[pid_ctrl_count];
    
    if(pid_ctrl_count >= PID_CTRL_COUNT)
    {
        LOG_ERR("Count too small");
        return INVALID_STD_PID_CTRL_HNDL;
    }
    
    if((kp_scale > 16) || (ki_scale > 16) || (kd_scale > 16))
    {
        LOG_ERR("Invalid parameter");
        return INVALID_STD_PID_CTRL_HNDL;
    }
    
    ptr->kp         = kp;
    ptr->kp_scale   = kp_scale;
    ptr->ki         = ki;
    ptr->ki_scale   = ki_scale;
    ptr->kd         = kd;
    ptr->kd_scale   = kd_scale;
    ptr->pos_limit  = pos_limit;
    ptr->neg_limit  = neg_limit;
    ptr->info       = info;
    ptr->print_debug_info = print_debug_info;
    
    ptr->i_action   = 0;
    ptr->d_action   = 0;
    
    pid_ctrl_count++;
    return (pid_ctrl_count - 1);
}
//------------------------------------------------------------------------------------------------//
S16 StdPidController_Calc(STD_PID_CTRL_HNDL hndl, S16 set_value, S16 meas_value)
{
	PID_CTRL_STRUCT* param_ptr;
	S32 delta_val;
	S32 temp_val;
	S32 p_action_term;
	S32 i_action_term;
	S32 d_action_term;
	S32 total_action;
    
    MODULE_CHECK();
    
	if(hndl >= pid_ctrl_count)
	{
		return 0;
    }
    
    param_ptr = &pid_ctrl[hndl];
    
	delta_val = ((S32)set_value - (S32)meas_value);                                             // [-2^16 ... 2^16]
    
	// P-action
	// --------
	// calculate P-action, the p_action_term is limited to [-2^31 ... 2^31]
    
	p_action_term = (delta_val * (S32)(param_ptr->kp)) >> param_ptr->kp_scale;                  // [-2^31 ... 2^31]
	if(p_action_term < 0)
	{
		p_action_term++;
	}
    
	// I-action
	// --------
	// calculate I-action, the param_ptr->i_action stores the integral of (Ki x E) ==> [-2^31 ... 2^31]
	// the i_action_term is limited to the (pos and neg limit) ==> [-2^15 ... 2^15]
    
	if (param_ptr->ki == 0)
	{
		i_action_term = param_ptr->i_action;
	}
	else
	{
		i_action_term = (delta_val * (S32)(param_ptr->ki));                                     // [-2^31 ... 2^31]
		if(i_action_term < 0)
		{
			i_action_term++;
		}
        
		if(SIGN(i_action_term) == SIGN(param_ptr->i_action))
		{
			// if signs are equal, there can be an overflow
			i_action_term += param_ptr->i_action;                                               // [-2^32 ... 2^32]
            
			if(SIGN(i_action_term) != SIGN(param_ptr->i_action))
			{
				if(SIGN(param_ptr->i_action) == 0)
				{
					i_action_term = MAX_SIGNED32;
				}
				else
				{
					i_action_term = MIN_SIGNED32;
				}
			}
		}                                                                                       // [-2^31 ... 2^31]
		else
		{
			i_action_term += param_ptr->i_action;                                               // [-2^31 ... 2^31]
		}
	}
    
	if(i_action_term > ((S32)param_ptr->pos_limit << param_ptr->ki_scale))
	{
		param_ptr->i_action = (S32)param_ptr->pos_limit << param_ptr->ki_scale;                 // [-2^31 ... 2^31]
		i_action_term = param_ptr->pos_limit;                                                   // [-2^15 ... 2^15]
	}
	else if(i_action_term < ((S32)param_ptr->neg_limit << param_ptr->ki_scale))
	{
		param_ptr->i_action = (S32)param_ptr->neg_limit << param_ptr->ki_scale;                 // [-2^31 ... 2^31]
		i_action_term = param_ptr->neg_limit;                                                   // [-2^15 ... 2^15]
	}
	else
	{
		param_ptr->i_action = i_action_term;                                                    // [-2^31 ... 2^31]
		i_action_term >>= param_ptr->ki_scale;                                                  // [-2^15 ... 2^15]
	}
    
	// D-action
	// --------
	// calculate d_action_term, the d_action_term is limited to [-2^31 ... 2^31]
	// param_ptr->i_action stores the previous delta_val [-2^16 ... 2^16]
    
	if(param_ptr->kd == 0)
	{
		d_action_term = 0;
	}
	else
	{
		d_action_term = delta_val - param_ptr->d_action;                                        // [-2^17 ... 2^17]
		temp_val = d_action_term * (S32)(param_ptr->kd);                                        // [-2^32 ... 2^32]

		if(SIGN(d_action_term) == SIGN(temp_val))
		{
			d_action_term = temp_val;                                                           // [-2^31 ... 2^31]
		}
		else
		{
			if(SIGN(d_action_term) == 0)
			{
				d_action_term = MAX_SIGNED32;
			}
			else
			{
				d_action_term = MIN_SIGNED32;
			}
		}                                                                                       // [-2^31 ... 2^31]
        
		d_action_term >>= param_ptr->kd_scale;                                                  // [-2^31 ... 2^31]
	}
	param_ptr->d_action = delta_val;                                                            // [-2^16 ... 2^16]
    
	// TOTAL
	// --------
	// calculate the sum of the P-, I- and D-action, taking into account possible overflows
    
    total_action = (p_action_term >> 16) + (d_action_term >> 16);
    
    if(param_ptr->print_debug_info)
    {
        LOG_DBG("PID hndl %d: p_term = %d, i_term = %d, d_term = %d", PU8(hndl),
                                                                      PS32(p_action_term),
                                                                      PS32(i_action_term),
                                                                      PS32(d_action_term));
    }
    
	if(total_action > 0)
	{
		return(param_ptr->pos_limit);
	}
	else if(total_action < -1)
	{
		return(param_ptr->neg_limit);
	}
	else
	{
		total_action = p_action_term + d_action_term + i_action_term;
		if(total_action > param_ptr->pos_limit)
		{
			return(param_ptr->pos_limit);
		}
		else if(total_action < param_ptr->neg_limit)
		{
			return(param_ptr->neg_limit);
		}
		else
		{
			return((S16)total_action);
		}
	}
}
//------------------------------------------------------------------------------------------------//
BOOL StdPidController_SetParam(STD_PID_CTRL_HNDL hndl, STD_PID_PARAM param, S32 value)
{
    MODULE_CHECK();
    
    PID_CTRL_STRUCT* ptr;
    
	if(hndl >= pid_ctrl_count)
	{
		return FALSE;
    }
    
    ptr = &pid_ctrl[hndl];
    
    switch(param)
    {
    case PID_PARAM_KP:
        ptr->kp = (U16)value;
        break;
        
    case PID_PARAM_KP_SCALE:
        if((U8)value > 16)
        {
            return FALSE;
        }
        ptr->kp_scale = (U8)value;
        break;
        
    case PID_PARAM_KI:
        ptr->ki = (U16)value;
        break;
        
    case PID_PARAM_KI_SCALE:
        if((U8)value > 16)
        {
            return FALSE;
        }
        ptr->i_action >>= ptr->ki_scale;
        ptr->ki_scale = (U8)value;
        ptr->i_action <<= ptr->ki_scale;
        break;
        
    case PID_PARAM_KD:
        ptr->kd = (U16)value;
        break;
        
    case PID_PARAM_KD_SCALE:
        if((U8)value > 16)
        {
            return FALSE;
        }
        ptr->kd_scale = (U8)value;
        break;
        
    case PID_PARAM_POS_LIMIT:
        ptr->pos_limit = (S16)value;
        break;
        
    case PID_PARAM_NEG_LIMIT:
        ptr->neg_limit = (S16)value;
        break;
        
    case PID_PARAM_I_TERM:
        if((S16)value > ptr->pos_limit)
        {
            ptr->i_action = (S32)(ptr->pos_limit) << ptr->ki_scale;
        }
        else if((S16)value < ptr->neg_limit)
        {
            ptr->i_action = (S32)(ptr->neg_limit) << ptr->ki_scale;
        }
        else
        {
            ptr->i_action = (S32)((S16)value) << ptr->ki_scale;
        }
        break;
        
    case PID_PARAM_D_TERM:
        ptr->d_action = value;
        break;
        
    default:
        return FALSE;
    }
    return TRUE;
}
//================================================================================================//
