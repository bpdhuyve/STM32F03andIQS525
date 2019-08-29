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
* $Author: nxp67988 $
* $Revision: 336 $
* $Date: 2013-08-08 20:14:50 +0530 (Thu, 08 Aug 2013) $
*/

#ifndef PHALTOP_SW_INT_T4T_H
#define PHALTOP_SW_INT_T4T_H

#ifdef NXPBUILD__PHAL_TOP_SW

#define PHAL_TOP_T4T_NDEF_APP_ID                            0x000001         /* Application id of the NDEF Application */
#define PHAL_TOP_T4T_ISOFILEID_APP                          0x05E1           /* ISO file id of the NDEF application */
#define PHAL_TOP_T4T_SELECT_EF_ID                           0x02U            /* File Selection by EF Id */
#define PHAL_TOP_T4T_SELECT_DF_NAME                         0x04U            /* File Selection by DF Name */
#define PHAL_TOP_T4T_CCLEN_OFFSET                           0x00U            /* Offset value of CCLEN */
#define PHAL_TOP_T4T_VERSION_OFFSET                         0x02U            /* Offset value of Mapping Version */
#define PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET             0x03U            /* Offset value of MLe (bytes) i.e Maximum R-APDU data size */
#define PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET                  0x05U            /* Offset value of MLc (bytes) i.e Maximum C-APDU data size */
#define PHAL_TOP_T4T_NDEFTLV_OFFSET                         0x07U            /* Offset value of NDEF File Control TLV */

/**
* \brief Check for NDEF message Presence and check for different TLVs
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T4T_CheckNdef(
                                        phalTop_T4T_t * pT4T,                /**< [In] Pointer to this T4T parameter structure. */
                                        uint8_t * pNdefPresence              /**< [Out] NDEF data Presence; true - NDEF present, false - Not present. */
                                        );

/**
* \brief Read NDEF message from Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T4T_ReadNdef(
                                       phalTop_T4T_t * pT4T,
                                       uint8_t * pData,
                                       uint16_t * pLength
                                       );
/**
* \brief Write NDEF message into Tag.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T4T_WriteNdef(
                                        phalTop_T4T_t * pT4T,
                                        uint8_t * pData,
                                        uint16_t wLength
                                        );

/**
* \brief Format Tag to write NDEF message.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T4T_FormatNdef(
                                         phalTop_T4T_t * pT4T                /**< [In] Pointer to this T4T parameter structure. */
                                         );

/**
* \brief Erase NDEF Message
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_Sw_Int_T4T_EraseNdef(
                                        phalTop_T4T_t * pT4T                 /**< [In] Pointer to this T4T parameter structure. */
                                        );


#endif /* NXPBUILD__PHAL_TOP_SW */
#endif /* PHALTOP_SW_INT_T4T_H */
