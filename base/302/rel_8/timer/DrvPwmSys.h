//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the system PWM driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef TIMER_DRVPWMSYS_H
#define TIMER_DRVPWMSYS_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//SYS include section
#include "timer\SysPwm.h"

//DRV include section
#include "timer\DrvPwm.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DrvPwmSys_Timer_Init(x,y,z)         SysPwm_Timer_Init(x,y,z)
#define DrvPwmSys_Timer_SetState(x,y)       SysPwm_Timer_SetState(x,y)
#define DrvPwmSys_Timer_Enable(x)           SysPwm_Timer_Enable(x)
#define DrvPwmSys_Timer_Disable(x)          SysPwm_Timer_Disable(x)
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
void DrvPwmSys_Init(void);

PWM_CHANNEL_HNDL DrvPwmSys_Register(TIMER timer, PWM_CHANNEL pwm_channel, PWM_POLARITY pwm_polarity);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* TIMER_DRVPWMSYS_H */
