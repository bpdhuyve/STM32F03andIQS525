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
* \file  phlnLlcp_Fri_Transport.c
* \brief
*
* $Author: nxp69453 $
* $Revision: 498 $
* $Date: 2014-02-11 10:12:36 +0530 (Tue, 11 Feb 2014) $
*
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI

#include "phlnLlcp_Fri_Llcp.h"
#include "phlnLlcp_Fri_Transport.h"
#include "phlnLlcp_Fri_Transport_Connectionless.h"
#include "phlnLlcp_Fri_Transport_Connection.h"
#include <phOsal.h>
#include "phlnLlcp_Fri_Utils.h"

static phStatus_t phlnLlcp_Fri_Transport_RegisterName(phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket,
                                                      uint8_t                            nSap,
                                                      phNfc_sData_t                     *psServiceName);

static phStatus_t phlnLlcp_Fri_Transport_DiscoverServicesEx(phlnLlcp_Fri_Transport_t  *psTransport);

static void phlnLlcp_Fri_Transport_Send_CB(void             *pContext,
                                           phStatus_t        status);

static phStatus_t phlnLlcp_Fri_Transport_GetFreeSap(phlnLlcp_Fri_Transport_t  *psTransport,
                                                    phNfc_sData_t             *psServiceName,
                                                    uint8_t                   *pnSap)
{
    uint8_t i;
    uint8_t sap;
    uint8_t min_sap_range, max_sap_range;
    phlnLlcp_Fri_Transport_Socket_t* pSocketTable = psTransport->pSocketTable;

    /* Calculate authorized SAP range */
    if ((psServiceName != NULL) && (psServiceName->length > 0))
    {
        /* Make sure that we will return the same SAP if service name was already used in the past */
        for(i=0 ; i<PHLNLLCP_FRI_SDP_ADVERTISED_NB ; i++)
        {
            if((psTransport->pCachedServiceNames[i].sServiceName.length > 0) &&
                (memcmp(psTransport->pCachedServiceNames[i].sServiceName.buffer, psServiceName->buffer, psServiceName->length) == 0))
            {
                /* Service name matched in cached service names list */
                *pnSap = psTransport->pCachedServiceNames[i].nSap;
                return PH_ERR_SUCCESS;
            }
        }

        /* SDP advertised service */
        min_sap_range = PHLNLLCP_FRI_SAP_SDP_ADVERTISED_FIRST;
        max_sap_range = PHLNLLCP_FRI_SAP_SDP_UNADVERTISED_FIRST;
    }
    else
    {
        /* Non-SDP advertised service */
        min_sap_range = PHLNLLCP_FRI_SAP_SDP_UNADVERTISED_FIRST;
        max_sap_range = PHLNLLCP_FRI_SAP_NUMBER;
    }

    /* Try all possible SAPs */
    for(sap=min_sap_range ; sap<max_sap_range ; sap++)
    {
        /* Go through socket list to check if current SAP is in use */
        for(i=0 ; i<PHLNLLCP_FRI_NB_SOCKET_MAX ; i++)
        {
            if((pSocketTable[i].eSocket_State >= phlnLlcp_Fri_TransportSocket_eSocketBound) &&
                (pSocketTable[i].socket_sSap == sap))
            {
                /* SAP is already in use */
                break;
            }
        }

        if (i >= PHLNLLCP_FRI_NB_SOCKET_MAX)
        {
            /* No socket is using current SAP, proceed with binding */
            *pnSap = sap;
            return PH_ERR_SUCCESS;
        }
    }

    /* If we reach this point, it means that no SAP is free */
    return PH_ERR_INSUFFICIENT_RESOURCES;
}

static phStatus_t phlnLlcp_Fri_Transport_EncodeSdreqTlv(phNfc_sData_t    *psTlvData,
                                                        uint32_t         *pOffset,
                                                        uint8_t           nTid,
                                                        phNfc_sData_t    *psServiceName)
{
    phStatus_t result;
    uint32_t nTlvOffset = *pOffset;
    uint32_t nTlvStartOffset = nTlvOffset;

    /* Encode the TID */
    result = phlnLlcp_Fri_Utils_EncodeTLV(psTlvData,
        &nTlvOffset,
        PHLNLLCP_FRI_TLV_TYPE_SDREQ,
        1,
        &nTid);
    if (result != PH_ERR_SUCCESS)
    {
        goto clean_and_return;
    }

    /* Encode the service name itself */
    result = phlnLlcp_Fri_Utils_AppendTLV(psTlvData,
                                    nTlvStartOffset,
                                    &nTlvOffset,
                                    (uint8_t)psServiceName->length,
                                    psServiceName->buffer);
    if (result != PH_ERR_SUCCESS)
    {
        goto clean_and_return;
    }

clean_and_return:
    /* Save offset if no error occured */
    if (result == PH_ERR_SUCCESS)
    {
        *pOffset = nTlvOffset;
    }

    return result;
}

static phStatus_t phlnLlcp_Fri_Transport_EncodeSdresTlv(phNfc_sData_t  *psTlvData,
                                                        uint32_t       *pOffset,
                                                        uint8_t         nTid,
                                                        uint8_t         nSap)
{  /* PRQA S 0779 */
    phStatus_t result;
    uint32_t nTlvStartOffset = *pOffset;

    /* Encode the TID */
    result = phlnLlcp_Fri_Utils_EncodeTLV(psTlvData,
                                    pOffset,
                                    PHLNLLCP_FRI_TLV_TYPE_SDRES,
                                    1,
                                    &nTid);
    if (result != PH_ERR_SUCCESS)
    {
        goto clean_and_return;
    }

    /* Encode the service name itself */
    result = phlnLlcp_Fri_Utils_AppendTLV(psTlvData,
                                         nTlvStartOffset,
                                         pOffset,
                                         1,
                                         &nSap);
    if (result != PH_ERR_SUCCESS)
    {
        goto clean_and_return;
    }

clean_and_return:
    /* Restore previous offset if an error occurred */
    if (result != PH_ERR_SUCCESS)
    {
        *pOffset = nTlvStartOffset;
    }

    return result;
}


static phlnLlcp_Fri_Transport_Socket_t *phlnLlcp_Fri_Transport_ServiceNameLoockup(phlnLlcp_Fri_Transport_t   *psTransport,
                                                                                  phNfc_sData_t              *pServiceName)
{
    uint32_t                            index;
    uint8_t                             cacheIndex;
    phlnLlcp_Fri_CachedServiceName_t   *pCachedServiceName;
    phlnLlcp_Fri_Transport_Socket_t    *pSocket;

    /* Search a socket with the SN */
    for(index=0; index<PHLNLLCP_FRI_NB_SOCKET_MAX; index++)
    {
        pSocket = &psTransport->pSocketTable[index];
        /* Test if the CO socket is in Listen state or the CL socket is bound and if its SN is the good one */
        if((((pSocket->eSocket_Type == phlnLlcp_Fri_Transport_eConnectionOriented)
            && (pSocket->eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketRegistered))
            || ((pSocket->eSocket_Type == phlnLlcp_Fri_Transport_eConnectionLess)
            && (pSocket->eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketBound)))
            && (pServiceName->length == pSocket->sServiceName.length)
            && !memcmp(pServiceName->buffer, pSocket->sServiceName.buffer, pServiceName->length))
        {
            /* Add new entry to cached service name/sap if not already in table */
            for(cacheIndex=0; cacheIndex < PHLNLLCP_FRI_SDP_ADVERTISED_NB; cacheIndex++)
            {
                pCachedServiceName = &psTransport->pCachedServiceNames[cacheIndex];

                if (pCachedServiceName->sServiceName.buffer != NULL)
                {
                    if ((pCachedServiceName->sServiceName.length == pServiceName->length) &&
                        (memcmp(pCachedServiceName->sServiceName.buffer, pServiceName->buffer, pServiceName->length) == 0))
                    {
                        /* Already registered */
                        break;
                    }
                }
                else
                {
                    if(sizeof(psTransport->pServiceNames[index]) >= pServiceName->length)
                    {
                        /* Reached end of existing entries and not found the service name,
                        * => Add the new entry */
                        pCachedServiceName->nSap = pSocket->socket_sSap;
                        pCachedServiceName->sServiceName.buffer = psTransport->pServiceNames[index];

                        memcpy(pCachedServiceName->sServiceName.buffer, pServiceName->buffer, pServiceName->length);  /* PRQA S 3200 */

                        pCachedServiceName->sServiceName.length = pServiceName->length;

                        break;
                    }
                    else
                    {
                        return NULL;  /* Insufficient memory */
                    }

                }
            }

            return pSocket;
        }
    }

    return NULL;
}


static phStatus_t phlnLlcp_Fri_Transport_DiscoveryAnswer( phlnLlcp_Fri_Transport_t  *psTransport )   /* PRQA S 3209 */
{  /* PRQA S 0779 */
    phStatus_t         result = PH_ERR_PENDING;
    phNfc_sData_t     sInfoBuffer;
    uint32_t          nTlvOffset;
    uint8_t           index;
    uint8_t           nTid, nSap;

    /* Test if a send is pending */
    if(!psTransport->bSendPending)
    {
        /* Set the header */
        psTransport->sLlcpHeader.dsap  = PHLNLLCP_FRI_SAP_SDP;
        psTransport->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_SNL;
        psTransport->sLlcpHeader.ssap  = PHLNLLCP_FRI_SAP_SDP;

        /* Prepare the info buffer */
        sInfoBuffer.buffer = psTransport->pDiscoveryBuffer;
        sInfoBuffer.length = sizeof(psTransport->pDiscoveryBuffer);

        /* Encode as many requests as possible */
        nTlvOffset = 0;
        for(index=0 ; index<psTransport->nDiscoveryResListSize ; index++)
        {
            /* Get current TID/SAP and try to encode them in SNL frame */
            nTid = psTransport->nDiscoveryResTidList[index];
            nSap = psTransport->nDiscoveryResSapList[index];
            /* Encode response */
            result = phlnLlcp_Fri_Transport_EncodeSdresTlv(&sInfoBuffer,
                                                        &nTlvOffset,
                                                        nTid,
                                                        nSap);
            if (result != PH_ERR_SUCCESS)
            {
                /* Impossible to fit the entire response */
                /* TODO: support response fragmentation */
                break;
            }
        }

        /* Reset list size to be able to handle a new request */
        psTransport->nDiscoveryResListSize = 0;

        /* Update buffer length to match real TLV size */
        sInfoBuffer.length = nTlvOffset;

        /* Send Pending */
        psTransport->bSendPending = true;

        /* Send SNL frame */
        result =  phlnLlcp_Fri_Send(psTransport->pLlcp,
                                  &psTransport->sLlcpHeader,
                                  NULL,
                                  &sInfoBuffer,
                                  &phlnLlcp_Fri_Transport_Send_CB,
                                  psTransport);
    }
    else
    {
        /* Impossible to send now, this function will be called again on next opportunity */
    }

    return result;
}


static void phlnLlcp_Fri_Transport_Handle_Discovery_IncomingFrame(phlnLlcp_Fri_Transport_t    *psTransport,
                                                                  phNfc_sData_t               *psData)
{
    phStatus_t           status = PH_ERR_SUCCESS;
    phStatus_t           result = 0;
    phNfc_sData_t        sValue;
    phNfc_sData_t        sServiceName;
    uint32_t             nInTlvOffset;
    uint8_t              nType;
    uint8_t              nTid;
    uint8_t              nSap;
    pphlnLlcp_Fri_Cr_t   pfSavedCb;
    void                *pfSavedContext;
    phlnLlcp_Fri_Transport_Socket_t  *pSocket;

    PH_UNUSED_VARIABLE(result);
    PH_UNUSED_VARIABLE(status);

    /* Parse all TLVs in frame */
    nInTlvOffset = 0;
    while(nInTlvOffset < psData->length)
    {
        result = phlnLlcp_Fri_Utils_DecodeTLV(psData,
                                       &nInTlvOffset,
                                       &nType,
                                       &sValue );
        switch(nType)
        {
        case PHLNLLCP_FRI_TLV_TYPE_SDREQ:
            if (sValue.length < 2)
            {
                /* Erroneous request, ignore */
                break;
            }
            /* Decode TID */
            nTid = sValue.buffer[0];
            /* Decode service name */
            sServiceName.buffer = sValue.buffer + 1;
            sServiceName.length = sValue.length - 1;

            /* Handle SDP service name */
            if((sServiceName.length == sizeof(PHLNLLCP_FRI_SERVICENAME_SDP)-1)
                && !memcmp(sServiceName.buffer, PHLNLLCP_FRI_SERVICENAME_SDP, sServiceName.length))
            {
                nSap = PHLNLLCP_FRI_SAP_SDP;
            }
            else
            {
                /* Match service name in socket list */
                pSocket = phlnLlcp_Fri_Transport_ServiceNameLoockup(psTransport, &sServiceName);
                if (pSocket != NULL)
                {
                    nSap = pSocket->socket_sSap;
                }
                else
                {
                    nSap = 0;
                }
            }

            /* Encode response */
            if (psTransport->nDiscoveryResListSize < PHLNLLCP_FRI_SNL_RESPONSE_MAX)
            {
                psTransport->nDiscoveryResSapList[psTransport->nDiscoveryResListSize] = nSap;
                psTransport->nDiscoveryResTidList[psTransport->nDiscoveryResListSize] = nTid;
                psTransport->nDiscoveryResListSize++;
            }
            else
            {
                /* Impossible to fit the entire response */
            }
            break;

        case PHLNLLCP_FRI_TLV_TYPE_SDRES:
            if (psTransport->pfDiscover_Cb == NULL)
            {
                /* Ignore response when no requests are pending */
                break;
            }
            if (sValue.length != 2)
            {
                /* Erroneous response, ignore it */
                break;
            }
            /* Decode TID and SAP */
            nTid = sValue.buffer[0];
            if (nTid >= psTransport->nDiscoveryListSize)
            {
                /* Unkown TID, ignore it */
                break;
            }
            nSap = sValue.buffer[1];
            /* Save response */
            psTransport->pnDiscoverySapList[nTid] = nSap;
            /* Update response counter */
            psTransport->nDiscoveryResOffset++;
            break;

        default:
            /* Ignored */
            break;
        }
    }

    /* If discovery requests have been received, send response */
    if (psTransport->nDiscoveryResListSize > 0)
    {
        status = phlnLlcp_Fri_Transport_DiscoveryAnswer(psTransport);
    }

    /* If all discovery responses have been received, trigger callback (if any) */
    if ((psTransport->pfDiscover_Cb != NULL) &&
        (psTransport->nDiscoveryResOffset >= psTransport->nDiscoveryListSize))
    {
        pfSavedCb = psTransport->pfDiscover_Cb;
        pfSavedContext = psTransport->pDiscoverContext;

        psTransport->pfDiscover_Cb = NULL;
        psTransport->pDiscoverContext = NULL;

        pfSavedCb(pfSavedContext, PH_ERR_SUCCESS);
    }
}

static void phlnLlcp_Fri_Transport_Recv_CB(void            *pContext,
                                           phNfc_sData_t   *psData,
                                           phStatus_t       status)
{
    phlnLlcp_Fri_sPacketHeader_t   sLlcpLocalHeader;
    uint8_t   dsap;
    uint8_t   ptype;
    uint8_t   ssap;

    phlnLlcp_Fri_Transport_t* pLlcpTransport = (phlnLlcp_Fri_Transport_t*)pContext;

    if(status != PH_ERR_SUCCESS)
    {
        pLlcpTransport->LinkStatusError = true;
    }
    else
    {
        status = (uint16_t) phlnLlcp_Fri_Utils_Buffer2Header( psData->buffer, 0x00, &sLlcpLocalHeader);

        dsap  = (uint8_t)sLlcpLocalHeader.dsap;
        ptype = (uint8_t)sLlcpLocalHeader.ptype;
        ssap  = (uint8_t)sLlcpLocalHeader.ssap;

        /* Update the length value (without the header length) */
        psData->length = psData->length - PHLNLLCP_FRI_PACKET_HEADER_SIZE;

        /* Update the buffer pointer */
        psData->buffer = psData->buffer + PHLNLLCP_FRI_PACKET_HEADER_SIZE;

        switch(ptype)
        {
            /* Connectionless */
        case PHLNLLCP_FRI_PTYPE_UI:
            {
                phlnLlcp_Fri_Handle_Connectionless_IncommingFrame(pLlcpTransport,
                                                              psData,
                                                              dsap,
                                                              ssap);
            }break;

            /* Service Discovery Protocol */
        case PHLNLLCP_FRI_PTYPE_SNL:
            {
                if ((ssap == PHLNLLCP_FRI_SAP_SDP) && (dsap == PHLNLLCP_FRI_SAP_SDP))
                {
                    phlnLlcp_Fri_Transport_Handle_Discovery_IncomingFrame(pLlcpTransport,
                                                           psData);
                }
                else
                {
                    /* Ignore frame if source and destination are not the SDP service */
                }
            }break;

            /* Connection oriented */
            /* NOTE: forward reserved PTYPE to enable FRMR sending */
        case PHLNLLCP_FRI_PTYPE_CONNECT:
        case PHLNLLCP_FRI_PTYPE_CC:
        case PHLNLLCP_FRI_PTYPE_DISC:
        case PHLNLLCP_FRI_PTYPE_DM:
        case PHLNLLCP_FRI_PTYPE_I:
        case PHLNLLCP_FRI_PTYPE_RR:
        case PHLNLLCP_FRI_PTYPE_RNR:
        case PHLNLLCP_FRI_PTYPE_FRMR:
        case PHLNLLCP_FRI_PTYPE_RESERVED1:
        case PHLNLLCP_FRI_PTYPE_RESERVED2:
        case PHLNLLCP_FRI_PTYPE_RESERVED3:
            {
                phlnLlcp_Fri_Handle_ConnectionOriented_IncommingFrame(pLlcpTransport,
                                                                 psData,
                                                                 dsap,
                                                                 ptype,
                                                                 ssap);
            }break;
        default:
            {

            }break;
        }

        /*Restart the Receive Loop */
        status  = phlnLlcp_Fri_Recv(pLlcpTransport->pLlcp,
                                  &phlnLlcp_Fri_Transport_Recv_CB,
                                  pLlcpTransport);
    }
}

static void phlnLlcp_Fri_Transport_Send_CB(void            *pContext,
                                           phStatus_t       status)
{
    phlnLlcp_Fri_Transport_t         *psTransport = (phlnLlcp_Fri_Transport_t*)pContext;
    phStatus_t                        result = PH_ERR_FAILED;
    phNfc_sData_t                     sFrmrBuffer;
    phlnLlcp_Fri_Send_CB_t            pfSavedCb;
    void                             *pSavedContext;
    phlnLlcp_Fri_Transport_Socket_t  *pCurrentSocket = NULL;
    uint8_t                           index;

    /* 1 - Reset the FLAG send pending*/

    psTransport->bSendPending = false;

    /* 2 - Handle pending error responses */

    if(psTransport->bFrmrPending)
    {
        /* Reset FRMR pending */
        psTransport->bFrmrPending = false;

        /* Send Frmr */
        sFrmrBuffer.buffer = psTransport->FrmrInfoBuffer;
        sFrmrBuffer.length = 0x04; /* Size of FRMR Information field */

        /* Send Pending */
        psTransport->bSendPending = true;

        result =  phlnLlcp_Fri_Send(psTransport->pLlcp,
                                &psTransport->sLlcpHeader,
                                NULL,
                                &sFrmrBuffer,
                                &phlnLlcp_Fri_Transport_Send_CB,
                                psTransport);

    }
    else if(psTransport->bDmPending)
    {
        /* Reset DM pending */
        psTransport->bDmPending = false;

        /* Send DM pending */
        result = phlnLlcp_Fri_Transport_SendDisconnectMode(psTransport,
                                                         psTransport->DmInfoBuffer[0],
                                                         psTransport->DmInfoBuffer[1],
                                                         psTransport->DmInfoBuffer[2]);
    }
    else
    {
        /* Nothing to Do */
    }

    /* 3 - Call the original callback */

    if (psTransport->pfLinkSendCb != NULL)
    {
        pfSavedCb = psTransport->pfLinkSendCb;
        pSavedContext = psTransport->pLinkSendContext;

        psTransport->pfLinkSendCb = NULL;
        psTransport->pLinkSendContext = NULL;

        (*pfSavedCb)(pSavedContext, status);
    }

    /* 4 - Handle pending send operations */

    /* Check for pending discovery requests/responses */
    if (psTransport->nDiscoveryResListSize > 0)
    {
        status = phlnLlcp_Fri_Transport_DiscoveryAnswer(psTransport);
    }
    if ( (psTransport->pfDiscover_Cb != NULL) &&
        (psTransport->nDiscoveryReqOffset < psTransport->nDiscoveryListSize) )
    {
        result = phlnLlcp_Fri_Transport_DiscoverServicesEx(psTransport);
    }

    /* Init index */
    index = psTransport->socketIndex;

    /* Check all sockets for pending operation */
    do
    {
        /* Modulo-increment index */
        index = (index + 1) % PHLNLLCP_FRI_NB_SOCKET_MAX;

        pCurrentSocket = &psTransport->pSocketTable[index];

        /* Dispatch to the corresponding transport layer */
        if (pCurrentSocket->eSocket_Type == phlnLlcp_Fri_Transport_eConnectionOriented)
        {
            result = phlnLlcp_Fri_Transport_ConnectionOriented_HandlePendingOperations(pCurrentSocket);
        }
        else if (pCurrentSocket->eSocket_Type == phlnLlcp_Fri_Transport_eConnectionLess)
        {
            result = phlnLlcp_Fri_Transport_Connectionless_HandlePendingOperations(pCurrentSocket);
        }
        else
        {
            /* Nothing to Do */
        }

        if (result != PH_ERR_FAILED)
        {
            /* Stop looping if pending operation has been found */
            break;
        }

    } while(index != psTransport->socketIndex);

    /* Save the new index */
}

phStatus_t phlnLlcp_Fri_Transport_Reset (phlnLlcp_Fri_Transport_t      *pLlcpTransport,
                                         phlnLlcp_Fri_t                *pLlcp)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t i;

    /* Check for NULL pointers */
    if(pLlcpTransport == NULL || pLlcp == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        /* Reset Transport structure */
        pLlcpTransport->pLlcp            = pLlcp;
        pLlcpTransport->LinkStatusError  = false;
        pLlcpTransport->bSendPending     = false;
        pLlcpTransport->bRecvPending     = false;
        pLlcpTransport->bDmPending       = false;
        pLlcpTransport->bFrmrPending     = false;
        pLlcpTransport->socketIndex      = false;
        pLlcpTransport->LinkStatusError  = 0;
        pLlcpTransport->pfDiscover_Cb    = NULL;

        /* Initialize cached service name/sap table */
        memset(pLlcpTransport->pCachedServiceNames, 0x00, sizeof(phlnLlcp_Fri_CachedServiceName_t)*PHLNLLCP_FRI_SDP_ADVERTISED_NB);  /* PRQA S 3200 */

        /* Reset all the socket info in the table */
        for(i=0;i<PHLNLLCP_FRI_NB_SOCKET_MAX;i++)
        {
            pLlcpTransport->pSocketTable[i].eSocket_State                  = phlnLlcp_Fri_TransportSocket_eSocketDefault;
            pLlcpTransport->pSocketTable[i].eSocket_Type                   = phlnLlcp_Fri_Transport_eDefaultType;
            pLlcpTransport->pSocketTable[i].index                          = i;
            pLlcpTransport->pSocketTable[i].pContext                       = NULL;
            pLlcpTransport->pSocketTable[i].pListenContext                 = NULL;
            pLlcpTransport->pSocketTable[i].pAcceptContext                 = NULL;
            pLlcpTransport->pSocketTable[i].pRejectContext                 = NULL;
            pLlcpTransport->pSocketTable[i].pConnectContext                = NULL;
            pLlcpTransport->pSocketTable[i].pDisonnectContext              = NULL;
            pLlcpTransport->pSocketTable[i].pSendContext                   = NULL;
            pLlcpTransport->pSocketTable[i].pRecvContext                   = NULL;
            pLlcpTransport->pSocketTable[i].pSocketErrCb                   = NULL;
            pLlcpTransport->pSocketTable[i].bufferLinearLength             = 0;
            pLlcpTransport->pSocketTable[i].bufferSendMaxLength            = 0;
            pLlcpTransport->pSocketTable[i].bufferRwMaxLength              = 0;
            pLlcpTransport->pSocketTable[i].ReceiverBusyCondition          = false;
            pLlcpTransport->pSocketTable[i].RemoteBusyConditionInfo        = false;
            pLlcpTransport->pSocketTable[i].socket_sSap                    = PHLNLLCP_FRI_SAP_DEFAULT;
            pLlcpTransport->pSocketTable[i].socket_dSap                    = PHLNLLCP_FRI_SAP_DEFAULT;
            pLlcpTransport->pSocketTable[i].bSocketRecvPending             = false;
            pLlcpTransport->pSocketTable[i].bSocketSendPending             = false;
            pLlcpTransport->pSocketTable[i].bSocketListenPending           = false;
            pLlcpTransport->pSocketTable[i].bSocketDiscPending             = false;
            pLlcpTransport->pSocketTable[i].bSocketConnectPending          = false;
            pLlcpTransport->pSocketTable[i].bSocketAcceptPending           = false;
            pLlcpTransport->pSocketTable[i].bSocketRRPending               = false;
            pLlcpTransport->pSocketTable[i].bSocketRNRPending              = false;
            pLlcpTransport->pSocketTable[i].psTransport                    = pLlcpTransport;
            pLlcpTransport->pSocketTable[i].pfSocketSend_Cb                = NULL;
            pLlcpTransport->pSocketTable[i].pfSocketRecv_Cb                = NULL;
            pLlcpTransport->pSocketTable[i].pfSocketRecvFrom_Cb            = NULL;
            pLlcpTransport->pSocketTable[i].pfSocketListen_Cb              = NULL;
            pLlcpTransport->pSocketTable[i].pfSocketConnect_Cb             = NULL;
            pLlcpTransport->pSocketTable[i].pfSocketDisconnect_Cb          = NULL;
            pLlcpTransport->pSocketTable[i].socket_VS                      = 0;
            pLlcpTransport->pSocketTable[i].socket_VSA                     = 0;
            pLlcpTransport->pSocketTable[i].socket_VR                      = 0;
            pLlcpTransport->pSocketTable[i].socket_VRA                     = 0;
            pLlcpTransport->pSocketTable[i].remoteRW                       = 0;
            pLlcpTransport->pSocketTable[i].localRW                        = 0;
            pLlcpTransport->pSocketTable[i].remoteMIU                      = 0;
            pLlcpTransport->pSocketTable[i].localMIUX                      = 0;
            pLlcpTransport->pSocketTable[i].index                          = 0;
            pLlcpTransport->pSocketTable[i].indexRwRead                    = 0;
            pLlcpTransport->pSocketTable[i].indexRwWrite                   = 0;

            memset(&pLlcpTransport->pSocketTable[i].sSocketOption, 0x00, sizeof(phlnLlcp_Fri_Transport_sSocketOptions_t));  /* PRQA S 3200 */

            pLlcpTransport->pSocketTable[i].sServiceName.length = 0;
        }

        /* Start The Receive Loop */
        status  = phlnLlcp_Fri_Recv(pLlcpTransport->pLlcp,
                                    &phlnLlcp_Fri_Transport_Recv_CB,
                                    pLlcpTransport);
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Transport_CloseAll (phlnLlcp_Fri_Transport_t *pLlcpTransport)
{
    phStatus_t                          status = PH_ERR_SUCCESS;
    phlnLlcp_Fri_CachedServiceName_t   *pCachedServiceName;
    uint8_t                             i;

    PH_ASSERT_NULL(pLlcpTransport);

    /* Close all sockets */
    for(i=0;i<PHLNLLCP_FRI_NB_SOCKET_MAX;i++)
    {
        if(pLlcpTransport->pSocketTable[i].eSocket_Type == phlnLlcp_Fri_Transport_eConnectionOriented)
        {
            switch(pLlcpTransport->pSocketTable[i].eSocket_State)
            {
            case phlnLlcp_Fri_TransportSocket_eSocketConnected:
            case phlnLlcp_Fri_TransportSocket_eSocketConnecting:
            case phlnLlcp_Fri_TransportSocket_eSocketAccepted:
            case phlnLlcp_Fri_TransportSocket_eSocketDisconnected:
            case phlnLlcp_Fri_TransportSocket_eSocketDisconnecting:
            case phlnLlcp_Fri_TransportSocket_eSocketRejected:
                status = phlnLlcp_Fri_Transport_Close(&pLlcpTransport->pSocketTable[i]);
                break;
            default:
                /* Do nothing */
                break;
            }
        }
        else
        {
            status = phlnLlcp_Fri_Transport_Close(&pLlcpTransport->pSocketTable[i]);
        }
    }

    /* Reset cached service name/sap table */
    for(i=0;i<PHLNLLCP_FRI_SDP_ADVERTISED_NB;i++)
    {
        pCachedServiceName = &pLlcpTransport->pCachedServiceNames[i];

        pCachedServiceName->nSap = 0;

        pCachedServiceName->sServiceName.length = 0;
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_LinkSend( phlnLlcp_Fri_Transport_t        *LlcpTransport,
                                           phlnLlcp_Fri_sPacketHeader_t     *psHeader,
                                           phlnLlcp_Fri_sPacketSequence_t   *psSequence,
                                           phNfc_sData_t                    *psInfo,
                                           phlnLlcp_Fri_Send_CB_t            pfSend_CB,
                                           void                             *pContext )
{
    phStatus_t status;
    /* Check if a send is already ongoing */
    if (LlcpTransport->pfLinkSendCb != NULL)
    {
        return PH_ERR_BUSY;
    }

    /* Save callback details */
    LlcpTransport->pfLinkSendCb = pfSend_CB;
    LlcpTransport->pLinkSendContext = pContext;

    /* Call the link-level send function */
    status = phlnLlcp_Fri_Send(LlcpTransport->pLlcp, psHeader, psSequence, psInfo, &phlnLlcp_Fri_Transport_Send_CB, (void*)LlcpTransport);
    if (status != PH_ERR_PENDING && status != PH_ERR_SUCCESS)
    {
        /* Clear out callbacks */
        LlcpTransport->pfLinkSendCb = NULL;
        LlcpTransport->pLinkSendContext = NULL;
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Transport_SendFrameReject(phlnLlcp_Fri_Transport_t          *psTransport,
                                                  uint8_t                            dsap,
                                                  uint8_t                            rejectedPTYPE,
                                                  uint8_t                            ssap,
                                                  phlnLlcp_Fri_sPacketSequence_t    *sLlcpSequence,
                                                  uint8_t                            WFlag,
                                                  uint8_t                            IFlag,
                                                  uint8_t                            RFlag,
                                                  uint8_t                            SFlag,
                                                  uint8_t                            vs,
                                                  uint8_t                            vsa,
                                                  uint8_t                            vr,
                                                  uint8_t                            vra)
{
    phStatus_t       status = PH_ERR_SUCCESS;
    phNfc_sData_t    sFrmrBuffer;
    uint8_t          flagValue;
    uint8_t          sequence = 0;
    uint8_t          index;
    uint8_t          socketFound = false;

    /* Search a socket waiting for a FRAME */
    for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
    {
        /* Test if the socket is in connected state and if its SSAP and DSAP are valid */
        if(psTransport->pSocketTable[index].socket_sSap == dsap
            && psTransport->pSocketTable[index].socket_dSap == ssap)
        {
            /* socket found */
            socketFound = true;
            break;
        }
    }

    /* Test if a socket has been found */
    if(socketFound)
    {
        /* Set socket state to disconnected */
        psTransport->pSocketTable[index].eSocket_State =  phlnLlcp_Fri_TransportSocket_eSocketDefault;  /* PRQA S 3689 */

        /* Call ErrCB due to a FRMR*/
        psTransport->pSocketTable[index].pSocketErrCb( psTransport->pSocketTable[index].pContext,PHLNLLCP_FRI_ERR_FRAME_REJECTED);  /* PRQA S 3689 */

        /* Close the socket */
        status = phlnLlcp_Fri_Transport_ConnectionOriented_Close(&psTransport->pSocketTable[index]);

        /* Set FRMR Header */
        psTransport->sLlcpHeader.dsap   = ssap;
        psTransport->sLlcpHeader.ptype  = PHLNLLCP_FRI_PTYPE_FRMR;
        psTransport->sLlcpHeader.ssap   = dsap;

        /* Set FRMR Information Field */
        flagValue = (WFlag<<7) | (IFlag<<6) | (RFlag<<5) | (SFlag<<4) | rejectedPTYPE;
        if (sLlcpSequence != NULL)
        {
            sequence = (uint8_t)((sLlcpSequence->ns<<4)|(sLlcpSequence->nr));
        }

        psTransport->FrmrInfoBuffer[0] = flagValue;
        psTransport->FrmrInfoBuffer[1] = sequence;
        psTransport->FrmrInfoBuffer[2] = (vs<<4)|vr ;
        psTransport->FrmrInfoBuffer[3] = (vsa<<4)|vra ;

        /* Test if a send is pending */
        if(psTransport->bSendPending)
        {
            psTransport->bFrmrPending = true;
            status = PH_ERR_PENDING;
        }
        else
        {
            sFrmrBuffer.buffer =  psTransport->FrmrInfoBuffer;
            sFrmrBuffer.length =  0x04; /* Size of FRMR Information field */

            /* Send Pending */
            psTransport->bSendPending = true;

            /* Send FRMR frame */
            status =  phlnLlcp_Fri_Send(psTransport->pLlcp,
                                     &psTransport->sLlcpHeader,
                                     NULL,
                                     &sFrmrBuffer,
                                     &phlnLlcp_Fri_Transport_Send_CB,
                                     psTransport);
        }
    }
    else
    {
        /* No active  socket*/
        /* FRMR Frame not handled*/
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Transport_SendDisconnectMode(phlnLlcp_Fri_Transport_t  *psTransport,
                                                     uint8_t                    dsap,
                                                     uint8_t                    ssap,
                                                     uint8_t                    dmOpCode)
{
    phStatus_t                       status = PH_ERR_SUCCESS;

    /* Test if a send is pending */
    if(psTransport->bSendPending)
    {
        /* DM pending */
        psTransport->bDmPending        = true;

        /* Store DM Info */
        psTransport->DmInfoBuffer[0] = dsap;
        psTransport->DmInfoBuffer[1] = ssap;
        psTransport->DmInfoBuffer[2] = dmOpCode;

        status = PH_ERR_PENDING;
    }
    else
    {
        /* Set the header */
        psTransport->sDmHeader.dsap  = dsap;
        psTransport->sDmHeader.ptype = PHLNLLCP_FRI_PTYPE_DM;
        psTransport->sDmHeader.ssap  = ssap;

        /* Save Operation Code to be provided in DM frame payload */
        psTransport->DmInfoBuffer[2] = dmOpCode;
        psTransport->sDmPayload.buffer    = &psTransport->DmInfoBuffer[2];
        psTransport->sDmPayload.length    = PHLNLLCP_FRI_DM_LENGTH;

        /* Send Pending */
        psTransport->bSendPending = true;

        /* Send DM frame */
        status =  phlnLlcp_Fri_Send(psTransport->pLlcp,
                                  &psTransport->sDmHeader,
                                  NULL,
                                  &psTransport->sDmPayload,
                                  &phlnLlcp_Fri_Transport_Send_CB,
                                  psTransport);
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_SocketGetLocalOptions(phlnLlcp_Fri_Transport_Socket_t  *pLlcpSocket,
                                                        phlnLlcp_Fri_sSocketOptions_t    *psLocalOptions)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if (pLlcpSocket == NULL || psLocalOptions == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /*  Test the socket type */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /*  Test the socket state */
    else if(pLlcpSocket->eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketDefault)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        status = phlnLlcp_Fri_Transport_ConnectionOriented_SocketGetLocalOptions(pLlcpSocket,
            psLocalOptions);
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_SocketGetRemoteOptions(phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket,
                                                         phlnLlcp_Fri_sSocketOptions_t     *psRemoteOptions)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if (pLlcpSocket == NULL || psRemoteOptions == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /*  Test the socket type */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /*  Test the socket state */
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketConnected)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        status = phlnLlcp_Fri_Transport_ConnectionOriented_SocketGetRemoteOptions(pLlcpSocket,
            psRemoteOptions);
    }

    return status;
}


static phStatus_t phlnLlcp_Fri_Transport_DiscoverServicesEx(phlnLlcp_Fri_Transport_t *psTransport)
{
    phStatus_t         result = PH_ERR_PENDING;
    phNfc_sData_t     sInfoBuffer;
    phNfc_sData_t     *psServiceName;
    uint32_t          nTlvOffset;

    /* Test if a send is pending */
    if(!psTransport->bSendPending)
    {
        /* Set the header */
        psTransport->sLlcpHeader.dsap  = PHLNLLCP_FRI_SAP_SDP;
        psTransport->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_SNL;
        psTransport->sLlcpHeader.ssap  = PHLNLLCP_FRI_SAP_SDP;

        /* Prepare the info buffer */
        sInfoBuffer.buffer = psTransport->pDiscoveryBuffer;
        sInfoBuffer.length = sizeof(psTransport->pDiscoveryBuffer);

        /* Encode as many requests as possible */
        nTlvOffset = 0;
        while(psTransport->nDiscoveryReqOffset < psTransport->nDiscoveryListSize)
        {
            /* Get current service name and try to encode it in SNL frame */
            psServiceName = &psTransport->psDiscoveryServiceNameList[psTransport->nDiscoveryReqOffset];
            result = phlnLlcp_Fri_Transport_EncodeSdreqTlv(&sInfoBuffer,
                                                &nTlvOffset,
                                                psTransport->nDiscoveryReqOffset,
                                                psServiceName);
            if (result != PH_ERR_SUCCESS)
            {
                /* Impossible to fit more requests in a single frame,
                * will be continued on next opportunity
                */
                break;
            }

            /* Update request counter */
            psTransport->nDiscoveryReqOffset++;
        }

        /* Update buffer length to match real TLV size */
        sInfoBuffer.length = nTlvOffset;

        /* Send Pending */
        psTransport->bSendPending = true;

        /* Send SNL frame */
        result =  phlnLlcp_Fri_Send(psTransport->pLlcp,
                                  &psTransport->sLlcpHeader,
                                  NULL,
                                  &sInfoBuffer,
                                  &phlnLlcp_Fri_Transport_Send_CB,
                                  psTransport);
    }
    else
    {
        /* Impossible to send now, this function will be called again on next opportunity */
    }

    return result;
}

phStatus_t phlnLlcp_Fri_Transport_DiscoverServices( phlnLlcp_Fri_Transport_t  *pLlcpTransport,
                                                   phNfc_sData_t              *psServiceNameList,
                                                   uint8_t                    *pnSapList,
                                                   uint8_t                     nListSize,
                                                   pphlnLlcp_Fri_Cr_t          pDiscover_Cb,
                                                   void                       *pContext )
{
    phStatus_t         result = PH_ERR_FAILED;

    /* Save request details */
    pLlcpTransport->psDiscoveryServiceNameList = psServiceNameList;
    pLlcpTransport->pnDiscoverySapList = pnSapList;
    pLlcpTransport->nDiscoveryListSize = nListSize;
    pLlcpTransport->pfDiscover_Cb = pDiscover_Cb;
    pLlcpTransport->pDiscoverContext = pContext;

    /* Reset internal counters */
    pLlcpTransport->nDiscoveryReqOffset = 0;
    pLlcpTransport->nDiscoveryResOffset = 0;

    /* Perform request */
    result = phlnLlcp_Fri_Transport_DiscoverServicesEx(pLlcpTransport);

    return result;
}

phStatus_t phlnLlcp_Fri_Transport_Socket(phlnLlcp_Fri_Transport_t                  *pLlcpTransport,
                                         phlnLlcp_Fri_Transport_eSocketType_t       eType,
                                         phlnLlcp_Fri_Transport_sSocketOptions_t   *psOptions,
                                         phNfc_sData_t                             *psWorkingBuffer,
                                         phlnLlcp_Fri_Transport_Socket_t          **pLlcpSocket,
                                         pphlnLlcp_Fri_TransportSocketErrCb_t       pErr_Cb,
                                         void                                      *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    phlnLlcp_Fri_sLinkParameters_t  LlcpLinkParamInfo;
    uint8_t index=0;
    uint8_t cpt;

    PH_ASSERT_NULL(psWorkingBuffer);

    /* Check for NULL pointers */
    if (   ((psOptions == NULL) && (eType == phlnLlcp_Fri_Transport_eConnectionOriented))
        || ((psWorkingBuffer == NULL) && (eType == phlnLlcp_Fri_Transport_eConnectionOriented))
        || (pLlcpSocket == NULL)
        || (pErr_Cb == NULL)
        || (pContext == NULL)
        || (pLlcpTransport == NULL))
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
        return status;
    }
    /*  Test the socket type*/
    else if(eType != phlnLlcp_Fri_Transport_eConnectionOriented && eType != phlnLlcp_Fri_Transport_eConnectionLess)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
        return status;
    }
    /* Connectionless sockets don't support options */
    else if ((psOptions != NULL) && (eType == phlnLlcp_Fri_Transport_eConnectionLess))
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
        return status;
    }
    else
    {
        /* Nothing to Do */
    }

    /* Get the local parameters of the LLCP Link */
    status = phlnLlcp_Fri_GetLocalInfo(pLlcpTransport->pLlcp,&LlcpLinkParamInfo);
    if(status != PH_ERR_SUCCESS)
    {
        status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_TRANSPORT);
        return status;
    }
    else
    {
        /* Search a socket free in the Socket Table*/
        do
        {
            if(pLlcpTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketDefault)
            {
                /* Set the socket pointer to socket of the table */
                *pLlcpSocket = &pLlcpTransport->pSocketTable[index];

                /* Store the socket info in the socket pointer */
                pLlcpTransport->pSocketTable[index].eSocket_Type     = eType;
                pLlcpTransport->pSocketTable[index].pSocketErrCb     = pErr_Cb;

                /* Store the context of the upper layer */
                pLlcpTransport->pSocketTable[index].pContext   = pContext;

                /* Set the pointers to the different working buffers */
                if(psOptions != NULL)
                {
                    if (eType == phlnLlcp_Fri_Transport_eConnectionOriented)
                    {
                        /* Test the socket options */
                        if (psOptions->rw > PHLNLLCP_FRI_RW_MAX)
                        {
                            status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
                            return status;
                        }

                        /* Set socket options */
                        memcpy(&pLlcpTransport->pSocketTable[index].sSocketOption, psOptions, sizeof(phlnLlcp_Fri_Transport_sSocketOptions_t));  /* PRQA S 3200 */

                        /* Set socket local params (MIUX & RW) */
                        pLlcpTransport->pSocketTable[index].localMIUX = (pLlcpTransport->pSocketTable[index].sSocketOption.miu - PHLNLLCP_FRI_MIU_DEFAULT) & PHLNLLCP_FRI_TLV_MIUX_MASK;
                        pLlcpTransport->pSocketTable[index].localRW   = pLlcpTransport->pSocketTable[index].sSocketOption.rw & PHLNLLCP_FRI_TLV_RW_MASK;

                        /* Set the Max length for the Send and Receive Window Buffer */
                        pLlcpTransport->pSocketTable[index].bufferSendMaxLength   = pLlcpTransport->pSocketTable[index].sSocketOption.miu;
                        pLlcpTransport->pSocketTable[index].bufferRwMaxLength     = pLlcpTransport->pSocketTable[index].sSocketOption.miu * ((pLlcpTransport->pSocketTable[index].sSocketOption.rw & PHLNLLCP_FRI_TLV_RW_MASK));
                        pLlcpTransport->pSocketTable[index].bufferLinearLength    = psWorkingBuffer->length - pLlcpTransport->pSocketTable[index].bufferSendMaxLength - pLlcpTransport->pSocketTable[index].bufferRwMaxLength;

                        /* Test the connection oriented buffers length */
                        if((pLlcpTransport->pSocketTable[index].bufferSendMaxLength + pLlcpTransport->pSocketTable[index].bufferRwMaxLength) > psWorkingBuffer->length
                            || ((pLlcpTransport->pSocketTable[index].bufferLinearLength < PHLNLLCP_FRI_MIU_DEFAULT) && (pLlcpTransport->pSocketTable[index].bufferLinearLength != 0)))
                        {
                            status = PH_ADD_COMPCODE(PH_ERR_BUFFER_TOO_SMALL, CID_FRI_NFC_LLCP_TRANSPORT);
                            return status;
                        }

                        /* Set the pointer and the length for the Receive Window Buffer */
                        for(cpt=0;cpt<pLlcpTransport->pSocketTable[index].localRW;cpt++)
                        {
                            pLlcpTransport->pSocketTable[index].sSocketRwBufferTable[cpt].buffer = psWorkingBuffer->buffer + (cpt*pLlcpTransport->pSocketTable[index].sSocketOption.miu);
                            pLlcpTransport->pSocketTable[index].sSocketRwBufferTable[cpt].length = 0;
                        }

                        /* Set the pointer and the length for the Send Buffer */
                        pLlcpTransport->pSocketTable[index].sSocketSendBuffer.buffer     = psWorkingBuffer->buffer + pLlcpTransport->pSocketTable[index].bufferRwMaxLength;
                        pLlcpTransport->pSocketTable[index].sSocketSendBuffer.length     = pLlcpTransport->pSocketTable[index].bufferSendMaxLength;

                        /** Set the pointer and the length for the Linear Buffer */
                        pLlcpTransport->pSocketTable[index].sSocketLinearBuffer.buffer   = psWorkingBuffer->buffer + pLlcpTransport->pSocketTable[index].bufferRwMaxLength + pLlcpTransport->pSocketTable[index].bufferSendMaxLength;
                        pLlcpTransport->pSocketTable[index].sSocketLinearBuffer.length   = pLlcpTransport->pSocketTable[index].bufferLinearLength;

                        if(pLlcpTransport->pSocketTable[index].sSocketLinearBuffer.length != 0)
                        {
                            /* Init Cyclic Fifo */
                            phlnLlcp_Fri_Utils_CyclicFifoInit(&pLlcpTransport->pSocketTable[index].sCyclicFifoBuffer,
                                                  pLlcpTransport->pSocketTable[index].sSocketLinearBuffer.buffer,
                                                    pLlcpTransport->pSocketTable[index].sSocketLinearBuffer.length);
                        }
                    }
                }
                /* Handle connectionless socket with buffering option */
                else if (eType == phlnLlcp_Fri_Transport_eConnectionLess)
                {
                    /* Determine how many packets can be bufferized in working buffer */
                    if (psWorkingBuffer != NULL)
                    {
                        /* NOTE: the extra byte is used to store SSAP */
                        pLlcpTransport->pSocketTable[index].localRW = (uint8_t) (psWorkingBuffer->length / (pLlcpTransport->pLlcp->sLocalParams.miu + 1));
                    }
                    else
                    {
                        pLlcpTransport->pSocketTable[index].localRW = 0;
                    }
                    if (pLlcpTransport->pSocketTable[index].localRW > PHLNLLCP_FRI_RW_MAX)
                    {
                        pLlcpTransport->pSocketTable[index].localRW = PHLNLLCP_FRI_RW_MAX;
                    }

                    /* Set the pointers and the lengths for buffering */
                    for(cpt=0 ; cpt<pLlcpTransport->pSocketTable[index].localRW ; cpt++)
                    {
                        pLlcpTransport->pSocketTable[index].sSocketRwBufferTable[cpt].buffer = psWorkingBuffer->buffer + (cpt*(pLlcpTransport->pLlcp->sLocalParams.miu + 1));
                        pLlcpTransport->pSocketTable[index].sSocketRwBufferTable[cpt].length = 0;
                    }

                    /* Set other socket internals */
                    pLlcpTransport->pSocketTable[index].indexRwRead      = 0;
                    pLlcpTransport->pSocketTable[index].indexRwWrite     = 0;
                }
                else
                {
                    /* Nothing to Do */
                }
                /* Store index of the socket */
                pLlcpTransport->pSocketTable[index].index = index;
                /* Set the socket into created state */
                pLlcpTransport->pSocketTable[index].eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketCreated;
                return status;
            }
            else
            {
                index++;
            }
        }while(index<PHLNLLCP_FRI_NB_SOCKET_MAX);

        status = PH_ADD_COMPCODE(PH_ERR_INSUFFICIENT_RESOURCES, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Close( phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket )  /* PRQA S 3209 */
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if (( pLlcpSocket != NULL)
        && (pLlcpSocket->psTransport != NULL)
        )
    {
        switch(pLlcpSocket->eSocket_Type )
        {
        case phlnLlcp_Fri_Transport_eConnectionOriented:
            {
                status = phlnLlcp_Fri_Transport_ConnectionOriented_Close(pLlcpSocket);
                break;
            }
        case phlnLlcp_Fri_Transport_eConnectionLess:
            {
                status = phlnLlcp_Fri_Transport_Connectionless_Close(pLlcpSocket);
                break;
            }
        default:
            {
                status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
                break;
            }
        }
        pLlcpSocket->psTransport->bSendPending = false;
        pLlcpSocket->psTransport->pfLinkSendCb = NULL;
        pLlcpSocket->psTransport->pLinkSendContext = NULL;
    }
    else
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Bind( phlnLlcp_Fri_Transport_Socket_t     *pLlcpSocket,
                                       uint8_t                               nSap,
                                       phNfc_sData_t                        *psServiceName)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t i;
    uint8_t min_sap_range;
    uint8_t max_sap_range;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketCreated)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        /* Calculate authorized SAP range */
        if ((psServiceName != NULL) && (psServiceName->length > 0))
        {
            /* SDP advertised service */
            min_sap_range = PHLNLLCP_FRI_SAP_SDP_ADVERTISED_FIRST;
            max_sap_range = PHLNLLCP_FRI_SAP_SDP_UNADVERTISED_FIRST;
        }
        else
        {
            /* Non-SDP advertised service */
            min_sap_range = PHLNLLCP_FRI_SAP_SDP_UNADVERTISED_FIRST;
            max_sap_range = PHLNLLCP_FRI_SAP_NUMBER;
        }

        /* Handle dynamic SAP allocation */
        if (nSap == 0)
        {
            status = phlnLlcp_Fri_Transport_GetFreeSap( pLlcpSocket->psTransport,
                psServiceName,
                &nSap);
            if (status != PH_ERR_SUCCESS)
            {
                return status;
            }
        }

        /* Test the SAP range */
        if(!IS_BETWEEN(nSap, min_sap_range, max_sap_range) &&
            !IS_BETWEEN(nSap, PHLNLLCP_FRI_SAP_WKS_FIRST, PHLNLLCP_FRI_SAP_SDP_ADVERTISED_FIRST))
        {
            status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        else
        {
            /* Test if the nSap it is used by another socket */
            for(i=0;i<PHLNLLCP_FRI_NB_SOCKET_MAX;i++)
            {
                if(pLlcpSocket->psTransport->pSocketTable[i].socket_sSap == nSap)
                {
                    return status = PH_ADD_COMPCODE(PH_ERR_ALREADY_REGISTERED, CID_FRI_NFC_LLCP_TRANSPORT);
                }
            }
            /* Set service name */
            status = phlnLlcp_Fri_Transport_RegisterName(pLlcpSocket, nSap, psServiceName);
            if (status != PH_ERR_SUCCESS)
            {
                return status;
            }
            /* Set the nSap value of the socket */
            pLlcpSocket->socket_sSap = nSap;
            /* Set the socket state */
            pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketBound;
        }
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Listen(phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
                                         pphlnLlcp_Fri_TransportSocketListenCb_t   pListen_Cb,
                                         void                                     *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL || pListen_Cb == NULL|| pContext == NULL )
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Check for socket state */
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Check for socket type */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if a listen is not pending with this socket */
    else if(pLlcpSocket->bSocketListenPending)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
      status = phlnLlcp_Fri_Transport_ConnectionOriented_Listen(pLlcpSocket,
                                                                pListen_Cb,
                                                                pContext);
    }
    return status;
}

static phStatus_t phlnLlcp_Fri_Transport_RegisterName( phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket,
                                                      uint8_t                             nSap,
                                                      phNfc_sData_t                      *psServiceName)
{
    phlnLlcp_Fri_Transport_t *       psTransport = pLlcpSocket->psTransport;
    uint8_t                          index;
    uint8_t                          bSnMatch, bSapMatch;
    phStatus_t                       status = PH_ERR_SUCCESS;

    /* Store the listen socket SN */
    static uint8_t ServiceName_buffer[256];

    PH_UNUSED_VARIABLE(status);

    /* Check in cache if sap has been used for different service name */
    for(index=0 ; index<PHLNLLCP_FRI_SDP_ADVERTISED_NB ; index++)
    {
        if(psTransport->pCachedServiceNames[index].sServiceName.length == 0)
        {
            /* Reached end of table */
            break;
        }

        bSnMatch = (uint8_t)(memcmp(psTransport->pCachedServiceNames[index].sServiceName.buffer, psServiceName->buffer, psServiceName->length) == 0);
        bSapMatch = psTransport->pCachedServiceNames[index].nSap == nSap;
        if(bSnMatch && bSapMatch)
        {
            /* Request match cache */
            break;
        }
        else if((bSnMatch && !bSapMatch) || (!bSnMatch && bSapMatch))
        {
            /* Request mismatch with cache */
            return PH_ERR_INVALID_PARAMETER;
        }
        else
        {
            /* Nothing to Do */
        }
    }

    /* Handle service with no name */
    if (psServiceName == NULL)
    {
        pLlcpSocket->sServiceName.length = 0;
    }
    else
    {
        /* Check if name already in use */
        for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
        {
            phlnLlcp_Fri_Transport_Socket_t* pCurrentSocket = &pLlcpSocket->psTransport->pSocketTable[index];

            if(   (pCurrentSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
                && (pCurrentSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketRegistered))
            {
                /* Only bound or listening sockets may have a service name */
                continue;  /* PRQA S 0770 */
            }

            if(pCurrentSocket->sServiceName.length != psServiceName->length) {
                /* Service name do not match, check next */
                continue;  /* PRQA S 0770 */
            }

            if(memcmp(pCurrentSocket->sServiceName.buffer, psServiceName->buffer, psServiceName->length) == 0)
            {
                /* Service name already in use */
                return PH_ERR_INVALID_PARAMETER;
            }
        }

        pLlcpSocket->sServiceName.buffer = ServiceName_buffer;
        pLlcpSocket->sServiceName.length = psServiceName->length;

        if (sizeof(ServiceName_buffer) < psServiceName->length)
        {
            return PHLNLLCP_FRI_NFCSTATUS_NOT_ENOUGH_MEMORY;
        }
        memcpy(pLlcpSocket->sServiceName.buffer, psServiceName->buffer, psServiceName->length); /* PRQA S 3200 */
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phlnLlcp_Fri_Transport_Accept(phlnLlcp_Fri_Transport_Socket_t             *pLlcpSocket,
                                         phlnLlcp_Fri_Transport_sSocketOptions_t     *psOptions,
                                         phNfc_sData_t                               *psWorkingBuffer,
                                         pphlnLlcp_Fri_TransportSocketErrCb_t         pErr_Cb,
                                         pphlnLlcp_Fri_TransportSocketAcceptCb_t      pAccept_RspCb,
                                         void                                        *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL || psOptions == NULL || psWorkingBuffer == NULL || pErr_Cb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Check for socket state */
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Check for socket type */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test the socket options */
    else if(psOptions->rw > PHLNLLCP_FRI_RW_MAX)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        /* Set the Max length for the Send and Receive Window Buffer */
        pLlcpSocket->bufferSendMaxLength   = psOptions->miu;
        pLlcpSocket->bufferRwMaxLength     = psOptions->miu * ((psOptions->rw & PHLNLLCP_FRI_TLV_RW_MASK));
        pLlcpSocket->bufferLinearLength    = psWorkingBuffer->length - pLlcpSocket->bufferSendMaxLength - pLlcpSocket->bufferRwMaxLength;

        /* Test the buffers length */
        if((pLlcpSocket->bufferSendMaxLength + pLlcpSocket->bufferRwMaxLength) > psWorkingBuffer->length
            || ((pLlcpSocket->bufferLinearLength < PHLNLLCP_FRI_MIU_DEFAULT)  && (pLlcpSocket->bufferLinearLength != 0)))
        {
            status = PH_ADD_COMPCODE(PH_ERR_BUFFER_TOO_SMALL, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        else
        {
            pLlcpSocket->psTransport->socketIndex = pLlcpSocket->index;

            status  = phlnLlcp_Fri_Transport_ConnectionOriented_Accept(pLlcpSocket,
                                                                    psOptions,
                                                                    psWorkingBuffer,
                                                                    pErr_Cb,
                                                                    pAccept_RspCb,
                                                                    pContext);
        }
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Reject( phlnLlcp_Fri_Transport_Socket_t         *pLlcpSocket,
                                         pphlnLlcp_Fri_TransportSocketRejectCb_t   pReject_RspCb,
                                         void                                     *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Check for socket state */
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Check for socket type */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        status = phlnLlcp_Fri_Transport_ConnectionOriented_Reject(pLlcpSocket,
                                                                pReject_RspCb,
                                                                pContext);
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Connect( phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
                                          uint8_t                                    nSap,
                                          pphlnLlcp_Fri_TransportSocketConnectCb_t   pConnect_RspCb,
                                          void                                      *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t nLocalSap;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL || pConnect_RspCb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test the port number value */
    else if(nSap<0x02 || nSap>63)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is a connectionOriented socket */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket has a service name */
    else if(pLlcpSocket->sServiceName.length != 0)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is not in connecting or connected state*/
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketCreated && pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        /* Implicit bind if socket is not already bound */
        if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
        {
            /* Bind to a free sap */
            status = phlnLlcp_Fri_Transport_GetFreeSap(pLlcpSocket->psTransport, NULL, &nLocalSap);
            if (status != PH_ERR_SUCCESS)
            {
                return status;
            }
            pLlcpSocket->socket_sSap = nLocalSap;
            pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketBound;
        }

        /* Test the SAP range for non SDP-advertised services */
        if(!IS_BETWEEN(pLlcpSocket->socket_sSap, PHLNLLCP_FRI_SAP_SDP_UNADVERTISED_FIRST, PHLNLLCP_FRI_SAP_NUMBER))
        {
            status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        else
        {
            status = phlnLlcp_Fri_Transport_ConnectionOriented_Connect(pLlcpSocket,
                                                                    nSap,
                                                                    NULL,
                                                                    pConnect_RspCb,
                                                                    pContext);
        }
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_ConnectByUri(phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                               phNfc_sData_t                             *psUri,
                                               pphlnLlcp_Fri_TransportSocketConnectCb_t   pConnect_RspCb,
                                               void                                      *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t nLocalSap;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL || pConnect_RspCb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is a connectionOriented socket */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is not in connect pending or connected state*/
    else if(pLlcpSocket->eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketConnecting || pLlcpSocket->eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketConnected)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test the length of the SN */
    else if(psUri->length > PHLNLLCP_FRI_SN_MAX_LENGTH)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        /* Implicit bind if socket is not already bound */
        if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
        {
            /* Bind to a free sap */
            status = phlnLlcp_Fri_Transport_GetFreeSap(pLlcpSocket->psTransport, NULL, &nLocalSap);
            if (status != PH_ERR_SUCCESS)
            {
                return status;
            }
            pLlcpSocket->socket_sSap = nLocalSap;
            pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketBound;
        }

        /* Test the SAP range for non SDP-advertised services */
        if(!IS_BETWEEN(pLlcpSocket->socket_sSap, PHLNLLCP_FRI_SAP_SDP_UNADVERTISED_FIRST, PHLNLLCP_FRI_SAP_NUMBER))
        {
            status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        else
        {
            status = phlnLlcp_Fri_Transport_ConnectionOriented_Connect(pLlcpSocket,
                                                                    PHLNLLCP_FRI_SAP_DEFAULT,
                                                                    psUri,
                                                                    pConnect_RspCb,
                                                                    pContext);
        }
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Disconnect( phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
                                             pphlnLlcp_Fri_LlcpSocketDisconnectCb_t     pDisconnect_RspCb,
                                             void                                      *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL || pDisconnect_RspCb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is a connectionOriented socket */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is connected  state*/
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketConnected)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        status = phlnLlcp_Fri_Transport_ConnectionOriented_Disconnect(pLlcpSocket,
                                                                    pDisconnect_RspCb,
                                                                    pContext);
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Send(phlnLlcp_Fri_Transport_Socket_t             *pLlcpSocket,
                                       phNfc_sData_t                               *psBuffer,
                                       pphlnLlcp_Fri_TransportSocketSendCb_t        pSend_RspCb,
                                       void                                        *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL || psBuffer == NULL || pSend_RspCb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is a connectionOriented socket */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is in connected state */
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketConnected)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test the length of the buffer */
    else if(psBuffer->length > pLlcpSocket->remoteMIU )
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if a send is pending */
    else if(pLlcpSocket->pfSocketSend_Cb != NULL)
    {
        status = PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        status = phlnLlcp_Fri_Transport_ConnectionOriented_Send(pLlcpSocket,
                                                              psBuffer,
                                                              pSend_RspCb,
                                                              pContext);
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_Recv( phlnLlcp_Fri_Transport_Socket_t        *pLlcpSocket,
                                       phNfc_sData_t                           *psBuffer,
                                       pphlnLlcp_Fri_TransportSocketRecvCb_t    pRecv_RspCb,
                                       void                                    *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Check for NULL pointers */
    if(pLlcpSocket == NULL || psBuffer == NULL || pRecv_RspCb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is a connectionOriented socket */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionOriented)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is in connected state */
    else if(pLlcpSocket->eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketDefault)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if a receive is pending */
    else if(pLlcpSocket->bSocketRecvPending == true)
    {
        status = PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        status = phlnLlcp_Fri_Transport_ConnectionOriented_Recv(pLlcpSocket,
                                                              psBuffer,
                                                              pRecv_RspCb,
                                                              pContext);
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_SendTo( phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                         uint8_t                                     nSap,
                                         phNfc_sData_t                              *psBuffer,
                                         pphlnLlcp_Fri_TransportSocketSendCb_t       pSend_RspCb,
                                         void                                       *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    phlnLlcp_Fri_sLinkParameters_t  LlcpRemoteLinkParamInfo;

    if(pLlcpSocket == NULL || psBuffer == NULL || pSend_RspCb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test the port number value */
    else if(nSap<2 || nSap>63)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is a connectionless socket */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionLess)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is in an updated state */
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if a send is pending */
    else if(pLlcpSocket->pfSocketSend_Cb != NULL)
    {
        status = PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        /* Get the local parameters of the LLCP Link */
        status = phlnLlcp_Fri_GetRemoteInfo(pLlcpSocket->psTransport->pLlcp,&LlcpRemoteLinkParamInfo);
        if(status != PH_ERR_SUCCESS)
        {
            status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        /* Test the length of the socket buffer for ConnectionLess mode*/
        else if(psBuffer->length > LlcpRemoteLinkParamInfo.miu)
        {
            status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        /* Test if the link is in error state */
        else if(pLlcpSocket->psTransport->LinkStatusError)
        {
            status = PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        else
        {
            status = phlnLlcp_Fri_Transport_Connectionless_SendTo(pLlcpSocket,
                                                               nSap,
                                                               psBuffer,
                                                               pSend_RspCb,
                                                               pContext);
        }
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Transport_RecvFrom( phlnLlcp_Fri_Transport_Socket_t             *pLlcpSocket,
                                           phNfc_sData_t                                *psBuffer,
                                           pphlnLlcp_Fri_TransportSocketRecvFromCb_t     pRecv_Cb,
                                           void                                         *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    if(pLlcpSocket == NULL || psBuffer == NULL || pRecv_Cb == NULL || pContext == NULL)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is a connectionless socket */
    else if(pLlcpSocket->eSocket_Type != phlnLlcp_Fri_Transport_eConnectionLess)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    /* Test if the socket is in an updated state */
    else if(pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketBound)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_STATE, CID_FRI_NFC_LLCP_TRANSPORT);
    }
    else
    {
        if(pLlcpSocket->bSocketRecvPending)
        {
            status = PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP_TRANSPORT);
        }
        else
        {
            status = phlnLlcp_Fri_Transport_Connectionless_RecvFrom(pLlcpSocket,
                                                                 psBuffer,
                                                                 pRecv_Cb,
                                                                 pContext);
        }
    }

    return status;
}
#endif /* NXPBUILD__PHLN_LLCP_FRI */
