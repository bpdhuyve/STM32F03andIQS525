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

#ifndef PHALTOP_SW_INT_T3T_H
#define PHALTOP_SW_INT_T3T_H

#ifdef NXPBUILD__PHAL_TOP_SW

/**
* \brief Check for NDEF message Presence and check for different TLVs
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T3T_CheckNdef(
                                        phalTop_T3T_t * pT3T,                /**< [In] Pointer to this T3T parameter structure. */
                                        uint8_t * pNdefPresence                /**< [Out] NDEF data Presence; 1 - NDEF present, 0 - Not present. */
                                        );

/**
* \brief Format Tag to write NDEF message.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T3T_FormatNdef(
    phalTop_T3T_t * pT3T                /**< [In] Pointer to this T3T parameter structure. */
    );

/**
* \brief Erase NDEF Message
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T3T_EraseNdef(
                                        phalTop_T3T_t * pT3T                /**< [In] Pointer to this T3T parameter structure. */
                                        );

/**
* \brief Read NDEF message from Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T3T_ReadNdef(
                                       phalTop_T3T_t * pT3T,                /**< [In] Pointer to this T3T parameter structure. */
                                       uint8_t * pData,                        /**< [Out] NDEF data from the Tag. */
                                       uint16_t * pLength                    /**< [Out] NDEF data length. */
                                       );

/**
* \brief Write NDEF message into Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T3T_WriteNdef(
                                        phalTop_T3T_t * pT3T,                /**< [In] Pointer to this T3T parameter structure. */
                                        uint8_t * pData,                    /**< [In] NDEF data to be written to tag. */
                                        uint16_t wLength                    /**< [In] Length of NDEF data to be written. */
                                        );
#endif /* NXPBUILD__PHAL_TOP_SW */
#endif /* PHALTOP_SW_INT_T3T_H */
