//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Source file with module responsible for handling the application interrupts
// This interrupt vector table handler module is designed to reduce the complexity of using/initialising interrupts.
// It initialises the vector table in DATA space RAM, which allows the user to register <em>Interrupt Service
// Routines</em> @ runtime.  <em>CRITICAL SECTION</em> entry and exit macro's are also defined here.\n
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORE__SYSINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORE__SYSINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               CORE__SYSINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  ADDRESS_INITIAL_STACK_POINTER: specifies the beginning of the stack
// @remark it is the highest address of the stack section, since the stack pointer counts down on 'push'
#ifndef ADDRESS_INITIAL_STACK_POINTER
    #error "ADDRESS_INITIAL_STACK_POINTER not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "stm32f0xx_iwdg.h"
#include "stm32f0xx_syscfg.h"
extern int main(void);

#if USE_FREERTOS
extern void xPortPendSVHandler( void );
extern void xPortSysTickHandler( void );
extern void vPortSVCHandler( void );
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define NVIC_preemtion_0_levels_subprio_16_levels       0x700
#define NVIC_preemtion_2_levels_subprio_8_levels        0x600
#define NVIC_preemtion_4_levels_subprio_4_levels        0x500
#define NVIC_preemtion_8_levels_subprio_2_levels        0x400
#define NVIC_preemtion_16_levels_subprio_0_levels       0x300

#define NVIC_PriorityGroup_0         ((u32)0x700) /* 0 bits for pre-emption priority
                                                     4 bits for subpriority */
#define NVIC_PriorityGroup_1         ((u32)0x600) /* 1 bits for pre-emption priority
                                                     3 bits for subpriority */
#define NVIC_PriorityGroup_2         ((u32)0x500) /* 2 bits for pre-emption priority
                                                     2 bits for subpriority */
#define NVIC_PriorityGroup_3         ((u32)0x400) /* 3 bits for pre-emption priority
                                                     1 bits for subpriority */
#define NVIC_PriorityGroup_4         ((u32)0x300) /* 4 bits for pre-emption priority
                                                     0 bits for subpriority */

#define AIRCR_VECTKEY_MASK           ((uint32_t)0x05FA0000)
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static __irq void SysIntHandleNmi(void);
static __irq void SysIntHandleHardFault(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
#pragma data_alignment=16
static ISR_CALLBACK           vector_table[48] @ "ram_isr_table";

#if USE_FREERTOS
const U32                       Interrupts[] @ "vector" =
{
    (U32)ADDRESS_INITIAL_STACK_POINTER,     // The initial stack pointer
    (U32)main,                              // 1 reset handler
    (U32)SysIntHandleNmi,                   // 2 NMI handler
    (U32)SysIntHandleHardFault,             // 3 hard fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    (U32)vPortSVCHandler,                   // 11 SVCall exception handler
    0,                                      // Reserved
    0,                                      // Reserved
    (U32)xPortPendSVHandler,                // 14 The PendSVC handler
    (U32)xPortSysTickHandler                // 15 SysTick interrupt handler
};
#else
const U32                       Interrupts[] @ "vector" =
{
    (U32)ADDRESS_INITIAL_STACK_POINTER,     // The initial stack pointer
    (U32)main,                              // 1 reset handler
    (U32)SysIntHandleNmi,                   // 2 NMI handler
    (U32)SysIntHandleHardFault,             // 3 hard fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // 11 SVCall exception handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // 14 The PendSVC handler
    0,                                      // 15 SysTick interrupt handler
};
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief   Handle NMI interrupt function.
static __irq void SysIntHandleNmi(void)
{
	LOG_ERR("");
}
//------------------------------------------------------------------------------------------------//
// @brief   Handle Hard Fault exception
static __irq void SysIntHandleHardFault(void)
{
	LOG_ERR("");
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysInt_Init(void)
{
    MEMSET((VPTR)vector_table, 0, SIZEOF(vector_table));
    MEMCPY((VPTR)vector_table, (VPTR)Interrupts, SIZEOF(Interrupts));
        
    // remap interrupts to RAM
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
    //SYSCFG->CFGR1 = SYSCFG_CFGR1_MEM_MODE;
    
    //set default prio's all to lowest
    MEMSET((VPTR)NVIC->IP, 0xC0, SIZEOF(NVIC->IP));
}
//------------------------------------------------------------------------------------------------//
void SysInt_RegisterIsr(ISR_VECTOR vector, ISR_CALLBACK isr_func)
{
    vector_table[vector + 16] = isr_func;
}
//------------------------------------------------------------------------------------------------//
void SysInt_UnregisterIsr(ISR_VECTOR vector)
{
    vector_table[vector + 16] = NULL;
}
//------------------------------------------------------------------------------------------------//
void SysInt_EnableIsr(ISR_VECTOR vector)
{
    if(vector_table[vector + 16] == NULL) //no handler registered yet
    {
        return;
    }
    
    NVIC->ISER[0] = (U32)0x01 << vector;
}
//------------------------------------------------------------------------------------------------//
void SysInt_DisableIsr(ISR_VECTOR vector)
{
    NVIC->ICER[0] = (U32)0x01 << vector;
}
//------------------------------------------------------------------------------------------------//
void SysInt_EnableInterrupts(void)
{
    __enable_irq();
}
//------------------------------------------------------------------------------------------------//
void SysInt_DisableInterrupts(void)
{
    __disable_irq();
}
//------------------------------------------------------------------------------------------------//
void SysInt_SetPriority(ISR_VECTOR vector, U8 priority)
{
/*
    // from void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct)
    
    tmppriority = NVIC->IP[NVIC_InitStruct->NVIC_IRQChannel >> 0x02];
    tmppriority &= (uint32_t)(~(((uint32_t)0xFF) << ((NVIC_InitStruct->NVIC_IRQChannel & 0x03) * 8)));
    tmppriority |= (uint32_t)((((uint32_t)NVIC_InitStruct->NVIC_IRQChannelPriority << 6) & 0xFF) << ((NVIC_InitStruct->NVIC_IRQChannel & 0x03) * 8));    
    
    NVIC->IP[NVIC_InitStruct->NVIC_IRQChannel >> 0x02] = tmppriority;
*/
    U8  index;
    U8  shift;
    U32 tmppriority;
    
    index = (U8)vector >> 2;
    shift = ((U8)vector & 0x03) << 3;
    
    tmppriority = NVIC->IP[index];
    tmppriority &= (U32)(~((U32)0xFF << shift));
    tmppriority |= (U32)((((U32)priority << 6) & 0xFF) << shift);
    
    NVIC->IP[index] = tmppriority;
}
//================================================================================================//
