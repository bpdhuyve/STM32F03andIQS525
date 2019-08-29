//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// CPU and System functionality
// This system module offers functionality for initialising CPU and other system parts.\n\n\n
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORE__SYSTEM_H
#define CORE__SYSTEM_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "core\ISystem.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @brief   Processor name definition for use in desperate situations.
#define STM32F0XX
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
// @brief   Function to get the PLL PREDIV clock frequency
U32 SysGetPreDivClk(void);

// @brief   Function to get the APB1 clock frequency
// @return  APB1 clock frequency in Hz.
U32 SysGetAPBClk(void);

// @brief   Function to get the AHB clock frequency
// @return  AHB clock frequency in Hz.
U32 SysGetAHBClk(void);

// @brief   Function to get the system clock frequency
// @return  SysClk frequency in Hz.
U32 SysGetSysClk(void);

// @brief   Function to get the timer clock frequency (TIMxCLK)
// @return  Timer clock frequency in Hz.
U32 SysGetTimClk(void);

// @brief   Function to get the HSE clock frequency (TIMxCLK)
// @return  HSE clock frequency in Hz.
U32 SysGetHSEClk(void);

// @brief   Selects the clock source to output on MCO pin.
// @param   clock_source : - RCC_MCO_NoClock: No clock selected
//                         - RCC_MCO_SYSCLK: System clock selected
//                         - RCC_MCO_HSI: HSI oscillator clock selected
//                         - RCC_MCO_HSE: HSE oscillator clock selected
//                         - RCC_MCO_PLLCLK_Div2: PLL clock divided by 2 selected
void SysSetMCOClockSource(U8 clock_source);

void System_Stop(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CORE__SYSTEM_H */
