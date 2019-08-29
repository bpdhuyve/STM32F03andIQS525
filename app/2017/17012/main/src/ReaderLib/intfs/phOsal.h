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
* Generic OSAL Component of Reader Library Framework.
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
* History:
*  PC: Generated 23. Aug 2012
*
*/

#ifndef PHOSAL_H
#define PHOSAL_H

#include <ph_Status.h>
#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif    /* __cplusplus */

#ifdef NXPBUILD__PH_OSAL_STUB

/** \defgroup phOsal_Stub Component : Stub
* \brief These Components implement the Osal commands.
* @{
*/
#define RDLIB_STUB_MAX_TIMERS           3U         /**< Maximum number of timers available */
#define PH_OSAL_STUB_ID                 0x04U      /**< ID for Stub osal component */

#define TIMER_USED                      1U         /**< Indicates that the timer is used */
#define TIMER_FREE                      0U         /**< Indicates that the timer is free */
#ifndef TIME_INTERVAL
#define TIME_INTERVAL    				(SystemFrequency/100 - 1)
#endif

/**
*
* \brief Timer callback interface which will be called once registered timer
* timeout expires.
* \param[in] TimerId  Timer Id for which callback is called.
* \retval  None
*/
typedef void (*ppCallBck_t)(uint32_t TimerId, void *pContext);

typedef struct Timer_Struct
{
    uint32_t       dwTimerId;              /**< ID of the timer */
    uint8_t        bTimerFree;             /**< Indicates whether the current timer is free or used */
    ppCallBck_t    pApplicationCallback;   /**< The call back function for this timer */
    void          *pContext;               /**< The argument to the call back function */
} Timer_Struct_t;

/**
* \brief lpc17xx osal parameter stucture
*/
typedef struct
{
    uint16_t           wId;                            /**< ID of this component, do not modify */
    Timer_Struct_t     gTimers[RDLIB_STUB_MAX_TIMERS];    /**< Timer structure */
} phOsal_Stub_DataParams_t;

/**
* Initializes the Stub timer component
*
* \return status code
* \retval #PH_ERR_SUCCESS Operation successful.
*
*/
phStatus_t phOsal_Stub_Init( phOsal_Stub_DataParams_t  *pDataParams );    /**<[In] Data parameters representing this component */


/** @} */
#endif /* NXPBUILD__PH_OSAL_STUB */

#ifdef NXPBUILD__PH_OSAL

/** \defgroup phOsal Operating System Abstraction Layer
* \brief These Components implement the Osal commands.
* @{
*/
#define PH_OSAL_TIMER_UNIT_US            0x00U       /**< Indicates that the specified delay is in microseconds.*/
#define PH_OSAL_TIMER_UNIT_MS            0x01U       /**< Indicates that the specified delay is in milliseconds. */
#define PH_OSAL_INVALID_TIMER_ID         0xFFFF      /**< Invalid Timer Id. */

/**
* \name OSAL error codes
*/
/* @{*/

#define PH_OSAL_ERR_NO_FREE_TIMER     (PH_ERR_CUSTOM_BEGIN + 0)   /**< Error condition indicating that no timer is available for allocation. */
#define PH_OSAL_ERR_INVALID_TIMER     (PH_ERR_CUSTOM_BEGIN + 1)   /**< Indicates that the timer ID that was supplied was invalid. */

/** @}*/

/* This Macro to be used to resolve unused and unreferenced compiler warnings. */
#define PHOSAL_UNUSED_VARIABLE(x) for((x)=(x);(x)!=(x);)

/**
*
* \brief Timer callback interface which will be called once registered timer
* timeout expires.
* \param[in] TimerId  Timer Id for which callback is called.
* \retval  None
*/
typedef void (*ppCallBck_t)(uint32_t TimerId, void *pContext);



/**
* \brief initialises timers to the application
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_Timer_Init( void  *pDataParams );   /**< [In] Pointer to this layers parameter structure. */


/**
* \brief Allocates a timer to the application
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_Timer_Create( void       *pDataParams,   /**< [In] Pointer to this layers parameter structure. */
                                uint32_t   *timerId        /**< [In] Timer Id */
                               );

/**
* \brief Start the timer
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_Timer_Start( void         *pDataParams,            /**< [In] Pointer to this layers parameter structure. */
                               uint32_t      dwTimerId,              /**< [In] Timer Id */
                               uint32_t      dwRegTimeCnt,           /**< [In] Time delay count */
                               uint16_t      wOption,                /**< [In] Option parameter. */
                               ppCallBck_t   pApplication_callback,  /**< [In] Callback to be called on time out */
                               void         *pContext                /**< [In] Callback function context */
                              );

/**
* \brief Stop the timer
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_Timer_Stop( void        *pDataParams,   /**< [In] Pointer to this layers parameter structure. */
                              uint32_t     dwTimerId      /**< [In] Timer Id */
                             );

/**
* \brief Timer wait function
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_Timer_Wait( void      *pDataParams,       /**< [In] Pointer to this layers parameter structure. */
                              uint8_t    bTimerDelayUnit,   /**< [In] Delay value unit could be in microseconds or milliseconds */
                              uint16_t   wDelay             /**< [In] Time Delay */
                             );

/**
* \brief Allocates a timer to the application
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_Timer_Reset( void      *pDataParams,      /**< [In] Pointer to this layers parameter structure. */
                               uint32_t   dwtimerId         /**< [In] Timer Id */
                              );


/**
* \brief Delete or deallocate the timer
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_Timer_Delete( void       *pDataParams,    /**< [In] Pointer to this layers parameter structure. */
                                uint32_t    dwTimerId       /**< [In] Timer Id */
                                );

phStatus_t phOsal_Timer_ExecCallback( void     *pDataParams,    /**< [In] Pointer to this layers parameter structure. */
                                      uint32_t  dwTimerId       /**< [In] Timer Id */
                                      );


/**
* \brief Get memory from the heap segment
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/

phStatus_t phOsal_GetMemory( void        *pDataParams, /**< [In] Pointer to this layers parameter structure. */
                            uint32_t     dwLength,    /**< [In] Required memory length */
                            void       **pMem         /**< [Out] Pointer to Memory allocated */
                            );

/**
* \brief Free the memory allocated
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phOsal_FreeMemory( void    *pDataParams,    /**< [In] Pointer to this layers parameter structure. */
                              void    *ptr             /**< [In] Pointer to memory to be freed */
                            );


/** @} */
#endif /* NXPBUILD__PH_OSAL */

#ifdef  __cplusplus
}
#endif /* __cplusplus */
#endif /* PHOSAL_H */

