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
* Software ISO18092 target mode Component of Reader Library Framework.
* $Author: nxp65585 $
* $Revision: 327 $
* $Date: 2013-08-02 16:23:39 +0530 (Fri, 02 Aug 2013) $
*
* History:
*  CEn: Generated 07. September 2010
*/

#ifndef PHPALI18092MT_SW_H
#define PHPALI18092MT_SW_H


#include <phpalI18092mT.h>


phStatus_t phpalI18092mT_Sw_ResetProtocol(
    phpalI18092mT_Sw_DataParams_t * pDataParams
    );

phStatus_t phpalI18092mT_Sw_AtrRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pAtr,
    uint16_t wAtrLength   
    );

phStatus_t phpalI18092mT_Sw_DepSend(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t * pTxBuffer,
    uint16_t wTxLength
    );

phStatus_t phpalI18092mT_Sw_DepRecv(
        phpalI18092mT_Sw_DataParams_t * pDataParams,
        uint16_t wOption,
        uint8_t ** ppRxBuffer,
        uint16_t * pRxLength);

phStatus_t phpalI18092mT_Sw_DslRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pDslReq,
    uint8_t wDslReqLength
    );

phStatus_t phpalI18092mT_Sw_PslRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pPslReq,
    uint16_t  wPslReqLength
    );

phStatus_t phpalI18092mT_Sw_RlsRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pRlsReq,
    uint16_t wRlsReqLength
    );

phStatus_t phpalI18092mT_Sw_SetAtrRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pNfcid3t,
    uint8_t bLrt,
    uint8_t bNadEnable,
    uint8_t bBst,
    uint8_t bBrt,
    uint8_t bTo,
    uint8_t *pGt,
    uint8_t bGtLength
    );

phStatus_t phpalI18092mT_Sw_Listen(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t ** ppRxBuffer,
    uint16_t * pRxLength,
    void * context
    );

phStatus_t phpalI18092mT_Sw_HandleChainedData(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t ** ppRxBuffer,
    uint16_t * pRxLength
    );

phStatus_t phpalI18092mT_Sw_SetConfig(
   phpalI18092mT_Sw_DataParams_t * pDataParams,
   uint16_t wConfig,
   uint16_t wValue
   );

phStatus_t phpalI18092mT_Sw_GetConfig(
   phpalI18092mT_Sw_DataParams_t * pDataParams,
   uint16_t wConfig,
   uint16_t * pValue
   );

#endif  /* PHPALI18092MT_SW_H */
