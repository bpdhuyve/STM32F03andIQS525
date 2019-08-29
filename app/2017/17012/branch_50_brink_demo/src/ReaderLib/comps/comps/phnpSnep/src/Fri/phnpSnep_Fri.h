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

/**
* =========================================================================== *
*                                                                             *
*                                                                             *
* \file  phnpSnep_Fri.h                                                       *
* \brief SNEP protocol management public interfaces listed in this module.    *
*                                                                             *
*                                                                             *
* Project:                                                                    *
*                                                                             *
* $Date: 2013-06-17 13:51:16 +0530 (Mon, 17 Jun 2013) $                       *
* $Author: nxp62726 $                                                         *
* $Revision: 172 $                                                            *
* $Aliases:   $                                                               *
*                                                                             *
* =========================================================================== *
*/

#ifndef PHNPSNEP_FRI_H
#define PHNPSNEP_FRI_H

#include <ph_Status.h>
#include <phnpSnep.h>

/* SNEP Version details (length in octets)*/
#define PHNPSNEP_FRI_VERSION_LENGTH   1
#define PHNPSNEP_FRI_VERSION_MAJOR    1
#define PHNPSNEP_FRI_VERSION_MINOR    0
#define PHNPSNEP_FRI_VERSION_FIELD    4

/* SNEP Request details*/
#define PHNPSNEP_FRI_REQUEST_LENGTH     0x01
#define PHNPSNEP_FRI_REQUEST_CONTINUE   0x00
#define PHNPSNEP_FRI_REQUEST_GET        0x01
#define PHNPSNEP_FRI_REQUEST_PUT        0x02
#define PHNPSNEP_FRI_REQUEST_REJECT     0x7F

/* Response field values */
#define PHNPSNEP_FRI_RESPONSE_CONTINUE              0x80
#define PHNPSNEP_FRI_RESPONSE_SUCCESS               0x81
#define PHNPSNEP_FRI_RESPONSE_NOT_FOUND             0xC0
#define PHNPSNEP_FRI_RESPONSE_EXCESS_DATA           0xC1
#define PHNPSNEP_FRI_RESPONSE_BAD_REQUEST           0xC2
#define PHNPSNEP_FRI_RESPONSE_NOT_IMPLEMENTED       0xE0
#define PHNPSNEP_FRI_RESPONSE_UNSUPPORTED_VERSION   0xE1
#define PHNPSNEP_FRI_RESPONSE_REJECT                0xFF

#define PHNPSNEP_FRI_REQUEST_LENGTH_SIZE              4
#define PHNPSNEP_FRI_REQUEST_ACCEPTABLE_LENGTH_SIZE   4

#define PHNPSNEP_FRI_HEADER_SIZE (PHNPSNEP_FRI_VERSION_LENGTH+PHNPSNEP_FRI_REQUEST_LENGTH+PHNPSNEP_FRI_REQUEST_LENGTH_SIZE)

/* TODO: define offsets for each filed */
#define PHNPSNEP_FRI_VERSION (PHNPSNEP_FRI_VERSION_MAJOR*16)+PHNPSNEP_FRI_VERSION_MINOR

/* NFC status needed for snep */

/* Response field values */
#define PHNPSNEP_FRI_STATUS_RESPONSE_CONTINUE              0x50
#define PHNPSNEP_FRI_STATUS_RESPONSE_SUCCESS               0x51
#define PHNPSNEP_FRI_STATUS_RESPONSE_NOT_FOUND             0x52
#define PHNPSNEP_FRI_STATUS_RESPONSE_EXCESS_DATA           0x53
#define PHNPSNEP_FRI_STATUS_RESPONSE_BAD_REQUEST           0x54
#define PHNPSNEP_FRI_STATUS_RESPONSE_NOT_IMPLEMENTED       0x55
#define PHNPSNEP_FRI_STATUS_RESPONSE_UNSUPPORTED_VERSION   0x56
#define PHNPSNEP_FRI_STATUS_RESPONSE_REJECT                0x57
#define PHNPSNEP_FRI_STATUS_REQUEST_REJECT                 0x58
#define PHNPSNEP_FRI_STATUS_INVALID_PROTOCOL_DATA          0x59
#define PHNPSNEP_FRI_STATUS_REQUEST_CONTINUE_FAILED        0x60
#define PHNPSNEP_FRI_STATUS_REQUEST_REJECT_FAILED          0x61

/* define all packet types */
typedef enum
{
    phnpSnep_Fri_Put = 0x00,   /* Put request by client */
    phnpSnep_Fri_Get,          /* Get request */
    phnpSnep_Fri_Continue,
    phnpSnep_Fri_Success,
    phnpSnep_Fri_NotFound,
    phnpSnep_Fri_ExcessData,
    phnpSnep_Fri_BadRequest,
    phnpSnep_Fri_NotImplemented,
    phnpSnep_Fri_UnsupportedVersion,
    phnpSnep_Fri_Reject,
    phnpSnep_Fri_Invalid
} phnpSnep_Fri_Packet_t;

/**
* \brief <b>Interface to create and configure SNEP server</b>.
*
* This function creates and configures the a SNEP server over LLCP.
*
* \param[in] pConfigInfo   Contains SNEP server configuration
* \param[in] pConnCb       This callback is called once LibNfc
*                          for each incoming connection to server.
* \param[out] pServerHandle   Server Session Handle.*
* \param[in] pContext      Upper layer context to be returned in
*                          the callback.
*
* \return Status code
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_Fri_Server_Init( phnpSnep_Fri_DataParams_t   *pDataParams,
                                     phnpSnep_Fri_Config_t       *pConfigInfo,
                                     pphnpSnep_Fri_ConnectCB_t    pConnCb,
                                     ph_NfcHandle                *pServerHandle,
                                     phnpSnep_Fri_ServerSession_t  *ServerSession,
                                     void                        *pContext);

/**
* \brief <b>Interface to accept incoming client connection to server and share inbox </b>.
*
* This function should be called after pphLibNfc_SnepServerConn_ntf_t callback is received
* to indicate the acceptance of incoming connection and sharing of inbox.
*
* \param[in] pDataInbox   Inbox to hold received and saved data.
* \param[in] pSockOps     Socket options (Latest Link Parameters).
* \param[in] hRemoteDevHandle Remote Device Handle.
* \param[in] ServerHandle Server Session Handle.
* \param[in] ConnHandle   Handle to incoming connection.
* \param[in] pPutNtfCb    Put Notification callback for incoming data.
* \param[in] pGetNtfCb    Get Notification callback for incoming data request.
* \param[in] pContext     Upper layer context to be returned in the callback.
*
* \return Status code
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_Fri_Server_Accept( phnpSnep_Fri_DataParams_t            *pDataParams,
                                       phNfc_sData_t                        *pDataInbox,
                                       phlnLlcp_Fri_Transport_sSocketOptions_t  *pSockOps,                                      
                                       ph_NfcHandle                          ServerHandle,
                                       ph_NfcHandle                          ConnHandle,
                                       pphnpSnep_Fri_Put_ntf_t               pPutNtfCb,
                                       pphnpSnep_Fri_Get_ntf_t               pGetNtfCb,
                                       void                                 *pContext );

/**
* \brief <b>Interface Un-initialize server session</b>.
*
* Un-initializes the server session and closes any open connections.
*
* \param[in] ServerHandle   Server Session Handle received in phLibNfc_SnepServer_Init()
*
* \return Status code
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_Fri_Server_DeInit( phnpSnep_Fri_DataParams_t     *pDataParams,
                                       ph_NfcHandle                   ServerHandle );


/* ----------------------- SNEP CLIENT INTERFACE METHODS -------------------------- */

/**
* \brief <b>Interface to create and configure SNEP Client</b>.
*
* This function creates and configures the a SNEP Client over LLCP.
*
* \param[in] pConfigInfo   Contains Peer SNEP server information
* \param[in] hRemDevHandle  Remote device handle
* \param[in] pConnClientCb  This callback is called once snep client
*                           is initialized and connected to SNEP Server.
* \param[in] pContext      Upper layer context to be returned in
*                          the callback.
*
* \return Status code
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_Fri_Client_Init( phnpSnep_Fri_DataParams_t    *pDataParams,
                                     phnpSnep_Fri_Config_t        *pConfigInfo,
                                     ph_NfcHandle                  hRemDevHandle,
                                     pphnpSnep_Fri_ConnectCB_t     pConnClientCb,
                                     phnpSnep_Fri_ClientSession_t *pClientSession,
                                     void                         *pContext);

/**
* \brief <b>Interface Un-initialize client session</b>.
*
* Un-initializes the Client session and closes any open connections.
*
* \param[in] ClientHandle   Client Session Handle received in pphLibNfc_SnepConn_ntf_t()
*
* \return Status code
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/

phStatus_t phnpSnep_Fri_Client_DeInit( phnpSnep_Fri_DataParams_t   *pDataParams,
                                       ph_NfcHandle                 ClientHandle);

/* ----------------------- SNEP CLIENT API------------------------------------ */
/**
*  The phLibNfc_SnepProtocolCliReqPut Sends the data to peer and calls the callback function
*  to confirm completion. It is the responsibility of the caller to deallocate memory returned
*  in the callback. Deallocation should be done twice once for phNfc_sData_t->buffer and again
*  for phNfc_sData_t.
*
*  \param[in]  ConnHandle              Connection handle to identify session uniquely.
*  \param[in]  pPutData                Data to send
*  \param[in]  fCbPut                  Completion notifier callback function
*  \param[in]  cbContext               Context to be passed to callback function
*
* \return Status code
* \retval PH_ERR_SUCCESS               Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_Fri_ClientReqPut( phnpSnep_Fri_DataParams_t   *pDataParams,
                                      ph_NfcHandle                 Socket_Handle,
                                      phNfc_sData_t               *sUri,
                                      pphnpSnep_Fri_ReqCb_t        DummyContext,
                                      void                        *cbContext
                                    );
/**
*  The phLibNfc_SnepProtocolCliReqGet Sends a request to receive data from peer and calls the callback function
*  to confirm receipt. It is the responsibility of the caller to deallocate memory returned in the callback.
*  Deallocation should be done twice once for phNfc_sData_t->buffer and again for phNfc_sData_t.
*
*  \param[in]  ConnHandle              Connection handle to identify session uniquely.
*  \param[in]  pGetData                Data sent to server as part of GET request.
*  \param[in]  fCbGet                  Reception notifier callback function.
*  \param[in]  acceptable_length       Max length of expected data.
*  \param[in]  cbContext               Context to be passed to callback function.
*
* \return Status code
* \retval PH_ERR_SUCCESS                  Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_Fri_ClientReqGet( phnpSnep_Fri_DataParams_t    *pDataParams,
                                      ph_NfcHandle                  ConnHandle,
                                      phNfc_sData_t                *pGetData,
                                      uint32_t                      Acceptable_length,
                                      pphnpSnep_Fri_ReqCb_t         fCbGet,
                                      void                         *cbContext);


/* ----------------------- SNEP SERVER API------------------------------------ */

/**
* \brief  Generic Server Response Callback definition.
*
* Generic callback definition used to indicate sending of Server response.
*
* \note : Status and error codes for this type of callback are documented in respective APIs
* wherever it is used.
* \param[in] pContext       LibNfc client context   passed in the corresponding request before.
* \param[in] status         Status of the response  callback.
* \param[in] ConnHandle     Snep Incoming Connection Handle.
*/
typedef void(*pphLibNfc_SnepProtocol_SendRspComplete_t)( void         *pContext,
                                                         phStatus_t    Status,
                                                         ph_NfcHandle  ConnHandle);


/**
*  \param[in]  ConnHandle              Connection handle to identify session uniquely.
*  \param[in]  pResponseData           Data sent to client as reponse to a request.
*  \param[in]  responseStatus          Response status.
*  \param[in]  fSendCompleteCb         Response sent notifier callback function.
*  \param[in]  cbContext               Context to be passed to callback function.
*
* \return Status code
* \retval PH_ERR_SUCCESS               Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_Fri_ServerSendResponse( phnpSnep_Fri_DataParams_t            *pDataParams  /* PRQA S 3209 */,
                                            ph_NfcHandle                          ConnHandle,
                                            phNfc_sData_t                        *pResponseData,
                                            phStatus_t                            responseStatus,
                                            pphnpSnep_Fri_Protocol_SendRspCb_t    fSendCompleteCb,
                                            void                                 *cbContext
                                           );


#endif /* PHNPSNEP_FRI_H */
