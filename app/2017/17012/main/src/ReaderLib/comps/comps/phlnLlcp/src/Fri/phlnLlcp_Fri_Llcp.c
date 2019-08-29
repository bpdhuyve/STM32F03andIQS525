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
* \file  phlnLlcp_Fri_Llcp.c
* \brief NFC LLCP core
* $Author: nxp62726 $
* $Revision: 501 $
* $Date: 2014-02-13 12:24:52 +0530 (Thu, 13 Feb 2014) $
*
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI
/*include files*/
#include <phlnLlcp.h>
#include <phOsal.h>
#include <ph_RefDefs.h>
#include "phlnLlcp_Fri_Llcp.h"
#include "phlnLlcp_Fri_Utils.h"


/* ------------------------ Internal functions forward declaration--------------------- */

static void phlnLlcp_Fri_Receive_CB( void                     *pContext,
                                    phStatus_t                 status,
                                    phNfc_sData_t             *psData);

static phStatus_t phlnLlcp_Fri_HandleIncomingPacket( phlnLlcp_Fri_t    *Llcp,
                                                    phNfc_sData_t      *psPacket );

static void phlnLlcp_Fri_ResetLTO( phlnLlcp_Fri_t *Llcp );

static phStatus_t phlnLlcp_Fri_InternalSend( phlnLlcp_Fri_t                   *Llcp,
                                            phlnLlcp_Fri_sPacketHeader_t      *psHeader,
                                            phlnLlcp_Fri_sPacketSequence_t    *psSequence,
                                            phNfc_sData_t                     *psInfo );

static uint8_t phlnLlcp_Fri_HandlePendingSend ( phlnLlcp_Fri_t *Llcp );

/* ----------------------- Internal functions Definitions -------------------------- */

static void phlnLlcp_Fri_Deallocate(void             *pOsal,
                                    phNfc_sData_t    *pData)
{
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    if (pData != NULL)
    {
        if (pData->buffer != NULL)
        {
            status = phOsal_FreeMemory(pOsal, pData->buffer);
        }
        status = phOsal_FreeMemory(pOsal, pData);
    }
}

static phStatus_t phlnLlcp_Fri_InternalDeactivate( phlnLlcp_Fri_t    *Llcp )   /* PRQA S 3208 */
{
    phlnLlcp_Fri_Send_CB_t pfSendCB;
    void  *pSendContext;
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    if ((Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_RECV)  ||
        (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_SEND)  ||
        (Llcp->state == PHLNLLCP_FRI_STATE_PAX)             ||
        (Llcp->state == PHLNLLCP_FRI_STATE_ACTIVATION))
    {
        /* Update state */
        Llcp->state = PHLNLLCP_FRI_STATE_DEACTIVATION;

        /* Stop timer */
        status = phOsal_Timer_Stop(Llcp->osal, Llcp->hSymmTimer);

        /* Return delayed send operation in error, in any */
        if (Llcp->psSendInfo != NULL)
        {
            Llcp->psSendHeader = NULL;
            Llcp->psSendSequence = NULL;
        }
        if (Llcp->pfSendCB != NULL)
        {
            /* Get Callback params */
            pfSendCB = Llcp->pfSendCB;
            pSendContext = Llcp->pSendContext;
            /* Reset callback params */
            Llcp->pfSendCB = NULL;
            Llcp->pSendContext = NULL;
            /* Call the callback */
            (pfSendCB)(pSendContext, PH_ERR_FAILED);
        }

        /* Notify service layer */
        Llcp->pfLink_CB(Llcp->pLinkContext, phlnLlcp_Fri_Mac_eLinkDeactivated);

        /* Forward check request to MAC layer */
        return phlnLlcp_Fri_Mac_Deactivate(&Llcp->MAC);
    }

    return PH_ERR_SUCCESS;
}


static phStatus_t phlnLlcp_Fri_SendSymm( phlnLlcp_Fri_t  *Llcp )     /* PRQA S 3209 */
{
    phlnLlcp_Fri_sPacketHeader_t   sHeader;
    uint8_t                        bPendingFlag;
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for pending messages to send */
    bPendingFlag = phlnLlcp_Fri_HandlePendingSend(Llcp);

    if (bPendingFlag == false)
    {
        /* Update activity monitor */
        if (Llcp->nSymmetryCounter < PHLNLLCP_FRI_SLOW_SYMMETRY)
        {
            Llcp->nSymmetryCounter++;
        }
        /* No send pending, send a SYMM instead */
        sHeader.dsap  = PHLNLLCP_FRI_SAP_LINK;
        sHeader.ssap  = PHLNLLCP_FRI_SAP_LINK;
        sHeader.ptype = PHLNLLCP_FRI_PTYPE_SYMM;
        status = phlnLlcp_Fri_InternalSend(Llcp, &sHeader, NULL, NULL);

        /* Incase of SNEP Working as a server*/
        if (Llcp->eRole != phlnLlcp_Fri_Mac_ePeerTypeInitiator)
        {
            /* Restart the receive incase of SYM send */
            status = phlnLlcp_Fri_Mac_Receive(&Llcp->MAC, &Llcp->sRxBuffer, &phlnLlcp_Fri_Receive_CB, Llcp);
        }
        return status;

    }
    else
    {
        /* A pending send has been sent, there is no need to send SYMM */
        return PH_ERR_SUCCESS;
    }
}


static phStatus_t phlnLlcp_Fri_SendPax( phlnLlcp_Fri_t                  *Llcp,
                                       phlnLlcp_Fri_sLinkParameters_t   *psLinkParams)
{
    uint8_t                       pTLVBuffer[PHLNLLCP_FRI_MAX_PARAM_TLV_LENGTH];
    phNfc_sData_t                 sParamsTLV;
    phlnLlcp_Fri_sPacketHeader_t  sHeader;
    phStatus_t                    result;

    /* Prepare link parameters TLV */
    sParamsTLV.buffer = pTLVBuffer;
    sParamsTLV.length = PHLNLLCP_FRI_MAX_PARAM_TLV_LENGTH;
    result = phlnLlcp_Fri_EncodeLinkParams(&sParamsTLV, psLinkParams, PHLNLLCP_FRI_VERSION);

    if (result != PH_ERR_SUCCESS)
    {
        /* Error while encoding */
        return PH_ERR_FAILED;
    }

    /* Check if ready to send */
    if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_SEND)
    {
        /* No send pending, send the PAX packet */
        sHeader.dsap  = PHLNLLCP_FRI_SAP_LINK;
        sHeader.ssap  = PHLNLLCP_FRI_SAP_LINK;
        sHeader.ptype = PHLNLLCP_FRI_PTYPE_PAX;
        return phlnLlcp_Fri_InternalSend(Llcp, &sHeader, NULL, &sParamsTLV);
    }
    else
    {
        /* Error: A send is already pending, cannot send PAX */
        /* NOTE: this should not happen since PAX are sent before any other packet ! */
        return PH_ERR_FAILED;
    }
}

static void phlnLlcp_Fri_Timer_CBRecv(uint32_t TimerId, void *pContext)
{
    phlnLlcp_Fri_t               *Llcp = (phlnLlcp_Fri_t*)pContext;
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    PHLNLLCP_FRI_UNUSED_VARIABLE(TimerId);

    /* Check current state */
    if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_RECV)
    {
        /* No data is coming before LTO, disconnecting */
        //phlnLlcp_Fri_InternalDeactivate(Llcp);  /* TODO:  */
    }
    else if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_SEND)
    {
        /* Send SYMM */
        status = phlnLlcp_Fri_SendSymm(Llcp);
    }
    else
    {
        /* Nothing to do if not in Normal Operation state */
    }
}

static void phlnLlcp_Fri_Timer_CBSend(uint32_t TimerId, void *pContext)
{
    phlnLlcp_Fri_t   *Llcp = (phlnLlcp_Fri_t*)pContext;
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    PHLNLLCP_FRI_UNUSED_VARIABLE(TimerId);

    if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_SEND)
    {
        /* Send SYMM */
        status = phlnLlcp_Fri_SendSymm(Llcp);
    }
    else
    {
        /* Nothing to do if not in Normal Operation state */
    }
}

static phStatus_t phlnLlcp_Fri_HandleAggregatedPacket( phlnLlcp_Fri_t  *Llcp,
                                                      phNfc_sData_t    *psRawPacket )
{
    phNfc_sData_t   sInfo;
    phNfc_sData_t   sCurrentInfo;
    uint16_t        length;
    phStatus_t      status;

    /* Get info field */
    sInfo.buffer = psRawPacket->buffer + PHLNLLCP_FRI_PACKET_HEADER_SIZE;
    sInfo.length = psRawPacket->length - PHLNLLCP_FRI_PACKET_HEADER_SIZE;
    length = (uint16_t) sInfo.length;

    /* Check for empty info field */
    if (length == 0)
    {
        return PH_ERR_FAILED;
    }

    /* Check consistency */
    while (sInfo.length != 0)
    {
        /* Check if enough room to read length */
        if (sInfo.length < sizeof(sInfo.length))
        {
            return PH_ERR_FAILED;
        }
        /* Read length */
        length = ((uint16_t)sInfo.buffer[0] << 8) | sInfo.buffer[1];

        /* Update info buffer */
        sInfo.buffer += 2; /*Size of length field is 2*/
        sInfo.length -= 2; /*Size of length field is 2*/

        /* Check if declared length fits in remaining space */
        if (length > sInfo.length)
        {
            return PH_ERR_FAILED;
        }
        /* Update info buffer */
        sInfo.buffer += length;
        sInfo.length -= length;
    }

    /* Get info field */
    sInfo.buffer = psRawPacket->buffer + PHLNLLCP_FRI_PACKET_HEADER_SIZE;
    sInfo.length = psRawPacket->length - PHLNLLCP_FRI_PACKET_HEADER_SIZE;

    /* Handle aggregated packets */
    while (sInfo.length != 0)
    {
        /* Read length */
        length = ((uint16_t)sInfo.buffer[0] << 8) | sInfo.buffer[1];

        /* Update info buffer */
        sInfo.buffer += 2;        /* Size of length field is 2 */
        sInfo.length -= 2;        /*Size of length field is 2*/

        /* Handle aggregated packet */
        sCurrentInfo.buffer=sInfo.buffer;
        sCurrentInfo.length=length;

        status = phlnLlcp_Fri_HandleIncomingPacket(Llcp, &sCurrentInfo);
        if ( (status != PH_ERR_SUCCESS) &&
            (status != PH_ERR_PENDING) )
        {
            /* TODO: Error: invalid frame */
        }
        /* Update info buffer */
        sInfo.buffer += length;
        sInfo.length -= length;
    }
    return PH_ERR_SUCCESS;
}


static phStatus_t phlnLlcp_Fri_ParseLinkParams( phNfc_sData_t                   *psParamsTLV,
                                               phlnLlcp_Fri_sLinkParameters_t   *psParsedParams,
                                               uint8_t                          *pnParsedVersion )
{
    phStatus_t                       status;
    uint8_t                          type;
    phlnLlcp_Fri_sLinkParameters_t   sParams;
    phNfc_sData_t                    sValueBuffer;
    uint32_t                         offset = 0;
    uint8_t                          version = PHLNLLCP_FRI_INVALID_VERSION;

    /* Check for NULL pointers */
    if ((psParamsTLV == NULL) || (psParsedParams == NULL) || (pnParsedVersion == NULL))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Prepare default param structure */
    sParams.miu    = PHLNLLCP_FRI_MIU_DEFAULT;
    sParams.wks    = PHLNLLCP_FRI_WKS_DEFAULT;
    sParams.lto    = PHLNLLCP_FRI_LTO_DEFAULT;
    sParams.option = PHLNLLCP_FRI_OPTION_DEFAULT;

    /* Decode TLV */
    while (offset < psParamsTLV->length)
    {
        status = phlnLlcp_Fri_Utils_DecodeTLV(psParamsTLV, &offset, &type, &sValueBuffer);
        if (status != PH_ERR_SUCCESS)
        {
            /* Error: Ill-formed TLV */
            return status;
        }
        switch(type)
        {
            case PHLNLLCP_FRI_TLV_TYPE_VERSION:
            {
                /* Check length */
                if (sValueBuffer.length != PHLNLLCP_FRI_TLV_LENGTH_VERSION)
                {
                    /* Error : Ill-formed VERSION parameter TLV */
                    break;
                }
                /* Get VERSION */
                version = sValueBuffer.buffer[0];
                break;
            }
            case PHLNLLCP_FRI_TLV_TYPE_MIUX:
            {
                /* Check length */
                if (sValueBuffer.length != PHLNLLCP_FRI_TLV_LENGTH_MIUX)
                {
                    /* Error : Ill-formed MIUX parameter TLV */
                    break;
                }
                /* Get MIU */
                sParams.miu = PHLNLLCP_FRI_MIU_DEFAULT + ((((uint16_t) sValueBuffer.buffer[0] << 8) | sValueBuffer.buffer[1]) & PHLNLLCP_FRI_TLV_MIUX_MASK);
                break;
            }
            case PHLNLLCP_FRI_TLV_TYPE_WKS:
            {
                /* Check length */
                if (sValueBuffer.length != PHLNLLCP_FRI_TLV_LENGTH_WKS)
                {
                    /* Error : Ill-formed MIUX parameter TLV */
                    break;
                }
                /* Get WKS */
                sParams.wks = ((uint16_t)sValueBuffer.buffer[0] << 8) | sValueBuffer.buffer[1];
                /* Ignored bits must always be set */
                sParams.wks |= PHLNLLCP_FRI_TLV_WKS_MASK;
                break;
            }
            case PHLNLLCP_FRI_TLV_TYPE_LTO:
            {
                /* Check length */
                if (sValueBuffer.length != PHLNLLCP_FRI_TLV_LENGTH_LTO)
                {
                    /* Error : Ill-formed LTO parameter TLV */
                    break;
                }
                /* Get LTO */
                sParams.lto = sValueBuffer.buffer[0];
                break;
            }
            case PHLNLLCP_FRI_TLV_TYPE_OPT:
            {
                /* Check length */
                if (sValueBuffer.length != PHLNLLCP_FRI_TLV_LENGTH_OPT)
                {
                    /* Error : Ill-formed OPT parameter TLV */
                    break;
                }
                /* Get OPT */
                sParams.option = sValueBuffer.buffer[0] & PHLNLLCP_FRI_TLV_OPT_MASK;
                break;
            }
            default:
            {
                /* Error : Unknown Type */
                break;
            }
        }
    }

    /* Check if a VERSION parameter has been provided */
    if (version == PHLNLLCP_FRI_INVALID_VERSION)
    {
        /* Error : Mandatory VERSION parameter not provided */
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Save response */
    *pnParsedVersion = version;
    memcpy(psParsedParams, &sParams, sizeof(phlnLlcp_Fri_sLinkParameters_t));  /* PRQA S 3200 */

    return PH_ERR_SUCCESS;
}


static phStatus_t phlnLlcp_Fri_VersionAgreement( uint8_t    localVersion,
                                                uint8_t     remoteVersion,
                                                uint8_t    *pNegociatedVersion )
{
    uint8_t     localMajor  = localVersion  & PHLNLLCP_FRI_VERSION_MAJOR_MASK;
    uint8_t     localMinor  = localVersion  & PHLNLLCP_FRI_VERSION_MINOR_MASK;
    uint8_t     remoteMajor = remoteVersion & PHLNLLCP_FRI_VERSION_MAJOR_MASK;
    uint8_t     remoteMinor = remoteVersion & PHLNLLCP_FRI_VERSION_MINOR_MASK;
    uint8_t     negociatedVersion;

    /* Check for NULL pointers */
    if (pNegociatedVersion == NULL)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Compare Major numbers */
    if (localMajor == remoteMajor)
    {
        /* Version agreement succeed : use lowest version */
        negociatedVersion = localMajor | ((remoteMinor<localMinor)?remoteMinor:localMinor);
    }
    else if (localMajor > remoteMajor)
    {
        /* Decide if versions are compatible */
        /* Currently, there is no backward compatibility to handle */
        return PH_ERR_FAILED;
    }
    else /* if (localMajor < remoteMajor) */
    {
        /* It is up to the remote host to decide if versions are compatible */
        /* Set negociated version to our local version, the remote will
        deacivate the link if its own version agreement fails */
        negociatedVersion = localVersion;
    }

    /* Save response */
    *pNegociatedVersion = negociatedVersion;

    return PH_ERR_SUCCESS;
}


static phStatus_t phlnLlcp_Fri_InternalActivate( phlnLlcp_Fri_t  *Llcp,
                                                phNfc_sData_t    *psParamsTLV)
{
    phStatus_t                        status;
    phlnLlcp_Fri_sLinkParameters_t   sRemoteParams;
    uint8_t                          remoteVersion;
    uint8_t                          negociatedVersion;
    const uint16_t nMaxHeaderSize =  PHLNLLCP_FRI_PACKET_HEADER_SIZE + PHLNLLCP_FRI_PACKET_SEQUENCE_SIZE;

    /* Parse parameters  */
    status = phlnLlcp_Fri_ParseLinkParams(psParamsTLV, &sRemoteParams, &remoteVersion);
    if (status != PH_ERR_SUCCESS)
    {
        /* Error: invalid parameters TLV */
        status = PH_ERR_FAILED;
    }
    else
    {
        /* Version agreement procedure */
        status = phlnLlcp_Fri_VersionAgreement(PHLNLLCP_FRI_VERSION , remoteVersion, &negociatedVersion);
        if (status != PH_ERR_SUCCESS)
        {
            /* Error: version agreement failed */
            status = PH_ERR_FAILED;
        }
        else
        {
            /* Save parameters */
            Llcp->version = negociatedVersion;
            memcpy(&Llcp->sRemoteParams, &sRemoteParams, sizeof(phlnLlcp_Fri_sLinkParameters_t));  /* PRQA S 3200 */

            /* Update remote MIU to match local Tx buffer size */
            if (Llcp->nTxBufferLength < (Llcp->sRemoteParams.miu + nMaxHeaderSize))
            {
                Llcp->sRemoteParams.miu = Llcp->nTxBufferLength - nMaxHeaderSize;
            }

            /* Initiate Symmetry procedure by resetting LTO timer */
            /* NOTE: this also updates current state */
            phlnLlcp_Fri_ResetLTO(Llcp);
        }
    }

    /* Notify upper layer, if Activation failed CB called by Deactivate */
    if (status == PH_ERR_SUCCESS)
    {
        /* Link activated ! */
        Llcp->pfLink_CB(Llcp->pLinkContext, phlnLlcp_Fri_Mac_eLinkActivated);
    }

    return status;
}


static phStatus_t phlnLlcp_Fri_HandleMACLinkActivated( phlnLlcp_Fri_t   *Llcp,
                                                      phNfc_sData_t     *psParamsTLV)
{
    phStatus_t                     status = PH_ERR_SUCCESS;
    phStatus_t                       temp = PH_ERR_SUCCESS;

    PH_UNUSED_VARIABLE(temp);

    /* Create the timer */
    status = phOsal_Timer_Create(Llcp->osal, &Llcp->hSymmTimer);
    if (Llcp->hSymmTimer == PH_OSAL_INVALID_TIMER_ID)
    {
        /* Error: unable to create timer */
        return PH_ERR_INSUFFICIENT_RESOURCES;
    }

    /* Check if params received from MAC activation procedure */
    if (psParamsTLV == NULL)
    {
        /* No params with selected MAC mapping, enter PAX mode for parameter exchange */
        Llcp->state = PHLNLLCP_FRI_STATE_PAX;
        /* Set default MIU for PAX exchange */
        Llcp->sRemoteParams.miu = PHLNLLCP_FRI_MIU_DEFAULT;
        /* If the local device is the initiator, it must initiate PAX exchange */
        if (Llcp->eRole == phlnLlcp_Fri_Mac_ePeerTypeInitiator)
        {
            /* Send PAX */
            status = phlnLlcp_Fri_SendPax(Llcp, &Llcp->sLocalParams);
        }
    }
    else
    {
        /* Params exchanged during MAX activation, try LLC activation */
        status = phlnLlcp_Fri_InternalActivate(Llcp, psParamsTLV);
    }

    if (status == PH_ERR_SUCCESS)
    {
        /* Start listening for incoming packets */
        Llcp->sRxBuffer.length = Llcp->nRxBufferLength;
        temp = phlnLlcp_Fri_Mac_Receive(&Llcp->MAC, &Llcp->sRxBuffer, &phlnLlcp_Fri_Receive_CB, Llcp);
    }

    return status;
}


static void phlnLlcp_Fri_HandleMACLinkDeactivated( phlnLlcp_Fri_t    *Llcp )
{
    uint8_t state = Llcp->state;
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    /* Delete the timer */
    if (Llcp->hSymmTimer != PH_OSAL_INVALID_TIMER_ID)
    {
        status = phOsal_Timer_Delete(Llcp->osal, Llcp->hSymmTimer);
    }

    /* Reset state */
    Llcp->state = PHLNLLCP_FRI_STATE_RESET_INIT;

    switch (state)
    {
    case PHLNLLCP_FRI_STATE_DEACTIVATION:
        {
            /* The service layer has already been notified, nothing more to do */
            break;
        }
    default:
        {
            /* Notify service layer of link failure */
            Llcp->pfLink_CB(Llcp->pLinkContext, phlnLlcp_Fri_Mac_eLinkDeactivated);
            break;
        }
    }
}


static void phlnLlcp_Fri_ChkLlcp_CB( void       *pContext,
                                    phStatus_t   status )
{
    /* Get monitor from context */
    phlnLlcp_Fri_t *Llcp = (phlnLlcp_Fri_t*)pContext;

    /* Update state */
    Llcp->state = PHLNLLCP_FRI_STATE_CHECKED;

    /* Invoke callback */
    Llcp->pfChk_CB(Llcp->pChkContext, status);
}

static void phlnLlcp_Fri_LinkStatus_CB( void                            *pContext,
                                       phlnLlcp_Fri_Mac_eLinkStatus_t    eLinkStatus,
                                       phNfc_sData_t                    *psParamsTLV,
                                       phlnLlcp_Fri_Mac_ePeerType_t      PeerRemoteDevType)
{
    phStatus_t status;

    /* Get monitor from context */
    phlnLlcp_Fri_t *Llcp = (phlnLlcp_Fri_t*)pContext;

    /* Save the local peer role (initiator/target) */
    Llcp->eRole = PeerRemoteDevType;

    /* Check new link status */
    switch(eLinkStatus)
    {
        case phlnLlcp_Fri_Mac_eLinkActivated:
        {
            /* Handle MAC link activation */
            status = phlnLlcp_Fri_HandleMACLinkActivated(Llcp, psParamsTLV);
            if (status != PH_ERR_SUCCESS)
            {
                /* Error: LLC link activation procedure failed, deactivate MAC link */
                status = phlnLlcp_Fri_InternalDeactivate(Llcp);
            }
            break;
        }
        case phlnLlcp_Fri_Mac_eLinkDeactivated:
        {
            /* Handle MAC link deactivation (cannot fail) */
            phlnLlcp_Fri_HandleMACLinkDeactivated(Llcp);
            break;
        }
        default:
        {
            /* Warning: Unknown link status, should not happen */
        }
    }
}


static void phlnLlcp_Fri_ResetLTO( phlnLlcp_Fri_t  *Llcp )
{
    uint32_t nDuration;
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    /* Stop timer */
    status = phOsal_Timer_Stop(Llcp->osal, Llcp->hSymmTimer);

    status = phOsal_Timer_Reset( Llcp->osal, Llcp->hSymmTimer);

    /* Update state */
    if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_RECV)
    {
        Llcp->state = PHLNLLCP_FRI_STATE_OPERATION_SEND;
    }
    else if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_SEND)
    {
        Llcp->state = PHLNLLCP_FRI_STATE_OPERATION_RECV;
    }
    else if (Llcp->state != PHLNLLCP_FRI_STATE_DEACTIVATION &&
            Llcp->state != PHLNLLCP_FRI_STATE_RESET_INIT)
    {
        /* Not yet in OPERATION state, perform first reset */
        if (Llcp->eRole == phlnLlcp_Fri_Mac_ePeerTypeInitiator)
        {
            Llcp->state = PHLNLLCP_FRI_STATE_OPERATION_SEND;
        }
        else
        {
            Llcp->state = PHLNLLCP_FRI_STATE_OPERATION_RECV;
        }
    }
    else
    {
        /* Nothing to Do */
    }
    /* Calculate timer duration */
    /* NOTE: nDuration is in 1/100s, and timer system takes values in 1/1000s */
    if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_RECV)
    {
        /* Response must be received before LTO announced by remote peer */
        nDuration = Llcp->sRemoteParams.lto * 10;
        /* Restart timer */
        status = phOsal_Timer_Start(Llcp->osal,Llcp->hSymmTimer,nDuration, PH_OSAL_TIMER_UNIT_MS,&phlnLlcp_Fri_Timer_CBRecv, Llcp);
    }
    else
    {
        /* Must answer before the local announced LTO */
        /* NOTE: to ensure the answer is completely sent before LTO, the
        timer is triggered _before_ LTO expiration */
        /* TODO: make sure time scope is enough, and avoid use of magic number */
        nDuration = (Llcp->sLocalParams.lto * 10) / (20 * 3.25); /* nDuration = 15ms; (20 * 3.25) division  */
        /* Reduce timeout when there is activity */
        /* Restart timer */
        status = phOsal_Timer_Start(Llcp->osal,Llcp->hSymmTimer,nDuration, PH_OSAL_TIMER_UNIT_MS,&phlnLlcp_Fri_Timer_CBSend,Llcp);
    }
}


static phStatus_t phlnLlcp_Fri_HandleLinkPacket( phlnLlcp_Fri_t   *Llcp,
                                                phNfc_sData_t     *psPacket )
{
    phStatus_t  result = PH_ERR_SUCCESS;
    phlnLlcp_Fri_sPacketHeader_t     sHeader;
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    /* Parse header */
    status = (uint16_t) phlnLlcp_Fri_Utils_Buffer2Header(psPacket->buffer, 0, &sHeader);

    /* Check packet type */
    switch (sHeader.ptype)
    {
        case PHLNLLCP_FRI_PTYPE_SYMM:
        {
            /* Update activity monitor */
            if (Llcp->nSymmetryCounter < PHLNLLCP_FRI_SLOW_SYMMETRY)
            {
                Llcp->nSymmetryCounter++;
            }
            /* Nothing to do, the LTO is handled upon all packet reception */
            result = PH_ERR_SUCCESS;
            break;
        }
        case PHLNLLCP_FRI_PTYPE_AGF:
        {
            /* Handle the aggregated packet */
            result = phlnLlcp_Fri_HandleAggregatedPacket(Llcp, psPacket);
            if (result != PH_ERR_SUCCESS)
            {
                /* Error: invalid info field, dropping frame */
            }
            break;
        }
        case PHLNLLCP_FRI_PTYPE_DISC:
        {
            /* Handle link disconnection request */
            result = phlnLlcp_Fri_InternalDeactivate(Llcp);
            break;
        }
        case PHLNLLCP_FRI_PTYPE_FRMR:
        {
            /* TODO: what to do upon reception of FRMR on Link SAP ? */
            result = PH_ERR_SUCCESS;
            break;
        }
        case PHLNLLCP_FRI_PTYPE_PAX:
        {
            /* Ignore PAX when in Normal Operation */
            result = PH_ERR_SUCCESS;
            break;
        }
        default:
        {
            /* Error: invalid ptype field, dropping packet */
            break;
        }
    }

    return result;
}


static phStatus_t phlnLlcp_Fri_HandleTransportPacket( phlnLlcp_Fri_t    *Llcp,
                                                     phNfc_sData_t      *psPacket )
{
    phlnLlcp_Fri_Recv_CB_t         pfRecvCB;
    void                          *pContext;
    phStatus_t                     result = PH_ERR_SUCCESS;

    /* Forward to upper layer */
    if (Llcp->pRecvContext/*pfRecvCB*/ != NULL)
    {
        /* Get callback details */
        pfRecvCB = Llcp->pfRecvCB;
        pContext = Llcp->pRecvContext;
        /* Reset callback details */
        Llcp->pfRecvCB = NULL;
        Llcp->pRecvContext = NULL;
        /* Call the callback */
        (pfRecvCB)(pContext, psPacket, PH_ERR_SUCCESS);
    }

    return result;
}


static uint8_t phlnLlcp_Fri_HandlePendingSend ( phlnLlcp_Fri_t   *Llcp )  /* PRQA S 3208 */
{
    phlnLlcp_Fri_sPacketHeader_t      sHeader;
    phNfc_sData_t                     sInfoBuffer;
    phlnLlcp_Fri_sPacketHeader_t     *psSendHeader = NULL;
    phlnLlcp_Fri_sPacketSequence_t   *psSendSequence = NULL;
    phNfc_sData_t                    *psSendInfo = NULL;
    phStatus_t                        result;
    uint8_t                           bDeallocate = false;
    uint8_t                           return_value = false;
    phStatus_t        status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    /* Handle pending disconnection request */
    if (Llcp->bDiscPendingFlag == true)
    {
        /* Last send si acheived, send the pending DISC packet */
        sHeader.dsap  = PHLNLLCP_FRI_SAP_LINK;
        sHeader.ssap  = PHLNLLCP_FRI_SAP_LINK;
        sHeader.ptype = PHLNLLCP_FRI_PTYPE_DISC;
        /* Set send params */
        psSendHeader = &sHeader;
        /* Reset flag */
        Llcp->bDiscPendingFlag = false;
    }
    /* Handle pending frame reject request */
    else if (Llcp->bFrmrPendingFlag == true)
    {
        /* Last send si acheived, send the pending FRMR packet */
        sInfoBuffer.buffer = Llcp->pFrmrInfo;
        sInfoBuffer.length = sizeof(Llcp->pFrmrInfo);
        /* Set send params */
        psSendHeader = &Llcp->sFrmrHeader;
        psSendInfo   = &sInfoBuffer;
        /* Reset flag */
        Llcp->bFrmrPendingFlag = false;
    }
    /* Handle pending service frame */
    else if (Llcp->pfSendCB != NULL)
    {
        /* Set send params */
        psSendHeader = Llcp->psSendHeader;
        psSendSequence = Llcp->psSendSequence;
        psSendInfo = Llcp->psSendInfo;
        /* Reset pending send infos */
        Llcp->psSendHeader = NULL;
        Llcp->psSendSequence = NULL;
        bDeallocate = true;
    }
    else
    {
        /* Nothing to Do */
    }
    /* Perform send, if needed */
    if (psSendHeader != NULL)
    {
        result = phlnLlcp_Fri_InternalSend(Llcp, psSendHeader, psSendSequence, psSendInfo);
        if ((result != PH_ERR_SUCCESS) && (result != PH_ERR_PENDING))
        {
            /* Error: send failed, impossible to recover */
            status = phlnLlcp_Fri_InternalDeactivate(Llcp);
        }
        return_value = true;
    }
    return return_value;
}

static phStatus_t phlnLlcp_Fri_HandleIncomingPacket( phlnLlcp_Fri_t    *Llcp,
                                                    phNfc_sData_t      *psPacket )
{
    phStatus_t                        status = PH_ERR_SUCCESS;
    phlnLlcp_Fri_sPacketHeader_t     sHeader;

    /* Parse header */
    status =  (uint16_t) phlnLlcp_Fri_Utils_Buffer2Header(psPacket->buffer, 0, &sHeader);

    /* Check destination */
    if (sHeader.dsap == PHLNLLCP_FRI_SAP_LINK)
    {
        /* Handle packet as destinated to the Link SAP */
        status = phlnLlcp_Fri_HandleLinkPacket(Llcp, psPacket);
    }
    else if (sHeader.dsap >= PHLNLLCP_FRI_SAP_NUMBER)
    {
        /* NOTE: this cannot happen since "psHeader->dsap" is only 6-bit wide */
        status = PH_ERR_FAILED;
    }
    else
    {
        /* Reset activity counter */
        Llcp->nSymmetryCounter = 0;
        /* Handle packet as destinated to the SDP and transport SAPs */
        status = phlnLlcp_Fri_HandleTransportPacket(Llcp, psPacket);
    }
    return status;
}

static void phlnLlcp_Fri_Receive_CB( void              *pContext,
                                    phStatus_t          status,
                                    phNfc_sData_t      *psData)
{
    /* Get monitor from context */
    phlnLlcp_Fri_t                *Llcp = (phlnLlcp_Fri_t*)pContext;
    phStatus_t                     result = PH_ERR_SUCCESS;
    phlnLlcp_Fri_sPacketHeader_t  sPacketHeader;

    /* Check reception status and for pending disconnection */
    if ((status != PH_ERR_SUCCESS) || (Llcp->bDiscPendingFlag == true))
    {
        /* Reset disconnection operation */
        Llcp->bDiscPendingFlag = false;
        /* Deactivate the link */
        status = phlnLlcp_Fri_InternalDeactivate(Llcp);
        return;
    }

    /* Parse header */
    status =  (uint16_t) phlnLlcp_Fri_Utils_Buffer2Header(psData->buffer, 0, &sPacketHeader);
    Llcp->sFrmrHeader = sPacketHeader;

    /* Check new link status */
    switch(Llcp->state)
    {
        /* Handle packets in PAX-waiting state */
        case PHLNLLCP_FRI_STATE_PAX:
        {
            /* Check packet type */
            if (sPacketHeader.ptype == PHLNLLCP_FRI_PTYPE_PAX)
            {
                /* Params exchanged during MAC activation, try LLC activation */
                result = phlnLlcp_Fri_InternalActivate(Llcp, psData+PHLNLLCP_FRI_PACKET_HEADER_SIZE);
                /* If the local LLC is the target, it must answer the PAX */
                if (Llcp->eRole == phlnLlcp_Fri_Mac_ePeerTypeTarget)
                {
                    /* Send PAX */
                    result = phlnLlcp_Fri_SendPax(Llcp, &Llcp->sLocalParams);
                }
            }
            else
            {
                /* Warning: Received packet with unhandled type in PAX-waiting state, drop it */
            }
            break;
        }

        /* Bad State */
        case PHLNLLCP_FRI_STATE_OPERATION_SEND:
        /*TODO handle error*/
            break;
    
        /* Handle normal operation packets */
        case PHLNLLCP_FRI_STATE_OPERATION_RECV:
        {
            /* Handle Symmetry procedure by resetting LTO timer */
            phlnLlcp_Fri_ResetLTO(Llcp);
            /* Handle packet */
            result = phlnLlcp_Fri_HandleIncomingPacket(Llcp, psData);
            if ( (result != PH_ERR_SUCCESS) &&
                (result != PH_ERR_PENDING) )
            {
                /* TODO: Error: invalid frame */
            }
            /* Perform pending send request, if any */
            status = phlnLlcp_Fri_HandlePendingSend(Llcp);
            break;
        }

        default:
        {
            /* Warning: Should not receive packets in other states, drop them */
        }
    }

    /* Restart reception */
    Llcp->sRxBuffer.buffer = Llcp->pRxBuffer;
    Llcp->sRxBuffer.length = Llcp->nRxBufferLength;
    status = phlnLlcp_Fri_Mac_Receive(&Llcp->MAC, &Llcp->sRxBuffer, &phlnLlcp_Fri_Receive_CB, Llcp);
}


static void phlnLlcp_Fri_Send_CB( void        *pContext,
                                 phStatus_t    status )
{
    /* Get monitor from context */
    phlnLlcp_Fri_t           *Llcp = (phlnLlcp_Fri_t*)pContext;
    phlnLlcp_Fri_Send_CB_t    pfSendCB;
    void                     *pSendContext;

    /* Call the upper layer callback if last packet sent was  */
    /* NOTE: if Llcp->psSendHeader is not NULL, this means that the send operation is still not initiated */
    if (Llcp->psSendHeader == NULL)
    {
        if (Llcp->pfSendCB != NULL)
        {
            /* Get Callback params */
            pfSendCB = Llcp->pfSendCB;
            pSendContext = Llcp->pSendContext;
            /* Reset callback params */
            Llcp->pfSendCB = NULL;
            Llcp->pSendContext = NULL;
            /* Call the callback */
            (pfSendCB)(pSendContext, status);
        }
    }

    /* Check reception status */
    if (status != PH_ERR_SUCCESS)
    {
        /* Error: Reception failed, link must be down */
        status = phlnLlcp_Fri_InternalDeactivate(Llcp);
    }
}


static phStatus_t phlnLlcp_Fri_InternalSend( phlnLlcp_Fri_t                    *Llcp,
                                            phlnLlcp_Fri_sPacketHeader_t       *psHeader,
                                            phlnLlcp_Fri_sPacketSequence_t     *psSequence,
                                            phNfc_sData_t                      *psInfo )
{
    phStatus_t status;
    phNfc_sData_t  *psRawPacket = &Llcp->sTxBuffer; /* Use internal Tx buffer */

    /* Handle Symmetry procedure */
    phlnLlcp_Fri_ResetLTO(Llcp);

    /* Generate raw packet to send (aggregate header + sequence + info fields) */
    psRawPacket->length = 0;
    psRawPacket->length += phlnLlcp_Fri_Utils_Header2Buffer(psHeader, psRawPacket->buffer, psRawPacket->length);
    if (psSequence != NULL)
    {
        psRawPacket->length += phlnLlcp_Fri_Utils_Sequence2Buffer(psSequence, psRawPacket->buffer, psRawPacket->length);
    }
    if (psInfo != NULL)
    {
        memcpy(psRawPacket->buffer + psRawPacket->length, psInfo->buffer, psInfo->length);  /* PRQA S 3200 */
        psRawPacket->length += psInfo->length;
    }

    /* Send raw packet */
    status = phlnLlcp_Fri_Mac_Send (
        &Llcp->MAC,
        psRawPacket,
        &phlnLlcp_Fri_Send_CB,
        Llcp );

    return status;
}

/* ---------------------------- Public functions ------------------------------- */

phStatus_t phlnLlcp_Fri_Init(
                             phlnLlcp_Fri_DataParams_t        *pDataParams,
                             uint16_t                          wSizeOfDataParams,
                             phlnLlcp_Fri_t                   *pLlcp,
                             phlnLlcp_Fri_sLinkParameters_t   *pLinkParams,
                             phlnLlcp_Fri_Transport_t         *pLlcpSocketTable,
                             phHal_sRemoteDevInformation_t    *pRemoteDevInfo,
                             void                             *pTxBuffer,
                             uint16_t                          wTxBufferLength,
                             void                             *pRxBuffer,
                             uint16_t                          wRxBufferLength,
                             void                             *pLowerDevice)
{
    if (sizeof(phlnLlcp_Fri_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
    }
    if (wTxBufferLength == 0 || wRxBufferLength == 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_LN_LLCP);
    }
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcp);
    PH_ASSERT_NULL (pLinkParams);
    PH_ASSERT_NULL (pLlcpSocketTable);
    PH_ASSERT_NULL (pRemoteDevInfo);
    PH_ASSERT_NULL (pTxBuffer);
    PH_ASSERT_NULL (pRxBuffer);
    PH_ASSERT_NULL (pLowerDevice);

    /* Init. private data */
    pDataParams->wId = PH_COMP_LN_LLCP | PHLN_LLCP_FRI_ID;
    pDataParams->pLlcp = pLlcp;
    pDataParams->pLowerDevice = pLowerDevice;
    pDataParams->pLinkParams = pLinkParams;
    pDataParams->pLlcpSocketTable = pLlcpSocketTable;
    pDataParams->pRemoteDevInfo = pRemoteDevInfo;
    pDataParams->pTxBuffer = pTxBuffer;
    pDataParams->wTxBufferLength = wTxBufferLength;
    pDataParams->wRxBufferLength = wRxBufferLength;
    pDataParams->pRxBuffer = pRxBuffer;
    return PH_ERR_SUCCESS;
}

phStatus_t phlnLlcp_Fri_EncodeLinkParams( phNfc_sData_t                   *psRawBuffer,
                                         phlnLlcp_Fri_sLinkParameters_t   *psLinkParams,
                                         uint8_t                           nVersion )
{
    uint32_t    nOffset = 0;
    uint16_t    miux;
    uint16_t    wks;
    uint8_t     pValue[2];
    phStatus_t   result = PH_ERR_SUCCESS;

    /* Check parameters */
    if ((psRawBuffer == NULL) || (psLinkParams == NULL))
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    /* Encode mandatory VERSION field */

    result = phlnLlcp_Fri_Utils_EncodeTLV(
        psRawBuffer,
        &nOffset,
        PHLNLLCP_FRI_TLV_TYPE_VERSION,
        PHLNLLCP_FRI_TLV_LENGTH_VERSION,
        &nVersion);

    /* Encode mandatory VERSION field */
    if (result == PH_ERR_SUCCESS)
    {
        /* Encode MIUX field, if needed */
        if (psLinkParams->miu != PHLNLLCP_FRI_MIU_DEFAULT)
        {
            miux = (psLinkParams->miu - PHLNLLCP_FRI_MIU_DEFAULT) & PHLNLLCP_FRI_TLV_MIUX_MASK;
            pValue[0] = (uint8_t)((miux >> 8) & 0xFF);
            pValue[1] =  (uint8_t)(miux       & 0xFF);
            result = phlnLlcp_Fri_Utils_EncodeTLV(
                psRawBuffer,
                &nOffset,
                PHLNLLCP_FRI_TLV_TYPE_MIUX,
                PHLNLLCP_FRI_TLV_LENGTH_MIUX,
                pValue);
        }
    }

    /* Encode WKS field */
    if (result == PH_ERR_SUCCESS)
    {
        wks = psLinkParams->wks | PHLNLLCP_FRI_TLV_WKS_MASK;
        pValue[0] = (uint8_t)((wks >> 8) & 0xFF);
        pValue[1] = (uint8_t)(wks       & 0xFF);
        result = phlnLlcp_Fri_Utils_EncodeTLV(
            psRawBuffer,
            &nOffset,
            PHLNLLCP_FRI_TLV_TYPE_WKS,
            PHLNLLCP_FRI_TLV_LENGTH_WKS,
            pValue);
    }

    /* Encode LTO field, if needed */
    if (result == PH_ERR_SUCCESS)
    {
        if (psLinkParams->lto != PHLNLLCP_FRI_LTO_DEFAULT)
        {
            result = phlnLlcp_Fri_Utils_EncodeTLV(
                psRawBuffer,
                &nOffset,
                PHLNLLCP_FRI_TLV_TYPE_LTO,
                PHLNLLCP_FRI_TLV_LENGTH_LTO,
                &psLinkParams->lto);
        }
    }

    /* Encode OPT field, if needed */
    if (result == PH_ERR_SUCCESS)
    {
        if (psLinkParams->option != PHLNLLCP_FRI_OPTION_DEFAULT)
        {
            result = phlnLlcp_Fri_Utils_EncodeTLV(
                psRawBuffer,
                &nOffset,
                PHLNLLCP_FRI_TLV_TYPE_OPT,
                PHLNLLCP_FRI_TLV_LENGTH_OPT,
                &psLinkParams->option);
        }
    }

    if (result != PH_ERR_SUCCESS)
    {
        /* Error: failed to encode TLV */
        return PH_ERR_FAILED;
    }

    /* Save new buffer size */
    psRawBuffer->length = nOffset;

    return result;
}


phStatus_t phlnLlcp_Fri_Reset( phlnLlcp_Fri_t                 *Llcp,
                              void                            *LowerDevice,
                              phlnLlcp_Fri_sLinkParameters_t  *psLinkParams,
                              void                            *pRxBuffer,
                              uint16_t                         nRxBufferLength,
                              void                            *pTxBuffer,
                              uint16_t                         nTxBufferLength,
                              phlnLlcp_Fri_LinkStatus_CB_t     pfLink_CB,
                              void                            *pContext )
{
    const uint16_t nMaxHeaderSize =  PHLNLLCP_FRI_PACKET_HEADER_SIZE + PHLNLLCP_FRI_PACKET_SEQUENCE_SIZE;
    phStatus_t result;

    /* Check parameters presence */
    if ((Llcp == NULL) || (LowerDevice == NULL) || (pfLink_CB == NULL) ||
        (pRxBuffer == NULL) || (pTxBuffer == NULL) )
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    /* Check parameters value */
    if (psLinkParams->miu < PHLNLLCP_FRI_MIU_DEFAULT)
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    /* Check if buffers are large enough to support minimal MIU */
    if ((nRxBufferLength < (nMaxHeaderSize + PHLNLLCP_FRI_MIU_DEFAULT)) ||
        (nTxBufferLength < (nMaxHeaderSize + PHLNLLCP_FRI_MIU_DEFAULT)) )
    {
        return PH_ERR_BUFFER_TOO_SMALL;
    }

    /* Check compatibility between reception buffer size and announced MIU */
    if (nRxBufferLength < (nMaxHeaderSize + psLinkParams->miu))
    {
        return PH_ERR_BUFFER_TOO_SMALL;
    }

    /* Start with a zero-filled monitor */
    memset(Llcp, 0x00, sizeof(phlnLlcp_Fri_t));  /* PRQA S 3200 */

    /* Reset the MAC Mapping layer */
    result = phlnLlcp_Fri_Mac_Reset(&Llcp->MAC, LowerDevice, &phlnLlcp_Fri_LinkStatus_CB, Llcp);
    if (result != PH_ERR_SUCCESS) {
        return result;
    }

    /* Save the working buffers */
    Llcp->sRxBuffer.buffer = pRxBuffer;
    Llcp->sRxBuffer.length = nRxBufferLength;
    Llcp->nRxBufferLength = nRxBufferLength;
    Llcp->pRxBuffer = pRxBuffer;
    Llcp->sTxBuffer.buffer = pTxBuffer;
    Llcp->sTxBuffer.length = nTxBufferLength;
    Llcp->nTxBufferLength = nTxBufferLength;

    /* Save the link status callback references */
    Llcp->pfLink_CB = pfLink_CB;
    Llcp->pLinkContext = pContext;

    /* Save the local link parameters */
    memcpy(&Llcp->sLocalParams, psLinkParams, sizeof(phlnLlcp_Fri_sLinkParameters_t));  /* PRQA S 3200 */

    /* Reset activity monitor */
    Llcp->nSymmetryCounter = 0;

    return PH_ERR_SUCCESS;
}


phStatus_t phlnLlcp_Fri_ChkLlcp( phlnLlcp_Fri_t                *Llcp,
                                phHal_sRemoteDevInformation_t  *psRemoteDevInfo,
                                phlnLlcp_Fri_Check_CB_t         pfCheck_CB,
                                void                           *pContext )
{
    /* Check parameters */
    if ( (Llcp == NULL) || (psRemoteDevInfo == NULL) || (pfCheck_CB == NULL) )
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP );
    }

    /* Check current state */
    if( Llcp->state != PHLNLLCP_FRI_STATE_RESET_INIT ) {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP);
    }

    /* Save the compliance check callback */
    Llcp->pfChk_CB = pfCheck_CB;
    Llcp->pChkContext = pContext;

    /* Forward check request to MAC layer */
    return phlnLlcp_Fri_Mac_ChkLlcp(&Llcp->MAC, psRemoteDevInfo, &phlnLlcp_Fri_ChkLlcp_CB, (void*)Llcp);
}


phStatus_t phlnLlcp_Fri_Activate( phlnLlcp_Fri_t *Llcp )
{
    /* Check parameters */
    if (Llcp == NULL)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Check current state */
    if( Llcp->state != PHLNLLCP_FRI_STATE_CHECKED ) {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP);
    }

    /* Update state */
    Llcp->state = PHLNLLCP_FRI_STATE_ACTIVATION;

    Llcp->MAC.Osal = Llcp->osal;

    /* Forward check request to MAC layer */
    return phlnLlcp_Fri_Mac_Activate(&Llcp->MAC);
}


phStatus_t phlnLlcp_Fri_Deactivate( phlnLlcp_Fri_t *Llcp )
{
    phStatus_t status =PH_ERR_SUCCESS;

    /* Check parameters */
    if (Llcp == NULL)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Check current state */
    if( (Llcp->state != PHLNLLCP_FRI_STATE_OPERATION_RECV)  &&
        (Llcp->state != PHLNLLCP_FRI_STATE_OPERATION_SEND) ) {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP);
    }

    /* Send DISC packet */
    // status = phlnLlcp_Fri_SendDisconnect(Llcp);   // need to fix
    if (status == PH_ERR_PENDING)
    {
        /* Wait for packet to be sent before deactivate link */
        //   return status;
    }

    /* Perform actual deactivation */
    return phlnLlcp_Fri_InternalDeactivate(Llcp);
}


phStatus_t phlnLlcp_Fri_GetLocalInfo( phlnLlcp_Fri_t                  *Llcp,
                                     phlnLlcp_Fri_sLinkParameters_t   *pParams )
{
    /* Check parameters */
    if ((Llcp == NULL) || (pParams == NULL))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Copy response */
    memcpy(pParams, &Llcp->sLocalParams, sizeof(phlnLlcp_Fri_sLinkParameters_t));  /* PRQA S 3200 */

    return PH_ERR_SUCCESS;
}


phStatus_t phlnLlcp_Fri_GetRemoteInfo( phlnLlcp_Fri_t                  *Llcp,
                                      phlnLlcp_Fri_sLinkParameters_t   *pParams )
{
    /* Check parameters */
    if ((Llcp == NULL) || (pParams == NULL))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Copy response */
    memcpy(pParams, &Llcp->sRemoteParams, sizeof(phlnLlcp_Fri_sLinkParameters_t));  /* PRQA S 3200 */

    return PH_ERR_SUCCESS;
}


phStatus_t phlnLlcp_Fri_Send( phlnLlcp_Fri_t                  *Llcp,
                             phlnLlcp_Fri_sPacketHeader_t     *psHeader,
                             phlnLlcp_Fri_sPacketSequence_t   *psSequence,
                             phNfc_sData_t                    *psInfo,
                             phlnLlcp_Fri_Send_CB_t            pfSend_CB,
                             void                             *pContext )
{
    phStatus_t result;

    /* Check parameters */
    if ((Llcp == NULL) || (psHeader == NULL) || (pfSend_CB == NULL))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Check if previous phlnLlcp_Fri_Send() has finished */
    if (Llcp->pfSendCB != NULL)
    {
        /* Error: a send operation is already running */
        return PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP);
    }

    /* Save the callback parameters */
    Llcp->pfSendCB = pfSend_CB;
    Llcp->pSendContext = pContext;

    /* Reset activity counter */
    Llcp->nSymmetryCounter = 0;

    if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_SEND)
    {
        /* Ready to send */
        result = phlnLlcp_Fri_InternalSend(Llcp, psHeader, psSequence, psInfo);
    }
    else if (Llcp->state == PHLNLLCP_FRI_STATE_OPERATION_RECV)
    {
        /* Not ready to send, save send params for later use */
        Llcp->psSendHeader = psHeader;
        Llcp->psSendSequence = psSequence;
        if (psInfo != NULL)
        {
            memcpy(Llcp->psSendInfo->buffer, psInfo->buffer, psInfo->length);  /* PRQA S 3200 */
            Llcp->psSendInfo->length = psInfo->length;
        }
        result = PH_ERR_PENDING;
    }
    else
    {
        Llcp->pfSendCB = NULL;
        Llcp->pSendContext = NULL;
        /* Incorrect state for sending ! */
        result = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP);
    }
    return result;
}


phStatus_t phlnLlcp_Fri_Recv( phlnLlcp_Fri_t            *Llcp,
                             phlnLlcp_Fri_Recv_CB_t      pfRecv_CB,
                             void                       *pContext )
{
    phStatus_t result = PH_ERR_SUCCESS;

    /* Check parameters */
    if ((Llcp == NULL) || (pfRecv_CB == NULL))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }

    /* Check if previous phlnLlcp_Fri_Recv() has finished */
    if (Llcp->pfRecvCB != NULL)
    {
        /* Error: a send operation is already running */
        return PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP);
    }

    /* Save the callback parameters */
    Llcp->pfRecvCB = pfRecv_CB;
    Llcp->pRecvContext = pContext;

    /* NOTE: nothing more to do, the receive function is called in background */

    return result;
}
#endif /* NXPBUILD__PHLN_LLCP_FRI */
