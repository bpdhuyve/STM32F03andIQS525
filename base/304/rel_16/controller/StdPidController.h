//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of PID controller (based on platform 1 version)
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STDPIDCONTROLLER_H
#define STDPIDCONTROLLER_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_STD_PID_CTRL_HNDL               255
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    PID_PARAM_KP            = 0,
    PID_PARAM_KP_SCALE      = 1,
    PID_PARAM_KI            = 2,
    PID_PARAM_KI_SCALE      = 3,
    PID_PARAM_KD            = 4,
    PID_PARAM_KD_SCALE      = 5,
    PID_PARAM_POS_LIMIT     = 6,
    PID_PARAM_NEG_LIMIT     = 7,
    PID_PARAM_I_TERM        = 8,
    PID_PARAM_D_TERM        = 9
}
STD_PID_PARAM;

typedef U8      STD_PID_CTRL_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void StdPidController_Init(void);

// @remark  Register a new PID controller
STD_PID_CTRL_HNDL StdPidController_Register(U16 kp,
                                            U8  kp_scale,
                                            U16 ki,
                                            U8  ki_scale,
                                            U16 kd,
                                            U8  kd_scale,
                                            S16 pos_limit,
                                            S16 neg_limit,
                                            STRING info,
                                            BOOL print_debug_info);

// @remark  This function calculates an output with a setvalue and a measured value as input.
//          The implementation is of the next form:
//          output = Kp x E(n) + lim(sum(Ki x E(i)) >> 16) + Kd x (E(n) - E(n-1))
//          K-values are calculated as follows: K = Kx / 2^Kx_scale
//          An anti-windup is implemented by limiting the integrator part
S16 StdPidController_Calc(STD_PID_CTRL_HNDL hndl, S16 set_value, S16 meas_value);

// @remark  Set a STD_PID_PARAM of a certain hndl
BOOL StdPidController_SetParam(STD_PID_CTRL_HNDL hndl, STD_PID_PARAM param, S32 value);
//================================================================================================//



#endif /* STDPIDCONTROLLER_H */
