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
* Generic MIFARE(R) Application Component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 161 $
* $Date: 2013-06-05 14:04:36 +0530 (Wed, 05 Jun 2013) $
*
* History:
*  CHu: Generated 31. July 2009
*
*/

#include <ph_Status.h>
#include <phalMfc.h>
#include "phalMfc_Int.h"
#include <phhalHw.h>
#include <phpalMifare.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHAL_MFC_SW
#include "Sw/phalMfc_Sw.h"
#endif /* NXPBUILD__PHAL_MFC_SW */

#ifdef NXPBUILD__PHAL_MFC

phStatus_t phalMfc_Authenticate(
                                void * pDataParams,
                                uint8_t bBlockNo,
                                uint8_t bKeyType,
                                uint16_t wKeyNumber,
                                uint16_t wKeyVersion,
                                uint8_t * pUid,
                                uint8_t bUidLength
                                )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_Authenticate((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo, bKeyType, wKeyNumber, wKeyVersion, pUid, bUidLength);
    return status;
}

phStatus_t phalMfc_Read(
                        void * pDataParams,
                        uint8_t bBlockNo,
                        uint8_t * pBlockData
                        )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_Read((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo, pBlockData);
    return status;
}

phStatus_t phalMfc_ReadValue(
                             void * pDataParams,
                             uint8_t bBlockNo,
                             uint8_t * pValue,
                             uint8_t * pAddrData
                             )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_ReadValue((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo, pValue, pAddrData);
    return status;
}

phStatus_t phalMfc_Write(
                         void * pDataParams,
                         uint8_t bBlockNo,
                         uint8_t * pBlockData
                         )
{
    phStatus_t PH_MEMLOC_REM status;

    status = phalMfc_Sw_Write((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo, pBlockData);
    return status;
}

phStatus_t phalMfc_WriteValue(
                              void * pDataParams,
                              uint8_t bBlockNo,
                              uint8_t * pValue,
                              uint8_t bAddrData
                              )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_WriteValue((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo, pValue, bAddrData);
    return status;
}

phStatus_t phalMfc_Increment(
                             void * pDataParams,
                             uint8_t bBlockNo,
                             uint8_t * pValue
                             )
{
    phStatus_t PH_MEMLOC_REM status;


    status = phalMfc_Sw_Increment((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo, pValue);
    return status;
}

phStatus_t phalMfc_Decrement(
                             void * pDataParams,
                             uint8_t bBlockNo,
                             uint8_t * pValue
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    
    status = phalMfc_Sw_Decrement((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo, pValue);
    return status;
}

phStatus_t phalMfc_Transfer(
                            void * pDataParams,
                            uint8_t bBlockNo
                            )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_Transfer((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo);
    return status;
}

phStatus_t phalMfc_Restore(
                           void * pDataParams,
                           uint8_t bBlockNo
                           )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_Restore((phalMfc_Sw_DataParams_t *)pDataParams, bBlockNo);
    return status;
}

phStatus_t phalMfc_IncrementTransfer(
                                     void * pDataParams,
                                     uint8_t bSrcBlockNo,
                                     uint8_t bDstBlockNo,
                                     uint8_t * pValue
                                     )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_IncrementTransfer((phalMfc_Sw_DataParams_t *)pDataParams, bSrcBlockNo, bDstBlockNo, pValue);
    return status;
}

phStatus_t phalMfc_DecrementTransfer(
                                     void * pDataParams,
                                     uint8_t bSrcBlockNo,
                                     uint8_t bDstBlockNo,
                                     uint8_t * pValue
                                     )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_DecrementTransfer((phalMfc_Sw_DataParams_t *)pDataParams, bSrcBlockNo, bDstBlockNo, pValue);
    return status;
}

phStatus_t phalMfc_RestoreTransfer(
                                   void * pDataParams,
                                   uint8_t bSrcBlockNo,
                                   uint8_t bDstBlockNo
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    status = phalMfc_Sw_RestoreTransfer((phalMfc_Sw_DataParams_t *)pDataParams, bSrcBlockNo, bDstBlockNo);
    return status;
}

phStatus_t phalMfc_PersonalizeUid(
                                  void * pDataParams,
                                  uint8_t bUidType
                                  )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phalMfc_Sw_PersonalizeUid((phalMfc_Sw_DataParams_t *)pDataParams, bUidType);
    return status;
}
#endif /* NXPBUILD__PHAL_MFC */
