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

#ifndef PHALTOP_SW_INT_T2T_H
#define PHALTOP_SW_INT_T2T_H

#ifdef NXPBUILD__PHAL_TOP_SW

#define PHAL_TOP_T2T_CC_LENGTH                                0x04U        /**< Length of T2T CC bytes */
#define PHAL_TOP_T2T_CC_BLOCK                                 0x03U        /**< CC block number */
#define PHAL_TOP_T2T_BYTES_PER_BLOCK                          0x04U        /**< Number of bytes per block */
#define PHAL_TOP_T2T_STATIC_LOCK_BLOCK                        0x02U        /**< Static lock block number */

#define PHAL_TOP_T2T_NDEF_TLV_HEADER_LEN                      0x01U        /**< NDEF TLV header(T field) length */

/**< Get length of TLV length field */
#define PHAL_TOP_T2T_GET_TLV_LEN_BYTES(len)                \
    ((len) > 0xFE? 3: 1)

/**
* \brief Calculate maximum possible NDEF message size.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
void phalTop_Sw_Int_T2T_CalculateMaxNdefSize(
    phalTop_T2T_t * pT2T            /**< [In] Pointer to this T2T parameter structure. */
    );

/**
* \brief Update the positions of lock/reserved/OTP bytes for current segment.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
    phalTop_T2T_t * pT2T    /**< [In] Pointer to this T2T parameter structure. */
    );

/**
* \brief Check whether current byte position is a lock/reserved/OTP bytes.
* \return Status code
* \retval #PH_ERR_SUCCESS - Current bytes is not a lock/reserved/OTP byte.
* \retval Others - Current bytes is a lock/reserved/OTP byte.
*/
phStatus_t phalTop_Sw_Int_T2T_CheckLockReservedOtp(
    phalTop_T2T_t * pT2T,    /**< [In] Pointer to this T2T parameter structure. */
    uint16_t wIndex            /**< [In] Current byte offset in segment. */
    );
/**
* \brief Detect all TLV block present in segment 0 of T2T.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_DetectTlvBlocks(
    phalTop_T2T_t * pT2T            /**< [In] Pointer to this T2T parameter structure. */
    );

/**
* \brief Check for NDEF message Presence and check for different TLVs
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_CheckNdef(
                                        phalTop_T2T_t * pT2T,                /**< [In] Pointer to this T2T parameter structure. */
                                        uint8_t * pNdefPresence                /**< [Out] NDEF data Presence; 1 - NDEF present, 0 - Not present. */
                                        );

/**
* \brief Format Tag to write NDEF message.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_FormatNdef(
    phalTop_T2T_t * pT2T                /**< [In] Pointer to this T2T parameter structure. */
    );

/**
* \brief Erase NDEF Message
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_EraseNdef(
                                        phalTop_T2T_t * pT2T                /**< [In] Pointer to this T2T parameter structure. */
                                        );

/**
* \brief Read NDEF message from Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_ReadNdef(
                                       phalTop_T2T_t * pT2T,                /**< [In] Pointer to this T2T parameter structure. */
                                       uint8_t * pData,                        /**< [Out] NDEF data from the Tag. */
                                       uint16_t * pLength                    /**< [Out] NDEF data length. */
                                       );

/**
* \brief Write NDEF message into Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_WriteNdef(
                                        phalTop_T2T_t * pT2T,                /**< [In] Pointer to this T2T parameter structure. */
                                        uint8_t * pData,                    /**< [In] NDEF data to be written to tag. */
                                        uint16_t wLength                    /**< [In] Length of NDEF data to be written. */
                                        );

/**
* \brief Set NDEF Tag to read only.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T2T_SetReadOnly(
                                          phalTop_T2T_t * pT2T               /**< [In] Pointer to this T2T parameter structure. */
                                          );
#endif /* NXPBUILD__PHAL_TOP_SW */
#endif /* PHALTOP_SW_INT_T2T_H */
