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

#include <ph_Typedefs.h>
#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phacDiscLoop.h>
#include <phalT1T.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW

#include "phalTop_Sw.h"
#include "phalTop_Sw_Int_T1T.h"
#include "phalTop_Sw_Int_T2T.h"
#include "phalTop_Sw_Int_T3T.h"
#include "phalTop_Sw_Int_T4T.h"

phStatus_t phalTop_Sw_CheckNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pNdefPresence
                                )
{
    phStatus_t PH_MEMLOC_REM status;

    switch(pDataParams->bTagType)
    {
    case PHAL_TOP_TAG_TYPE_T1T_TAG:
        status = phalTop_Sw_Int_T1T_CheckNdef(
            pDataParams->pT1T,
            pNdefPresence);
        break;

    case PHAL_TOP_TAG_TYPE_T2T_TAG:
        status = phalTop_Sw_Int_T2T_CheckNdef(
            pDataParams->pT2T,
            pNdefPresence);
        break;

    case PHAL_TOP_TAG_TYPE_T3T_TAG:
        status = phalTop_Sw_Int_T3T_CheckNdef(
            pDataParams->pT3T,
            pNdefPresence);
        break;

    case PHAL_TOP_TAG_TYPE_T4T_TAG:
        status = phalTop_Sw_Int_T4T_CheckNdef(
            pDataParams->pT4T,
            pNdefPresence);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        break;
    }

    return status;
}

phStatus_t phalTop_Sw_FormatNdef(
                                 phalTop_Sw_DataParams_t * pDataParams
                                 )
{
    phStatus_t PH_MEMLOC_REM status;

    switch(pDataParams->bTagType)
    {
    case PHAL_TOP_TAG_TYPE_T1T_TAG:
        status = phalTop_Sw_Int_T1T_FormatNdef(
            pDataParams->pT1T);
        break;

    case PHAL_TOP_TAG_TYPE_T2T_TAG:
        status = phalTop_Sw_Int_T2T_FormatNdef(
            pDataParams->pT2T);
        break;

    case PHAL_TOP_TAG_TYPE_T3T_TAG:
        status = phalTop_Sw_Int_T3T_FormatNdef(
            pDataParams->pT3T);
        break;

    case PHAL_TOP_TAG_TYPE_T4T_TAG:
        status = phalTop_Sw_Int_T4T_FormatNdef(
            pDataParams->pT4T);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        break;
    }

    return status;
}

phStatus_t phalTop_Sw_EraseNdef(
                                phalTop_Sw_DataParams_t * pDataParams
                                )
{
    phStatus_t PH_MEMLOC_REM status;

    switch(pDataParams->bTagType)
    {
    case PHAL_TOP_TAG_TYPE_T1T_TAG:
        status = phalTop_Sw_Int_T1T_EraseNdef(
            pDataParams->pT1T);
        break;

    case PHAL_TOP_TAG_TYPE_T2T_TAG:
        status = phalTop_Sw_Int_T2T_EraseNdef(
            pDataParams->pT2T);
        break;

    case PHAL_TOP_TAG_TYPE_T3T_TAG:
        status = phalTop_Sw_Int_T3T_EraseNdef(
            pDataParams->pT3T);
        break;

    case PHAL_TOP_TAG_TYPE_T4T_TAG:
        status = phalTop_Sw_Int_T4T_EraseNdef(
            pDataParams->pT4T);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        break;
    }

    return status;
}

phStatus_t phalTop_Sw_Reset(
                            phalTop_Sw_DataParams_t * pDataParams
                            )
{
    /* Apply T1T default parameters */
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bTagState                 = PHAL_TOP_T1T_STATE_UNKNOWN;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bLockTlvCount             = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bMemoryTlvCount           = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bNdefFormatted            = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bNdefPresence             = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bNdefSupport              = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bProprietaryTlvCount      = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bRwa                      = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bTagMemoryType            = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bTerminatorTlvPresence    = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bTms                      = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->bVno                      = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->wMaxNdefLength            = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->wNdefHeaderAddr           = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->wNdefLength               = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->wNdefMsgAddr              = 0;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->sSegment.bValidity        = PH_OFF;
    ((phalTop_T1T_t *)(pDataParams->pT1T))->sSegment.bAddress         = PHAL_TOP_T1T_INVALID_SEGMENT_ADDR;

    /* Apply T2T default parameters */
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bTagState                 = PHAL_TOP_T2T_STATE_UNKNOWN;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bEraseProprietaryTlv      = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bLockTlvCount             = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bMemoryTlvCount           = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bNdefFormatted            = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bNdefPresence             = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bProprietaryTlvCount      = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bRwa                      = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bTagMemoryType            = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bTerminatorTlvPresence    = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bTms                      = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->bVno                      = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->wMaxNdefLength            = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->wNdefHeaderAddr           = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->wNdefLength               = 0;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->wNdefMsgAddr              = 0;

    /* Apply T3T default parameters */
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bTagState                 = PHAL_TOP_T3T_STATE_UNKNOWN;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bNdefFormatted            = 0;
    ((phalTop_T1T_t *)(pDataParams->pT3T))->bNdefSupport              = 0;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bNdefPresence             = 0;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bRwa                      = 0;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bVno                      = 0;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->wNdefLength               = 0;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bNbr                      = 0;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bNbw                      = 0;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->bNmaxb                    = 0;

    /* Apply T4T default parameters */
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bTagState                 = PHAL_TOP_T4T_STATE_UNKNOWN;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdefFormatted            = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdefPresence             = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdefSupport              = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bVno                      = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bRa                       = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bWa                       = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->wCCLEN                    = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdef_FileID[0]           = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdef_FileID[1]           = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->bMaxFileSize              = 0;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->wNLEN                     = 0;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Init(
                           phalTop_Sw_DataParams_t * pDataParams,
                           uint16_t wSizeOfDataParams,
                           void * pTopT1T,
                           void * pTopT2T,
                           void * pTopT3T,
                           void * pTopT4T,
                           void * pTopT5T
                           )
{
    if (sizeof(phalTop_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pTopT1T);
    PH_ASSERT_NULL (pTopT2T);
    PH_ASSERT_NULL (pTopT3T);
    PH_ASSERT_NULL (pTopT4T);

    pDataParams->wId = PH_COMP_AL_TOP | PHAL_TOP_SW_ID;
    pDataParams->pT1T = pTopT1T;
    pDataParams->pT2T = pTopT2T;
    pDataParams->pT3T = pTopT3T;
    pDataParams->pT4T = pTopT4T;

    return phalTop_Sw_Reset(pDataParams);
}

phStatus_t phalTop_Sw_SetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint16_t wConfig,
                                uint16_t wValue
                                )
{
    phStatus_t PH_MEMLOC_REM status;

    switch(wConfig)
    {
    case PHAL_TOP_CONFIG_TAG_TYPE:
        if((wValue != PHAL_TOP_TAG_TYPE_T1T_TAG) &&
            (wValue != PHAL_TOP_TAG_TYPE_T2T_TAG) &&
            (wValue != PHAL_TOP_TAG_TYPE_T3T_TAG) &&
            (wValue != PHAL_TOP_TAG_TYPE_T4T_TAG)) 
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        pDataParams->bTagType = (uint8_t)wValue;
        break;

    case PHAL_TOP_CONFIG_T1T_NDEF_VNO:
        if(wValue != PHAL_TOP_T1T_NDEF_SUPPORTED_VNO)
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T1T_t *)(pDataParams->pT1T))->bVno = (uint8_t)wValue;
        break;

    case PHAL_TOP_CONFIG_T1T_TMS:
        if((wValue > PHAL_TOP_T1T_MAX_TMS) || (wValue == 0) || (wValue < 0x0E))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T1T_t *)(pDataParams->pT1T))->bTms = (uint8_t)((wValue / 8) - 1);
        break;

    case PHAL_TOP_CONFIG_T1T_RW_ACCESS:
        if((wValue != PHAL_TOP_T1T_CC_RWA_RO) &&
            (wValue != PHAL_TOP_T1T_CC_RWA_RW))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T1T_t *)(pDataParams->pT1T))->bRwa = (uint8_t)(wValue);

        if(((phalTop_T1T_t *)(pDataParams->pT1T))->bRwa)
        {
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_SetReadOnly(pDataParams->pT1T));
        }
        break;

    case PHAL_TOP_CONFIG_T1T_TERMINATOR_TLV:
        if((wValue != PH_ON) && (wValue != PH_OFF))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T1T_t *)(pDataParams->pT1T))->bTerminatorTlvPresence = (uint8_t)(wValue);
        break;

    case PHAL_TOP_CONFIG_T2T_NDEF_VNO:
        if(wValue != PHAL_TOP_T2T_NDEF_SUPPORTED_VNO)
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T2T_t *)(pDataParams->pT2T))->bVno = (uint8_t)wValue;
        break;

    case PHAL_TOP_CONFIG_T2T_TMS:
        if(wValue < 0x30)
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T2T_t *)(pDataParams->pT2T))->bTms = (uint8_t)(wValue / 8);
        break;

    case PHAL_TOP_CONFIG_T2T_RW_ACCESS:
        if((wValue != PHAL_TOP_T2T_CC_RWA_RO) &&
            (wValue != PHAL_TOP_T2T_CC_RWA_RW))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T2T_t *)(pDataParams->pT2T))->bRwa = (uint8_t)(wValue);

        if(((phalTop_T2T_t *)(pDataParams->pT2T))->bRwa)
        {
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_SetReadOnly(pDataParams->pT2T));
        }
        break;

    case PHAL_TOP_CONFIG_T2T_TERMINATOR_TLV:
        if((wValue != PH_ON) && (wValue != PH_OFF))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T2T_t *)(pDataParams->pT2T))->bTerminatorTlvPresence = (uint8_t)(wValue);
        break;

    case PHAL_TOP_CONFIG_T2T_PROPRIETARY_TLV:
        if((wValue != PH_ON) && (wValue != PH_OFF))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T2T_t *)(pDataParams->pT2T))->bEraseProprietaryTlv = (uint8_t)(wValue);
        break;

    case PHAL_TOP_CONFIG_T3T_NDEF_VNO:
        if((wValue != PHAL_TOP_T3T_NDEF_SUPPORTED_VNO1) &&
            (wValue != PHAL_TOP_T3T_NDEF_SUPPORTED_VNO2))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T3T_t *)(pDataParams->pT3T))->bVno = (uint8_t)(wValue);
        break;

    case PHAL_TOP_CONFIG_T3T_MAX_NDEF_LENGTH:
        /* Nmaxb of  Felica Lite is 208 Bytes*/
        if(wValue < (0x000D * PHAL_TOP_T3T_BLOCK_SIZE))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T3T_t *)(pDataParams->pT3T))->bNmaxb = (wValue / PHAL_TOP_T3T_BLOCK_SIZE);
        if (wValue % PHAL_TOP_T3T_BLOCK_SIZE)
        {
            ((phalTop_T3T_t *)(pDataParams->pT3T))->bNmaxb += 1;
        }
        break;

    case PHAL_TOP_CONFIG_T3T_NBR:
        if((wValue == 0x00) || (wValue > 0x04))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T3T_t *)(pDataParams->pT3T))->bNbr = (uint8_t)(wValue);
        break;

    case PHAL_TOP_CONFIG_T3T_NBW:
        if(wValue != 0x01)
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T3T_t *)(pDataParams->pT3T))->bNbw = (uint8_t)(wValue);
        break;

    case PHAL_TOP_CONFIG_T4T_NDEF_FILE_ID:
        if((wValue == 0x0000) ||
            (wValue == 0xE102) ||
            (wValue == 0xE103) ||
            (wValue == 0x3F00) ||
            (wValue == 0x3FFF))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdef_FileID[0] = (uint8_t)(wValue & 0x00FF);
        ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdef_FileID[1] = (uint8_t)((wValue & 0xFF00) >> 8);
        break;

    case PHAL_TOP_CONFIG_T4T_NDEF_VNO:
        if((wValue != PHAL_TOP_T4T_NDEF_SUPPORTED_VNO1) &&
            (wValue != PHAL_TOP_T4T_NDEF_SUPPORTED_VNO2))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T4T_t *)(pDataParams->pT4T))->bVno = (uint8_t)wValue;
        break;

    case PHAL_TOP_CONFIG_T4T_WRITE_ACCESS:
        if((wValue != PHAL_TOP_T4T_NDEF_FILE_NO_WRITE_ACCESS) &&
            (wValue != PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T4T_t *)(pDataParams->pT4T))->bWa = (uint8_t)(wValue);
        break;

    case PHAL_TOP_CONFIG_T4T_MAX_NDEF_FILE_SIZE:
        if((wValue < 0x0005) ||
            (wValue > 0x80FE))
        {
            return PH_ADD_COMPCODE(
                PH_ERR_INVALID_PARAMETER,
                PH_COMP_AL_TOP
                );
        }
        ((phalTop_T4T_t *)(pDataParams->pT4T))->bMaxFileSize = wValue;
        break;

    default:
        return PH_ADD_COMPCODE(
            PH_ERR_INVALID_PARAMETER,
            PH_COMP_AL_TOP
            );
        break;
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_GetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint16_t wConfig,
                                uint16_t  * wValue
                                )
{
    uint16_t Calculate_TMS = 0;
    switch(wConfig)
    {
    case PHAL_TOP_CONFIG_TAG_TYPE:
        *wValue = pDataParams->bTagType;
        break;

    case PHAL_TOP_CONFIG_TAG_FORMATTABLE:
        if(pDataParams->bTagType == PHAL_TOP_TAG_TYPE_T1T_TAG)
        {
            *wValue = ((phalTop_T1T_t *)(pDataParams->pT1T))->bNdefFormatted;
        }
        else if(pDataParams->bTagType == PHAL_TOP_TAG_TYPE_T2T_TAG)
        {
            *wValue = ((phalTop_T2T_t *)(pDataParams->pT2T))->bNdefFormatted;
        }
        else if(pDataParams->bTagType == PHAL_TOP_TAG_TYPE_T3T_TAG)
        {
            *wValue = ((phalTop_T3T_t *)(pDataParams->pT3T))->bNdefFormatted;
        }
        else if(pDataParams->bTagType == PHAL_TOP_TAG_TYPE_T4T_TAG)
        {
            *wValue = ((phalTop_T4T_t *)(pDataParams->pT4T))->bNdefFormatted;
        }
        else
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }
        break;

    case PHAL_TOP_CONFIG_T1T_NDEF_VNO:
        *wValue = ((phalTop_T1T_t *)(pDataParams->pT1T))->bVno;
        break;

    case PHAL_TOP_CONFIG_T1T_TMS:
        Calculate_TMS = ((phalTop_T1T_t *)(pDataParams->pT1T))->bTms;
        *wValue = (Calculate_TMS + 1) * 8;
        break;

    case PHAL_TOP_CONFIG_T1T_RW_ACCESS:
        *wValue = ((phalTop_T1T_t *)(pDataParams->pT1T))->bRwa;
        break;

    case PHAL_TOP_CONFIG_T1T_TERMINATOR_TLV:
        *wValue = ((phalTop_T1T_t *)(pDataParams->pT1T))->bTerminatorTlvPresence;
        break;

    case PHAL_TOP_CONFIG_T1T_MAX_NDEF_LENGTH:
        phalTop_Sw_Int_T1T_CalculateMaxNdefSize(pDataParams->pT1T);
        *wValue = ((phalTop_T1T_t *)(pDataParams->pT1T))->wMaxNdefLength;
        break;

    case PHAL_TOP_CONFIG_T1T_GET_TAG_STATE:
        *wValue = ((phalTop_T1T_t *)(pDataParams->pT1T))->bTagState;
        break;

    case PHAL_TOP_CONFIG_T2T_NDEF_VNO:
        *wValue = ((phalTop_T2T_t *)(pDataParams->pT2T))->bVno;
        break;

    case PHAL_TOP_CONFIG_T2T_TMS:
        Calculate_TMS = ((phalTop_T2T_t *)(pDataParams->pT2T))->bTms;
        *wValue = Calculate_TMS * 8;
        break;

    case PHAL_TOP_CONFIG_T2T_RW_ACCESS:
        *wValue = ((phalTop_T2T_t *)(pDataParams->pT2T))->bRwa;
        break;

    case PHAL_TOP_CONFIG_T2T_TERMINATOR_TLV:
        *wValue = ((phalTop_T2T_t *)(pDataParams->pT2T))->bTerminatorTlvPresence;
        break;

    case PHAL_TOP_CONFIG_T2T_PROPRIETARY_TLV:
        *wValue = ((phalTop_T2T_t *)(pDataParams->pT2T))->bEraseProprietaryTlv;

    case PHAL_TOP_CONFIG_T2T_MAX_NDEF_LENGTH:
        phalTop_Sw_Int_T2T_CalculateMaxNdefSize(pDataParams->pT2T);
        *wValue = ((phalTop_T2T_t *)(pDataParams->pT2T))->wMaxNdefLength;
        break;

    case PHAL_TOP_CONFIG_T2T_GET_TAG_STATE:
        *wValue = ((phalTop_T2T_t *)(pDataParams->pT2T))->bTagState;
        break;

    case PHAL_TOP_CONFIG_T3T_NDEF_VNO:
        *wValue = ((phalTop_T3T_t *)(pDataParams->pT3T))->bVno;
        break;

    case PHAL_TOP_CONFIG_T3T_RW_ACCESS:
        *wValue = ((phalTop_T3T_t *)(pDataParams->pT3T))->bRwa;
        break;

    case PHAL_TOP_CONFIG_T3T_MAX_NDEF_LENGTH:
        *wValue = ((phalTop_T3T_t *)(pDataParams->pT3T))->bNmaxb;
        break;

    case PHAL_TOP_CONFIG_T3T_NBR:
        *wValue = ((phalTop_T3T_t *)(pDataParams->pT3T))->bNbr;
        break;

    case PHAL_TOP_CONFIG_T3T_NBW:
        *wValue = ((phalTop_T3T_t *)(pDataParams->pT3T))->bNbw;
        break;

    case PHAL_TOP_CONFIG_T3T_GET_TAG_STATE:
        *wValue = ((phalTop_T3T_t *)(pDataParams->pT3T))->bTagState;
        break;

    case  PHAL_TOP_CONFIG_T4T_MAX_NDEF_FILE_SIZE:
        *wValue = ((phalTop_T4T_t *)(pDataParams->pT4T))->bMaxFileSize;
        break;

    case PHAL_TOP_CONFIG_T4T_NDEF_VNO:
        *wValue = ((phalTop_T4T_t *)(pDataParams->pT4T))->bVno;
        break;

    case PHAL_TOP_CONFIG_T4T_WRITE_ACCESS:
        *wValue = ((phalTop_T4T_t *)(pDataParams->pT4T))->bWa;
        break;

    case PHAL_TOP_CONFIG_T4T_READ_ACCESS:
        *wValue = ((phalTop_T4T_t *)(pDataParams->pT4T))->bRa;
        break;

    case PHAL_TOP_CONFIG_T4T_GET_TAG_STATE:
        *wValue = ((phalTop_T4T_t *)(pDataParams->pT4T))->bTagState;
        break;

    case PHAL_TOP_CONFIG_T4T_NLEN:
        *wValue = ((phalTop_T4T_t *)(pDataParams->pT4T))->wNLEN;
        break;

    default:
        return PH_ADD_COMPCODE(
            PH_ERR_INVALID_PARAMETER,
            PH_COMP_AL_TOP
            );
        break;
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_ReadNdef(
                               phalTop_Sw_DataParams_t * pDataParams,
                               uint8_t * pData,
                               uint16_t * pLength
                               )
{
    phStatus_t PH_MEMLOC_REM status;

    switch(pDataParams->bTagType)
    {
    case PHAL_TOP_TAG_TYPE_T1T_TAG:
        status = phalTop_Sw_Int_T1T_ReadNdef(
            pDataParams->pT1T,
            pData,
            pLength);
        break;

    case PHAL_TOP_TAG_TYPE_T2T_TAG:
        status = phalTop_Sw_Int_T2T_ReadNdef(
            pDataParams->pT2T,
            pData,
            pLength);
        break;

    case PHAL_TOP_TAG_TYPE_T3T_TAG:
        status = phalTop_Sw_Int_T3T_ReadNdef(
            pDataParams->pT3T,
            pData,
            pLength);
        break;

    case PHAL_TOP_TAG_TYPE_T4T_TAG:
        status = phalTop_Sw_Int_T4T_ReadNdef(
            pDataParams->pT4T,
            pData,
            pLength);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        break;
    }

    return status;
}

phStatus_t phalTop_Sw_WriteNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pData,
                                uint16_t wLength
                                )
{
    phStatus_t PH_MEMLOC_REM status;

    switch(pDataParams->bTagType)
    {
    case PHAL_TOP_TAG_TYPE_T1T_TAG:
        status = phalTop_Sw_Int_T1T_WriteNdef(
            pDataParams->pT1T,
            pData,
            wLength);
        break;

    case PHAL_TOP_TAG_TYPE_T2T_TAG:
        status = phalTop_Sw_Int_T2T_WriteNdef(
            pDataParams->pT2T,
            pData,
            wLength);
        break;

    case PHAL_TOP_TAG_TYPE_T3T_TAG:
        status = phalTop_Sw_Int_T3T_WriteNdef(
            pDataParams->pT3T,
            pData,
            wLength);
        break;

    case PHAL_TOP_TAG_TYPE_T4T_TAG:
        status = phalTop_Sw_Int_T4T_WriteNdef(
            pDataParams->pT4T,
            pData,
            wLength);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        break;
    }

    return status;
}

#ifdef __DEBUG
/* This API would be available only for debugging purpose */
phStatus_t phalTop_Sw_SetPtr(
                             phalTop_Sw_DataParams_t * pDataParams,
                             void * pT1T,
                             void * pT2T,
                             void * pT3T,
                             void * pT4T
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    status = PH_ERR_SUCCESS;

    ((phalTop_T1T_t *)(pDataParams->pT1T))->phalT1TDataParams = pT1T;
    ((phalTop_T2T_t *)(pDataParams->pT2T))->phalT2TDataParams = pT2T;
    ((phalTop_T3T_t *)(pDataParams->pT3T))->phalT3TDataParams = pT3T;
    ((phalTop_T4T_t *)(pDataParams->pT4T))->phalT4TDataParams = pT4T;

    return status;     
}
#endif /* __DEBUG */


#endif /* NXPBUILD__PHAL_TOP_SW */
