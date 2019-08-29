//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the PWM system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef TIMER__ISYSPWM_H
#define TIMER__ISYSPWM_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "timer\ISysTimer.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SysPwm_Timer_Enable(x)              SysPwm_Timer_SetState(x,TRUE)
#define SysPwm_Timer_Disable(x)             SysPwm_Timer_SetState(x,FALSE)

#define SysPwm_Channel_Enable(x,y)          SysPwm_Channel_SetState(x,y,TRUE)
#define SysPwm_Channel_Disable(x,y)         SysPwm_Channel_SetState(x,y,FALSE)
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    PWM_MODE_CENTER_ALIGN,
    PWM_MODE_EDGE_ALIGN,
}
PWM_MODE;

typedef enum
{
    PWM_POLARITY_ACTIVE_HIGH,
    PWM_POLARITY_ACTIVE_LOW
}
PWM_POLARITY;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark none
void SysPwm_Init(void);

// @remark none
BOOL SysPwm_Timer_Init(TIMER timer, U32 pwm_base_frequency, PWM_MODE pwm_mode);

// @remark none
BOOL SysPwm_Timer_SetState(TIMER timer, BOOL enable);

// @remark none
BOOL SysPwm_Channel_Init(TIMER timer, PWM_CHANNEL pwm_channel, PWM_POLARITY pwm_polarity);

// @remark dutycyclle 0 = 0%; 0xFFFF = 100%
BOOL SysPwm_Channel_SetDutyCycle(TIMER timer, PWM_CHANNEL pwm_channel, U16 duty_cycle);

// @remark none
BOOL SysPwm_Channel_SetState(TIMER timer, PWM_CHANNEL pwm_channel, BOOL enable);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



#endif /* TIMER__ISYSPWM_H */
