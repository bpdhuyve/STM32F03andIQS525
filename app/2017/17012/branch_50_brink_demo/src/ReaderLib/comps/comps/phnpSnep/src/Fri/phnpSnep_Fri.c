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
* \file  phnpSnep_Fri_Snep.c                                                  *
* \brief SNEP protocol management public interfaces listed in this module.    *
*                                                                             *
*                                                                             *
* Project:                                                                    *
*                                                                             *
* $Date: 2013-06-17 13:51:16 +0530 (Mon, 17 Jun 2013) $                       *
* $Author: nxp62726 $                                                         *
* $Revision: 172 $                                                            *
* $Aliases:    $                                                              *
*                                                                             *
* =========================================================================== *
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PHNP_SNEP_FRI

#include <phOsal.h>
#include "phnpSnep.h"
#include "phnpSnep_Fri.h"
#include "phnpSnep_Fri_ContextMgmt.h"
#include "phnpSnep_Fri_Utils.h"

/* ----------------------- Macros -------------------------- */
#define PHNPSNEP_FRI_DEFAULT_SERVER_SAP           0x04
#define PHNPSNEP_FRI_NON_DEFAULT_SERVER_SAP       0x15
#define PHNPSNEP_FRI_DEFAULT_SERVER_NAME_LEN      0x0F


/* ----------------------- Globals --------------------------*/

uint8_t gphnpSnep_Fri_Snep_DefaultServerName[PHNPSNEP_FRI_DEFAULT_SERVER_NAME_LEN] = {'u','r','n',':','n','f','c',':','s','n',':','s','n','e','p'};

/* ----------------------- Internal functions headers -------------------------- */

static void phnpSnep_Fri_Llcp_SocketErr_Cb( void       *pContext,
                                           uint8_t     nErrCode);

static void phnpSnep_Fri_Llcp_AcceptSocketErr_Cb( void      *pContext,
                                                 uint8_t    nErrCode);

static void phnpSnep_Fri_Llcp_ListenSocket_Cb( void             *pContext,
                                              ph_NfcHandle      hIncomingSocket);

static void phnpSnep_Fri_Llcp_ConnectSocket_Cb( void           *pContext,
                                               uint8_t         nErrCode,
                                               phStatus_t      status);

static void phnpSnep_Fri_Llcp_AcceptSocket_Cb( void            *pContext,
                                              phStatus_t       status);

/* --------------------------- Internal functions ------------------------------ */

static void phnpSnep_Fri_Llcp_SocketErr_Cb( void       *pContext,
                                           uint8_t     nErrCode)
{
}

static void phnpSnep_Fri_Llcp_AcceptSocketErr_Cb( void     *pContext,
                                                 uint8_t    nErrCode)
{
    phStatus_t  status;
    phnpSnep_Fri_ServerConnection_t *pServerConn = (phnpSnep_Fri_ServerConnection_t*)pContext;

    if (NULL != (void *)pServerConn &&
        0 != pServerConn->hSnepServerConnHandle)
    {
        status = phnpSnep_Fri_ContextMgmt_RemoveOneServerConnection( pServerConn->pSnepDataParamsContext,
            pServerConn->hSnepServerConnHandle);

        if (PH_ERR_SUCCESS != status)
        {
#ifdef DEBUG
            printf("Server Connection Removed FAILED \n");  /* PRQA S 3200 */
#endif /* DEBUG */
        }
    }/* No Else */
}

static void phnpSnep_Fri_Llcp_ListenSocket_Cb( void          *pContext,
                                              ph_NfcHandle    hIncomingSocket)
{
    phnpSnep_Fri_ServerSession_t *pServerSession = (phnpSnep_Fri_ServerSession_t*)pContext;

    if (NULL != pServerSession)
    {
        if (phnpSnep_Fri_Server_Initialized == pServerSession->Server_state &&
            NULL != pServerSession->pConnectionCb)
        {
            pServerSession->pConnectionCb( pServerSession->pListenContext,
                                           hIncomingSocket,
                                           PHNPSNEP_FRI_INCOMING_CONNECTION
                                         );
        }/* No Else */
    }/* No Else */
}

static void phnpSnep_Fri_Llcp_ConnectSocket_Cb( void        *pContext,
                                               uint8_t       nErrCode,
                                               phStatus_t    status)
{
    phnpSnep_Fri_ClientSession_t *pClientSession = (phnpSnep_Fri_ClientSession_t*)pContext;
    phlnLlcp_Fri_Transport_sSocketOptions_t sRemoteOptions;
    phnpSnep_Fri_DataParams_t *pDataParams =  (phnpSnep_Fri_DataParams_t *) pClientSession->pSnepDataParamsContext;

    if (PH_ERR_SUCCESS == status)
    {
        status = PHNPSNEP_FRI_CONNECTION_FAILED;
        if (NULL != pClientSession)
        {
            if (phnpSnep_Fri_Client_Initialized == pClientSession->Client_state &&
                NULL != pClientSession->pConnectionCb)
            {
                status = PHNPSNEP_FRI_CONNECTION_SUCCESS;
            }/* No Else */
        }/* No Else */
    }
    else
    {
        status = PHNPSNEP_FRI_CONNECTION_FAILED;
#ifdef DEBUG
        printf("Socket PHNPSNEP_FRI_CONNECTION_FAILED\n");  /* PRQA S 3200 */
#endif /* DEBUG */
    }

    if (NULL != pClientSession &&
        NULL != pClientSession->pConnectionCb)
    {
        /* TODO enable the conditions to get Socket options; used in chunking buffer memcopy in function */
        if (PH_ERR_SUCCESS == phlnLlcp_Transport_SocketGetRemoteOptions( pDataParams->plnLlcpDataParams,
                                                                         (phlnLlcp_Fri_Transport_Socket_t *) pClientSession->hSnepClientHandle,
                                                                         &sRemoteOptions)
                                                                       )
        {
            if( sRemoteOptions.miu > PHLNLLCP_FRI_MIU_MAX )
            {
                /* For remote MIU is more than PHLNLLCP_FRI_MIU_MAX assign Default MIU */
                pClientSession->iRemoteMiu = PHNPSNEP_FRI_LLCP_MIU_DEFAULT;
            }
            else
            {
                pClientSession->iRemoteMiu = sRemoteOptions.miu;
            }
        }
        else
        {
            pClientSession->iRemoteMiu = PHNPSNEP_FRI_LLCP_MIU_DEFAULT;
        }

        pClientSession->pConnectionCb( pClientSession->pClientContext,
                                       pClientSession->hSnepClientHandle,
                                       status);
    }
}

static void phnpSnep_Fri_Llcp_AcceptSocket_Cb( void        *pContext,
                                              phStatus_t    status)
{
    phnpSnep_Fri_ServerConnection_t           *pServerConn = (phnpSnep_Fri_ServerConnection_t*) pContext;
    phnpSnep_Fri_ServerSession_t              *pServerSession = NULL;
    phlnLlcp_Fri_Transport_sSocketOptions_t    sRemoteOptions;
    phnpSnep_Fri_DataParams_t                 *pDataParams = (phnpSnep_Fri_DataParams_t *) pServerConn->pSnepDataParamsContext;

    if (NULL != pServerConn &&
        PH_ERR_SUCCESS == status)
    {
        pServerSession = (phnpSnep_Fri_ServerSession_t *)phnpSnep_Fri_ContextMgmt_GetServerSessionByConnection( pDataParams,
                                                                                                                pServerConn->hSnepServerConnHandle);

        if (NULL != pServerSession &&
            NULL != pServerSession->pConnectionCb)
        {
            status = PHNPSNEP_FRI_CONNECTION_SUCCESS;
        }
        else
        {
            status = PHNPSNEP_FRI_CONNECTION_FAILED;
        }
    }
    else
    {
        status = PHNPSNEP_FRI_CONNECTION_FAILED;
#ifdef DEBUG
        printf("AcceptSocket_Cb Failed:\n");     /* PRQA S 3200 */
#endif /* DEBUG */
    }

    if (NULL != pServerSession &&
        NULL != pServerSession->pConnectionCb)
    {
        if (PH_ERR_SUCCESS == phlnLlcp_Transport_SocketGetRemoteOptions( pDataParams->plnLlcpDataParams,
                                                                        (phlnLlcp_Fri_Transport_Socket_t *) pServerConn->hSnepServerConnHandle,
                                                                        &sRemoteOptions))
        {
            if( sRemoteOptions.miu > PHLNLLCP_FRI_MIU_MAX )
            {
                /* For remote MIU is more than PHLNLLCP_FRI_MIU_MAX assign Default MIU */
                pServerConn->iRemoteMiu = PHNPSNEP_FRI_LLCP_MIU_DEFAULT;
            }
            else
            {
                pServerConn->iRemoteMiu = sRemoteOptions.miu;
            }
        }
        else
        {
            pServerConn->iRemoteMiu = PHNPSNEP_FRI_LLCP_MIU_DEFAULT;
        }

        pServerConn->ServerConnectionState = phnpSnep_Fri_Server_Initialized;

        pServerSession->pConnectionCb( pServerConn->pConnectionContext,
                                       pServerConn->hSnepServerConnHandle,
                                       status);

        pServerConn->responseDataContext.pChunkingBuffer->length = pServerConn->iMiu;

        status = phlnLlcp_Transport_Recv( pDataParams->plnLlcpDataParams,
                                          (phlnLlcp_Fri_Transport_Socket_t *) pServerConn->hSnepServerConnHandle,
                                          pServerConn->responseDataContext.pChunkingBuffer,
                                         &phnpSnep_Fri_Utils_SocketRecvCbForServer,
                                          (void*) pServerConn);
    }
}

/* ---------------------------- Public functions ------------------------------- */

phStatus_t phnpSnep_Fri_Server_Init( phnpSnep_Fri_DataParams_t     *pDataParams,
                                    phnpSnep_Fri_Config_t          *pConfigInfo,
                                    pphnpSnep_Fri_ConnectCB_t       pConnCb,
                                    ph_NfcHandle                   *pServerHandle,
                                    phnpSnep_Fri_ServerSession_t   *pServer,
                                    void                           *pContext)
{

    phStatus_t                      status;
    phNfc_sData_t                   ServerName = { gphnpSnep_Fri_Snep_DefaultServerName, PHNPSNEP_FRI_DEFAULT_SERVER_NAME_LEN };
    phnpSnep_Fri_ServerSession_t   *pServerSession = NULL;
    phnpSnep_Fri_ServerContext_t   *pServerContext = NULL;

    PH_ASSERT_NULL(pServerSession);

    if (NULL == pConfigInfo ||
        NULL == pConnCb ||
        NULL == pServerHandle ||
        PHNPSNEP_FRI_MIN_MIU_VAL > pConfigInfo->sOptions.miu)
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if (NULL != pServerContext)
    {
        pServerSession = phnpSnep_Fri_ContextMgmt_AddServerSession( pDataParams,
                                                                    pServer );

        if (NULL != pServerSession)
        {
            if (NULL != pServerSession->pWorkingBuffer->buffer)
            {
                /*Create LLCP Socket*/
                status = phlnLlcp_Transport_Socket( pDataParams->plnLlcpDataParams,
                                                    phlnLlcp_Fri_Transport_eConnectionOriented,
                                                    &pConfigInfo->sOptions,
                                                    pServerSession->pWorkingBuffer,
                                                   (phlnLlcp_Fri_Transport_Socket_t **) &pServerSession->hSnepServerHandle,
                                                    &phnpSnep_Fri_Llcp_SocketErr_Cb,
                                                    (void *)pServerSession
                                                   );
            }
            else
            {
                status = PH_ERR_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            status = PH_ERR_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        status = PH_ERR_INSUFFICIENT_RESOURCES;
    }

    if (PH_ERR_SUCCESS == status)
    {
        if (phnpSnep_Fri_Server_Default == pConfigInfo->SnepServerType)
        {
            pServerSession->SnepServerSap = PHNPSNEP_FRI_DEFAULT_SERVER_SAP;
            /* Bind Socket */
            status = phlnLlcp_Transport_Bind( pDataParams->plnLlcpDataParams,
                                              (phlnLlcp_Fri_Transport_Socket_t *) pServerSession->hSnepServerHandle,
                                              pServerSession->SnepServerSap,
                                              &ServerName
                                            );
        }
        else
        {
            /*@TODO Server sap should be dynamically selected for non default server
            * Currently there is a limitation of one default and one non default server */
            pServerSession->SnepServerSap = PHNPSNEP_FRI_NON_DEFAULT_SERVER_SAP;
            /* Bind Socket */
            status = phlnLlcp_Transport_Bind( pDataParams->plnLlcpDataParams,
                                              (phlnLlcp_Fri_Transport_Socket_t *) pServerSession->hSnepServerHandle,
                                              pServerSession->SnepServerSap,
                                              pConfigInfo->SnepServerName
                                            );
        }
    }

    if (PH_ERR_SUCCESS == status)
    {
        if ( phnpSnep_Fri_Server_Default == pConfigInfo->SnepServerType &&
            NULL == pConfigInfo->SnepServerName)
        {
            status = phlnLlcp_Transport_Listen( pDataParams->plnLlcpDataParams,
                                                (phlnLlcp_Fri_Transport_Socket_t *) pServerSession->hSnepServerHandle,
                                               (phlnLlcp_Fri_TransportSocketListenCb_t) &phnpSnep_Fri_Llcp_ListenSocket_Cb,
                                                (void*)pServerSession
                                              );

            pServerSession->SnepServerType = phnpSnep_Fri_Server_Default;
        }
        else if ( phnpSnep_Fri_Server_NonDefault == pConfigInfo->SnepServerType &&
            NULL != pConfigInfo->SnepServerName)
        {
            status = phlnLlcp_Transport_Listen( pDataParams->plnLlcpDataParams,
                                                (phlnLlcp_Fri_Transport_Socket_t *) pServerSession->hSnepServerHandle,
                                               (phlnLlcp_Fri_TransportSocketListenCb_t) &phnpSnep_Fri_Llcp_ListenSocket_Cb,
                                                (void*)pServerSession
                                               );
            pServerSession->SnepServerType = phnpSnep_Fri_Server_NonDefault;
        }
        else
        {
            status = PH_ERR_INVALID_PARAMETER;
        }

        if (PH_ERR_SUCCESS == status)
        {
            pServerSession->pConnectionCb = pConnCb;
            pServerSession->Server_state = phnpSnep_Fri_Server_Initialized;
            *pServerHandle = pServerSession->hSnepServerHandle;
            pServerSession->pListenContext = pContext;
            pServerSession->SnepServerVersion =PHNPSNEP_FRI_VERSION;
        }
    }
    else
    {
        /*@TODO Deallocate the Server Session and Server Context */
    }

    return status;
}


phStatus_t phnpSnep_Fri_Server_Accept( phnpSnep_Fri_DataParams_t                *pDataParams,
                                      phNfc_sData_t                             *pDataInbox,
                                      phlnLlcp_Fri_Transport_sSocketOptions_t   *pSockOps,
                                      ph_NfcHandle                               ServerHandle,
                                      ph_NfcHandle                               ConnHandle,
                                      pphnpSnep_Fri_Put_ntf_t                    pPutNtfCb,
                                      pphnpSnep_Fri_Get_ntf_t                    pGetNtfCb,
                                      void                                      *pContext )
{

    phStatus_t    status;
    phnpSnep_Fri_ServerConnection_t   *pServerConn = NULL;
    phnpSnep_Fri_ServerSession_t      *pServerSession = NULL;

    if (NULL == pDataInbox ||
        NULL == pSockOps ||
        PHNPSNEP_FRI_MIN_MIU_VAL > pSockOps->miu)
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    pServerConn = (phnpSnep_Fri_ServerConnection_t *) phnpSnep_Fri_ContextMgmt_AddServerConnection( pDataParams,
                                                                                                    ServerHandle);

    pServerSession = phnpSnep_Fri_ContextMgmt_GetServerSessionContext( pDataParams,
                                                                       ServerHandle);

    if (NULL != pServerConn)
    {
        if( pDataInbox->buffer != NULL )
        {
            pServerConn->iInboxSize = pDataInbox->length;
            if( phnpSnep_Fri_Server_Default == pServerSession->SnepServerType
                &&  PHNPSNEP_FRI_MIN_INBOX_SIZE > pServerConn->iInboxSize)
            {
                return PH_ERR_INVALID_PARAMETER;
            }
        }
        else
        {
            return PH_ERR_INVALID_PARAMETER;
        }
        pServerConn->hSnepServerConnHandle = ConnHandle;
        pServerConn->pDataInbox = pDataInbox;
        pServerConn->pPutNtfCb = pPutNtfCb;
        pServerConn->pGetNtfCb = pGetNtfCb;
        pServerConn->pConnectionContext = pContext;

        if( pDataInbox != NULL )
        {
            pServerConn->iInboxSize = pDataInbox->length;
        }

        pServerConn->iMiu = pSockOps->miu;
        pServerConn->ServerConnectionState = phnpSnep_Fri_Server_Uninitialized;
        pServerConn->SnepServerVersion = PHNPSNEP_FRI_VERSION;

        if (NULL != pServerConn->responseDataContext.pChunkingBuffer)
        {
            if (NULL != pServerConn->responseDataContext.pChunkingBuffer->buffer)
            {
                pServerConn->responseDataContext.pChunkingBuffer->length = pSockOps->miu;
            }
            else
            {
                return PH_ERR_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            return PH_ERR_INSUFFICIENT_RESOURCES;
        }

        if (NULL != pServerConn->pConnWorkingBuffer->buffer)
        {
            status = phlnLlcp_Transport_Accept( pDataParams->plnLlcpDataParams,
                                               (phlnLlcp_Fri_Transport_Socket_t *)ConnHandle,
                                               pSockOps,
                                                pServerConn->pConnWorkingBuffer,
                                               &phnpSnep_Fri_Llcp_AcceptSocketErr_Cb,
                                               &phnpSnep_Fri_Llcp_AcceptSocket_Cb,
                                               (void*)pServerConn);
        }
        else
        {
            status = PH_ERR_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        status = PH_ERR_INSUFFICIENT_RESOURCES;
    }

    return status;
}

phStatus_t phnpSnep_Fri_Server_DeInit( phnpSnep_Fri_DataParams_t    *pDataParams,
                                       ph_NfcHandle                  ServerHandle )
{

    phStatus_t    status = PH_ERR_SUCCESS;
    phnpSnep_Fri_ServerContext_t    *pServerContext = NULL;

    /* Context should not be allocated if already NULL */
    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if (NULL != pServerContext)
    {
        status=phnpSnep_Fri_ContextMgmt_RemoveServerSession( pDataParams,
                                                             ServerHandle );
    }
    else
    {
        status = PH_ERR_NOT_INITIALISED;
    }
    return status;
}


/*----------------------- SNEP CLIENT ---------------------------------*/

phStatus_t phnpSnep_Fri_Client_Init( phnpSnep_Fri_DataParams_t     *pDataParams,
                                    phnpSnep_Fri_Config_t          *pConfigInfo,
                                    ph_NfcHandle                    hRemDevHandle,
                                    pphnpSnep_Fri_ConnectCB_t       pConnClientCb,
                                    phnpSnep_Fri_ClientSession_t   *pClient,
                                    void                           *pContext)

{

    phStatus_t status;
    phStatus_t temp = PH_ERR_SUCCESS;
    PH_UNUSED_VARIABLE(temp);

    phNfc_sData_t                  ServerName = { gphnpSnep_Fri_Snep_DefaultServerName, PHNPSNEP_FRI_DEFAULT_SERVER_NAME_LEN};
    phnpSnep_Fri_ClientSession_t  *pClientSession = NULL;
    phnpSnep_Fri_ClientContext_t  *pClientContext = NULL;

    if (NULL == pConfigInfo ||
        NULL == pConnClientCb ||
        PHNPSNEP_FRI_MIN_MIU_VAL > pConfigInfo->sOptions.miu)
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    pClientContext = phnpSnep_Fri_ContextMgmt_GetClientContext( pDataParams );

    if (NULL != pClientContext)
    {
        pClientSession = phnpSnep_Fri_ContextMgmt_AddClientSession( pDataParams,
                                                                    pClient);
        if (NULL != pClientSession)
        {
            if (NULL != pClientSession->sWorkingBuffer->buffer)
            {
               /*Create Socket LLCP Socket*/
               status = phlnLlcp_Transport_Socket( pDataParams->plnLlcpDataParams,
                                                   phlnLlcp_Fri_Transport_eConnectionOriented,
                                                   &pConfigInfo->sOptions,
                                                   pClientSession->sWorkingBuffer,
                                                   (phlnLlcp_Fri_Transport_Socket_t **) &pClientSession->hSnepClientHandle,
                                                   &phnpSnep_Fri_Llcp_SocketErr_Cb,
                                                   (void *) pClientSession
                                                  );
            }
            else
            {
                status = PH_ERR_INSUFFICIENT_RESOURCES;
                status = phnpSnep_Fri_ContextMgmt_RemoveIncompleteClientSession( pDataParams, pClientSession);
            }
        }
        else
        {
            status = PH_ERR_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        status = PH_ERR_INSUFFICIENT_RESOURCES;
    }

    if (PH_ERR_SUCCESS == status)
    {
        pClientSession->Client_state = phnpSnep_Fri_Client_Initialized;
        pClientSession->pConnectionCb = pConnClientCb;
        pClientSession->pClientContext = pContext;
        pClientSession->iMiu = pConfigInfo->sOptions.miu;
        pClientSession->SnepClientVersion = PHNPSNEP_FRI_VERSION;
        pClientSession->hRemoteDevHandle = hRemDevHandle;
        pClientSession->pSnepDataParamsContext = pDataParams;

        if (phnpSnep_Fri_Server_Default == pConfigInfo->SnepServerType && NULL == pConfigInfo->SnepServerName)
        {
           status = phlnLlcp_Transport_ConnectByUri( pDataParams->plnLlcpDataParams,
                                                     (phlnLlcp_Fri_Transport_Socket_t *) pClientSession->hSnepClientHandle,
                                                    &ServerName,
                                                    &phnpSnep_Fri_Llcp_ConnectSocket_Cb,
                                                    (void *) pClientSession
                                                  );

        }
        else if (phnpSnep_Fri_Server_NonDefault == pConfigInfo->SnepServerType && NULL != pConfigInfo->SnepServerName)
        {
            status = phlnLlcp_Transport_ConnectByUri( pDataParams->plnLlcpDataParams,
                                                      (phlnLlcp_Fri_Transport_Socket_t *) pClientSession->hSnepClientHandle,
                                                      pConfigInfo->SnepServerName,
                                                     &phnpSnep_Fri_Llcp_ConnectSocket_Cb,
                                                     (void *) pClientSession
                                                    );
        }
        else
        {
            /* No Else */
        }

        if (PH_ERR_PENDING != status && PH_ERR_SUCCESS != status)
        {
            temp = phnpSnep_Fri_ContextMgmt_RemoveIncompleteClientSession( pDataParams,
                                                                           pClientSession);
        }
    }
    else
    {
        /* Deallocate the Server Session and Server Context */
        status = phnpSnep_Fri_ContextMgmt_RemoveIncompleteClientSession( pDataParams, pClientSession);
    }

    return status;
}

phStatus_t phnpSnep_Fri_Client_DeInit( phnpSnep_Fri_DataParams_t    *pDataParams,
                                      ph_NfcHandle                  ClientHandle)
{
    phStatus_t status;
    phnpSnep_Fri_ClientContext_t    *pClientContext = NULL;

    /* Context should not be allocated if already NULL */
    pClientContext = phnpSnep_Fri_ContextMgmt_GetClientContext(pDataParams);

    if (NULL != pClientContext)
    {
        if (0 != ClientHandle)
        {
            if (PH_ERR_SUCCESS == phnpSnep_Fri_ContextMgmt_RemoveClientSession( pDataParams,
                                                                                ClientHandle))
            {
                status = PH_ERR_SUCCESS;
            }
            else
            {
                status = PH_ERR_FAILED;
            }
        }
        else
        {
            status = phnpSnep_Fri_ContextMgmt_RemoveAllClientSessions( pDataParams );
        }
    }
    else
    {
        status = PH_ERR_NOT_INITIALISED;
    }
    return status;
}

phStatus_t phnpSnep_Fri_Init( phnpSnep_Fri_DataParams_t     *pDataParams,
                              uint16_t                       wSizeOfDataParams,
                              void                          *plnLlcpDataParams
                            )
{
    phlnLlcp_Fri_DataParams_t  *lnLlcpDataparams = (phlnLlcp_Fri_DataParams_t *) plnLlcpDataParams;

    if (sizeof(phnpSnep_Fri_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
    }

    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (plnLlcpDataParams);

    /* Init. private data */
    pDataParams->wId = PH_COMP_NP_SNEP | PHNP_SNEP_FRI_ID;
    pDataParams->plnLlcpDataParams = plnLlcpDataParams;
    pDataParams->pOsal = lnLlcpDataparams->pLlcp->osal;

    return PH_ERR_SUCCESS;
}

phStatus_t phnpSnep_Fri_ClientReqPut( phnpSnep_Fri_DataParams_t   *pDataParams,
                                     ph_NfcHandle                  ConnHandle,
                                     phNfc_sData_t                *pPutData,
                                     pphnpSnep_Fri_ReqCb_t         fCbPut,
                                     void                         *cbContext
                                     )
{

    phStatus_t status = PH_ERR_SUCCESS;
    pphnpSnep_Fri_ClientSession_t pClientSessionContext = NULL;

    if (NULL == pPutData ||
        NULL == pPutData->buffer ||
        0 == pPutData->length ||
        NULL == pDataParams ||
        NULL == ConnHandle )
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    pClientSessionContext = phnpSnep_Fri_ContextMgmt_GetClientSession( pDataParams,
                                                                       ConnHandle );
    if (NULL != pClientSessionContext)
    {
        phnpSnep_Fri_Utils_ResetCliDataContext(pClientSessionContext);

        status = phnpSnep_Fri_Utils_PrepareSnepPacket( pDataParams,
                                                       phnpSnep_Fri_Put,
                                                       pPutData,
                                                       pClientSessionContext->putGetDataContext.pSnepPacket,
                                                       pClientSessionContext->putGetDataContext.pSnepHeader,
                                                       pClientSessionContext->SnepClientVersion,
                                                       pClientSessionContext->acceptableLength);

        if (pClientSessionContext->putGetDataContext.pSnepPacket != NULL )
        {
             /*this transfer needs chunking, so lets allocate chunking buffer */

            if(NULL == pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer)
            {
                 status = PH_ERR_INSUFFICIENT_RESOURCES;
            }
            else
            {
                 pClientSessionContext->putGetDataContext.pChunkingBuffer->length = (pClientSessionContext->iRemoteMiu > pClientSessionContext->iMiu)
                                       ? pClientSessionContext->iRemoteMiu : pClientSessionContext->iMiu;
            }

            if (PH_ERR_SUCCESS == status)
            {
                if (pClientSessionContext->putGetDataContext.pSnepPacket->length > pClientSessionContext->iRemoteMiu)
                {
                    pClientSessionContext->putGetDataContext.bWaitForContinue = true;
                    pClientSessionContext->putGetDataContext.bContinueReceived = false;
                }
            }
            pClientSessionContext->pReqCb = fCbPut;
            pClientSessionContext->pReqCbContext = cbContext;
        }
    }
    else
    {
        status = PH_ERR_FAILED;
        return status;
    }

    if (PH_ERR_SUCCESS == status && NULL != pClientSessionContext)
    {
        status = phnpSnep_Fri_Utils_Req( pDataParams,
                                         pClientSessionContext);
        /* if sending of request is successful then get the reply and analyze it to report appropriate NFC Status message and data to upper layer */
    }
    else
    {
        if (pClientSessionContext->putGetDataContext.pChunkingBuffer != NULL)
        {
            pClientSessionContext->putGetDataContext.pChunkingBuffer->length = 0;
        }
        /* ELSE */
    }

    return status;
}

phStatus_t phnpSnep_Fri_ClientReqGet( phnpSnep_Fri_DataParams_t   *pDataParams,
                                     ph_NfcHandle                  ConnHandle,
                                     phNfc_sData_t                *pGetData,
                                     uint32_t                      Acceptable_length,
                                     pphnpSnep_Fri_ReqCb_t         fCbGet,
                                     void                         *cbContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    pphnpSnep_Fri_ClientSession_t pClientSessionContext = NULL;

    if ( NULL == pGetData ||
         NULL == pGetData->buffer ||
         0 == pGetData->length ||
         NULL == pDataParams ||
         NULL == ConnHandle )
        {
            return PH_ERR_INVALID_PARAMETER;
        }

    pClientSessionContext = phnpSnep_Fri_ContextMgmt_GetClientSession( pDataParams,
                                                                       ConnHandle);

    if (NULL != pClientSessionContext)
    {
        phnpSnep_Fri_Utils_ResetCliDataContext(pClientSessionContext);

        /* TODO Temporary Fix */
        pClientSessionContext->acceptableLength = Acceptable_length;

        status = phnpSnep_Fri_Utils_PrepareSnepPacket( pDataParams,
                                                       phnpSnep_Fri_Get,
                                                       pGetData,
                                                       pClientSessionContext->putGetDataContext.pSnepPacket,
                                                       pClientSessionContext->putGetDataContext.pSnepHeader,
                                                       pClientSessionContext->SnepClientVersion,
                                                       pClientSessionContext->acceptableLength);

        if (pClientSessionContext->putGetDataContext.pSnepPacket != NULL)
        {
            /* this transfer needs chunking, so lets allocate chunking buffer */

            if(!pClientSessionContext->putGetDataContext.pChunkingBuffer->buffer)
            {
                status = PH_ERR_INSUFFICIENT_RESOURCES;
            }
            else
            {
                pClientSessionContext->putGetDataContext.pChunkingBuffer->length = (pClientSessionContext->iRemoteMiu > pClientSessionContext->iMiu)
                    ? pClientSessionContext->iRemoteMiu : pClientSessionContext->iMiu;
            }

            if (pClientSessionContext->putGetDataContext.pSnepPacket->length > pClientSessionContext->iRemoteMiu)
            {
                if (PH_ERR_SUCCESS == status)
                {
                    pClientSessionContext->putGetDataContext.bWaitForContinue = true;
                    pClientSessionContext->putGetDataContext.bContinueReceived = false;

                }
            }
            pClientSessionContext->pReqCb = fCbGet;
            pClientSessionContext->pReqCbContext = cbContext;
        }
    }
    else
    {
        status = PH_ERR_FAILED;
        return status;
    }

    if (PH_ERR_SUCCESS == status)
    {
        status = phnpSnep_Fri_Utils_Req( pDataParams,
                                         pClientSessionContext);
        /* if sending of request is successful then get the reply and analyze it to report
        appropriate NFC Status message and data to upper layer */
    }
    else
    {
        if (pClientSessionContext->putGetDataContext.pChunkingBuffer != NULL)
        {
            pClientSessionContext->putGetDataContext.pChunkingBuffer->length = 0;
        }
    }

    return status;
}

phStatus_t phnpSnep_Fri_ServerSendResponse( phnpSnep_Fri_DataParams_t           *pDataParams,
                                           ph_NfcHandle                          ConnHandle,
                                           phNfc_sData_t                        *pResponseData,
                                           phStatus_t                            responseStatus,
                                           pphnpSnep_Fri_Protocol_SendRspCb_t    fSendCompleteCb,
                                           void                                 *cbContext)
{

    phStatus_t    status = PH_ERR_SUCCESS;
    pphnpSnep_Fri_ServerConnection_t   pServerSessionContext = NULL;

    pServerSessionContext = phnpSnep_Fri_ContextMgmt_GetServerConnectionContext( pDataParams,
                                                                                  ConnHandle);

    if (NULL != pServerSessionContext)
    {
        if (PHNPSNEP_FRI_REQUEST_GET == pServerSessionContext->responseDataContext.pChunkingBuffer->buffer[PHNPSNEP_FRI_VERSION_LENGTH])
        {
            if (PH_ERR_SUCCESS == responseStatus)
            {
                if (NULL != pResponseData && NULL != pResponseData->buffer
                        &&  0 != pResponseData->length)
                {
                    if (pResponseData->length > pServerSessionContext->responseDataContext.iAcceptableLength)
                    {
                        /* fail the request with excess data error */
                        pServerSessionContext->responseDataContext.bIsExcessData = true;
                        if (NULL != pServerSessionContext->responseDataContext.pSnepPacket &&
                            NULL != pServerSessionContext->responseDataContext.pSnepPacket->buffer)
                        {
                            pServerSessionContext->responseDataContext.pSnepPacket->length = 0;
                        }
                        status = phnpSnep_Fri_Utils_PrepareSnepPacket( pDataParams,
                                                                       phnpSnep_Fri_Utils_GetPacketType(PHNPSNEP_FRI_STATUS_RESPONSE_EXCESS_DATA),
                                                                       NULL,
                                                                       pServerSessionContext->responseDataContext.pSnepPacket,
                                                                       pServerSessionContext->responseDataContext.pSnepHeader,
                                                                       pServerSessionContext->SnepServerVersion,
                                                                       0); /* acceptable length is not used for server packets */
                    }
                    else
                    {
                        status = phnpSnep_Fri_Utils_PrepareSnepPacket( pDataParams,
                                                                       phnpSnep_Fri_Utils_GetPacketType(responseStatus),
                                                                       pResponseData,
                                                                       pServerSessionContext->responseDataContext.pSnepPacket,
                                                                       pServerSessionContext->responseDataContext.pSnepHeader,
                                                                       pServerSessionContext->SnepServerVersion,
                                                                       0);
                    }
                }
                else
                {
                    return PH_ERR_INVALID_PARAMETER;
                }
            }
            else
            {
                status = phnpSnep_Fri_Utils_PrepareSnepPacket( pDataParams,
                                                               phnpSnep_Fri_Utils_GetPacketType(responseStatus),
                                                               NULL,
                                                               pServerSessionContext->responseDataContext.pSnepPacket,
                                                               pServerSessionContext->responseDataContext.pSnepHeader,
                                                               pServerSessionContext->SnepServerVersion,
                                                               0); /* acceptable length is not used for server packets */
            }
            pServerSessionContext->iDataTobeReceived = 0;
        }
        else
        {
            /* reset Context (Dont do memset to zero as it would erase pointers in data context */
            if (PHNPSNEP_FRI_STATUS_RESPONSE_CONTINUE != responseStatus)
            {
                phnpSnep_Fri_Utils_ResetServerConnectionContext( pServerSessionContext );
            }/* No Else */

            /* Send Continue response to the Client*/
            status = phnpSnep_Fri_Utils_PrepareSnepPacket( pDataParams,
                                                           phnpSnep_Fri_Utils_GetPacketType(responseStatus),
                                                           pResponseData,
                                                           pServerSessionContext->responseDataContext.pSnepPacket,
                                                           pServerSessionContext->responseDataContext.pSnepHeader,
                                                           pServerSessionContext->SnepServerVersion,
                                                           0); /* acceptable length is not used for server packets */
        }

        if (pServerSessionContext->responseDataContext.pSnepPacket)
        {
            if (pServerSessionContext->responseDataContext.pSnepPacket->length > pServerSessionContext->iRemoteMiu)
            {
                if (pServerSessionContext->responseDataContext.pChunkingBuffer)
                {
                    if(!pServerSessionContext->responseDataContext.pChunkingBuffer->buffer)
                    {
                        status = PH_ERR_INSUFFICIENT_RESOURCES;
                    }
                    else
                    {
                        pServerSessionContext->responseDataContext.pChunkingBuffer->length =  pServerSessionContext->iRemoteMiu;
                    }
                }
                else
                {
                    status = PH_ERR_INSUFFICIENT_RESOURCES;
                }

                if (PH_ERR_SUCCESS == status)
                {
                    pServerSessionContext->responseDataContext.bWaitForContinue = true;
                    pServerSessionContext->responseDataContext.bContinueReceived = false;
                }
            }
            pServerSessionContext->responseDataContext.fSendCompleteCb = fSendCompleteCb;
            pServerSessionContext->responseDataContext.cbContext = cbContext;
        }
        else
        {
            status = PH_ERR_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        return  status = PH_ERR_FAILED;
    }

    if ( PH_ERR_SUCCESS == status)
    {
        status = phnpSnep_Fri_Utils_SendResponse( pDataParams,
                                                  pServerSessionContext);
    }

    if (PH_ERR_PENDING != status)
    {
        if (pServerSessionContext->responseDataContext.pChunkingBuffer != NULL)
        {
            pServerSessionContext->responseDataContext.pChunkingBuffer->length = 0;
        }
    }

    return  status;
}

#endif /* NXPBUILD__PHNP_SNEP_FRI */

