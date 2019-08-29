/*
*                    Copyright (c), NXP Semiconductors
*
*                       (C)NXP Electronics N.V.2005
*         All rights are reserved. Reproduction in whole or in part is
*        prohibited without the written consent of the copyright owner.
*    NXP reserves the right to make changes without notice at any time.
*   NXP makes no warranty, expressed, implied or statutory, including but
*   not limited to any implied warranty of merchantability or fitness for any
*  particular purpose, or that the use will not infringe any third party patent,
*   copyright or trademark. NXP must not be liable for any loss or damage
*                            arising from its use.
*
*/

/*!
* \file  phlnLlcp_Fri_OvrHal.c
* \brief Overlapped HAL
*
* $Author: nxp40786 $
* $Revision: 458 $
* $Date: 2014-01-04 18:36:24 +0530 (Sat, 04 Jan 2014) $
*
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI

#include "phlnLlcp_Fri_OvrHal.h"
#include <phpalI18092mPI.h>
#include <phpalI18092mT.h>
#include <phlnLlcp.h>
#include <phOsal.h>

static uint8_t bOvrHal_running;
static phlnLlcp_Fri_OvrHalTask_t   sOvrHalTask;


void phlnLlcp_Fri_OvrHal_Init(void)
{
    sOvrHalTask.timerID = PH_OSAL_INVALID_TIMER_ID;
}

void phlnLlcp_Fri_OvrHal_DeInit(void)
{
    bOvrHal_running = false;
}

phStatus_t phlnLlcp_Fri_OvrHal_Receive(void                    *OvrHal,
                                       phlnLlcp_Fri_CplRt_t    *CompletionInfo,
                                       uint8_t                **RecvBuf,
                                       uint16_t                *RecvLength)
{
    phStatus_t status = PH_ERR_PENDING;
    phStatus_t   temp = PH_ERR_SUCCESS;

    PH_UNUSED_VARIABLE(temp);

    /* Check the Input Parameters */
    if ((NULL == OvrHal) || (NULL == CompletionInfo) || (NULL == RecvBuf) ||
        (NULL == RecvLength))
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_OVR_HAL);
    }
    else
    {
        sOvrHalTask.CompletionInfo   = CompletionInfo;
        sOvrHalTask.RecvBuf          = RecvBuf;
        sOvrHalTask.RecvLength       = RecvLength;
        sOvrHalTask.ovrHal           = OvrHal;
        sOvrHalTask.bAction          = receive;

        /* Start the Receive action */
        temp = phpalI18092mT_Listen(OvrHal,
                                    PHHAL_HW_OPTION_START_CMD_ONLY,
                                    sOvrHalTask.RecvBuf,
                                    sOvrHalTask.RecvLength,
                                    &sOvrHalTask);
    }
    return status;
}

phStatus_t phlnLlcp_Fri_OvrHal_Send(void                    *OvrHal,
                                    phlnLlcp_Fri_CplRt_t    *CompletionInfo,
                                    uint8_t                 *SendBuf,
                                    uint16_t                 SendLength)
{
    phStatus_t status = PH_ERR_PENDING;

    /* Check the Input Parameters */
    if ((NULL == OvrHal) || (NULL == CompletionInfo) || (NULL == (void*)SendBuf)
        || (0 == SendLength))
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_OVR_HAL);
    }
    else
    {
        sOvrHalTask.CompletionInfo   = CompletionInfo;
        sOvrHalTask.SendBuf          = SendBuf;
        sOvrHalTask.SendLength       = SendLength;
        sOvrHalTask.ovrHal           = OvrHal;
        sOvrHalTask.bAction          = transmit;

        /* Start the Timer Ticks */
        phlnLlcp_Fri_OvrHal_Task(0, &sOvrHalTask);

    }

    return status;
}

/* Async Data exchange - using timer */
phStatus_t phlnLlcp_Fri_OvrHal_Transceive(void                    *OvrHal,
                                          phlnLlcp_Fri_CplRt_t    *CompletionInfo,
                                          uint8_t                 *SendBuf,
                                          uint16_t                 SendLength,
                                          uint8_t                **RecvBuf,
                                          uint16_t                *RecvLength)
{
    phStatus_t status = PH_ERR_PENDING;

    /* Check the Input Parameters */
    if ((NULL == OvrHal) || (NULL == CompletionInfo) || (NULL == (void*)SendBuf)
        || (NULL == RecvBuf) || (NULL == RecvLength) || (0 == SendLength))
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_OVR_HAL);
    }
    else
    {
        sOvrHalTask.CompletionInfo   = CompletionInfo;
        sOvrHalTask.SendBuf          = SendBuf;
        sOvrHalTask.SendLength       = SendLength;
        sOvrHalTask.RecvBuf          = RecvBuf;
        sOvrHalTask.RecvLength       = RecvLength;
        sOvrHalTask.ovrHal           = OvrHal;
        sOvrHalTask.bAction          = transceive;

        /* Start the Timer Ticks */
        phlnLlcp_Fri_StartTicks(&sOvrHalTask);

    }

    return status;
}

void phlnLlcp_Fri_OvrHal_Task(uint32_t timerId, void * pOvrHalTask)
{
    pphlnLlcp_Fri_Cr_t CompletionCB;
    phlnLlcp_Fri_OvrHalTask_t *param;
    uint8_t tempBuf[] = {0x0, 0x0};
    phStatus_t status = PH_ERR_SUCCESS;

    bOvrHal_running = true;

    if(pOvrHalTask != NULL)
    {
        param = pOvrHalTask;

        CompletionCB = param->CompletionInfo->CompletionRoutine;

        switch(param->bAction)
        {
        case transmit:
            status = phpalI18092mT_DepSend(
                param->ovrHal,
                PH_EXCHANGE_DEFAULT,
                param->SendBuf,
                param->SendLength);
            break;

        case receive:
            /* Receive action is triggered in a separate function */
            break;

        case transceive:
            status = phpalI18092mPI_Exchange(param->ovrHal,
                                            PH_EXCHANGE_DEFAULT,
                                            param->SendBuf,
                                            param->SendLength,
                                            param->RecvBuf,
                                            param->RecvLength);
            break;

        default:
            status = PH_ERR_INVALID_DATA_PARAMS;
            break;
        }

        if (status == PH_ERR_SUCCESS)
        {
            CompletionCB(param->CompletionInfo->Context, PH_ERR_SUCCESS);
        }
        else
        {
#ifdef DEBUG
        	printf("Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n", status);
#endif /* DEBUG */
        	CompletionCB(param->CompletionInfo->Context, PH_ERR_IO_TIMEOUT);
        }
    }

    bOvrHal_running = true;
}

void phlnLlcp_Fri_StartTicks(void   *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint32_t nDuration = 1;

    phlnLlcp_Fri_Mac_t *LlcpMac;
    phlnLlcp_Fri_OvrHalTask_t   *pOvrHalTask = (phlnLlcp_Fri_OvrHalTask_t *)pContext;
    LlcpMac = (phlnLlcp_Fri_Mac_t *)(pOvrHalTask->CompletionInfo->Context);
    pOvrHalTask->Osal = LlcpMac->Osal;

    PH_UNUSED_VARIABLE(status);

    if (pOvrHalTask->timerID == PH_OSAL_INVALID_TIMER_ID)
    {
        status = phOsal_Timer_Create(pOvrHalTask->Osal, &(pOvrHalTask->timerID));
    }

    status = phOsal_Timer_Stop(pOvrHalTask->Osal, pOvrHalTask->timerID);

    /* Reset the Timer */
    status = phOsal_Timer_Reset(pOvrHalTask->Osal,pOvrHalTask->timerID);

    status = phOsal_Timer_Start( pOvrHalTask->Osal,
                                 pOvrHalTask->timerID,
                                 nDuration,
                                 PH_OSAL_TIMER_UNIT_US,
                                 &phlnLlcp_Fri_OvrHal_Task,
                                 (void *)pOvrHalTask);

    return;
}


void phlnLlcp_Fri_StopTicks(void   *pContext)
{
    phlnLlcp_Fri_OvrHalTask_t   *pOvrHalTask = (phlnLlcp_Fri_OvrHalTask_t *)pContext;
    phStatus_t status = PH_ERR_SUCCESS;

    PH_UNUSED_VARIABLE(status);

    status = phOsal_Timer_Stop(pOvrHalTask->Osal, pOvrHalTask->timerID);

    /* Reset the Timer */
    status = phOsal_Timer_Reset(pOvrHalTask->Osal,pOvrHalTask->timerID);

    return;
}

#endif /* NXPBUILD__PHLN_LLCP_FRI */
