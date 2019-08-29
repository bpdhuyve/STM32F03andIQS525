/*
*                  Copyright (c), NXP Semiconductors
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


/**
* \file  phlnLlcp_Fri_MacNfcip.c
* \brief NFC LLCP MAC Mappings For Different RF Technologies.
*
* $Author: nxp40786 $
* $Revision: 452 $
* $Date: 2013-12-19 20:09:00 +0530 (Thu, 19 Dec 2013) $
*
*
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI

#include "phlnLlcp_Fri_MacNfcip.h"
#include "phlnLlcp_Fri_Mac.h"
#include "phlnLlcp_Fri_OvrHal.h"

static phStatus_t phlnLlcp_Fri_MacNfcip_Send(phlnLlcp_Fri_Mac_t               *LlcpMac,
                                             phNfc_sData_t                    *psData,
                                             phlnLlcp_Fri_Mac_Send_CB_t        LlcpMacSend_Cb,
                                             void                             *pContext);


static void phlnLlcp_Fri_MacNfcip_TriggerRecvCb(phlnLlcp_Fri_Mac_t  *LlcpMac,
                                                phStatus_t           status)
{
    phlnLlcp_Fri_Mac_Reveive_CB_t pfReceiveCB;
    void                          *pReceiveContext;

    if (LlcpMac->MacReceive_Cb != NULL)
    {
        /* Save callback params */
        pfReceiveCB = LlcpMac->MacReceive_Cb;
        pReceiveContext = LlcpMac->MacReceive_Context;

        /* Reset the pointer to the Receive Callback and Context*/
        LlcpMac->MacReceive_Cb = NULL;
        LlcpMac->MacReceive_Context = NULL;

        /* Call the receive callback */
        pfReceiveCB(pReceiveContext, status, LlcpMac->psReceiveBuffer);
    }
}

static void phlnLlcp_Fri_MacNfcip_TriggerSendCb(phlnLlcp_Fri_Mac_t  *LlcpMac,
                                                phStatus_t           status)
{
    phlnLlcp_Fri_Mac_Send_CB_t pfSendCB;
    void                       *pSendContext;

    if (LlcpMac->MacSend_Cb != NULL)
    {
        /* Save context in local variables */
        pfSendCB     = LlcpMac->MacSend_Cb;
        pSendContext = LlcpMac->MacSend_Context;

        /* Reset the pointer to the Send Callback */
        LlcpMac->MacSend_Cb = NULL;
        LlcpMac->MacSend_Context = NULL;

        /* Call Send callback */
        pfSendCB(pSendContext, status);
    }
}

static phStatus_t phlnLlcp_Fri_MacNfcip_Chk(phlnLlcp_Fri_Mac_t                   *LlcpMac,
                                            phlnLlcp_Fri_Mac_Chk_CB_t             ChkLlcpMac_Cb,
                                            void                                 *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t Llcp_Magic_Number[] = {0x46,0x66,0x6D};

    if(NULL == LlcpMac || NULL == ChkLlcpMac_Cb || NULL == pContext)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        status = (phStatus_t )memcmp(Llcp_Magic_Number,LlcpMac->psRemoteDevInfo->RemoteDevInfo.NfcIP_Info.ATRInfo,3);
        if(!status)
        {
            LlcpMac->sConfigParam.buffer = &LlcpMac->psRemoteDevInfo->RemoteDevInfo.NfcIP_Info.ATRInfo[3] ;
            LlcpMac->sConfigParam.length = (LlcpMac->psRemoteDevInfo->RemoteDevInfo.NfcIP_Info.ATRInfo_Length - 3);
            status = PH_ERR_SUCCESS;
        }
        else
        {
            status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_MAC);
        }
        ChkLlcpMac_Cb(pContext,status);
    }

    return status;
}

static phStatus_t phlnLlcp_Fri_MacNfcip_Activate (phlnLlcp_Fri_Mac_t   *LlcpMac)
{
    phStatus_t status  = PH_ERR_SUCCESS;

    if(LlcpMac == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        LlcpMac->LinkState = phlnLlcp_Fri_Mac_eLinkActivated;
        LlcpMac->LinkStatus_Cb(LlcpMac->LinkStatus_Context,
                             LlcpMac->LinkState,
                             &LlcpMac->sConfigParam,
                             LlcpMac->PeerRemoteDevType);
    }

    return status;
}

static phStatus_t phlnLlcp_Fri_MacNfcip_Deactivate (phlnLlcp_Fri_Mac_t   *LlcpMac)
{
    phStatus_t status  = PH_ERR_SUCCESS;

    if(NULL == LlcpMac)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        /* Set the flag of LinkStatus to deactivate */
        LlcpMac->LinkState = phlnLlcp_Fri_Mac_eLinkDeactivated;

        if (LlcpMac->SendPending)
        {
            /* Reset Flag */
            LlcpMac->SendPending = false;
            phlnLlcp_Fri_MacNfcip_TriggerSendCb(LlcpMac, PH_ERR_FAILED);
        }

        if (LlcpMac->RecvPending)
        {
            /* Reset Flag */
            LlcpMac->RecvPending = false;
            phlnLlcp_Fri_MacNfcip_TriggerRecvCb(LlcpMac, PH_ERR_FAILED);
        }

        LlcpMac->LinkStatus_Cb(LlcpMac->LinkStatus_Context,
            LlcpMac->LinkState,
            NULL,
            LlcpMac->PeerRemoteDevType);
    }

    return status;
}

static void phlnLlcp_Fri_MacNfcip_Send_Cb(void        *pContext,
                                          phStatus_t   Status)
{
    phlnLlcp_Fri_Mac_t   *LlcpMac = (phlnLlcp_Fri_Mac_t *)pContext;

#ifdef LLCP_CHANGES
    if(gpphLibContext->LibNfcState.next_state
        == eLibNfcHalStateShutdown)
    {
        phLibNfc_Pending_Shutdown();
        Status = PHLNLLCP_FRI_NFCSTATUS_SHUTDOWN;
    }
#endif /* #ifdef LLCP_CHANGES */

    /* Reset Send and Receive Flag */
    LlcpMac->SendPending = false;
    LlcpMac->RecvPending = false;

    phlnLlcp_Fri_MacNfcip_TriggerSendCb(LlcpMac, Status);

}

static void phlnLlcp_Fri_MacNfcip_Receive_Cb(void        *pContext,
                                             phStatus_t   Status)
{
    phlnLlcp_Fri_Mac_t   *LlcpMac = (phlnLlcp_Fri_Mac_t *)pContext;

    phlnLlcp_Fri_MacNfcip_TriggerRecvCb(LlcpMac, Status);

    /* Test if a send is pending */
    if(LlcpMac->SendPending)
    {
        Status = phlnLlcp_Fri_MacNfcip_Send(LlcpMac,LlcpMac->psSendBuffer,LlcpMac->MacSend_Cb,LlcpMac->MacSend_Context);
    }
}

static void phlnLlcp_Fri_MacNfcip_Transceive_Cb(void        *pContext,
                                                phStatus_t   Status)
{
    phlnLlcp_Fri_Mac_t   *LlcpMac = (phlnLlcp_Fri_Mac_t *)pContext;

    /* Reset Send and Receive Flag */
    LlcpMac->SendPending = false;
    LlcpMac->RecvPending = false;

    /* Call the callbacks */
    phlnLlcp_Fri_MacNfcip_TriggerSendCb(LlcpMac, Status);
    phlnLlcp_Fri_MacNfcip_TriggerRecvCb(LlcpMac, Status);
}

static phStatus_t phlnLlcp_Fri_MacNfcip_Send(phlnLlcp_Fri_Mac_t               *LlcpMac,
                                             phNfc_sData_t                    *psData,
                                             phlnLlcp_Fri_Mac_Send_CB_t        LlcpMacSend_Cb,
                                             void                             *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    if(NULL == LlcpMac || NULL == psData || NULL == LlcpMacSend_Cb || NULL == pContext)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else if(LlcpMac->MacSend_Cb != NULL && LlcpMac->PeerRemoteDevType == phlnLlcp_Fri_Mac_ePeerTypeInitiator)
    {
        /*Previous callback is pending */
        status = PHLNLLCP_FRI_NFCSTATUS_REJECTED;
    }
    else
    {
        /* Save the LlcpMacSend_Cb */
        LlcpMac->MacSend_Cb = LlcpMacSend_Cb;
        LlcpMac->MacSend_Context = pContext;

        switch(LlcpMac->PeerRemoteDevType)
        {
        case phlnLlcp_Fri_Mac_ePeerTypeInitiator:
            {
                if(LlcpMac->RecvPending)
                {
                    /*set the completion routines for the LLCP Transceive function*/
                    LlcpMac->MacCompletionInfo.CompletionRoutine = &phlnLlcp_Fri_MacNfcip_Transceive_Cb;
                    LlcpMac->MacCompletionInfo.Context = LlcpMac;
                    LlcpMac->SendPending = true;

                    status = phlnLlcp_Fri_OvrHal_Transceive(LlcpMac->LowerDevice,
                        &LlcpMac->MacCompletionInfo,
                        psData->buffer,
                        (uint16_t)psData->length,
                        &(LlcpMac->psReceiveBuffer->buffer),
                        (uint16_t*)&LlcpMac->psReceiveBuffer->length);
                }
                else
                {
                    LlcpMac->SendPending = true;
                    LlcpMac->psSendBuffer = psData;
                    return status = PH_ERR_PENDING;
                }
            }break;
        case phlnLlcp_Fri_Mac_ePeerTypeTarget:
            {
                if(!LlcpMac->RecvPending)
                {
                    LlcpMac->SendPending = true;
                    LlcpMac->psSendBuffer = psData;
                    return status = PH_ERR_PENDING;
                }
                else
                {
                    /*set the completion routines for the LLCP Send function*/
                    LlcpMac->MacCompletionInfo.CompletionRoutine = &phlnLlcp_Fri_MacNfcip_Send_Cb;
                    LlcpMac->MacCompletionInfo.Context = LlcpMac;
                    status = phlnLlcp_Fri_OvrHal_Send(LlcpMac->LowerDevice,
                        &LlcpMac->MacCompletionInfo,
                        psData->buffer,
                        (uint16_t)psData->length);
                }
            }break;
        default:
            {
                status = PH_ADD_COMPCODE(PH_ERR_INVALID_DEVICE, CID_FRI_NFC_LLCP_MAC);
            }break;
        }
    }
    return status;
}

static phStatus_t phlnLlcp_Fri_MacNfcip_Receive(phlnLlcp_Fri_Mac_t               *LlcpMac,
                                                phNfc_sData_t                    *psData,
                                                phlnLlcp_Fri_Mac_Reveive_CB_t     LlcpMacReceive_Cb,
                                                void                             *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    if(NULL == LlcpMac || NULL==psData || NULL == LlcpMacReceive_Cb || NULL == pContext)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        /* Save the LlcpMacReceive_Cb */
        LlcpMac->MacReceive_Cb = LlcpMacReceive_Cb;
        LlcpMac->MacReceive_Context = pContext;

        /* Save the pointer to the receive buffer */
        LlcpMac->psReceiveBuffer= psData;

        switch(LlcpMac->PeerRemoteDevType)
        {
        case phlnLlcp_Fri_Mac_ePeerTypeInitiator:
            {
                if(LlcpMac->SendPending)
                {
                    /*set the completion routines for the LLCP Transceive function*/
                    LlcpMac->MacCompletionInfo.CompletionRoutine = &phlnLlcp_Fri_MacNfcip_Transceive_Cb;
                    LlcpMac->MacCompletionInfo.Context = LlcpMac;
                    LlcpMac->RecvPending = true;

                    status = phlnLlcp_Fri_OvrHal_Transceive(LlcpMac->LowerDevice,
                                                       &LlcpMac->MacCompletionInfo,
                                                       LlcpMac->psSendBuffer->buffer,
                                                       (uint16_t)LlcpMac->psSendBuffer->length,
                                                       &(psData->buffer),
                                                       (uint16_t*)&psData->length);
                }
                else
                {
                    LlcpMac->RecvPending = true;
                    return status = PH_ERR_PENDING;
                }
            }
            break;
        case phlnLlcp_Fri_Mac_ePeerTypeTarget:
            {
                /*set the completion routines for the LLCP Receive function*/
                LlcpMac->MacCompletionInfo.CompletionRoutine = &phlnLlcp_Fri_MacNfcip_Receive_Cb;
                /* save the context of LlcpMacNfcip */
                LlcpMac->MacCompletionInfo.Context = LlcpMac;
                LlcpMac->RecvPending = true;

                status = phlnLlcp_Fri_OvrHal_Receive(LlcpMac->LowerDevice,
                                             &LlcpMac->MacCompletionInfo,
                                             &(LlcpMac->psReceiveBuffer->buffer),
                                             (uint16_t*)&LlcpMac->psReceiveBuffer->length);
            }break;
        default:
            {
                status = PH_ADD_COMPCODE(PH_ERR_INVALID_DEVICE, CID_FRI_NFC_LLCP_MAC);
            }break;
        }
    }
    return status;
}


phStatus_t phlnLlcp_Fri_MacNfcip_Register (phlnLlcp_Fri_Mac_t *LlcpMac)
{
    phStatus_t status = PH_ERR_SUCCESS;

    if(NULL != LlcpMac)
    {
        LlcpMac->LlcpMacInterface.chk        = &phlnLlcp_Fri_MacNfcip_Chk;
        LlcpMac->LlcpMacInterface.activate   = &phlnLlcp_Fri_MacNfcip_Activate;
        LlcpMac->LlcpMacInterface.deactivate = &phlnLlcp_Fri_MacNfcip_Deactivate;
        LlcpMac->LlcpMacInterface.send       = &phlnLlcp_Fri_MacNfcip_Send;
        LlcpMac->LlcpMacInterface.receive    = &phlnLlcp_Fri_MacNfcip_Receive;

        return PH_ERR_SUCCESS;
    }
    else
    {
        return status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_MAC);
    }
}

#endif /* NXPBUILD__PHLN_LLCP_FRI */
