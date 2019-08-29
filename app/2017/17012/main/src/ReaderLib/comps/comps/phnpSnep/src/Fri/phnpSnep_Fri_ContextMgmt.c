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
* \file  phnpSnep_Fri_ContextMgmt.c                                           *
* \brief SNEP protocol management public interfaces listed in this module.    *
*                                                                             *
*                                                                             *
* Project:                                                                    *
*                                                                             *
* $Date: 2014-02-13 12:24:52 +0530 (Thu, 13 Feb 2014) $                       *
* $Author: nxp62726 $                                                         *
* $Revision: 501 $                                                            *
* $Aliases:  $                                                                *
*                                                                             *
* =========================================================================== *
*/
#include <ph_Status.h>

#ifdef NXPBUILD__PHNP_SNEP_FRI

#include"phnpSnep_Fri_ContextMgmt.h"

/* --------------------------- Public Functions ------------------------------ */

phStatus_t phnpSnep_Fri_ContextMgmt_InitServerContext( phnpSnep_Fri_DataParams_t  *pDataParams )
{
    phStatus_t   status = PH_ERR_SUCCESS;

    return status;
}

/* Returns the Server Context*/
phnpSnep_Fri_ServerContext_t *phnpSnep_Fri_ContextMgmt_GetServerContext( phnpSnep_Fri_DataParams_t   *pDataParams )
{
    return &(pDataParams->gpServerContext);
}

/* Removes a Server Context if server count is '0'
* Should be called in every Server De-init.
* Returns FALSE if still active server sessions are present */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveServerContext( phnpSnep_Fri_DataParams_t  *pDataParams)
{
    phStatus_t status = PH_ERR_SUCCESS;

    if (0 == pDataParams->gpServerContext.CurrentServerCnt)
    {
        status = PH_ERR_SUCCESS;
    }
    return status;
}

/* Retrieves a Server Session matching the given Handle */
phnpSnep_Fri_ServerSession_t *phnpSnep_Fri_ContextMgmt_GetServerSessionContext( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                                               ph_NfcHandle                  ServerHandle)
{
    uint8_t   count;
    phnpSnep_Fri_ServerSession_t   *pSession = NULL;
    phnpSnep_Fri_ServerContext_t   *pServerContext = NULL;

    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if ((NULL != pServerContext) && (0 != pServerContext->CurrentServerCnt))
    {
        for (count = 0; count < PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT; count++)
        {
            if (NULL != pServerContext->pServerSession[count])
            {
                if (ServerHandle == pServerContext->pServerSession[count]->hSnepServerHandle)
                {
                    pSession = pServerContext->pServerSession[count];
                    break;
                } /* No Else */
            } /* No Else */
        }
    }/* No Else */

    return pSession;
}

/* Retrieves a Server Connection matching the given Handle */
phnpSnep_Fri_ServerConnection_t *phnpSnep_Fri_ContextMgmt_GetServerConnectionContext( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                                                     ph_NfcHandle                 ConnHandle)
{
    uint8_t count_srv=0, count_con=0;
    phnpSnep_Fri_ServerConnection_t   *pConnection = NULL;
    phnpSnep_Fri_ServerSession_t      *pServerSession = NULL;
    phnpSnep_Fri_ServerContext_t      *pServerContext = NULL;

    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if ((NULL != pServerContext) && (0 != pServerContext->CurrentServerCnt))
    {
        for (count_srv = 0; count_srv < PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT; count_srv++)
        {
            pServerSession = pServerContext->pServerSession[count_srv];

            if ((NULL != pServerSession) && (0 != pServerSession->CurrentConnCnt))
            {
                for (count_con = 0; count_con < PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN; count_con++)
                {
                    if (NULL != pServerSession->pServerConnection[count_con])
                    {
                        if (ConnHandle == pServerSession->pServerConnection[count_con]->hSnepServerConnHandle)
                        {
                            pConnection = pServerSession->pServerConnection[count_con];
                            break;
                        } /* No Else */
                    } /* No Else */
                }
            } /* No Else */

            if (NULL != pConnection)
            {
                break;
            } /* No Else */
        }
    } /* No Else */

    return pConnection;
}

/* Adds a new Server to the Server Context If returns NULL add session failed */
phnpSnep_Fri_ServerSession_t *phnpSnep_Fri_ContextMgmt_AddServerSession( phnpSnep_Fri_DataParams_t      *pDataParams,
                                                                        phnpSnep_Fri_ServerSession_t    *pServerSession )
{
    uint8_t        count =0;
    phnpSnep_Fri_ServerContext_t     *pServerContext = NULL;

    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if ((NULL != pServerContext) && (PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT > pServerContext->CurrentServerCnt))
    {
        for (count = 0;count < PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT;count++)
        {
            if (NULL != pServerSession)
            {
                pServerContext->pServerSession[count] = pServerSession;
                pServerContext->CurrentServerCnt++;
                break;
            }/* No Else */
        }
    }/* No Else */
    return pServerContext->pServerSession[count];
}

/* Adds a new Connection to the Server Session Context */
phnpSnep_Fri_ServerConnection_t *phnpSnep_Fri_ContextMgmt_AddServerConnection( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                                              ph_NfcHandle                  ServerHandle)
{
    uint8_t count=0;

    phnpSnep_Fri_ServerConnection_t    *pConnection = NULL;
    phnpSnep_Fri_ServerSession_t       *pServerConnContext = NULL;

    pServerConnContext = phnpSnep_Fri_ContextMgmt_GetServerSessionContext( pDataParams,
                                                                            ServerHandle);

    if ((NULL != pServerConnContext ) && ( PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN > pServerConnContext->CurrentConnCnt))
    {
        for (count = 0;count < PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN;count++)
        {
            if (NULL != pServerConnContext->pServerConnection[count])
            {
                pServerConnContext->CurrentConnCnt++;
                pConnection = pServerConnContext->pServerConnection[count];
                pConnection->pSnepDataParamsContext = pDataParams; /* To access the pDataParams in the callback function */
                break;
            }/* No Else */
        }
    }/* No Else */
    return pConnection;
}

/* Removes a Server from the Server Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveServerSession( phnpSnep_Fri_DataParams_t    *pDataParams,
                                                        ph_NfcHandle                   ServerHandle)
{

    phStatus_t   status = PH_ERR_FAILED;
    uint8_t      count=0;
    phnpSnep_Fri_ServerContext_t   *pServerContext = NULL;

    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if (NULL != pServerContext)
    {
        for (count = 0; count < PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT; count++)
        {
            if (NULL != pServerContext->pServerSession[count])
            {
                if (ServerHandle == pServerContext->pServerSession[count]->hSnepServerHandle)
                {
                    if (0 != pServerContext->pServerSession[count]->CurrentConnCnt)
                    {
                        status = phnpSnep_Fri_ContextMgmt_RemoveAllServerConnections( pDataParams,
                                                                                       ServerHandle);
                    }
                    else
                    {
                        status = PH_ERR_SUCCESS;
                    }
                    status = PH_ERR_SUCCESS;
                    pServerContext->CurrentServerCnt--;

                    break;
                } /* No Else */
            } /* No Else */
        }
    }/* No Else */
    return status;
}

/* Removes a Connection from the Server Session Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveOneServerConnection( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                              ph_NfcHandle                  ConnHandle)
{

    phStatus_t  status = PH_ERR_NOT_REGISTERED;
    uint8_t     count_srv=0, count_con=0;
    phnpSnep_Fri_ServerConnection_t     *pConnection = NULL;
    phnpSnep_Fri_ServerSession_t        *pServerSession = NULL;
    phnpSnep_Fri_ServerContext_t        *pServerContext = NULL;

    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if ((NULL != pServerContext) && (0 != pServerContext->CurrentServerCnt))
    {
        for (count_srv = 0; count_srv < PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT; count_srv++)
        {
            pServerSession = pServerContext->pServerSession[count_srv];
            if ((NULL != pServerSession) &&
                (0 != pServerSession->CurrentConnCnt))
            {
                for (count_con = 0; count_con < PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN; count_con++)
                {
                    if (NULL != pServerSession->pServerConnection[count_con])
                    {
                        if (ConnHandle == pServerSession->pServerConnection[count_con]->hSnepServerConnHandle)
                        {
                            status = PH_ERR_SUCCESS;
                            pServerSession->CurrentConnCnt--;
                            break;
                        } /* No Else */
                    } /* No Else */
                }
            } /* No Else */
            if (NULL != pConnection)
            {
                break;
            } /* No Else */
        }
    }
    else
    {
        status = PH_ERR_INVALID_PARAMETER;
    }

    return status;
}

/* Removes all Connections from a Server Session Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveAllServerConnections( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                               ph_NfcHandle                  ServerHandle)
{

    phStatus_t status = PH_ERR_NOT_REGISTERED;
    uint8_t    count=0;
    phnpSnep_Fri_ServerSession_t     *pServerSession = NULL;

    pServerSession = phnpSnep_Fri_ContextMgmt_GetServerSessionContext( pDataParams,
                                                                       ServerHandle );
    if ((NULL != pServerSession) &&
        (0 != pServerSession->CurrentConnCnt))
    {
        for (count = 0;count < PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN;count++)
        {
            if (NULL != pServerSession->pServerConnection[count])
            {
                status = PH_ERR_SUCCESS;
                pServerSession->CurrentConnCnt--;

            }/* No Else */
        }
    }
    else
    {
        status = PH_ERR_INVALID_PARAMETER;
    }
    return status;
}


/* Retreives a Server Session matching the given Handle */
phnpSnep_Fri_ServerSession_t *phnpSnep_Fri_ContextMgmt_GetServerSessionByConnection( phnpSnep_Fri_DataParams_t  *pDataParams,
                                                                                    ph_NfcHandle                 ConnHandle)
{
    uint8_t count_srv=0, count_con=0;
    phnpSnep_Fri_ServerSession_t   *pServerSession = NULL;
    phnpSnep_Fri_ServerContext_t   *pServerContext = NULL;

    pServerContext = phnpSnep_Fri_ContextMgmt_GetServerContext( pDataParams );

    if ((NULL != pServerContext) && (0 != pServerContext->CurrentServerCnt))
    {
        for (count_srv = 0; count_srv < PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT; count_srv++)
        {
            pServerSession = pServerContext->pServerSession[count_srv];

            if ((NULL != pServerSession) && (0 != pServerSession->CurrentConnCnt))
            {
                for (count_con = 0; count_con < PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN; count_con++)
                {
                    if (NULL != pServerSession->pServerConnection[count_con])
                    {
                        if (ConnHandle == pServerSession->pServerConnection[count_con]->hSnepServerConnHandle)
                        {
                            return pServerSession;
                        } /* No Else */
                    } /* No Else */
                }
            } /* No Else */
        }
    } /* No Else */

    return pServerSession;
}

/* ----------------------- SNEP Client -------------------------- */

/* Retrieves a Client Context
* If NULL is returned indicates context allocation failed */
phnpSnep_Fri_ClientContext_t *phnpSnep_Fri_ContextMgmt_GetClientContext( phnpSnep_Fri_DataParams_t   *pDataParams)
{
    return &(pDataParams->gpClientContext);
}

/* Retrieves a Client Session matching the given Handle */
phnpSnep_Fri_ClientSession_t *phnpSnep_Fri_ContextMgmt_GetClientSession( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                                        ph_NfcHandle                  ClientHandle)
{
    uint8_t count=0;
    phnpSnep_Fri_ClientSession_t   *pSession = NULL;
    phnpSnep_Fri_ClientContext_t   *pClientContext = NULL;

    pClientContext = phnpSnep_Fri_ContextMgmt_GetClientContext(pDataParams);

    if ((NULL != pClientContext) && (0 != pClientContext->CurrentClientCnt))
    {
        for (count = 0; count < PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT; count++)
        {
            if (NULL != pClientContext->pClientSession[count])
            {
                if (ClientHandle == pClientContext->pClientSession[count]->hSnepClientHandle)
                {
                    pSession = pClientContext->pClientSession[count];

                    break;
                } /* No Else */
            } /* No Else */
        }
    }/* No Else */

    return pSession;
}

/* Adds a new Client session to the Client Context */
phnpSnep_Fri_ClientSession_t *phnpSnep_Fri_ContextMgmt_AddClientSession( phnpSnep_Fri_DataParams_t     *pDataParams,
                                                                        phnpSnep_Fri_ClientSession_t   *pClientSession )
{
    uint8_t count=0;
    phnpSnep_Fri_ClientContext_t    *pClientContext = NULL;

    pClientContext = phnpSnep_Fri_ContextMgmt_GetClientContext(pDataParams);

    if ((NULL != pClientContext) && ( PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT > pClientContext->CurrentClientCnt))
    {
        for (count = 0;count < PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT;count++)
        {
            if (NULL == pClientContext->pClientSession[count])
            {
                pClientContext->pClientSession[count] = pClientSession;
                pClientContext->CurrentClientCnt++;
                break;
            }/* No Else */
        }
    }/* No Else */

    return pClientContext->pClientSession[count];
}

/* Removes a Client from the Client Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveClientSession( phnpSnep_Fri_DataParams_t    *pDataParams,
                                                        ph_NfcHandle                  ClientHandle)
{
    phStatus_t  status = PH_ERR_FAILED;
    uint8_t count=0;
    phnpSnep_Fri_ClientContext_t   *pClientContext = NULL;

    pClientContext = phnpSnep_Fri_ContextMgmt_GetClientContext(pDataParams);

    if (NULL != pClientContext)
    {
        for (count = 0; count < PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT; count++)
        {
            if (NULL != pClientContext->pClientSession[count])
            {
                if (ClientHandle == pClientContext->pClientSession[count]->hSnepClientHandle)
                {
                    status = phlnLlcp_Transport_Close (
                                                        pDataParams->plnLlcpDataParams,
                                                        (phlnLlcp_Fri_Transport_Socket_t *) pClientContext->pClientSession[count]->hSnepClientHandle
                                                      );
                    pClientContext->pClientSession[count] = NULL;
                    pClientContext->CurrentClientCnt--;

                    break;
                }
            }
        }
    }
    return status;
}

/* Removes a Incomplete Client from the Client Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveIncompleteClientSession( phnpSnep_Fri_DataParams_t     *pDataParams,
                                                                  phnpSnep_Fri_ClientSession_t   *pClientSession )
{
    phStatus_t   status = PH_ERR_FAILED;
    uint8_t      count=0;
    phnpSnep_Fri_ClientContext_t *pClientContext = NULL;

    pClientContext = phnpSnep_Fri_ContextMgmt_GetClientContext(pDataParams);

    if (NULL != pClientContext && NULL != pClientSession)
    {
        for (count = 0; count < PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT; count++)
        {
            if (NULL != pClientContext->pClientSession[count])
            {
                if (pClientSession == pClientContext->pClientSession[count])
                {
                    pClientContext->CurrentClientCnt--;
                }
            }
        }
    }

    return status;
}

/* Removes all Client Sessions from Client Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveAllClientSessions( phnpSnep_Fri_DataParams_t    *pDataParams )
{
    phStatus_t   status = PH_ERR_FAILED;
    uint8_t      count=0;
    phnpSnep_Fri_ClientContext_t   *pClientContext = NULL;

    pClientContext = phnpSnep_Fri_ContextMgmt_GetClientContext(pDataParams);
    if (NULL != pClientContext)
    {
        for (count = 0; count < PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT; count++)
        {
            if (NULL != pClientContext->pClientSession[count])
            {
                if (NULL != pClientContext->pClientSession[count]->sWorkingBuffer->buffer)
                {
                    status = phlnLlcp_Transport_Close( pDataParams->plnLlcpDataParams,
                                                       (phlnLlcp_Fri_Transport_Socket_t *) pClientContext->pClientSession[count]->hSnepClientHandle);

                    pClientContext->pClientSession[count]->sWorkingBuffer->length=0;
                }
                pClientContext->CurrentClientCnt--;

            }
        }
    }

    return status;
}

#endif /* NXPBUILD__PHNP_SNEP_FRI */
