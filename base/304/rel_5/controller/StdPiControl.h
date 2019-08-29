//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the exported functions of the PI control std lib module
// ported from platform 1 (StdPiControl2)
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CONTROLLER__STDPICONTROL_H
#define CONTROLLER__STDPICONTROL_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_STD_PI_CTRL_HNDL      0x00FF
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8      STD_PI_CTRL_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/**
 * @brief   Initialiser for the PI control module
 *
 * Initialises the PI control module and registers this entity to the Module Manager.\n
 */
void StdPiControlInit(void);

/**
 * @brief   Registration function for a PI controller
 *
 * Function to register a PI controller. The 6 kp, ki and antiwindup params are saved for each PI controller.
 * The returned handle identifies the registered PI controller. \n
 *
 * @param   kp        : the kp value for the PI controller
 * @param   kp_scale  : the number of times the kp will be shifted right (=division by 2^kp_scale)
 * @param   ki        : the ki value for the PI controller
 * @param   ki_scale  : the number of times the ki will be shifted right (=division by 2^ki_scale)
 * @param   pos_limit : the upper anti windup limit
 * @param   neg_limit : the lower anti windup limit
 *
 * @return  HANDLE to the registered PI controller
 */
STD_PI_CTRL_HNDL StdPiRegisterController(S16 kp,
                                         S16 kp_scale,
                                         S16 ki,
                                         S16 ki_scale,
                                         S16 pos_limit,
                                         S16 neg_limit);

/**
 * @brief   The PI controller function
 *
 * This function calculates an output with a setvalue and a measured value as input. The implementation is of
 * the normal (non recursive) form: i.e.    \n
 * output = Kp x E(n) + Ki x [E(n) + E(n-1) + ... + E(0)] \n
 * First the output is calculated without the new I contribution i.e.: \n
 * output = Kp x E(n) + Ki x [E(n-1) + E(n-2) + ... + E(0)] \n
 * If this output falls beyond the anti windup limits then the new I contribution is added and this new sum
 * is limited to the anti windup limits. Else the output is limited directly to this limits and the term E(n)
 * is not added to a i_action variable, i.e. i_action stays: [E(n-1) + E(n-2) + ... + E(0)].
 *
 * @param   hndl     : handle to the specific PI controller
 * @param   set_val  : set value input for the PI controller
 * @param   meas_val : measured value input for the PI controller
 *
 * @return  the PI controller output (-32768 .... +32767)
 */
S16 StdPiController(STD_PI_CTRL_HNDL hndl, S16 set_val, S16 meas_val);

/**
 * @brief   Method to change the Kp constant of the PI controller
 *
 * Method to change the Kp constant of the PI controller .\n
 *
 * @param   hndl      : handle to the specific PI controller
 * @param   kp        : the kp value for the PI controller
 * @param   kp_scale  : the number of times the kp will be shifted right (=division by 2^kp_scale)
 *
 * @remark	Don't use this function in background, when PI controller interrupt is enabled.
 */
void StdPiControlChangeKpParams(STD_PI_CTRL_HNDL hndl, S16 kp, S16 kp_scale);

/**
 * @brief   Method to change the Ki constant of the PI controller
 *
 * Method to change the Ki constant of the PI controller .\n
 *
 * @param   hndl      : handle to the specific PI controller
 * @param   ki        : the ki value for the PI controller
 * @param   ki_scale  : the number of times the ki will be shifted right (=division by 2^ki_scale)
 *
 * @remark	Don't use this function in background, when PI controller interrupt is enabled.
 */
void StdPiControlChangeKiParams(STD_PI_CTRL_HNDL hndl, S16 ki, S16 ki_scale);

/**
 * @brief   Method to change the Kp and Ki mantisses of the PI controller
 *
 * Method to change the Kp and Ki mantisses of the PI controller.\n
 *
 * @param   hndl :	handle to the specific PI controller
 * @param   kp :	the new kp value for the PI controller
 * @param   ki :	the new ki value for the PI controller
 *
 * @remark	The scale of Kp and Ki remains unaffected by this function
 * @remark	Don't use this function in background, when PI controller interrupt is enabled.
 */
void StdPiControlSetKpAndKi(STD_PI_CTRL_HNDL hndl, S16 kp, S16 ki);

/**
 * @brief   Method to change the Kp mantisse of the PI controller
 *
 * @param   hndl :	handle to the specific PI controller
 * @param   kp :	the new kp value for the PI controller
 *
 * @remark	The scale of Kp remains unaffected by this function
 */
void StdPiControlSetKp(STD_PI_CTRL_HNDL hndl, S16 kp);

/**
 * @brief   Method to change the Ki mantisse of the PI controller
 *
 * @param   hndl :	handle to the specific PI controller
 * @param   ki :	the new ki value for the PI controller
 *
 * @remark	The scale of Ki remains unaffected by this function
 * @remark	Don't use this function in background, when PI controller interrupt is enabled.
 */
void StdPiControlSetKi(STD_PI_CTRL_HNDL hndl, S16 ki);

/**
 * @brief   Method to change the anti windup params of the PI controller
 *
 * Method to change the anti windup params of the PI controller .\n
 *
 * @param   hndl      : handle to the specific PI controller
 * @param   pos_limit : the upper anti windup limit
 * @param   neg_limit : the lower anti windup limit
 *
 * @remark	Don't use this function in background, when PI controller interrupt is enabled.
 */
void StdPiControlChangeAwParams(STD_PI_CTRL_HNDL hndl, S16 pos_limit, S16 neg_limit);

/**
 * @brief   Method to reset the I action of a registered PI controller
 *
 * Method to reset the I action of a registered PI controller.\n
 *
 * @param   hndl      : handle to the specific PI controller
 */
void StdPiControlResetIaction(STD_PI_CTRL_HNDL hndl);

/**
 * @brief   Method to initialize the I action of a registered PI controller with a certain value
 *
 * @param   hndl    : handle to the specific PI controller
 * @param	value	: the init value
 */
void StdPiControlInitIaction(STD_PI_CTRL_HNDL hndl, S32 value);

/**
 * @brief   Method to get the I action of a registered PI controller
 *
 * Method to get the I action of a registered PI controller.\n
 *
 * @param   hndl      : handle to the specific PI controller
 *
 * @return	 the PI controller I action
 */
S32 StdPiControlGetIaction(STD_PI_CTRL_HNDL hndl);

/**
 * @brief   Method to get the Kp of a registered PI controller
 *
 * Method to get the Kp of a registered PI controller.\n
 *
 * @param   hndl      : handle to the specific PI controller
 *
 * @return	 the PI controller Kp
 */
S16 StdPiControlGetKp(STD_PI_CTRL_HNDL hndl);

/**
 * @brief   Method to get the Kp_scale of a registered PI controller
 *
 * Method to get the Kp_scale of a registered PI controller.\n
 *
 * @param   hndl      : handle to the specific PI controller
 *
 * @return	 the PI controller Kp_scale
 */
S16 StdPiControlGetKpScale(STD_PI_CTRL_HNDL hndl);

/**
 * @brief   Method to get the Ki of a registered PI controller
 *
 * Method to get the Ki of a registered PI controller.\n
 *
 * @param   hndl      : handle to the specific PI controller
 *
 * @return	 the PI controller Ki
 */
S16 StdPiControlGetKi(STD_PI_CTRL_HNDL hndl);

/**
 * @brief   Method to get the Ki_scale of a registered PI controller
 *
 * Method to get the Ki_scale of a registered PI controller.\n
 *
 * @param   hndl      : handle to the specific PI controller
 *
 * @return	 the PI controller Ki_scale
 */
S16 StdPiControlGetKiScale(STD_PI_CTRL_HNDL hndl);
//================================================================================================//



#endif /* CONTROLLER__PI__STDPICONTROL_H */
