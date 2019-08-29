//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the system PWM driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TIMER_DRVPWMSYS_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TIMER_DRVPWMSYS_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               TIMER_DRVPWMSYS_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef DRVPWMSYS_COUNT
	#define DRVPWMSYS_COUNT			    5
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "timer\DrvPwmSys.h"
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
BOOL DrvPwmSys_SetDutyCycle(PWM_CHANNEL_ID channel_id, U16 duty_cycle);
BOOL DrvPwmSys_SetState(PWM_CHANNEL_ID channel_id, BOOL enable);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static PWM_CHANNEL_HOOK_LIST                pwm_channel_hook_list;
static PWM_CHANNEL_STRUCT                   pwm_channel_struct[DRVPWMSYS_COUNT];
static U8                                   pwm_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
BOOL DrvPwmSys_SetDutyCycle(PWM_CHANNEL_ID channel_id, U16 duty_cycle)
{
    return SysPwm_Channel_SetDutyCycle((TIMER)(channel_id >> 4), (PWM_CHANNEL)(channel_id & 0x0F), duty_cycle);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvPwmSys_SetState(PWM_CHANNEL_ID channel_id, BOOL enable)
{
    return SysPwm_Channel_SetState((TIMER)(channel_id >> 4), (PWM_CHANNEL)(channel_id & 0x0F), enable);
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvPwmSys_Init(void)
{
    LOG_DEV("DrvPwmSys_Init");
    SysPwm_Init();
    
    pwm_channel_hook_list.set_dutycycle_hook = DrvPwmSys_SetDutyCycle;
    pwm_channel_hook_list.set_state_hook = DrvPwmSys_SetState;
    
    MEMSET((VPTR)pwm_channel_struct, 0, SIZEOF(pwm_channel_struct));
    pwm_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
PWM_CHANNEL_HNDL DrvPwmSys_Register(TIMER timer, PWM_CHANNEL pwm_channel, PWM_POLARITY pwm_polarity)
{
    PWM_CHANNEL_HNDL    channel_hndl = &pwm_channel_struct[pwm_channel_count];
    
    LOG_DEV("DrvPwmSys_Register");
    if(pwm_channel_count < DRVPWMSYS_COUNT)
    {
        if(SysPwm_Channel_Init(timer, pwm_channel, pwm_polarity))
        {
            channel_hndl->hook_list_ptr = &pwm_channel_hook_list;
            channel_hndl->channel_id = ((U8)timer << 4) | (U8)pwm_channel;
            pwm_channel_count++;
            return channel_hndl;
        }
        LOG_WRN("PWM init failed");
        return NULL;
    }
    LOG_ERR("PWM count overrun - %d : %d", PU8(timer), PU8(pwm_channel));
    return NULL;
}
//================================================================================================//
