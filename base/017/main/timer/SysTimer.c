//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation for timers
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TIMER__SYSTIMER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TIMER__SYSTIMER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               TIMER__SYSTIMER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef EVENT_CALLBACK_COUNT
	#define EVENT_CALLBACK_COUNT        10
#elif EVENT_CALLBACK_COUNT > 40
    #undef EVENT_CALLBACK_COUNT
    #define EVENT_CALLBACK_COUNT        40
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS lib include section
#include "timer\SysTimer.h"

#include "core\stm32f0xx_rcc.h"
#include "core\stm32f0xx_tim.h"
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
static __irq void SysTimerIntUpdateIsrTimer1(void);
static __irq void SysTimerIntCompareIsrTimer1(void);
static __irq void SysTimerIntUpdateIsrTimer2(void);
static __irq void SysTimerIntUpdateIsrTimer3(void);
static void SysTimerGlobalTimerInterrupt(TIMER timer);
static void SysTimerCompareTimerInterrupt(TIMER timer);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static U8                       periodic_interrupt_callback[TIMER_COUNT];
static U8                       compare_interrupt_callback[TIMER_COUNT][4];
static VPTR                     event_callbacks[EVENT_CALLBACK_COUNT];
static U8                       event_callback_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
const TIMER_REG_HNDL            timer_registers[TIMER_COUNT] = {(TIMER_REG_HNDL) TIM1_BASE,
                                                                (TIMER_REG_HNDL) TIM2_BASE,
                                                                (TIMER_REG_HNDL) TIM3_BASE,
                                                                (TIMER_REG_HNDL) TIM6_BASE,
                                                                (TIMER_REG_HNDL) TIM7_BASE,
                                                                (TIMER_REG_HNDL) TIM14_BASE,
                                                                (TIMER_REG_HNDL) TIM15_BASE,
                                                                (TIMER_REG_HNDL) TIM16_BASE,
                                                                (TIMER_REG_HNDL) TIM17_BASE};

const CLOCK_DEF                 timer_clock_defs[TIMER_COUNT] = {APB2, 11, //RCC_APB2Periph_TIM1,
                                                                 APB1,  0, //RCC_APB1Periph_TIM2,
                                                                 APB1,  1, //RCC_APB1Periph_TIM3,
                                                                 APB1,  4, //RCC_APB1Periph_TIM6,
                                                                 APB1,  5, //RCC_APB1Periph_TIM7,
                                                                 APB1,  8, //RCC_APB1Periph_TIM14,
                                                                 APB2, 16, //RCC_APB2Periph_TIM15,
                                                                 APB2, 17, //RCC_APB2Periph_TIM16,
                                                                 APB2, 18}; //RCC_APB2Periph_TIM17};
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer1(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_1);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntCompareIsrTimer1(void)
{
    SysTimerCompareTimerInterrupt(TIMER_1);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer2(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_2);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer3(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_3);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer6(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_6);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer7(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_7);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer14(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_14);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer15(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_15);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer16(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_16);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysTimerIntUpdateIsrTimer17(void)
{
    SysTimerGlobalTimerInterrupt(TIMER_17);
}
//------------------------------------------------------------------------------------------------//
// @brief global interrupt function
static void SysTimerGlobalTimerInterrupt(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    U8      period_callback_index = periodic_interrupt_callback[timer];

    if(timer_reg_ptr->SR & 0x0001) //update interrupt flag
    {
        if(period_callback_index != 0xFF)
        {
            ((TIMER_PERIODIC_ISR)event_callbacks[period_callback_index])(timer); //call callback
        }
        timer_reg_ptr->SR &= ~0x0001;  //clear interrupt flag
    }
    
    if(timer >= 1) //general purpose and basic timers
    {
        //check the 4 compares
        SysTimerCompareTimerInterrupt(timer);
    }
}
//------------------------------------------------------------------------------------------------//
// @brief compare interrupt function -- only for advanced timers
static void SysTimerCompareTimerInterrupt(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    U8*     compare_callback_index = compare_interrupt_callback[timer];
    U8      i;

    //check the 4 compares
    for(i = 0; i < SysTimer_Timer_GetNumberOfCompares(timer); i++)
    {
        if((timer_reg_ptr->SR & (0x0002 << i)) && (timer_reg_ptr->DIER & (0x0002 << i))) //compare i
        {
            if(compare_callback_index[i] != 0xFF)
            {
                ((TIMER_COMPARE_ISR)event_callbacks[compare_callback_index[i]])(timer, (COMPARE_NUMBER)i);
            }
            timer_reg_ptr->SR &= ~(0x0002 << i);  //clear interrupt flag
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysTimer_Init(void)
{
    MEMSET((VPTR)compare_interrupt_callback, 0xFF, SIZEOF(compare_interrupt_callback));
    MEMSET((VPTR)periodic_interrupt_callback, 0xFF, SIZEOF(periodic_interrupt_callback));
    MEMSET((VPTR)event_callbacks, 0, SIZEOF(event_callbacks));
    event_callback_count = 0;
}
//------------------------------------------------------------------------------------------------//
U32 SysTimer_Timer_Init(TIMER timer, U32 source_clock_frequency)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    
    if(timer > TIMER_17)
    {
        LOG_ERR("Illegal timer %d", PU8(timer));
    }

    // enable the clock & get clock speed
    if(timer_clock_defs[timer].apb == APB2)
    {
        RCC_APB2PeriphClockCmd(0x00000001 << timer_clock_defs[timer].bit, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(0x00000001 << timer_clock_defs[timer].bit, ENABLE);
    }
    
    RCC_ClocksTypeDef clocks ;
    
    RCC_GetClocksFreq(&clocks) ;
    
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure ;
    
    TIM_TimeBaseStructure.TIM_Period = UINT16_MAX ;
    TIM_TimeBaseStructure.TIM_Prescaler = (clocks.PCLK_Frequency / source_clock_frequency) - 1 ;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ;

    switch(timer)
    {
    case TIMER_1:
        TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM1_BRK_UP_TRG_COM_IRQn, SysTimerIntUpdateIsrTimer1);
        SysInt_RegisterIsr(TIM1_CC_IRQn, SysTimerIntCompareIsrTimer1);
        break;
        
    case TIMER_2:
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM2_IRQn, SysTimerIntUpdateIsrTimer2);
        break;

    case TIMER_3:
        TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM3_IRQn, SysTimerIntUpdateIsrTimer3);
        break;

    case TIMER_6:
        TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM6_DAC_IRQn, SysTimerIntUpdateIsrTimer6);
        break;

    case TIMER_7:
        TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM7_IRQn, SysTimerIntUpdateIsrTimer7);
        break;

    case TIMER_14:
        TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM14_IRQn, SysTimerIntUpdateIsrTimer14);
        break;

    case TIMER_15:
        TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM15_IRQn, SysTimerIntUpdateIsrTimer15);
        break;

    case TIMER_16:
        TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM16_IRQn, SysTimerIntUpdateIsrTimer16);
        break;

    case TIMER_17:
        TIM_TimeBaseInit(TIM17, &TIM_TimeBaseStructure);
        SysInt_RegisterIsr(TIM17_IRQn, SysTimerIntUpdateIsrTimer17);
        break;
    }
    
    // return real clock frequency
    return (U32)(clocks.PCLK_Frequency / (timer_reg_ptr->PSC + 1));
}
//------------------------------------------------------------------------------------------------//
U8 SysTimer_Timer_GetTimerWidth(TIMER timer)
{
    if(timer == TIMER_2)
    {
        return 32;
    }
    return 16;
}
//------------------------------------------------------------------------------------------------//
U8 SysTimer_Timer_GetNumberOfCompares(TIMER timer)
{
    if(timer > 2)
    {
        return 1;
    }
    else
    {
        return 4;
    }
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_Start(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    timer_reg_ptr->CR1 |= 0x0001;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_Stop(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    timer_reg_ptr->CR1 &= ~0x0001;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_SetPeriod(TIMER timer, U32 period_count)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    if((period_count > 0x0000FFFF) && (timer != TIMER_2))
    {
        //EXCEPTION
        return FALSE;
    }
    timer_reg_ptr->ARR = (U16)period_count;
    timer_reg_ptr->EGR |= 0x0001;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_SetCompare(TIMER timer, COMPARE_NUMBER compare_number, U32 compare_count)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    
    if((compare_number >= SysTimer_Timer_GetNumberOfCompares(timer)) || ((compare_count > 0x0000FFFF) && (timer != TIMER_2)))
    {
        //EXCEPTION
        return FALSE;
    }
    
    switch(compare_number)
    {
    case COMPARE_1:
        timer_reg_ptr->CCR1 = (U16)compare_count;
        break;

    case COMPARE_2:
        timer_reg_ptr->CCR2 = (U16)compare_count;
        break;

    case COMPARE_3:
        timer_reg_ptr->CCR3 = (U16)compare_count;
        break;

    case COMPARE_4:
        timer_reg_ptr->CCR4 = (U16)compare_count;
        break;

    default:
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_RegisterCompareInterrupt(TIMER timer, COMPARE_NUMBER compare_number, TIMER_COMPARE_ISR compare_callback)
{
    if(compare_number >= SysTimer_Timer_GetNumberOfCompares(timer))
    {
        //EXCEPTION
        return FALSE;
    }
    if(event_callback_count < EVENT_CALLBACK_COUNT)
    {
        event_callbacks[event_callback_count] = (VPTR)compare_callback;
        compare_interrupt_callback[timer][compare_number] = event_callback_count;
        SysTimer_Timer_DisableCompareInterrupt(timer, compare_number);
        event_callback_count++;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_RegisterPeriodInterrupt(TIMER timer, TIMER_PERIODIC_ISR periodic_callback)
{
    if(event_callback_count < EVENT_CALLBACK_COUNT)
    {
        event_callbacks[event_callback_count] = (VPTR)periodic_callback;
        periodic_interrupt_callback[timer] = event_callback_count;
        SysTimer_Timer_DisablePeriodicInterrupt(timer);
        event_callback_count++;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_EnablePeriodicInterrupt(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    timer_reg_ptr->DIER |= 0x0001;  //interrupt enable
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_DisablePeriodicInterrupt(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    timer_reg_ptr->DIER &= ~0x0001;  //interrupt disable
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_EnableCompareInterrupt(TIMER timer, COMPARE_NUMBER compare_number)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    timer_reg_ptr->DIER |= (0x0002 << compare_number);  //interrupt enable
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_DisableCompareInterrupt(TIMER timer, COMPARE_NUMBER compare_number)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    timer_reg_ptr->DIER &= ~(0x0002 << compare_number);  //interrupt enable
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysTimer_Timer_RestartCount(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    //Re-initialize the counter and generates an update of the registers. Note that the prescaler
    //counter is cleared too (anyway the prescaler ratio is not affected). The counter is cleared if
    //the center-aligned mode is selected or if DIR=0 (upcounting), else it takes the auto-reload
    //value (TIMx_ARR) if DIR=1 (downcounting).
    timer_reg_ptr->EGR |= 0x0001; //re-init the timer
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
U32 SysTimer_Timer_GetCount(TIMER timer)
{
    TIMER_REG_HNDL  timer_reg_ptr = (TIMER_REG_HNDL)timer_registers[timer];
    return (timer_reg_ptr->CNT);
}
//================================================================================================//
