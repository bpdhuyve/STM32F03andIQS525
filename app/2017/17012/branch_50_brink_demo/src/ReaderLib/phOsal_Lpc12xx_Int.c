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
 * $Author: nxp53811 $
 * $Revision:  $
 * $Date: 2013-09-25 $
 *
 * History:
 *
 */

#include <ph_Status.h>

#ifdef NXPBUILD__PH_OSAL_STUB

#include "phOsal_Lpc12xx_Int.h"

static phOsal_Stub_DataParams_t* pTimerOsal;

/*==============================================================================================
 * Function:	TIMER32_0_IRQHandler
 *
 * brief:	interrupt handler for TIMER 0
 *
 * -------------------------------------------------------------------------------------------*/
void TIMER32_0_IRQHandler( void )
    {
//    if ( LPC_TMR32B0->IR & (0x01) )
//        {
//        LPC_TMR32B0->IR = 0x1;         /* clear interrupt flag */
//        }
//    /* This is the only timer we are going to use for call backs  */
//    phOsal_Timer_ExecCallback(pTimerOsal, 0);
//
//    return;
    }

/*==============================================================================================
 * Function:	TIMER32_1_IRQHandler
 *
 * brief:	interrupt handler for TIMER 1
 *
 * -------------------------------------------------------------------------------------------*/
void TIMER32_1_IRQHandler( void )
    {
//    if ( LPC_TMR32B1->IR & (0x01) )
//        {
//        LPC_TMR32B1->IR = 0x1;         /* clear interrupt flag */
//        }
//    phOsal_Timer_ExecCallback(pTimerOsal, 1);
//
//    return;
    }

/*==============================================================================================
 * Function:	TIMER16_0_IRQHandler
 *
 * brief:	interrupt handler for TIMER 2
 *
 * -------------------------------------------------------------------------------------------*/
void TIMER16_0_IRQHandler( void )
    {
//    if ( LPC_TMR16B0->IR & (0x01) )
//        {
//        LPC_TMR16B0->IR = 0x1;         /* clear interrupt flag */
//        }
//    phOsal_Timer_ExecCallback(pTimerOsal, 2);
//
//    return;
    }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Timer_Init
 *
 * DESCRIPTION:
 * 		timer 0 is set to 32 bit TIMER_0
 * 		timer 1 is set to 32 bit TIMER_1
 *		   timer 2 is set to 16 bit TIMER_0
 *		   timer 3 is set to 16 bit TIMER_1
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Timer_Init(phOsal_Stub_DataParams_t * pDataParams)
   {
//   uint8_t bCount;
//
//   /* Initialize the timer structure */
//   for (bCount = 0; bCount < LPC12XX_MAX_TIMERS; bCount++)
//      {
//      pDataParams->gTimers[bCount].dwTimerId = bCount;
//      pDataParams->gTimers[bCount].bTimerFree = TIMER_FREE;
//      pDataParams->gTimers[bCount].pApplicationCallback = NULL;
//      pDataParams->gTimers[bCount].pContext = NULL;
//      }
//
//   /* Also keep the timers ready with all the default settings.
//    *       This also does the job of registering the IRQ with NVIC
//    */
//   phOsal_Lpc12xx_Int_Init_Timer_16_32(0, LPC12XX_DEFAULT_TIME_INTERVAL);	// 32 bit TIMER_0
//   phOsal_Lpc12xx_Int_Init_Timer_16_32(1, LPC12XX_DEFAULT_TIME_INTERVAL);	// 32 bit TIMER_1
//   phOsal_Lpc12xx_Int_Init_Timer_16_32(2, LPC12XX_DEFAULT_TIME_INTERVAL);	// 16 bit TIMER_0
//   phOsal_Lpc12xx_Int_Init_Timer_16_32(3, LPC12XX_DEFAULT_TIME_INTERVAL);	// 16 bit TIMER_1
//
//   pTimerOsal = pDataParams;
//
//   /* Set interrupts from timers to second lowest level to keep interface
//    * bus communication enabled during execution of application callback */
//   NVIC_SetPriority(TIMER_16_0_IRQn, 2);
//   NVIC_SetPriority(TIMER_16_1_IRQn, 2);
//   NVIC_SetPriority(TIMER_32_0_IRQn, 2);
//   NVIC_SetPriority(TIMER_32_1_IRQn, 2);

   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Timer_Create
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Timer_Create(
      phOsal_Stub_DataParams_t *pDataParams,
      uint32_t * pTimerId
)
   {
//   uint8_t bCount;
//
//   *pTimerId = -1;
//
//   /* Look for a free timer that is available for use */
//   for (bCount = 0; bCount < LPC12XX_MAX_TIMERS; bCount++)
//      {
//      if (pDataParams->gTimers[bCount].bTimerFree == TIMER_FREE)
//         {
//         /* Use this timer */
//         pDataParams->gTimers[bCount].bTimerFree = TIMER_USED;
//         pDataParams->gTimers[bCount].dwTimerId = bCount;
//         *pTimerId = pDataParams->gTimers[bCount].dwTimerId;
//
//         break;
//         }
//      }
   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Timer_Register
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
phStatus_t phOsal_Lpc12xx_Int_Timer_Register(
      phOsal_Stub_DataParams_t *pDataParams,
      uint32_t     dwTimerId,
      uint32_t     dwRegTimeCnt,
      uint16_t     wOption,
      ppCallBck_t  pApplicationCallback,
      void         *pContext
)
   {
   if (dwTimerId > LPC12XX_MAX_TIMERS ||  pDataParams->gTimers[dwTimerId].bTimerFree == TIMER_FREE)
      {
      /* Can't use a non existent timer */
      /* Can't start a free timer, first create the timer */

      return PH_ERR_INTERNAL_ERROR;
      }

   /* Remember the call back function as well as argument to be passed */
   pDataParams->gTimers[dwTimerId].pContext = pContext;
   pDataParams->gTimers[dwTimerId].pApplicationCallback = pApplicationCallback;

   phOsal_Lpc12xx_Int_Init_Timer_16_32(dwTimerId, dwRegTimeCnt);

   if (wOption == 1)
      {
      /* Load the delay value in Milli seconds */
      phOsal_Lpc12xx_Int_Load_TimerMs(dwTimerId, dwRegTimeCnt);
      }
   else if (wOption == 0)
      {
      /* Load the delay value in Micro seconds */
      phOsal_Lpc12xx_Int_Load_TimerUs(dwTimerId, dwRegTimeCnt);
      }
   else
      {
      return PH_ERR_INTERNAL_ERROR;
      }

   /* Start the timer */
   phOsal_Lpc12xx_Int_Enable_Timer(dwTimerId);

   return PH_ERR_SUCCESS;
   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Timer_Stop
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
phStatus_t phOsal_Lpc12xx_Int_Timer_Stop( uint32_t dwTimerId )
   {
   if (dwTimerId > LPC12XX_MAX_TIMERS)
      {
      return PH_ERR_INTERNAL_ERROR;
      }
   /* Stop the timer */
   phOsal_Lpc12xx_Int_Disable_Timer(dwTimerId);

   return PH_ERR_SUCCESS;
   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Timer_Delete
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
phStatus_t phOsal_Lpc12xx_Int_Timer_Delete(
      phOsal_Stub_DataParams_t * pDataParams,
      uint32_t dwTimerId
)
   {
   if (dwTimerId > LPC12XX_MAX_TIMERS)
      {
      return PH_ERR_INTERNAL_ERROR;
      }

   /* Stop the timer and also mark the timer as available */
   phOsal_Lpc12xx_Int_Disable_Timer(dwTimerId);
   pDataParams->gTimers[dwTimerId].dwTimerId = -1;
   pDataParams->gTimers[dwTimerId].bTimerFree = TIMER_FREE;
   pDataParams->gTimers[dwTimerId].pApplicationCallback = NULL;

   return PH_ERR_SUCCESS;
   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Enable_Timer
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Enable_Timer(uint32_t dwTimerId)
   {
//   if ( dwTimerId == 0 )
//      {
//      LPC_TMR32B0->TCR = 1;
//      }
//   else if ( dwTimerId == 1 )
//      {
//      LPC_TMR32B1->TCR = 1;
//      }
//   else if  ( dwTimerId == 2 )
//      {
//      LPC_TMR16B0->TCR = 1;
//      }
//   else if  ( dwTimerId == 3 )
//      {
//      LPC_TMR16B1->TCR = 1;
//      }
//
//   return;
   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Disable_Timer
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Disable_Timer(uint32_t dwTimerId)
   {
//   if ( dwTimerId == 0 )
//      {
//      LPC_TMR32B0->TCR = 0;
//      }
//   else if ( dwTimerId == 1 )
//      {
//      LPC_TMR32B1->TCR = 0;
//      }
//   else if  ( dwTimerId == 2 )
//      {
//      LPC_TMR16B0->TCR = 0;
//      }
//   else if  ( dwTimerId == 3 )
//      {
//      LPC_TMR16B1->TCR = 0;
//      }
//
//   return;
   }

/*==============================================================================================
 * FUNCTION:   phOsal_Lpc12xx_Int_Reset_Timer
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Reset_Timer( uint32_t dwTimerId )
   {
//   if ( dwTimerId == 0 )
//      {
//      LPC_TMR32B0->TCR |= (uint32_t) (1<<1);
//      LPC_TMR32B0->TCR &= ~((uint32_t)(1<<1));
//      LPC_TMR32B0->IR = 0x1<<0;
//      }
//   else if ( dwTimerId == 1 )
//      {
//      LPC_TMR32B1->TCR |= (uint32_t) (1<<1);
//      LPC_TMR32B1->TCR &= ~((uint32_t)(1<<1));
//      LPC_TMR32B1->IR = 0x1<<0;
//      }
//   else if ( dwTimerId == 2 )
//      {
//      LPC_TMR16B0->TCR |= (uint32_t) (1<<1);
//      LPC_TMR16B0->TCR &= ~((uint32_t)(1<<1));
//      LPC_TMR16B0->IR = 0x1<<0;
//      }
//   else if  ( dwTimerId == 3 )
//      {
//      LPC_TMR16B1->TCR |= (uint32_t) (1<<1);
//      LPC_TMR16B1->TCR &= ~((uint32_t)(1<<1));
//      LPC_TMR16B1->IR = 0x1<<0;
//      }
//   return;
   }

/*==============================================================================================
 * FUNCTION:   phOsal_Lpc12xx_Int_Init_Timer_16_32
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Init_Timer_16_32(uint32_t dwTimerId, uint32_t TimerInterval)
   {

//   /* 32 bit timer initialization */
//   /* --------------------------- */
//   if ( dwTimerId == 0 )
//      {
//      LPC_SYSCON->PRESETCTRL |= (0x1<<6);	// reset de-asserted
//      LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<9);	// set system CLK
//
//      LPC_TMR32B0->MR0 = TimerInterval;
//
//      /* Interrupt and Reset on MR0 */
//      LPC_TMR32B0->MCR = (0x5);
//
//      /* Enable the TIMER0 Interrupt */
//      NVIC_EnableIRQ(TIMER_32_0_IRQn);
//      }
//   else if ( dwTimerId == 1 )
//      {
//      LPC_SYSCON->PRESETCTRL |= (0x1<<7);	// reset de-asserted
//      LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);	// set system CLK
//
//      LPC_TMR32B1->MR0 = TimerInterval;
//
//      /* Interrupt and Reset on MR0 */
//      LPC_TMR32B1->MCR = (0x5);
//
//      /* Enable the TIMER1 Interrupt */
//      NVIC_EnableIRQ(TIMER_32_1_IRQn);
//      }
//
//   /* 16 bit timer initialization */
//   /* --------------------------- */
//   else if ( dwTimerId == 2 )
//      {
//      LPC_SYSCON->PRESETCTRL |= (0x1<<4);	// reset de-asserted
//      LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<7);	// set system CLK
//
//      LPC_TMR16B0->MR0 = TimerInterval;
//
//      /* Interrupt and Reset on MR0 */
//      LPC_TMR16B0->MCR = (0x5);
//
//      /* Enable the TIMER0 Interrupt */
//      NVIC_EnableIRQ(TIMER_16_0_IRQn);
//      }
//   else if ( dwTimerId == 3 )
//      {
//      LPC_SYSCON->PRESETCTRL |= (0x1<<5);	// reset de-asserted
//      LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);	// set system CLK
//
//      LPC_TMR16B1->MR0 = TimerInterval;
//
//      /* Interrupt and Reset on MR0 */
//      LPC_TMR16B1->MCR = (0x5);
//
//      /* Enable the TIMER1 Interrupt */
//      NVIC_EnableIRQ(TIMER_16_1_IRQn);
//      }
//   return;
   }

/*==============================================================================================
 * FUNCTION:   phOsal_Lpc12xx_Int_Load_TimerMs
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Load_TimerMs(uint32_t dwTimerId, uint32_t dwTimerInterval)
   {
//   /* Load the given timer interval onto match register 0 */
//
//   /* time number 0 and 1 for 32 bit timer*/
//
//   if (dwTimerId == 0)
//      {
//      LPC_TMR32B0->MR0 = dwTimerInterval* ((SystemCoreClock) / 1000);
//      }
//   else if (dwTimerId == 1)
//      {
//      LPC_TMR32B1->MR0 = dwTimerInterval* ((SystemCoreClock) / 1000);
//      }
//
//   /* time number 2 and 3 for 16 bit timer*/
//
//   else if (dwTimerId == 2)
//      {
//      /* check the upper border from reason 16 bit Match Register  */
//      if (dwTimerInterval > TIMER_16_MAX_MS_TIME)
//         {
//         dwTimerInterval = TIMER_16_MAX_MS_TIME;
//         }
//      /* check the lower border to 0, minimal value is 1ms */
//      if (!dwTimerInterval )
//         {
//         dwTimerInterval = TIMER_16_MIN_MS_TIME;
//         }
//      /* calculate the timer register value */
//      dwTimerInterval--;
//
//      LPC_TMR16B0->PR  = dwTimerInterval; /* set prescaler to get 1 M counts/sec */
//      LPC_TMR16B0->MR0 = (SystemCoreClock/1000) & TIMER_16_BIT_MASK;
//      }
//   else if (dwTimerId == 3)
//      {
//      /* check the upper border from reason 16 bit Match Register  */
//      if (dwTimerInterval > TIMER_16_MAX_MS_TIME)
//         {
//         dwTimerInterval = TIMER_16_MAX_MS_TIME;
//         }
//      /* check the lower border to 0, minimal value is 1ms */
//      if (!dwTimerInterval )
//         {
//         dwTimerInterval = TIMER_16_MIN_MS_TIME;
//         }
//      /* calculate the timer register value */
//      dwTimerInterval--;
//
//      LPC_TMR16B1->PR  = dwTimerInterval; /* set prescaler to get 1 M counts/sec */
//      LPC_TMR16B1->MR0 = (SystemCoreClock/1000) & TIMER_16_BIT_MASK;
//      }
//
//   return;
   }

/*==============================================================================================
 * FUNCTION:   phOsal_Lpc12xx_Int_Load_TimerUs
 *
 * DESCRIPTION:
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Load_TimerUs(uint32_t dwTimerId, uint32_t dwTimerInterval)
   {
//   /* Load the given timer interval onto match register 0 */
//
//   /* time number 0 and 1 for 32 bit timer*/
//
//   if (dwTimerId == 0)
//      {
//      LPC_TMR32B0->MR0 = dwTimerInterval* ((SystemCoreClock) / 1000000);
//      }
//   else if (dwTimerId == 1)
//      {
//      LPC_TMR32B1->MR0 = dwTimerInterval* ((SystemCoreClock) / 1000000);
//      }
//
//   /* time number 2 and 3 for 16 bit timer*/
//
//   else if (dwTimerId == 2)
//      {
//      /*
//       * setup timer #1 for delay
//       * real stable setting time is
//       *       - 2us/48MHz
//       *       - 3us/24MHz
//       *       - 6us/12MHz
//       * maximum time setting is 1000us (it is limitation from reason 1000us = 1ms)
//       */
//
//      if (dwTimerInterval > TIMER_16_MAX_US_TIME)
//         {
//         dwTimerInterval = TIMER_16_MAX_US_TIME;
//         }
//      dwTimerInterval *= (SystemCoreClock/1000000) & TIMER_16_BIT_MASK;
//      LPC_TMR16B0->MR0 = dwTimerInterval;
//      }
//   else if (dwTimerId == 3)
//      {
//      /*
//       * setup timer #1 for delay
//       * real stable setting time is
//       *       - 2us/48MHz
//       *       - 3us/24MHz
//       *       - 6us/12MHz
//       * maximum time setting is 1000us (it is limitation from reason 1000us = 1ms)
//       */
//
//      if (dwTimerInterval > TIMER_16_MAX_US_TIME)
//         {
//         dwTimerInterval = TIMER_16_MAX_US_TIME;
//         }
//      dwTimerInterval *= (SystemCoreClock/1000000) & TIMER_16_BIT_MASK;
//      LPC_TMR16B1->MR0 = dwTimerInterval;
//      }
//
//   return;
   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Delay16Ms
 *
 * DESCRIPTION:
 * 		Start the timer delay in mili_seconds until elapsed
 * 		There is setting of the PLL setting for required frequency
 * 		   #define SYSPLLCTRL_Val
 * 		      - 0x00000024 -> 60MHz
 * 		      - 0x00000023 -> 48MHz
 * 		      - 0x00000041 -> 24MHz
 * 		      - 0x00000060 -> 12MHz
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Delay_Ms(uint32_t dwTimerId, uint32_t delayInMs)
   {
//   /* 32 bit timers */
//   /* ------------- */
//   if (dwTimerId == 0)
//      {
//      /* setup timer #0 for delay */
//      LPC_TMR32B0->TCR = 0x02;        /* reset timer */
//      LPC_TMR32B0->PR  = 0x00;        /* set prescaler to zero */
//      LPC_TMR32B0->MR0 = delayInMs * (SystemCoreClock / 1000);
//      LPC_TMR32B0->IR  = 0xff;        /* reset all interrrupts */
//      LPC_TMR32B0->MCR = 0x04;        /* stop timer on match */
//      LPC_TMR32B0->TCR = 0x01;        /* start timer */
//
//      /* wait until delay time has elapsed */
//      while (LPC_TMR32B0->TCR & 0x01);
//      }
//   else if (dwTimerId == 1)
//      {
//      /* setup timer #1 for delay */
//      LPC_TMR32B1->TCR = 0x02;        /* reset timer */
//      LPC_TMR32B1->PR  = 0x00;        /* set prescaler to zero */
//      LPC_TMR32B1->MR0 = delayInMs * (SystemCoreClock / 1000);
//      LPC_TMR32B1->IR  = 0xff;        /* reset all interrrupts */
//      LPC_TMR32B1->MCR = 0x04;        /* stop timer on match */
//      LPC_TMR32B1->TCR = 0x01;        /* start timer */
//
//      /* wait until delay time has elapsed */
//      while (LPC_TMR32B1->TCR & 0x01);
//      }
//
//   /* 16 bit timers */
//   /* ------------- */
//   else if (dwTimerId == 2)
//      {
//      /*
//       * setup timer #0 for delay
//       * maximum time setting is 0xFFFF ms
//       */
//
//      /* check the upper border from reason 16 bit Match Register  */
//      if (delayInMs > TIMER_16_MAX_MS_TIME)
//         {
//         delayInMs = TIMER_16_MAX_MS_TIME;
//         }
//      /* check the lower border to 0, minimal value is 1ms */
//      if (!delayInMs )
//         {
//         delayInMs = TIMER_16_MIN_MS_TIME;
//         }
//      /* calculate the timer register value */
//      delayInMs--;
//
//      LPC_TMR16B0->TCR = 0x02;		   /* reset timer */
//      LPC_TMR16B0->PR  = delayInMs; /* set prescaler to get 1 M counts/sec */
//      LPC_TMR16B0->MR0 = (SystemCoreClock/1000) & TIMER_16_BIT_MASK;
//      LPC_TMR16B0->IR  = 0xff;		   /* reset all interrrupts */
//      LPC_TMR16B0->MCR = 0x04;		   /* stop timer on match */
//      LPC_TMR16B0->TCR = 0x01;		   /* start timer */
//      /* wait until delay time has elapsed */
//      while (LPC_TMR16B0->TCR & 0x01);
//      }
//   else if (dwTimerId == 3)
//      {
//      /*
//       * setup timer #1 for delay
//       * maximum time setting is 0xFFFF ms
//       */
//
//      /* check the upper border from reason 16 bit Match Register  */
//      if (delayInMs > TIMER_16_MAX_MS_TIME)
//         {
//         delayInMs = TIMER_16_MAX_MS_TIME;
//         }
//      /* check the lower border to 0, minimal value is 1ms */
//      if (!delayInMs )
//         {
//         delayInMs = TIMER_16_MIN_MS_TIME;
//         }
//      /* calculate the timer register value */
//      delayInMs--;
//
//      LPC_TMR16B1->TCR = 0x02;		/* reset timer */
//      LPC_TMR16B1->PR  = delayInMs; /* set prescaler to get 1 M counts/sec */
//      LPC_TMR16B1->MR0 = (SystemCoreClock/1000) & TIMER_16_BIT_MASK;
//      LPC_TMR16B1->IR  = 0xff;		/* reset all interrrupts */
//      LPC_TMR16B1->MCR = 0x04;		/* stop timer on match */
//      LPC_TMR16B1->TCR = 0x01;		/* start timer */
//      /* wait until delay time has elapsed */
//      while (LPC_TMR16B1->TCR & 0x01);
//      }
//
//   return;
   }

/*==============================================================================================
 * FUNCTION:	phOsal_Lpc12xx_Int_Delay_Us
 *
 * DESCRIPTION:
 *       Start the timer delay in micro_seconds until elapsed
 *       There is setting of the PLL setting for required frequency
 *          #define SYSPLLCTRL_Val
 *             - 0x00000024 -> 60MHz
 *             - 0x00000023 -> 48MHz
 *             - 0x00000041 -> 24MHz
 *             - 0x00000060 -> 12MHz
 *
 ---------------------------------------------------------------------------------------------*/
void phOsal_Lpc12xx_Int_Delay_Us(uint32_t dwTimerId, uint32_t delayInUs)
   {
//   /* 32 bit timers */
//   /* ------------- */
//   if (dwTimerId == 0)
//      {
//      /* setup timer #0 for delay */
//      LPC_TMR32B0->TCR = 0x02;        /* reset timer */
//      LPC_TMR32B0->PR  = 0x00;        /* set prescaler to zero */
//      LPC_TMR32B0->MR0 = delayInUs * (SystemCoreClock/1000000);
//      LPC_TMR32B0->IR  = 0xff;        /* reset all interrrupts */
//      LPC_TMR32B0->MCR = 0x04;        /* stop timer on match */
//      LPC_TMR32B0->TCR = 0x01;        /* start timer */
//
//      /* wait until delay time has elapsed */
//      while (LPC_TMR32B0->TCR & 0x01);
//      }
//   else if (dwTimerId == 1)
//      {
//      /* setup timer #1 for delay */
//      LPC_TMR32B1->TCR = 0x02;        /* reset timer */
//      LPC_TMR32B1->PR  = 0x00;        /* set prescaler to zero */
//      LPC_TMR32B1->MR0 = delayInUs * (SystemCoreClock/1000000);
//      LPC_TMR32B1->IR  = 0xff;        /* reset all interrrupts */
//      LPC_TMR32B1->MCR = 0x04;        /* stop timer on match */
//      LPC_TMR32B1->TCR = 0x01;        /* start timer */
//
//      /* wait until delay time has elapsed */
//      while (LPC_TMR32B1->TCR & 0x01);
//      }
//   /* 16 bit timers */
//   /* ------------- */
//   else if (dwTimerId == 2)
//      {
//      /*
//       * setup timer #0 for delay
//       * setup timer #1 for delay
//       * real stable setting time is
//       *       - 2us/48MHz
//       *       - 3us/24MHz
//       *       - 6us/12MHz
//       * maximum time setting is 1000us (it is limitation from reason 1000us = 1ms)
//       */
//
//      /* check the upper border from reason 16 bit Match Register  */
//      if (delayInUs > TIMER_16_MAX_US_TIME)
//         {
//         delayInUs = TIMER_16_MAX_US_TIME;
//         }
//      /* calculate the timer register value */
//      delayInUs *= (SystemCoreClock/1000000) & TIMER_16_BIT_MASK;
//
//      LPC_TMR16B0->TCR = 0x02;      /* reset timer */
//      LPC_TMR16B1->PR  = 0x00;      /* set the prescaler to 0 */
//      LPC_TMR16B0->MR0 = delayInUs;
//      LPC_TMR16B0->IR  = 0xff;      /* reset all interrrupts */
//      LPC_TMR16B0->MCR = 0x04;      /* stop timer on match */
//      LPC_TMR16B0->TCR = 0x01;      /* start timer */
//      /* wait until delay time has elapsed */
//      while (LPC_TMR16B0->TCR & 0x01);
//      }
//   else if (dwTimerId == 3)
//      {
//      /*
//       * setup timer #1 for delay
//       * real stable setting time is
//       *       - 2us/48MHz
//       *       - 3us/24MHz
//       *       - 6us/12MHz
//       * maximum time setting is 1000us (it is limitation from reason 1000us = 1ms)
//       */
//
//      /* check the upper border from reason 16 bit Match Register  */
//      if (delayInUs > TIMER_16_MAX_US_TIME)
//         {
//         delayInUs = TIMER_16_MAX_US_TIME;
//         }
//      /* calculate the timer register value */
//      delayInUs *= (SystemCoreClock/1000000) & TIMER_16_BIT_MASK;
//
//      LPC_TMR16B1->TCR = 0x02;      /* reset timer */
//      LPC_TMR16B1->PR  = 0x00;      /* set the prescaler to 0 */
//      LPC_TMR16B1->MR0 = delayInUs;
//      LPC_TMR16B1->IR  = 0xff;      /* reset all interrrupts */
//      LPC_TMR16B1->MCR = 0x04;      /* stop timer on match */
//      LPC_TMR16B1->TCR = 0x01;      /* start timer */
//      /* wait until delay time has elapsed */
//      while (LPC_TMR16B1->TCR & 0x01);
//      }
//
//   return;
   }

#endif // NXPBUILD__PH_OSAL_STUB
/*==============================================================================================
 * 		End of file
 =============================================================================================*/

