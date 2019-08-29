/*
*         Copyright (c), NXP Semiconductors Gratkorn / Austria
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
* Generic ISO18092 target mode Component of Reader Library Framework.
* $Author: nxp69453 $
* $Revision: 498 $
* $Date: 2014-02-11 10:12:36 +0530 (Tue, 11 Feb 2014) $
*
* History:
*  CEn: Generated 07. September 2010
*/

#ifndef PHPALI18092MT_H
#define PHPALI18092MT_H

#include <ph_Status.h>
#include <phhalHw.h>

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

#ifdef NXPBUILD__PHPAL_I18092MT_SW

/** \defgroup phpalI18092mT_Sw Component : Software
* @{
*/

#define PHPAL_I18092MT_SW_ID                               0x01U        /**< ID for software I18092 MT component */
#define PHPAL_I18092MT_NFCID3_LENGTH                       0x0AU        /**< Length of NFCID3. */

/** Typedef for the error handling call back function */
typedef void(*pInterruptSetCallback)(uint8_t);

/**
* \brief ISO18092 mode passive target parameter structure
*/
typedef struct 
{
    uint16_t wId;                                                      /**< Layer ID for this component, NEVER MODIFY */
    void * pHalDataParams;                                             /**< Pointer to the parameter structure of underlaying hardware component */
    uint8_t bNfcIdValid;                                               /**< Whether current NfcID is valid or not. */
    uint8_t aNfcid3i[PHPAL_I18092MT_NFCID3_LENGTH];                    /**< Array holding 10 bytes long NFCID3. */
    uint8_t aNfcid3t[PHPAL_I18092MT_NFCID3_LENGTH];                    /**< Array holding 10 bytes long NFCID3 of the initiator . */
    uint8_t bStateNow;                                                 /**< Current Exchange-State. */
    uint8_t bDid;                                                      /**< Device Identifier; '0' to disable usage else 1-14. */
    uint8_t bNadEnabled;                                               /**< Node Address Enabler; Unequal '0' if enabled. */
    uint8_t bNad;                                                      /**< Node Address; Ignored if bNadEnabled is equal '0'. */
    uint8_t bWt;                                                       /**< Waiting Time; 0-14 */
    uint8_t bFsl;                                                      /**< Frame Length Byte; 0-3 */
    uint8_t bPni;                                                      /**< Current Packet Number (Packet number information); 0-3 */
    uint8_t bDst;                                                      /**< Divisor Send Integer; 0-2 */
    uint8_t bDrt;                                                      /**< Divisor Receive Integer; 0-2 */
    uint8_t bBsi;                                                      /**< Send bit-rate supported by the initiator */
    uint8_t bBri;                                                      /**< Receive bit-rate supported by the initiator */
    uint8_t bLri;                                                      /**< Length reduction value is stored here */
    uint8_t bBst;                                                      /**< Send bit-rate supported by the initiator */
    uint8_t bBrt;                                                      /**< Receive bit-rate supported by the initiator */
    uint8_t bLrt;                                                      /**< Length reduction value is stored here */
    uint8_t bMaxRetryCount;                                            /**< Maximum retry count for retransmissions. */
    uint8_t * pGt;                                                     /**< Holds the pointer to the general bytes of the target */
    uint8_t bGtLength;                                                 /**< Target general bytes length */
    uint8_t bTo;                                                       /**< Hold the time out value */
    uint8_t bTargetState;                                              /**< Holds the target state */
    uint8_t bRtoxDisabled;                                             /**< RTOX Disabler (i.e. '0' if RTOX enabled). */
    uint8_t bRtox;                                                     /**< Response timeout extension value */
    pInterruptSetCallback pSet_Interrupt;                              /**< Holds Set_Interrupt function address */
    void * ovrTask;                                                    /**< Task context */
    uint8_t bCmdtype;                                                  /**< Command type */
    void * phOsal;                                                     /**< Pointer to OSAL layer */
    uint32_t dwTimerId;                                                /**< Timer ID (For RTOX) */
    phStatus_t rtoxStatus;                                             /**< RTOX status */
    uint8_t bActiveMode;                                               /**< Active mode flag */
    uint8_t bAutocollRetryCount;                                       /**< Autocoll retry count; default value is 0 */
}phpalI18092mT_Sw_DataParams_t;

/**
* \brief Initialise this layer.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phpalI18092mT_Sw_Init(
    phpalI18092mT_Sw_DataParams_t * pDataParams,                       /**< [In] Pointer to this layer's parameter structure. */
    uint16_t wSizeOfDataParams,                                        /**< [In] Specifies the size of the data parameter structure. */
    void * pHalDataParams,                                             /**< [In] Pointer to the parameter structure of the underlying layer. */
    pInterruptSetCallback pSetInterrupt                                /**< [In] Pointer to Interrupt enabling function */
    );

/** @} */
#endif  /* NXPBUILD__PHPAL_I18092MT_SW */

#ifdef NXPBUILD__PHPAL_I18092MT

/** \defgroup phpalI18092mT ISO18092 mode Target
* \brief These Components implement the ISO/IEC ISO18092 contactless protocol. Supported mode is active/passive target, i.e.
* using these components it is possible to communicate with a active/passive initiator in peer to peer mode.
* @{
*/

/** \addtogroup ph_Error
* phpalI18092 Custom Errors
* @{
*/
/** \name phpalI18092mT Error Codes
*/
/*@{*/
#define PHPAL_I18092MT_ERR_RECOVERY_FAILED  (PH_ERR_CUSTOM_BEGIN+0)     /**< ISO18092 error recovery failed. */
/*@}*/
/** @} */

#define PHPAL_I18092MT_DID_MAX          0x0EU         /**< The last valid DID. */
#define PHPAL_I18092MT_WT_MAX           0x0EU         /**< The maximum allowed WT value, NFCForum-TS-DigitalProtocol-1.0 section 14.11. */
#define PHPAL_I18092MT_FSL_MAX          0x03U         /**< The maximum allowed FSL value. */
#define PHPAL_I18092MT_PNI_MAX          0x03U         /**< The maximum allowed PNI value. */

/**
* \name ISO18092 Mode target Parameters
*/
/*@{*/
/**
* \brief Set / Get packet number.
*/
#define PHPAL_I18092MT_CONFIG_PACKETNO                 0x0000U
/**
* \brief Set / Get Device Identifier.
*/
#define PHPAL_I18092MT_CONFIG_DID                      0x0001U
/**
* \brief Set / Get Node Address.
*
* \c wValue is parsed as follows:
* \verbatim
* NadEnabled = (wValue & 0xFF00) ? 1 : 0;
* Nad = (wValue & 0x00FF);
* \endverbatim
*/
#define PHPAL_I18092MT_CONFIG_NAD                      0x0002U
/**                                                    
* \brief Set / Get Waiting Time.                       
*/                                                     
#define PHPAL_I18092MT_CONFIG_WT                       0x0003U
/**                                                    
* \brief Set / Get Frame Length.                       
*/                                                     
#define PHPAL_I18092MT_CONFIG_FSL                      0x0004U
/**
* \brief Set / Get Maximum Retry Count.
*/
#define PHPAL_I18092MT_CONFIG_MAXRETRYCOUNT            0x0005U
/**                                                    
* \brief Set / Get active mode configuration.          
*/                                                     
#define PHPAL_I18092MT_CONFIG_ACTIVEMODE               0x0006U
/**                                                    
* \brief Enable / Disable RTOX feature.                
*/                                                     
#define PHPAL_I18092MT_CONFIG_RTOX                     0x0007U
/**                                                    
* \brief RTOX value.                                   
*/                                                     
#define PHPAL_I18092MT_CONFIG_RTOX_VALUE               0x0008U
/**
* \brief Autocoll retry count.
*/
#define PHPAL_I18092MT_CONFIG_AUTOCOLL_RETRY_COUNT     0x0009U


/** Attribute request command code. */
#define PHPAL_I18092MT_SW_CMD_ATR_REQ                  0x00U

/** Bit position of LRt field within PPt byte for attribute request command. */
#define PHPAL_I18092MT_SW_ATR_PPT_LRT_BIT              0x04U

/** Attribute response command code. */
#define PHPAL_I18092MT_SW_CMD_ATR_RES                  0x01U

/** Parameter select request command code. */
#define PHPAL_I18092MT_SW_CMD_PSL_REQ                  0x04U

/** Parameter select response command code. */
#define PHPAL_I18092MT_SW_CMD_PSL_RES                  0x05U

/** Data exchange protocol request command code. */
#define PHPAL_I18092MT_SW_CMD_DEP_REQ                  0x06U

/** Data exchange protocol response command code. */
#define PHPAL_I18092MT_SW_CMD_DEP_RES                  0x07U

/** Deselect request command code. */
#define PHPAL_I18092MT_SW_CMD_DSL_REQ                  0x08U

/** Deselect response command code. */
#define PHPAL_I18092MT_SW_CMD_DSL_RES                  0x09U

/** Release request command code. */
#define PHPAL_I18092MT_SW_CMD_RLS_REQ                  0x0AU

/** Release response command code. */
#define PHPAL_I18092MT_SW_CMD_RLS_RES                  0x0BU
/** Release response command code. */
#define PHPAL_I18092MT_SW_CMD_ATN                      0xFEU
/*@}*/

/**
* \name Values for #phpalI18092mT_Deselect \c bDeselectCommand parameter.
*/
/*@{*/
#define PHPAL_I18092MT_DESELECT_DSL   0x08U     /**< DSL is sent for deselection of target. */
#define PHPAL_I18092MT_DESELECT_RLS   0x0AU     /**< RLS is sent for deselection of target. */
/*@}*/

/**
* \name DRI/DSI (BRI/BSI) Values
*/
/*@{*/
#define PHPAL_I18092MT_DATARATE_106     0x00U   /**< DRT(I)/DST(I) value for 106 kBit/s. */
#define PHPAL_I18092MT_DATARATE_212     0x01U   /**< DRT(T)/DST(T) value for 212 kBit/s. */
#define PHPAL_I18092MT_DATARATE_424     0x02U   /**< DRI(T)/DSI(T) value for 424 kBit/s. */
#define PHPAL_I18092MT_DATARATE_212_424 0x03U   /**< DRI(T)/DSI(T) value for 212/424 Kbits/s. */
/*@}*/


/**
* \name Target states 
*/
/*@{*/
#define PHPAL_I18092MT_STATE_NONE              0x00U   /**< Target state none */
#define PHPAL_I18092MT_STATE_DEP_ACTIVATED     0x01U   /**< Target in DEP activated state */
/*@}*/


/**
* \name FSL/LRI Values
*/
/*@{*/
#define PHPAL_I18092MT_FRAMESIZE_64   0x00U   /**< FSL value for max. frame size of 64 Bytes.  */
#define PHPAL_I18092MT_FRAMESIZE_128  0x01U   /**< FSL value for max. frame size of 128 Bytes. */
#define PHPAL_I18092MT_FRAMESIZE_192  0x02U   /**< FSL value for max. frame size of 192 Bytes. */
#define PHPAL_I18092MT_FRAMESIZE_254  0x03U   /**< FSL value for max. frame size of 254 Bytes. */
/*@}*/

/**
* \name Maximum Values
*/
/*@{*/
#define PHPAL_I18092MT_MAX_GI_LENGTH   48U    /**< Maximum length of Gi data.  */
/*@}*/

/**
* \brief Reset the ISO18092 protocol parameters.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phpalI18092mT_ResetProtocol(
                                        void * pDataParams   /**< [In] Pointer to this layer's parameter structure. */
                                        );

/**
* \brief Validate and send the ATR response.
*
* \retval #PH_ERR_SUCCESS Operation successful, target deselected.
* \retval #PH_ERR_PROTOCOL_ERROR Received response is not ISO/IEC 18092 complaint.
* \retval #PH_ERR_IO_TIMEOUT Timeout waiting for reply, e.g. target removal.
*/
phStatus_t phpalI18092mT_AtrRes(
    void * pDataParams,                 /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pAtr,                     /**< [In] Received Attribute Request bytes. */
    uint16_t wAtrLength                 /**< [In] Received Attribute Request length. */
    );

/**
* \brief Send the Dep response.
*
* \retval #PH_ERR_SUCCESS Operation successful, target deselected.
* \retval #PH_ERR_PROTOCOL_ERROR Received response is not ISO/IEC 18092 complaint.
* \retval #PH_ERR_IO_TIMEOUT Timeout waiting for reply, e.g. target removal.
*/
phStatus_t phpalI18092mT_DepSend(
        void * pDataParams,                             /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                               /**< [In] Option parameter. */
        uint8_t * pTxBuffer,                            /**< [In] Data to transmit. */
        uint16_t wTxLength                              /**< [In] Length of data to transmit. */
        );


/**
* \brief Validate and send the Dsl response.
*
* \retval #PH_ERR_SUCCESS Operation successful, target deselected.
* \retval #PH_ERR_PROTOCOL_ERROR Received response is not ISO/IEC 18092 complaint.
* \retval #PH_ERR_IO_TIMEOUT Timeout waiting for reply, e.g. target removal.
*/
phStatus_t phpalI18092mT_DslRes(
    void * pDataParams,                                 /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pDslReq,                                  /**< [In] Received DSL Request bytes. */
    uint8_t wDslReqLength                               /**< [In] Received DSL Request length. */
    );

/**
* \brief Validate and send the Psl response.
*
* \retval #PH_ERR_SUCCESS Operation successful, target deselected.
* \retval #PH_ERR_PROTOCOL_ERROR Received response is not ISO/IEC 18092 complaint.
* \retval #PH_ERR_IO_TIMEOUT Timeout waiting for reply, e.g. target removal.
*/
phStatus_t phpalI18092mT_PslRes(
    void * pDataParams,                                 /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pPslReq,                                  /**< [In] Received PSL request bytes */
    uint16_t  wPslReqLength                             /**< [In] Received PSL request length */
    );

/**
* \brief Validate and send the Rls response.
*
* \retval #PH_ERR_SUCCESS Operation successful, target deselected.
* \retval #PH_ERR_PROTOCOL_ERROR Received response is not ISO/IEC 18092 complaint.
* \retval #PH_ERR_IO_TIMEOUT Timeout waiting for reply, e.g. target removal.
*/
phStatus_t phpalI18092mT_RlsRes(
    void * pDataParams,                                 /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pRlsReq,                                  /**< [In] Received RLS request bytes */
    uint16_t wRlsReqLength                              /**< [In] Received RLS request length */
    );

/**
* \brief Set the ATR response.
*
* \retval #PH_ERR_SUCCESS Operation successful, target deselected.
* \retval #PH_ERR_PROTOCOL_ERROR Received response is not ISO/IEC 18092 complaint.
* \retval #PH_ERR_IO_TIMEOUT Timeout waiting for reply, e.g. target removal.
*/
phStatus_t phpalI18092mT_SetAtrRes(
    void * pDataParams,                                 /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pNfcid3t,                                 /**< [In] NFCID3 */
    uint8_t bLrt,                                       /**< [In] Length Reduction of Target; 0-3  */
    uint8_t bNadEnable,                                 /**< [In] Enable usage of Node Address; Unequal '0' to enable. */
    uint8_t bBst,                                       /**< [In] Bit Rates Supported by Target in Sending Direction. */
    uint8_t bBrt,                                       /**< [In] Bit Rates Supported by Target in Receiving Direction. */
    uint8_t bTo,                                        /**< [In] Waiting Time (WT), used to code the Response Waiting Time (RWT). */
    uint8_t *pGt,                                       /**< [In] Optional General Information bytes. */
    uint8_t bGtLength                                   /**< [In] Number of General Information bytes. */
    );

/**
* \brief Start the listen action.
*
* \retval #PH_ERR_SUCCESS Operation successful, target deselected.
* \retval #PH_ERR_PROTOCOL_ERROR Received response is not ISO/IEC 18092 complaint.
* \retval #PH_ERR_IO_TIMEOUT Timeout waiting for reply, e.g. target removal.
*/
phStatus_t phpalI18092mT_Listen(
    void * pDataParams,                                 /**< [In] Pointer to this layer's parameter structure. */
    uint16_t wOption,                                   /**< [In] Option parameter. */
    uint8_t ** ppRxBuffer,                              /**< [Out] Received buffer. */
    uint16_t * pRxLength,                               /**< [Out] Received buffer length. */
    void * context                                      /**< [In] Application task context */
    );

/**
* \brief Set configuration parameter.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phpalI18092mT_SetConfig(
                                    void * pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                                    uint16_t wConfig,    /**< [In] Configuration Identifier. */
                                    uint16_t wValue      /**< [In] Configuration Value. */
                                    );

/**
* \brief Get configuration parameter.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phpalI18092mT_GetConfig(
                                    void * pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                                    uint16_t wConfig,    /**< [In] Configuration Identifier. */
                                    uint16_t * pValue    /**< [Out] Configuration Value. */
                                    );
/** @} */
#endif /* NXPBUILD__PHPAL_I18092MT */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif  /* PHPALI18092MT_H */

