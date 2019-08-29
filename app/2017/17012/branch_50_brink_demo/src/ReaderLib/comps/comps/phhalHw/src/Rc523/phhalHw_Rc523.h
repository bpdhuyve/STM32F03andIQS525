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
* RC523 specific HAL-Component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 161 $
* $Date: 2013-06-05 14:04:36 +0530 (Wed, 05 Jun 2013) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHHALHW_RC523_H
#define PHHALHW_RC523_H

#include <ph_Status.h>
#include <phhalHw.h>

#define PHHAL_HW_RC523_FIFOSIZE         64U         /**< Size of RC internal FiFo buffer */
#define PHHAL_HW_RC523_DEFAULT_TIMEOUT  150U        /**< Default timeout in microseconds */

phStatus_t phhalHw_Rc523_Exchange(
                                  phhalHw_Rc523_DataParams_t * pDataParams,
                                  uint16_t wOption,
                                  uint8_t * pTxBuffer,
                                  uint16_t wTxLength,
                                  uint8_t ** ppRxBuffer,
                                  uint16_t * pRxLength
                                  );

phStatus_t phhalHw_Rc523_WriteRegister(
                                       phhalHw_Rc523_DataParams_t * pDataParams,
                                       uint8_t bAddress,
                                       uint8_t bValue
                                       );

phStatus_t phhalHw_Rc523_ReadRegister(
                                      phhalHw_Rc523_DataParams_t * pDataParams,
                                      uint8_t bAddress,
                                      uint8_t * pValue
                                      );

phStatus_t phhalHw_Rc523_ApplyProtocolSettings(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t bCardType
    );

phStatus_t phhalHw_Rc523_MfcAuthenticateKeyNo(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t bBlockNo,
    uint8_t bKeyType,
    uint16_t wKeyNo,
    uint16_t wKeyVersion,
    uint8_t * pUid
    );

phStatus_t phhalHw_Rc523_MfcAuthenticate(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t bBlockNo,
    uint8_t bKeyType,
    uint8_t * pKey,
    uint8_t * pUid
    );

phStatus_t phhalHw_Rc523_SetConfig(
                                   phhalHw_Rc523_DataParams_t * pDataParams,
                                   uint16_t wConfig,
                                   uint16_t wValue
                                   );

phStatus_t phhalHw_Rc523_GetConfig(
                                   phhalHw_Rc523_DataParams_t * pDataParams,
                                   uint16_t wConfig,
                                   uint16_t * pValue
                                   );

phStatus_t phhalHw_Rc523_FieldOn(
                                 phhalHw_Rc523_DataParams_t * pDataParams
                                 );

phStatus_t phhalHw_Rc523_FieldOff(
                                  phhalHw_Rc523_DataParams_t * pDataParams
                                  );

phStatus_t phhalHw_Rc523_FieldReset(
                                    phhalHw_Rc523_DataParams_t * pDataParams
                                    );

phStatus_t phhalHw_Rc523_Wait(
                              phhalHw_Rc523_DataParams_t * pDataParams,
                              uint8_t bUnit,
                              uint16_t wTimeout
                              );

phStatus_t phhalHw_Rc523_SetMinFDT(
                                  phhalHw_Rc523_DataParams_t * pDataParams,
                                  uint16_t wValue
                                  );

/**
* \brief Stores data for \ref phhalHw_Rc523_Cmd_Autocoll command into internal buffer.
*
* \b Note: Only applicable if in Target mode (e.g. #PHHAL_HW_CARDTYPE_I18092MPT).
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER \c bDataLength is neither zero nor 25.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phhalHw_Rc523_Config(
                                    phhalHw_Rc523_DataParams_t * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                                    uint8_t * pSensRes,                         /**< [In] 2 bytes. */
                                    /** [In] 3 bytes; The first NfcId1 byte is fixed to \c 08h and the check byte is calculated automatically. */
                                    uint8_t * pNfcId1,
                                    uint8_t SelRes,                             /**< [In] 1 byte. */
                                    /** [In] 2 bytes (shall be 01h, FEh) + 6 bytes NfcId2 + 8 bytes Pad + 2 bytes SystemCode. */
                                    uint8_t * pPollingResp,
                                    uint8_t bNfcId3                             /**< [In] 1 byte. */
                                    );

/**
* \brief Handles Felica polling or MIFARE anticollision.
*
* Only applicable if in Target mode (e.g. #PHHAL_HW_CARDTYPE_I18092MPT).\n
* \b Note: Ensure that the correct configuration has been loaded using the \ref phhalHw_Rc523_Cmd_Config command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_IO_TIMEOUT No response received.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phhalHw_Rc523_Autocoll(
                                      phhalHw_Rc523_DataParams_t * pDataParams, /**< [In] Pointer to this layer's parameter structure. */
                                      uint8_t ** ppRxBuffer,                    /**< [Out] Pointer to received data. */
                                      uint16_t * pRxLength                      /**< [Out] Number of received data bytes. */
                                      );


#endif /* PHHALHW_RC523_H */
