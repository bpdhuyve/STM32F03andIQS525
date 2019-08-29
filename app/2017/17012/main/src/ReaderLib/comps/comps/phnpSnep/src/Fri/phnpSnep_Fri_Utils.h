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
* \file  phnpSnep_Fri_Utils.h                                                 *
* \brief SNEP protocol helper modules.                                        *
*                                                                             *
*                                                                             *
* Project:                                                                    *
*                                                                             *
* $Date: 2013-06-17 13:51:16 +0530 (Mon, 17 Jun 2013) $                       *
* $Author: nxp62726 $                                                         *
* $Revision: 172 $                                                            *
* $Aliases:  $                                                                *
*                                                                             *
* =========================================================================== *
*/

#ifndef PHNPSNEP_FRI_UTILS_H
#define PHNPSNEP_FRI_UTILS_H

#include <ph_Status.h>


#include "phnpSnep_Fri.h"

phStatus_t phnpSnep_Fri_Utils_Req( phnpSnep_Fri_DataParams_t      *pDataParams,
                                  pphnpSnep_Fri_ClientSession_t    pClientSessionContext);

phStatus_t phnpSnep_Fri_Utils_SendResponse( phnpSnep_Fri_DataParams_t        *pDataParams,
                                           pphnpSnep_Fri_ServerConnection_t   pSnepServerConnection);


/*@TODO Remove once LLCP Send issue resolved */
void phnpSnep_Fri_Utils_SocketSendData( void         *pContext,
                                        phStatus_t    status);

void phnpSnep_Fri_Utils_SocketReceiveData( void        *pContext,
                                           phStatus_t   status);

void phnpSnep_Fri_Utils_SocketSendDataServer( void        *pContext,
                                              phStatus_t    status);

phStatus_t  phnpSnep_Fri_Utils_PrepareSnepPacket( phnpSnep_Fri_DataParams_t    *pDataParams,
                                                 phnpSnep_Fri_Packet_t          packetType,
                                                 phNfc_sData_t                 *pData,
                                                 phNfc_sData_t                 *pSnepPacket,
                                                 phNfc_sData_t                 *pSnepHeader,
                                                 uint8_t                        version,
                                                 uint32_t                       acceptableLength);

phStatus_t phnpSnep_Fri_Utils_CollectReply( pphnpSnep_Fri_ClientSession_t   pClientSessionContext );

phStatus_t phnpSnep_Fri_Utils_GetNfcStatusFromSnepResponse( uint8_t   snepCode );

uint8_t phnpSnep_Fri_Utils_GetSnepResponseCodeFromPacketType( phnpSnep_Fri_Packet_t    packetType );

uint8_t phnpSnep_Fri_Utils_GetSnepResponseCodeFromStatus( phStatus_t    nfcStatus );

phnpSnep_Fri_Packet_t phnpSnep_Fri_Utils_GetPacketType(phStatus_t    nfcStatus);

void phnpSnep_Fri_Utils_SendRequestContinue( pphnpSnep_Fri_ClientSession_t   pClientSessionContext );

void phnpSnep_Fri_Utils_SendRequestReject( pphnpSnep_Fri_ClientSession_t    pClientSessionContext );

void phnpSnep_Fri_Utils_ClearMemNCallResponseCb( pphnpSnep_Fri_ServerConnection_t    pSnepServerConnection,
                                                 phStatus_t                          status);

void phnpSnep_Fri_Utils_NotifyUpperLayer( phnpSnep_Fri_DataParams_t          *pDataParams,
                                          pphnpSnep_Fri_ServerConnection_t    pServerConnectionContext);

uint8_t phnpSnep_Fri_Utils_VersionsCompatibleChk( uint8_t    ver1,
                                                 uint8_t    ver2);

void phnpSnep_Fri_Utils_ResetServerConnectionContext( pphnpSnep_Fri_ServerConnection_t    pServerConnectionContext);

void phnpSnep_Fri_Utils_ResetCliDataContext( pphnpSnep_Fri_ClientSession_t    pClientSessionContext);

void phnpSnep_Fri_Utils_SocketRecvCbForServer( void         *pContext,
                                               phStatus_t    status);
#endif /* PHNPSNEP_FRI_UTILS_H */
