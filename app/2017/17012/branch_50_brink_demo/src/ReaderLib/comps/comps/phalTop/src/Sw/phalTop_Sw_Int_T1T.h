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
* Internal functions for Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author: nxp66431 $
* $Revision: 336 $
* $Date: 2013-08-08 20:14:50 +0530 (Thu, 08 Aug 2013) $
*/

#ifndef PHALTOP_SW_INT_T1T_H
#define PHALTOP_SW_INT_T1T_H

#ifdef NXPBUILD__PHAL_TOP_SW

#define PHAL_TOP_T1T_INVALID_SEGMENT_ADDR                    0xEFU        /**< Invalid segment address */
#define PHAL_TOP_T1T_CC_LENGTH                               0x04U        /**< Length of T1T CC bytes */
#define PHAL_TOP_T1T_MAX_TMS                                 0x0800U      /**< Max. memory size for T1T */
#define PHAL_TOP_T1T_MAX_NDEF_TLV_LEN_FIELD                  0x03U        /**< Max. Length of NDEF TLV Length field */
#define PHAL_TOP_T1T_NDEF_TLV_HEADER_LEN                     0x01U        /**< NDEF TLV header(T field) length */

#define PHAL_TOP_T1T_CC_NMN_ADDR                               0x08U        /**< CC NMN byte address */
#define PHAL_TOP_T1T_CC_VNO_ADDR                               0x09U        /**< CC NDEF version number byte address */
#define PHAL_TOP_T1T_CC_TMS_ADDR                               0x0AU        /**< CC Tag Memory Size byte address */
#define PHAL_TOP_T1T_CC_RWA_ADDR                               0x0BU        /**< CC Read-write access byte address */

/** \name phalTop Custom Error Codes
*/
/** @{ */
#define PHAL_TOP_T1T_ERR_RESERVED_BYTE                         (PH_ERR_CUSTOM_BEGIN + 0)        /**< Tag operations error - Reserved/lock/OTP byte */
/** @} */

/**< Get the actual byte offset from read segment data */
#define PHAL_TOP_T1T_GET_SEG_DATA_OFFSET(seg, index)    \
    ((seg) != 0 ? ((index) + 1): ((index) + 2))
/**< Get length of TLV length field */
#define PHAL_TOP_T1T_GET_TLV_LEN_BYTES(len)                \
    ((len) > 0xFE? 3: 1)

/**
* \brief Calculate maximum possible NDEF message size.
*/
void phalTop_Sw_Int_T1T_CalculateMaxNdefSize(
    phalTop_T1T_t * pT1T            /**< [In] Pointer to this T1T parameter structure. */
    );

/**
* \brief Update the positions of lock/reserved/OTP bytes for current segment.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_UpdateLockReservedOtp(
    phalTop_T1T_t * pT1T    /**< [In] Pointer to this T1T parameter structure. */
    );

/**
* \brief Check whether current byte position is a lock/reserved/OTP bytes.
* \return Status code
* \retval #PH_ERR_SUCCESS - Current bytes is not a lock/reserved/OTP byte.
* \retval Others - Current bytes is a lock/reserved/OTP byte.
*/
phStatus_t phalTop_Sw_Int_T1T_CheckLockReservedOtp(
    phalTop_T1T_t * pT1T,    /**< [In] Pointer to this T1T parameter structure. */
    uint16_t wIndex            /**< [In] Current byte offset in segment. */
    );
/**
* \brief Detect all TLV block present in segment 0 of T1T.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_DetectTlvBlocks(
    phalTop_T1T_t * pT1T            /**< [In] Pointer to this T1T parameter structure. */
    );

/**
* \brief Check for NDEF message Presence and check for different TLVs
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_CheckNdef(
                                        phalTop_T1T_t * pT1T,                /**< [In] Pointer to this T1T parameter structure. */
                                        uint8_t * pNdefPresence                /**< [Out] NDEF data Presence; 1 - NDEF present, 0 - Not present. */
                                        );

/**
* \brief Format Tag to write NDEF message.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_FormatNdef(
    phalTop_T1T_t * pT1T                /**< [In] Pointer to this T1T parameter structure. */
    );

/**
* \brief Erase NDEF Message
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_EraseNdef(
                                        phalTop_T1T_t * pT1T                /**< [In] Pointer to this T1T parameter structure. */
                                        );

/**
* \brief Read NDEF message from Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_ReadNdef(
                                       phalTop_T1T_t * pT1T,                /**< [In] Pointer to this T1T parameter structure. */
                                       uint8_t * pData,                        /**< [Out] NDEF data from the Tag. */
                                       uint16_t * pLength                    /**< [Out] NDEF data length. */
                                       );

/**
* \brief Write NDEF message into Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_WriteNdef(
                                        phalTop_T1T_t * pT1T,                /**< [In] Pointer to this T1T parameter structure. */
                                        uint8_t * pData,                    /**< [In] NDEF data to be written to tag. */
                                        uint16_t wLength                    /**< [In] Length of NDEF data to be written. */
                                        );

/**
* \brief Set Tag Read Only.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_SetReadOnly(
                                          phalTop_T1T_t * pT1T              /**< [In] Pointer to this T1T parameter structure. */
                                          );

/**
* \brief Internal read function to read by block or byte based on tag type.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_Read(
                                   phalTop_T1T_t * pT1T,                    /**< [In] Pointer to this T1T parameter structure. */
                                   uint16_t wAddress,                       /**< [In] Byte address */
                                   uint8_t * pData,                         /**< [Out] Read Data */
                                   uint8_t bFreshRead                       /**< [In] Option flag to read again even if already read */
                                   );

/**
* \brief Internal write function to write by block or byte based on tag type.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T1T_Write(
                                   phalTop_T1T_t * pT1T,                    /**< [In] Pointer to this T1T parameter structure. */
                                   uint16_t wAddress,                       /**< [In] Byte address */
                                   uint8_t * pData,                         /**< [In] Write Data */
                                   uint16_t pDataIndex                      /**< [In] Data Index */
                                   );
#endif /* NXPBUILD__PHAL_TOP_SW */
#endif /* PHALTOP_SW_INT_T1T_H */
