//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation for PWM
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TIMER__SYSPWM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TIMER__SYSPWM_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               TIMER__SYSPWM_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS lib include section
#include "timer\SysPwm.h"
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
void SysPwm_Init(void)
{
    // nothing to do
}
//------------------------------------------------------------------------------------------------//
BOOL SysPwm_Timer_Init(TIMER timer, U32 pwm_base_frequency, PWM_MODE pwm_mode)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    U32             tim_clk;
    
    if(timer >= TIMER_COUNT)
    {
        LOG_ERR("Illegal timer %d", PU8(timer));
        return FALSE;
    }

    // enable the clock & get clock speed
    if(timer_clock_defs[timer].apb == APB2)
    {
        RCC_APB2PeriphClockCmd(0x00000001 << timer_clock_defs[timer].bit, ENABLE);
        tim_clk = SysGetTimClk();
    }
    else
    {
        RCC_APB1PeriphClockCmd(0x00000001 << timer_clock_defs[timer].bit, ENABLE);
        tim_clk = SysGetTimClk();
    }
    
    timer_reg_ptr->CR1 = 0x0000; //disable the timer
    timer_reg_ptr->CR2 = 0x0000;
    timer_reg_ptr->SMCR = 0x0000;
    
    tim_clk /= pwm_base_frequency;
    
    LOG_DBG("timer %d - clock : %d", PU8(timer), PU32(tim_clk));
    
    timer_reg_ptr->PSC = (U16)(tim_clk >> 16);
    
    if(pwm_mode == PWM_MODE_CENTER_ALIGN)
    {
        timer_reg_ptr->CR1 = 0x00E0; //0000.0000.1010.0000
        timer_reg_ptr->ARR = (U16)(tim_clk / (timer_reg_ptr->PSC + 1)) >> 1;
    }
    else
    {
        timer_reg_ptr->CR1 = 0x0080; //0000.0000.1000.0000
        timer_reg_ptr->ARR = (U16)(tim_clk / (timer_reg_ptr->PSC + 1)) - 1;
    }
    
    LOG_DEV("CR1 : %04h - ARR : %d", PU16(timer_reg_ptr->CR1), PU16(timer_reg_ptr->ARR));
    
    timer_reg_ptr->CCR1 = 0x0000;
    timer_reg_ptr->CCR2 = 0x0000;
    timer_reg_ptr->CCR3 = 0x0000;
    timer_reg_ptr->CCR4 = 0x0000;
    
    timer_reg_ptr->EGR = 0x0001; // set UG bit to initialize all registers
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysPwm_Timer_SetState(TIMER timer, BOOL enable)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    if(enable)
    {
        timer_reg_ptr->CR1 |= 0x0001;
    }
    else
    {
        timer_reg_ptr->CR1 &= ~0x0001;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysPwm_Channel_Init(TIMER timer, PWM_CHANNEL pwm_channel, PWM_POLARITY pwm_polarity)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    
    if(timer >= TIMER_COUNT)
    {
        LOG_ERR("Illegal timer %d", PU8(timer));
        return FALSE;
    }

    if(pwm_polarity == PWM_POLARITY_ACTIVE_HIGH)
    {
        if(pwm_channel >= PWM_CHANNEL_1N)
        {
            timer_reg_ptr->CCER &= ~(0x0008 << (4 * (pwm_channel - PWM_CHANNEL_1N)));
        }
        else
        {
            timer_reg_ptr->CCER &= ~(0x0002 << (4 * pwm_channel));
        }
    }
    else
    {
        if(pwm_channel >= PWM_CHANNEL_1N)
        {
            timer_reg_ptr->CCER |= (0x0008 << (4 * (pwm_channel - PWM_CHANNEL_1N)));
        }
        else
        {
            timer_reg_ptr->CCER |= (0x0002 << (4 * pwm_channel));
        }
    }
    
    if(pwm_channel >= PWM_CHANNEL_1N)
    {
        pwm_channel -= PWM_CHANNEL_1N;
    }
    
    if(pwm_channel >= PWM_CHANNEL_3)
    {
        timer_reg_ptr->CCMR2 |= (0x0068 << (8 * (pwm_channel - PWM_CHANNEL_3)));
    }
    else
    {
        timer_reg_ptr->CCMR1 |= (0x0068 << (8 * pwm_channel));
    }
    
    return SysPwm_Channel_SetDutyCycle(timer, pwm_channel, 0);
}
//------------------------------------------------------------------------------------------------//
BOOL SysPwm_Channel_SetDutyCycle(TIMER timer, PWM_CHANNEL pwm_channel, U16 duty_cycle)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    U16             value;
    
    if(timer >= TIMER_COUNT)
    {
        LOG_ERR("Illegal timer %d", PU8(timer));
        return FALSE;
    }

    if(pwm_channel >= PWM_CHANNEL_1N)
    {
        pwm_channel -= PWM_CHANNEL_1N;
    }
    
    if(duty_cycle == 0xFFFF)
    {
        value = timer_reg_ptr->ARR + 1;
    }
    else
    {
        value = (U16)(((U32)(timer_reg_ptr->ARR + 1) * (U32)duty_cycle) >> 16);
    }
    
    LOG_DBG("timer %d - chan %d - duty cycle :%d", PU8(timer), PU8(pwm_channel), PU16(value));
    
    switch(pwm_channel)
    {
    case PWM_CHANNEL_1:
        timer_reg_ptr->CCR1 = value;
        break;
    case PWM_CHANNEL_2:
        timer_reg_ptr->CCR2 = value;
        break;
    case PWM_CHANNEL_3:
        timer_reg_ptr->CCR3 = value;
        break;
    case PWM_CHANNEL_4:
        timer_reg_ptr->CCR4 = value;
        break;
    }
    
    LOG_DEV("CCR : %d %d %d %d", PU16(timer_reg_ptr->CCR1), PU16(timer_reg_ptr->CCR2), PU16(timer_reg_ptr->CCR3), PU16(timer_reg_ptr->CCR4));
    LOG_DEV("CCER : %04h", PU16(timer_reg_ptr->CCER));
    LOG_DEV("CCMR : %04h %04h", PU16(timer_reg_ptr->CCMR1), PU16(timer_reg_ptr->CCMR2));
    LOG_DEV("CR1 : %04h", PU16(timer_reg_ptr->CR1));
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysPwm_Channel_SetState(TIMER timer, PWM_CHANNEL pwm_channel, BOOL enable)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    
    if(timer >= TIMER_COUNT)
    {
        LOG_ERR("Illegal timer %d", PU8(timer));
        return FALSE;
    }

    if(enable)
    {
        if(pwm_channel >= PWM_CHANNEL_1N)
        {
            timer_reg_ptr->CCER |= 0x0004 << (4 * (pwm_channel - PWM_CHANNEL_1N));
        }
        else
        {
            timer_reg_ptr->CCER |= 0x0001 << (4 * pwm_channel);
        }
        
        LOG_DEV("CCER : %04h", PU16(timer_reg_ptr->CCER));
        
        timer_reg_ptr->BDTR |= 0x8000;  //main output enable on advanced timers
    }
    else
    {
        if(pwm_channel >= PWM_CHANNEL_1N)
        {
            timer_reg_ptr->CCER &= ~(0x0004 << (4 * (pwm_channel - PWM_CHANNEL_1N)));
        }
        else
        {
            timer_reg_ptr->CCER &= ~(0x0001 << (4 * pwm_channel));
        }
    }
    return TRUE;
}
//================================================================================================//
