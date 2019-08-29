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
* \file  phnpSnep.h                                                           *
* \brief SNEP protocol management public interfaces listed in this module.    *
*                                                                             *
*                                                                             *
* Project:                                                                    *
*                                                                             *
* $Date: 2014-02-11 10:12:36 +0530 (Tue, 11 Feb 2014) $                       *
* $Author: nxp69453 $                                                         *
* $Revision: 498 $                                                            *
* $Aliases:  $                                                                *
*                                                                             *
* =========================================================================== *
*/

#ifndef PHNPSNEP_H
#define PHNPSNEP_H

#include <ph_Status.h>
#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */



#include <phlnLlcp.h>

#ifdef NXPBUILD__PHNP_SNEP_FRI
/** \defgroup phnpSnep_Fri Component : SNEP FRI
* \brief implementation of SNEP FRI
*
* @{
*/

/* ----------------------- MACROS -------------------------- */
#define PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT   0x01         /**< Maximum SNEP server count. */
#define PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN  0x01         /**< Maximum server connection. */
#define PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT   0x01         /**< Maximum SNEP client count. */

#define PHNPSNEP_FRI_MIN_MIU_VAL            6           /**< Minimum MIU requirement for SNEP. */
#define PHNPSNEP_FRI_MIN_INBOX_SIZE         1024        /**< Minimum inbox size for default server. */
#define PHNPSNEP_FRI_INCOMING_CONNECTION    (0x0025)    /**< Indicates incoming connection. */
#define PHNPSNEP_FRI_CONNECTION_SUCCESS     (0x0026)    /**< Indicates Connection was successful. */
#define PHNPSNEP_FRI_CONNECTION_FAILED      (0x0027)    /**< Indicates Connection failed. */

/* ----------------------- SNEP SERVER CALLBACKS -------------------------- */

/**
* \brief  Generic Response Callback definition.
*
* Generic callback definition used as callback type in few APIs below.
*
* \note : Status and error codes for this type of callback are documented in respective APIs
* wherever it is used.
*
* \param[in] pContext       client context   passed in the corresponding request before.
* \param[in] status         Status of the response  callback.
* \param[in] ConnHandle     Snep Incoming Connection Handle.
*/
typedef void(*pphnpSnep_Fri_ConnectCB_t) (void  *pContext, uint32_t ConnHandle, phStatus_t  Status );

/**
* \brief  Generic Response Callback definition.
*
* Generic callback definition used as callback type in few APIs below.
*
* \note : Status and error codes for this type of callback are documented in respective APIs
* wherever it is used.
*
* \param[in] pContext       client context   passed in the corresponding request
*                           before.
* \param[in] status         Status of the response  callback.
* \param[in] pDataInbox     Incoming data buffer (NDEF Message).
* \param[in] ConnHandle     Snep Connection Handle.
*/
typedef void(*pphnpSnep_Fri_Put_ntf_t)( void             *pContext,
                                        phStatus_t        Status,
                                        phNfc_sData_t    *pDataInbox,
                                        ph_NfcHandle      ConnHandle);

/**
* \brief  Generic Response Callback definition.
*
* Generic callback definition used as callback type in few APIs below.
*
* \note : The parameter 'pGetMsgId' is NDEF Message Identifier for Incoming GET request.
*This buffer is deleted by caller as the callback returns. Status and error codes for this type of callback are documented in respective APIs
* wherever it is used.
*
* \param[in] pContext       client context   passed in the corresponding request
*                           before.
* \param[in] status         Status of the response  callback.
* \param[in] pGetMsgId      NDEF Message Identifier for Incoming GET request.
* \param[in] ConnHandle     Snep Connection Handle.
*/
typedef void(*pphnpSnep_Fri_Get_ntf_t) ( void            *pContext,
		                                 phStatus_t       Status,
		                                 phNfc_sData_t   *pGetMsgId,
		                                 ph_NfcHandle     ConnHandle);

/* ----------------------- SNEP CLIENT CALLBACKS -------------------------- */

/**
* \brief  Generic client Callback definition for Put and Get
*
* \note : Status and error codes for this type of callback are documented in respective APIs wherever it is used.
* \param[in] ConnHandle     Snep Incoming Connection Handle.
* \param[in] pContext      client context   passed in the corresponding request before.
* \param[out] status       Status of the response  callback.
* \param[out] pReqResponse      data returned by peer for request sent. may be NULL(for PUT request).
*/
typedef void (*pphnpSnep_Fri_ReqCb_t) ( ph_NfcHandle     ConnHandle,
                                        void            *pContext,
                                        phStatus_t       Status,
                                        phNfc_sData_t   *pReqResponse);

typedef void(*pphnpSnep_Fri_Protocol_SendRspCb_t) ( void           *pContext,
                                                    phStatus_t      Status,
                                                    ph_NfcHandle    ConnHandle);
/**
*\brief Defines snep server type
*/
typedef enum
{
    phnpSnep_Fri_Server_Default = 0x00,     /**< Default NFC Forum SNEP server. */
    phnpSnep_Fri_Server_NonDefault          /**< Proprietary SNEP server. */

} phnpSnep_Fri_Server_type_t;

/**
*\brief Defines states of snep client.
*/
typedef enum
{
    phnpSnep_Fri_Client_Uninitialized = 0x00,   /**< Client Not Registered. */
    phnpSnep_Fri_Client_Initialized,            /**< Client Registered not connected to Server. */
    phnpSnep_Fri_Client_Connected,              /**< Client Connected with peer Server. */
    phnpSnep_Fri_Client_PutResponse,            /**< Client waiting for PUT response. */
    phnpSnep_Fri_Client_GetResponse             /**< Client waiting for GET response. */
} phnpSnep_Fri_Client_status_t;

/**
*\brief Defines server status type.
*/
typedef enum
{
    phnpSnep_Fri_Server_Uninitialized = 0x00,   /**< Server Not Registered. */
    phnpSnep_Fri_Server_Initialized,            /**< Server Registered not connected to client. */
    phnpSnep_Fri_Server_Connected,              /**< Server Connected with atleast one Peer client. */
    phnpSnep_Fri_Server_Sending,                /**< Server Sending Response to Peer client. */
    phnpSnep_Fri_Server_Received_Put,           /**< Server Connection Received PUT Request. */
    phnpSnep_Fri_Server_Received_Get            /**< Server Connection Received GET Request. */
} phnpSnep_Fri_Server_status_t;

/* ----------------------- SNEP SERVER ------------------------------------ */

/**
*\brief Defines Response data context
*/
typedef struct phnpSnep_Fri_sendResponseDataContext
{
    uint8_t                               bWaitForContinue;  /**< Do we need to wait for continue before send. */
    uint8_t                               bContinueReceived; /**< Have we received continue to send. */
    uint8_t                               bIsExcessData;     /**< Flag to indicate excess data. */
    uint32_t                              iAcceptableLength; /**< Acceptable length. */
    uint32_t                              iDataSent;         /**< count of data sent so far. */
    phNfc_sData_t                        *pSnepPacket;       /**< prepared snep packet. */
    phNfc_sData_t                        *pSnepHeader;       /**< prepared snep Header. */
    phNfc_sData_t                        *pChunkingBuffer;   /**< Chucking buffer for send. */
    pphnpSnep_Fri_Protocol_SendRspCb_t    fSendCompleteCb;   /**< Send complete callback. */
    void                                 *cbContext;         /**< Context. */
}phnpSnep_Fri_sendResponseDataContext_t;

/**
*\brief Defines put get data context
*/
typedef struct phnpSnep_Fri_putGetDataContext
{
    uint8_t           bWaitForContinue;      /**< Do we need to wait for continue before send. */
    uint8_t           bContinueReceived;     /**< Have we received continue to send. */
    uint32_t          iDataSent;             /**< count of data sent so far. */
    uint32_t          iDataReceived;         /**< count of data received so far. */
    uint8_t          *pNdefData;
    phNfc_sData_t    *pSnepHeader;
    phNfc_sData_t    *pSnepPacket;           /**< prepared snep packet. */
    phNfc_sData_t    *pReqResponse;          /**< response data to be sent back to upper layer. */
    phNfc_sData_t    *pChunkingBuffer;       /**< chunking and temporary buffer for receive. */
}phnpSnep_Fri_putGetDataContext_t;

/**
*\brief Defines snep socket type and options
*/
typedef struct phnpSnep_Fri_Config
{
    phlnLlcp_Fri_Transport_sSocketOptions_t    sOptions;          /**< LLCP Socket Options. */
    phnpSnep_Fri_Server_type_t                 SnepServerType;    /**< SNEP Server type to be Initialized. */
    phNfc_sData_t                             *SnepServerName;    /**< SNEP Service name. */
}phnpSnep_Fri_Config_t;

/**
*\brief Single Data link connection on SNEP server
*/
typedef struct phnpSnep_Fri_ServerConnection
{
    uint8_t                        SnepServerVersion;        /**< SNEP protocol version supported by Server. */
    uint32_t                       iInboxSize;               /**< size of inbox buffer. */
    uint32_t                       iDataTobeReceived;        /**< size of NDEF msg inside SNEP packet to be read. */
    uint32_t                       iMiu;                     /**< Local MIU for DLC connection. */
    uint32_t                       iRemoteMiu;               /**< Remote MIU for DLC connection. */
    ph_NfcHandle                   hSnepServerConnHandle;    /**< SNEP Server Data link connection Handle. */
    ph_NfcHandle                   hRemoteDevHandle;         /**< Remote device Handle for peer device. */
    phnpSnep_Fri_Server_status_t   ServerConnectionState;    /**< Connection Status. */
    pphnpSnep_Fri_Put_ntf_t        pPutNtfCb;                /**< Put Notification to upper layer. */
    pphnpSnep_Fri_Get_ntf_t        pGetNtfCb;                /**< Get Notification to upper layer. */
    phNfc_sData_t                 *pConnWorkingBuffer;       /**< Working buffer for LLCP connection. */
    phNfc_sData_t                 *pDataInbox;               /**< SNEP Connection Inbox. */
    phNfc_sData_t                 *pSnepWorkingBuffer;       /**< Working buffer for SNEP connection. */
    void                          *pContextForPutCb;         /**< Callback Context */
    void                          *pContextForGetCb;         /**< context of get callback. */
    void                          *pConnectionContext;       /**< Connection Context. */
    void                          *pSnepDataParamsContext;   /**< SNEP Data Parameter Context */
    phnpSnep_Fri_sendResponseDataContext_t      responseDataContext;     /**< context of data transfer transaction. */

}phnpSnep_Fri_ServerConnection_t, *pphnpSnep_Fri_ServerConnection_t;

/**
*\brief Single SNEP Server Info
*/
typedef struct phnpSnep_Fri_ServerSession
{
    uint8_t                             SnepServerSap;          /**< SNEP Server Sap on LLCP. */
    uint8_t                             SnepServerVersion;      /**< SNEP protocol version supported by Server. */
    uint8_t                             SnepServerType;         /**< SNEP server type as initialized. */
    uint8_t                             CurrentConnCnt;         /**< Current connection count. */
    phnpSnep_Fri_Server_status_t        Server_state;           /**< SNEP Server status. */
    ph_NfcHandle                        hSnepServerHandle;      /**< SNEP Server Data link connection Handle. */
    phNfc_sData_t                      *pWorkingBuffer;         /**< Working buffer for LLCP socket. */
    phnpSnep_Fri_ServerConnection_t    *pServerConnection[PHNPSNEP_FRI_MAX_SNEP_SERVER_CONN];   /**< SNEP server connection. */
    pphnpSnep_Fri_ConnectCB_t           pConnectionCb;          /**< Connection notification callback. */
    void                               *pListenContext;         /**< Server Listen Context. */
}phnpSnep_Fri_ServerSession_t, *pphnpSnep_Fri_ServerSession_t;

/**
*
*
*\brief SNEP Server context. Maintains all active snep server entries
*/
typedef struct phnpSnep_Fri_ServerContext
{
    uint8_t                          CurrentServerCnt;                                    /**< Currently registered SNEP servers. */
    phnpSnep_Fri_ServerSession_t    *pServerSession[PHNPSNEP_FRI_MAX_SNEP_SERVER_CNT];    /**< SNEP server sessions. */
}phnpSnep_Fri_ServerContext_t;

/* ----------------------- SNEP CLIENT ------------------------------------ */
/**
*\brief Single SNEP Client session structure
*/
typedef struct phnpSnep_Fri_ClientSession
{
    uint8_t                         SnepClientVersion;      /**< SNEP protocol version supported by Client. */
    uint8_t                         bChunking;              /**< Chunking Buffer. */
    uint32_t                        iMiu;                   /**< Local MIU for DLC connection. */
    uint32_t                        iRemoteMiu;             /**< Remote MIU for DLC connection. */
    uint32_t                        acceptableLength;       /**< Get Request acceptable Length */
    phnpSnep_Fri_Client_status_t    Client_state;           /**< SNEP Client status. */
    ph_NfcHandle                    hSnepClientHandle;      /**< SNEP Client Data link connection Handle. */
    ph_NfcHandle                    hRemoteDevHandle;       /**< Remote device Handle for peer device. */
    phNfc_sData_t                  *sWorkingBuffer;         /**< Working buffer for LLCP socket. */
    pphnpSnep_Fri_ConnectCB_t       pConnectionCb;          /**< Connection notification callback. */
    pphnpSnep_Fri_ReqCb_t           pReqCb;                 /**< Put callback & associated context. */
    phnpSnep_Fri_Config_t          *pSnepClientInitDataParams; /**< Defines snep server type */
    void                           *pClientContext;         /**< Client Connect Context. */
    void                           *pReqCbContext;          /**< context for Request callback. */
    void                           *pSnepDataParamsContext; /**< SNEP Data Parameter Context */
    phnpSnep_Fri_putGetDataContext_t    putGetDataContext;  /**< Context for PUT callback. */
}phnpSnep_Fri_ClientSession_t, *pphnpSnep_Fri_ClientSession_t;

/**
*\brief SNEP Client context structure
*/
typedef struct phnpSnep_Fri_ClientContext
{
    uint8_t                          CurrentClientCnt;                                    /**< Currently registered SNEP Clients */
    phnpSnep_Fri_ClientSession_t    *pClientSession[PHNPSNEP_FRI_MAX_SNEP_CLIENT_CNT];    /**< SNEP Client sessions */
}phnpSnep_Fri_ClientContext_t;

/**
*\brief SNEP parameter structure
*/
typedef struct phnpSnep_Fri_DataParams
{
    uint16_t                         wId;                /**< Layer ID for this component, NEVER MODIFY! */
    phnpSnep_Fri_ClientContext_t     gpClientContext;    /**< SNEP Client context. Maintains all active snep Client entries. */
    phnpSnep_Fri_ServerContext_t     gpServerContext;    /**< SNEP Server context. Maintains all active snep Server entries. */
    void                            *plnLlcpDataParams;  /**< LLCP Data Params. */
    void                            *pOsal;              /**< Osal component parameter. */
}phnpSnep_Fri_DataParams_t;


/**
* \brief Initialize the SNEP component.
* \param[in] pDataParams        Pointer to this layer's parameter structure.
* \param[in] wSizeOfDataParams  Size of this layer's Data Params.
* \param[in] plnLlcpDataparams  LLCP Data Params.
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS   Invalid Component ID
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phnpSnep_Fri_Init( phnpSnep_Fri_DataParams_t   *pDataParams,
                              uint16_t                     wSizeOfDataParams,
                              void                        *plnLlcpDataparams
                             );

/** @} */
#endif /* NXPBUILD__PHNP_SNEP_FRI */

#ifdef NXPBUILD__PHNP_SNEP

    /** \defgroup phnpSnep SNEP
    * \brief These are the components which are used to abstract SNEP functionality
    *
    * @{
    */
#define PHNP_SNEP_FRI_ID               0x01    /**< ID for SNEP FRI component */

#define PHNPSNEP_FRI_LLCP_MIU_DEFAULT  128     /**< Default MIU value (in bytes).*/

/**
* \brief <b>Interface to create and configure SNEP Server</b>.
* This function creates and configures the a SNEP Server over LLCP.
*
* \param[in] pDataParams    Pointer to this layer's parameter structure.
* \param[in] pConfigInfo    Contains SNEP Server information
* \param[in] pConnCb        This callback is called once SNEP Server
*                           is initialized and connected to SNEP Client.
* \param[in] pServerHandle  Server Session handle
* \param[in] pServerSession Server Session params
* \param[in] pContext       Upper layer context to be returned in the callback.
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER       One or more of the supplied parameters
*                                         could not be properly interpreted.
* \retval #PH_ERR_BUFFER_TOO_SMALL        The working buffer is too small for the MIU and RW
*                                         declared in the options.
* \retval #PH_ERR_FAILED                  Operation failed.
* \retval #PH_ERR_BUSY                    Previous request in progress can not accept new request.
* \retval #PH_ERR_INVALID_STATE           The socket is not in a valid state, or not of
*                                         a valid type to perform the requsted operation.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t  phnpSnep_Server_Init( void                         *pDataParams,
                                  phnpSnep_Fri_Config_t        *pConfigInfo,
                                  pphnpSnep_Fri_ConnectCB_t     pConnCb,
                                  ph_NfcHandle                 *pServerHandle,
                                  phnpSnep_Fri_ServerSession_t *pServerSession,
                                  void                         *pContext);

/**
* \brief <b>Interface to create and configure SNEP Server Accept connection</b>.
* This function Accepts the a SNEP Client connection.
*
* \param[in] pDataParams    Pointer to this layer's parameter structure.
* \param[in] pDataInbox     Application receive buffer
* \param[in] pSockOps       Socket Options
* \param[in] ServerHandle   Server Session handle.
* \param[in] ConnHandle     Server Connection handle.
* \param[in] pPutNtfCb      Notifier callback for Put Request
* \param[in] pGetNtfCb      Notifier callback for Get Request
* \param[in] pContext       Upper layer context to be returned in the callback.
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER       One or more of the supplied parameters
*                                         could not be properly interpreted.
* \retval #PH_ERR_BUFFER_TOO_SMALL        The working buffer is too small for the MIU and RW
*                                         declared in the options.
* \retval #PH_ERR_FAILED                  Operation failed.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t  phnpSnep_Server_Accept( void                              *pDataParams,
                                    phNfc_sData_t                     *pDataInbox,
                                    phlnLlcp_Fri_Transport_sSocketOptions_t *pSockOps,
                                    ph_NfcHandle                       ServerHandle,
                                    ph_NfcHandle                       ConnHandle,
                                    pphnpSnep_Fri_Put_ntf_t            pPutNtfCb,
                                    pphnpSnep_Fri_Get_ntf_t            pGetNtfCb,
                                    void                              *pContext );

/**
* \brief <b>SNEP Server De-Initialization</b>.
* This function DeInt's the a SNEP Server session.
*
* \param[in] pDataParams    Pointer to this layer's parameter structure.
* \param[in] ServerHandle   Server Session handle.
* \return Status code
* \retval #PH_ERR_SUCCESS   Operation successful.
* \retval #PH_ERR_FAILED    Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phnpSnep_Server_DeInit ( void          *pDataParams,
                                    ph_NfcHandle   ServerHandle );

/**
*
* \brief <b>Interface to create and configure SNEP Client Initialization</b>.
* This function creates and configures the a SNEP Client over LLCP.
*
* \param[in] pDataParams    Pointer to this layer's parameter structure.
* \param[in] pConfigInfo    Contains Peer SNEP Client information
* \param[in] hRemDevHandle  Remote device handle
* \param[in] pConnClientCb  This callback is called once snep client is initialized and connected to SNEP Server.
* \param[in] pClientSession Client Session Params
* \param[in] pContext       Upper layer context to be returned in the callback.
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER       One or more of the supplied parameters
*                                         could not be properly interpreted.
* \retval #PH_ERR_BUFFER_TOO_SMALL        The working buffer is too small for the MIU and RW
*                                         declared in the options.
* \retval #PH_ERR_PENDING                 Client initialization is in progress.
* \retval #PH_ERR_INVALID_STATE           The socket is not in a valid state, or not of
*                                         a valid type to perform the requsted operation.
* \retval #PH_ERR_FAILED                  Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phnpSnep_Client_Init( void                          *pDataParams,
                                 phnpSnep_Fri_Config_t         *pConfigInfo,
                                 ph_NfcHandle                   hRemDevHandle,
                                 pphnpSnep_Fri_ConnectCB_t      pConnClientCb,
                                 phnpSnep_Fri_ClientSession_t  *pClientSession,
                                 void                          *pContext);

/**
* \brief <b>Snep Client De-Initialization</b>.
* This function will perform Client DeInit.
* \param[in] pDataParams    Pointer to this layer's parameter structure.
* \param[in] ConnHandle     Client Connection handle.
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_FAILED                  Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phnpSnep_Client_DeInit( void           *pDataParams,
                                   ph_NfcHandle    ConnHandle
                                  );


/**
* \brief <b>SNEP Client Request Put</b>.
* This function will perform PUT action.
* \param[in] pDataParams    Pointer to this layer's parameter structure.
* \param[in] ConnHandle     Client Connection handle.
* \param[in] pPutData       Put Request Data
* \param[in] fCbPut         Notifier callback for Put Request
* \param[in] cbContext      Upper layer context to be returned in the callback.
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_PENDING                 PUT operation is in progress.
* \retval #PH_ERR_INVALID_PARAMETER       One or more of the supplied parameters
*                                         could not be properly interpreted.
* \retval #PH_ERR_INVALID_STATE           The socket is not in a valid state, or not of
*                                         a valid type to perform the requsted operation.
* \retval #PH_ERR_FAILED                  Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phnpSnep_ClientReqPut( void                    *pDataParams,
                                  ph_NfcHandle             ConnHandle,
                                  phNfc_sData_t           *pPutData,
                                  pphnpSnep_Fri_ReqCb_t    fCbPut,
                                  void                    *cbContext);


/**
*\brief <b>SNEP Protocol Client Request Get</b>.
* This function will perform GET action.
* \param[in] pDataParams       Pointer to this layer's parameter structure.
* \param[in] ConnHandle        Client Connection handle.
* \param[in] pGetData          Get Request Data
* \param[in] acceptable_length Acceptance Data Lenght
* \param[in] fCbGet            Notifier callback for Get Request
* \param[in] cbContext         Upper layer context to be returned in the callback.
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER       One or more of the supplied parameters
*                                         could not be properly interpreted.
* \retval #PH_ERR_PENDING                 GET operation is in progress.
* \retval #PH_ERR_INVALID_STATE           The socket is not in a valid state, or not of
*                                         a valid type to perform the requsted operation.
* \retval #PH_ERR_FAILED                  Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phnpSnep_ClientReqGet( void                    *pDataParams,
                                  ph_NfcHandle             ConnHandle,
                                  phNfc_sData_t           *pGetData,
                                  uint32_t                 acceptable_length,
                                  pphnpSnep_Fri_ReqCb_t    fCbGet,
                                  void                    *cbContext);

/**
*\brief <b>SNEP Send Response Get</b>.
* This function performs SNEP Server Send Response.
* \param[in] pDataParams       Pointer to this layer's parameter structure.
* \param[in]  ConnHandle       Connection handle to identify session uniquely.
* \param[in]  pResponseData    Data sent to client as reponse to a request.
* \param[in]  responseStatus   Response status.
* \param[in]  fSendCompleteCb  Response sent notifier callback function.
* \param[in]  cbContext        Context to be passed to callback function.
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER       One or more of the supplied parameters
*                                         could not be properly interpreted.
* \retval #PH_ERR_PENDING                 GET operation is in progress.
* \retval #PH_ERR_INVALID_STATE           The socket is not in a valid state, or not of
*                                         a valid type to perform the requsted operation.
* \retval #PH_ERR_FAILED                  Operation failed.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phnpSnep_ServerSendResponse( void             *pDataParams,
                                        ph_NfcHandle      ConnHandle,
                                        phNfc_sData_t    *pResponseData,
                                        phStatus_t        responseStatus,
                                        ph_NfcHandle      fSendCompleteCb,
                                        void             *cbContext
                                        );

/** @} */

#endif /* NXPBUILD__PHNP_SNEP_FRI */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHNPSNEP_H */
