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
* \file  phnpSnep_Fri_ContextMgmt.h                                           *
* \brief SNEP protocol management public interfaces listed in this module.    *
*                                                                             *
*                                                                             *
* Project:                                                                    *
*                                                                             *
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $                       *
* $Author: nxp62726 $                                                         *
* $Revision: 416 $                                                            *
* $Aliases:  $                                                                *
*                                                                             *
* =========================================================================== *
*/
#ifndef PHNPSNEP_FRI_CONTEXTMGMT_H
#define PHNPSNEP_FRI_CONTEXTMGMT_H

#include <ph_Status.h>
#include "phnpSnep_Fri.h"

/* Initializes the Server Context 
* Returns TRUE if succeeded */

phStatus_t phnpSnep_Fri_ContextMgmt_InitServerContext( phnpSnep_Fri_DataParams_t    *pDataParams);

/* Retrieves a Server Context
* If NULL is returned indicates context is not initialized */

phnpSnep_Fri_ServerContext_t *phnpSnep_Fri_ContextMgmt_GetServerContext( phnpSnep_Fri_DataParams_t   *pDataParams);

/* Removes a Server Context if server count is '0'
* Should be called in every Server De-init.
* Returns FALSE if still active server sessions are present */

phStatus_t phnpSnep_Fri_ContextMgmt_RemoveServerContext( phnpSnep_Fri_DataParams_t   *pDataParams );     /* PRQA S 3209 */

/* Retrieves a Server Session matching the given Handle */

phnpSnep_Fri_ServerSession_t *phnpSnep_Fri_ContextMgmt_GetServerSessionContext( phnpSnep_Fri_DataParams_t  *pDataParams,
                                                                               ph_NfcHandle                 ServerHandle);

/* Adds a new Connection to the Server Session Context */

phnpSnep_Fri_ServerConnection_t *phnpSnep_Fri_ContextMgmt_AddServerConnection( phnpSnep_Fri_DataParams_t  *pDataParams,
                                                                              ph_NfcHandle                 ServerHandle);

/* Retrieves a Server Connection matching the given Handle */

phnpSnep_Fri_ServerConnection_t *phnpSnep_Fri_ContextMgmt_GetServerConnectionContext( phnpSnep_Fri_DataParams_t  *pDataParams,
                                                                                     ph_NfcHandle                 ConnHandle);

/* Adds a new Server to the Server Context */

phnpSnep_Fri_ServerSession_t *phnpSnep_Fri_ContextMgmt_AddServerSession( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                                        phnpSnep_Fri_ServerSession_t  *pServerSession);

/* Adds a new Connection to the Server Session */

phnpSnep_Fri_ServerConnection_t *phnpSnep_Fri_ContextMgmt_AddServerConnection( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                                              ph_NfcHandle                  ServerHandle);

/* Removes a Server session from the Server Context */

phStatus_t phnpSnep_Fri_ContextMgmt_RemoveServerSession( phnpSnep_Fri_DataParams_t  *pDataParams,
                                                        ph_NfcHandle                 ServerHandle);

/* Removes a Connection from the Server Session Context */

phStatus_t phnpSnep_Fri_ContextMgmt_RemoveOneServerConnection( phnpSnep_Fri_DataParams_t    *pDataParams,
                                                              ph_NfcHandle                   ConnHandle);

/* Removes all Connections from a Server Session Context */

phStatus_t phnpSnep_Fri_ContextMgmt_RemoveAllServerConnections( phnpSnep_Fri_DataParams_t   *pDataParams,
                                                               ph_NfcHandle                  ServerHandle);

/* Retrieves a Server Session matching the connection */

phnpSnep_Fri_ServerSession_t *phnpSnep_Fri_ContextMgmt_GetServerSessionByConnection( phnpSnep_Fri_DataParams_t  *pDataParams,
                                                                                    ph_NfcHandle                 ConnHandle);


/* ----------------------- SNEP Client -------------------------- */

/* Retrieves a Client Context
* If NULL is returned indicates context allocation failed */
phnpSnep_Fri_ClientContext_t *phnpSnep_Fri_ContextMgmt_GetClientContext( phnpSnep_Fri_DataParams_t    *pDataParams );

/* Retrieves a Client Session matching the given Handle */
phnpSnep_Fri_ClientSession_t    * phnpSnep_Fri_ContextMgmt_GetClientSession( phnpSnep_Fri_DataParams_t    *pDataParams,
                                                                            ph_NfcHandle                   ClientHandle);

/* Adds a new Client session to the Client Context */
phnpSnep_Fri_ClientSession_t  *phnpSnep_Fri_ContextMgmt_AddClientSession( phnpSnep_Fri_DataParams_t     *pDataParams,
                                                                         phnpSnep_Fri_ClientSession_t   *ClientSession );

/* Removes a Client from the Client Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveClientSession( phnpSnep_Fri_DataParams_t  *pDataParams,
                                                        ph_NfcHandle                ClientHandle);

/* Removes a Incomplete Client from the Client Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveIncompleteClientSession( phnpSnep_Fri_DataParams_t      *pDataParams,  /* PRQA S 3209 */
                                                                  phnpSnep_Fri_ClientSession_t    *pClientSession);

/* Removes all Client Sessions from Client Context */
phStatus_t phnpSnep_Fri_ContextMgmt_RemoveAllClientSessions( phnpSnep_Fri_DataParams_t    *pDataParams );     /* PRQA S 3209 */

#endif /* PHNPSNEP_FRI_CONTEXTMGMT_H */
