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
#include <ph_Typedefs.h>
#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phacDiscLoop.h>
#include <phalT1T.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW

#include "phalTop_Sw_Int_T1T.h"


phStatus_t phalTop_Sw_Int_T1T_SetReadOnly(
    phalTop_T1T_t * pT1T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bRxData[11];
    uint16_t   PH_MEMLOC_REM wRxLength;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint16_t   PH_MEMLOC_REM wTempIndex;

    /* Write Read/Write access byte*/
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        11,
        pT1T->bRwa,
        bRxData,
        &wRxLength));

    /* Update Lock-0 */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteNoEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        112,
        0xFF,
        bRxData,
        &wRxLength));

    /* Update Lock-1 */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteNoEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        113,
        0xFF,
        bRxData,
        &wRxLength));

    /* Update dynamic lock bytes */
    for(wIndex = 0; wIndex < pT1T->bLockTlvCount; wIndex++)
    {
        for(wTempIndex = 0; wTempIndex < (pT1T->asLockCtrlTvl[wIndex].bSizeInBits / 8); wTempIndex++)
        {
            PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteNoEraseByte(
                pT1T->phalT1TDataParams,
                pT1T->bUid,
                (uint8_t)(pT1T->asLockCtrlTvl[wIndex].wByteAddr + wTempIndex),
                0xFF,
                bRxData,
                &wRxLength));
        }

        if(pT1T->asLockCtrlTvl[wIndex].bSizeInBits % 8)
        {
            PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteNoEraseByte(
                pT1T->phalT1TDataParams,
                pT1T->bUid,
                (uint8_t)(pT1T->asLockCtrlTvl[wIndex].wByteAddr + wTempIndex),
                0xFF,
                bRxData,
                &wRxLength));
        }
    }
    pT1T->bTagState = PHAL_TOP_T1T_STATE_READONLY;
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

void phalTop_Sw_Int_T1T_CalculateMaxNdefSize(
    phalTop_T1T_t * pT1T
    )
{
    uint16_t PH_MEMLOC_REM wIndex;

    if(pT1T->bTagMemoryType == PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC)
    {
        pT1T->wMaxNdefLength = 90;
    }
    else
    {
        pT1T->wMaxNdefLength = ((pT1T->bTms + 1) * 8) - ((3*8) + PHAL_TOP_T1T_CC_LENGTH +
            PHAL_TOP_T1T_MAX_NDEF_TLV_LEN_FIELD + PHAL_TOP_T1T_NDEF_TLV_HEADER_LEN);
        if(pT1T->bLockTlvCount)
        {
            for(wIndex = 0; wIndex < pT1T->bLockTlvCount; wIndex++)
            {
                pT1T->wMaxNdefLength -= 5;
                pT1T->wMaxNdefLength -=
                    (pT1T->asLockCtrlTvl[wIndex].bSizeInBits / 8);
            }
        }

        if(pT1T->bMemoryTlvCount)
        {
            for(wIndex = 0; wIndex < pT1T->bMemoryTlvCount; wIndex++)
            {
                pT1T->wMaxNdefLength -= 5;
                pT1T->wMaxNdefLength -=
                    pT1T->asMemCtrlTvl[wIndex].bSizeInBytes;
            }
        }
    }
}

phStatus_t phalTop_Sw_Int_T1T_UpdateLockReservedOtp(
    phalTop_T1T_t * pT1T
    )
{
    uint16_t PH_MEMLOC_REM wOffset;
    uint16_t PH_MEMLOC_REM wIndex;
    uint16_t PH_MEMLOC_REM wSegmentOffset;

    memset(pT1T->sSegment.bLockReservedOtp, 0, 16);     /* PRQA S 3200 */
    if(pT1T->sSegment.bAddress == 0)
    {
        pT1T->sSegment.bLockReservedOtp[0x00] = 0xFF;
        pT1T->sSegment.bLockReservedOtp[0x0D] = 0xFF;
        pT1T->sSegment.bLockReservedOtp[0x0E] = 0xFF;
        pT1T->sSegment.bLockReservedOtp[0x0F] = 0xFF;
    }
    else
    {
        /* Update Lock Bytes */
        for(wIndex = 0; wIndex < pT1T->bLockTlvCount; wIndex++)
        {
            if((pT1T->asLockCtrlTvl[wIndex].wByteAddr /
                PHAL_TOP_T1T_SEGMENT_SIZE) ==
                pT1T->sSegment.bAddress)
            {
                wSegmentOffset =
                    pT1T->asLockCtrlTvl[wIndex].wByteAddr %
                    PHAL_TOP_T1T_SEGMENT_SIZE;

                for(wOffset = wSegmentOffset;
                    wOffset < (wSegmentOffset +
                    (pT1T->asLockCtrlTvl[wIndex].bSizeInBits / 8));
                wOffset++)
                {
                    pT1T->sSegment.bLockReservedOtp[wOffset / 8] |=
                        (uint8_t)(1 << (wOffset % 8));
                }
            }
        }
        /* Update Memory Bytes */
        for(wIndex = 0; wIndex < pT1T->bMemoryTlvCount; wIndex++)
        {
            if(pT1T->asMemCtrlTvl[wIndex].wByteAddr /
                PHAL_TOP_T1T_SEGMENT_SIZE ==
                pT1T->sSegment.bAddress)
            {
                wSegmentOffset =
                    pT1T->asMemCtrlTvl[wIndex].wByteAddr %
                    PHAL_TOP_T1T_SEGMENT_SIZE;

                for(wOffset = wSegmentOffset;
                    wOffset < (wSegmentOffset +
                    (pT1T->asMemCtrlTvl[wIndex].bSizeInBytes));
                wOffset++)
                {
                    pT1T->sSegment.bLockReservedOtp[wOffset / 8] |=
                        (uint8_t)(1 << (wOffset % 8));
                }
            }
        }
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T1T_CheckLockReservedOtp(
    phalTop_T1T_t * pT1T,
    uint16_t wIndex
    )
{
    if((pT1T->sSegment.bLockReservedOtp[(wIndex % PHAL_TOP_T1T_SEGMENT_SIZE) / 8]) &&
        (1 << (wIndex % 8)))
    {
        return PHAL_TOP_T1T_ERR_RESERVED_BYTE;
    }
    else
    {
        return PH_ERR_SUCCESS;
    }
}

phStatus_t phalTop_Sw_Int_T1T_Read(
                                   phalTop_T1T_t * pT1T,
                                   uint16_t wAddress,
                                   uint8_t * pData,
                                   uint8_t bFreshRead
                                   )
{
    phStatus_t     PH_MEMLOC_REM status;
    uint16_t          PH_MEMLOC_REM wLength;

    if(pT1T->bTagMemoryType != PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC)
    {
        if((pT1T->sSegment.bBlockAddress != (wAddress / 8)) ||
            (pT1T->sSegment.bValidity == PH_OFF) || (bFreshRead == PH_ON))
        {
            pT1T->sSegment.bBlockAddress = (uint8_t)(wAddress / 8);
            pT1T->sSegment.bAddress = (uint8_t)(wAddress / PHAL_TOP_T1T_SEGMENT_SIZE);
            PH_CHECK_SUCCESS_FCT(status, phalT1T_ReadBlock(
                pT1T->phalT1TDataParams,
                pT1T->bUid,
                pT1T->sSegment.bBlockAddress,
                pData,
                &wLength));
            pT1T->sSegment.bValidity = PH_ON;
        }
        pData[0] = pData[1 + (wAddress % 8)];
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(status, phalT1T_ReadByte(
            pT1T->phalT1TDataParams,
            pT1T->bUid,
            (uint8_t)wAddress,
            pData,
            &wLength));

        pData[0] = pData[1];
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T1T_Write(
                                    phalTop_T1T_t * pT1T,
                                    uint16_t wAddress,
                                    uint8_t * pData,
                                    uint16_t pDataIndex
                                    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bRxData[11];
    uint8_t    PH_MEMLOC_REM bTempData[11];
    uint16_t   PH_MEMLOC_REM wRxLength;
    uint8_t    PH_MEMLOC_REM bReservedBytesPresent = 0;
    uint8_t    PH_MEMLOC_REM bFirstBlock = 0;
    uint8_t    PH_MEMLOC_REM bLastBlock = 0;
    uint16_t   PH_MEMLOC_REM wIndex = 0;

    if(pT1T->bTagMemoryType != PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC)
    {
        if(((pT1T->sSegment.bLockReservedOtp[((wAddress % PHAL_TOP_T1T_SEGMENT_SIZE) / 8)] & 0xFF) == 0x00))
        {
            if((wAddress % 8) == 0)
            {
                if((pDataIndex + 8) <= pT1T->wNdefLength)
                {
                    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseBlock(
                        pT1T->phalT1TDataParams,
                        pT1T->bUid,
                        (uint8_t)(wAddress / 8),
                        &pData[pDataIndex],
                        bRxData,
                        &wRxLength));
                }
                else
                {
                    bLastBlock = 1; /* If Last block of NDEF message is less than 8 bytes, write byte by byte */
                }
            }
        }
        else
        {
            bReservedBytesPresent = 1;
        }

        if((pT1T->wNdefMsgAddr % 8) && (pT1T->wNdefMsgAddr == wAddress))
        {
            bFirstBlock = 1; /* If First block of NDEF message is not starting from start of a block, write byte by byte */
        }
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
            pT1T->phalT1TDataParams,
            pT1T->bUid,
            (uint8_t)wAddress,
            pData[pDataIndex],
            bRxData,
            &wRxLength));

        if((pT1T->bTerminatorTlvPresence == PH_ON) &&
            (wAddress < ((pT1T->bTms + 1) * 8)) && ((pDataIndex + 1) == pT1T->wNdefLength))
        {
            for(wIndex = (wAddress +1); wIndex < ((pT1T->bTms + 1) * 8); wIndex++)
            {
                if((phalTop_Sw_Int_T1T_CheckLockReservedOtp(pT1T, wIndex)) == PH_ERR_SUCCESS)
                {
                    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
                        pT1T->phalT1TDataParams,
                        pT1T->bUid,
                        (uint8_t)wIndex,
                        0xFE,
                        bRxData,
                        &wRxLength));
                    break;
                }
            }
        }
    }

    if((bLastBlock) || (bFirstBlock) || (bReservedBytesPresent))
    {
        if(bFirstBlock)
        {
            PH_CHECK_SUCCESS_FCT(status, phalT1T_ReadBlock(
                pT1T->phalT1TDataParams,
                pT1T->bUid,
                (uint8_t)(wAddress / 8),
                bTempData,
                &wRxLength));

            memcpy(&bTempData[(wAddress % 8) + 1], &pData[pDataIndex], (8 - (wAddress % 8)));   /* PRQA S 3200 */

            PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseBlock(
                pT1T->phalT1TDataParams,
                pT1T->bUid,
                (uint8_t)(wAddress / 8),
                &bTempData[1],
                bRxData,
                &wRxLength));
        }
        else if(bLastBlock)
        {
            memset(bTempData, 0, 8);    /* PRQA S 3200 */
            memcpy(bTempData, &pData[pDataIndex], (pT1T->wNdefLength - pDataIndex));    /* PRQA S 3200 */
            if(((pT1T->wNdefLength - pDataIndex) < 8) && (pT1T->bTerminatorTlvPresence == PH_ON))
            {
                bTempData[(pT1T->wNdefLength - pDataIndex)] = 0xFE; /* Terminator TLV */
            }

            PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseBlock(
                pT1T->phalT1TDataParams,
                pT1T->bUid,
                (uint8_t)(wAddress / 8),
                bTempData,
                bRxData,
                &wRxLength));

            if(((pT1T->wNdefLength - pDataIndex) == 8) && (pT1T->bTerminatorTlvPresence == PH_ON)
                && (wAddress < ((pT1T->bTms + 1) * 8)))
            {
                memset(bTempData, 0, 8);    /* PRQA S 3200 */
                bTempData[0] = 0xFE;

                for(wIndex = ((wAddress / 8) + 1); (wIndex * 8) < ((pT1T->bTms + 1) * 8); wIndex++)
                {
                    if((phalTop_Sw_Int_T1T_CheckLockReservedOtp(pT1T, (wIndex * 8))) == PH_ERR_SUCCESS)
                    {
                        PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseBlock(
                            pT1T->phalT1TDataParams,
                            pT1T->bUid,
                            (uint8_t)wIndex,
                            bTempData,
                            bRxData,
                            &wRxLength));
                    }
                }
            }
        }
        else
        {
            /* TODO: For reserved bytes */
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

/* Finds the count and position of memory, lock and NDEF TLV */
phStatus_t phalTop_Sw_Int_T1T_DetectTlvBlocks(
    phalTop_T1T_t * pT1T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint8_t    PH_MEMLOC_REM bPageAddr;
    uint8_t    PH_MEMLOC_REM bByteOffset;
    uint8_t    PH_MEMLOC_REM bBytesPerPage;
    uint8_t    PH_MEMLOC_REM bLength[3];
    uint8_t    PH_MEMLOC_REM bExitLoop = 0;
    uint16_t   PH_MEMLOC_REM wStartIndex;
    uint16_t   PH_MEMLOC_REM wEndIndex;
    uint8_t    PH_MEMLOC_REM bData[11];
    uint8_t       PH_MEMLOC_REM bFreshRead = PH_ON;

    while(!bExitLoop)
    {
        if(pT1T->sSegment.bAddress == 0)
        {
            wStartIndex = 12;
            wEndIndex   = 104;
        }
        else
        {
            wStartIndex = pT1T->sSegment.bAddress * PHAL_TOP_T1T_SEGMENT_SIZE;
            wEndIndex   = (pT1T->sSegment.bAddress + 1) * PHAL_TOP_T1T_SEGMENT_SIZE;
        }
        
        wIndex = wStartIndex;
        while(wIndex < wEndIndex)
        {
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, wIndex, bData, bFreshRead));
            bFreshRead = PH_OFF;
            switch(bData[0])
            {
            case PHAL_TOP_T1T_NULL_TLV:
                break;

            case PHAL_TOP_T1T_LOCK_CTRL_TLV:
                if(pT1T->bTagMemoryType != PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC)
                {
                    pT1T->asLockCtrlTvl[pT1T->bLockTlvCount].wOffset = wIndex;

                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 2), bData, PH_OFF));
                    bPageAddr = (uint8_t)((bData[0] & 0xF0) >> 4);
                    bByteOffset = bData[0] & 0x0F;

                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 3), bData, PH_OFF));
                    pT1T->asLockCtrlTvl[pT1T->bLockTlvCount].bSizeInBits = bData[0];

                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 4), bData, PH_OFF));
                    bBytesPerPage = bData[0] & 0x0F;
                    pT1T->asLockCtrlTvl[pT1T->bLockTlvCount].bBytesLockedPerBit = (uint8_t)((bData[0] & 0xF0) >> 4);

                    pT1T->asLockCtrlTvl[pT1T->bLockTlvCount].wByteAddr =
                        (bPageAddr * (1 << bBytesPerPage)) + bByteOffset;
                    pT1T->bLockTlvCount++;
                    wIndex = wIndex + 4;
                }
                break;

            case PHAL_TOP_T1T_MEMORY_CTRL_TLV:
                if(pT1T->bTagMemoryType != PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC)
                {
                    pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount].wOffset = wIndex;

                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 2), bData, PH_OFF));
                    bPageAddr = (uint8_t)((bData[0] & 0xF0) >> 4);
                    bByteOffset = (uint8_t)(bData[0] & 0x0F);

                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 3), bData, PH_OFF));
                    pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount].bSizeInBytes = bData[0];

                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 4), bData, PH_OFF));
                    bBytesPerPage = bData[0] & 0x0F;

                    pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount].wByteAddr =
                        (bPageAddr * (1 << bBytesPerPage)) + bByteOffset;
                    pT1T->bMemoryTlvCount++;
                    wIndex = wIndex + 4;
                }
                break;

            case PHAL_TOP_T1T_NDEF_TLV:
                if(pT1T->bNdefPresence == true)
                {
                    pT1T->wNdefHeaderAddr = wIndex;
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 1), bData, PH_OFF));
                    bLength[0] = bData[0];
                    if(bLength[0] == 0xFF)
                    {
                        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 2), bData, PH_OFF));
                        bLength[1] = bData[0];
                        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 3), bData, PH_OFF));
                        bLength[2] = bData[0];
                        pT1T->wNdefLength = ((uint16_t)bLength[1] << 8) | bLength[2];
                        pT1T->wNdefMsgAddr = pT1T->wNdefHeaderAddr + 4;
                        wIndex = wIndex + pT1T->wNdefLength + 3;
                    }
                    else
                    {
                        pT1T->wNdefLength = bLength[0];
                        pT1T->wNdefMsgAddr = pT1T->wNdefHeaderAddr + 2;
                        wIndex = wIndex + pT1T->wNdefLength + 1;
                    }
                }
                bExitLoop = 1;
                break;

            case PHAL_TOP_T1T_PROPRIETARY_TLV:
                pT1T->asPropTvl[pT1T->bProprietaryTlvCount].wOffset = wIndex;
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 1), bData, PH_OFF));
                bLength[0] = bData[0];
                if(bLength[0] == 0xFF)
                {
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 2), bData, PH_OFF));
                    bLength[1] = bData[0];
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, (wIndex + 3), bData, PH_OFF));
                    bLength[2] = bData[0];
                    pT1T->asPropTvl[pT1T->bProprietaryTlvCount].wLength =
                        ((uint16_t)bLength[1] << 8) | bLength[2];
                    wIndex = wIndex + (((uint16_t)bLength[1] << 8) | bLength[2]) + 3;
                }
                else
                {
                    pT1T->asPropTvl[pT1T->bProprietaryTlvCount].wLength = bLength[0];
                    wIndex = wIndex + bLength[0] + 1;
                }
                break;

            case PHAL_TOP_T1T_TERMINATOR_TLV:
                pT1T->bTerminatorTlvPresence = PH_ON;
                bExitLoop = 1;
                break;

            default:
                bExitLoop = 1;
                break;
            }

            if (bExitLoop)
            {
                break;
            }
            wIndex++;
        }

        if((pT1T->bNdefPresence != true) || (pT1T->bTagMemoryType == PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC))
        {
            bExitLoop = 1;
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T1T_CheckNdef(
                                        phalTop_T1T_t * pT1T,
                                        uint8_t * pNdefPresence
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bData[11];

    memcpy(pT1T->bUid, ((phalT1T_Sw_DataParams_t *)(pT1T->phalT1TDataParams))->abUid, 4);   /* PRQA S 3200 */
    pT1T->bTagState = PHAL_TOP_T1T_STATE_UNKNOWN;
    if((((phalT1T_Sw_DataParams_t *)(pT1T->phalT1TDataParams))->abHR[0] & 0x10) != 0x10)
    {
        pT1T->bNdefSupport = PH_UNSUPPORTED;
        pT1T->bNdefFormatted = PH_UNSUPPORTED;
        *pNdefPresence = false;
    }
    else
    {
        pT1T->bNdefFormatted = PH_SUPPORTED;
        pT1T->bNdefSupport = PH_SUPPORTED;
        if((((phalT1T_Sw_DataParams_t *)(pT1T->phalT1TDataParams))->abHR[0] & 0x01) == 0x01)
        {
            pT1T->bTagMemoryType = PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC;
        }
        else
        {
            pT1T->bTagMemoryType = PHAL_TOP_T1T_TAG_MEMORY_TYPE_DYNAMIC;
        }

        pT1T->sSegment.bAddress = 0;

        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(
            pT1T,
            PHAL_TOP_T1T_CC_NMN_ADDR,
            bData,
            PH_ON));
        if(bData[0] != PHAL_TOP_T1T_NDEF_NMN)
        {
            pT1T->bNdefPresence = false;
            *pNdefPresence = false;
        }
        else
        {
             pT1T->bNdefPresence = true;
            *pNdefPresence = true;
        }

        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(
            pT1T,
            PHAL_TOP_T1T_CC_VNO_ADDR,
            bData,
            PH_OFF));
        pT1T->bVno = bData[0];

        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(
            pT1T,
            PHAL_TOP_T1T_CC_TMS_ADDR,
            bData,
            PH_OFF));
        pT1T->bTms = bData[0];

        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(
            pT1T,
            PHAL_TOP_T1T_CC_RWA_ADDR,
            bData,
            PH_OFF));
        pT1T->bRwa = bData[0];

        /* If NDEF version is supported, detect TLVs */
        if((uint8_t)(pT1T->bVno & 0xF0) <= (uint8_t)(PHAL_TOP_T1T_NDEF_SUPPORTED_VNO & 0xF0))
        {
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_DetectTlvBlocks(
                pT1T));
        }

        if((pT1T->bNdefPresence == true) && (pT1T->wNdefLength != 0) &&
            ((uint8_t)(pT1T->bVno & 0xF0) <= (uint8_t)(PHAL_TOP_T1T_NDEF_SUPPORTED_VNO & 0xF0)))
        {
            if(pT1T->bRwa == PHAL_TOP_T1T_CC_RWA_RW)
            {
                pT1T->bTagState = PHAL_TOP_T1T_STATE_READWRITE;
            }
            else if(pT1T->bRwa == PHAL_TOP_T1T_CC_RWA_RO)
            {
                pT1T->bTagState = PHAL_TOP_T1T_STATE_READONLY;
            }
            else
            {
                /* RFU */
            }
        }
        else
        {
            pT1T->bTagState = PHAL_TOP_T1T_STATE_INITIALIZED;
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T1T_FormatNdef(
    phalTop_T1T_t * pT1T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t PH_MEMLOC_REM bRxData[11];
    uint16_t PH_MEMLOC_REM wRxLength;

    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(
        pT1T,
        PHAL_TOP_T1T_CC_RWA_ADDR,
        bRxData,
        PH_OFF));
    /* Check Read/Write Access Byte of CC before formating the Tag*/
    if((pT1T->bNdefSupport != PH_SUPPORTED) 
        || (bRxData[0] != PHAL_TOP_T1T_CC_RWA_RW))
    {
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_TOP);
    }

    if((pT1T->bTms == 0) || (pT1T->bTms < 0x0E))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    if((pT1T->bRwa != PHAL_TOP_T1T_CC_RWA_RO) &&
        (pT1T->bRwa != PHAL_TOP_T1T_CC_RWA_RW))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Make NDEF Magic Number value '0' */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        8,
        0x00,
        bRxData,
        &wRxLength));

    /* Write NDEF version number */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        9,
        PHAL_TOP_T1T_NDEF_SUPPORTED_VNO,
        bRxData,
        &wRxLength));

    /* Write TMS byte*/
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        10,
        pT1T->bTms,
        bRxData,
        &wRxLength));

    /* Write Read/Write access byte*/
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        11,
        pT1T->bRwa,
        bRxData,
        &wRxLength));

    if((pT1T->bLockTlvCount !=0) &&
            (pT1T->bMemoryTlvCount != 0))
    {
        pT1T->wNdefHeaderAddr =
            (pT1T->asLockCtrlTvl[pT1T->bLockTlvCount - 1].wOffset >
            pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount - 1].wOffset?
            pT1T->asLockCtrlTvl[pT1T->bLockTlvCount - 1].wOffset:
        pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount - 1].wOffset) + 5;
    }
    else if(pT1T->bLockTlvCount !=0)
    {
        pT1T->wNdefHeaderAddr =
            pT1T->asLockCtrlTvl[pT1T->bLockTlvCount - 1].wOffset + 5;
    }
    else if(pT1T->bMemoryTlvCount != 0)
    {
        pT1T->wNdefHeaderAddr =
            pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount - 1].wOffset + 5;
    }
    else
    {
        pT1T->wNdefHeaderAddr = 12;
    }

    pT1T->bNdefFormatted = true;
    pT1T->bNdefPresence = false;
    pT1T->bTagState = PHAL_TOP_T1T_STATE_INITIALIZED;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T1T_EraseNdef(
                                        phalTop_T1T_t * pT1T
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bRxData[11];
    uint16_t   PH_MEMLOC_REM wRxLength;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint16_t   PH_MEMLOC_REM wTempIndex;
    uint8_t    PH_MEMLOC_REM bTerminatorTlvLen = 0;

    /* Make NDEF Magic Number value '0' */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        8,
        0x00,
        bRxData,
        &wRxLength));

    pT1T->sSegment.bValidity = PH_OFF;

    if((pT1T->wNdefHeaderAddr / PHAL_TOP_T1T_SEGMENT_SIZE) !=
        pT1T->sSegment.bAddress)
    {
        pT1T->sSegment.bAddress =
            (uint8_t)(pT1T->wNdefHeaderAddr / PHAL_TOP_T1T_SEGMENT_SIZE);
    }

    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_UpdateLockReservedOtp(
        pT1T));

    /* Check for Terminator TVL */
    if(pT1T->bTerminatorTlvPresence == PH_ON)
    {
        bTerminatorTlvLen = 1;
    }

    /* Write 0s in NDEF area */
    for(wIndex = pT1T->wNdefHeaderAddr, wTempIndex = 0;
        wTempIndex < (pT1T->wNdefLength + PHAL_TOP_T1T_GET_TLV_LEN_BYTES(pT1T->wNdefLength) +
        PHAL_TOP_T1T_NDEF_TLV_HEADER_LEN + bTerminatorTlvLen);
    wIndex++)
    {
        if((wIndex/PHAL_TOP_T1T_SEGMENT_SIZE) != pT1T->sSegment.bAddress)
        {
            pT1T->sSegment.bAddress = (uint8_t)(wIndex/PHAL_TOP_T1T_SEGMENT_SIZE);
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_UpdateLockReservedOtp(
                pT1T));
        }

        if((phalTop_Sw_Int_T1T_CheckLockReservedOtp(pT1T, wIndex)) == PH_ERR_SUCCESS)
        {
            PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
                pT1T->phalT1TDataParams,
                pT1T->bUid,
                (uint8_t)wIndex,
                0x00,
                bRxData,
                &wRxLength));

            wTempIndex++;
        }
    }

    /* Erase Terminator TLV */
    if(pT1T->bTerminatorTlvPresence == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
            pT1T->phalT1TDataParams,
            pT1T->bUid,
            (uint8_t)wIndex,
            0x00,
            bRxData,
            &wRxLength));
    }
    pT1T->bNdefPresence = false;
    pT1T->bTagState = PHAL_TOP_T1T_STATE_INITIALIZED;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T1T_ReadNdef(
                                       phalTop_T1T_t * pT1T,
                                       uint8_t * pData,
                                       uint16_t * pLength
                                       )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t PH_MEMLOC_REM wIndex;
    uint16_t PH_MEMLOC_REM wTempIndex;
    uint8_t PH_MEMLOC_REM bData[11];
    uint8_t PH_MEMLOC_REM bFreshRead = PH_ON;

    /* TODO: Check for read access*/

    if((pT1T->bTagState != PHAL_TOP_T1T_STATE_READWRITE) &&
       (pT1T->bTagState != PHAL_TOP_T1T_STATE_READONLY))
    {
        /* Tag not in Read/Write or Read Only state */
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
    }
    else
    {
        if((pT1T->wNdefMsgAddr / PHAL_TOP_T1T_SEGMENT_SIZE) !=
            pT1T->sSegment.bAddress)
        {
            pT1T->sSegment.bAddress = (uint8_t)(pT1T->wNdefMsgAddr / PHAL_TOP_T1T_SEGMENT_SIZE);
        }
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_UpdateLockReservedOtp(pT1T));

        for(wIndex = pT1T->wNdefMsgAddr, wTempIndex = 0;
            wTempIndex < pT1T->wNdefLength;
            wIndex++)
        {
            if(((wIndex / PHAL_TOP_T1T_SEGMENT_SIZE) !=    pT1T->sSegment.bAddress))
            {
                pT1T->sSegment.bAddress = (uint8_t)(wIndex / PHAL_TOP_T1T_SEGMENT_SIZE);
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_UpdateLockReservedOtp(pT1T));
            }

            if((phalTop_Sw_Int_T1T_CheckLockReservedOtp(pT1T, wIndex)) == PH_ERR_SUCCESS)
            {
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Read(pT1T, wIndex, bData, bFreshRead));
                bFreshRead = PH_OFF;
                pData[wTempIndex] =    bData[0];
                wTempIndex++;
            }
        }

        if(wTempIndex == pT1T->wNdefLength)
        {
            *pLength = pT1T->wNdefLength;
        }
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T1T_WriteNdef(
                                        phalTop_T1T_t * pT1T,
                                        uint8_t * pData,
                                        uint16_t wLength
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t PH_MEMLOC_REM bRxData[11];
    uint16_t PH_MEMLOC_REM wRxLength;
    uint16_t PH_MEMLOC_REM wIndex;
    uint16_t PH_MEMLOC_REM wTempIndex;

    if(pT1T->bNdefSupport != PH_SUPPORTED)
    {
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_TOP);
    }

    if(pT1T->bRwa == PHAL_TOP_T1T_CC_RWA_RO)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Make NDEF Magic Number value '0' */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        8,
        0x00,
        bRxData,
        &wRxLength));

    /* Write NDEF version number */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        9,
        PHAL_TOP_T1T_NDEF_SUPPORTED_VNO,
        bRxData,
        &wRxLength));

    /* Write Read/Write access byte*/
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        11,
        pT1T->bRwa,
        bRxData,
        &wRxLength));

    /* Calculate Max. NDEF size */
    phalTop_Sw_Int_T1T_CalculateMaxNdefSize(
        pT1T);

    /* Check NDEF length  */
    if((wLength > pT1T->wMaxNdefLength) || (wLength == 0))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Calculate NDEF Header Address */
    if((pT1T->bLockTlvCount !=0) &&
            (pT1T->bMemoryTlvCount != 0))
    {
        pT1T->wNdefHeaderAddr =
            (pT1T->asLockCtrlTvl[pT1T->bLockTlvCount - 1].wOffset >
            pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount - 1].wOffset?
            pT1T->asLockCtrlTvl[pT1T->bLockTlvCount - 1].wOffset:
        pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount - 1].wOffset) + 5;
    }
    else if(pT1T->bLockTlvCount !=0)
    {
        pT1T->wNdefHeaderAddr =
            pT1T->asLockCtrlTvl[pT1T->bLockTlvCount - 1].wOffset + 5;
    }
    else if(pT1T->bMemoryTlvCount != 0)
    {
        pT1T->wNdefHeaderAddr =
            pT1T->asMemCtrlTvl[pT1T->bMemoryTlvCount - 1].wOffset + 5;
    }
    else
    {
        pT1T->wNdefHeaderAddr = 12;
    }

    /* Write NDEF TVL */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        (uint8_t)pT1T->wNdefHeaderAddr,
        PHAL_TOP_T1T_NDEF_TLV,
        bRxData,
        &wRxLength));

    /* Write NDEF length */
    if(wLength > 0xFE)
    {
        pT1T->wNdefMsgAddr = pT1T->wNdefHeaderAddr + 4;

        PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
            pT1T->phalT1TDataParams,
            pT1T->bUid,
            (uint8_t)(pT1T->wNdefHeaderAddr + 1),
            0xFF,
            bRxData,
            &wRxLength));

        PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
            pT1T->phalT1TDataParams,
            pT1T->bUid,
            (uint8_t)(pT1T->wNdefHeaderAddr + 2),
            (uint8_t)(wLength >> 8),
            bRxData,
            &wRxLength));

        PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
            pT1T->phalT1TDataParams,
            pT1T->bUid,
            (uint8_t)(pT1T->wNdefHeaderAddr + 3),
            (uint8_t)wLength,
            bRxData,
            &wRxLength));
    }
    else
    {
        pT1T->wNdefMsgAddr = pT1T->wNdefHeaderAddr + 2;
        PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
            pT1T->phalT1TDataParams,
            pT1T->bUid,
            (uint8_t)(pT1T->wNdefHeaderAddr + 1),
            (uint8_t)wLength,
            bRxData,
            &wRxLength));
    }

    /* Update NDEF Length */
    pT1T->wNdefLength = wLength;

    if((pT1T->wNdefMsgAddr / PHAL_TOP_T1T_SEGMENT_SIZE) !=
        pT1T->sSegment.bAddress)
    {
        pT1T->sSegment.bAddress =
            (uint8_t)(pT1T->wNdefMsgAddr / PHAL_TOP_T1T_SEGMENT_SIZE);
    }

    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_UpdateLockReservedOtp(
        pT1T));

    /* Write NDEF data */
    for(wIndex = pT1T->wNdefMsgAddr, wTempIndex = 0;
        wTempIndex < (pT1T->wNdefLength);
        wIndex++)
    {
        if((wIndex/PHAL_TOP_T1T_SEGMENT_SIZE) != pT1T->sSegment.bAddress)
        {
            pT1T->sSegment.bAddress = (uint8_t)(wIndex/PHAL_TOP_T1T_SEGMENT_SIZE);
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_UpdateLockReservedOtp(
                pT1T));
        }

        if((phalTop_Sw_Int_T1T_CheckLockReservedOtp(pT1T, wIndex)) == PH_ERR_SUCCESS)
        {
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T1T_Write(
                pT1T,
                wIndex,
                pData,
                wTempIndex));

            wTempIndex++;
        }
    }

    /* Write NDEF Magic Number */
    PH_CHECK_SUCCESS_FCT(status, phalT1T_WriteEraseByte(
        pT1T->phalT1TDataParams,
        pT1T->bUid,
        8,
        PHAL_TOP_T1T_NDEF_NMN,
        bRxData,
        &wRxLength));

    pT1T->bNdefPresence = true;
    pT1T->bNdefFormatted = true;

    if(pT1T->bRwa == PHAL_TOP_T1T_CC_RWA_RW)
    {
        pT1T->bTagState = PHAL_TOP_T1T_STATE_READWRITE;
    }
    else if(pT1T->bRwa == PHAL_TOP_T1T_CC_RWA_RO)
    {
        pT1T->bTagState = PHAL_TOP_T1T_STATE_READONLY;
    }
    else
    {
        /* RFU */
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}
#endif /* NXPBUILD__PHAL_TOP_SW */
