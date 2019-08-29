//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions for the System Tick interface
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORE__ISYSTICK_H
#define CORE__ISYSTICK_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
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
// @brief   Initialiser for the SysTick module with a given period and event callback
// @param   tick_period_in_us :   the period @ which the tick should be generated in usec
// @param   tick_hook :           the function which should be called on this period
void SysTick_Init(U32 tick_period_in_us, EVENT_CALLBACK tick_hook);

// @brief   Function to update the perion of a timer
// @param   tick_period_in_us :   the period @ which the tick should be generated in usec
BOOL SysTick_SetPeriod(U32 tick_period_in_us);

// @brief   Function to start the SysTick
BOOL SysTick_Start(void);

// @brief   Function to stop the SysTick
BOOL SysTick_Stop(void);

// @brief   Function to reset the SysTick
BOOL SysTick_Restart(void);

// @brief   Function to disable the tick interrupt
BOOL SysTick_DisableInterrupt(void);

// @brief   Function to enable the tick interrupt
BOOL SysTick_EnableInterrupt(void);

// @brief   Function to get the tick count
U32 SysTick_GetTickCount(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CORE__ISYSTICK_H */
