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
* \file  phlnLlcp_Fri_Transport_Connection.c
* \brief
*
* $Author: nxp40786 $
* $Revision: 452 $
* $Date: 2013-12-19 20:09:00 +0530 (Thu, 19 Dec 2013) $
*
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI

#include "phlnLlcp_Fri_Transport.h"
#include "phlnLlcp_Fri_Transport_Connection.h"
#include "phlnLlcp_Fri_Llcp.h"
#include "phlnLlcp_Fri_Utils.h"
#include <phOsal.h>


/* Function definition */
static phStatus_t phlnLlcp_Fri_Send_ReceiveReady_Frame(phlnLlcp_Fri_Transport_Socket_t      *pLlcpSocket);

static phStatus_t phlnLlcp_Fri_Send_ReceiveNotReady_Frame(phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket);

static phStatus_t phlnLlcp_Fri_performSendInfo(phlnLlcp_Fri_Transport_Socket_t    *psLlcpSocket);

/**********   End Function definition   ***********/

static void phlnLlcp_Fri_Transport_ConnectionOriented_TriggerSendCb(phlnLlcp_Fri_Transport_Socket_t  *pLlcpSocket,
                                                                    phStatus_t                        status)
{
    pphlnLlcp_Fri_TransportSocketRecvCb_t  pfSendCb;
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

static void phlnLlcp_Fri_Transport_ConnectionOriented_TriggerRecvCb(phlnLlcp_Fri_Transport_Socket_t  *pLlcpSocket,
                                                                    phStatus_t                        status)
{  /* PRQA S 0779 */
    pphlnLlcp_Fri_TransportSocketRecvCb_t  pfRecvCb;
    void*                                  pRecvContext;

    if (pLlcpSocket->pfSocketRecv_Cb != NULL)
    {
        /* Copy CB + context in local variables */
        pfRecvCb = pLlcpSocket->pfSocketRecv_Cb;
        pRecvContext = pLlcpSocket->pRecvContext;

        /* Reset CB + context */
        pLlcpSocket->pfSocketRecv_Cb = NULL;
        pLlcpSocket->pRecvContext = NULL;

        /* Perform callback */
        pfRecvCb(pRecvContext, status);
    }
}

/* TODO: comment function phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB */
static void phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB(void         *pContext,
                                                                  phStatus_t    status)
{  /* PRQA S 0779 */
    phlnLlcp_Fri_Transport_t          *psTransport;
    phlnLlcp_Fri_Transport_Socket_t    psTempLlcpSocket;
    phStatus_t                         result = 0;

    PH_UNUSED_VARIABLE(result);

    /* Get Send CB context */
    psTransport = (phlnLlcp_Fri_Transport_t*)pContext;

    if(status == PH_ERR_SUCCESS)
    {
        /* Test the socket */
        switch(psTransport->pSocketTable[psTransport->socketIndex].eSocket_State)
        {
            case phlnLlcp_Fri_TransportSocket_eSocketAccepted:
            {
                /* Set socket state to Connected */
                psTransport->pSocketTable[psTransport->socketIndex].eSocket_State  = phlnLlcp_Fri_TransportSocket_eSocketConnected;
                /* Call the Accept Callback */
                psTransport->pSocketTable[psTransport->socketIndex].pfSocketAccept_Cb(psTransport->pSocketTable[psTransport->socketIndex].pAcceptContext,status);
                psTransport->pSocketTable[psTransport->socketIndex].pfSocketAccept_Cb = NULL;
                psTransport->pSocketTable[psTransport->socketIndex].pAcceptContext = NULL;
            }break;

            case phlnLlcp_Fri_TransportSocket_eSocketRejected:
            {
                /* Store the Llcp socket in a local Llcp socket */
                psTempLlcpSocket = psTransport->pSocketTable[psTransport->socketIndex];

                /* Reset the socket  and set the socket state to default */
                result = phlnLlcp_Fri_Transport_Close(&psTransport->pSocketTable[psTransport->socketIndex]);

                /* Call the Reject Callback */
                psTempLlcpSocket.pfSocketSend_Cb(psTempLlcpSocket.pRejectContext,status);
                psTempLlcpSocket.pfSocketSend_Cb = NULL;
            }break;

            case phlnLlcp_Fri_TransportSocket_eSocketConnected:
            {
                if(!psTransport->pSocketTable[psTransport->socketIndex].bSocketSendPending)
                {
                    phlnLlcp_Fri_Transport_ConnectionOriented_TriggerSendCb(&psTransport->pSocketTable[psTransport->socketIndex], status);
                }
            }break;
            default:
            /* Nothing to do */
            break;
        }
    }
    else
    {
        /* Send CB error */
        if(!psTransport->pSocketTable[psTransport->socketIndex].bSocketSendPending)
        {
            phlnLlcp_Fri_Transport_ConnectionOriented_TriggerSendCb(&psTransport->pSocketTable[psTransport->socketIndex], status);
        }
    }
}


phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_HandlePendingOperations(phlnLlcp_Fri_Transport_Socket_t   *pSocket)
{
    phStatus_t                  result = PH_ERR_FAILED;
    phlnLlcp_Fri_Transport_t   *psTransport = pSocket->psTransport;

    /* I FRAME */
    if(pSocket->bSocketSendPending == true)
    {
        /* Test the RW window */
        if(CHECK_SEND_RW(pSocket))
        {
            result = phlnLlcp_Fri_performSendInfo(pSocket);
        }
    }
    /* RR FRAME */
    else if(pSocket->bSocketRRPending == true)
    {
        /* Reset RR pending */
        pSocket->bSocketRRPending = false;

        /* Send RR Frame */
        result = phlnLlcp_Fri_Send_ReceiveReady_Frame(pSocket);
    }

    /* RNR Frame */
    else if(pSocket->bSocketRNRPending == true)
    {
        /* Reset RNR pending */
        pSocket->bSocketRNRPending = false;

        /* Send RNR Frame */
        result = phlnLlcp_Fri_Send_ReceiveNotReady_Frame(pSocket);
    }
    /* CC Frame */
    else if(pSocket->bSocketAcceptPending == true)
    {
        /* Reset Accept pending */
        pSocket->bSocketAcceptPending = false;

        /* Fill the psLlcpHeader stuture with the DSAP,CC PTYPE and the SSAP */
        pSocket->sLlcpHeader.dsap  = pSocket->socket_dSap;
        pSocket->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_CC;
        pSocket->sLlcpHeader.ssap  = pSocket->socket_sSap;

        /* Send Pending */
        pSocket->psTransport->bSendPending = true;

        /* Set the socket state to accepted */
        pSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketAccepted;

        /* Send a CC Frame */
        result =  phlnLlcp_Fri_Transport_LinkSend(psTransport,
                                   &pSocket->sLlcpHeader,
                                   NULL,
                                   &pSocket->sSocketSendBuffer,
                                   &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                   psTransport);
    }
    /* CONNECT FRAME */
    else if(pSocket->bSocketConnectPending == true)
    {
        /* Reset Accept pending */
        pSocket->bSocketConnectPending = false;

        /* Send Pending */
        pSocket->psTransport->bSendPending = true;

        /* Set the socket in connecting state */
        pSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketConnecting;

        /* send CONNECT */
        result =  phlnLlcp_Fri_Transport_LinkSend(psTransport,
                                 &pSocket->sLlcpHeader,
                                 NULL,
                                 &pSocket->sSocketSendBuffer,
                                 &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                 psTransport);
    }
    /* DISC FRAME */
    else if(pSocket->bSocketDiscPending == true)
    {
        /* Reset Disc Pending */
        pSocket->bSocketDiscPending = false;

        /* Send Pending */
        pSocket->psTransport->bSendPending = true;

        /* Set the socket in connecting state */
        pSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketDisconnecting;

        /* Send DISC */
        result =  phlnLlcp_Fri_Transport_LinkSend(psTransport,
                                   &pSocket->sLlcpHeader,
                                   NULL,
                                   &pSocket->sSocketSendBuffer,
                                   &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                   psTransport);

        /* Call ErrCB due to a DISC */
        pSocket->pSocketErrCb(pSocket->pContext, PHLNLLCP_FRI_ERR_DISCONNECTED);
    }
    else
    {
        /* Nothing to Do */
    }

    return result;
}

static phStatus_t phlnLlcp_Fri_performSendInfo(phlnLlcp_Fri_Transport_Socket_t    *psLlcpSocket)
{
    phlnLlcp_Fri_Transport_t   *psTransport = psLlcpSocket->psTransport;
    phStatus_t                  status;

    /* Reset Send Pending */
    psLlcpSocket->bSocketSendPending = false;

    /* Send Pending */
    psTransport->bSendPending = true;

    /* Set the Header */
    psLlcpSocket->sLlcpHeader.dsap   = psLlcpSocket->socket_dSap;
    psLlcpSocket->sLlcpHeader.ptype  = PHLNLLCP_FRI_PTYPE_I;
    psLlcpSocket->sLlcpHeader.ssap   = psLlcpSocket->socket_sSap;

    /* Set Sequence Numbers */
    psLlcpSocket->sSequence.ns = psLlcpSocket->socket_VS;
    psLlcpSocket->sSequence.nr = psLlcpSocket->socket_VR;

    /* Update the VRA */
    psLlcpSocket->socket_VRA = psLlcpSocket->socket_VR;

    /* Store the index of the socket */
    psTransport->socketIndex = psLlcpSocket->index;

    /* Update VS */
    psLlcpSocket->socket_VS = (psLlcpSocket->socket_VS+1)%16;

    /* Send I_PDU */
    status =  phlnLlcp_Fri_Transport_LinkSend(psTransport,
                                             &psLlcpSocket->sLlcpHeader,
                                             &psLlcpSocket->sSequence,
                                             &psLlcpSocket->sSocketSendBuffer,
                                             &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                             psTransport);

    return status;
}

static void phlnLlcp_Fri_Transport_ConnectionOriented_Abort(phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket)
{  /* PRQA S 0779 */
    phlnLlcp_Fri_Transport_ConnectionOriented_TriggerSendCb(pLlcpSocket, PHLNLLCP_FRI_NFCSTATUS_ABORTED);
    phlnLlcp_Fri_Transport_ConnectionOriented_TriggerRecvCb(pLlcpSocket, PHLNLLCP_FRI_NFCSTATUS_ABORTED);
    if (pLlcpSocket->pfSocketAccept_Cb != NULL)
    {
        pLlcpSocket->pfSocketAccept_Cb(pLlcpSocket->pAcceptContext, PHLNLLCP_FRI_NFCSTATUS_ABORTED);
        pLlcpSocket->pfSocketAccept_Cb = NULL;
    }
    pLlcpSocket->pAcceptContext = NULL;
    if (pLlcpSocket->pfSocketConnect_Cb != NULL)
    {
        pLlcpSocket->pfSocketConnect_Cb(pLlcpSocket->pConnectContext, 0, PHLNLLCP_FRI_NFCSTATUS_ABORTED);
        pLlcpSocket->pfSocketConnect_Cb = NULL;
    }
    pLlcpSocket->pConnectContext = NULL;
    if (pLlcpSocket->pfSocketDisconnect_Cb != NULL)
    {
        pLlcpSocket->pfSocketDisconnect_Cb(pLlcpSocket->pDisonnectContext, PHLNLLCP_FRI_NFCSTATUS_ABORTED);
        pLlcpSocket->pfSocketDisconnect_Cb = NULL;
    }
    pLlcpSocket->pDisonnectContext = NULL;

    pLlcpSocket->pfSocketRecvFrom_Cb = NULL;
    pLlcpSocket->pfSocketListen_Cb = NULL;
    pLlcpSocket->pListenContext = NULL;
}


static phStatus_t phlnLlcp_Fri_Send_ReceiveReady_Frame(phlnLlcp_Fri_Transport_Socket_t     *pLlcpSocket)  /* PRQA S 3208 */
{
    phStatus_t   status = PH_ERR_SUCCESS;

    /* Test if a send is pending */
    if(pLlcpSocket->psTransport->bSendPending == true)
    {
        pLlcpSocket->bSocketRRPending = true;
        status = PH_ERR_PENDING;
    }
    else
    {
        /* Set the header of the RR frame */
        pLlcpSocket->sLlcpHeader.dsap   = pLlcpSocket->socket_dSap;
        pLlcpSocket->sLlcpHeader.ptype  = PHLNLLCP_FRI_PTYPE_RR;
        pLlcpSocket->sLlcpHeader.ssap   = pLlcpSocket->socket_sSap;

        /* Set sequence number for RR Frame */
        pLlcpSocket->sSequence.ns = 0;
        pLlcpSocket->sSequence.nr = pLlcpSocket->socket_VR;

        /* Update VRA */
        pLlcpSocket->socket_VRA = (uint8_t)pLlcpSocket->sSequence.nr;

        /* Send Pending */
        pLlcpSocket->psTransport->bSendPending = true;

        /* Store the index of the socket */
        pLlcpSocket->psTransport->socketIndex = pLlcpSocket->index;

        /* Send RR frame */
        status =  phlnLlcp_Fri_Transport_LinkSend(pLlcpSocket->psTransport,
                                   &pLlcpSocket->sLlcpHeader,
                                   &pLlcpSocket->sSequence,
                                   NULL,
                                   &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                   pLlcpSocket->psTransport);
    }

    return status;
}

static phStatus_t phlnLlcp_Fri_Send_ReceiveNotReady_Frame(phlnLlcp_Fri_Transport_Socket_t     *pLlcpSocket)
{
    phStatus_t   status = PH_ERR_SUCCESS;


    /* Test if a send is pending */
    if(pLlcpSocket->psTransport->bSendPending == true)
    {
        pLlcpSocket->bSocketRNRPending = true;
        status = PH_ERR_PENDING;
    }
    else
    {
        /* Set the header of the RNR frame */
        pLlcpSocket->sLlcpHeader.dsap   = pLlcpSocket->socket_dSap;
        pLlcpSocket->sLlcpHeader.ptype  = PHLNLLCP_FRI_PTYPE_RNR;
        pLlcpSocket->sLlcpHeader.ssap   = pLlcpSocket->socket_sSap;

        /* Set sequence number for RNR Frame */
        pLlcpSocket->sSequence.ns = 0x00;
        pLlcpSocket->sSequence.nr = pLlcpSocket->socket_VR;

        /* Update VRA */
        pLlcpSocket->socket_VRA = (uint8_t)pLlcpSocket->sSequence.nr;

        /* Send Pending */
        pLlcpSocket->psTransport->bSendPending = true;

        /* Store the index of the socket */
        pLlcpSocket->psTransport->socketIndex = pLlcpSocket->index;

        /* Send RNR frame */
        status =  phlnLlcp_Fri_Transport_LinkSend(pLlcpSocket->psTransport,
                                   &pLlcpSocket->sLlcpHeader,
                                   &pLlcpSocket->sSequence,
                                   NULL,
                                   &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                   pLlcpSocket->psTransport);
    }
    return status;
}


static phStatus_t phlnLlcp_Fri_GetSocket_Params(phNfc_sData_t       *psParamsTLV,
                                                phNfc_sData_t       *psServiceName,
                                                uint8_t             *pRemoteRW_Size,
                                                uint16_t            *pRemoteMIU)
{
    phStatus_t           status = PH_ERR_SUCCESS;
    phNfc_sData_t        sValueBuffer;
    uint32_t             offset = 0;
    uint8_t              type;

    /* Check for NULL pointers */
    if ((psParamsTLV == NULL) || (pRemoteRW_Size == NULL) || (pRemoteMIU == NULL))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
    }
    else
    {
        /* Decode TLV */
        while (offset < psParamsTLV->length)
        {
            status = phlnLlcp_Fri_Utils_DecodeTLV(psParamsTLV, &offset, &type,&sValueBuffer);
            if (status != PH_ERR_SUCCESS)
            {
                /* Error: Ill-formed TLV */
                return status;
            }
            switch(type)
            {
                case PHLNLLCP_FRI_TLV_TYPE_SN:
                {
                    /* Test if a SN is present in the TLV */
                    if(sValueBuffer.length == 0)
                    {
                        /* Error : Ill-formed SN parameter TLV */
                        break;
                    }
                    /* Get the Service Name */
                    *psServiceName = sValueBuffer;
                }break;
                case PHLNLLCP_FRI_TLV_TYPE_RW:
                {
                    /* Check length */
                    if (sValueBuffer.length != PHLNLLCP_FRI_TLV_LENGTH_RW)
                    {
                        /* Error : Ill-formed MIUX parameter TLV */
                        break;
                    }
                    *pRemoteRW_Size = sValueBuffer.buffer[0];
                }break;
                case PHLNLLCP_FRI_TLV_TYPE_MIUX:
                {
                    /* Check length */
                    if (sValueBuffer.length != PHLNLLCP_FRI_TLV_LENGTH_MIUX)
                    {
                        /* Error : Ill-formed MIUX parameter TLV */
                        break;
                    }
                    *pRemoteMIU = PHLNLLCP_FRI_MIU_DEFAULT + ((((uint16_t)sValueBuffer.buffer[0] << 8) | sValueBuffer.buffer[1]) & PHLNLLCP_FRI_TLV_MIUX_MASK);
                }break;
                default:
                {
                    /* Error : Unknown type */
                    break;
                }
            }
        }
    }
    return status;
}


/* TODO: comment function Handle_ConnectFrame */
static void phlnLlcp_Fri_Handle_ConnectionFrame(phlnLlcp_Fri_Transport_t      *psTransport,
                                                phNfc_sData_t                 *psData,
                                                uint8_t                        dsap,
                                                uint8_t                        ssap)
{
    phStatus_t                                status = PH_ERR_SUCCESS;

    uint8_t                                   index;
    uint8_t                                   socketFound      = false;
    phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket      = NULL;
    phlnLlcp_Fri_Transport_Socket_t          *psLocalLlcpSocket = NULL;
    pphlnLlcp_Fri_TransportSocketListenCb_t   pListen_Cb        = NULL;
    void                                     *pListenContext    = NULL;

    phNfc_sData_t                             sServiceName;
    uint8_t                                   remoteRW  = PHLNLLCP_FRI_RW_DEFAULT;
    uint16_t                                  remoteMIU = PHLNLLCP_FRI_MIU_DEFAULT;

    status = phlnLlcp_Fri_GetSocket_Params(psData,
                                           &sServiceName,
                                           &remoteRW,
                                           &remoteMIU);

    if(status != PH_ERR_SUCCESS)
    {
        /* Incorrect TLV */
        /* send FRMR */
        status  = phlnLlcp_Fri_Transport_SendFrameReject(psTransport,
                                                       dsap,
                                                       PHLNLLCP_FRI_PTYPE_CONNECT,
                                                       ssap,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00);
    }
    else
    {
        if(dsap == PHLNLLCP_FRI_SAP_SDP)
        {
            /* Search a socket with the SN */
            for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
            {
                /* Test if the socket is in Listen state and if its SN is the good one */
                if(psTransport->pSocketTable[index].bSocketListenPending
                    &&  (sServiceName.length == psTransport->pSocketTable[index].sServiceName.length)
                    && !memcmp(sServiceName.buffer,psTransport->pSocketTable[index].sServiceName.buffer,sServiceName.length))
                {
                    /* socket with the SN found */
                    socketFound = true;

                    psLocalLlcpSocket = &psTransport->pSocketTable[index];

                    /* Get the new ssap number, it is the ssap number of the socket found */
                    dsap = psLocalLlcpSocket->socket_sSap;
                    /* Get the ListenCB of the socket */
                    pListen_Cb = psLocalLlcpSocket->pfSocketListen_Cb;
                    pListenContext = psLocalLlcpSocket->pListenContext;
                    break;
                }
            }
        }
        else
        {
            /* Search a socket with the DSAP */
            for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
            {
                /* Test if the socket is in Listen state and if its port number is the good one */
                if(psTransport->pSocketTable[index].bSocketListenPending && psTransport->pSocketTable[index].socket_sSap == dsap)
                {
                    /* socket with the SN found */
                    socketFound = true;

                    psLocalLlcpSocket = &psTransport->pSocketTable[index];

                    /* Get the Listen CB and the Context of the socket */
                    pListen_Cb = psLocalLlcpSocket->pfSocketListen_Cb;
                    pListenContext = psLocalLlcpSocket->pListenContext;
                    break;
                }
            }
        }
    }

    /* Test if a socket has beeen found */
    if(socketFound)
    {
        /* Reset the FLAG socketFound*/
        socketFound = false;

        /* Search a socket free and no socket connect on this DSAP*/
        for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
        {
            if(psTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketDefault && socketFound != true)
            {
                socketFound = true;

                psTransport->pSocketTable[index].index = index;
                psTransport->socketIndex = psTransport->pSocketTable[index].index;

                /* Create a communication socket */
                pLlcpSocket = &psTransport->pSocketTable[index];

                /* Set the communication option of the Remote Socket */
                pLlcpSocket->remoteMIU = remoteMIU;
                pLlcpSocket->remoteRW  = remoteRW;

                /* Set SSAP/DSAP of the new socket created for the communication with the remote */
                pLlcpSocket->socket_dSap = ssap;
                pLlcpSocket->socket_sSap = dsap;

                /* Set the state and the type of the new socket */
                pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketBound;
                pLlcpSocket->eSocket_Type  = phlnLlcp_Fri_Transport_eConnectionOriented;
            }
            else if(((psTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketConnected)
                || (psTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketAccepted))
                && ((psTransport->pSocketTable[index].socket_sSap == ssap)&&(psTransport->pSocketTable[index].socket_dSap == dsap)))
            {
                socketFound = false;

                if(pLlcpSocket != NULL)
                {
                    /* Reset Socket Information */
                    pLlcpSocket->remoteMIU = 0;
                    pLlcpSocket->remoteRW  = 0;

                    /* Set SSAP/DSAP of the new socket created for the communication with the remote */
                    pLlcpSocket->socket_dSap = 0;
                    pLlcpSocket->socket_sSap = 0;

                    /* Set the state and the type of the new socket */
                    pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketDefault;
                    pLlcpSocket->eSocket_Type  = phlnLlcp_Fri_Transport_eDefaultType;
                    break;
                }
            }
            else
            {
                /* Nothing to Do */
            }
        }

        /* Test if a socket has been found */
        if(socketFound)
        {
            /* Call the Listen CB */
            pListen_Cb(pListenContext,pLlcpSocket);
        }
        else
        {
            /* No more socket are available */
            /* Send a DM (0x21) */
            status = phlnLlcp_Fri_Transport_SendDisconnectMode (psTransport,
                                                             ssap,
                                                             dsap,
                                                             PHLNLLCP_FRI_DM_OPCODE_SOCKET_NOT_AVAILABLE);
        }
    }
    else
    {
        /* Service Name not found or Port number not found */
        /* Send a DM (0x02) */
        status = phlnLlcp_Fri_Transport_SendDisconnectMode (psTransport,
                                                            ssap,
                                                            dsap,
                                                            PHLNLLCP_FRI_DM_OPCODE_SAP_NOT_FOUND);
    }
}

/* TODO: comment function Handle_ConnectFrame */
static void phlnLlcp_Fri_Handle_ConnectionCompleteFrame(phlnLlcp_Fri_Transport_t      *psTransport,
                                                        phNfc_sData_t                *psData,
                                                        uint8_t                       dsap,
                                                        uint8_t                       ssap)
{
    phStatus_t                         status = PH_ERR_SUCCESS;
    uint8_t                            index;
    uint8_t                            remoteRW  = PHLNLLCP_FRI_RW_DEFAULT;
    uint16_t                           remoteMIU = PHLNLLCP_FRI_MIU_DEFAULT;
    uint8_t                            socketFound = false;
    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket = NULL;

    status = phlnLlcp_Fri_GetSocket_Params(psData,
                                           NULL,
                                           &remoteRW,
                                           &remoteMIU);

    if(status != PH_ERR_SUCCESS)
    {
        /* Incorrect TLV */
        /* send FRMR */
        status  = phlnLlcp_Fri_Transport_SendFrameReject(psTransport,
                                                       dsap,
                                                       PHLNLLCP_FRI_PTYPE_CC,
                                                       ssap,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00);
    }
    else
    {
        /* Search a socket in connecting state and with the good SSAP */
        for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
        {
            /* Test if the socket is in Connecting state and if its SSAP number is the good one */
            if(psTransport->pSocketTable[index].eSocket_State  == phlnLlcp_Fri_TransportSocket_eSocketConnecting
                && psTransport->pSocketTable[index].socket_sSap == dsap)
            {
                /* socket with the SN found */
                socketFound = true;

                /* Update the DSAP value with the incomming Socket sSap */
                psTransport->pSocketTable[index].socket_dSap = ssap;

                /* Store a pointer to the socket found */
                psLocalLlcpSocket = &psTransport->pSocketTable[index];
                PH_ASSERT_NULL(psLocalLlcpSocket);
                break;
            }
        }
        /* Test if a socket has been found */
        if(socketFound)
        {
            /* Set the socket state to connected */
            psLocalLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketConnected;

            /* Reset the socket_VS,socket_VR,socket_VSA and socket_VRA variables */
            psLocalLlcpSocket->socket_VR  = 0;
            psLocalLlcpSocket->socket_VRA = 0;
            psLocalLlcpSocket->socket_VS  = 0;
            psLocalLlcpSocket->socket_VSA = 0;

            /* Store the Remote parameters (MIU,RW) */
            psLocalLlcpSocket->remoteMIU  = remoteMIU;
            psLocalLlcpSocket->remoteRW   = remoteRW;

            /* Call the Connect CB and reset callback info */
            psLocalLlcpSocket->pfSocketConnect_Cb(psLocalLlcpSocket->pConnectContext,0x00,PH_ERR_SUCCESS);
            psLocalLlcpSocket->pfSocketConnect_Cb = NULL;
            psLocalLlcpSocket->pConnectContext = NULL;
        }
        else
        {
            /* No socket Active */
            /* CC Frame not handled */
        }
    }
}

/* TODO: comment function phlnLlcp_Fri_Handle_DisconnectFrame */
static void phlnLlcp_Fri_Handle_DisconnectFrame(phlnLlcp_Fri_Transport_t      *psTransport,
                                                uint8_t                       dsap,
                                                uint8_t                       ssap)
{
    phStatus_t   status = PH_ERR_SUCCESS;
    uint8_t     index;
    uint8_t     socketFound = false;
    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket = NULL;

    PH_UNUSED_VARIABLE(status);

    /* Search a socket in connected state and the good SSAP */
    for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
    {
        /* Test if the socket is in Connected state and if its SSAP number is the good one */
        if(psTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketConnected
            && psTransport->pSocketTable[index].socket_sSap == dsap)
        {
            /* socket found */
            socketFound = true;

            /* Store a pointer to the socket found */
            psLocalLlcpSocket = &psTransport->pSocketTable[index];
            break;
        }
    }

    /* Test if a socket has been found */
    if(socketFound)
    {
        /* Test if a send IFRAME is pending with this socket */
        if((psLocalLlcpSocket->bSocketSendPending == true) || (psLocalLlcpSocket->bSocketRecvPending == true))
        {
            /* Call the send CB, a disconnect abort the send request */
            if (psLocalLlcpSocket->bSocketSendPending == true)
            {
                phlnLlcp_Fri_Transport_ConnectionOriented_TriggerSendCb(psLocalLlcpSocket, PH_ERR_FAILED);
            }
            /* Call the send CB, a disconnect abort the receive request */
            if (psLocalLlcpSocket->bSocketRecvPending == true)
            {
                phlnLlcp_Fri_Transport_ConnectionOriented_TriggerRecvCb(psLocalLlcpSocket, PH_ERR_FAILED);
            }
            psLocalLlcpSocket->bSocketRecvPending = false;
            psLocalLlcpSocket->bSocketSendPending = false;
        }

        /* Update the socket state */
        psLocalLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketDisconnecting;

        /* Send a DM*/
        /* TODO: use a socket internal flag to save */
        status = phlnLlcp_Fri_Transport_SendDisconnectMode(psTransport,
                                                         ssap,
                                                         dsap,
                                                         PHLNLLCP_FRI_DM_OPCODE_DISCONNECTED);

        /* Call ErrCB due to a DISC */
        psTransport->pSocketTable[index].pSocketErrCb(psTransport->pSocketTable[index].pContext, PHLNLLCP_FRI_ERR_DISCONNECTED);  /* PRQA S 3689 */
    }
    else
    {
        /* No socket Active */
        /* DISC Frame not handled */
    }
}

/* TODO: comment function Handle_ConnectFrame */
static void phlnLlcp_Fri_Handle_DisconnetModeFrame(phlnLlcp_Fri_Transport_t      *psTransport,
                                                   phNfc_sData_t                *psData,
                                                   uint8_t                       dsap,
                                                   uint8_t                       ssap)
{
    phStatus_t                          status = PH_ERR_SUCCESS;
    uint8_t                             index;
    uint8_t                             socketFound = false;
    uint8_t                             dmOpCode;
    phlnLlcp_Fri_Transport_Socket_t    *psLocalLlcpSocket = NULL;

    PH_UNUSED_VARIABLE(status);
    PH_ASSERT_NULL(psLocalLlcpSocket);

    /* Test if the DM buffer is correct */
    if(psData->length != PHLNLLCP_FRI_DM_LENGTH)
    {
        /* send FRMR */
        status  = phlnLlcp_Fri_Transport_SendFrameReject(psTransport,
                                                       dsap,
                                                       PHLNLLCP_FRI_PTYPE_DM,
                                                       ssap,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00,
                                                       0x00);
    }
    else
    {
        /* Search a socket waiting for a DM (Disconnecting State) */
        for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
        {
            /* Test if the socket is in Disconnecting  or connecting state and if its SSAP number is the good one */
            if((psTransport->pSocketTable[index].eSocket_State   == phlnLlcp_Fri_TransportSocket_eSocketDisconnecting
                || psTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketConnecting)
                && psTransport->pSocketTable[index].socket_sSap   == dsap)
            {
                /* socket found */
                socketFound = true;

                /* Store a pointer to the socket found */
                psLocalLlcpSocket = &psTransport->pSocketTable[index];
                break;
            }
        }

        /* Test if a socket has been found */
        if(socketFound)
        {
            /* Set dmOpcode */
            dmOpCode = psData->buffer[0];

            switch(dmOpCode)
            {
                case PHLNLLCP_FRI_DM_OPCODE_DISCONNECTED:
                {
                    /* Set the socket state to disconnected */
                    psLocalLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketCreated;

                    /* Call Disconnect CB */
                    if (psLocalLlcpSocket->pfSocketDisconnect_Cb != NULL)
                    {
                        psLocalLlcpSocket->pfSocketDisconnect_Cb(psLocalLlcpSocket->pDisonnectContext,PH_ERR_SUCCESS);
                        psLocalLlcpSocket->pfSocketDisconnect_Cb = NULL;
                    }

                }break;

            case PHLNLLCP_FRI_DM_OPCODE_CONNECT_REJECTED:
            case PHLNLLCP_FRI_DM_OPCODE_CONNECT_NOT_ACCEPTED:
            case PHLNLLCP_FRI_DM_OPCODE_SAP_NOT_ACTIVE:
            case PHLNLLCP_FRI_DM_OPCODE_SAP_NOT_FOUND:
            case PHLNLLCP_FRI_DM_OPCODE_SOCKET_NOT_AVAILABLE:
                {
                    /* Set the socket state to bound */
                    psLocalLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketCreated;
                    if(psLocalLlcpSocket->pfSocketConnect_Cb != NULL)
                    {
                        /* Call Connect CB */
                        psLocalLlcpSocket->pfSocketConnect_Cb(psLocalLlcpSocket->pConnectContext,dmOpCode,PH_ERR_FAILED);
                        psLocalLlcpSocket->pfSocketConnect_Cb = NULL;
                    }
                }break;
            default:
                break;
            }
        }
    }
}


/* TODO: comment function phlnLlcp_Fri_Handle_Receive_IFrame */
static void phlnLlcp_Fri_Handle_Receive_IFrame(phlnLlcp_Fri_Transport_t      *psTransport,
                                               phNfc_sData_t                 *psData,
                                               uint8_t                       dsap,
                                               uint8_t                       ssap)
{
    phStatus_t   status = PH_ERR_SUCCESS;

    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket = NULL;
    phlnLlcp_Fri_sPacketSequence_t     sLlcpLocalSequence;

    uint32_t    dataLengthAvailable = 0;
    uint32_t    dataLengthWrite = 0;
    uint8_t     index;
    uint8_t     socketFound = false;
    uint8_t     WFlag = 0;
    uint8_t     IFlag = 0;
    uint8_t     RFlag = 0;
    uint8_t     SFlag = 0;
    uint8_t     nr_val;
    uint32_t    offset = 0;
    uint32_t    rw_offset;

    PH_UNUSED_VARIABLE(dataLengthWrite);
    PH_UNUSED_VARIABLE(status);
    PH_ASSERT_NULL(psLocalLlcpSocket);

    /* Get NS and NR Value of the I Frame*/
    status = (uint16_t) phlnLlcp_Fri_Utils_Buffer2Sequence( psData->buffer, offset, &sLlcpLocalSequence);

    /* Update the buffer pointer */
    psData->buffer = psData->buffer + PHLNLLCP_FRI_PACKET_SEQUENCE_SIZE;

    /* Update the length value (without the header length) */
    psData->length = psData->length - PHLNLLCP_FRI_PACKET_SEQUENCE_SIZE;

    /* Search a socket waiting for an I FRAME (Connected State) */
    for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
    {
        /* Test if the socket is in connected state and if its SSAP and DSAP are valid */
        if((  (psTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketConnected)
            || (psTransport->pSocketTable[index].eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketAccepted))
            && psTransport->pSocketTable[index].socket_sSap == dsap
            && psTransport->pSocketTable[index].socket_dSap == ssap)
        {
            /* socket found */
            socketFound = true;

            /* Store a pointer to the socket found */
            psLocalLlcpSocket = &psTransport->pSocketTable[index];
            break;
        }
    }

    /* Test if a socket has been found */
    if(socketFound)
    {
         /* Calculate offset of current frame in RW, and check validity */
        if(sLlcpLocalSequence.ns >= psLocalLlcpSocket->socket_VRA)
        {
            rw_offset = sLlcpLocalSequence.ns - psLocalLlcpSocket->socket_VRA;
        }
        else
        {
            rw_offset = 16 - (psLocalLlcpSocket->socket_VRA - sLlcpLocalSequence.ns);
        }
        if(rw_offset >= psLocalLlcpSocket->localRW)
        {
            /* FRMR 0x01 */
            SFlag = true;
        }

        /* Check Info length */
        if(psData->length > (uint32_t)(psLocalLlcpSocket->localMIUX + PHLNLLCP_FRI_MIU_DEFAULT))
        {
            IFlag = true;
        }

        /* Test NR */
        nr_val = (uint8_t)sLlcpLocalSequence.nr;
        do
        {
            if(nr_val == psLocalLlcpSocket->socket_VS)
            {
                break;
            }

            nr_val = (nr_val+1)%16;

            if(nr_val == psLocalLlcpSocket->socket_VSA)
            {
                /* FRMR 0x02 */
                RFlag = true;
                break;
            }
        }while(nr_val != sLlcpLocalSequence.nr);

        if(IFlag != 0 || RFlag != 0 || SFlag != 0)
        {
            /* Send FRMR */
            status = phlnLlcp_Fri_Transport_SendFrameReject(psTransport,
                                                         dsap,
                                                         PHLNLLCP_FRI_PTYPE_I,
                                                         ssap,
                                                         &sLlcpLocalSequence,
                                                         WFlag,
                                                         IFlag,
                                                         RFlag,
                                                         SFlag,
                                                         psLocalLlcpSocket->socket_VS,
                                                         psLocalLlcpSocket->socket_VSA,
                                                         psLocalLlcpSocket->socket_VR,
                                                         psLocalLlcpSocket->socket_VRA);

        }
        else
        {
            /* Update VSA */
            psLocalLlcpSocket->socket_VSA = (uint8_t)sLlcpLocalSequence.nr;

            /* Test if the Linear Buffer length is null */
            if(psLocalLlcpSocket->bufferLinearLength == 0)
            {
                /* Test if a Receive is pending and RW empty */
                if(psLocalLlcpSocket->bSocketRecvPending == true && (psLocalLlcpSocket->indexRwWrite == psLocalLlcpSocket->indexRwRead))
                {
                    /* Reset Flag */
                    psLocalLlcpSocket->bSocketRecvPending = false;

                    /* Save I_FRAME into the Receive Buffer */
                    memcpy(psLocalLlcpSocket->sSocketRecvBuffer->buffer,psData->buffer,psData->length);  /* PRQA S 3200 */
                    psLocalLlcpSocket->sSocketRecvBuffer->length = psData->length;

                    /* Update VR */
                    psLocalLlcpSocket->socket_VR = (psLocalLlcpSocket->socket_VR+1)%16;

                    psLocalLlcpSocket->bRecvReady_NotRequired = 0;
                    /* Call the Receive CB */
                    phlnLlcp_Fri_Transport_ConnectionOriented_TriggerRecvCb(psLocalLlcpSocket, PH_ERR_SUCCESS);

                    /* Test if a send is pending with this socket */
                    if(psLocalLlcpSocket->bSocketSendPending == true && CHECK_SEND_RW(psLocalLlcpSocket))
                    {
                        /* Test if a send is pending at LLC layer */
                        if(psTransport->bSendPending != true)
                        {
                            status = phlnLlcp_Fri_performSendInfo(psLocalLlcpSocket);
                        }
                    }
                    else if (!psLocalLlcpSocket->bRecvReady_NotRequired)
                    {
                        /* RR */
                        status = phlnLlcp_Fri_Send_ReceiveReady_Frame(psLocalLlcpSocket);
                    }
                    else
                    {
                        /* No action */
                    }
                }
                else
                {
                    /* Test if RW is full */
                    if((psLocalLlcpSocket->indexRwWrite - psLocalLlcpSocket->indexRwRead)<psLocalLlcpSocket->localRW)
                    {
                        if(psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].length == 0)
                        {
                            /* Save I_FRAME into the RW Buffers */
                            memcpy(psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].buffer,psData->buffer,psData->length);  /* PRQA S 3200 */
                            psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].length = psData->length;

                            if(psLocalLlcpSocket->ReceiverBusyCondition != true)
                            {
                                /* Receiver Busy condition */
                                psLocalLlcpSocket->ReceiverBusyCondition = true;

                                /* Send RNR */
                                status = phlnLlcp_Fri_Send_ReceiveNotReady_Frame(psLocalLlcpSocket);
                            }
                            /* Update the RW write index */
                            psLocalLlcpSocket->indexRwWrite++;
                        }
                    }
                }
            }
            else
            {
                /* Copy the buffer into the RW buffer */
                memcpy(psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].buffer,psData->buffer,psData->length);  /* PRQA S 3200 */

                /* Update the length */
                psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].length = psData->length;

                /* Test the length of the available place in the linear buffer */
                dataLengthAvailable = phlnLlcp_Fri_Utils_CyclicFifoAvailable(&psLocalLlcpSocket->sCyclicFifoBuffer);

                if(dataLengthAvailable >= psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].length)
                {
                    /* Store Data into the linear buffer */
                    dataLengthWrite = phlnLlcp_Fri_Utils_CyclicFifoWrite(&psLocalLlcpSocket->sCyclicFifoBuffer,
                                                                        psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].buffer,
                                                                        psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].length);

                    /* Update VR */
                    psLocalLlcpSocket->socket_VR = (psLocalLlcpSocket->socket_VR+1)%16;

                    /* Update the length */
                    psLocalLlcpSocket->sSocketRwBufferTable[(psLocalLlcpSocket->indexRwWrite%psLocalLlcpSocket->localRW)].length = 0x00;

                    /* Test if a Receive Pending*/
                    if(psLocalLlcpSocket->bSocketRecvPending == true)
                    {
                        /* Reset Flag */
                        psLocalLlcpSocket->bSocketRecvPending = false;

                        status = phlnLlcp_Fri_Transport_ConnectionOriented_Recv(psLocalLlcpSocket, psLocalLlcpSocket->sSocketRecvBuffer,psLocalLlcpSocket->pfSocketRecv_Cb, psLocalLlcpSocket->pRecvContext);
                    }

                    /* Test if a send is pending with this socket */
                    if((psLocalLlcpSocket->bSocketSendPending == true) && CHECK_SEND_RW(psLocalLlcpSocket))
                    {
                        /* Test if a send is pending at LLC layer */
                        if(psTransport->bSendPending != true)
                        {
                            status = phlnLlcp_Fri_performSendInfo(psLocalLlcpSocket);
                        }
                    }
                    else
                    {
                        /* RR */
                        status = phlnLlcp_Fri_Send_ReceiveReady_Frame(psLocalLlcpSocket);
                    }
                }
                else
                {
                    if(psLocalLlcpSocket->ReceiverBusyCondition != true)
                    {
                        /* Receiver Busy condition */
                        psLocalLlcpSocket->ReceiverBusyCondition = true;

                        /* Send RNR */
                        status = phlnLlcp_Fri_Send_ReceiveNotReady_Frame(psLocalLlcpSocket);
                    }

                    /* Update the RW write index */
                    psLocalLlcpSocket->indexRwWrite++;
                }
            }
        }
    }
    else
    {
        /* No active  socket*/
        /* I FRAME not Handled */
    }
}

static void phlnLlcp_Fri_Handle_ReceiveReady_Frame(phlnLlcp_Fri_Transport_t      *psTransport,
                                                   phNfc_sData_t                 *psData,
                                                   uint8_t                        dsap,
                                                   uint8_t                        ssap)
{
    phStatus_t   status = PH_ERR_SUCCESS;
    uint8_t     index;
    uint8_t     socketFound = false;
    uint8_t      WFlag = 0;
    uint8_t      IFlag = 0;
    uint8_t      RFlag = 0;
    uint8_t      SFlag = 0;
    uint32_t     offset = 0;
    uint8_t      nr_val;

    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket = NULL;
    phlnLlcp_Fri_sPacketSequence_t     sLlcpLocalSequence;

    PH_ASSERT_NULL(psLocalLlcpSocket);
    PH_UNUSED_VARIABLE(status);

    /* Get NS and NR Value of the I Frame*/
    status = (uint16_t) phlnLlcp_Fri_Utils_Buffer2Sequence( psData->buffer, offset, &sLlcpLocalSequence);

    /* Search a socket waiting for an RR FRAME (Connected State) */
    for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
    {
        /* Test if the socket is in connected state and if its SSAP and DSAP are valid */
        if(psTransport->pSocketTable[index].eSocket_State  == phlnLlcp_Fri_TransportSocket_eSocketConnected
            && psTransport->pSocketTable[index].socket_sSap == dsap
            && psTransport->pSocketTable[index].socket_dSap == ssap)
        {
            /* socket found */
            socketFound = true;

            /* Store a pointer to the socket found */
            psLocalLlcpSocket = &psTransport->pSocketTable[index];
            psLocalLlcpSocket->index = psTransport->pSocketTable[index].index;
            break;
        }
    }

    /* Test if a socket has been found */
    if(socketFound)
    {
        /* Test NR */
        nr_val = (uint8_t)sLlcpLocalSequence.nr;
        do
        {
            if(nr_val == psLocalLlcpSocket->socket_VS)
            {
                break;
            }

            nr_val = (nr_val+1)%16;

            if(nr_val == psLocalLlcpSocket->socket_VSA)
            {
                RFlag = true;
                break;
            }

        }while(nr_val != sLlcpLocalSequence.nr);

        /* Test if Info field present */
        if(psData->length > 1)
        {
            WFlag = true;
            IFlag = true;
        }

        if (WFlag || IFlag || RFlag || SFlag)
        {
            /* Send FRMR */
            status = phlnLlcp_Fri_Transport_SendFrameReject(psTransport,
                                                            dsap, PHLNLLCP_FRI_PTYPE_RR, ssap,
                                                            &sLlcpLocalSequence,
                                                            WFlag, IFlag, RFlag, SFlag,
                                                            psLocalLlcpSocket->socket_VS,
                                                            psLocalLlcpSocket->socket_VSA,
                                                            psLocalLlcpSocket->socket_VR,
                                                            psLocalLlcpSocket->socket_VRA);
        }
        else
        {
            /* Test Receiver Busy condition */
            if(psLocalLlcpSocket->RemoteBusyConditionInfo == true)
            {
                /* Notify the upper layer */
                psLocalLlcpSocket->pSocketErrCb(psLocalLlcpSocket->pContext,PHLNLLCP_FRI_ERR_NOT_BUSY_CONDITION);
                psLocalLlcpSocket->RemoteBusyConditionInfo = false;
            }
            /* Update VSA */
            psLocalLlcpSocket->socket_VSA = (uint8_t)sLlcpLocalSequence.nr;

            /* Test if a send is pendind */
            if(psLocalLlcpSocket->bSocketSendPending == true)
            {
                /* Test if a send is pending at LLC layer */
                if(psTransport->bSendPending != true)
                {
                    status = phlnLlcp_Fri_performSendInfo(psLocalLlcpSocket);
                }
            }
        }
    }
    else
    {
        /* No active  socket*/
        /* RR Frame not handled*/
    }
}

static void phlnLlcp_Fri_Handle_ReceiveNotReady_Frame(phlnLlcp_Fri_Transport_t      *psTransport,
                                                      phNfc_sData_t                 *psData,
                                                      uint8_t                        dsap,
                                                      uint8_t                        ssap)
{
    phStatus_t   status = PH_ERR_SUCCESS;
    uint8_t     index;
    uint8_t     socketFound = false;
    uint8_t      bWFlag = 0;
    uint8_t      bIFlag = 0;
    uint8_t      bRFlag = 0;
    uint8_t      bSFlag = 0;
    uint32_t    offset = 0;
    uint8_t     nr_val;

    phlnLlcp_Fri_Transport_Socket_t   *psLocalLlcpSocket = NULL;
    phlnLlcp_Fri_sPacketSequence_t     sLlcpLocalSequence;

    PH_UNUSED_VARIABLE(status);
    PH_ASSERT_NULL(psLocalLlcpSocket);

    /* Get NS and NR Value of the I Frame*/
    status = (uint16_t) phlnLlcp_Fri_Utils_Buffer2Sequence( psData->buffer, offset, &sLlcpLocalSequence);

    /* Search a socket waiting for an RNR FRAME (Connected State) */
    for(index=0;index<PHLNLLCP_FRI_NB_SOCKET_MAX;index++)
    {
        /* Test if the socket is in connected state and if its SSAP and DSAP are valid */
        if(psTransport->pSocketTable[index].eSocket_State  == phlnLlcp_Fri_TransportSocket_eSocketConnected
            && psTransport->pSocketTable[index].socket_sSap == dsap
            && psTransport->pSocketTable[index].socket_dSap == ssap)
        {
            /* socket found */
            socketFound = true;

            /* Store a pointer to the socket found */
            psLocalLlcpSocket = &psTransport->pSocketTable[index];
            break;
        }
    }

    /* Test if a socket has been found */
    if(socketFound)
    {
        /* Test NR */
        nr_val = (uint8_t)sLlcpLocalSequence.nr;
        do
        {
            if(nr_val == psLocalLlcpSocket->socket_VS)
            {
                break;
            }

            nr_val = (nr_val+1)%16;

            if(nr_val == psLocalLlcpSocket->socket_VSA)
            {
                /* FRMR 0x02 */
                bRFlag = true;
                break;
            }
        }while(nr_val != sLlcpLocalSequence.nr);

        /* Test if Info field present */
        if(psData->length > 1)
        {
            /* Send FRMR */
            bWFlag = true;
            bIFlag = true;
        }

        if( bWFlag != 0 || bIFlag != 0 || bRFlag != 0)
        {
            /* Send FRMR */
            status = phlnLlcp_Fri_Transport_SendFrameReject(psTransport,
                                                         dsap, PHLNLLCP_FRI_PTYPE_RNR, ssap,
                                                         &sLlcpLocalSequence,
                                                         bWFlag, bIFlag, bRFlag, bSFlag,
                                                         psLocalLlcpSocket->socket_VS,
                                                         psLocalLlcpSocket->socket_VSA,
                                                         psLocalLlcpSocket->socket_VR,
                                                         psLocalLlcpSocket->socket_VRA);
        }
        else
        {
            /* Notify the upper layer */
            psLocalLlcpSocket->pSocketErrCb(psTransport->pSocketTable[index].pContext,PHLNLLCP_FRI_ERR_BUSY_CONDITION);  /* PRQA S 3689 */
            psLocalLlcpSocket->RemoteBusyConditionInfo = true;

            /* Update VSA */
            psLocalLlcpSocket->socket_VSA = (uint8_t)sLlcpLocalSequence.nr;

            /* Test if a send is pendind */
            if(psLocalLlcpSocket->bSocketSendPending == true && CHECK_SEND_RW(psLocalLlcpSocket))
            {
                /* Test if a send is pending at LLC layer */
                if(psTransport->bSendPending != true)
                {
                    status = phlnLlcp_Fri_performSendInfo(psLocalLlcpSocket);
                }
            }
        }
    }
    else
    {
        /* No active  socket*/
        /* RNR Frame not handled*/
    }
}

static void phlnLlcp_Fri_Handle_FrameReject_Frame(phlnLlcp_Fri_Transport_t      *psTransport,
                                                  uint8_t                        dsap,
                                                  uint8_t                        ssap)
{
    phStatus_t   status = PH_ERR_SUCCESS;
    uint8_t     index;
    uint8_t     socketFound = false;

    PH_UNUSED_VARIABLE(status);

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
        psTransport->pSocketTable[index].eSocket_State =  phlnLlcp_Fri_TransportSocket_eSocketDisconnected;  /* PRQA S 3689 */

        /* Call ErrCB due to a FRMR*/
        psTransport->pSocketTable[index].pSocketErrCb( psTransport->pSocketTable[index].pContext,PHLNLLCP_FRI_ERR_FRAME_REJECTED);  /* PRQA S 3689 */

        /* Close the socket */
        status = phlnLlcp_Fri_Transport_ConnectionOriented_Close(&psTransport->pSocketTable[index]);  /* PRQA S 3689 */
    }
    else
    {
        /* No active  socket*/
        /* FRMR Frame not handled*/
    }
}

/* TODO: comment function phlnLlcp_Fri_Handle_ConnectionOriented_IncommingFrame */
void phlnLlcp_Fri_Handle_ConnectionOriented_IncommingFrame(phlnLlcp_Fri_Transport_t          *psTransport,
                                                           phNfc_sData_t                     *psData,
                                                           uint8_t                            dsap,
                                                           uint8_t                            ptype,
                                                           uint8_t                            ssap)
{
    phStatus_t           status = PH_ERR_SUCCESS;
    phlnLlcp_Fri_sPacketSequence_t  sSequence = {0,0};

    PH_UNUSED_VARIABLE(status);

    switch(ptype)
    {
    case PHLNLLCP_FRI_PTYPE_CONNECT:
        {
            phlnLlcp_Fri_Handle_ConnectionFrame(psTransport,
                                                psData,
                                                dsap,
                                                ssap);
        }break;

    case PHLNLLCP_FRI_PTYPE_DISC:
        {
            phlnLlcp_Fri_Handle_DisconnectFrame(psTransport,
                                                dsap,
                                                ssap);
         }break;

      case PHLNLLCP_FRI_PTYPE_CC:
         {
            phlnLlcp_Fri_Handle_ConnectionCompleteFrame(psTransport,
                                                        psData,
                                                        dsap,
                                                        ssap);
         }break;

    case PHLNLLCP_FRI_PTYPE_DM:
        {
            phlnLlcp_Fri_Handle_DisconnetModeFrame(psTransport,
                                                  psData,
                                                  dsap,
                                                  ssap);
        }break;

    case PHLNLLCP_FRI_PTYPE_FRMR:
        {
            phlnLlcp_Fri_Handle_FrameReject_Frame(psTransport,
                                                  dsap,
                                                  ssap);
        }break;

    case PHLNLLCP_FRI_PTYPE_I:
        {
            phlnLlcp_Fri_Handle_Receive_IFrame(psTransport,
                                               psData,
                                               dsap,
                                               ssap);
         }break;

    case PHLNLLCP_FRI_PTYPE_RR:
        {
            phlnLlcp_Fri_Handle_ReceiveReady_Frame(psTransport,
                                                  psData,
                                                  dsap,
                                                  ssap);
         }break;

    case PHLNLLCP_FRI_PTYPE_RNR:
        {
            phlnLlcp_Fri_Handle_ReceiveNotReady_Frame(psTransport,
                                                     psData,
                                                     dsap,
                                                     ssap);
         }break;

    case PHLNLLCP_FRI_PTYPE_RESERVED1:
    case PHLNLLCP_FRI_PTYPE_RESERVED2:
    case PHLNLLCP_FRI_PTYPE_RESERVED3:
        {
            status = phlnLlcp_Fri_Transport_SendFrameReject( psTransport,dsap, ptype, ssap,&sSequence, true, false, false, false, 0, 0, 0, 0);
        }break;
    default:
        break;
    }
}

/**
* \brief <b>Get the local options of a socket</b>.
*
* This function returns the local options (maximum packet size and receive window size) used
* for a given connection-oriented socket. This function shall not be used with connectionless
* sockets.
*
* \param[out] pLlcpSocket           A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  psLocalOptions        A pointer to be filled with the local options of the socket.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_NOT_INITIALISED          Indicates stack is not yet initialized.
* \retval PHLNLLCP_FRI_NFCSTATUS_SHUTDOWN                 Shutdown in progress.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_SocketGetLocalOptions(phlnLlcp_Fri_Transport_Socket_t  *pLlcpSocket,
                                                                           phlnLlcp_Fri_sSocketOptions_t    *psLocalOptions)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Get Local MIUX */
    psLocalOptions->miu = pLlcpSocket->sSocketOption.miu;

    /* Get Local Receive Window */
    psLocalOptions->rw = pLlcpSocket->sSocketOption.rw;

    return status;
}

/**
* \brief <b>Get the local options of a socket</b>.
*
* This function returns the remote options (maximum packet size and receive window size) used
* for a given connection-oriented socket. This function shall not be used with connectionless
* sockets.
*
* \param[out] pLlcpSocket           A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  psRemoteOptions       A pointer to be filled with the remote options of the socket.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_NOT_INITIALISED          Indicates stack is not yet initialized.
* \retval PHLNLLCP_FRI_NFCSTATUS_SHUTDOWN                 Shutdown in progress.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_SocketGetRemoteOptions(phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket,
                                                                            phlnLlcp_Fri_sSocketOptions_t     *psRemoteOptions)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Get Remote MIUX */
    psRemoteOptions->miu = pLlcpSocket->remoteMIU;

    /* Get Remote  Receive Window */
    psRemoteOptions->rw = pLlcpSocket->remoteRW;

    return status;
}


/**
*
* \brief <b>Listen for incoming connection requests on a socket</b>.
*
* This function switches a socket into a listening state and registers a callback on
* incoming connection requests. In this state, the socket is not able to communicate
* directly. The listening state is only available for connection-oriented sockets
* which are still not connected. The socket keeps listening until it is closed, and
* thus can trigger several times the pListen_Cb callback.
*
*
* \param[in]  pLlcpSocket        A pointer to a FriNfc_LlcpTransport_Socket_t.
* \param[in]  pListen_Cb         The callback to be called each time the
*                                socket receive a connection request.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state to switch
*                                            to listening state.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Listen(phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
                                                            pphlnLlcp_Fri_TransportSocketListenCb_t   pListen_Cb,
                                                            void*                                     pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Store the listen callback */
    pLlcpSocket->pfSocketListen_Cb = pListen_Cb;

    /* store the context */
    pLlcpSocket->pListenContext = pContext;

    /* Set RecvPending to true */
    pLlcpSocket->bSocketListenPending = true;

    /* Set the socket state*/
    pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketRegistered;

    return status;
}

/**
*
* \brief <b>Accept an incoming connection request for a socket</b>.
*
* This functions allows the client to accept an incoming connection request.
* It must be used with the socket provided within the listen callback. The socket
* is implicitly switched to the connected state when the function is called.
*
* \param[in]  pLlcpSocket           A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  psOptions             The options to be used with the socket.
* \param[in]  psWorkingBuffer       A working buffer to be used by the library.
* \param[in]  pErr_Cb               The callback to be called each time the accepted socket
*                                   is in error.
* \param[in]  pAccept_RspCb         The callback to be called when the Accept operation is completed
* \param[in]  pContext              Upper layer context to be returned in the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_BUFFER_TOO_SMALL         The working buffer is too small for the MIU and RW
*                                            declared in the options.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Accept(phlnLlcp_Fri_Transport_Socket_t             *pLlcpSocket,
                                                            phlnLlcp_Fri_Transport_sSocketOptions_t     *psOptions,
                                                            phNfc_sData_t                               *psWorkingBuffer,
                                                            pphlnLlcp_Fri_TransportSocketErrCb_t         pErr_Cb,
                                                            pphlnLlcp_Fri_TransportSocketAcceptCb_t      pAccept_RspCb,
                                                            void                                        *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    uint32_t offset = 0;
    uint8_t  pMiux[2];
    uint8_t  i;

    /* Store the options in the socket */
    memcpy(&pLlcpSocket->sSocketOption, psOptions, sizeof(phlnLlcp_Fri_Transport_sSocketOptions_t));  /* PRQA S 3200 */

    /* Set socket local params (MIUX & RW) */
    pLlcpSocket ->localMIUX = (pLlcpSocket->sSocketOption.miu - PHLNLLCP_FRI_MIU_DEFAULT) & PHLNLLCP_FRI_TLV_MIUX_MASK;
    pLlcpSocket ->localRW   = pLlcpSocket->sSocketOption.rw & PHLNLLCP_FRI_TLV_RW_MASK;

    /* Set the pointer and the length for the Receive Window Buffer */
    for(i=0;i<pLlcpSocket->localRW;i++)
    {
        pLlcpSocket->sSocketRwBufferTable[i].buffer = psWorkingBuffer->buffer + (i*pLlcpSocket->sSocketOption.miu);
        pLlcpSocket->sSocketRwBufferTable[i].length = 0;
    }

    /* Set the pointer and the length for the Send Buffer */
    pLlcpSocket->sSocketSendBuffer.buffer     = psWorkingBuffer->buffer + pLlcpSocket->bufferRwMaxLength;
    pLlcpSocket->sSocketSendBuffer.length     = pLlcpSocket->bufferSendMaxLength;

    /* Set the pointer and the length for the Linear Buffer */
    pLlcpSocket->sSocketLinearBuffer.buffer   = psWorkingBuffer->buffer + pLlcpSocket->bufferRwMaxLength + pLlcpSocket->bufferSendMaxLength;
    pLlcpSocket->sSocketLinearBuffer.length   = pLlcpSocket->bufferLinearLength;

    if(pLlcpSocket->sSocketLinearBuffer.length != 0)
    {
        /* Init Cyclic Fifo */
        phlnLlcp_Fri_Utils_CyclicFifoInit(&pLlcpSocket->sCyclicFifoBuffer,
                                   pLlcpSocket->sSocketLinearBuffer.buffer,
                                   pLlcpSocket->sSocketLinearBuffer.length);
    }

    pLlcpSocket->pSocketErrCb            = pErr_Cb;
    pLlcpSocket->pContext                = pContext;

    /* store the pointer to the Accept callback */
    pLlcpSocket->pfSocketAccept_Cb   = pAccept_RspCb;
    pLlcpSocket->pAcceptContext      = pContext;

    /* Reset the socket_VS,socket_VR,socket_VSA and socket_VRA variables */
    pLlcpSocket->socket_VR  = 0;
    pLlcpSocket->socket_VRA = 0;
    pLlcpSocket->socket_VS  = 0;
    pLlcpSocket->socket_VSA = 0;

    /* MIUX */
    if(pLlcpSocket->localMIUX != PHLNLLCP_FRI_MIUX_DEFAULT)
    {
        /* Encode MIUX value */
        pMiux[0] = (uint8_t)(pLlcpSocket->localMIUX >> 8);
        pMiux[1] = (uint8_t)(pLlcpSocket->localMIUX & 0xff);

        /* Encode MIUX in TLV format */
        status =  phlnLlcp_Fri_Utils_EncodeTLV(&pLlcpSocket->sSocketSendBuffer,
                                        &offset,
                                        PHLNLLCP_FRI_TLV_TYPE_MIUX,
                                        PHLNLLCP_FRI_TLV_LENGTH_MIUX,
                                        pMiux);
        if(status != PH_ERR_SUCCESS)
        {
            /* Call the CB */
            status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_TRANSPORT);
            goto clean_and_return;
        }
    }

    /* Receive Window */
    if(pLlcpSocket->sSocketOption.rw != PHLNLLCP_FRI_RW_DEFAULT)
    {
        /* Encode RW value */
        phlnLlcp_Fri_Utils_EncodeRW(&pLlcpSocket->sSocketOption.rw);

        /* Encode RW in TLV format */
        status =  phlnLlcp_Fri_Utils_EncodeTLV(&pLlcpSocket->sSocketSendBuffer,
                                          &offset,
                                          PHLNLLCP_FRI_TLV_TYPE_RW,
                                          PHLNLLCP_FRI_TLV_LENGTH_RW,
                                          &pLlcpSocket->sSocketOption.rw);
        if(status != PH_ERR_SUCCESS)
        {
            /* Call the CB */
            status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_TRANSPORT);
            goto clean_and_return;
        }
    }


    /* Test if a send is pending */
    if(pLlcpSocket->psTransport->bSendPending == true)
    {
        pLlcpSocket->bSocketAcceptPending = true;

        /* Update Send Buffer length value */
        pLlcpSocket->sSocketSendBuffer.length = offset;

        status = PH_ERR_PENDING;
    }
    else
    {
        /* Fill the psLlcpHeader stuture with the DSAP,CC PTYPE and the SSAP */
        pLlcpSocket->sLlcpHeader.dsap  = pLlcpSocket->socket_dSap;
        pLlcpSocket->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_CC;
        pLlcpSocket->sLlcpHeader.ssap  = pLlcpSocket->socket_sSap;

        /* Send Pending */
        pLlcpSocket->psTransport->bSendPending = true;

        /* Set the socket state to accepted */
        pLlcpSocket->eSocket_State           = phlnLlcp_Fri_TransportSocket_eSocketAccepted;

        /* Update Send Buffer length value */
        pLlcpSocket->sSocketSendBuffer.length = offset;

        /* Store the index of the socket */
        pLlcpSocket->psTransport->socketIndex = pLlcpSocket->index;

        /* Send a CC Frame */
        status =  phlnLlcp_Fri_Transport_LinkSend(pLlcpSocket->psTransport,
                                   &pLlcpSocket->sLlcpHeader,
                                   NULL,
                                   &pLlcpSocket->sSocketSendBuffer,
                                   &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                   pLlcpSocket->psTransport);
    }

clean_and_return:
    if(status != PH_ERR_PENDING)
    {
        pLlcpSocket->pfSocketAccept_Cb = NULL;
        pLlcpSocket->pAcceptContext = NULL;
    }

    return status;
}

/**
*
* \brief <b>Reject an incoming connection request for a socket</b>.
*
* This functions allows the client to reject an incoming connection request.
* It must be used with the socket provided within the listen callback. The socket
* is implicitly closed when the function is called.
*
* \param[in]  pLlcpSocket           A pointer to a phlnLlcp_Fri_Transport_Socket_t.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Reject( phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
                                                            pphlnLlcp_Fri_TransportSocketRejectCb_t    pReject_RspCb,
                                                            void                                      *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Set the state of the socket */
    pLlcpSocket->eSocket_State   = phlnLlcp_Fri_TransportSocket_eSocketRejected;

    /* Store the Reject callback */
    pLlcpSocket->pfSocketSend_Cb = pReject_RspCb;
    pLlcpSocket->pRejectContext  = pContext;

    /* Store the index of the socket */
    pLlcpSocket->psTransport->socketIndex = pLlcpSocket->index;

    /* Send a DM*/
    status = phlnLlcp_Fri_Transport_SendDisconnectMode(pLlcpSocket->psTransport,
                                                      pLlcpSocket->socket_dSap,
                                                      pLlcpSocket->socket_sSap,
                                                      PHLNLLCP_FRI_DM_OPCODE_CONNECT_REJECTED);

    return status;
}

/**
*
* \brief <b>Try to establish connection with a socket on a remote SAP</b>.
*
* This function tries to connect to a given SAP on the remote peer. If the
* socket is not bound to a local SAP, it is implicitly bound to a free SAP.
*
* \param[in]  pLlcpSocket        A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  nSap               The destination SAP to connect to.
* \param[in]  psUri              The URI corresponding to the destination SAP to connect to.
* \param[in]  pConnect_RspCb     The callback to be called when the connection
*                                operation is completed.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_PENDING                  Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Connect( phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
                                                             uint8_t                                    nSap,
                                                             phNfc_sData_t                             *psUri,
                                                             pphlnLlcp_Fri_TransportSocketConnectCb_t   pConnect_RspCb,
                                                             void                                      *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    uint32_t offset = 0;
    uint8_t  pMiux[2];

    /* Test if a nSap is present */
    if(nSap != PHLNLLCP_FRI_SAP_DEFAULT)
    {
        /* Set DSAP port number with the nSap value */
        pLlcpSocket->socket_dSap = nSap;
    }
    else
    {
        /* Set DSAP port number with the SDP port number */
        pLlcpSocket->socket_dSap = PHLNLLCP_FRI_SAP_SDP;
    }

    /* Store the Connect callback and context */
    pLlcpSocket->pfSocketConnect_Cb = pConnect_RspCb;
    pLlcpSocket->pConnectContext = pContext;

    /* Set the socket Header */
    pLlcpSocket->sLlcpHeader.dsap  = pLlcpSocket->socket_dSap;
    pLlcpSocket->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_CONNECT;
    pLlcpSocket->sLlcpHeader.ssap  = pLlcpSocket->socket_sSap;

    /* MIUX */
    if(pLlcpSocket->localMIUX != PHLNLLCP_FRI_MIUX_DEFAULT)
    {
        /* Encode MIUX value */
        pMiux[0] = (uint8_t)(pLlcpSocket->localMIUX >> 8);
        pMiux[1] = (uint8_t)(pLlcpSocket->localMIUX & 0xff);

        /* Encode MIUX in TLV format */
        status =  phlnLlcp_Fri_Utils_EncodeTLV(&pLlcpSocket->sSocketSendBuffer,
                                            &offset,
                                            PHLNLLCP_FRI_TLV_TYPE_MIUX,
                                            PHLNLLCP_FRI_TLV_LENGTH_MIUX,
                                            pMiux);
        if(status != PH_ERR_SUCCESS)
        {
            status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_TRANSPORT);
            goto clean_and_return;
        }
    }

    /* Receive Window */
    if(pLlcpSocket->sSocketOption.rw != PHLNLLCP_FRI_RW_DEFAULT)
    {
        /* Encode RW value */
        phlnLlcp_Fri_Utils_EncodeRW(&pLlcpSocket->sSocketOption.rw);

        /* Encode RW in TLV format */
        status =  phlnLlcp_Fri_Utils_EncodeTLV(&pLlcpSocket->sSocketSendBuffer,
                                        &offset,
                                        PHLNLLCP_FRI_TLV_TYPE_RW,
                                        PHLNLLCP_FRI_TLV_LENGTH_RW,
                                        &pLlcpSocket->sSocketOption.rw);
        if(status != PH_ERR_SUCCESS)
        {
            status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_TRANSPORT);
            goto clean_and_return;
        }
    }

    /* Test if a Service Name is present */
    if(psUri != NULL)
    {
        /* Encode SN in TLV format */
        status =  phlnLlcp_Fri_Utils_EncodeTLV(&pLlcpSocket->sSocketSendBuffer,
                                        &offset,
                                        PHLNLLCP_FRI_TLV_TYPE_SN,
                                        (uint8_t)psUri->length,
                                        psUri->buffer);
        if(status != PH_ERR_SUCCESS)
        {
            status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_TRANSPORT);
            goto clean_and_return;
        }
    }

    /* Test if a send is pending */
    if(pLlcpSocket->psTransport->bSendPending == true)
    {
        pLlcpSocket->bSocketConnectPending =  true;

        /* Update Send Buffer length value */
        pLlcpSocket->sSocketSendBuffer.length = offset;

        status = PH_ERR_PENDING;
    }
    else
    {
        /* Send Pending */
        pLlcpSocket->psTransport->bSendPending = true;

        /* Update Send Buffer length value */
        pLlcpSocket->sSocketSendBuffer.length = offset;

        /* Set the socket in connecting state */
        pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketConnecting;

        /* Store the index of the socket */
        pLlcpSocket->psTransport->socketIndex = pLlcpSocket->index;

        status =  phlnLlcp_Fri_Transport_LinkSend(pLlcpSocket->psTransport,
                                   &pLlcpSocket->sLlcpHeader,
                                   NULL,
                                   &pLlcpSocket->sSocketSendBuffer,
                                   &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                   pLlcpSocket->psTransport);
    }

clean_and_return:
    if(status != PH_ERR_PENDING)
    {
        pLlcpSocket->pfSocketConnect_Cb = NULL;
        pLlcpSocket->pConnectContext = NULL;
    }

    return status;
}


/**
* \brief <b>Disconnect a currently connected socket</b>.
*
* This function initiates the disconnection of a previously connected socket.
*
* \param[in]  pLlcpSocket        A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  pDisconnect_RspCb  The callback to be called when the
*                                operation is completed.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_PENDING                  Disconnection operation is in progress,
*                                            pDisconnect_RspCb will be called upon completion.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_NOT_INITIALISED          Indicates stack is not yet initialized.
* \retval PHLNLLCP_FRI_NFCSTATUS_SHUTDOWN                 Shutdown in progress.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Disconnect( phlnLlcp_Fri_Transport_Socket_t         *pLlcpSocket,
                                                                pphlnLlcp_Fri_LlcpSocketDisconnectCb_t    pDisconnect_RspCb,
                                                                void                                     *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /* Store the Disconnect callback  and context*/
    pLlcpSocket->pfSocketDisconnect_Cb = pDisconnect_RspCb;
    pLlcpSocket->pDisonnectContext = pContext;

    /* Set the socket in connecting state */
    pLlcpSocket->eSocket_State = phlnLlcp_Fri_TransportSocket_eSocketDisconnecting;

    /* Test if a send IFRAME is pending with this socket */
    if((pLlcpSocket->bSocketSendPending == true) || (pLlcpSocket->bSocketRecvPending == true))
    {
        pLlcpSocket->bSocketSendPending = false;
        pLlcpSocket->bSocketRecvPending = false;

        /* Call the send CB, a disconnect abort the send request */
        phlnLlcp_Fri_Transport_ConnectionOriented_TriggerSendCb(pLlcpSocket, PH_ERR_FAILED);
        /* Call the send CB, a disconnect abort the receive request */
        phlnLlcp_Fri_Transport_ConnectionOriented_TriggerRecvCb(pLlcpSocket, PH_ERR_FAILED);
    }

    /* Set the socket Header */
    pLlcpSocket->sLlcpHeader.dsap  = pLlcpSocket->socket_dSap;
    pLlcpSocket->sLlcpHeader.ptype = PHLNLLCP_FRI_PTYPE_DISC;
    pLlcpSocket->sLlcpHeader.ssap  = pLlcpSocket->socket_sSap;

    /* Test if a send is pending */
    if( pLlcpSocket->psTransport->bSendPending == true)
    {
        pLlcpSocket->bSocketDiscPending =  true;
        status = PH_ERR_PENDING;
    }
    else
    {
        /* Send Pending */
        pLlcpSocket->psTransport->bSendPending = true;

        /* Store the index of the socket */
        pLlcpSocket->psTransport->socketIndex = pLlcpSocket->index;

        status =  phlnLlcp_Fri_Transport_LinkSend(pLlcpSocket->psTransport,
                                   &pLlcpSocket->sLlcpHeader,
                                   NULL,
                                   NULL,
                                   &phlnLlcp_Fri_Transport_ConnectionOriented_SendLlcp_CB,
                                   pLlcpSocket->psTransport);
        if(status != PH_ERR_PENDING)
        {
            pLlcpSocket->pfSocketConnect_Cb = NULL;
            pLlcpSocket->pConnectContext = NULL;
        }
    }

    return status;
}

/* TODO: comment function phlnLlcp_Fri_Transport_Connectionless_SendTo_CB */
static void phlnLlcp_Fri_Transport_ConnectionOriented_DisconnectClose_CB(void          *pContext,
                                                                         phStatus_t     status)
{  /* PRQA S 0779 */
    phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket = (phlnLlcp_Fri_Transport_Socket_t*)pContext;

    if(status == PH_ERR_SUCCESS)
    {
        /* Reset the pointer to the socket closed */
        pLlcpSocket->eSocket_State            = phlnLlcp_Fri_TransportSocket_eSocketDefault;
        pLlcpSocket->eSocket_Type             = phlnLlcp_Fri_Transport_eDefaultType;
        pLlcpSocket->pContext                 = NULL;
        pLlcpSocket->pSocketErrCb             = NULL;
        pLlcpSocket->socket_sSap              = PHLNLLCP_FRI_SAP_DEFAULT;
        pLlcpSocket->socket_dSap              = PHLNLLCP_FRI_SAP_DEFAULT;
        pLlcpSocket->bSocketRecvPending       = false;
        pLlcpSocket->bSocketSendPending       = false;
        pLlcpSocket->bSocketListenPending     = false;
        pLlcpSocket->bSocketDiscPending       = false;
        pLlcpSocket->socket_VS                = 0;
        pLlcpSocket->socket_VSA               = 0;
        pLlcpSocket->socket_VR                = 0;
        pLlcpSocket->socket_VRA               = 0;
        pLlcpSocket->indexRwRead              = 0;
        pLlcpSocket->indexRwWrite             = 0;

        phlnLlcp_Fri_Transport_ConnectionOriented_Abort(pLlcpSocket);

        memset(&pLlcpSocket->sSocketOption, 0x00, sizeof(phlnLlcp_Fri_Transport_sSocketOptions_t));  /* PRQA S 3200 */

        pLlcpSocket->sServiceName.length = 0;
    }
    else
    {
        /* Disconnect close Error */
    }
}

/**
* \brief <b>Close a socket on a LLCP-connected device</b>.
*
* This function closes a LLCP socket previously created using phlnLlcp_Fri_Transport_Socket.
* If the socket was connected, it is first disconnected, and then closed.
*
* \param[in]  pLlcpSocket                    A pointer to a phlnLlcp_Fri_Transport_Socket_t.

* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Close(phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket)
{
    phStatus_t status = PH_ERR_SUCCESS;

    PH_UNUSED_VARIABLE(status);

    if(pLlcpSocket->eSocket_State == phlnLlcp_Fri_TransportSocket_eSocketConnected)
    {
        status = phlnLlcp_Fri_Transport_ConnectionOriented_Disconnect(pLlcpSocket,
                                                                    &phlnLlcp_Fri_Transport_ConnectionOriented_DisconnectClose_CB,
                                                                    pLlcpSocket);
    }
    else
    {
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

        pLlcpSocket->indexRwRead                        = 0;
        pLlcpSocket->indexRwWrite                       = 0;

        phlnLlcp_Fri_Transport_ConnectionOriented_Abort(pLlcpSocket);

        memset(&pLlcpSocket->sSocketOption, 0x00, sizeof(phlnLlcp_Fri_Transport_sSocketOptions_t));    /* PRQA S 3200 */

        pLlcpSocket->sServiceName.length = 0;
    }
    return PH_ERR_SUCCESS;
}


/**
*
* \brief <b>Send data on a socket</b>.
*
* This function is used to write data on a socket. This function
* can only be called on a connection-oriented socket which is already
* in a connected state.
*
* \param[in]  pLlcpSocket        A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  psBuffer           The buffer containing the data to send.
* \param[in]  pSend_RspCb        The callback to be called when the
*                                operation is completed.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_PENDING                  Reception operation is in progress,
*                                            pSend_RspCb will be called upon completion.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Send(phlnLlcp_Fri_Transport_Socket_t         *pLlcpSocket,
                                                          phNfc_sData_t                           *psBuffer,
                                                          pphlnLlcp_Fri_TransportSocketSendCb_t    pSend_RspCb,
                                                          void                                    *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;


    /* Test the RW window */
    if(!CHECK_SEND_RW(pLlcpSocket))
    {
        /* Store the Send CB and context */
        pLlcpSocket->pfSocketSend_Cb    = pSend_RspCb;
        pLlcpSocket->pSendContext       = pContext;

        /* Set Send pending */
        pLlcpSocket->bSocketSendPending = true;

        /* Store send buffer pointer */
        pLlcpSocket->sSocketSendBuffer = *psBuffer;

        /* Set status */
        status = PH_ERR_PENDING;
    }
    else
    {
        /* Store send buffer pointer */
        pLlcpSocket->sSocketSendBuffer = *psBuffer;

        /* Store the Send CB and context */
        pLlcpSocket->pfSocketSend_Cb    = pSend_RspCb;
        pLlcpSocket->pSendContext       = pContext;

        /* Test if a send is pending */
        if(pLlcpSocket->psTransport->bSendPending == true)
        {
            /* Set Send pending */
            pLlcpSocket->bSocketSendPending = true;

            /* Set status */
            status = PH_ERR_PENDING;
        }
        else
        {
            /* Perform I-Frame send */
            status = phlnLlcp_Fri_performSendInfo(pLlcpSocket);
            if(status != PH_ERR_PENDING)
            {
                pLlcpSocket->pfSocketSend_Cb = NULL;
                pLlcpSocket->pSendContext = NULL;
            }
        }

    }
    return status;
}


/**
*
* \brief <b>Read data on a socket</b>.
*
* This function is used to read data from a socket. It reads at most the
* size of the reception buffer, but can also return less bytes if less bytes
* are available. If no data is available, the function will be pending until
* more data comes, and the response will be sent by the callback. This function
* can only be called on a connection-oriented socket.
*
*
* \param[in]  pLlcpSocket        A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  psBuffer           The buffer receiving the data.
* \param[in]  pRecv_RspCb        The callback to be called when the
*                                operation is completed.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_PENDING                  Reception operation is in progress,
*                                            pRecv_RspCb will be called upon completion.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Recv(phlnLlcp_Fri_Transport_Socket_t       *pLlcpSocket,
                                                          phNfc_sData_t                         *psBuffer,
                                                          phlnLlcp_Fri_TransportSocketRecvCb_t   pRecv_RspCb,
                                                          void                                  *pContext)    /* PRQA S 3209 */
{
    phStatus_t  status = PH_ERR_SUCCESS;
    uint32_t    dataLengthStored    = 0;
    uint32_t    dataLengthAvailable = 0;
    uint32_t    dataLengthRead      = 0;
    uint32_t    dataLengthWrite     = 0;
    uint8_t     dataBufferized      = false;

    PH_UNUSED_VARIABLE(dataLengthWrite);

    /* Test if the WorkingBuffer Length is null */
    if(pLlcpSocket->bufferLinearLength == 0)
    {
        if (pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketConnected)
        {
            return PH_ERR_FAILED;
        }

        /* Test If data is present in the RW Buffer */
        if(pLlcpSocket->indexRwRead != pLlcpSocket->indexRwWrite)
        {
            if(pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].length != 0)
            {
                /* Save I_FRAME into the Receive Buffer */
                memcpy(psBuffer->buffer,pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].buffer,pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].length);  /* PRQA S 3200 */
                psBuffer->length = pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].length;

                dataBufferized = true;

                /* Update VR */
                pLlcpSocket->socket_VR = (pLlcpSocket->socket_VR+1)%16;

                /* Update RW Buffer length */
                pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].length = 0;

                /* Update Value Rw Read Index*/
                pLlcpSocket->indexRwRead++;
            }
        }

        if(dataBufferized == true)
        {
            /* Call the Receive CB */
            pRecv_RspCb(pContext,PH_ERR_SUCCESS);

            if(pLlcpSocket->ReceiverBusyCondition == true)
            {
                /* Reset the ReceiverBusyCondition Flag */
                pLlcpSocket->ReceiverBusyCondition = false;
                /* RR */
                /* TODO: report status? */
                status = phlnLlcp_Fri_Send_ReceiveReady_Frame(pLlcpSocket);
            }
        }
        else
        {
            /* Set Receive pending */
            pLlcpSocket->bSocketRecvPending = true;

            /* Store the buffer pointer */
            pLlcpSocket->sSocketRecvBuffer = psBuffer;

            /* Store the Recv CB and context */
            pLlcpSocket->pfSocketRecv_Cb  = pRecv_RspCb;
            pLlcpSocket->pRecvContext     = pContext;

            /* Set status */
            status = PH_ERR_PENDING;
        }
    }
    else
    {
        /* Test if data is present in the linear buffer*/
        dataLengthStored = phlnLlcp_Fri_Utils_CyclicFifoUsage(&pLlcpSocket->sCyclicFifoBuffer);

        if(dataLengthStored != 0)
        {
            if(psBuffer->length > dataLengthStored)
            {
                psBuffer->length = dataLengthStored;
            }

            /* Read data from the linear buffer */
            dataLengthRead = phlnLlcp_Fri_Utils_CyclicFifoFifoRead(&pLlcpSocket->sCyclicFifoBuffer,
                                                                psBuffer->buffer,
                                                                psBuffer->length);

            if(dataLengthRead != 0)
            {
                /* Test If data is present in the RW Buffer */
                while(pLlcpSocket->indexRwRead != pLlcpSocket->indexRwWrite)
                {
                    /* Get the data length available in the linear buffer  */
                    dataLengthAvailable = phlnLlcp_Fri_Utils_CyclicFifoAvailable(&pLlcpSocket->sCyclicFifoBuffer);

                    /* Exit if not enough memory available in linear buffer */
                    if(dataLengthAvailable < pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].length)
                    {
                        break;
                    }

                    /* Write data into the linear buffer */
                    dataLengthWrite = phlnLlcp_Fri_Utils_CyclicFifoWrite(&pLlcpSocket->sCyclicFifoBuffer,
                                                               pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].buffer,
                                                               pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].length);
                    /* Update VR */
                    pLlcpSocket->socket_VR = (pLlcpSocket->socket_VR+1)%16;

                    /* Set flag bufferized to true */
                    dataBufferized = true;

                    /* Update RW Buffer length */
                    pLlcpSocket->sSocketRwBufferTable[(pLlcpSocket->indexRwRead%pLlcpSocket->localRW)].length = 0;

                    /* Update Value Rw Read Index*/
                    pLlcpSocket->indexRwRead++;
                }

                /* Test if data has been bufferized after a read access */
                if(dataBufferized == true)
                {
                    /* Get the data length available in the linear buffer  */
                    dataLengthAvailable = phlnLlcp_Fri_Utils_CyclicFifoAvailable(&pLlcpSocket->sCyclicFifoBuffer);
                    if((dataLengthAvailable >= pLlcpSocket->sSocketOption.miu) && (pLlcpSocket->ReceiverBusyCondition == true))
                    {
                        /* Reset the ReceiverBusyCondition Flag */
                        pLlcpSocket->ReceiverBusyCondition = false;
                        /* RR */
                        /* TODO: report status? */
                        status = phlnLlcp_Fri_Send_ReceiveReady_Frame(pLlcpSocket);
                    }
                }

                /* Call the Receive CB */
                pRecv_RspCb(pContext,PH_ERR_SUCCESS);
            }
            else
            {
                /* Call the Receive CB   */
                status = PH_ERR_FAILED;
            }
        }
        else
        {
            if (pLlcpSocket->eSocket_State != phlnLlcp_Fri_TransportSocket_eSocketConnected)
            {
                status = PH_ERR_FAILED;
            }
            else
            {
                /* Set Receive pending */
                pLlcpSocket->bSocketRecvPending = true;

                /* Store the buffer pointer */
                pLlcpSocket->sSocketRecvBuffer = psBuffer;

                /* Store the Recv CB and context */
                pLlcpSocket->pfSocketRecv_Cb  = pRecv_RspCb;
                pLlcpSocket->pRecvContext     = pContext;

                /* Set status */
                status = PH_ERR_PENDING;
            }
        }
    }

    if(status != PH_ERR_PENDING &&
        status != PH_ERR_SUCCESS)
    {
        /* Note: The receive callback must be released to avoid being called at abort */
        pLlcpSocket->pfSocketRecv_Cb = NULL;
        pLlcpSocket->pRecvContext = NULL;
    }

    return status;
}

#endif /* NXPBUILD__PHLN_LLCP_FRI */
