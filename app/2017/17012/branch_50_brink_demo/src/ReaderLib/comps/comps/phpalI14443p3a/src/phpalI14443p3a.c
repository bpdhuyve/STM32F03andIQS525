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
* Generic ISO14443-3A Component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 161 $
* $Date: 2013-06-05 14:04:36 +0530 (Wed, 05 Jun 2013) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#include <ph_Status.h>
#include <phpalI14443p3a.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHPAL_I14443P3A_SW
#include "Sw/phpalI14443p3a_Sw.h"
#endif /* NXPBUILD__PHPAL_I14443P3A_SW */

#ifdef NXPBUILD__PHPAL_I14443P3A

phStatus_t phpalI14443p3a_RequestA(
                                   void * pDataParams,
                                   uint8_t * pAtqa
                                   )
{
	phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_Sw_RequestA((phpalI14443p3a_Sw_DataParams_t *)pDataParams, pAtqa);
    return status;
}

phStatus_t phpalI14443p3a_WakeUpA(
                                  void * pDataParams,
                                  uint8_t * pAtqa
                                  )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_Sw_WakeUpA((phpalI14443p3a_Sw_DataParams_t *)pDataParams, pAtqa);
    return status;
}

phStatus_t phpalI14443p3a_HaltA(
                                void * pDataParams
                                )
{
    phStatus_t PH_MEMLOC_REM status;


    status = phpalI14443p3a_Sw_HaltA((phpalI14443p3a_Sw_DataParams_t *)pDataParams);
    return status;
}

phStatus_t phpalI14443p3a_Anticollision(
                                        void * pDataParams,
                                        uint8_t bCascadeLevel,
                                        uint8_t * pUidIn,
                                        uint8_t bNvbUidIn,
                                        uint8_t * pUidOut,
                                        uint8_t * pNvbUidOut
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_Sw_Anticollision((phpalI14443p3a_Sw_DataParams_t *)pDataParams, bCascadeLevel, pUidIn, bNvbUidIn, pUidOut, pNvbUidOut);
    return status;
}

phStatus_t phpalI14443p3a_Select(
                                 void * pDataParams,
                                 uint8_t bCascadeLevel,
                                 uint8_t * pUidIn,
                                 uint8_t * pSak
                                 )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_Sw_Select((phpalI14443p3a_Sw_DataParams_t *)pDataParams, bCascadeLevel, pUidIn, pSak);
    return status;
}

phStatus_t phpalI14443p3a_ActivateCard(
                                       void * pDataParams,
                                       uint8_t * pUidIn,
                                       uint8_t bLenUidIn,
                                       uint8_t * pUidOut,
                                       uint8_t * pLenUidOut,
                                       uint8_t * pSak,
                                       uint8_t * pMoreCardsAvailable
                                       )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_Sw_ActivateCard((phpalI14443p3a_Sw_DataParams_t *)pDataParams, pUidIn, bLenUidIn, pUidOut, pLenUidOut, pSak, pMoreCardsAvailable);
    return status;
}

phStatus_t phpalI14443p3a_Exchange(
                                   void * pDataParams,
                                   uint16_t wOption,
                                   uint8_t * pTxBuffer,
                                   uint16_t wTxLength,
                                   uint8_t ** ppRxBuffer,
                                   uint16_t * pRxLength
                                   )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_Sw_Exchange((phpalI14443p3a_Sw_DataParams_t *)pDataParams, wOption, pTxBuffer, wTxLength, ppRxBuffer, pRxLength);
    return status;
}

phStatus_t phpalI14443p3a_GetSerialNo(
                                      void * pDataParams,
                                      uint8_t * pUidOut,
                                      uint8_t * pLenUidOut
                                      )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_Sw_GetSerialNo((phpalI14443p3a_Sw_DataParams_t *)pDataParams, pUidOut, pLenUidOut);
    return status;
}

phStatus_t phpalI14443p3a_T1T_Exchange(
                                         void * pDataParams,                             /**< [In] */
                                         uint16_t wOption,                               /**< [In] */
                                         uint8_t * pTxBuffer,                            /**< [In] */
                                         uint16_t wTxLength,                             /**< [In] */
                                         uint8_t ** ppRxBuffer,                          /**< [Out] */
                                         uint16_t * pRxLength                            /**< [Out] */
                                      )
{

    phStatus_t PH_MEMLOC_REM status;
    status = phpalI14443p3a_T1T_Sw_Exchange(
                (phpalI14443p3a_Sw_DataParams_t *)pDataParams,
                wOption,
                pTxBuffer,
                wTxLength,
                ppRxBuffer,
                pRxLength
                );
    return status;
}

#endif /* NXPBUILD__PHPAL_I14443P3A */
