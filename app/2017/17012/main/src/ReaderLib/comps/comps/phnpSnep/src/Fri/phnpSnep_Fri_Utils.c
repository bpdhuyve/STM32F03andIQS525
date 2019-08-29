/*
* =============================================================================
*
*                    Copyright (c), NXP Semiconductors
*
*                       (C)NXP Electronics N.V.2008
*         All rights are reserved. Reproduction in whole or in part is
*        prohibited without the written consent of the copyright owner.
*    NXP reserves the right to make changes without notice at any time.
*   NXP makes no warranty, expressed, implied or statutory, including but
*   not limited to any implied warranty of merchantability or fitness for any
*  particular purpose, or that the use will not infringe any third party patent,
*   copyright or trademark. NXP must not be liable for any loss or damage
*                            arising from its use.
*
* =============================================================================
*/

/*!
* =========================================================================== *
*                                                                             *
*                                                                             *
* \file  phnpSnep_Fri_Utils.c                                                 *
* \brief SNEP protocol management public interfaces listed in this module.    *
*                                                                             *
*                                                                             *
* Project:                                                                    *
*                                                                             *
* $Date: 2013-06-17 13:51:16 +0530 (Mon, 17 Jun 2013) $                       *
* $Author: nxp62726 $                                                         *
* $Revision: 172 $                                                            *
* $Aliases:  $                                                                *
*                                                                             *
* =========================================================================== */
#include <ph_Status.h>


#ifdef NXPBUILD__PHNP_SNEP_FRI
#include <phOsal.h>
#include "phnpSnep_Fri_Utils.h"
#include "phnpSnep_Fri_ContextMgmt.h"

static void phnpSnep_Fri_Utils_ClearMemNCallCb( phnpSnep_Fri_DataParams_t       *pDataParams,
                                                pphnpSnep_Fri_ClientSession_t    pClientSessionContext,
                                                phStatus_t                       status,
                                                phNfc_sData_t                   *pReqResponse);

static void phnpSnep_Fri_Utils_LlcpSocketReceiveCb( void            *pContext,
                                                    phStatus_t       status );

static void phnpSnep_Fri_Utils_LlcpSocketRecvCbForRecvBegin( void         *pContext,
                                                             phStatus_t    status);      /* PRQA S 0779 */

static void phnpSnep_Fri_Utils_LlcpSocketRecvCbForRspContinue( void         *pContext,
                                                               phStatus_t    status);    /* PRQA S 0779 */

static void phnpSnep_Fri_Utils_LlcpSocketRecvCbForReqContinue( void         *pContext,
                                                               phStatus_t    status);    /* PRQA S 0779 */

static void phnpSnep_Fri_Utils_LlcpSocketSendCbForReqContinue( void        *pContext,
                                                              phStatus_t    status);

static void phnpSnep_Fri_Utils_LlcpSocketSendCbForReqReject( void          *pContext,
                                                             phStatus_t     status);        /* PRQA S 0779 */

static void phnpSnep_Fri_Utils_LlcpSocketSendResponseCb( void         *pContext,
                                                         phStatus_t    status);        /* PRQA S 0779 */

static void phnpSnep_Fri_Utils_ServerSendcompleteInternal( void           *pContext,
                                                           phStatus_t      Status,
                                                           ph_NfcHandle    ConnHandle);

static void phnpSnep_Fri_Utils_ServerSendContinuecomplete( void           *pContext,
                                                           phStatus_t      status,
                                                           ph_NfcHandle    ConnHandle);

static void phnpSnep_Fri_Utils_SocketSendCb( void         *pContext,
                                             phStatus_t    status);



phStatus_t phnpSnep_Fri_Utils_Req( phnpSnep_Fri_DataParams_t      *pDataParams,
                                  pphnpSnep_Fri_ClientSession_t    pClientSessionContext)
{
    phStatus_t status  =  PH_ERR_SUCCESS;
    phlnLlcp_Fri_Transport_Socket_t *psLocalLlcpSocket =(phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle;

    /* Copy the Header Part of SNEP message */
    memcpy(pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer,   /* PRQA S 3200 */
        pClientSessionContext->putGetDataContext.pSnepHeader->buffer,
        pClientSessionContext->putGetDataContext.pSnepHeader->length);

    if (pClientSessionContext->putGetDataContext.bWaitForContinue)
    {
        /* Copy the Ndef message */
            memcpy(pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer + pClientSessionContext->putGetDataContext.pSnepHeader->length,  /* PRQA S 3200 */
                   pClientSessionContext->putGetDataContext.pSnepPacket->buffer,
                  (pClientSessionContext->iRemoteMiu - pClientSessionContext->putGetDataContext.pSnepHeader->length ));

        /* Send data depending upon Remote MIU */
        pClientSessionContext->putGetDataContext.pChunkingBuffer->length = pClientSessionContext->iRemoteMiu;

        pClientSessionContext->putGetDataContext.iDataSent = pClientSessionContext->iRemoteMiu;
    }
    else
    {
        /* Copy the Ndef message */
        memcpy(pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer+pClientSessionContext->putGetDataContext.pSnepHeader->length,  /* PRQA S 3200 */
               pClientSessionContext->putGetDataContext.pSnepPacket->buffer,
              (pClientSessionContext->putGetDataContext.pSnepPacket->length - pClientSessionContext->putGetDataContext.pSnepHeader->length ));

        pClientSessionContext->putGetDataContext.iDataSent = pClientSessionContext->putGetDataContext.pSnepPacket->length;

        pClientSessionContext->putGetDataContext.pChunkingBuffer->length = pClientSessionContext->putGetDataContext.pSnepPacket->length;
    }

    status = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                     (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                      pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                     &phnpSnep_Fri_Utils_SocketSendCb,
                                      pClientSessionContext
                                    );

    /* Response is driven from SNEP and LLCP layer should not respond with RR */
    psLocalLlcpSocket->bRecvReady_NotRequired=1;

    if (PH_ERR_PENDING != status && PH_ERR_SUCCESS != status)
    {
        phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                            pClientSessionContext,
                                            status,
                                            NULL);
    }

    return status;
}

phStatus_t phnpSnep_Fri_Utils_SendResponse( phnpSnep_Fri_DataParams_t         *pDataParams,
                                           pphnpSnep_Fri_ServerConnection_t   pSnepServerConnection)
{
    phStatus_t   ret = PH_ERR_SUCCESS;
    phlnLlcp_Fri_Transport_Socket_t *psLocalLlcpSocket = (phlnLlcp_Fri_Transport_Socket_t *) pSnepServerConnection->hSnepServerConnHandle;

    memcpy(pSnepServerConnection->responseDataContext.pChunkingBuffer->buffer,
        pSnepServerConnection->responseDataContext.pSnepHeader->buffer,
        pSnepServerConnection->responseDataContext.pSnepHeader->length);  /* PRQA S 3200 */

    if (pSnepServerConnection->responseDataContext.bWaitForContinue)
    {
        memcpy(pSnepServerConnection->responseDataContext.pChunkingBuffer->buffer + pSnepServerConnection->responseDataContext.pSnepHeader->length,
            pSnepServerConnection->responseDataContext.pSnepPacket->buffer,
            (pSnepServerConnection->iRemoteMiu - pSnepServerConnection->responseDataContext.pSnepHeader->length));  /* PRQA S 3200 */

        /* Send data depending upon Remote MIU */
        pSnepServerConnection->responseDataContext.pChunkingBuffer->length = pSnepServerConnection->iRemoteMiu;

        pSnepServerConnection->responseDataContext.iDataSent = pSnepServerConnection->iRemoteMiu;
    }
    else
    {
         memcpy(pSnepServerConnection->responseDataContext.pChunkingBuffer->buffer + pSnepServerConnection->responseDataContext.pSnepHeader->length,
                pSnepServerConnection->responseDataContext.pSnepPacket->buffer,
               (pSnepServerConnection->responseDataContext.pSnepPacket->length - pSnepServerConnection->responseDataContext.pSnepHeader->length));  /* PRQA S 3200 */

         pSnepServerConnection->responseDataContext.pChunkingBuffer->length = pSnepServerConnection->responseDataContext.pSnepPacket->length;

         pSnepServerConnection->responseDataContext.iDataSent = pSnepServerConnection->responseDataContext.pSnepPacket->length;
    }

    ret = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                  (phlnLlcp_Fri_Transport_Socket_t *) pSnepServerConnection->hSnepServerConnHandle,
                                   pSnepServerConnection->responseDataContext.pChunkingBuffer,
                                  &phnpSnep_Fri_Utils_LlcpSocketSendResponseCb,
                                   pSnepServerConnection
                                 );

           /* Response is driven from SNEP and LLCP layer should not respond with RR */
           psLocalLlcpSocket->bRecvReady_NotRequired = 1;

           if (PH_ERR_PENDING != ret)
           {
               phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                           ret);
           }

    return ret;
}

static void phnpSnep_Fri_Utils_SocketSendCb( void         *pContext,
                                            phStatus_t    status)
{
  
    phStatus_t  ret = PH_ERR_SUCCESS;
    uint32_t    iFragmentLength = 0;
    pphnpSnep_Fri_ClientSession_t       pClientSessionContext = (pphnpSnep_Fri_ClientSession_t)pContext;
    phnpSnep_Fri_DataParams_t          *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;
    phlnLlcp_Fri_Transport_Socket_t    *psLocalLlcpSocket =(phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle;

    if (pClientSessionContext->putGetDataContext.pChunkingBuffer == NULL)
    {
        /*pChunkingBuffer is occasionally NULL on fast-taps. */
        status = PH_ERR_FAILED;
    }

    if (PH_ERR_SUCCESS == status)
    {
        if (pClientSessionContext->putGetDataContext.pSnepPacket->length > pClientSessionContext->putGetDataContext.iDataSent)
        {
            /*Send remaining data, check for continue packet */
            if (pClientSessionContext->putGetDataContext.bWaitForContinue &&
                !pClientSessionContext->putGetDataContext.bContinueReceived)
            {
                /* Recv data depending upon Remote MIU */
                pClientSessionContext->putGetDataContext.pChunkingBuffer->length = pClientSessionContext->iMiu;

                ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                               (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                               pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                               &phnpSnep_Fri_Utils_LlcpSocketRecvCbForRspContinue,
                                               pClientSessionContext
                                             );

                if (PH_ERR_PENDING != ret)
                {
                    phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                        pClientSessionContext,
                                                        ret,
                                                        NULL);
                }
            }
            else
            {
                iFragmentLength = ((pClientSessionContext->putGetDataContext.pSnepPacket->length - pClientSessionContext->putGetDataContext.iDataSent) < pClientSessionContext->iRemoteMiu)
                        ? (pClientSessionContext->putGetDataContext.pSnepPacket->length - pClientSessionContext->putGetDataContext.iDataSent) : (pClientSessionContext->iRemoteMiu);

                memcpy( pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer,
                        pClientSessionContext->putGetDataContext.pSnepPacket->buffer + pClientSessionContext->putGetDataContext.iDataSent - pClientSessionContext->putGetDataContext.pSnepHeader->length,
                        iFragmentLength);  /* PRQA S 3200 */

                pClientSessionContext->putGetDataContext.pChunkingBuffer->length = iFragmentLength;
                pClientSessionContext->putGetDataContext.iDataSent += iFragmentLength;

                if (true == pClientSessionContext->putGetDataContext.bWaitForContinue)
                {
                    ret = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                                  (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                                   pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                                  &phnpSnep_Fri_Utils_SocketSendCb,
                                                  pClientSessionContext
                                                 );
                    /* Response is driven from SNEP and LLCP layer should not respond with RR */
                    psLocalLlcpSocket->bRecvReady_NotRequired=1;

                    if (PH_ERR_PENDING != ret)
                    {
                        phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                            pClientSessionContext,
                                                            ret,
                                                            NULL);
                    }
                    else
                    {
                        pClientSessionContext->putGetDataContext.bWaitForContinue = false;
                    }
                }
                else
                {
                    phnpSnep_Fri_Utils_SocketSendData( pContext,
                                                       PH_ERR_SUCCESS);
                }
            }
        }
        else
        {
            /* start the reply collection*/
            status = phnpSnep_Fri_Utils_CollectReply( pClientSessionContext );
        }
    }
    else
    {
        phnpSnep_Fri_Utils_ClearMemNCallCb(pDataParams, pClientSessionContext, status, NULL);
    }
}

static void phnpSnep_Fri_Utils_LlcpSocketRecvCbForRspContinue( void        *pContext,
                                                              phStatus_t    status)
{
    uint8_t *pSnepPktTraverse = NULL;
    pphnpSnep_Fri_ClientSession_t    pClientSessionContext = (pphnpSnep_Fri_ClientSession_t)pContext;
    phnpSnep_Fri_DataParams_t       *pDataParams =  (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;
    PH_UNUSED_VARIABLE(status);

    pSnepPktTraverse = pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer;

    if (PH_ERR_SUCCESS == status &&
        PHNPSNEP_FRI_RESPONSE_CONTINUE == pSnepPktTraverse[PHNPSNEP_FRI_VERSION_LENGTH])
    {
        if( phnpSnep_Fri_Utils_VersionsCompatibleChk( pClientSessionContext->SnepClientVersion,
                                                      *pSnepPktTraverse)
                                                     )
        {
            pClientSessionContext->putGetDataContext.bContinueReceived = true;
            phnpSnep_Fri_Utils_SocketSendCb( pClientSessionContext,
                                             PH_ERR_SUCCESS);
        }
        else
        {
            pClientSessionContext->putGetDataContext.bWaitForContinue = false;

            phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                pClientSessionContext,
                                                PHNPSNEP_FRI_RESPONSE_UNSUPPORTED_VERSION,
                                                NULL);
        }
    }
    else
    {
        pClientSessionContext->putGetDataContext.bWaitForContinue = false;

        if( phnpSnep_Fri_Utils_VersionsCompatibleChk( pClientSessionContext->SnepClientVersion,
                                                      *pSnepPktTraverse) )
        {
            /* PHNPSNEP_FRI_RESPONSE_REJECT will be received in this path */
            phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                pClientSessionContext,
                                                phnpSnep_Fri_Utils_GetNfcStatusFromSnepResponse( pSnepPktTraverse[PHNPSNEP_FRI_VERSION_LENGTH]),
                                                NULL);
        }
        else
        {
            phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                pClientSessionContext,
                                                PHNPSNEP_FRI_RESPONSE_UNSUPPORTED_VERSION,
                                                NULL);
        }
    }
}

/*@TODO Remove once LLCP Send issue resolved */
void phnpSnep_Fri_Utils_SocketSendData( void         *pContext,
                                        phStatus_t    status)
{
    phStatus_t  ret = PH_ERR_SUCCESS;
    pphnpSnep_Fri_ClientSession_t      pClientSessionContext = (pphnpSnep_Fri_ClientSession_t)pContext;
    phnpSnep_Fri_DataParams_t         *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;
    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket =(phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle;

    if (NULL != pClientSessionContext &&
         PH_ERR_SUCCESS == status)
    {
        ret = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                       (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                       pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                       &phnpSnep_Fri_Utils_SocketSendCb,
                                       pClientSessionContext
                                );
        /* Response is driven from SNEP and LLCP layer should not respond with RR */
        psLocalLlcpSocket->bRecvReady_NotRequired=1;

        if (PH_ERR_PENDING != ret)
        {
            phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                pClientSessionContext,
                                                ret,
                                                NULL);
        }
    }
}

/*@TODO Remove once LLCP Receive issue resolved */
void phnpSnep_Fri_Utils_SocketReceiveData( void         *pContext,
                                           phStatus_t    status)
{
    phStatus_t  ret = PH_ERR_SUCCESS;
    pphnpSnep_Fri_ServerConnection_t   pServerConnectionContext = (pphnpSnep_Fri_ServerConnection_t)pContext;
    phnpSnep_Fri_DataParams_t         *pDataParams = (phnpSnep_Fri_DataParams_t *) pServerConnectionContext->pSnepDataParamsContext;

    PH_UNUSED_VARIABLE(ret);

    if (NULL != pServerConnectionContext &&
         PH_ERR_SUCCESS == status)
    {
        ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                      (phlnLlcp_Fri_Transport_Socket_t *) pServerConnectionContext->hSnepServerConnHandle,
                                       pServerConnectionContext->responseDataContext.pChunkingBuffer,
                                       &phnpSnep_Fri_Utils_SocketRecvCbForServer,
                                       pServerConnectionContext);
    }
}

/*@TODO Remove once LLCP Send issue resolved */
void phnpSnep_Fri_Utils_SocketSendDataServer( void        *pContext,
                                              phStatus_t   status)
{
    phStatus_t  ret = PH_ERR_SUCCESS;
    pphnpSnep_Fri_ServerConnection_t    pServerConnectionContext = (pphnpSnep_Fri_ServerConnection_t)pContext;
    phnpSnep_Fri_DataParams_t          *pDataParams = (phnpSnep_Fri_DataParams_t *) pServerConnectionContext->pSnepDataParamsContext;
    phlnLlcp_Fri_Transport_Socket_t    *psLocalLlcpSocket =(phlnLlcp_Fri_Transport_Socket_t *) pServerConnectionContext->hSnepServerConnHandle;

    if (NULL != pServerConnectionContext &&
        PH_ERR_SUCCESS == status)
    {
         ret = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                        (phlnLlcp_Fri_Transport_Socket_t *) pServerConnectionContext->hSnepServerConnHandle,
                                        pServerConnectionContext->responseDataContext.pChunkingBuffer,
                                        &phnpSnep_Fri_Utils_SocketSendCb,
                                        pServerConnectionContext
                                      );
        /* Response is driven from SNEP and LLCP layer should not respond with RR */
        psLocalLlcpSocket->bRecvReady_NotRequired=1;

        if (PH_ERR_PENDING != ret)
        {
            phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pServerConnectionContext,
                                                        ret);
        }
    }
}

static void phnpSnep_Fri_Utils_LlcpSocketSendResponseCb( void         *pContext,
                                                         phStatus_t    status)
{
    phStatus_t  ret = PH_ERR_SUCCESS;
    uint32_t iFragmentLength = 0;
    phnpSnep_Fri_ServerConnection_t    *pSnepServerConnection = (phnpSnep_Fri_ServerConnection_t *)pContext;
    phnpSnep_Fri_DataParams_t          *pDataParams = (phnpSnep_Fri_DataParams_t *) pSnepServerConnection->pSnepDataParamsContext;
    phlnLlcp_Fri_Transport_Socket_t    *psLocalLlcpSocket = (phlnLlcp_Fri_Transport_Socket_t *) pSnepServerConnection->hSnepServerConnHandle;

    pSnepServerConnection->responseDataContext.pChunkingBuffer->length = pSnepServerConnection->iMiu;

    if (PH_ERR_SUCCESS == status)
    {
        if (pSnepServerConnection->responseDataContext.pSnepPacket->length > pSnepServerConnection->responseDataContext.iDataSent)
        {
            /*Send remaining data, check for continue packet */
            if (pSnepServerConnection->responseDataContext.bWaitForContinue)
            {
                if (!pSnepServerConnection->responseDataContext.bContinueReceived)
                {
                    ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                                   (phlnLlcp_Fri_Transport_Socket_t *) pSnepServerConnection->hSnepServerConnHandle,
                                                   pSnepServerConnection->responseDataContext.pChunkingBuffer,
                                                   &phnpSnep_Fri_Utils_LlcpSocketRecvCbForReqContinue,
                                                   pSnepServerConnection);
                    if (PH_ERR_PENDING != ret)
                    {
                        phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                                    ret);
                    }
                }
                else
                {
                    iFragmentLength = ((pSnepServerConnection->responseDataContext.pSnepPacket->length - pSnepServerConnection->responseDataContext.iDataSent) < pSnepServerConnection->iRemoteMiu)
                        ? (pSnepServerConnection->responseDataContext.pSnepPacket->length - pSnepServerConnection->responseDataContext.iDataSent) : (pSnepServerConnection->iRemoteMiu);

                    memcpy( pSnepServerConnection->responseDataContext.pChunkingBuffer->buffer,
                            pSnepServerConnection->responseDataContext.pSnepPacket->buffer + pSnepServerConnection->responseDataContext.iDataSent - pSnepServerConnection->responseDataContext.pSnepHeader->length,
                            iFragmentLength);  /* PRQA S 3200 */

                    pSnepServerConnection->responseDataContext.pChunkingBuffer->length = iFragmentLength;
                    pSnepServerConnection->responseDataContext.iDataSent += iFragmentLength;

                    ret = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                                   (phlnLlcp_Fri_Transport_Socket_t *) pSnepServerConnection->hSnepServerConnHandle,
                                                   pSnepServerConnection->responseDataContext.pChunkingBuffer,
                                                   &phnpSnep_Fri_Utils_LlcpSocketSendResponseCb,
                                                   pSnepServerConnection);

                    /* Response is driven from SNEP and LLCP layer should not respond with RR */
                    psLocalLlcpSocket->bRecvReady_NotRequired=1;

                    if (PH_ERR_PENDING != ret)
                    {
                        phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                                    ret);
                    }
                }
            }
        }
        else
        {
            if (pSnepServerConnection->responseDataContext.bIsExcessData)
            {
                phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                            PHNPSNEP_FRI_STATUS_RESPONSE_EXCESS_DATA);
            }
            else
            {
                phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                            ret);
            }
        }
    }
    else
    {
        phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                    status);
    }
}

phStatus_t  phnpSnep_Fri_Utils_PrepareSnepPacket( phnpSnep_Fri_DataParams_t    *pDataParams,
                                                 phnpSnep_Fri_Packet_t         packetType,
                                                 phNfc_sData_t                *pData,
                                                 phNfc_sData_t                *pSnepPacket,
                                                 phNfc_sData_t                *pSnepHeader,
                                                 uint8_t                       version,
                                                 uint32_t                      acceptableLength)
{
    phStatus_t    status = PH_ERR_SUCCESS;
    uint8_t      *pSnepPktTraverse = NULL;
    uint8_t      *pBuffer = 0;
    uint32_t      iSnepPacketSize = 0;

    if (NULL != pData)
    {
        iSnepPacketSize = PHNPSNEP_FRI_HEADER_SIZE + pData->length;
    }
    else
    {
        iSnepPacketSize = PHNPSNEP_FRI_HEADER_SIZE;
    }

    if (phnpSnep_Fri_Get == packetType)
    {
        /* For Get Request from Client */
        iSnepPacketSize += PHNPSNEP_FRI_REQUEST_ACCEPTABLE_LENGTH_SIZE;
    }

    if (NULL != pSnepHeader && NULL != pSnepHeader->buffer)
    {

        pSnepPktTraverse = pSnepHeader->buffer;
        memset(pSnepPktTraverse, 0, pSnepHeader->length);  /* PRQA S 3200 */

        *pSnepPktTraverse = version;
        pSnepPktTraverse += PHNPSNEP_FRI_VERSION_LENGTH;

        *pSnepPktTraverse = phnpSnep_Fri_Utils_GetSnepResponseCodeFromPacketType( packetType );

        pSnepPktTraverse += PHNPSNEP_FRI_REQUEST_LENGTH;

        if (NULL != pData)
        {
            /* Big endian length */
            pBuffer = (uint8_t*)&(pData->length);
           *pSnepPktTraverse = pBuffer[3];
            pSnepPktTraverse++;
           *pSnepPktTraverse = pBuffer[2];
            pSnepPktTraverse++;
           *pSnepPktTraverse = pBuffer[1];
            pSnepPktTraverse++;
           *pSnepPktTraverse = pBuffer[0];
            pSnepPktTraverse++;
        }
        else
        {
            *((uint32_t *)pSnepPktTraverse) = 0x00;
        }

        pSnepHeader->length = PHNPSNEP_FRI_HEADER_SIZE;

        if (phnpSnep_Fri_Get == packetType)
        {
            pBuffer = (uint8_t*)&acceptableLength;
           *pSnepPktTraverse = pBuffer[3];
            pSnepPktTraverse++;
           *pSnepPktTraverse = pBuffer[2];
            pSnepPktTraverse++;
           *pSnepPktTraverse = pBuffer[1];
            pSnepPktTraverse++;
           *pSnepPktTraverse = pBuffer[0];
            pSnepPktTraverse++;

            pSnepHeader->length = PHNPSNEP_FRI_HEADER_SIZE + PHNPSNEP_FRI_REQUEST_ACCEPTABLE_LENGTH_SIZE;
        }

        if (NULL != pSnepPacket )
        {
            pSnepPacket->length = iSnepPacketSize;

            if (NULL != pData)
            {
                pSnepPacket->buffer = pData->buffer;
            }
        }
    }
    else
    {
        if (NULL != pSnepPacket)
        {
            pSnepPacket->length = 0;
        }
    }

    return status;
}

static void phnpSnep_Fri_Utils_LlcpSocketRecvCbForReqContinue( void           *pContext,
                                                              phStatus_t      status)
{

    uint8_t      *pSnepPktTraverse = NULL;
    phStatus_t    temp = PH_ERR_SUCCESS;

    phnpSnep_Fri_ServerConnection_t   *pSnepServerConnection = (phnpSnep_Fri_ServerConnection_t *) pContext;

    PH_UNUSED_VARIABLE(status);
    PH_UNUSED_VARIABLE(temp);

    pSnepPktTraverse = pSnepServerConnection->responseDataContext.pChunkingBuffer->buffer;

    if (PHNPSNEP_FRI_REQUEST_CONTINUE == pSnepPktTraverse[PHNPSNEP_FRI_VERSION_LENGTH])
    {
        if( phnpSnep_Fri_Utils_VersionsCompatibleChk( pSnepServerConnection->SnepServerVersion,
                                                     *pSnepPktTraverse) )
        {
            pSnepServerConnection->responseDataContext.bContinueReceived = true;
            phnpSnep_Fri_Utils_LlcpSocketSendResponseCb( pSnepServerConnection,
                                                         PH_ERR_SUCCESS);
        }
        else
        {
            pSnepServerConnection->responseDataContext.bWaitForContinue = false;

            /* send Reject, in completion reset context and restart read */
            status = phnpSnep_Fri_ServerSendResponse ( pSnepServerConnection->pSnepDataParamsContext ,
                                              pSnepServerConnection->hSnepServerConnHandle,
                                              NULL,
                                              PHNPSNEP_FRI_STATUS_RESPONSE_UNSUPPORTED_VERSION,
                                              &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                              pSnepServerConnection);

        }
    }
    else if (PHNPSNEP_FRI_REQUEST_REJECT == pSnepPktTraverse[PHNPSNEP_FRI_VERSION_LENGTH])
    {
        pSnepServerConnection->responseDataContext.bWaitForContinue = false;
        if( phnpSnep_Fri_Utils_VersionsCompatibleChk( pSnepServerConnection->SnepServerVersion,
                                                     *pSnepPktTraverse) )
        {
            phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                        PHNPSNEP_FRI_STATUS_REQUEST_REJECT);
        }
        else
        {

            phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                        PHNPSNEP_FRI_STATUS_RESPONSE_UNSUPPORTED_VERSION);

            /* send Reject, in completion reset context and restart read */
            status = phnpSnep_Fri_ServerSendResponse ( pSnepServerConnection->pSnepDataParamsContext,
                                                       pSnepServerConnection->hSnepServerConnHandle,
                                                       NULL,
                                                       PHNPSNEP_FRI_STATUS_RESPONSE_UNSUPPORTED_VERSION,
                                                      &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                                       pSnepServerConnection
                                                     );
        }
    }
    else
    {
        phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pSnepServerConnection,
                                                    PHNPSNEP_FRI_STATUS_INVALID_PROTOCOL_DATA);

        status = phnpSnep_Fri_ServerSendResponse ( pSnepServerConnection->pSnepDataParamsContext,
                                                   pSnepServerConnection->hSnepServerConnHandle,
                                                   NULL,
                                                   PHNPSNEP_FRI_STATUS_RESPONSE_BAD_REQUEST,
                                                  &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                                   pSnepServerConnection
                                                 );

    }
}


phStatus_t phnpSnep_Fri_Utils_CollectReply( pphnpSnep_Fri_ClientSession_t   pClientSessionContext)     /* PRQA S 3209 */
{
    phStatus_t  ret = PH_ERR_SUCCESS;

    pClientSessionContext->putGetDataContext.pChunkingBuffer->length = pClientSessionContext->iMiu; /* Receive the data depending upon local MIU size */
    phnpSnep_Fri_DataParams_t   *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;

    ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                   (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                   pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                   &phnpSnep_Fri_Utils_LlcpSocketRecvCbForRecvBegin,
                                   pClientSessionContext
                                 );

    if (PH_ERR_PENDING != ret &&
            PH_ERR_SUCCESS != ret)
    {
        phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                            pClientSessionContext,
                                            ret,
                                            NULL);
    }
    return ret;
}

static void phnpSnep_Fri_Utils_LlcpSocketRecvCbForRecvBegin( void         *pContext,
                                                             phStatus_t    status)
{
    uint8_t    *pSnepPktTraverse = NULL;
    uint8_t    *pdata_len        = NULL;
    uint8_t    *pConvert         = NULL;
    uint32_t    iInfoLength      = 0;

    pphnpSnep_Fri_ClientSession_t    pClientSessionContext = (pphnpSnep_Fri_ClientSession_t)pContext;
    phnpSnep_Fri_DataParams_t       *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;

    if (PH_ERR_SUCCESS ==  status)
    {
        pSnepPktTraverse = pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer;
        pdata_len = (pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer + PHNPSNEP_FRI_REQUEST_LENGTH + PHNPSNEP_FRI_VERSION_LENGTH);
        pConvert = (uint8_t*)&iInfoLength;
        pConvert[3] = *pdata_len;
        pdata_len++;
        pConvert[2] = *pdata_len;
        pdata_len++;
        pConvert[1] = *pdata_len;
        pdata_len++;
        pConvert[0] = *pdata_len;

        if (0 == iInfoLength)
        {
            phnpSnep_Fri_Utils_ClearMemNCallCb ( pDataParams,
                                                 pClientSessionContext,
                                                 phnpSnep_Fri_Utils_GetNfcStatusFromSnepResponse( pSnepPktTraverse[PHNPSNEP_FRI_VERSION_LENGTH] ),
                                                 NULL);
        }
        else
        {
            if (NULL != pClientSessionContext->putGetDataContext.pReqResponse &&
                NULL != pClientSessionContext->putGetDataContext.pReqResponse->buffer)
            {
                pClientSessionContext->putGetDataContext.pReqResponse->length = iInfoLength;
                if((iInfoLength + PHNPSNEP_FRI_HEADER_SIZE) <= pClientSessionContext->iMiu)
                {
                    pClientSessionContext->putGetDataContext.pReqResponse->length = iInfoLength;

                    memcpy(pClientSessionContext->putGetDataContext.pReqResponse->buffer, pSnepPktTraverse + PHNPSNEP_FRI_HEADER_SIZE, iInfoLength);  /* PRQA S 3200 */

                    phnpSnep_Fri_Utils_ClearMemNCallCb ( pDataParams,
                                                         pClientSessionContext,
                                                         phnpSnep_Fri_Utils_GetNfcStatusFromSnepResponse( pSnepPktTraverse[PHNPSNEP_FRI_VERSION_LENGTH] ),
                                                         pClientSessionContext->putGetDataContext.pReqResponse);
                }
                else
                {
                    if( phnpSnep_Fri_Utils_VersionsCompatibleChk( pClientSessionContext->SnepClientVersion,
                                                                 *pSnepPktTraverse) )
                    {
                        /*send request continue, and in callback start receive again*/
                        memcpy( pClientSessionContext->putGetDataContext.pReqResponse->buffer,pSnepPktTraverse + PHNPSNEP_FRI_HEADER_SIZE, pClientSessionContext->putGetDataContext.pChunkingBuffer->length - PHNPSNEP_FRI_HEADER_SIZE);  /* PRQA S 3200 */

                        pClientSessionContext->putGetDataContext.iDataReceived = (pClientSessionContext->putGetDataContext.pChunkingBuffer->length - PHNPSNEP_FRI_HEADER_SIZE);
                        phnpSnep_Fri_Utils_SendRequestContinue(pClientSessionContext);
                    }
                    else
                    {
                        /*send request Reject, and in callback start receive again*/
                        pClientSessionContext->putGetDataContext.iDataReceived = 0;
                        phnpSnep_Fri_Utils_SendRequestReject(pClientSessionContext);
                    }
                }
            }
            else
            {
                phnpSnep_Fri_Utils_SendRequestReject(pClientSessionContext);
            }
        }
    }
    else
    {
        phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                            pClientSessionContext,
                                            status,
                                            NULL);
    }
}

phStatus_t phnpSnep_Fri_Utils_GetNfcStatusFromSnepResponse( uint8_t snepCode )
{
    phStatus_t  ret = PHNPSNEP_FRI_STATUS_INVALID_PROTOCOL_DATA;

    switch(snepCode)
    {
    case PHNPSNEP_FRI_RESPONSE_CONTINUE:
        ret = PHNPSNEP_FRI_STATUS_RESPONSE_CONTINUE;
        break;
    case PHNPSNEP_FRI_RESPONSE_SUCCESS:
        ret = PH_ERR_SUCCESS;
        break;
    case PHNPSNEP_FRI_RESPONSE_NOT_FOUND:
        ret = PHNPSNEP_FRI_STATUS_RESPONSE_NOT_FOUND;
        break;
    case PHNPSNEP_FRI_RESPONSE_EXCESS_DATA:
        ret = PHNPSNEP_FRI_STATUS_RESPONSE_EXCESS_DATA;
        break;
    case PHNPSNEP_FRI_RESPONSE_BAD_REQUEST:
        ret = PHNPSNEP_FRI_STATUS_RESPONSE_BAD_REQUEST;
        break;
    case PHNPSNEP_FRI_RESPONSE_NOT_IMPLEMENTED:
        ret = PHNPSNEP_FRI_STATUS_RESPONSE_NOT_IMPLEMENTED;
        break;
    case PHNPSNEP_FRI_RESPONSE_UNSUPPORTED_VERSION:
        ret = PHNPSNEP_FRI_STATUS_RESPONSE_UNSUPPORTED_VERSION;
        break;
    case PHNPSNEP_FRI_RESPONSE_REJECT:
        ret = PHNPSNEP_FRI_STATUS_RESPONSE_REJECT;
        break;
    default:
        ret = PHNPSNEP_FRI_STATUS_INVALID_PROTOCOL_DATA;
    }
    return ret;
}

uint8_t phnpSnep_Fri_Utils_GetSnepResponseCodeFromPacketType( phnpSnep_Fri_Packet_t   packetType )
{
    uint8_t ret = PHNPSNEP_FRI_RESPONSE_REJECT;
    switch(packetType)
    {
    case phnpSnep_Fri_Put:
        ret = PHNPSNEP_FRI_REQUEST_PUT;
        break;
    case phnpSnep_Fri_Get:
        ret = PHNPSNEP_FRI_REQUEST_GET;
        break;
    case phnpSnep_Fri_Continue:
        ret = PHNPSNEP_FRI_RESPONSE_CONTINUE;
        break;
    case phnpSnep_Fri_Success:
        ret = PHNPSNEP_FRI_RESPONSE_SUCCESS;
        break;
    case phnpSnep_Fri_NotFound:
        ret = PHNPSNEP_FRI_RESPONSE_NOT_FOUND;
        break;
    case phnpSnep_Fri_ExcessData:
        ret = PHNPSNEP_FRI_RESPONSE_EXCESS_DATA;
        break;
    case phnpSnep_Fri_BadRequest:
        ret = PHNPSNEP_FRI_RESPONSE_BAD_REQUEST;
        break;
    case phnpSnep_Fri_NotImplemented:
        ret = PHNPSNEP_FRI_RESPONSE_NOT_IMPLEMENTED;
        break;
    case phnpSnep_Fri_UnsupportedVersion:
        ret = PHNPSNEP_FRI_RESPONSE_UNSUPPORTED_VERSION;
        break;
    case phnpSnep_Fri_Reject:
        ret = PHNPSNEP_FRI_RESPONSE_REJECT;
        break;
    default:
        ret = PHNPSNEP_FRI_RESPONSE_REJECT;
    }
    return ret;
}

phnpSnep_Fri_Packet_t phnpSnep_Fri_Utils_GetPacketType( phStatus_t    nfcStatus )
{
    phnpSnep_Fri_Packet_t ret = phnpSnep_Fri_Invalid;
    switch(nfcStatus)
    {
    case PHNPSNEP_FRI_STATUS_RESPONSE_CONTINUE:
        ret = phnpSnep_Fri_Continue;
        break;
    case PH_ERR_SUCCESS:
        ret = phnpSnep_Fri_Success;
        break;
    case PHNPSNEP_FRI_STATUS_RESPONSE_NOT_FOUND:
        ret = phnpSnep_Fri_NotFound;
        break;
    case PHNPSNEP_FRI_STATUS_RESPONSE_EXCESS_DATA:
        ret = phnpSnep_Fri_ExcessData;
        break;
    case PHNPSNEP_FRI_STATUS_RESPONSE_BAD_REQUEST:
        ret = phnpSnep_Fri_BadRequest;
        break;
    case PHNPSNEP_FRI_STATUS_RESPONSE_NOT_IMPLEMENTED:
        ret = phnpSnep_Fri_NotImplemented;
        break;
    case PHNPSNEP_FRI_STATUS_RESPONSE_UNSUPPORTED_VERSION:
        ret = phnpSnep_Fri_UnsupportedVersion;
        break;
    case PHNPSNEP_FRI_STATUS_RESPONSE_REJECT:
        ret = phnpSnep_Fri_Reject;
        break;
    default:
        ret = phnpSnep_Fri_Invalid;
    }
    return ret;
}

void phnpSnep_Fri_Utils_SendRequestContinue( pphnpSnep_Fri_ClientSession_t    pClientSessionContext )
{
    phStatus_t  ret = PH_ERR_SUCCESS;
    uint8_t *pSnepPktTraverse = pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer ;
    phnpSnep_Fri_DataParams_t         *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;
    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket =  (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle;

    PH_UNUSED_VARIABLE(ret);

    memset(pSnepPktTraverse, 0, pClientSessionContext->iRemoteMiu);  /* PRQA S 3200 */

    *pSnepPktTraverse = PHNPSNEP_FRI_VERSION;
    pSnepPktTraverse += PHNPSNEP_FRI_VERSION_LENGTH;

    *pSnepPktTraverse = PHNPSNEP_FRI_REQUEST_CONTINUE;
    pSnepPktTraverse += PHNPSNEP_FRI_REQUEST_LENGTH;
    pClientSessionContext->putGetDataContext.pChunkingBuffer->length = PHNPSNEP_FRI_HEADER_SIZE;

    ret = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                   (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                   pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                   &phnpSnep_Fri_Utils_LlcpSocketSendCbForReqContinue,
                                   pClientSessionContext);

    /* Response is driven from SNEP and LLCP layer should not respond with RR */
    psLocalLlcpSocket->bRecvReady_NotRequired = 1;
}

static void phnpSnep_Fri_Utils_LlcpSocketSendCbForReqContinue( void        *pContext,
                                                              phStatus_t    status)
{
    pphnpSnep_Fri_ClientSession_t    pClientSessionContext = (pphnpSnep_Fri_ClientSession_t)pContext;
    phnpSnep_Fri_DataParams_t       *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;

    if (PH_ERR_SUCCESS == status)
    {
        pClientSessionContext->putGetDataContext.pChunkingBuffer->length = 0;
        phnpSnep_Fri_Utils_LlcpSocketReceiveCb( pClientSessionContext,
                                                PH_ERR_SUCCESS);
    }
    else
    {
        phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                            pClientSessionContext,
                                            PHNPSNEP_FRI_STATUS_REQUEST_CONTINUE_FAILED,
                                            NULL);
    }
}


void phnpSnep_Fri_Utils_SendRequestReject( pphnpSnep_Fri_ClientSession_t   pClientSessionContext )
{
    phStatus_t   ret = PH_ERR_SUCCESS;
    uint8_t     *pSnepPktTraverse = pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer ;
    PH_UNUSED_VARIABLE(ret);

    phnpSnep_Fri_DataParams_t         *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;
    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket =  (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle;

    memset(pSnepPktTraverse, 0, pClientSessionContext->iRemoteMiu);  /* PRQA S 3200 */

    *pSnepPktTraverse = PHNPSNEP_FRI_VERSION;
    pSnepPktTraverse += PHNPSNEP_FRI_VERSION_LENGTH;

    *pSnepPktTraverse = PHNPSNEP_FRI_REQUEST_REJECT;
    pSnepPktTraverse += PHNPSNEP_FRI_REQUEST_LENGTH;
    pClientSessionContext->putGetDataContext.pChunkingBuffer->length = PHNPSNEP_FRI_HEADER_SIZE;

    ret = phlnLlcp_Transport_Send( pDataParams->plnLlcpDataParams,
                                   (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                   pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                   &phnpSnep_Fri_Utils_LlcpSocketSendCbForReqReject,
                                   pClientSessionContext);

    /* Response is driven from SNEP and LLCP layer should not respond with RR */
    psLocalLlcpSocket->bRecvReady_NotRequired = 1;
}

static void phnpSnep_Fri_Utils_LlcpSocketSendCbForReqReject( void             *pContext,
                                                             phStatus_t        status)
{
    pphnpSnep_Fri_ClientSession_t    pClientSessionContext = (pphnpSnep_Fri_ClientSession_t)pContext;
    phnpSnep_Fri_DataParams_t       *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;

    if (PH_ERR_SUCCESS == status)
    {
        status = PHNPSNEP_FRI_STATUS_REQUEST_REJECT;
    }
    else
    {
        status = PHNPSNEP_FRI_STATUS_REQUEST_REJECT_FAILED;
    }
    phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                        pClientSessionContext,
                                        status,
                                        NULL);
}

static void phnpSnep_Fri_Utils_LlcpSocketReceiveCb( void         *pContext,
                                                    phStatus_t    status )
{
    uint8_t      *pSnepPktTraverse = NULL;
    phStatus_t    ret = PH_ERR_SUCCESS;

    pphnpSnep_Fri_ClientSession_t    pClientSessionContext = (pphnpSnep_Fri_ClientSession_t)pContext;
    phnpSnep_Fri_DataParams_t       *pDataParams = (phnpSnep_Fri_DataParams_t *) pClientSessionContext->pSnepDataParamsContext;

    if (PH_ERR_SUCCESS ==  status)
    {
        pSnepPktTraverse = pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer;

        if (pClientSessionContext->putGetDataContext.pChunkingBuffer->length)
        {
            memcpy (pClientSessionContext->putGetDataContext.pReqResponse->buffer + pClientSessionContext->putGetDataContext.iDataReceived,
                    pSnepPktTraverse,
                    pClientSessionContext->putGetDataContext.pChunkingBuffer->length);  /* PRQA S 3200 */

            pClientSessionContext->putGetDataContext.iDataReceived += pClientSessionContext->putGetDataContext.pChunkingBuffer->length;
        }
        if (pClientSessionContext->putGetDataContext.iDataReceived < pClientSessionContext->putGetDataContext.pReqResponse->length)
        {
            pClientSessionContext->putGetDataContext.pChunkingBuffer->length = pClientSessionContext->iMiu;

            ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                           (phlnLlcp_Fri_Transport_Socket_t *) pClientSessionContext->hSnepClientHandle,
                                           pClientSessionContext->putGetDataContext.pChunkingBuffer,
                                           &phnpSnep_Fri_Utils_LlcpSocketReceiveCb,
                                           pClientSessionContext
                                         );

            if(!((PH_ERR_SUCCESS == ret) || (PH_ERR_PENDING == ret)))
            {
                /* delete response message buffer in all error cases */
                phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                    pClientSessionContext,
                                                    status,
                                                    NULL);
            }
        }
        else
        {
            phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                                pClientSessionContext,
                                                status,
                                                pClientSessionContext->putGetDataContext.pReqResponse);
        }

    }
    else
    {
        phnpSnep_Fri_Utils_ClearMemNCallCb( pDataParams,
                                            pClientSessionContext,
                                            status,
                                            NULL);
    }
}

/* This function is called in the cases of failure or success to clean memory and call the callback to inform failure */
static void phnpSnep_Fri_Utils_ClearMemNCallCb( phnpSnep_Fri_DataParams_t       *pDataParams,
                                                pphnpSnep_Fri_ClientSession_t    pClientSessionContext,
                                                phStatus_t                       status,
                                                phNfc_sData_t                   *pReqResponse)
{
    if (pClientSessionContext->putGetDataContext.pSnepPacket)
    {
        if (NULL != pClientSessionContext->putGetDataContext.pSnepPacket->buffer)
        {
            pClientSessionContext->putGetDataContext.pSnepPacket->length = 0;
        } /* No Else */
    }

    pClientSessionContext->pReqCb( pClientSessionContext->hSnepClientHandle,
                                   pClientSessionContext->pReqCbContext,
                                   status,
                                   pReqResponse );
}


void phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pphnpSnep_Fri_ServerConnection_t    pSnepServerConnection,
                                                 phStatus_t                          status)
{
    phStatus_t   ret = 0;
    phnpSnep_Fri_DataParams_t   *pDataParams = (phnpSnep_Fri_DataParams_t *) pSnepServerConnection->pSnepDataParamsContext;

    PH_UNUSED_VARIABLE(ret);

    pSnepServerConnection->responseDataContext.fSendCompleteCb (pSnepServerConnection->responseDataContext.cbContext,
                                                                status,
                                                                pSnepServerConnection->hSnepServerConnHandle);

    /* reset context and restart receive */
    pSnepServerConnection->responseDataContext.pChunkingBuffer->length = pSnepServerConnection->iMiu;

    ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                   (phlnLlcp_Fri_Transport_Socket_t *)pSnepServerConnection->hSnepServerConnHandle,
                                   pSnepServerConnection->responseDataContext.pChunkingBuffer,
                                   &phnpSnep_Fri_Utils_SocketRecvCbForServer,
                                   pSnepServerConnection);

}

/* Server receive callback. reads the complete message and informs the upper layer */
void phnpSnep_Fri_Utils_SocketRecvCbForServer( void           *pContext,
                                               phStatus_t      status)
{
    uint8_t   *pSnepPktTraverse = NULL;
    uint8_t   *pdata_len    = NULL;
    uint8_t   *pConvert     = NULL;
    uint8_t    req_type     = 0;
    uint32_t   iInfoLength  = 0;

    phStatus_t  ret = PH_ERR_SUCCESS;
    pphnpSnep_Fri_ServerConnection_t   pServerConnectionContext = (pphnpSnep_Fri_ServerConnection_t)pContext;
    pphnpSnep_Fri_ServerSession_t      pServerSessionContext = NULL;
    phnpSnep_Fri_DataParams_t         *pDataParams = (phnpSnep_Fri_DataParams_t *) pServerConnectionContext->pSnepDataParamsContext;

    PH_UNUSED_VARIABLE(ret);

    if (PH_ERR_SUCCESS ==  status)
    {
        pServerSessionContext = phnpSnep_Fri_ContextMgmt_GetServerSessionByConnection( pDataParams,
                                                                                        pServerConnectionContext->hSnepServerConnHandle);

        pSnepPktTraverse = pServerConnectionContext->responseDataContext.pChunkingBuffer->buffer;

        if (!pServerConnectionContext->iDataTobeReceived)
        {
            pdata_len = (pServerConnectionContext->responseDataContext.pChunkingBuffer->buffer + PHNPSNEP_FRI_REQUEST_LENGTH + PHNPSNEP_FRI_VERSION_LENGTH);
            pConvert = (uint8_t*)&iInfoLength;
            pConvert[3] = *pdata_len;
            pdata_len++;
            pConvert[2] = *pdata_len;
            pdata_len++;
            pConvert[1] = *pdata_len;
            pdata_len++;
            pConvert[0] = *pdata_len;
            pServerConnectionContext->iDataTobeReceived = iInfoLength;

            req_type = pServerConnectionContext->responseDataContext.pChunkingBuffer->buffer[PHNPSNEP_FRI_VERSION_LENGTH];

            if ((PHNPSNEP_FRI_REQUEST_PUT == req_type || PHNPSNEP_FRI_REQUEST_GET == req_type) && 0 != iInfoLength)
            {
                if( phnpSnep_Fri_Utils_VersionsCompatibleChk( pServerConnectionContext->SnepServerVersion,
                                                             *pSnepPktTraverse) )
                {
                    if (pServerConnectionContext->SnepServerVersion > (*pSnepPktTraverse))
                    {
                        pServerConnectionContext->SnepServerVersion = *pSnepPktTraverse;
                    }

                    if (iInfoLength > pServerConnectionContext->iInboxSize)
                    {
                        /* send Reject, in completion reset context and restart read */
                        status = phnpSnep_Fri_ServerSendResponse ( pDataParams,
                                                          pServerConnectionContext->hSnepServerConnHandle,
                                                          NULL,
                                                          PHNPSNEP_FRI_STATUS_RESPONSE_REJECT,
                                                          &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                                          pServerConnectionContext
                                                        );
#ifdef DEBUG
                        printf("Error: The data buffer size is less than the Request message length, SNEP Reject frame has been Sent\n");
#endif /* DEBUG */
                    }
                    else if ( NULL != pServerSessionContext
                              &&  PHNPSNEP_FRI_REQUEST_GET == pServerConnectionContext->responseDataContext.pChunkingBuffer->buffer[PHNPSNEP_FRI_VERSION_LENGTH]
                              &&  phnpSnep_Fri_Server_Default == pServerSessionContext->SnepServerType )
                    {
                        /* Default server should Reject Incoming GET Request */
                        status = phnpSnep_Fri_ServerSendResponse ( pDataParams,
                                                          pServerConnectionContext->hSnepServerConnHandle,
                                                          NULL,
                                                          PHNPSNEP_FRI_STATUS_RESPONSE_NOT_IMPLEMENTED,
                                                          &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                                          pServerConnectionContext
                                                        );
                    }
                    else
                    {
                        if (PHNPSNEP_FRI_REQUEST_PUT == req_type)
                        {
                            pServerConnectionContext->ServerConnectionState = phnpSnep_Fri_Server_Received_Put;
                            pServerConnectionContext->pSnepWorkingBuffer = pServerConnectionContext->pDataInbox;

                            /* Clear the receive buffer memory */
                            memset(pServerConnectionContext->pSnepWorkingBuffer->buffer, 0, pServerConnectionContext->pSnepWorkingBuffer->length);
                        }
                        else
                        {
                            pdata_len = (pServerConnectionContext->responseDataContext.pChunkingBuffer->buffer + PHNPSNEP_FRI_HEADER_SIZE);
                            pConvert = (uint8_t*)&(pServerConnectionContext->responseDataContext.iAcceptableLength);
                            pConvert[3] = *pdata_len;
                            pdata_len++;
                            pConvert[2] = *pdata_len;
                            pdata_len++;
                            pConvert[1] = *pdata_len;
                            pdata_len++;
                            pConvert[0] = *pdata_len;

                            pServerConnectionContext->ServerConnectionState = phnpSnep_Fri_Server_Received_Get;

                            pServerConnectionContext->pSnepWorkingBuffer = pServerConnectionContext->pDataInbox;

                            /* Clear the receive buffer memory */
                            memset(pServerConnectionContext->pSnepWorkingBuffer->buffer, 0, pServerConnectionContext->pSnepWorkingBuffer->length);

                        }

                        memcpy( pServerConnectionContext->pSnepWorkingBuffer->buffer,
                                pSnepPktTraverse + PHNPSNEP_FRI_HEADER_SIZE,
                               (pServerConnectionContext->responseDataContext.pChunkingBuffer->length - PHNPSNEP_FRI_HEADER_SIZE));  /* PRQA S 3200 */

                        pServerConnectionContext->pSnepWorkingBuffer->length = (pServerConnectionContext->responseDataContext.pChunkingBuffer->length- PHNPSNEP_FRI_HEADER_SIZE);

                        if (pServerConnectionContext->iDataTobeReceived > pServerConnectionContext->pSnepWorkingBuffer->length)
                        {
                            /* send continue, in completion restart receive*/
                            status = phnpSnep_Fri_ServerSendResponse ( pDataParams,
                                                              pServerConnectionContext->hSnepServerConnHandle,
                                                              NULL,
                                                              PHNPSNEP_FRI_STATUS_RESPONSE_CONTINUE,
                                                              &phnpSnep_Fri_Utils_ServerSendContinuecomplete,
                                                              pServerConnectionContext);
                        }
                        else
                        {
                            /* call PUT/GET Request Notify */
                            phnpSnep_Fri_Utils_NotifyUpperLayer( pDataParams,
                                                                 pServerConnectionContext);
                        }
                    }
                }
                else
                {
                    /* Reject request with Unsupported version, in completion reset context and restart read */
                    status = phnpSnep_Fri_ServerSendResponse ( pDataParams,
                                                      pServerConnectionContext->hSnepServerConnHandle,
                                                      NULL,
                                                      PHNPSNEP_FRI_STATUS_RESPONSE_UNSUPPORTED_VERSION,
                                                      &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                                      pServerConnectionContext
                                                    );
                }
            }
            else
            {
                /* reset context and restart receive */
                pServerConnectionContext->iDataTobeReceived = 0;
                pServerConnectionContext->responseDataContext.pChunkingBuffer->length = pServerConnectionContext->iRemoteMiu;

                /* send 'Bad Request' response, as it is a malformed request */
                status =  phnpSnep_Fri_ServerSendResponse ( pDataParams,
                                                  pServerConnectionContext->hSnepServerConnHandle,
                                                  NULL,
                                                  PHNPSNEP_FRI_STATUS_RESPONSE_BAD_REQUEST,
                                                  &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                                  pServerConnectionContext
                                                 );
            }
        }
        else
        {
            memcpy( (pServerConnectionContext->pSnepWorkingBuffer->buffer+pServerConnectionContext->pSnepWorkingBuffer->length),
                    pSnepPktTraverse,
                    pServerConnectionContext->responseDataContext.pChunkingBuffer->length);  /* PRQA S 3200 */

            pServerConnectionContext->pSnepWorkingBuffer->length += pServerConnectionContext->responseDataContext.pChunkingBuffer->length;

            if (pServerConnectionContext->iDataTobeReceived > pServerConnectionContext->pSnepWorkingBuffer->length)
            {
                pServerConnectionContext->responseDataContext.pChunkingBuffer->length = pServerConnectionContext->iMiu;
                phnpSnep_Fri_Utils_SocketReceiveData( pContext,
                                                      PH_ERR_SUCCESS);
            }
            else
            {
                /* call to PUT or GET request Notify */
                phnpSnep_Fri_Utils_NotifyUpperLayer( pDataParams,
                                                     pServerConnectionContext);
            }
        }
    }
    else
    {
        pServerConnectionContext->iDataTobeReceived = 0;

        pServerConnectionContext->responseDataContext.pChunkingBuffer->length = pServerConnectionContext->iMiu;

        ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                      (phlnLlcp_Fri_Transport_Socket_t *) pServerConnectionContext->hSnepServerConnHandle,
                                       pServerConnectionContext->responseDataContext.pChunkingBuffer,
                                       &phnpSnep_Fri_Utils_SocketRecvCbForServer,
                                       pServerConnectionContext);
    }
}

void phnpSnep_Fri_Utils_NotifyUpperLayer( phnpSnep_Fri_DataParams_t          *pDataParams,
                                         pphnpSnep_Fri_ServerConnection_t    pServerConnectionContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(status);

    if (pServerConnectionContext)
    {
        if (phnpSnep_Fri_Server_Received_Get == pServerConnectionContext->ServerConnectionState)
        {
            pServerConnectionContext->pGetNtfCb( pServerConnectionContext->pConnectionContext,
                                                 PH_ERR_SUCCESS,
                                                 pServerConnectionContext->pSnepWorkingBuffer,
                                                 pServerConnectionContext->hSnepServerConnHandle);

            if (NULL != pServerConnectionContext->pSnepWorkingBuffer->buffer)
            {
                pServerConnectionContext->pSnepWorkingBuffer->length = 0;
            }
        }
        else if (phnpSnep_Fri_Server_Received_Put == pServerConnectionContext->ServerConnectionState)
        {
            pServerConnectionContext->pPutNtfCb( pServerConnectionContext->pConnectionContext,
                                                 PH_ERR_SUCCESS,
                                                 pServerConnectionContext->pSnepWorkingBuffer,
                                                 pServerConnectionContext->hSnepServerConnHandle);
        }
        else
        {
            status =  phnpSnep_Fri_ServerSendResponse( pDataParams,
                                              pServerConnectionContext->hSnepServerConnHandle,
                                              NULL,
                                              PHNPSNEP_FRI_STATUS_RESPONSE_BAD_REQUEST,
                                              &phnpSnep_Fri_Utils_ServerSendcompleteInternal,
                                              pServerConnectionContext
                                            );
        }
    }
}


static void phnpSnep_Fri_Utils_ServerSendContinuecomplete( void           *pContext,
                                                           phStatus_t      status,
                                                           ph_NfcHandle    ConnHandle)
{
    pphnpSnep_Fri_ServerConnection_t   pSnepServerConnection = (pphnpSnep_Fri_ServerConnection_t)pContext;
    phnpSnep_Fri_DataParams_t         *pDataParams = (phnpSnep_Fri_DataParams_t *) pSnepServerConnection->pSnepDataParamsContext;
    PH_UNUSED_VARIABLE(status);
    PH_UNUSED_VARIABLE(ConnHandle);

    pSnepServerConnection->responseDataContext.pChunkingBuffer->length = pSnepServerConnection->iMiu;

    status = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                     (phlnLlcp_Fri_Transport_Socket_t *) pSnepServerConnection->hSnepServerConnHandle,
                                      pSnepServerConnection->responseDataContext.pChunkingBuffer,
                                     &phnpSnep_Fri_Utils_SocketRecvCbForServer,
                                      pSnepServerConnection);

}

static void phnpSnep_Fri_Utils_ServerSendcompleteInternal ( void           *pContext,
                                                            phStatus_t      Status,
                                                            ph_NfcHandle    ConnHandle)
{
    pphnpSnep_Fri_ServerConnection_t pSnepServerConnection = (pphnpSnep_Fri_ServerConnection_t)pContext;
    phnpSnep_Fri_DataParams_t  *pDataParams = (phnpSnep_Fri_DataParams_t *) pSnepServerConnection->pSnepDataParamsContext;

    phStatus_t    ret = PH_ERR_SUCCESS;

    PH_UNUSED_VARIABLE(Status);
    PH_UNUSED_VARIABLE(ConnHandle);
    PH_UNUSED_VARIABLE(ret);

    pSnepServerConnection->iDataTobeReceived = 0;
    pSnepServerConnection->responseDataContext.pChunkingBuffer->length = pSnepServerConnection->iMiu;

    ret = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                   (phlnLlcp_Fri_Transport_Socket_t *) pSnepServerConnection->hSnepServerConnHandle,
                                   pSnepServerConnection->responseDataContext.pChunkingBuffer,
                                   &phnpSnep_Fri_Utils_SocketRecvCbForServer,
                                   pSnepServerConnection);
}

uint8_t phnpSnep_Fri_Utils_VersionsCompatibleChk( uint8_t   ver1,
                                                  uint8_t   ver2)
{
    if ((ver1>>PHNPSNEP_FRI_VERSION_FIELD) == (ver2>>PHNPSNEP_FRI_VERSION_FIELD))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void phnpSnep_Fri_Utils_ResetServerConnectionContext( pphnpSnep_Fri_ServerConnection_t    pServerConnectionContext )
{
    pServerConnectionContext->iDataTobeReceived = 0;                          /* Clear data counter to be received */
    pServerConnectionContext->responseDataContext.iAcceptableLength = 0;      /* Clear Acceptable Length */
    pServerConnectionContext->responseDataContext.bIsExcessData = false;      /* Clear Excess Data Flag*/
    pServerConnectionContext->responseDataContext.iDataSent = 0;              /* Clear data count sent so far */
    pServerConnectionContext->responseDataContext.pSnepPacket->length = 0;    /* Clear SNEP packet length */
    pServerConnectionContext->responseDataContext.bWaitForContinue = false;   /* Clear WaitForContinue flag */
    pServerConnectionContext->responseDataContext.bContinueReceived = false;  /* Clear ContinueReceived flag */
    pServerConnectionContext->responseDataContext.fSendCompleteCb = NULL;     /* Clear Send Complete Callback */

}

void phnpSnep_Fri_Utils_ResetCliDataContext( pphnpSnep_Fri_ClientSession_t     pClientSessionContext )
{
    pClientSessionContext->putGetDataContext.iDataSent = 0;               /* Clear data count sent so far */
    pClientSessionContext->putGetDataContext.iDataReceived = 0;           /* Clear data count received so far */
    pClientSessionContext->putGetDataContext.pSnepPacket->length = 0;     /* prepared snep packet */
    pClientSessionContext->putGetDataContext.bWaitForContinue = false;    /* Do we need to wait for continue before send */
    pClientSessionContext->putGetDataContext.bContinueReceived = false;   /* Have we received continue to send */
    pClientSessionContext->putGetDataContext.pReqResponse->length = 0;    /* response data to be sent back to upper layer */
    pClientSessionContext->putGetDataContext.pChunkingBuffer->length = 0; /* Reset Chunking buffer length */
}

#endif /* NXPBUILD__PHNP_SNEP_FRI */
