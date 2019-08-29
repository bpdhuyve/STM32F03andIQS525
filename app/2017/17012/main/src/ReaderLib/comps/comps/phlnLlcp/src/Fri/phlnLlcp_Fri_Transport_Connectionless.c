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
 * \file  phlnLlcp_Fri_Transport_Connectionless.c
 * \brief 
 *
 * $Author: nxp62726 $
 * $Revision: 423 $
 * $Date: 2013-11-16 12:03:58 +0530 (Sat, 16 Nov 2013) $
 *
 */
#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI

#include "phlnLlcp_Fri_Transport_Connectionless.h"
#include "phlnLlcp_Fri_Transport.h"
#include "phlnLLcp_Fri_Llcp.h"
#include <phOsal.h>

static void phlnLlcp_Fri_Transport_Connectionless_SendTo_CB( void         *pContext,
                                                            phStatus_t   status);

static void phlnLlcp_Fri_Transport_Connectionless_TriggerSendCb( phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket,
                                                                phStatus_t                       status)
{  /* PRQA S 0779 */
   pphlnLlcp_Fri_TransportSocketSendCb_t  pfSendCb;
    void                                  *pSendContext;

   if (pLlcpSocket->pfSocketSend_Cb != NULL)
   {
      /* Copy CB + context in local variables */
      pfSendCb = pLlcpSocket->pfSocketSend_Cb;
      pSendContext = pLlcpSocket->pSendContext;

      /* Reset CB + context */
      pLlcpSocket->pfSocketSend_Cb = NULL;
      pLlcpSocket->pSendContext = NULL;

      /* Perform callback */
      pfSendCb(pSendContext, status);
   }
}

static void phlnLlcp_Fri_Transport_Connectionless_TriggerRecvFromCb( phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket,
                                                                    uint8_t                          ssap,
                                                                    phStatus_t                       status)
{  /* PRQA S 0779 */
   pphlnLlcp_Fri_TransportSocketRecvFromCb_t  pfSocketRecvFrom_Cb;
    void                                      *pRecvContext;

   /* TODO: use phlnLlcp_Fri_Transport_Connectionless_TriggerRecvFromCb? */
   if (pLlcpSocket->pfSocketRecvFrom_Cb != NULL)
   {
      /* Copy CB + context in local variables */
      pfSocketRecvFrom_Cb = pLlcpSocket->pfSocketRecvFrom_Cb;
      pRecvContext = pLlcpSocket->pRecvContext;

      /* Reset CB + context */
      pLlcpSocket->pfSocketRecvFrom_Cb = NULL;
      pLlcpSocket->pfSocketRecv_Cb = NULL;
      pLlcpSocket->pRecvContext = NULL;

      /* Perform callback */
      pfSocketRecvFrom_Cb(pRecvContext, ssap, status);
   }
}

phStatus_t phlnLlcp_Fri_Transport_Connectionless_HandlePendingOperations( phlnLlcp_Fri_Transport_Socket_t    *pSocket )
{
   phStatus_t status = PH_ERR_FAILED;

   /* Check if something is pending and if transport layer is ready to send */
   if ((pSocket->pfSocketSend_Cb != NULL) &&
       (pSocket->psTransport->bSendPending == false))
   {
      /* Fill the psLlcpHeader stuture with the DSAP,PTYPE and the SSAP */
      pSocket->sLlcpHeader.dsap  = pSocket->socket_dSap;
      pSocket->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_UI;
      pSocket->sLlcpHeader.ssap  = pSocket->socket_sSap;

      /* Send to data to the approiate socket */
      status =  phlnLlcp_Fri_Transport_LinkSend(pSocket->psTransport,
                                   &pSocket->sLlcpHeader,
                                   NULL,
                                   &pSocket->sSocketSendBuffer,
                                   &phlnLlcp_Fri_Transport_Connectionless_SendTo_CB,
                                   pSocket);
   }
   else
   {
      /* Cannot send now, retry later */
   }

   return status;
}


/* TODO: comment function phlnLlcp_Fri_Handle_Connectionless_IncommingFrame */
void phlnLlcp_Fri_Handle_Connectionless_IncommingFrame(phlnLlcp_Fri_Transport_t      *pLlcpTransport,
                                                       phNfc_sData_t                 *psData,
                                                       uint8_t                        dsap,
                                                       uint8_t                        ssap)
{
    phlnLlcp_Fri_Transport_Socket_t  *pSocket = NULL;
   uint8_t                           i       = 0;
   uint8_t                           writeIndex;

   /* Look through the socket table for a match */
   for(i=0;i<PHLNLLCP_FRI_NB_SOCKET_MAX;i++)
   {
      if(pLlcpTransport->pSocketTable[i].socket_sSap == dsap)
      {
         /* Socket found ! */
         pSocket = &pLlcpTransport->pSocketTable[i];

         /* Forward directly to application if a read is pending */
         if (pSocket->bSocketRecvPending == true)
         {
            /* Reset the RecvPending variable */
            pSocket->bSocketRecvPending = false;

            /* Copy the received buffer into the receive buffer */
            memcpy(pSocket->sSocketRecvBuffer->buffer, psData->buffer, psData->length);  /* PRQA S 3200 */

            /* Update the received length */
            *pSocket->receivedLength = psData->length;

            /* call the recv callback */
            phlnLlcp_Fri_Transport_Connectionless_TriggerRecvFromCb(pSocket, ssap, PH_ERR_SUCCESS);
         }
         /* If no read is pending, try to bufferize for later reading */
         else
         {
            if((pSocket->indexRwWrite - pSocket->indexRwRead) < pSocket->localRW)
            {
               writeIndex = (uint8_t)(pSocket->indexRwWrite % pSocket->localRW);
               /* Save SSAP */
               pSocket->sSocketRwBufferTable[writeIndex].buffer[0] = ssap;
               /* Save UI frame payload */
               memcpy(pSocket->sSocketRwBufferTable[writeIndex].buffer + 1, psData->buffer, psData->length);  /* PRQA S 3200 */
               pSocket->sSocketRwBufferTable[writeIndex].length = psData->length;
               
               /* Update the RW write index */
               pSocket->indexRwWrite++;
            }
            else
            {
               /* Unable to bufferize the packet, drop it */
            }
         }
         break;
      }
   }
}

/* TODO: comment function phlnLlcp_Fri_Transport_Connectionless_SendTo_CB */
static void phlnLlcp_Fri_Transport_Connectionless_SendTo_CB(void*        pContext,
                                                            phStatus_t    status)
{
   phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket = (phlnLlcp_Fri_Transport_Socket_t*)pContext;

   /* Reset the SendPending variable */
   pLlcpSocket->bSocketSendPending = false;

   /* Call the send callback */
   phlnLlcp_Fri_Transport_Connectionless_TriggerSendCb(pLlcpSocket,status);
}

static void phlnLlcp_Fri_Transport_Connectionless_Abort(phlnLlcp_Fri_Transport_Socket_t* pLlcpSocket)
{  /* PRQA S 0779 */
   phlnLlcp_Fri_Transport_Connectionless_TriggerSendCb(pLlcpSocket, PHLNLLCP_FRI_NFCSTATUS_ABORTED);
   phlnLlcp_Fri_Transport_Connectionless_TriggerRecvFromCb(pLlcpSocket, 0, PHLNLLCP_FRI_NFCSTATUS_ABORTED);
   pLlcpSocket->pAcceptContext = NULL;
   pLlcpSocket->pfSocketAccept_Cb = NULL;
   pLlcpSocket->pListenContext = NULL;
   pLlcpSocket->pfSocketListen_Cb = NULL;
   pLlcpSocket->pConnectContext = NULL;
   pLlcpSocket->pfSocketConnect_Cb = NULL;
   pLlcpSocket->pDisonnectContext = NULL;
   pLlcpSocket->pfSocketDisconnect_Cb = NULL;
}

/**
*
* \brief <b>Close a socket on a LLCP-connectionless device</b>.
*
* This function closes a LLCP socket previously created using phlnLlcp_Fri_Transport_Socket.
*
* \param[in]  pLlcpSocket                    A pointer to a phlnLlcp_Fri_Transport_Socket_t.

* \retval PH_ERR_SUCCESS                  Operation successful.
*/
phStatus_t phlnLlcp_Fri_Transport_Connectionless_Close(phlnLlcp_Fri_Transport_Socket_t*   pLlcpSocket)
{
   phStatus_t status = PH_ERR_SUCCESS;
   /* Reset the pointer to the socket closed */
   pLlcpSocket->eSocket_State                      = phlnLlcp_Fri_TransportSocket_eSocketDefault;
   pLlcpSocket->eSocket_Type                       = phlnLlcp_Fri_Transport_eDefaultType;
   pLlcpSocket->pContext                           = NULL;
   pLlcpSocket->pSocketErrCb                       = NULL;
   pLlcpSocket->socket_sSap                        = PHLNLLCP_FRI_SAP_DEFAULT;
   pLlcpSocket->socket_dSap                        = PHLNLLCP_FRI_SAP_DEFAULT;
   pLlcpSocket->bSocketRecvPending                 = false;
   pLlcpSocket->bSocketSendPending                 = false;
   pLlcpSocket->bSocketListenPending               = false;
   pLlcpSocket->bSocketDiscPending                 = false;
   pLlcpSocket->RemoteBusyConditionInfo            = false;
   pLlcpSocket->ReceiverBusyCondition              = false;
   pLlcpSocket->socket_VS                          = 0;
   pLlcpSocket->socket_VSA                         = 0;
   pLlcpSocket->socket_VR                          = 0;
   pLlcpSocket->socket_VRA                         = 0;


   PH_UNUSED_VARIABLE(status);

   phlnLlcp_Fri_Transport_Connectionless_Abort(pLlcpSocket);

   memset(&pLlcpSocket->sSocketOption, 0x00, sizeof(phlnLlcp_Fri_Transport_sSocketOptions_t));  /* PRQA S 3200 */

   pLlcpSocket->sServiceName.length = 0;

   return PH_ERR_SUCCESS;
}

/**
*
* \brief <b>Send data on a socket to a given destination SAP</b>.
*
* This function is used to write data on a socket to a given destination SAP.
* This function can only be called on a connectionless socket.
* 
*
* \param[in]  pLlcpSocket        A pointer to a LlcpSocket created.
* \param[in]  nSap               The destination SAP.
* \param[in]  psBuffer           The buffer containing the data to send.
* \param[in]  pSend_RspCb        The callback to be called when the 
*                                operation is completed.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
*/
phStatus_t phlnLlcp_Fri_Transport_Connectionless_SendTo(phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                                       uint8_t                                     nSap,
                                                       phNfc_sData_t                              *psBuffer,
                                                       pphlnLlcp_Fri_TransportSocketSendCb_t       pSend_RspCb,
                                                       void*                                       pContext)
{
   phStatus_t status = PH_ERR_FAILED;

   /* Store send callback  and context*/
   pLlcpSocket->pfSocketSend_Cb = pSend_RspCb;
   pLlcpSocket->pSendContext    = pContext;

   /* Test if a send is already pending at transport level */
   if(pLlcpSocket->psTransport->bSendPending == true)
   {
      /* Save the request so it can be handled in phlnLlcp_Fri_Transport_Connectionless_HandlePendingOperations() */
      pLlcpSocket->sSocketSendBuffer = *psBuffer;
      pLlcpSocket->socket_dSap      = nSap;
      status = PH_ERR_PENDING;
   }
   else
   {
      /* Fill the psLlcpHeader stuture with the DSAP,PTYPE and the SSAP */
      pLlcpSocket->sLlcpHeader.dsap  = nSap;
      pLlcpSocket->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_UI;
      pLlcpSocket->sLlcpHeader.ssap  = pLlcpSocket->socket_sSap;

      /* Send to data to the approiate socket */
      status =  phlnLlcp_Fri_Transport_LinkSend(pLlcpSocket->psTransport,
                                   &pLlcpSocket->sLlcpHeader,
                                   NULL,
                                   psBuffer,
                                   &phlnLlcp_Fri_Transport_Connectionless_SendTo_CB,
                                   pLlcpSocket);
   }

   return status;
}


/**
*
* \brief <b>Read data on a socket and get the source SAP</b>.
*
* This function is the same as phLibNfc_Llcp_Recv, except that the callback includes
* the source SAP. This functions can only be called on a connectionless socket.
* 
*
* \param[in]  pLlcpSocket        A pointer to a LlcpSocket created.
* \param[in]  psBuffer           The buffer receiving the data.
* \param[in]  pRecv_RspCb        The callback to be called when the 
*                                operation is completed.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
*/
phStatus_t phlnLlcp_Fri_Transport_Connectionless_RecvFrom(phlnLlcp_Fri_Transport_Socket_t               *pLlcpSocket,
                                                         phNfc_sData_t                                  *psBuffer,
                                                         pphlnLlcp_Fri_TransportSocketRecvFromCb_t       pRecv_Cb,
                                                         void                                           *pContext)
{
   phStatus_t   status = PH_ERR_PENDING;
   uint8_t     readIndex;
   uint8_t     ssap;

   if(pLlcpSocket->bSocketRecvPending)
   {
      status = PH_ADD_COMPCODE(PHLNLLCP_FRI_NFCSTATUS_REJECTED, CID_FRI_NFC_LLCP_TRANSPORT);
   }
   else
   {
      /* Check if pending packets in RW */
      if(pLlcpSocket->indexRwRead != pLlcpSocket->indexRwWrite)
      {
         readIndex = (uint8_t)(pLlcpSocket->indexRwRead % pLlcpSocket->localRW);

         /* Extract ssap and buffer from RW buffer */
         ssap = pLlcpSocket->sSocketRwBufferTable[readIndex].buffer[0];
         memcpy(psBuffer->buffer, pLlcpSocket->sSocketRwBufferTable[readIndex].buffer + 1, pLlcpSocket->sSocketRwBufferTable[readIndex].length);  /* PRQA S 3200 */
         psBuffer->length = pLlcpSocket->sSocketRwBufferTable[readIndex].length;

         /* Reset RW buffer length */
         pLlcpSocket->sSocketRwBufferTable[readIndex].length = 0;

         /* Update Value Rw Read Index */
         pLlcpSocket->indexRwRead++;

         /* call the recv callback */
         pRecv_Cb(pContext, ssap, PH_ERR_SUCCESS);

         status = PH_ERR_SUCCESS;
      }
      /* Otherwise, wait for a packet to come */
      else
      {
         /* Store the callback and context*/
         pLlcpSocket->pfSocketRecvFrom_Cb  = pRecv_Cb;
         pLlcpSocket->pRecvContext         = pContext;

         /* Store the pointer to the receive buffer */
         pLlcpSocket->sSocketRecvBuffer   =  psBuffer;
         pLlcpSocket->receivedLength      =  &psBuffer->length;

         /* Set RecvPending to true */
         pLlcpSocket->bSocketRecvPending = true;
      }
   }
   return status;
}
#endif /* NXPBUILD__PHLN_LLCP_FRI */
