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
* \file  phlnLlcp_Fri_Transport_Connectionless.h
* \brief 
*
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
*/
#ifndef PHLNLLCP_FRI_TRANSPORT_CONNECTIONLESS_H
#define PHLNLLCP_FRI_TRANSPORT_CONNECTIONLESS_H

#include <ph_Status.h>
#include <phlnLlcp.h>

void phlnLlcp_Fri_Handle_Connectionless_IncommingFrame(phlnLlcp_Fri_Transport_t     *pLlcpTransport,
                                                       phNfc_sData_t                *psData,
                                                       uint8_t                       dsap,
                                                       uint8_t                       ssap);

phStatus_t phlnLlcp_Fri_Transport_Connectionless_HandlePendingOperations(phlnLlcp_Fri_Transport_Socket_t *pSocket);

/**
*
* \brief <b>Close a socket on a LLCP-connectionless device</b>.
*
* This function closes a LLCP socket previously created using phlnLlcp_Fri_Transport_Socket.
*
* \param[in]  pLlcpSocket                    A pointer to a phlnLlcp_Fri_Transport_Socket_t.

* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_Connectionless_Close(phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket);

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
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_PENDING                  Reception operation is in progress,
*                                            pSend_RspCb will be called upon completion.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of 
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_NOT_INITIALISED          Indicates stack is not yet initialized.
* \retval PHLNLLCP_FRI_NFCSTATUS_SHUTDOWN                 Shutdown in progress.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_Connectionless_SendTo( phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                                       uint8_t                                     nSap,
                                                       phNfc_sData_t                              *psBuffer,
                                                       pphlnLlcp_Fri_TransportSocketSendCb_t       pSend_RspCb,
                                                       void                                       *pContext);

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
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_PENDING                  Reception operation is in progress,
*                                            pRecv_RspCb will be called upon completion.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state, or not of 
*                                            a valid type to perform the requsted operation.
* \retval PH_ERR_NOT_INITIALISED          Indicates stack is not yet initialized.
* \retval PHLNLLCP_FRI_NFCSTATUS_SHUTDOWN                 Shutdown in progress.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_Connectionless_RecvFrom( phlnLlcp_Fri_Transport_Socket_t                 *pLlcpSocket,
                                                         phNfc_sData_t                                    *psBuffer,
                                                         pphlnLlcp_Fri_TransportSocketRecvFromCb_t         pRecv_Cb,
                                                          void                                             *pContext);

#endif /* PHLNLLCP_FRI_TRANSPORT_CONNECTIONLESS_H */
