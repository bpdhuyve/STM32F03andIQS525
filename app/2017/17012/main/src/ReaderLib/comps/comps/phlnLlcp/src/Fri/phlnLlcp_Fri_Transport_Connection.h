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
* \file  phlnLlcp_Fri_Transport_Connection.h
* \brief 
*
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
*/
#ifndef PHLNLLCP_FRI_TRANSPORT_CONNECTION_H
#define PHLNLLCP_FRI_TRANSPORT_CONNECTION_H

#include <ph_Status.h>
#include <phlnLlcp.h>

void phlnLlcp_Fri_Handle_ConnectionOriented_IncommingFrame(phlnLlcp_Fri_Transport_t     *pLlcpTransport,
                                                           phNfc_sData_t                *psData,
                                                           uint8_t                       dsap,
                                                           uint8_t                       ptype,
                                                           uint8_t                       ssap);

phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_HandlePendingOperations(phlnLlcp_Fri_Transport_Socket_t *pSocket);

/**
*
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
                                                                           phlnLlcp_Fri_sSocketOptions_t    *psLocalOptions);

/**
*
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
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_SocketGetRemoteOptions(phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket,
                                                                            phlnLlcp_Fri_sSocketOptions_t         *psRemoteOptions);

/**
*
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
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Close(phlnLlcp_Fri_Transport_Socket_t     *pLlcpSocket);

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
* \param[in]  pLlcpSocket        A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  pListen_Cb         The callback to be called each time the
*                                socket receive a connection request.
* \param[in]  pContext           Upper layer context to be returned in
*                                the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval PH_ERR_INVALID_STATE            The socket is not in a valid state to switch to listening state.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Listen(phlnLlcp_Fri_Transport_Socket_t        *pLlcpSocket,
                                                           pphlnLlcp_Fri_TransportSocketListenCb_t   pListen_Cb,
                                                            void                                     *pContext);


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
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Accept(phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                                            phlnLlcp_Fri_Transport_sSocketOptions_t     *psOptions,
                                                            phNfc_sData_t                               *psWorkingBuffer,
                                                           pphlnLlcp_Fri_TransportSocketErrCb_t         pErr_Cb,
                                                           pphlnLlcp_Fri_TransportSocketAcceptCb_t      pAccept_RspCb,
                                                            void                                        *pContext);


/**
*
* \brief <b>Reject an incoming connection request for a socket</b>.
*
* This functions allows the client to reject an incoming connection request.
* It must be used with the socket provided within the listen callback. The socket
* is implicitly closed when the function is called.
*
* \param[in]  pLlcpSocket           A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  pReject_RspCb         The callback to be called when the Reject operation is completed
* \param[in]  pContext              Upper layer context to be returned in the callback.
*
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval PH_ERR_INVALID_PARAMETER        One or more of the supplied parameters could not be properly interpreted.
* \retval PH_ERR_FAILED                   Operation failed.
*/
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Reject( phlnLlcp_Fri_Transport_Socket_t        *pLlcpSocket,
                                                            pphlnLlcp_Fri_TransportSocketRejectCb_t   pReject_RspCb,
                                                            void                                      *pContext);

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
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Connect( phlnLlcp_Fri_Transport_Socket_t            *pLlcpSocket,
                                                             uint8_t                                    nSap,
                                                             phNfc_sData_t                             *psUri,
                                                             pphlnLlcp_Fri_TransportSocketConnectCb_t   pConnect_RspCb,
                                                             void                                      *pContext);

/**
*
* \brief <b>Disconnect a currently connected socket</b>.
*
* This function initiates the disconnection of a previously connected socket.
*
* \param[in]  pLlcpSocket        A pointer to a phlnLlcp_Fri_Transport_Socket_t.
* \param[in]  pDisconnect_RspCb  The callback to be called when the operation is completed.
* \param[in]  pContext           Upper layer context to be returned in the callback.
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
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Disconnect( phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
                                                                 pphlnLlcp_Fri_LlcpSocketDisconnectCb_t         pDisconnect_RspCb,
                                                                void                                         *pContext);

/**
*
* \brief <b>Send data on a socket</b>.
*
* This function is used to write data on a socket. This function
* can only be called on a connection-oriented socket which is already
* in a connected state.
* 
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
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Send( phlnLlcp_Fri_Transport_Socket_t            *pLlcpSocket,
                                                          phNfc_sData_t                               *psBuffer,
                                                         pphlnLlcp_Fri_TransportSocketSendCb_t        pSend_RspCb,
                                                          void                                        *pContext);

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
phStatus_t phlnLlcp_Fri_Transport_ConnectionOriented_Recv( phlnLlcp_Fri_Transport_Socket_t            *pLlcpSocket,
                                                          phNfc_sData_t                               *psBuffer,
                                                           pphlnLlcp_Fri_TransportSocketRecvCb_t        pRecv_RspCb,
                                                          void                                        *pContext);
#endif /* PHLNLLCP_FRI_TRANSPORT_CONNECTION_H */
