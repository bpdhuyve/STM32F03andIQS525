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
* \file  phlnLlcp_Fri_Llcp.h
* \brief NFC LLCP core
* $Author: nxp62726 $
* $Revision: 501 $
* $Date: 2014-02-13 12:24:52 +0530 (Thu, 13 Feb 2014) $
*
*
*/

#ifndef PHLNLLCP_FRI_LLCP_H
#define PHLNLLCP_FRI_LLCP_H

#include <ph_Status.h>
#include <phlnLlcp.h>
#include "phlnLlcp_Fri_Mac.h"

/** 
* \name NFC Forum Logical Link Control Protocol
*
* File: \ref phlnLlcp_Fri_Llcp.h
*
*/
/*=========== CONSTANTS ===========*/

/**
*  \name LLCP local protocol version.
*
*/
/*@{*/
#define PHLNLLCP_FRI_VERSION_MAJOR    0x01  /**< Major number of local LLCP version.*/
#define PHLNLLCP_FRI_VERSION_MINOR    0x01  /**< Minor number of local LLCP version.*/
#define PHLNLLCP_FRI_VERSION          ((PHLNLLCP_FRI_VERSION_MAJOR << 4) | PHLNLLCP_FRI_VERSION_MINOR) /**< Local LLCP version.*/
/*@}*/

/**
*  \name LLCP well-known SAPs.
*
*/
/*@{*/
#define PHLNLLCP_FRI_SAP_LINK                     0x00 /**< Link SAP.*/
#define PHLNLLCP_FRI_SAP_SDP                      0x01 /**< Service Discovery Protocol SAP.*/
#define PHLNLLCP_FRI_SAP_WKS_FIRST                0x02 /**< Other Well-Known Services defined by the NFC Forum.*/
#define PHLNLLCP_FRI_SAP_SDP_ADVERTISED_FIRST     0x10 /**< First SAP number from SDP-avertised SAP range.*/
#define PHLNLLCP_FRI_SAP_SDP_UNADVERTISED_FIRST   0x20 /**< First SAP number from SDP-unavertised SAP range.*/
#define PHLNLLCP_FRI_SAP_NUMBER                   0x40 /**< Number of possible SAP values (also first invalid value).*/
#define PHLNLLCP_FRI_SAP_DEFAULT                  0xFF /**< Default number when a socket is created or reset */
/*@}*/

/**
*  \name LLCP well-known SAPs.
*
*/
/*@{*/
#define PHLNLLCP_FRI_SERVICENAME_SDP      "urn:nfc:sn:sdp" /**< Service Discovery Protocol name.*/
/*@}*/

/**
*  \name Length value for DM opCode
*
*/
/*@{*/
#define PHLNLLCP_FRI_DM_LENGTH        0x01 /**< Length value for DM opCode */
/*@}*/


/**
* \internal 
* \name Masks used with parameters value.
*
*/
/*@{*/
#define PHLNLLCP_FRI_TLV_MIUX_MASK        0x07FFU   /**< \internal Mask to apply to MIUX TLV Value.*/
#define PHLNLLCP_FRI_TLV_WKS_MASK         0x0001    /**< \internal Minimal bits to be set in WKS TLV Value.*/
#define PHLNLLCP_FRI_TLV_RW_MASK          0x0F      /**< \internal Mask to apply to RW TLV Value.*/
#define PHLNLLCP_FRI_TLV_OPT_MASK         0x03      /**< \internal Mask to apply to OPT TLV Value.*/
/*@}*/

/**
* \internal 
* \name Type codes for parameters in TLV.
*
*/
/*@{*/
#define PHLNLLCP_FRI_TLV_TYPE_VERSION       0x01   /**< \internal VERSION parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_MIUX          0x02   /**< \internal MIUX parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_WKS           0x03   /**< \internal WKS parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_LTO           0x04   /**< \internal LTO parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_RW            0x05   /**< \internal RW parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_SN            0x06   /**< \internal SN parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_OPT           0x07   /**< \internal OPT parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_SDREQ         0x08   /**< \internal SDREQ parameter Type code.*/
#define PHLNLLCP_FRI_TLV_TYPE_SDRES         0x09   /**< \internal SDRES parameter Type code.*/
/*@}*/

/**
* \internal 
* \name Fixed Value length for parameters in TLV.
*
*/
/*@{*/
#define PHLNLLCP_FRI_TLV_LENGTH_HEADER         2    /**< \internal Fixed length of Type and Length fields in TLV.*/
#define PHLNLLCP_FRI_TLV_LENGTH_VERSION        1    /**< \internal Fixed length of VERSION parameter Value.*/
#define PHLNLLCP_FRI_TLV_LENGTH_MIUX           2    /**< \internal Fixed length of MIUX parameter Value.*/
#define PHLNLLCP_FRI_TLV_LENGTH_WKS            2    /**< \internal Fixed length of WKS parameter Value.*/
#define PHLNLLCP_FRI_TLV_LENGTH_LTO            1    /**< \internal Fixed length of LTO parameter Value.*/
#define PHLNLLCP_FRI_TLV_LENGTH_RW             1    /**< \internal Fixed length of RW parameter Value.*/
#define PHLNLLCP_FRI_TLV_LENGTH_OPT            1    /**< \internal Fixed length of OPT parameter Value.*/
/*@}*/

/**
*  \name LLCP packet field sizes.
*
*/
/*@{*/
#define PHLNLLCP_FRI_PACKET_HEADER_SIZE      2    /**< Size of the general packet header (DSAP+PTYPE+SSAP).*/
#define PHLNLLCP_FRI_PACKET_SEQUENCE_SIZE    1    /**< Size of the sequence field, if present.*/
#define PHLNLLCP_FRI_PACKET_MAX_SIZE        (PHLNLLCP_FRI_PACKET_HEADER_SIZE + \
    PHLNLLCP_FRI_PACKET_SEQUENCE_SIZE + \
    PHLNLLCP_FRI_MIU_DEFAULT + \
    PHLNLLCP_FRI_TLV_MIUX_MASK) /**< Maximum size of a packet */
/*@}*/

/**
* \internal
* \name States of the LLC state machine.
*
*/
/*@{*/
#define PHLNLLCP_FRI_STATE_RESET_INIT           0   /**< \internal Initial state.*/
#define PHLNLLCP_FRI_STATE_CHECKED              1   /**< \internal The tag has been checked for LLCP compliance.*/
#define PHLNLLCP_FRI_STATE_ACTIVATION           2   /**< \internal The activation phase.*/
#define PHLNLLCP_FRI_STATE_PAX                  3   /**< \internal Parameter exchange phase.*/
#define PHLNLLCP_FRI_STATE_OPERATION_RECV       4   /**< \internal Normal operation phase (ready to receive).*/
#define PHLNLLCP_FRI_STATE_OPERATION_SEND       5   /**< \internal Normal operation phase (ready to send).*/
#define PHLNLLCP_FRI_STATE_DEACTIVATION         6   /**< \internal The deactivation phase.*/
/*@}*/

/**
* \internal
* \name Masks used for VERSION parsing.
*
*/
/*@{*/
#define PHLNLLCP_FRI_VERSION_MAJOR_MASK       0xF0U    /**< \internal Mask to apply to get major version number.*/
#define PHLNLLCP_FRI_VERSION_MINOR_MASK       0x0F     /**< \internal Mask to apply to get minor version number.*/
/*@}*/

/**
* \internal
* \name Invalid values for parameters.
*
*/
/*@{*/
#define PHLNLLCP_FRI_INVALID_VERSION        0x00   /**< \internal Invalid VERSION value.*/
/*@}*/

/**
* \internal
* \name Internal constants.
*
*/
/*@{*/
#define PHLNLLCP_FRI_MAX_PARAM_TLV_LENGTH \
    (( PHLNLLCP_FRI_TLV_LENGTH_HEADER + PHLNLLCP_FRI_TLV_LENGTH_VERSION ) + \
    ( PHLNLLCP_FRI_TLV_LENGTH_HEADER + PHLNLLCP_FRI_TLV_LENGTH_MIUX ) + \
    ( PHLNLLCP_FRI_TLV_LENGTH_HEADER + PHLNLLCP_FRI_TLV_LENGTH_WKS ) + \
    ( PHLNLLCP_FRI_TLV_LENGTH_HEADER + PHLNLLCP_FRI_TLV_LENGTH_LTO ) + \
    ( PHLNLLCP_FRI_TLV_LENGTH_HEADER + PHLNLLCP_FRI_TLV_LENGTH_OPT ))   /**< \internal Maximum size of link params TLV.*/
#define PHLNLLCP_FRI_SLOW_SYMMETRY                8
/*@}*/

/*========== MACROS ===========*/

#define ABS_SUB(a,b) ((a)>(b)?((a)-(b)):((b)-(a)))
#define CHECK_SEND_RW(socket) ( (ABS_SUB((socket)->socket_VS, (socket)->socket_VSA)  % 16) < (socket)->remoteRW )

/* This Macro to be used to resolve Unused and unreference
* compiler warnings.
*/

#define PHLNLLCP_FRI_UNUSED_VARIABLE(x) for((x)=(x);(x)!=(x);)

/*========== FUNCTIONS ===========*/

phStatus_t phlnLlcp_Fri_EncodeLinkParams( phNfc_sData_t                    *psRawBuffer,
                                         phlnLlcp_Fri_sLinkParameters_t    *psLinkParams,
                                         uint8_t                            nVersion );

phStatus_t phlnLlcp_Fri_Reset( phlnLlcp_Fri_t                  *Llcp,
                              void                             *LowerDevice,
                              phlnLlcp_Fri_sLinkParameters_t   *psLinkParams,
                              void                             *pRxBuffer,
                              uint16_t                          nRxBufferLength,
                              void                             *pTxBuffer,
                              uint16_t                          nTxBufferLength,
                              phlnLlcp_Fri_LinkStatus_CB_t      pfLink_CB,
                              void                             *pContext );

phStatus_t phlnLlcp_Fri_ChkLlcp( phlnLlcp_Fri_t                *Llcp,
                                phHal_sRemoteDevInformation_t  *psRemoteDevInfo,
                                phlnLlcp_Fri_Check_CB_t         pfCheck_CB,
                                void                           *pContext );

phStatus_t phlnLlcp_Fri_Activate( phlnLlcp_Fri_t  *Llcp );

phStatus_t phlnLlcp_Fri_Deactivate( phlnLlcp_Fri_t *Llcp );

phStatus_t phlnLlcp_Fri_GetLocalInfo( phlnLlcp_Fri_t                   *Llcp,
                                     phlnLlcp_Fri_sLinkParameters_t    *pParams );

phStatus_t phlnLlcp_Fri_GetRemoteInfo( phlnLlcp_Fri_t                  *Llcp,
                                      phlnLlcp_Fri_sLinkParameters_t   *pParams );

phStatus_t phlnLlcp_Fri_Send( phlnLlcp_Fri_t                  *Llcp,
                             phlnLlcp_Fri_sPacketHeader_t     *psHeader,
                             phlnLlcp_Fri_sPacketSequence_t   *psSequence,
                             phNfc_sData_t                    *psInfo,
                             phlnLlcp_Fri_Send_CB_t            pfSend_CB,
                             void                             *pContext );

phStatus_t phlnLlcp_Fri_Recv( phlnLlcp_Fri_t           *Llcp,
                             phlnLlcp_Fri_Recv_CB_t     pfRecv_CB,
                             void                      *pContext );

/** @}
* end of phlnLlcp_Fri group
*/
#endif /* PHLNLLCP_FRI_LLCP_H */
