//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the SysTick on Timer 6
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORE__SYSTICK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORE__SYSTICK_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               CORE__SYSTICK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SYSTIMER
	#define SYSTIMER                    14
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "core\SysTick.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#if (SYSTIMER == 1)
    #define TIM_REG                     TIM1
    #define ENABLE_TIMER_CLOCK          RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE)
    #define ISR_REGISTER                TIM1_BRK_UP_TRG_COM_IRQn
#elif (SYSTIMER == 2)
    #define TIM_REG                     TIM2
    #define ENABLE_TIMER_CLOCK          RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)
    #define ISR_REGISTER                TIM2_IRQn
#elif (SYSTIMER == 3)
    #define TIM_REG                     TIM3
    #define ENABLE_TIMER_CLOCK          RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE)
    #define ISR_REGISTER                TIM3_IRQn
#elif (SYSTIMER == 6)
    #define TIM_REG                     TIM6
    #define ENABLE_TIMER_CLOCK          RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE)
    #define ISR_REGISTER                TIM6_DAC_IRQn
#elif (SYSTIMER == 14)
    #define TIM_REG                     TIM14
    #define ENABLE_TIMER_CLOCK          RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE)
    #define ISR_REGISTER                TIM14_IRQn
#elif (SYSTIMER == 15)
    #define TIM_REG                     TIM15
    #define ENABLE_TIMER_CLOCK          RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE)
    #define ISR_REGISTER                TIM15_IRQn
#elif (SYSTIMER == 16)
    #define TIM_REG                     TIM16
    #define ENABLE_TIMER_CLOCK          RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE)
    #define ISR_REGISTER                TIM16_IRQn
#elif (SYSTIMER == 17)
    #define TIM_REG                     TIM17
    #define ENABLE_TIMER_CLOCK          RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE)
    #define ISR_REGISTER                TIM17_IRQn
#else
    #error "Undefined SYSTIMER"
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static __irq void SysTick_IntUpdateIsrTimer(void);
//================================================================================================//


//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static EVENT_CALLBACK       systick_hook = NULL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static __irq void SysTick_IntUpdateIsrTimer(void)
{
    if(TIM_REG->SR & 0x0001)
    {
        if(systick_hook != NULL)
        {
            systick_hook();
        }
        TIM_REG->SR &= ~0x0001;  //clear interrupt flag
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysTick_Init(U32 tick_period_in_us, EVENT_CALLBACK tick_hook)
{
    U32     tim_clk;

    // register tick hook
    systick_hook = tick_hook;

    // enable clock
    ENABLE_TIMER_CLOCK;
    tim_clk = SysGetTimClk();

    // set prescaler
    TIM_REG->PSC = (tim_clk / 1000000) - 1;   // set to 1Mhz

    // set control registers
    TIM_REG->CR1 = 0x0084;
    TIM_REG->CR2 = 0x0000;
    TIM_REG->SMCR = 0x0000;
    TIM_REG->DIER = 0x0000;
    TIM_REG->ARR = tick_period_in_us;
    TIM_REG->EGR |= 0x0001; //re-init the timer with all configured values

    // set interrupt
    SysInt_RegisterIsr(ISR_REGISTER, SysTick_IntUpdateIsrTimer);
    SysInt_EnableIsr(ISR_REGISTER);
}
//------------------------------------------------------------------------------------------------//
BOOL SysTick_SetPeriod(U32 tick_period_in_us)
{
    TIM_REG->ARR = tick_period_in_us;
    TIM_REG->EGR |= 0x0001; //re-init the timer with all configured values
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTick_Start(void)
{
    TIM_REG->CR1 |= 0x0001;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTick_Stop(void)
{
    TIM_REG->CR1 &= ~0x0001;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTick_Restart(void)
{
    TIM_REG->EGR |= 0x0001; //re-init the timer
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTick_DisableInterrupt(void)
{
    TIM_REG->DIER &= ~0x0001;  //interrupt disable
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTick_EnableInterrupt(void)
{
    TIM_REG->DIER |= 0x0001;  //interrupt enable
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
U32 SysTick_GetTickCount(void)
{
    return (U32)TIM_REG->CNT;
}
//================================================================================================//
