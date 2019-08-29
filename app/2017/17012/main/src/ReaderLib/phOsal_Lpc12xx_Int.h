/*
 *         Copyright (c), NXP Semiconductors Bangalore / India
 *
 *                     (C)NXP Semiconductors
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 *particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 */

/** \file
 * LPC12xx OSAL Component of Reader Library Framework.
 * $Author: prabakaran.c $
 * $Revision: 1.4 $
 * $Date: Mon Dec  3 09:40:04 2012 $
 *
 * History:
 *  PC: Generated 23. Aug 2012
 *
 */

#ifndef PHOSAL_LPC12XX_INT_H
#define PHOSAL_LPC12XX_INT_H

//#include <LPC12xx.h>
#include <phOsal.h>
#include "ph_NxpBuild.h"
#include "comps\comps\phOsal\src\Stub\phOsal_Stub.h"

/* limitations for 16bit timers used to emulate 32 bit timers */
#define TIMER_16_BIT_MASK        0xFFFFU
#define TIMER_16_MAX_US_TIME     1000       // maximum time for microsecond timer
#define TIMER_16_MIN_MS_TIME     1          // manimum time for milisecond timer
#define TIMER_16_MAX_MS_TIME     0xFFFFU    // maximum time for milisecond timer
#define TIMER_16_PERIOD_MS_TIME  0xBB80U    // default value to generate 1 milisecond

#define LPC12XX_MAX_TIMERS		 3U    		/* number of timers on LPC1227 counting 32bit and 16 bit */
#define LPC12XX_DEFAULT_TIME_INTERVAL     10000U         /* Default timer interval that will be loaded */

/* references to timers peripherals */
#define LPC_TMR32B0              LPC_CT32B0  /* Timer 0 - 32bit */
#define LPC_TMR32B1           	 LPC_CT32B1  /* Timer 1 - 32bit */
#define LPC_TMR16B0			     LPC_CT16B0	 /* Timer 0 - 16bit */
#define LPC_TMR16B1			     LPC_CT16B1	 /* Timer 1 - 16bit */

/**
 * \ Initializes timers to the application
 */

void phOsal_Lpc12xx_Int_Timer_Init(phOsal_Stub_DataParams_t *pDataParams);

/**
 * Creates a timer component.
 * Internally, an array is maintained which stores timers along with
 * information as to whether the timer is available or not. This
 * function searches a free timer that is available and returns the
 * timer id in \a pTimerId. If there are no free timers,
 * then appropriate error code is returned.
 *
 * \return   status code
 * \retval   #PH_ERR_SUCCESS on success
 * \retval   #PH_OSAL_ERR_NO_FREE_TIMER if no timers are available
 *
 */

void phOsal_Lpc12xx_Int_Timer_Create(phOsal_Stub_DataParams_t *pDataParams, uint32_t *pTimerId);

/**
 * Timer Register.
 * \retval  #PH_ERR_SUCCESS on success
 *
 */

phStatus_t phOsal_Lpc12xx_Int_Timer_Register(
        phOsal_Stub_DataParams_t *pDataParams,
        uint32_t     dwTimerId,
        uint32_t     dwRegTimeCnt,
        uint16_t     wOption,
        ppCallBck_t  pApplicationCallback,
        void         *pContext
);

/**
 * Stops the said timer.
 * This function does not free the timer. It only disables the timer.
 * Use phOsal_Timer_Delete() to free the timer.
 *
 * \return  status code
 * \retval  #PH_OSAL_ERR_INVALID_TIMER if the timer ID supplied was invalid
 * \retval  #PH_ERR_SUCCESS            on success
 *
 */

phStatus_t phOsal_Lpc12xx_Int_Timer_Stop(uint32_t dwTimerId);

/**
 * Frees the timer.
 * When this function is called, the timer with given ID is returned to the
 * free timer pool.
 *
 * \return  status code
 * \retval  #PH_OSAL_ERR_INVALID_TIMER if the timer ID supplied was invalid
 * \retval  #PH_ERR_SUCCESS            on success
 *
 */

phStatus_t phOsal_Lpc12xx_Int_Timer_Delete(
        phOsal_Stub_DataParams_t *pDataParams,
        uint32_t dwTimerId
);



/**
 * Start the timer
 * When this function is called with timer number provided
 * as an argument, the corresponding timer is enabled.
 *
 *
 */
void phOsal_Lpc12xx_Int_Enable_Timer(uint32_t dwTimerId);

/**
 * Stop the timer.
 * When this function is called with timer number provided
 * as an argument, the corresponding timer is disabled.
 *
 *
 */

void phOsal_Lpc12xx_Int_Disable_Timer(uint32_t dwTimerId);

/**
 * Timer Reset
 *
 */

void phOsal_Lpc12xx_Int_Reset_Timer(uint32_t dwTimerId);

/**
 * \ Initializes the timer.
 */

void phOsal_Lpc12xx_Int_Init_Timer_16_32(uint32_t dwTimerId, uint32_t TimerInterval);

/**
 * \ Load the given timer interval.
 */

void phOsal_Lpc12xx_Int_Load_TimerMs(uint32_t dwTimerId, uint32_t dwTimerInterval);

/**
 * \ Load the given timer interval.
 */

void phOsal_Lpc12xx_Int_Load_TimerUs(uint32_t dwTimerId, uint32_t dwTimerInterval);

/**
 * \ Load the given timer interval.
 */
void phOsal_Lpc12xx_Int_Delay_Us(uint32_t dwTimerId, uint32_t delayInUs);

/**
 * \ Load the given timer interval.
 */
void phOsal_Lpc12xx_Int_Delay_Ms(uint32_t dwTimerId, uint32_t delayInUs);


#endif /* PHOSAL_LPC12XX_INT_H */

/*==============================================================================================
 *       End of file
 =============================================================================================*/
