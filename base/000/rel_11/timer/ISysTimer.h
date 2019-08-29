//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module to configure timers
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef TIMER__ISYSTIMER_H
#define TIMER__ISYSTIMER_H
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
// @brief   Prototype of a periodic interrupt handler function
typedef void (*TIMER_PERIODIC_ISR)(TIMER t); //-> te vervangen door event typedef

// @brief	Prototype of ISR for compare timer interrupts
// @param	compare_number : the compare channel/register of the timer
typedef void (*TIMER_COMPARE_ISR)(TIMER t, COMPARE_NUMBER compare_number);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void SysTimer_Init(void);

// @brief	Function to initialise a timer with a frequency.
// @param	timer : the selected timer
// @param	source_clock_frequency : The frequency of the tick.
// @remark	When calling this funcion the timer will be (if possible) configured for timer use. No detection is done to see
// if the timer was already configured for something else. The timer will be reconfigured.
// After calling this function all timer interrupts will be disabled and the timer will be stopped.
// You should call the SysTimer_Timer_Start function to start the timer.
// @return the actual configured tick frequency (in best case equal to the requested source_clock_frequency)
U32 SysTimer_Timer_Init(TIMER timer, U32 source_clock_frequency);

// @brief	Function to get the timer width (32, 16 or 8 bit)
// @param	timer : the selected timer
// @return  the widht of the timer(32, 16 or 8 bit)
U8 SysTimer_Timer_GetTimerWidth(TIMER timer);

// @brief	Function to get the number of compare registers
// @param	timer : the selected timer
// @return  the number of compare registers
U8 SysTimer_Timer_GetNumberOfCompares(TIMER timer);

// @brief	Function to start a timer
// @param	timer : the selected timer
// @return  TRUE on success
BOOL SysTimer_Timer_Start(TIMER timer);

// @brief	Function to stop a timer
// @param	timer : the selected timer
// @return  TRUE on success
BOOL SysTimer_Timer_Stop(TIMER timer);

// @brief	Function to set the timer period.
// When the period has been reached, the timer value will be reset and the period handler callback will be called.
// @param	timer : the selected timer
// @param	period_count : the period count value. The user should be aware of the timer width to know the maximum possible period count
// @return  TRUE on success, FALSE if the period count is too big.
BOOL SysTimer_Timer_SetPeriod(TIMER timer, U32 period_count);

// @brief	Function to set the timer period.
// When the period has been reached, the timer value will be reset and the period handler callback will be called.
// @param	timer : the selected timer
// @param	compare_number : the compare you want to configure
// @param	compare_count : the count for the compare. The user should be aware of the timer width to know the maximum possible count
// @return  TRUE on success, FALSE if the compare_count is too big or if the compare number doesn't exist
BOOL SysTimer_Timer_SetCompare(TIMER timer, COMPARE_NUMBER compare_number, U32 compare_count);

// @brief	Function to register an interrupt to go of when one of the timer's compare values has been reached.
// @param	timer : the selected timer
// @param	compare_number : the compare you want to configure
// @param	compare_callback : function to be called when the timer matches the compare
// @remark	After calling this regiser function the interrupt will be standard disabled. you should enable with
//  		the function SysTimer_Timer_EnableCompareInterrupt.
// @return TRUE on success
BOOL SysTimer_Timer_RegisterCompareInterrupt(TIMER timer, COMPARE_NUMBER compare_number, TIMER_COMPARE_ISR compare_callback);

// @brief	Function to register an interrupt to go of when the timer's period elapses. This can be used for
// 			periodic events.
// @param	timer : the selected timer
// @param	periodic_callback : function to be called when the timer period elapses
// @remark	After calling this regiser function the interrupt will be standard disabled. you should enable with
//  		the function SysTimer_Timer_EnablePeriodicInterrupt.
// @return TRUE on success
BOOL SysTimer_Timer_RegisterPeriodInterrupt(TIMER timer, TIMER_PERIODIC_ISR periodic_callback);

// @brief	Function to enable the periodic interrupt of the timer. This can be used for
// 			periodic events.
// @param	timer : the selected timer
// @return TRUE on success
BOOL SysTimer_Timer_EnablePeriodicInterrupt(TIMER timer);

// @brief	Function to disable the periodic interrupt of the timer. This can be used for
// 			periodic events.
// @param	timer : the selected timer
// @return TRUE on success
BOOL SysTimer_Timer_DisablePeriodicInterrupt(TIMER timer);

// @brief	Function to enable the compare interrupt of the timer.
// @param	timer : the selected timer
// @return TRUE on success
BOOL SysTimer_Timer_EnableCompareInterrupt(TIMER timer, COMPARE_NUMBER compare_number);

// @brief	Function to disable the compare interrupt of the timer.
// @param	timer : the selected timer
// @return TRUE on success
BOOL SysTimer_Timer_DisableCompareInterrupt(TIMER timer, COMPARE_NUMBER compare_number);

// @brief   Function to reset the hardware timer count to 0.
//          If the timer is not running, this function will NOT start it.
// @param	timer : the selected timer
// @return TRUE on success, FALSE when timer handle is invalid or when there is no interrupt callback registered
BOOL SysTimer_Timer_RestartCount(TIMER timer);

// @brief   This function returns the tick count
// @param	timer : the selected timer
// @return  Time in tick.
U32 SysTimer_Timer_GetCount(TIMER timer);

// @remark  Contains a LOG_ERR, so function will not return if invalid
void SysTimer_Timer_CheckIfLegal(TIMER timer);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



#endif /* TIMER__ISYSTIMER_H */
