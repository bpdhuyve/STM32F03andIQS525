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
#include <phalFelica.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW

#include "phalTop_Sw_Int_T3T.h"

phStatus_t phalTop_Sw_Int_T3T_CheckNdef(
                                        phalTop_T3T_t * pT3T,
                                        uint8_t * pNdefPresence
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM wLength;
    uint8_t    PH_MEMLOC_REM bNDEFSystemCode[2] = { 0x12, 0xFC};
    uint8_t    PH_MEMLOC_REM bSystemCode[2] = { 0x88, 0xB4};
    uint8_t    PH_MEMLOC_REM wMoreCardsAvailable;
    uint8_t    PH_MEMLOC_REM bBlockList[2] = { 0x80, 0x00};   /* Read Zero Block to get Attribute Block */
    uint8_t    PH_MEMLOC_REM bMCBlockList[2] = { 0x80, 0x88}; /* Memory Configuration(MC) Block is 88h */
    uint8_t    PH_MEMLOC_REM bReadServiceList[2] = { 0x0B, 0x00};
    uint8_t    PH_MEMLOC_REM bRxNumBlocks = 0;
    uint8_t    PH_MEMLOC_REM bBlockData[16];

    *pNdefPresence = PH_OFF;
    pT3T->bNdefPresence = PH_OFF;
    pT3T->bNdefSupport = PH_UNSUPPORTED;
    pT3T->bNdefFormatted = PH_UNSUPPORTED;

    status = phalFelica_ActivateCard(
        pT3T->phalT3TDataParams,
        bNDEFSystemCode,
        0X00,
        pT3T->bUid,
        &wLength,
        &wMoreCardsAvailable);

    if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        pT3T->bNdefSupport = PH_SUPPORTED;
        pT3T->bNdefFormatted = PH_SUPPORTED;

        PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
            pT3T->phalT3TDataParams,
            0x01,
            bReadServiceList,
            0x01,
            bBlockList,
            0x02,
            &bRxNumBlocks,
            bBlockData));

        pT3T->bVno = bBlockData[0];
        pT3T->bNbr = bBlockData[1];
        pT3T->bNbw = bBlockData[2];
        pT3T->bNmaxb = (uint16_t)(((uint16_t)bBlockData[3] << 8) | bBlockData[4]);
        pT3T->bRwa = bBlockData[10];
        pT3T->wNdefLength = (uint16_t)(((uint32_t)bBlockData[11] << 16) | (((uint16_t)bBlockData[12] << 8) | bBlockData[13]));

        if((pT3T->wNdefLength != 0) && ((pT3T->bVno == PHAL_TOP_T3T_NDEF_SUPPORTED_VNO1)
            || (pT3T->bVno == PHAL_TOP_T3T_NDEF_SUPPORTED_VNO2)))
        {
            pT3T->bNdefPresence = PH_ON;
            *pNdefPresence = PH_ON;

            if(pT3T->bRwa == PHAL_TOP_T3T_AIB_RWA_RW)
            {
                pT3T->bTagState = PHAL_TOP_T3T_STATE_READWRITE;
            }
            else if(pT3T->bRwa == PHAL_TOP_T3T_AIB_RWA_RO)
            {
                pT3T->bTagState = PHAL_TOP_T3T_STATE_READONLY;
            }
            else
            {
                /* Do Nothing */
            }
        }
        else if(pT3T->wNdefLength == 0)
        {
            pT3T->bTagState = PHAL_TOP_T3T_STATE_INITIALIZED;
        }
        else
        {
            /* Do Nothing */
        }
    }
    else
    {
        /* Check if the Tag is Formattable or not and update NdefSupport flag */
        status = phalFelica_ActivateCard(
            pT3T->phalT3TDataParams,
            bSystemCode,
            0,                                                    /* Time Slot */
            pT3T->bUid,
            &wLength,
            &wMoreCardsAvailable);
        if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
        {
        	/* This is not a valid T3T TAG. */
        	return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
    }

        /* Read MC(Memory Configuration) Block to get SYS_OP Value */
        PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
            pT3T->phalT3TDataParams,
            0x01,
            bReadServiceList,
            0x01,
            bMCBlockList,
            0x02,
            &bRxNumBlocks,
            bBlockData));

        if((bBlockData[0] == 0xFF) && (bBlockData[1] == 0xFF) && (bBlockData[2] == 0xFF)
            && (bBlockData[3] == 0x00) && (bBlockData[4] == 0xFF))
        {
        	pT3T->bNdefFormatted = PH_SUPPORTED;
        }
        else if((bBlockData[0] == 0xFF) && (bBlockData[1] == 0xFF) && (bBlockData[2] == 0xFF)
            && (bBlockData[3] == 0x01) && (bBlockData[4] == 0xFF))
        {
        	pT3T->bNdefFormatted = PH_SUPPORTED;
        }
        else
        {
        	pT3T->bNdefFormatted = PH_UNSUPPORTED;
        }
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T3T_FormatNdef(
    phalTop_T3T_t * pT3T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint16_t   PH_MEMLOC_REM wCheckSum;
    uint8_t    PH_MEMLOC_REM bBlockData[16];
    uint8_t    PH_MEMLOC_REM bBlockList[2] = { 0x80, 0x00};   /* Read Zero Block to get Attribute Block */
    uint8_t    PH_MEMLOC_REM bSystemCode[2] = { 0x88, 0xB4};
    uint8_t    PH_MEMLOC_REM bReadServiceList[2] = { 0x0B, 0x00};
    uint8_t    PH_MEMLOC_REM bWriteServiceList[2] = { 0x09, 0x00};
    uint8_t    PH_MEMLOC_REM bRxNumBlocks = 0;
    uint8_t    PH_MEMLOC_REM bMCBlockList[2] = { 0x80, 0x88}; /* Memory Configuration(MC) Block is 88h */
    uint8_t    PH_MEMLOC_REM wLength;
    uint8_t    PH_MEMLOC_REM wMoreCardsAvailable;

    if(pT3T->bNdefSupport != PH_SUPPORTED)
    {
        /* Write MC Block to change Tag to NDEF Complaiant as CheckNDEF falied to activate Tag as NDEF. */
        PH_CHECK_SUCCESS_FCT(status, phalFelica_ActivateCard(
            pT3T->phalT3TDataParams,
            bSystemCode,
            0,                                                    /* Time Slot */
            pT3T->bUid,
            &wLength,
            &wMoreCardsAvailable));

        /* Read MC(Memory Configuration) Block to get SYS_OP Value */
        PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
            pT3T->phalT3TDataParams,
            0x01,
            bReadServiceList,
            0x01,
            bMCBlockList,
            0x02,
            &bRxNumBlocks,
            bBlockData));

        if((bBlockData[0] == 0xFF) && (bBlockData[1] == 0xFF) && (bBlockData[2] == 0xFF)
            && (bBlockData[3] == 0x00) && (bBlockData[4] == 0xFF))
        {
            bBlockData[3] = 0x01;

            PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
                pT3T->phalT3TDataParams,
                0x01,
                bWriteServiceList,
                0x01,
                bMCBlockList,
                0x02,
                bBlockData));
        }
        else if((bBlockData[0] == 0xFF) && (bBlockData[1] == 0xFF) && (bBlockData[2] == 0xFF)
            && (bBlockData[3] == 0x01) && (bBlockData[4] == 0xFF))
        {
            /* Proceed with write of Attribute Information block */
        }
        else
        {
            return PH_ADD_COMPCODE(PHAL_TOP_ERR_INVALID_TAG, PH_COMP_AL_TOP);
        }
    }

    /* Write Attribute Information Block(AIB) with some Nbr and Nbw values */
    if((pT3T->bVno == PHAL_TOP_T3T_NDEF_SUPPORTED_VNO1) ||
        (pT3T->bVno == PHAL_TOP_T3T_NDEF_SUPPORTED_VNO2))
    {
        bBlockData[0] = pT3T->bVno;
    }
    else
    {
        bBlockData[0] = 0x11;
    }
    bBlockData[1] = 0x01;                                 /* Value of Nbr for Felica Lite is 1 */
    bBlockData[2] = 0x01;                                 /* Value of Nbw for Felica Lite is 1 */
    if(pT3T->bNmaxb < 0x000D)
    {
        bBlockData[3] = 0x00;                             /* MSB Value of Nmaxb */
        bBlockData[4] = 0x0D;                             /* LSB Value of Nmaxb */
    }
    else
    {
        bBlockData[3] = (uint8_t)((uint16_t)(pT3T->bNmaxb & 0xFF00) >> 8);
        bBlockData[4] = (uint8_t)(pT3T->bNmaxb & 0x00FF);
    }
    bBlockData[5] = 0x00;
    bBlockData[6] = 0x00;
    bBlockData[7] = 0x00;
    bBlockData[8] = 0x00;
    bBlockData[PHAL_TOP_T3T_WRITE_FLAG_ADDRESS] = 0x00U;
    bBlockData[10] = PHAL_TOP_T3T_AIB_RWA_RW;
    bBlockData[11] = 0x00U;
    bBlockData[12] = 0x00U;
    bBlockData[13] = 0x00U;

    for(wIndex = 0, wCheckSum = 0; wIndex < 14; wIndex++)
    {
        wCheckSum += bBlockData[wIndex];
    }
    bBlockData[14] = (uint8_t)(wCheckSum >> 8);
    bBlockData[15] = (uint8_t)(wCheckSum & 0x00FF);

    PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
        pT3T->phalT3TDataParams,
        0x01,
        bWriteServiceList,
        0x01,
        bBlockList,
        0x02,
        bBlockData));

    pT3T->bVno = bBlockData[0];
    pT3T->bNmaxb = (uint16_t)(((uint16_t)bBlockData[3] << 8) | bBlockData[4]);
    pT3T->bNdefFormatted = PH_SUPPORTED;
    pT3T->bNdefPresence = PH_OFF;
    pT3T->wNdefLength = 0x00;
    pT3T->bTagState = PHAL_TOP_T3T_STATE_INITIALIZED;
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T3T_ReadNdef(
                                       phalTop_T3T_t * pT3T,
                                       uint8_t * pData,
                                       uint16_t * pLength
                                       )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wIndex;                             /* Block Index */
    uint16_t   PH_MEMLOC_REM wTempIndex;
    uint8_t    PH_MEMLOC_REM bBlockData[240];
    uint8_t    PH_MEMLOC_REM bReadBlocks;
    uint8_t    PH_MEMLOC_REM bReadNo;
    uint8_t    PH_MEMLOC_REM bCount = 0;
    uint32_t    PH_MEMLOC_REM bNdefLength;
    uint8_t    PH_MEMLOC_REM bWriteServiceList[2] = { 0x09, 0x00};
    uint8_t    PH_MEMLOC_REM bBlockList[0x1E] = { 0x80, 0x01, 0x80, 0x02, 0x80, 0x03, 0x80, 0x04, 0x80, 0x05,
        0x80, 0x06, 0x80, 0x07, 0x80, 0x08, 0x80, 0x09, 0x80, 0x0A,
        0x80, 0x0B, 0x80, 0x0C, 0x80, 0x0D, 0x80, 0x0E, 0x80, 0x0F};
    uint8_t    PH_MEMLOC_REM bRxNumBlocks = 0;

    if(pT3T->bNdefPresence != PH_ON)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
    }
    if(!((pT3T->bTagState == PHAL_TOP_T3T_STATE_READWRITE) || (pT3T->bTagState == PHAL_TOP_T3T_STATE_READONLY)))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
    }

    bReadBlocks = pT3T->bNbr;
    if(bReadBlocks == 0x00)
    {
        bReadBlocks = 0x01;
    }
    else if(bReadBlocks > 0x0F)
    {
        /* Assumed Max Nbr value is 0x0F */
        bReadBlocks = 0x0F;
    }
    else
    {
        /* Do Nothing */
    }

    bNdefLength = pT3T->wNdefLength;
    bReadNo = (uint8_t)(bNdefLength / (bReadBlocks * PHAL_TOP_T3T_BLOCK_SIZE));
    if(bNdefLength % (bReadBlocks * PHAL_TOP_T3T_BLOCK_SIZE))
    {
        bReadNo += 1;
    }

    for(wIndex = 0x01, wTempIndex = 0;
        wIndex <= bReadNo;
        wIndex++)
    {
        PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
            pT3T->phalT3TDataParams,
            0x01,
            bWriteServiceList,
            bReadBlocks,
            bBlockList,
            (bReadBlocks * 2),
            &bRxNumBlocks,
            bBlockData));

        if(bNdefLength <= (bReadBlocks * PHAL_TOP_T3T_BLOCK_SIZE))
        {
            /* Copy the remaining bytes in the Block excluding the padding bytes */
            memcpy(&pData[wTempIndex], &bBlockData[0], bNdefLength);    /* PRQA S 3200 */
        }
        else
        {
            bNdefLength = bNdefLength - (bReadBlocks * PHAL_TOP_T3T_BLOCK_SIZE);
            memcpy(&pData[wTempIndex], &bBlockData[0], (bReadBlocks * PHAL_TOP_T3T_BLOCK_SIZE));    /* PRQA S 3200 */
            wTempIndex = wTempIndex + (bReadBlocks * PHAL_TOP_T3T_BLOCK_SIZE);
            for(bCount = 0; bCount < bReadBlocks; bCount++)
            {
                bBlockList[(bCount * 2)+ 1] += bReadBlocks;
            }
        }
    }
    *pLength = (uint16_t) pT3T->wNdefLength;
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T3T_WriteNdef(
                                        phalTop_T3T_t * pT3T,
                                        uint8_t * pData,
                                        uint16_t wLength
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wTempIndex;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint16_t   PH_MEMLOC_REM wCheckSum;
    uint8_t    PH_MEMLOC_REM bBlockData[16];
    uint8_t    PH_MEMLOC_REM bAttBlock[2] = { 0x80, 0x00};   /* Read Zero Block to get Attribute Block */
    uint8_t    PH_MEMLOC_REM bBlockList[2] = { 0x80, 0x01};  /* First Block to Write Ndef Data */
    uint8_t    PH_MEMLOC_REM bReadServiceList[2] = { 0x0B, 0x00};
    uint8_t    PH_MEMLOC_REM bWriteServiceList[2] = { 0x09, 0x00};
    uint8_t    PH_MEMLOC_REM bRxNumBlocks = 0;


    if(!((pT3T->bNdefFormatted == PH_SUPPORTED) || (pT3T->bNdefSupport == PH_SUPPORTED)))
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
    }

    /* Check NDEF length */
    if((wLength > (pT3T->bNmaxb * PHAL_TOP_T3T_BLOCK_SIZE)) || (wLength == 0))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Read and Update Attribute Block before Writing NDEF */
    PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
        pT3T->phalT3TDataParams,
        0x01,
        bReadServiceList,
        0x01,                                                /* wNoBlocks */
        bAttBlock,
        0x02,
        &bRxNumBlocks,
        bBlockData));

    if(bBlockData[0x0A] != PHAL_TOP_T3T_AIB_RWA_RW)
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_READONLY_TAG, PH_COMP_AL_TOP);
    }

    bBlockData[PHAL_TOP_T3T_WRITE_FLAG_ADDRESS] = PHAL_TOP_T3T_WRITE_FLAG_SET;
    bBlockData[11] = 0x00U;
    bBlockData[12] = 0x00U;
    bBlockData[13] = 0x00U;

    /* Calculate CheckSum */
    for(wIndex = 0, wCheckSum = 0; wIndex < 14; wIndex++)
    {
        wCheckSum += bBlockData[wIndex];
    }
    bBlockData[14] = (uint8_t)(wCheckSum >> 8);
    bBlockData[15] = (uint8_t)(wCheckSum & 0x00FF);

    PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
        pT3T->phalT3TDataParams,
        0x01,
        bWriteServiceList,
        0x01,
        bAttBlock,
        0x02,
        bBlockData));

    /* Write NDEF Data */
    for(wIndex = 0, wTempIndex = 0;
        wIndex < (wLength / PHAL_TOP_T3T_BLOCK_SIZE);
        wIndex++)
    {
        memcpy(&bBlockData[0], &pData[wTempIndex], PHAL_TOP_T3T_BLOCK_SIZE);    /* PRQA S 3200 */

        PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
            pT3T->phalT3TDataParams,
            0x01,
            bWriteServiceList,
            0x01,
            bBlockList,
            0x02,
            bBlockData));

        wTempIndex += 16;
        if(bBlockList[1] == 0xFF)
        {
            bBlockList[0] += 1;
        }
        else
        {
            bBlockList[1] += 1;
        }
    }

    /* Write the remaining NDEF bytes in the Last Block after adding padding bytes */
    wIndex = wLength % PHAL_TOP_T3T_BLOCK_SIZE;
    if (wIndex != 0)
    {
        memcpy(&bBlockData[0], &pData[wTempIndex], wIndex);     /* PRQA S 3200 */
        memset(&bBlockData[wIndex], 0x00, PHAL_TOP_T3T_BLOCK_SIZE - wIndex);    /* PRQA S 3200 */
        wTempIndex = wTempIndex + wIndex;

        PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
            pT3T->phalT3TDataParams,
            0x01,
            bWriteServiceList,
            0x01,
            bBlockList,
            0x02,
            bBlockData));
    }

    /* User NDEF Lenght should be equal to Written NDEF Lenght */
    if(wLength != wTempIndex)
    {
        return PH_ADD_COMPCODE(PH_ERR_READ_WRITE_ERROR, PH_COMP_AL_TOP);
    }

    /* Read and Update Attribute Block after Writing NDEF */
    PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
        pT3T->phalT3TDataParams,
        0x01,
        bReadServiceList,
        0x01,
        bAttBlock,
        0x02,
        &bRxNumBlocks,
        bBlockData));

    bBlockData[PHAL_TOP_T3T_WRITE_FLAG_ADDRESS] = PHAL_TOP_T3T_WRITE_FLAG_RESET;
    bBlockData[11] = 0x00U;                          /* As the User is able to input 16bit Lenght Upper byte of Ln is 0x00. */
    bBlockData[12] = (uint8_t)(wTempIndex >> 8);
    bBlockData[13] = (uint8_t)(wTempIndex & 0x00FF);

    /* Calculate CheckSum */
    for(wIndex = 0, wCheckSum = 0; wIndex < 14; wIndex++)
    {
        wCheckSum += bBlockData[wIndex];
    }
    bBlockData[14] = (uint8_t)(wCheckSum >> 8);
    bBlockData[15] = (uint8_t)(wCheckSum & 0x00FF);

    PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
        pT3T->phalT3TDataParams,
        0x01,
        bWriteServiceList,
        0x01,
        bAttBlock,
        0x02,
        bBlockData));

    pT3T->bTagState = PHAL_TOP_T3T_STATE_READWRITE;
    pT3T->wNdefLength = wLength;
    pT3T->bNdefPresence = PH_ON;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T3T_EraseNdef(
                                        phalTop_T3T_t * pT3T
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wLength;
    uint16_t   PH_MEMLOC_REM wIndex;
    uint16_t   PH_MEMLOC_REM wTempIndex;
    uint16_t   PH_MEMLOC_REM wCheckSum;
    uint8_t    PH_MEMLOC_REM bBlockData[16];
    uint8_t    PH_MEMLOC_REM bAttBlock[2] = { 0x80, 0x00};  /* Read Zero Block to get Attribute Block */
    uint8_t    PH_MEMLOC_REM bBlockList[2] = { 0x80, 0x01}; /* First Block to Write Ndef Data */
    uint8_t    PH_MEMLOC_REM bReadServiceList[2] = { 0x0B, 0x00};
    uint8_t    PH_MEMLOC_REM bWriteServiceList[2] = { 0x09, 0x00};
    uint8_t    PH_MEMLOC_REM bRxNumBlocks = 0;

    /* Read and Update Attribute Block before Writing NDEF */
    PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
        pT3T->phalT3TDataParams,
        0x01,
        bReadServiceList,
        0x01,
        bAttBlock,
        0x02,
        &bRxNumBlocks,
        bBlockData));

    bBlockData[PHAL_TOP_T3T_WRITE_FLAG_ADDRESS] = PHAL_TOP_T3T_WRITE_FLAG_SET;
    wLength = (uint16_t)(((uint16_t)bBlockData[12] << 8) | bBlockData[13]);
    bBlockData[11] = 0x00U;
    bBlockData[12] = 0x00U;
    bBlockData[13] = 0x00U;

    /* Calculate CheckSum */
    for(wIndex = 0, wCheckSum = 0; wIndex < 14; wIndex++)
    {
        wCheckSum += bBlockData[wIndex];
    }
    bBlockData[14] = (uint8_t)(wCheckSum >> 8);
    bBlockData[15] = (uint8_t)(wCheckSum & 0x00FF);

    PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
        pT3T->phalT3TDataParams,
        0x01,
        bWriteServiceList,
        0x01,
        bAttBlock,
        0x02,
        bBlockData));

    if(wLength % PHAL_TOP_T3T_BLOCK_SIZE)
    {
        wLength = (wLength / PHAL_TOP_T3T_BLOCK_SIZE) + 1;
    }
    else
    {
        wLength = wLength / PHAL_TOP_T3T_BLOCK_SIZE;
    }
    /* Write 0s in NDEF Block area */
    for(wIndex = 0, wTempIndex = 0;
        wIndex < wLength + 1;
        wIndex++)
    {
        memset(&bBlockData[0], 0x00, PHAL_TOP_T3T_BLOCK_SIZE);  /* PRQA S 3200 */

        PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
            pT3T->phalT3TDataParams,
            0x01,
            bWriteServiceList,
            0x01,
            bBlockList,
            0x02,
            bBlockData));

        wTempIndex += 16;
        if(bBlockList[1] == 0xFF)
        {
            bBlockList[0] += 1;
        }
        else
        {
            bBlockList[1] += 1;
        }
    }

    /* Read and Update Attribute Block after Writing 0s in NDEF area */
    PH_CHECK_SUCCESS_FCT(status, phalFelica_Read(
        pT3T->phalT3TDataParams,
        0x01,
        bReadServiceList,
        0x01,
        bAttBlock,
        0x02,
        &bRxNumBlocks,
        bBlockData));

    bBlockData[PHAL_TOP_T3T_WRITE_FLAG_ADDRESS] = PHAL_TOP_T3T_WRITE_FLAG_RESET;
    bBlockData[11] = 0x00U;
    bBlockData[12] = 0x00U;
    bBlockData[13] = 0x00U;

    /* Calculate CheckSum */
    for(wIndex = 0, wCheckSum = 0; wIndex < 14; wIndex++)
    {
        wCheckSum += bBlockData[wIndex];
    }
    bBlockData[14] = (uint8_t)(wCheckSum >> 8);
    bBlockData[15] = (uint8_t)(wCheckSum & 0x00FF);

    PH_CHECK_SUCCESS_FCT(status, phalFelica_Write(
        pT3T->phalT3TDataParams,
        0x01,
        bWriteServiceList,
        0x01,
        bAttBlock,
        0x02,
        bBlockData));

    pT3T->bNdefPresence = PH_OFF;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}


#endif /* NXPBUILD__PHAL_TOP_SW */
