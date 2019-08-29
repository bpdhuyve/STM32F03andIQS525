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
* Software Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author: nxp66431 $
* $Revision: 336 $
* $Date: 2013-08-08 20:14:50 +0530 (Thu, 08 Aug 2013) $
*/

#ifndef PHALTOP_SW_H
#define PHALTOP_SW_H

#ifdef NXPBUILD__PHAL_TOP_SW

/**
* \brief Check for NDEF message Presence and check for different TLVs
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_CheckNdef(
                                phalTop_Sw_DataParams_t * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                uint8_t * pNdefPresence                    /**< [Out] NDEF data Presence; 1 - NDEF present, 0 - Not present. */
                                );

/**
* \brief Format Tag to write NDEF message.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_FormatNdef(
                                 phalTop_Sw_DataParams_t * pDataParams    /**< [In] Pointer to this layer's parameter structure. */
                                 );

/**
* \brief Erase NDEF Message
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_EraseNdef(
                                phalTop_Sw_DataParams_t * pDataParams    /**< [In] Pointer to this layer's parameter structure. */
                                );

/**
* \brief Reset Tag Operation parameters
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Reset(
                            phalTop_Sw_DataParams_t * pDataParams        /**< [In] Pointer to this layer's parameter structure. */
                            );

/**
* \brief Set configuration parameter.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_SetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                uint16_t wConfig,                        /**< [In] Configuration Identifier. */
                                uint16_t wValue                            /**< [In] Configuration Value. */
                                );

/**
* \brief Get configuration parameter.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_GetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                uint16_t wConfig,                        /**< [In] Configuration Identifier. */
                                uint16_t  * wValue                        /**< [Out] Configuration Value. */
                                );

/**
* \brief Read NDEF message from Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_ReadNdef(
                               phalTop_Sw_DataParams_t * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                               uint8_t * pData,                            /**< [Out] NDEF data from the Tag. */
                               uint16_t * pLength                        /**< [Out] NDEF data length. */
                               );

/**
* \brief Write NDEF message into Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_WriteNdef(
                                phalTop_Sw_DataParams_t * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                uint8_t * pData,                        /**< [In] NDEF data to be written to tag. */
                                uint16_t wLength                        /**< [In] Length of NDEF data to be written. */
                                );

#ifdef __DEBUG

phStatus_t phalTop_Sw_SetPtr(
                             phalTop_Sw_DataParams_t * pDataParams,
                             void * pT1T,
                             void * pT2T,
                             void * pT3T,
                             void * pT4T
                             );
#endif /* __DEBUG */

#endif /* NXPBUILD__PHAL_TOP_SW */
#endif /* PHALTOP_SW_H */
