/*
*         Copyright (c), NXP Semiconductors Bangalore / India
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

/** \file
* Generic LLCP Link layer Component of Reader Library Framework.
* $Author: nxp62726 $
* $Revision: 501 $
* $Date: 2014-02-13 12:24:52 +0530 (Thu, 13 Feb 2014) $
*
* History:
*  PC: Generated 23. Aug 2012
*
*/
#ifndef PHLNLLCP_H
#define PHLNLLCP_H

#include <ph_Status.h>

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

#ifdef NXPBUILD__PHLN_LLCP_FRI

/** \defgroup phlnLlcp_Fri Component : LLCP FRI
* \brief Forum Reference implementation of LLCP
*
* @{
*/

#define PHLN_LLCP_FRI_ID            0x01        /**< ID for LLCP FRI component */

/**
 *  \name NFC Types
 *
 *
 */
/*@{*/
#define PHLNLLCP_FRI_HAL_ATQA_LENGTH           0x02U       /**< ATQA length */
#define PHLNLLCP_FRI_HAL_MAX_UID_LENGTH        0x0AU       /**< Maximum UID length expected */
#define PHLNLLCP_FRI_HAL_MAX_ATR_LENGTH        0x30U       /**< Maximum ATR_RES (General Bytes) */
/*@}*/

/**
 *  \name NFC error status
 *
 *
 */
/*@{*/
#define PHLNLLCP_FRI_NFCSTATUS_REJECTED             (0x0093)       /**< Rejected, Previous callback is pending */
#define PHLNLLCP_FRI_NFCSTATUS_ABORTED              (0x0096)       /**< Aborted */

#ifndef PHLNLLCP_FRI_NFCSTATUS_NOT_ENOUGH_MEMORY
#define PHLNLLCP_FRI_NFCSTATUS_NOT_ENOUGH_MEMORY    (0x001F)       /**< Not enough memory */
#endif
/*@}*/

/**
 *  \name NFC-IP1 Device Types
 *
 *
 */
/*@{*/
#define    PHLNLLCP_FRI_HAL_ENFCIP1_TARGET           phlnLlcp_Fri_eNfcIP1_Target     /**< Target device */
#define    PHLNLLCP_FRI_HAL_ENFCIP1_INITIATOR        phlnLlcp_Fri_eNfcIP1_Initiator  /**< Initiator device */
/*@}*/

/**
 *  \name LLCP default parameters.
 *
 *  Definitions for use when wanting to use default LLCP parameter values.
 *
 */
 /*@{*/
#define PHLNLLCP_FRI_MIU_DEFAULT         128U  /**< Default MIU value (in bytes).*/
#define PHLNLLCP_FRI_WKS_DEFAULT         1    /**< Default WKS value (bitfield).*/
#define PHLNLLCP_FRI_LTO_DEFAULT         10   /**< Default LTO value (in step of 10ms).*/
#define PHLNLLCP_FRI_RW_DEFAULT          1    /**< Default RW value (in frames).*/
#define PHLNLLCP_FRI_OPTION_DEFAULT      0    /**< Default OPTION value (in frames).*/
#define PHLNLLCP_FRI_MIUX_DEFAULT        0    /**< Default MIUX value (in bytes) */
#define PHLNLLCP_FRI_SN_MAX_LENGTH       255  /**< Max length value for the Service Name */
#define PHLNLLCP_FRI_RW_MAX              15   /**< Max RW value (in frames).*/
/*@}*/

/**
 *  \name LLCP config parameters.
 *
 *  Definitions used for internal LLCP configuration.
 *
 */
 /*@{*/
#define PHLNLLCP_FRI_NB_SOCKET_MAX          2         /**< Max.number of simultaneous sockets */
#define PHLNLLCP_FRI_SNL_RESPONSE_MAX       255       /**< Max.number of simultaneous discovery requests */
#define PHLNLLCP_FRI_MIU_MAX                248       /**< Miu max */
/*@}*/

/**
 * \name Fixed value for ERROR opcode.
 *
 */
 /*@{*/
#define PHLNLLCP_FRI_ERR_DISCONNECTED                0x00      /**< Disconnection error. */
#define PHLNLLCP_FRI_ERR_FRAME_REJECTED              0x01      /**< Frame rejected notification error. */
#define PHLNLLCP_FRI_ERR_BUSY_CONDITION              0x02      /**< Busy condition notification. */
#define PHLNLLCP_FRI_ERR_NOT_BUSY_CONDITION          0x03      /**< Not busy condition notification. */
/*@}*/

/**
 * \name Fixed value for DM opcode.
 *
 */
 /*@{*/
#define PHLNLLCP_FRI_DM_OPCODE_DISCONNECTED            0x00       /**< Disconnected opcode for DM mode. */
#define PHLNLLCP_FRI_DM_OPCODE_SAP_NOT_ACTIVE          0x01       /**< Service Access Point not active. */
#define PHLNLLCP_FRI_DM_OPCODE_SAP_NOT_FOUND           0x02       /**< Service Access Point not found. */
#define PHLNLLCP_FRI_DM_OPCODE_CONNECT_REJECTED        0x03       /**< Connection rejected. */
#define PHLNLLCP_FRI_DM_OPCODE_CONNECT_NOT_ACCEPTED    0x20       /**< Connection not accepted. */
#define PHLNLLCP_FRI_DM_OPCODE_SOCKET_NOT_AVAILABLE    0x21       /**< Socket not available. */
#define PHLNLLCP_FRI_SDP_ADVERTISED_NB                 0x10       /**< Number of SDP advertised SAP slots. */
/*@}*/

/**
*  \name LLCP packet types.
*
*/
/*@{*/
#define PHLNLLCP_FRI_PTYPE_SYMM        0x00 /**< Symmetry.*/
#define PHLNLLCP_FRI_PTYPE_PAX         0x01 /**< PArameter Exchange.*/
#define PHLNLLCP_FRI_PTYPE_AGF         0x02 /**< AGgregated Frame.*/
#define PHLNLLCP_FRI_PTYPE_UI          0x03 /**< Unnumbered Information.*/
#define PHLNLLCP_FRI_PTYPE_CONNECT     0x04 /**< Connect.*/
#define PHLNLLCP_FRI_PTYPE_DISC        0x05 /**< Disconnect.*/
#define PHLNLLCP_FRI_PTYPE_CC          0x06 /**< Connection Complete.*/
#define PHLNLLCP_FRI_PTYPE_DM          0x07 /**< Disconnected Mode.*/
#define PHLNLLCP_FRI_PTYPE_FRMR        0x08 /**< FRaMe Reject.*/
#define PHLNLLCP_FRI_PTYPE_SNL         0x09 /**< Service Name Lookup.*/
#define PHLNLLCP_FRI_PTYPE_RESERVED1   0x0A /**< Reserved.*/
#define PHLNLLCP_FRI_PTYPE_RESERVED2   0x0B /**< Reserved.*/
#define PHLNLLCP_FRI_PTYPE_I           0x0C /**< Information.*/
#define PHLNLLCP_FRI_PTYPE_RR          0x0D /**< Receive Ready.*/
#define PHLNLLCP_FRI_PTYPE_RNR         0x0E /**< Receive Not Ready.*/
#define PHLNLLCP_FRI_PTYPE_RESERVED3   0x0F /**< Reserved.*/
/*@}*/

/**
*
*  \brief NFCIP1 Data rates
*
*/
typedef enum phlnLlcp_Fri_eDataRate
{
    phlnLlcp_Fri_eDataRate_106 = 0x00U,      /**< Data rate setting for 106 kBit/s. */
    phlnLlcp_Fri_eDataRate_212,              /**< Data rate setting for 212 kBit/s. */
    phlnLlcp_Fri_eDataRate_424,              /**< Data rate setting for 424 kBit/s. */
    phlnLlcp_Fri_eDataRate_RFU               /**< RFU. */
} phlnLlcp_Fri_eDataRate_t;

/**
*
*  \brief RF Device Type Listing
*
*  The <em> RF Device Type List </em> is used to identify the type of
*  remote device that is discovered/connected. There separate
*  types to identify a Remote Reader (denoted by _PCD) and
*  Remote Tag (denoted by _PICC)
*  \note None.
*
*/
typedef enum phlnLlcp_Fri_eRFDevType
{
    phlnLlcp_Fri_eUnknown_DevType = 0x00U,       /**< Unknown device type */
    phlnLlcp_Fri_eISO14443_A_PCD,                /**< ISO/IEC 14443-A PCD */
    phlnLlcp_Fri_eISO14443_B_PCD,                /**< ISO/IEC 14443-B PCD */
    phlnLlcp_Fri_eISO14443_BPrime_PCD,           /**< ISO/IEC 14443 B-Prime PCD */
    phlnLlcp_Fri_eFelica_PCD,                    /**< Felica PCD */
    phlnLlcp_Fri_eJewel_PCD,                     /**< Jewel PCD */
    phlnLlcp_Fri_eISO15693_PCD,                  /**< ISO/IEC 15693 PCD */
    phlnLlcp_Fri_ePCD_DevType,                   /**< Generic PCD */
    phlnLlcp_Fri_ePICC_DevType,                  /**< Generic PICC */
    phlnLlcp_Fri_eISO14443_A_PICC,               /**< ISO/IEC 14443-A PICC */
    phlnLlcp_Fri_eISO14443_4A_PICC,              /**< ISO/IEC 14443-4A PICC */
    phlnLlcp_Fri_eISO14443_3A_PICC,              /**< ISO/IEC 14443-3A PICC */
    phlnLlcp_Fri_eMifare_PICC,                   /**< Mifare PICC */
    phlnLlcp_Fri_eISO14443_B_PICC,               /**< ISO/IEC 14443-B PICC */
    phlnLlcp_Fri_eISO14443_4B_PICC,              /**< ISO/IEC 14443-4B PICC */
    phlnLlcp_Fri_eISO14443_BPrime_PICC,          /**< ISO/IEC 14443 B-Prime PICC */
    phlnLlcp_Fri_eFelica_PICC,                   /**< Felica PICC */
    phlnLlcp_Fri_eJewel_PICC,                    /**< ISO/IEC Jewel PICC */
    phlnLlcp_Fri_eISO15693_PICC,                 /**< ISO/IEC 15693 PICC */
    phlnLlcp_Fri_eNfcIP1_Target,                 /**< Target device */
    phlnLlcp_Fri_eNfcIP1_Initiator,              /**< Initiator device */
    phlnLlcp_Fri_eInvalid_DevType                /**< Invalid device type */
} phlnLlcp_Fri_eRFDevType_t;

/**
*
*  \brief RF Device Type Listing
*
*/
typedef phlnLlcp_Fri_eRFDevType_t phlnLlcp_Fri_eRemDevType_t;

/**
*
*  \brief Socket types
*
*/
typedef enum phlnLlcp_Fri_Transport_eSocketType
{
   phlnLlcp_Fri_Transport_eDefaultType,             /**< Default communication type */
   phlnLlcp_Fri_Transport_eConnectionOriented,      /**< Connection-oriented communication */
   phlnLlcp_Fri_Transport_eConnectionLess           /**< Connection-less communication */
}phlnLlcp_Fri_Transport_eSocketType_t;

/**
*
*  \brief LLCP Link state
*
*/
typedef enum phlnLlcp_Fri_TransportSocket_eSocketState
{
   phlnLlcp_Fri_TransportSocket_eSocketDefault,         /**< Default state */
   phlnLlcp_Fri_TransportSocket_eSocketCreated,         /**< Socket created state */
   phlnLlcp_Fri_TransportSocket_eSocketBound,           /**< Socket bound state */
   phlnLlcp_Fri_TransportSocket_eSocketRegistered,      /**< Socket registered state */
   phlnLlcp_Fri_TransportSocket_eSocketConnected,       /**< Socket connected state */
   phlnLlcp_Fri_TransportSocket_eSocketConnecting,      /**< Socket connecting state */
   phlnLlcp_Fri_TransportSocket_eSocketAccepted,        /**< Socket accepted state */
   phlnLlcp_Fri_TransportSocket_eSocketDisconnected,    /**< Socket disconnected state */
   phlnLlcp_Fri_TransportSocket_eSocketDisconnecting,   /**< Socket disconnecting state */
   phlnLlcp_Fri_TransportSocket_eSocketRejected,        /**< Socket rejected state */
}phlnLlcp_Fri_TransportSocket_eSocketState_t;

/**
*
*  \brief LLCP Link status
*
*/
typedef enum phlnLlcp_Fri_Mac_eLinkStatus
{
   phlnLlcp_Fri_Mac_eLinkDefault,          /**< Default link status */
   phlnLlcp_Fri_Mac_eLinkActivated,        /**< Link activated */
   phlnLlcp_Fri_Mac_eLinkDeactivated       /**< Link deactivated */
}phlnLlcp_Fri_Mac_eLinkStatus_t;

/**
*
*  \brief LLCP Link status
*
*/
typedef phlnLlcp_Fri_Mac_eLinkStatus_t    phlnLlcp_Fri_eLinkStatus_t;

/**
*
*  \brief MAC mapping/types
*
*/
typedef enum phlnLlcp_Fri_Mac_eType
{
   phlnLlcp_Fri_Mac_eTypeNfcip,           /**< NFCIP */
   phlnLlcp_Fri_Mac_eTypeIso14443         /**< ISO/IEC 14444 */
}phlnLlcp_Fri_Mac_eType_t;

/**
*
*  \brief LLCP P2P types
*
*/
typedef enum phlnLlcp_Fri_Mac_ePeerType
{
   phlnLlcp_Fri_Mac_ePeerTypeInitiator,      /**< Peer Type Initiator */
   phlnLlcp_Fri_Mac_ePeerTypeTarget          /**< Peer Type Target */
}phlnLlcp_Fri_Mac_ePeerType_t;

/**
*
*  \brief LLCP P2P types
*
*/

/* Forward declarations */
typedef phlnLlcp_Fri_Mac_ePeerType_t      phlnLlcp_Fri_eRole_t;

struct phlnLlcp_Fri_Mac;
typedef struct phlnLlcp_Fri_Mac phlnLlcp_Fri_Mac_t;

struct phlnLlcp_Fri_Transport_Socket;
typedef struct phlnLlcp_Fri_Transport_Socket phlnLlcp_Fri_Transport_Socket_t;

struct phlnLlcp_Fri_Transport;
typedef struct phlnLlcp_Fri_Transport phlnLlcp_Fri_Transport_t;


/**
 * \brief Data Buffer Structure to hold the Data Buffer
 *
 * This structure holds the Data in the Buffer of the specified
 * size.
 *
 */
typedef struct phNfc_sData
{
    uint8_t      *buffer;      /**< Data buffer*/
    uint32_t      length;      /**< Data length*/
}phNfc_sData_t;

/**
*
*  \brief NFCIP1 Gate Information Container
*
*  The <em> NFCIP1 structure </em> includes the available information
*  related to the discovered NFCIP1 remote device. This information
*  is updated for every device discovery.
*  \note None.
*
*/
typedef struct phlnLlcp_Fri_sNfcIPInfo
{
    uint8_t         NFCID[PHLNLLCP_FRI_HAL_MAX_UID_LENGTH];       /**< NFCID3 */
    uint8_t         NFCID_Length;                                 /**< NFCID length */
    uint8_t         ATRInfo[PHLNLLCP_FRI_HAL_MAX_ATR_LENGTH];     /**< ATR */
    uint8_t         ATRInfo_Length;                               /**< ATR length */
    uint8_t         SelRes;                                       /**< SAK information of the tag discovered */
    uint8_t         SenseRes[PHLNLLCP_FRI_HAL_ATQA_LENGTH];       /**< ATQA information of the tag discovered */
    uint8_t         Nfcip_Active;                                 /**< Is Detection Mode of the NFCIP Target Active */
    uint16_t        MaxFrameLength;                               /**< Maximum frame length supported by the NFCIP device */
    phlnLlcp_Fri_eDataRate_t         Nfcip_Datarate;              /**< Data rate supported by the NFCIP device */
} phlnLlcp_Fri_sNfcIPInfo_t;

/**
*
*  \brief Remote Device Specific Information Container
*
*  The <em> Remote Device Information Union </em> includes the available Remote Device Information
*  structures. Following the device detected, the corresponding data structure is used.
*
*
*/
typedef union phlnLlcp_Fri_uRemoteDevInfo
{
    phlnLlcp_Fri_sNfcIPInfo_t              NfcIP_Info;     /**< NFCIP1 remote device information */
} phlnLlcp_Fri_uRemoteDevInfo_t;

/**
 *
 *  \brief Remote Device Information Structure
 *
 *  The <em> Remote Device Information Structure </em> holds information about one single Remote
 *  Device detected by the polling function .\n
 *  It lists parameters common to all supported remote devices.
 *
 *
 *
 */
typedef struct phlnLlcp_Fri_sRemoteDevInformation
{
    uint8_t                           SessionOpened;      /**< [out] Boolean flag indicating the validity of the handle of the remote device. */
    phlnLlcp_Fri_eRemDevType_t        RemDevType;         /**< [out] Remote device type which says that remote is Reader A or Reader B or NFCIP or Felica or Reader B Prime or Jewel*/
    phlnLlcp_Fri_uRemoteDevInfo_t     RemoteDevInfo;      /**< Union of available Remote Device. \ref phlnLlcp_Fri_uRemoteDevInfo_t Information.  */
} phlnLlcp_Fri_sRemoteDevInformation_t;

typedef phlnLlcp_Fri_sRemoteDevInformation_t phHal_sRemoteDevInformation_t;

/**
*
*
* \brief LLCP socket option parameter structure
*/
typedef struct phlnLlcp_Fri_Transport_sSocketOptions
{
    uint8_t    rw;      /**< The Receive Window size (4 bits)*/
    uint16_t   miu;     /**< The remote Maximum Information Unit Extension (NOTE: this is MIUX, not MIU !)*/
}phlnLlcp_Fri_Transport_sSocketOptions_t;


/**
*
*
* \brief LLCP completion routine callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_Cr_t)(void * pContext, phStatus_t status);

/**
*
*
* \brief LLCP compliance checking callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*phlnLlcp_Fri_Check_CB_t) (void *pContext, phStatus_t  status);

/**
*
*
* \brief LLCP link status notification callback definition
* \param[in] pContext       Context.
* \param[in] eLinkStatus    Link status.
*/
typedef void (*phlnLlcp_Fri_LinkStatus_CB_t) (void *pContext, phlnLlcp_Fri_eLinkStatus_t eLinkStatus);

/**
*
*
* \brief LLCP link send callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*phlnLlcp_Fri_Send_CB_t) (void *pContext, phStatus_t status);

/**
*
*
* \brief LLCP link receive callback definition
* \param[in] pContext       Context.
* \param[in] psData         Received data.
* \param[in] status         Status of the response  callback.
*/
typedef void (*phlnLlcp_Fri_Recv_CB_t) (void *pContext, phNfc_sData_t *psData, phStatus_t status);

/**
*
*
* \brief LLCP socket error notification callback definition
* \param[in] pContext       Context.
* \param[in] nErrCode       Error code.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketErrCb_t) ( void* pContext, uint8_t nErrCode);

/**
*
*
* \brief LLCP socket error notification callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketErrCb_t phlnLlcp_Fri_TransportSocketErrCb_t;


/**
*
*
* \brief LLCP socket listen callback definition
* \param[in] pContext           Context.
* \param[in] IncomingSocket     Incoming socket.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketListenCb_t) (void* pContext, phlnLlcp_Fri_Transport_Socket_t *IncomingSocket);

/**
*
*
* \brief LLCP socket listen callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketListenCb_t phlnLlcp_Fri_TransportSocketListenCb_t;

/**
*
*
* \brief LLCP socket connect callback definition
* \param[in] pContext       Context.
* \param[in] nErrCode       Error code.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketConnectCb_t)  ( void* pContext, uint8_t nErrCode, phStatus_t status);

/**
*
*
* \brief LLCP socket connect callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketConnectCb_t phlnLlcp_Fri_TransportSocketConnectCb_t;

/**
*
*
* \brief LLCP socket disconnect callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketDisconnectCb_t) (void* pContext, phStatus_t status);

/**
*
*
* \brief LLCP socket accept callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketAcceptCb_t) (void* pContext, phStatus_t status);

/**
*
*
* \brief LLCP socket accept callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketAcceptCb_t phlnLlcp_Fri_TransportSocketAcceptCb_t;

/**
*
*
* \brief LLCP socket reject callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketRejectCb_t) (void* pContext, phStatus_t status);

/**
*
*
* \brief LLCP socket reject callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketRejectCb_t phlnLlcp_Fri_TransportSocketRejectCb_t;

/**
*
*
* \brief LLCP socket reception callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketRecvCb_t) (void* pContext, phStatus_t status);

/**
*
*
* \brief LLCP socket reception callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketRecvCb_t phlnLlcp_Fri_TransportSocketRecvCb_t;

/**
*
*
* \brief LLCP socket reception with SSAP callback definition
* \param[in] pContext       Context.
* \param[in] ssap           Source service access point.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketRecvFromCb_t) (void* pContext, uint8_t ssap, phStatus_t  status);

/**
*
*
* \brief LLCP socket reception with SSAP callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketRecvFromCb_t phlnLlcp_Fri_TransportSocketRecvFromCb_t;

/**
*
*
* \brief LLCP socket emission callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*pphlnLlcp_Fri_TransportSocketSendCb_t) (void* pContext, phStatus_t status);

/**
*
*
* \brief LLCP socket emission callback definition
*/
typedef pphlnLlcp_Fri_TransportSocketSendCb_t phlnLlcp_Fri_TransportSocketSendCb_t;

/**
*
*
* \brief MAC chk callback definition
* \param[in] pContext       Context.
* \param[in] status         Status of the response  callback.
*/
typedef void (*phlnLlcp_Fri_Mac_Chk_CB_t) (void *pContext, phStatus_t status);

/**
*
*
* \brief MAC link status callback definition
* \param[in] pContext           Context.
* \param[in] status             Status of the response callback.
* \param[in] eLinkStatus        Link status.
* \param[in] PeerRemoteDevType  Remote device type.
*/
typedef void (*phlnLlcp_Fri_Mac_LinkStatus_CB_t) ( void *                           pContext,
                                                   phlnLlcp_Fri_Mac_eLinkStatus_t   eLinkStatus,
                                                   phNfc_sData_t                    *psData,
                                                   phlnLlcp_Fri_Mac_ePeerType_t     PeerRemoteDevType
                                                  );

/**
*
*
* \brief MAC send callback definition
* \param[in] pContext           Context.
* \param[in] status             Status of the response callback.
*/
typedef void (*phlnLlcp_Fri_Mac_Send_CB_t) (void *pContext, phStatus_t status);

/**
*
*
* \brief MAC receive callback definition
* \param[in] pContext           Context.
* \param[in] status             Status of the response callback.
* \param[in] psData             Received data.
*/
typedef void (*phlnLlcp_Fri_Mac_Reveive_CB_t)( void               *pContext,
                                               phStatus_t         status,
                                               phNfc_sData_t      *psData
                                              );

/**
 *
 *
 * \brief Completion Routine structure
 *
 * This structure finds itself within each component that requires to report completion
 * to an upper (calling) component.\n\n
 * Depending on the actual implementation (static or dynamic completion information) the stack
 * Initialization \b or the calling component needs to inform the initialized \b or called component
 * about the completion path. This information is submitted via this structure.
 *
 */
typedef struct phlnLlcp_Fri_CplRt
{
    pphlnLlcp_Fri_Cr_t  CompletionRoutine;/*!< Address of the upper Layer's \b Process(ing) function to call upon completion.
                                          *   The stack initializer (or depending on the implementation: the calling component)
                                          *   needs to set this member to the address of the function that needs to be within
                                          *   the completion path: A calling component would give its own processing function
                                          *   address to the lower layer.
                                          */
    void                *Context;         /*!< Instance address (context) parameter.
                                          *   The stack initializer (or depending on the implementation: the calling component)
                                          *   needs to set this member to the address of the component context structure instance
                                          *   within the completion path: A calling component would give its own instance address
                                          *   to the lower layer.
                                          */
} phlnLlcp_Fri_CplRt_t;

typedef enum ovrHalAction
{
    transmit = 0,     /**< Hal transmit Action */
    receive,          /**< Hal receive Action */
    transceive        /**< Hal transceive Action */
}ovrHalAction_t;

typedef struct
{
    uint8_t                        *SendBuf;           /**< Pointer to the data to send Buffer */
    uint8_t                        **RecvBuf;          /**< Pointer to the data to receive Buffer */
    uint16_t                       SendLength;         /**< Length of Send data */
    uint16_t                       *RecvLength;        /**< Length of Recv data */
    ovrHalAction_t                 bAction;            /**< Type of action */
    uint32_t                       timerID;            /**< Timer Id  */
    phlnLlcp_Fri_CplRt_t           *CompletionInfo;    /**< Completion Routine Info */
    void                           *ovrHal;            /**< Ovrhal holder */
    void                           *Osal;              /**< Osal component holder */
} phlnLlcp_Fri_OvrHalTask_t;

/**
*
*
* \brief MAC chk function definition for interface with lower layer
* \param[in] pContext           Context.
* \param[in] ChkLlcpMac_Cb      MAC Chk callback.
* \param[in] LlcpMac            Pointer to MAC structure.
*/
typedef phStatus_t (*pphlnLlpc_Fri_Mac_Chk_t) ( phlnLlcp_Fri_Mac_t               *LlcpMac,
                                                phlnLlcp_Fri_Mac_Chk_CB_t        ChkLlcpMac_Cb,
                                                void                             *pContext
                                               );
/**
*
*
* \brief MAC activate function definition for interface with lower layer
* \param[in] LlcpMac            Pointer to MAC structure.
*/
typedef phStatus_t (*pphlnLlpc_Fri_Mac_Activate_t) (phlnLlcp_Fri_Mac_t  *LlcpMac);

/**
*
*
* \brief MAC deactivate function definition for interface with lower layer
* \param[in] LlcpMac            Pointer to MAC structure.
*/
typedef phStatus_t (*pphlnLlpc_Fri_Mac_Deactivate_t) (phlnLlcp_Fri_Mac_t  *LlcpMac);

/**
*
*
* \brief MAC send function definition for interface with lower layer
* \param[in] pContext           Context.
* \param[in] ChkLlcpMac_Cb      MAC Chk callback.
* \param[in] LlcpMac            Pointer to MAC structure.
* \param[out] psData            Send data.
*/
typedef phStatus_t (*pphlnLlpc_Fri_Mac_Send_t)( phlnLlcp_Fri_Mac_t             *LlcpMac,
                                                phNfc_sData_t                  *psData,
                                                phlnLlcp_Fri_Mac_Send_CB_t     LlcpMacSend_Cb,
                                                void                           *pContext
                                               );
/**
*
*
* \brief MAC receive function definition for interface with lower layer
* \param[in] pContext           Context.
* \param[in] ChkLlcpMac_Cb      MAC Chk callback.
* \param[in] LlcpMac            Pointer to MAC structure.
* \param[out] psData            Received data.
*/
typedef phStatus_t (*pphlnLlpc_Fri_Mac_Receive_t)( phlnLlcp_Fri_Mac_t               *LlcpMac,
                                                   phNfc_sData_t                    *psData,
                                                   phlnLlcp_Fri_Mac_Reveive_CB_t    LlcpMacReceive_Cb,
                                                   void                             *pContext
                                                  );

/**
*
*
*\brief LLCP link status. Refer to \ref phlnLlcp_Fri_Mac_eLinkStatus_t
*
*/
typedef phlnLlcp_Fri_Transport_eSocketType_t  phlnLlcp_Transport_eSocketType_t;

/**
*
*
* \brief LLCP socket disconnect callback definition
* \param[in] pContext           Context.
* \param[in] status             Status of the response callback.
*/
typedef void (*pphlnLlcp_Fri_LlcpSocketDisconnectCb_t) (void* pContext, phStatus_t status);

/**
*
*
* \brief LLCP socket disconnect callback definition
*/
typedef pphlnLlcp_Fri_LlcpSocketDisconnectCb_t phlnLlcp_SocketDisconnectCb_t;

/**
*
*
* \brief LLCP socket option parameter structure
*/
typedef phlnLlcp_Fri_Transport_sSocketOptions_t phlnLlcp_Fri_sSocketOptions_t;

/**
*
*
* \brief LLCP link parameter structure
*/
typedef struct phlnLlcp_Fri_sLinkParameters
{
    uint8_t    lto;        /**< The remote Link TimeOut (in 1/100s)*/
    uint8_t    option;     /**< The remote options*/
    uint16_t   miu;        /**< The remote Maximum Information Unit (NOTE: this is MIU, not MIUX !)*/
    uint16_t   wks;        /**< The remote Well-Known Services*/

} phlnLlcp_Fri_sLinkParameters_t;

/**
 * phlnLlcp_Fri_Util_Fifo_Buffer - A Cyclic FIFO buffer
 * If pIn == pOut the buffer is empty.
 */
typedef struct phlnLlcp_Fri_Util_Fifo_Buffer
{
   uint8_t          *pBuffStart;                    /**< Points to first valid location in buffer */
   uint8_t          *pBuffEnd;                      /**< Points to last valid location in buffer */
   volatile uint8_t *pIn;                           /**< Points to 1 before where the next TU1 will enter buffer */
   volatile uint8_t *pOut;                          /**< Points to 1 before where the next TU1 will leave buffer */
   volatile uint8_t  bFull;                         /**< TRUE if buffer is full */
}phlnLlcp_Fri_Util_Fifo_Buffer, *pphlnLlcp_Fri_Util_Fifo_Buffer;

/**
 *
 * \brief Generic Interface structure with the Lower Layer
 */
typedef struct phlnLlcp_Fri_Mac_Interface
{
   pphlnLlpc_Fri_Mac_Chk_t              chk;            /**< Chk function */
   pphlnLlpc_Fri_Mac_Activate_t         activate;       /**< Activate function */
   pphlnLlpc_Fri_Mac_Deactivate_t       deactivate;     /**< Deactivate function */
   pphlnLlpc_Fri_Mac_Send_t             send;           /**< Send function */
   pphlnLlpc_Fri_Mac_Receive_t          receive;        /**< Receive function */
} phlnLlcp_Fri_Mac_Interface_t;

/**
 *
 * \brief Definition of the MAC type
 */
struct phlnLlcp_Fri_Mac
{
    uint8_t                            RecvPending;           /**< Receive pending flag */
    uint8_t                            SendPending;           /**< Send pending flag */
    phlnLlcp_Fri_Mac_eLinkStatus_t     LinkState;             /**< Link state */
    phlnLlcp_Fri_Mac_ePeerType_t       PeerRemoteDevType;     /**< P2P device type (Target or Initiator) */
    phlnLlcp_Fri_Mac_LinkStatus_CB_t   LinkStatus_Cb;         /**< Link status callback */
    phlnLlcp_Fri_Mac_Send_CB_t         MacSend_Cb;            /**< Send callback */
    phlnLlcp_Fri_Mac_Reveive_CB_t      MacReceive_Cb;         /**< Receive callback */
    phNfc_sData_t                     *psReceiveBuffer;       /**< Receive buffer */
    phNfc_sData_t                     *psSendBuffer;          /**< Send buffer */
    phHal_sRemoteDevInformation_t     *psRemoteDevInfo;       /**< Remote device information */
    void                              *MacReceive_Context;    /**< Receive callback context */
    void                              *LowerDevice;           /**< Holds the completion routine informations of the Map Layer */
    void                              *MacSend_Context;       /**< Send callback context */
    void                              *LinkStatus_Context;    /**< Link status callback context */
    void                              *Osal;                  /**< Pointer to OSAL layer */
    phNfc_sData_t                      sConfigParam;          /**< Buffer for configuration parameter */
    phlnLlcp_Fri_CplRt_t               MacCompletionInfo;     /**< Completion Routine */
    phlnLlcp_Fri_Mac_Interface_t       LlcpMacInterface;      /**< MAC interface functions */
};

/**
 *
 * \brief Cached service name
 */
typedef struct phlnLlcp_Fri_CachedServiceName
{
    uint8_t             nSap;               /**< Service access point */
    phNfc_sData_t       sServiceName;       /**< Service name */

}phlnLlcp_Fri_CachedServiceName_t;

/**
 *
 * \brief Packet header structure
 */
typedef struct phlnLlcp_Fri_sPacketHeader
{
    uint8_t    dsap;                 /**< The destination service access point */
    uint8_t    ptype;                /**< The packet type */
    uint8_t    ssap;                 /**< The source service access point */
} phlnLlcp_Fri_sPacketHeader_t;

/**
 *
 * \brief Packet sequence structure
 */
typedef struct phlnLlcp_Fri_sPacketSequence
{
    uint8_t   ns;          /**< Sequence number for sending */
    uint8_t   nr;          /**< Sequence number for reception */
} phlnLlcp_Fri_sPacketSequence_t;

typedef struct phlnLlcp_Fri_sSendOperation
{
    phlnLlcp_Fri_sPacketHeader_t     *psHeader;       /**< Packet header */
    phlnLlcp_Fri_sPacketSequence_t   *psSequence;     /**< Packet sequence number for sending and reception */
    phNfc_sData_t                    *psInfo;         /**< Data buffer */
    phlnLlcp_Fri_Send_CB_t            pfSend_CB;      /**< Send callback */
    void                             *pContext;       /**< Upper layer context */
} phlnLlcp_Fri_sSendOperation_t;

typedef struct phlnLlcp_Fri_sRecvOperation
{
    uint8_t                     nSap;        /**< Service access point */
    phNfc_sData_t              *psBuffer;    /**< Data buffer */
    phlnLlcp_Fri_Recv_CB_t      pfRecv_CB;   /**< Receive callback */
    void                       *pContext;    /**< Upper layer context */

} phlnLlcp_Fri_sRecvOperation_t;

/**
* \brief LLCP parameter structure
*/
typedef struct phlnLlcp_Fri
{
    uint8_t                             state;                     /**< The current state */
    uint8_t                             nSymmetryCounter;          /**< Activity counter, used to modulate symmetry timeout */
    uint8_t                             version;                   /**< Negotiated protocol version (major number on MSB, minor on LSB) */
    uint8_t                             pFrmrInfo[4];              /**< Info field of pending FRMR packet */
    uint8_t                             pCtrlTxBuffer[10];         /**< Control frames buffer */
    uint8_t                             pCtrlTxBufferLength;       /**< Control frames buffer size */
    uint8_t                             bDiscPendingFlag;          /**< DISC packet send pending flag */
    uint8_t                             bFrmrPendingFlag;          /**< FRMR packet send pending flag */
    uint16_t                            nRxBufferLength;           /**< Actual size of reception buffer */
    uint16_t                            nTxBufferLength;           /**< Actual size of emission buffer */
    phlnLlcp_Fri_sPacketHeader_t        sFrmrHeader;               /**< Header of pending FRMR packet */
    phlnLlcp_Fri_Mac_ePeerType_t        eRole;                     /**< Local LLC role */
    uint32_t                            hSymmTimer;                /**< Symmetry timer */
    phlnLlcp_Fri_Recv_CB_t              pfRecvCB;                  /**< Receive callback */
    uint8_t                            *pRxBuffer;                 /**< base pointer buffer for sRxBuffer */
    phlnLlcp_Fri_sPacketHeader_t       *psSendHeader;              /**< Pending send header */
    phlnLlcp_Fri_sPacketSequence_t     *psSendSequence;            /**< Pending send sequence */
    phNfc_sData_t                      *psSendInfo;                /**< Pending send info */
    void                               *pLinkContext;              /**< Callback context for link status notification */
    void                               *pChkContext;               /**< Callback context for compliance checking */
    void                               *pSendContext;              /**< Send callback */
    void                               *pRecvContext;              /**< Receive callback */
    void                               *osal;                      /**< Pointer to OSAL layer */
    phlnLlcp_Fri_sLinkParameters_t      sLocalParams;              /**< Local link parameters */
    phlnLlcp_Fri_sLinkParameters_t      sRemoteParams;             /**< Remote link parameters */
    phNfc_sData_t                       sRxBuffer;                 /**< Internal reception buffer, its size may vary during time but not exceed nRxBufferSize */
    phNfc_sData_t                       sTxBuffer;                 /**< Internal emission buffer, its size may vary during time but not exceed nTxBufferSize */
    phlnLlcp_Fri_LinkStatus_CB_t        pfLink_CB;                 /**< Callback function for link status notification */
    phlnLlcp_Fri_Check_CB_t             pfChk_CB;                  /**< Callback function for compliance checking */
    phlnLlcp_Fri_Send_CB_t              pfSendCB;                  /**< Send callback */
    phlnLlcp_Fri_Mac_t                  MAC;                       /**< MAC mapping instance */
} phlnLlcp_Fri_t;

/**
 *
 * \brief LLCP socket parameter structure
 */
struct phlnLlcp_Fri_Transport_Socket
{
    uint8_t                                        socket_sSap;                   /**< Source service access point*/
    uint8_t                                        socket_dSap;                   /**< Destination service access point*/
    uint8_t                                        remoteRW;                      /**< Remote receive window */
    uint8_t                                        localRW;                       /**< Local receive window */
    uint8_t                                        index;                         /**< Socket index */
    uint8_t                                        nTid;                          /**< Transaction identifier */
    uint8_t                                        bSocketRecvPending;            /**< Receive pending flag */
    uint8_t                                        bSocketSendPending;            /**< Send pending flag */
    uint8_t                                        bSocketListenPending;          /**< Listen pending flag */
    uint8_t                                        bSocketDiscPending;            /**< Disconnect (DISC) frame pending flag */
    uint8_t                                        bSocketConnectPending;         /**< Connect pending flag */
    uint8_t                                        bSocketAcceptPending;          /**< Accept pending flag */
    uint8_t                                        bSocketRRPending;              /**< Receive Ready (RR) frame pending flag */
    uint8_t                                        bSocketRNRPending;             /**< Receive Not Ready (RNR) frame pending flag */
    uint8_t                                        RemoteBusyConditionInfo;       /**< Remote busy condition flag */
    uint8_t                                        ReceiverBusyCondition;         /**< Receive busy condition flag */
    uint8_t                                        socket_VS;                     /**< Socket Send State Variable */
    uint8_t                                        socket_VSA;                    /**< Socket Send Acknowledgment State Variable */
    uint8_t                                        socket_VR;                     /**< Socket Receive State Variable */
    uint8_t                                        socket_VRA;                    /**< Socket Receive Acknowledgment State Variable */
    uint8_t                                        bRecvReady_NotRequired;        /**< Flag for Receive Ready PDU Required/Not */
    phlnLlcp_Fri_sPacketSequence_t                 sSequence;                     /**< Packet sequence */
    phlnLlcp_Fri_sPacketHeader_t                   sLlcpHeader;                   /**< Packet header */
    phlnLlcp_Fri_Transport_sSocketOptions_t        sSocketOption;                 /**< Socket option */
    uint16_t                                       remoteMIU;                     /**< Remote Maximum Information Unit */
    uint16_t                                       localMIUX;                     /**< Local Maximum Information Unit Extension */
    uint32_t                                       bufferLinearLength;            /**< Linear buffer length */
    uint32_t                                       bufferSendMaxLength;           /**< Maximum length for send window buffer */
    uint32_t                                       bufferRwMaxLength;             /**< Maximum length for receive window buffer */
    uint32_t                                       indexRwRead;                   /**< Receive window read index */
    uint32_t                                       indexRwWrite;                  /**< Receive window write index */
    phlnLlcp_Fri_Transport_t                      *psTransport;                   /**< Pointer to the Socket table */
    phNfc_sData_t                                 *sSocketRecvBuffer;             /**< Receive buffer */
    uint32_t                                      *receivedLength;                /**< Received length */
    void                                          *pListenContext;                /**< Listen callback context */
    void                                          *pAcceptContext;                /**< Accept callback context */
    void                                          *pRejectContext;                /**< Reject callback context */
    void                                          *pConnectContext;               /**< Connect callback context */
    void                                          *pDisonnectContext;             /**< Disconnect callback context */
    void                                          *pSendContext;                  /**< Send callback context */
    void                                          *pRecvContext;                  /**< Receive callback context */
    void                                          *pContext;                      /**< Upper layer context */
    phlnLlcp_Fri_TransportSocket_eSocketState_t    eSocket_State;                 /**< Current state */
    phlnLlcp_Fri_Transport_eSocketType_t           eSocket_Type;                  /**< Socket type */
    pphlnLlcp_Fri_TransportSocketErrCb_t           pSocketErrCb;                  /**< Error callback */
    pphlnLlcp_Fri_TransportSocketDisconnectCb_t    pfSocketDisconnect_Cb;         /**< Socket disconnect callback */
    phNfc_sData_t                                  sServiceName;                  /**< Service name */
    phNfc_sData_t                                  sSocketSendBuffer;             /**< Send buffer */
    phNfc_sData_t                                  sSocketLinearBuffer;           /**< Linear buffer */
    pphlnLlcp_Fri_TransportSocketAcceptCb_t        pfSocketAccept_Cb;             /**< Socket accept callback */
    pphlnLlcp_Fri_TransportSocketSendCb_t          pfSocketSend_Cb;               /**< Socket send callback */
    pphlnLlcp_Fri_TransportSocketRecvCb_t          pfSocketRecv_Cb;               /**< Socket reception callback */
    pphlnLlcp_Fri_TransportSocketListenCb_t        pfSocketListen_Cb;             /**< Socket listen callback */
    pphlnLlcp_Fri_TransportSocketConnectCb_t       pfSocketConnect_Cb;            /**< Socket connect callback */
    pphlnLlcp_Fri_TransportSocketRecvFromCb_t      pfSocketRecvFrom_Cb;           /**< Socket reception with SSAP callback */
    phlnLlcp_Fri_Util_Fifo_Buffer                  sCyclicFifoBuffer;             /**< Cyclic fifo buffer */
    phNfc_sData_t                                  sSocketRwBufferTable[PHLNLLCP_FRI_RW_MAX];    /**< Table of PHLNLLCP_FRI_RW_MAX Receive Windows Buffers */
};

/**
 * \brief Declaration of a TRANSPORT Type with a table of PHFRINFC_LLCP_NB_SOCKET_DEFAULT sockets
 *        and a pointer a Llcp layer
 */
 struct phlnLlcp_Fri_Transport
{
    uint8_t                                 bSendPending;                                           /**< Send pending flag */
    uint8_t                                 bRecvPending;                                           /**< Receive pending flag */
    uint8_t                                 bDmPending;                                             /**< DM pending flag */
    uint8_t                                 bFrmrPending;                                           /**< FRMR pending flag */
    uint8_t                                 socketIndex;                                            /**< Index of socket  from socket table */
    uint8_t                                 LinkStatusError;                                        /**< Link status error flag */
    uint8_t                                 nDiscoveryListSize;                                     /**< Discovery list size */
    uint8_t                                 nDiscoveryReqOffset;                                    /**< Discovery request offset */
    uint8_t                                 nDiscoveryResOffset;                                    /**< Discovery response offset */
    uint8_t                                 nDiscoveryResListSize;                                  /**< Discovery response list size */
    phlnLlcp_Fri_sPacketSequence_t          sSequence;                                              /**< Packet sequence number for sending and reception*/
    phlnLlcp_Fri_sPacketHeader_t            sLlcpHeader;                                            /**< Llcp header*/
    phlnLlcp_Fri_sPacketHeader_t            sDmHeader;                                              /**< Info field of pending DM packet*/
    uint8_t                                 DmInfoBuffer[3];                                        /**< Info field for DM frame */
    uint8_t                                 FrmrInfoBuffer[4];                                      /**< Info field of pending FRMR packet*/
    uint8_t                                 nDiscoveryResTidList[PHLNLLCP_FRI_SNL_RESPONSE_MAX];    /**< Discovery response TID list */
    uint8_t                                 nDiscoveryResSapList[PHLNLLCP_FRI_SNL_RESPONSE_MAX];    /**< Discovery response SAP list */
    uint8_t                                 pDiscoveryBuffer[PHLNLLCP_FRI_MIU_DEFAULT];             /**< Discovery buffer */
    uint8_t                                *pnDiscoverySapList;                                     /**< Discovery service access point list */
    uint8_t                                *pServiceNames[PHLNLLCP_FRI_NB_SOCKET_MAX];               /**< Discovery service access point list */
    phNfc_sData_t                          *psDiscoveryServiceNameList;                             /**< Service discovery name */
    phlnLlcp_Fri_t                         *pLlcp;                                                  /**< Pointer to the Llcp layer */
    void                                   *pLinkSendContext;                                       /**< Send callback context */
    void                                   *pDiscoverContext;                                       /**< Discover callback context */
    phlnLlcp_Fri_Send_CB_t                  pfLinkSendCb;                                           /**< Send callback */
    phNfc_sData_t                           sDmPayload;                                             /**< DM frame payload */
    pphlnLlcp_Fri_Cr_t                      pfDiscover_Cb;                                          /**< Discover callback */
    phlnLlcp_Fri_Transport_Socket_t         pSocketTable[PHLNLLCP_FRI_NB_SOCKET_MAX];               /**< Socket table */
    phlnLlcp_Fri_CachedServiceName_t        pCachedServiceNames[PHLNLLCP_FRI_SDP_ADVERTISED_NB];    /**< Service name/SAP table */
};

/**
* \brief FRI LLCP parameter structure
*/
typedef struct
{
    uint16_t                         wId;              /**< Layer ID for this component, NEVER MODIFY! */
    uint16_t                         wTxBufferLength;  /**< Length of the Tx buffer */
    uint16_t                         wRxBufferLength;  /**< Pointer to FRI llcp elements */
    phlnLlcp_Fri_t                  *pLlcp;            /**< Pointer to the Llcp layer */
    phlnLlcp_Fri_sLinkParameters_t  *pLinkParams;      /**< Pointer to the Link parameters */
    phlnLlcp_Fri_Transport_t        *pLlcpSocketTable; /**< Pointer to the Socket table */
    phHal_sRemoteDevInformation_t   *pRemoteDevInfo;   /**< Pointer to the Remote device information */
    void                            *pTxBuffer;        /**< Pointer to the Tx buffer */
    void                            *pRxBuffer;        /**< Length of the Rx buffer */
    void                            *pLowerDevice;     /**< Pointer to the Lower device */
} phlnLlcp_Fri_DataParams_t;

/**
* \brief Initialise the LLCP FRI component.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phlnLlcp_Fri_Init( phlnLlcp_Fri_DataParams_t         *pDataParams,          /**< [In] Pointer to this layer's parameter structure. */
                              uint16_t                           wSizeOfDataParams,    /**< [In] Size of the Dataparams */
                              phlnLlcp_Fri_t                    *pLlcp,                /**< [In] Pointer to the Llcp layer */
                              phlnLlcp_Fri_sLinkParameters_t    *pLinkParams,          /**< [In] Pointer to the Link parameters */
                              phlnLlcp_Fri_Transport_t          *pLlcpSocketTable,     /**< [In] Pointer to the Socket table */
                              phHal_sRemoteDevInformation_t     *pRemoteDevInfo,       /**< [In] Pointer to the Remote device information */
                              void                              *pTxBuffer,            /**< [In] Pointer to the Tx buffer */
                              uint16_t                           wTxBufferLength,      /**< [In] Length of the Tx buffer */
                              void                              *pRxBuffer,            /**< [In] Pointer to the Rx buffer */
                              uint16_t                           wRxBufferLength,      /**< [In] Length of the Rx buffer */
                              void                              *pLowerDevice          /**< [In] Pointer to the Lower device */
                             );

/** @} */
#endif /* NXPBUILD__PHLN_LLCP_FRI */

#ifdef NXPBUILD__PHLN_LLCP

/** \defgroup phlnLlcp LLCP
* \brief These are the Components which are used to abstract the functionality of the physical reader device to a generic interface.
* @{
*/

/**
* \brief Reset the LLCP layer
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS   Invalid Component ID
* \retval #PH_ERR_INVALID_PARAMETER     Parameter value is invalid.
* \retval #PH_ERR_BUFFER_TOO_SMALL      The working buffer is too small for the MIU and RW
*                                       declared in the options.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phlnLlcp_Reset( void                          *pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                           phlnLlcp_Fri_LinkStatus_CB_t   pfLink_CB,    /**< [In] Pointer to the callback function. */
                           void                          *pContext      /**< [In] Pointer to the Reset context */
                          );

/**
* \brief Check the LLCP layer link
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS   Invalid Component ID
* \retval #PH_ERR_INVALID_STATE         If LLCP is not is proper state.
* \retval #PH_ERR_INVALID_DEVICE        Device is neither Initiator nor Target
* \retval #PH_ERR_FAILED                Operation failed.
* \retval #PH_ERR_BUSY                  Previous request in progress can not accept new request.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phlnLlcp_ChkLlcp( void                      *pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                             phlnLlcp_Fri_Check_CB_t    pfCheck_CB,   /**< [In] Pointer to the callback function. */
                             void                      *pContext      /**< [In] Pointer to the Reset context */
                             );

/**
* \brief Activate the LLCP layer link
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS   Invalid Component ID
* \retval #PH_ERR_PENDING               Activation operation is in progress.
* \retval #PH_ERR_INVALID_PARAMETER     One or more of the supplied parameters
*                                       could not be properly interpreted.
* \retval #PH_ERR_NOT_INITIALISED       Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                Operation failed.
* \retval #PH_ERR_BUSY                  Previous request in progress can not accept new request.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Activate( void  *pDataParams   /**< [In] Pointer to this layer's parameter structure. */
                             );

/**
* \brief Deactivate the LLCP layer link
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS   Invalid Component ID
* \retval #PH_ERR_PENDING               Activation operation is in progress.
* \retval #PH_ERR_INVALID_PARAMETER     One or more of the supplied parameters
*                                       could not be properly interpreted.
* \retval #PH_ERR_NOT_INITIALISED       Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                Operation failed.
* \retval #PH_ERR_BUSY                  Previous request in progress can not accept new request.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Deactivate( void   *pDataParams   /**< [In] Pointer to this layer's parameter structure. */
                               );

/**
* \brief Get the local node information
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER     One or more of the supplied parameters
*                                       could not be properly interpreted.
* \retval #PH_ERR_FAILED                Operation failed.
* \retval #PH_ERR_BUSY                  Previous request in progress can not accept new request.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_GetLocalInfo( void  *pDataParams    /**< [In] Pointer to this layer's parameter structure. */
                                 );

/**
* \brief Get the Remote node information
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER     One or more of the supplied parameters
*                                       could not be properly interpreted.
* \retval #PH_ERR_FAILED                Operation failed.
* \retval #PH_ERR_BUSY                  Previous request in progress can not accept new request.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_GetRemoteInfo( void  *pDataParams    /**< [In] Pointer to this layer's parameter structure. */
                                  );

/**
* \brief API to send the data over LLCP
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_STATE         If LLCP is not is proper state.
* \retval #PH_ERR_INVALID_PARAMETER     One or more of the supplied parameters
*                                       could not be properly interpreted.
* \retval #PH_ERR_FAILED                Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Send( void                             *pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                          phlnLlcp_Fri_sPacketHeader_t     *pHeader,       /**< [In] Pointer to the header */
                          phlnLlcp_Fri_sPacketSequence_t   *pSequence,     /**< [In] Pointer to the Packet sequence */
                          phNfc_sData_t                    *pInfo,         /**< [In] Pointer to the Data */
                          phlnLlcp_Fri_Send_CB_t            pfSend_CB,     /**< [In] Pointer to the callback function */
                          void                             *pContext       /**< [In] Pointer to the context */
                          );

/**
* \brief API to receive the data over LLCP
* \return Status code
* \retval #PH_ERR_SUCCESS               Operation successful.
* \retval #PH_ERR_INVALID_STATE         If LLCP is not is proper state.
* \retval #PH_ERR_PENDING               Reception operation is in progress.
* \retval #PH_ERR_INVALID_PARAMETER     One or more of the supplied parameters
*                                       could not be properly interpreted.
* \retval #PH_ERR_FAILED                Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Recv(void                      *pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                         phlnLlcp_Fri_Recv_CB_t     pfRecv_CB,     /**< [In] Pointer to the callback function */
                         void                      *pContext       /**< [In] Pointer to the context */
                         );
/**
* \brief Reset API for Transport layer
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS        Invalid Component ID
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PHLNLLCP_FRI_NFCSTATUS_REJECTED   A send operation is already running
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Reset(void    *pDataParams  /**< [In] Pointer to this layer's parameter structure. */
                                    );

/**
* \brief Close all the sockets
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS        Invalid Component ID
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_CloseAll(void  *pDataParams   /**< [In] Pointer to this layer's parameter structure. */
                                       );

/**
* \brief Get the local node information
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS        Invalid Component ID
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_INVALID_STATE              Socket state is Invalid
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_SocketGetLocalOptions( void                                      *pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                                                     phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,   /**< [In] Pointer to socket */
                                                     phlnLlcp_Fri_Transport_sSocketOptions_t   *pLocalOptions  /**< [Out] Pointer to local socket options */
                                                    );

/**
* \brief Get the local node information
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS        Invalid Component ID
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_INVALID_STATE              Socket state is Invalid
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_SocketGetRemoteOptions( void                                     *pDataParams,      /**< [In] Pointer to this layer's parameter structure. */
                                                      phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,      /**< [In] Pointer to socket */
                                                      phlnLlcp_Fri_Transport_sSocketOptions_t  *psRemoteOptions   /**< [Out] Pointer to remote socket options */
                                                     );

/**
* \brief Create a socket on a LLCP-connected device
*
* This function creates a socket for a given LLCP link. Sockets can be of two types :
* connection-oriented and connection-less. If the socket is connection-oriented, the caller
* must provide a working buffer to the socket in order to handle incoming data. This buffer
* must be large enough to fit the receive window (RW * MIU), the remaining space being
* used as a linear buffer to store incoming data as a stream. Data will be readable later
* using the phLibNfc_LlcpTransport_Recv function.
* The options and working buffer are not required if the socket is used as a listening socket,
* since it cannot be directly used for communication.
*
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_BUFFER_TOO_SMALL           The working buffer is too small for the MIU and RW
*                                            declared in the options.
* \retval #PH_ERR_INSUFFICIENT_RESOURCES     No more socket handle available.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Socket( void                                     *pDataParams,      /**< [In] Pointer to this layer's parameter structure. */
                                      phlnLlcp_Transport_eSocketType_t          eType,            /**< [In] The socket type. */
                                      phlnLlcp_Fri_Transport_sSocketOptions_t  *pOptions,         /**< [In] The options to be used with the socket. */
                                      phNfc_sData_t                            *pWorkingBuffer,   /**< [In] A working buffer to be used by the library. */
                                      phlnLlcp_Fri_Transport_Socket_t         **pLlcpSocket,      /**< [Out] A pointer on the socket to be filled with a socket found on the socket table. */
                                      phlnLlcp_Fri_TransportSocketErrCb_t       pErr_Cb,          /**< [In] The callback to be called each time the socket is in error. */
                                      void                                     *pContext          /**< [In] Upper layer context to be returned in the callback */
                                    );

/**
* \brief Close the socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Close( void                            *pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                                     phlnLlcp_Fri_Transport_Socket_t *pLlcpSocket   /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                    );

/**
* \brief Bind a socket to a local SAP
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_ALREADY_REGISTERED         The selected SAP is already bound to another
                                             socket.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Bind( void                             *pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                    phlnLlcp_Fri_Transport_Socket_t  *pLlcpSocket,    /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                    uint8_t                           nSap,           /**< [In] A port number for a specific socket */
                                    phNfc_sData_t                    *psServiceName   /**< [In] Service name */
                                    );

/**
* \brief Listen for incoming connection requests on a socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Listen( void                                    *pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                                      phlnLlcp_Fri_Transport_Socket_t         *pLlcpSocket,   /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                      phlnLlcp_Fri_TransportSocketListenCb_t   pListen_Cb,    /**< [In] The callback to be called each time the socket receive a connection request. */
                                      void                                    *pContext       /**< [In] Upper layer context to be returned in the callback. */
                                     );

/**
* \brief Accept an incoming connection request for a socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_BUFFER_TOO_SMALL           The working buffer is too small for the MIU and RW
*                                            declared in the options.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Accept( void                                      *pDataParams,      /**< [In] Pointer to this layer's parameter structure. */
                                      phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,      /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                      phlnLlcp_Fri_Transport_sSocketOptions_t   *pOptions,         /**< [In] The options to be used with the socket. */
                                      phNfc_sData_t                             *psWorkingBuffer,  /**< [In] A working buffer to be used by the library. */
                                      phlnLlcp_Fri_TransportSocketErrCb_t        pErr_Cb,          /**< [In] The callback to be called each time the accepted socket is in error. */
                                      phlnLlcp_Fri_TransportSocketAcceptCb_t     pAccept_RspCb,    /**< [In] The callback to be called each time the socket is accepted. */
                                      void                                      *pContext          /**< [In] Upper layer context to be returned in the callback. */
                                     );

/**
* \brief Reject an incoming connection request for a socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Reject( void                                   *pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                      phlnLlcp_Fri_Transport_Socket_t        *pLlcpSocket,    /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                      phlnLlcp_Fri_TransportSocketRejectCb_t  pReject_RspCb,  /**< [In] The callback to be call when the Reject operation is completed */
                                      void                                   *pContext        /**< [In] Upper layer context to be returned in the callback. */
                                     );

/**
* \brief Try to establish connection with a socket on a remote SAP
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_PENDING                    Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Connect( void                                     *pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                                       phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,     /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                       uint8_t                                   nSap,            /**< [In] The destination SAP to connect to. */
                                       phlnLlcp_Fri_TransportSocketConnectCb_t   pConnect_RspCb,  /**< [In] The callback to be call when the connection operation is completed */
                                       void                                     *pContext         /**< [In] Upper layer context to be returned in the callback. */
                                      );

/**
* \brief Try to establish connection with a socket on a remote service, given its URI
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_PENDING                    Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_ConnectByUri( void                                     *pDataParams,      /**< [In] Pointer to this layer's parameter structure. */
                                            phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,      /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                            phNfc_sData_t                            *psUri,            /**< [In] The URI corresponding to the destination SAP to connect to. */
                                            phlnLlcp_Fri_TransportSocketConnectCb_t   pConnect_RspCb,   /**< [In] The callback to be call when the connection operation is completed */
                                            void                                     *pContext          /**< [In] Upper layer context to be returned in the callback. */
                                            );

/**
* \brief Disconnect a currently connected socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_PENDING                    Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Disconnect( void                             *pDataParams,         /**< [In] Pointer to this layer's parameter structure. */
                                          phlnLlcp_Fri_Transport_Socket_t  *pLlcpSocket,         /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                          phlnLlcp_SocketDisconnectCb_t     pDisconnect_RspCb,   /**< [In] The callback to be call when the disconnection operation is completed */
                                          void                             *pContext             /**< [In] Upper layer context to be returned in the callback. */
                                         );

/**
* \brief Send data on a socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_PENDING                    Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Send( void                                  *pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                                    phlnLlcp_Fri_Transport_Socket_t       *pLlcpSocket,   /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                    phNfc_sData_t                         *pBuffer,       /**< [In] The buffer containing the data to send. */
                                    phlnLlcp_Fri_TransportSocketSendCb_t   pSend_RspCb,   /**< [In] The callback to be call when the send operation is completed */
                                    void                                  *pContext       /**< [In] Upper layer context to be returned in the callback. */
                                    );

/**
* \brief Read data on a socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_PENDING                    Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_Recv( void                                   *pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                    phlnLlcp_Fri_Transport_Socket_t        *pLlcpSocket,    /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                    phNfc_sData_t                          *pBuffer,        /**< [In] The buffer receiving the data. */
                                    phlnLlcp_Fri_TransportSocketRecvCb_t    pRecv_RspCb,    /**< [In] The callback to be call when the receive operation is completed */
                                    void                                   *pContext        /**< [In] Upper layer context to be returned in the callback. */
                                    );

/*****************************************/
/*           ConnectionLess              */
/*****************************************/

/**
* \brief Send data on a socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_PENDING                    Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_SendTo( void                                 *pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                                      phlnLlcp_Fri_Transport_Socket_t      *pLlcpSocket,   /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                      uint8_t                               nSap,          /**< [In] Destination SAP number. */
                                      phNfc_sData_t                        *pBuffer,       /**< [In] The buffer containing the data to send. */
                                      phlnLlcp_Fri_TransportSocketSendCb_t  pSend_RspCb,   /**< [In] The callback to be call when the send operation is completed */
                                      void                                 *pContext       /**< [In] Upper layer context to be returned in the callback. */
                                     );

/**
* \brief Read data on a socket
* \return Status code
* \retval #PH_ERR_SUCCESS                    Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER          One or more of the supplied parameters
*                                            could not be properly interpreted.
* \retval #PH_ERR_PENDING                    Connection operation is in progress,
*                                            pConnect_RspCb will be called upon completion.
* \retval #PH_ERR_INVALID_STATE              The socket is not in a valid state, or not of
*                                            a valid type to perform the requsted operation.
* \retval #PH_ERR_NOT_INITIALISED            Indicates stack is not yet initialized.
* \retval #PH_ERR_FAILED                     Operation failed.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phlnLlcp_Transport_RecvFrom( void                                     *pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                                        phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,   /**< [In] A pointer to a phlnLlcp_Fri_Transport_Socket_t. */
                                        phNfc_sData_t                            *pBuffer,       /**< [In] The buffer receiving the data. */
                                        phlnLlcp_Fri_TransportSocketRecvFromCb_t  pRecv_Cb,      /**< [In] The callback to be call when the receive operation is completed */
                                        void                                     *pContext       /**< [In] Upper layer context to be returned in the callback. */
                                        );

/** @} */
#endif /* NXPBUILD__PHLN_LLCP */

#ifdef __cplusplus
} /* Extern C */
#endif /* Extern C */

#endif /* PHLNLLCP_H*/
