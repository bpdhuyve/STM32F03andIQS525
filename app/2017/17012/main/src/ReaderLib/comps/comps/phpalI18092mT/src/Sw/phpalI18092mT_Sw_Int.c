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
* Internal functions for software ISO18092 target mode component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 458 $
* $Date: 2014-01-04 18:36:24 +0530 (Sat, 04 Jan 2014) $
*
* History:
*  CEn: Generated 07. September 2010
*/

#include <ph_Status.h>
#include <phhalHw.h>
#include <ph_RefDefs.h>
#include <phOsal.h>

#ifdef NXPBUILD__PHPAL_I18092MT_SW
#include "phpalI18092mT_Sw.h"
#include "phpalI18092mT_Sw_Int.h"

phStatus_t phpalI18092mT_Sw_WriteFrameHeader(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t bCommandCode,
    uint8_t bDataLength
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    uint8_t     PH_MEMLOC_REM aHeader[4];
    uint8_t     PH_MEMLOC_REM bLength = 0;

    /* At a datarate of 106kbps an additional start byte needs to be added. ISO/IEC 18092:2004(E) section 12.1 */
    if ((pDataParams->bDst == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
    {
        aHeader[bLength++] = PHPAL_I18092MT_SW_START_BYTE;
    }

    bDataLength++; /* Include the length of start byte */


    /* Set LEN byte (includes the length itself). */
    aHeader[bLength++] = bDataLength + 2/*Command Length*/;

    /* Only NFC initiator mode is supported so only requests are sent. */
    aHeader[bLength++] = PHPAL_I18092MT_SW_CMD_RESPONSE;

    /* Set the command code. */
    aHeader[bLength++] = bCommandCode;

    PH_CHECK_SUCCESS_FCT(status, phhalHw_Exchange(
        pDataParams->pHalDataParams,
        wOption,
        aHeader,
        (uint16_t)bLength,
        NULL,
        NULL));
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pFrame,
    uint16_t wFrameLength,
    uint8_t bExpectedResponseCode,
    uint8_t ** ppValidatedFrame,
    uint16_t * pValidatedFrameLength,
    uint8_t ** ppPayload,
    uint16_t * pPayloadLength
    )
{
    phStatus_t  PH_MEMLOC_REM bOffset = 0;
    uint8_t     PH_MEMLOC_REM bPfb;

    /* TODO: Check for technology type detected and set the DRi by possibly 
       reading the baud rate info from some hardware register. */

    /* At 106kbps all fields are off-set by one. */
    if ((pDataParams->bDst == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
    {

        /* Check maximum frame length */
        if (wFrameLength > (PHPAL_I18092MT_SW_MAX_FRAMESIZE+1))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }

        /* Check minimum frame length */
        if (wFrameLength < (1 + PHPAL_I18092MT_SW_MIN_FRAMESIZE))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }

        /* Check for start byte presence and ignore it. */
        if (pFrame[bOffset++] != PHPAL_I18092MT_SW_START_BYTE)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }
    else
    {
        /* Check maximum frame length */
        if (wFrameLength > PHPAL_I18092MT_SW_MAX_FRAMESIZE)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
        /* Check minimum frame length */
        if (wFrameLength < PHPAL_I18092MT_SW_MIN_FRAMESIZE)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }
    /* Remove Startbyte if present */
    *ppValidatedFrame = &pFrame[bOffset];
    *pValidatedFrameLength = wFrameLength - (uint16_t)bOffset;

    /* Compare length field with frame length. */
    if (pFrame[bOffset++] != (uint8_t)*pValidatedFrameLength)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
    }

    /* Check if the received frame is a reponse frame. */
    if (pFrame[bOffset++] != PHPAL_I18092MT_SW_CMD_REQUEST)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
    }

    /* Check if expected response code matches the received response code. */
    if (pFrame[bOffset++] != bExpectedResponseCode)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
    }

    /* In case the request is a DEP PDU also check for presence of DID, NAD and PNI. */
    if (bExpectedResponseCode == PHPAL_I18092MT_SW_CMD_DEP_REQ)
    {
        /* PFB should be present in frame. */
        if (wFrameLength < (bOffset + 1))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }

        /* Retrieve PFB */
        bPfb = pFrame[bOffset++];

        /* DID enabled */
        if (pDataParams->bDid > 0)
        {
            /* DID presence should be indicated. */
            if (!(bPfb & PHPAL_I18092MT_SW_PFB_DID_MASK))
            {
                return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }
            /* DID should be present in frame. */
            if (wFrameLength < (bOffset + 1))
            {
                return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }
            /* DID should match. */
            if (pFrame[bOffset++] != pDataParams->bDid)
            {
                return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }
        }
        /* DID disabled */
        else
        {
            /* DID presence should not be indicated. */
            if (bPfb & PHPAL_I18092MT_SW_PFB_DID_MASK)
            {
                return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }
        }

        /* Check NAD only for first PDU frames. */
        if (!(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT) &&
            !(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_LAST_BIT))
        {
            /* NAD enabled */
            if (pDataParams->bNadEnabled)
            {
                /* NAD presence should be indicated. */
                if (!(bPfb & PHPAL_I18092MT_SW_PFB_NAD_MASK))
                {
                    return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
                }
                /* NAD should be present in frame. */
                if (wFrameLength < (bOffset + 1))
                {
                    return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
                }
                /* NAD should match. */
                if (pFrame[bOffset++] != pDataParams->bNad)
                {
                    return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
                }
            }
            /* NAD disabled */
            else
            {
                /* NAD presence should not be indicated. */
                if (bPfb & PHPAL_I18092MT_SW_PFB_NAD_MASK)
                {
                    return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
                }
            }
        }

        /* For information or ACK PDU also check for PNI. */
        switch (bPfb & PHPAL_I18092MT_SW_PFB_PDU_TYPE_MASK)
        {
        case PHPAL_I18092MT_SW_PFB_INFORMATION_PDU_MASK:
        case PHPAL_I18092MT_SW_PFB_ACK_PDU_MASK:
            if ((bPfb & PHPAL_I18092MT_SW_PFB_PNI_MASK) != pDataParams->bPni)
            {
                //printf("Error 18 \n");
                //return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }
            break;

        default:
            /* nothing. */
            break;
        }
    }

    /* Return Payload. */
    *ppPayload = &pFrame[bOffset];
    *pPayloadLength = wFrameLength - bOffset;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}


phStatus_t phpalI18092mT_Sw_GetPduHeaderLength(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pProtLength
    )
{
    /* init. protocol lentgh */
    *pProtLength = 4;
   /* The protocol adds the start byte incase of Type_A 106, so we should ignore this increment */

    if ((pDataParams->bDst == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
    {
        ++(*pProtLength);
    }

    /* Add DID if used */
    if (pDataParams->bDid > 0)
    {
        ++(*pProtLength);
    }
    /* Add NAD if enabled and chaining is not used. */
    if (pDataParams->bNadEnabled)
    {
        if ((pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_FIRST_BIT) ||
            (
            !(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT) &&
            !(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_LAST_BIT)
            ))
        {
            ++(*pProtLength);
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_UpdatePduHeader(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t bDataLength
    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wPfb;
    uint8_t     PH_MEMLOC_REM bLengthPos = 0;
    uint8_t     PH_MEMLOC_REM bProtLength;

    /* Ignore TypeA Startbyte if present */
    if ((pDataParams->bDst == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
    {
        ++bLengthPos;
    }

    /* Retrieve protocol header length */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_GetPduHeaderLength(pDataParams, &bProtLength));

    /* Add protocol header length to overall length */
    bDataLength = bProtLength + bDataLength;

    /* Prepare access to length position */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_ADDITIONAL_INFO, (uint16_t)bLengthPos));

    /* Write new length */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_TXBUFFER, (uint16_t)bDataLength));

    /* Prepare access to PFB position */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_ADDITIONAL_INFO, (uint16_t)(bLengthPos + 3)));

    /* Read current PFB */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_TXBUFFER, &wPfb));

    /* Set chaining bit if needed. */
    if (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT)
    {
        if (!(wPfb & PHPAL_I18092MT_SW_PFB_OPTION_MASK))
        {
            wPfb |= PHPAL_I18092MT_SW_PFB_OPTION_MASK;

            /* Write new PFB */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_TXBUFFER, wPfb));
        }
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}


phStatus_t phpalI18092mT_Sw_WritePduHeader(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t bPduMask,
    uint8_t bOption,
    uint8_t bDataLength
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aCmd[3];
    uint8_t     PH_MEMLOC_REM bCmdLength = 0;

    aCmd[bCmdLength++] = bPduMask;

    /* Add PNI only if PDU is not supervisory. */
    if (bPduMask != PHPAL_I18092MT_SW_PFB_SUPERVISORY_PDU_MASK)
    {
        aCmd[0] |= pDataParams->bPni;
    }

    /* Add DID if enabled. */
    if (pDataParams->bDid > 0)
    {
        aCmd[0] |= PHPAL_I18092MT_SW_PFB_DID_MASK;
        aCmd[bCmdLength++] = pDataParams->bDid;
    }

    /* Add NAD if enabled and chaining is not used. */
    if ((pDataParams->bNadEnabled) && (bPduMask != PHPAL_I18092MT_SW_PFB_ACK_PDU_MASK))
    {
        if ((pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_FIRST_BIT) ||
            (
            !(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT) &&
            !(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_LAST_BIT)
            ))
        {
            aCmd[0] |= PHPAL_I18092MT_SW_PFB_NAD_MASK;
            aCmd[bCmdLength++] = pDataParams->bNad;
        }
    }

    /* Set option bit. */
    if (bOption != PH_OFF)
    {
        aCmd[0] |= PHPAL_I18092MT_SW_PFB_OPTION_MASK;
    }

    /* Add command to overall length */
    bDataLength = bDataLength + bCmdLength;


    /* Write frame header */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_WriteFrameHeader(pDataParams, PH_EXCHANGE_BUFFER_FIRST, PHPAL_I18092MT_SW_CMD_DEP_RES, bDataLength));

    /* Append command */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_Exchange(
        pDataParams->pHalDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        aCmd,
        (uint16_t)bCmdLength,
        NULL,
        NULL));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}


phStatus_t phpalI18092mT_Sw_TransceivePdu(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t * pTxBuffer,
    uint16_t wTxLength,
    uint8_t ** ppRawResponse,
    uint16_t * pRawResponseLength,
    uint8_t ** ppNormalizedResponse,
    uint16_t * pNormalizedResponseLength,
    uint8_t ** ppPayload,
    uint16_t * pPayloadLength
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    phStatus_t  PH_MEMLOC_REM bRetransmit = PH_OFF;
    uint8_t     PH_MEMLOC_REM bPfb;
    uint16_t    PH_MEMLOC_REM wRxBufferStartPos;
    uint16_t    PH_MEMLOC_REM wRxBufferStartPosTmp;
    uint8_t     PH_MEMLOC_REM bPni;
    uint8_t     PH_MEMLOC_REM bSendAtn = PH_OFF;
    uint16_t    PH_MEMLOC_REM wTimeoutPrev = 0;
    uint8_t     PH_MEMLOC_REM bTimeoutInMs = 0;

    /* Reset returned lengths */
    *pPayloadLength = 0;
    *pNormalizedResponseLength = 0;

    /* Before starting transmission get current read buffer position. */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
        pDataParams->pHalDataParams,
        PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
        &wRxBufferStartPos));
    wRxBufferStartPosTmp = wRxBufferStartPos;



    /* This is absolutely not allowed ;-) */
    if (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_RETRANSMIT_BIT)
    {
        return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_PAL_I18092MT);
    }

    /* Get current timeout */
    status = phhalHw_GetConfig(
        pDataParams->pHalDataParams,
        PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,
        &wTimeoutPrev);
    if ((status & PH_ERR_MASK) == PH_ERR_PARAMETER_OVERFLOW)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
                pDataParams->pHalDataParams,
                PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,
                &wTimeoutPrev));
        bTimeoutInMs = 1;
    }
    else
    {
        PH_CHECK_SUCCESS(status);
        bTimeoutInMs = 0;
    }

    /* Set maximum timeout value for exchange */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
        pDataParams->pHalDataParams,
        PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,
        (uint16_t)(PHPAL_I18092MT_SW_RWT_MAX_US / 1000)));

    /* Retry loop */
    do
    {

        /* Copy payload to internal HAL buffer and start transmission. */
        status = phhalHw_Exchange(
            pDataParams->pHalDataParams,
            wOption,
            pTxBuffer,
            wTxLength,
            ppRawResponse,
            pRawResponseLength);

        /* Only internal buffer was written. */
        if (wOption & PH_EXCHANGE_BUFFERED_BIT)
        {
            return status;
        }

        /* Prepare Exchange Option for retry. */
        wOption = PH_EXCHANGE_BUFFER_LAST;

        /* If the evaluation below succeeds this leads to a successful loop termination */
        wTxLength = 0;

        /* Exchange was successful */
        if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            /* Check for valid response frame. */
            statusTmp = phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
                pDataParams,
                &(*ppRawResponse)[wRxBufferStartPosTmp],
                (*pRawResponseLength - wRxBufferStartPosTmp),
                PHPAL_I18092MT_SW_CMD_DEP_REQ,
                ppNormalizedResponse,
                pNormalizedResponseLength,
                ppPayload,
                pPayloadLength);

            if (statusTmp != PH_ERR_SUCCESS)
            {
                 /*Restart transcieve*/
                PH_CHECK_SUCCESS_FCT(status, phhalHw_Receive(
                     pDataParams->pHalDataParams,
                     PHHAL_HW_OPTION_START_CMD_ONLY | PHHAL_HW_OPTION_START_TRANSCIEVE,
                     NULL,
                     NULL));
                return statusTmp;
            }

            /* retrieve PFB */
            bPfb = (*ppNormalizedResponse)[3];

            /* Reset bSendAtn flag */
            bSendAtn = PH_OFF;

            /* If a NACK is received, check PNI and need to retransmit the packet, ISO_IEC_18092-NFCIP-1 12.6.1.3.3(3) */
            if (PHPAL_I18092MT_SW_IS_NACK(bPfb))
            {
                bPni = bPfb & 0x03;
                if (bPni != pDataParams->bPni)
                {
                    bRetransmit = PH_OFF;
                    status = PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
                }
                else
                {
                    bRetransmit = PH_ON;
                }
            }
            else if (PHPAL_I18092MT_SW_IS_ACK(bPfb))
            {
                /* ACK only allowed as an reply to a chained PDU. NFCForum-TS-DigitalProtocol-1.0 section 14.12.4.3. */
                if (((pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_MASK) == PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_TX) &&
                    (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT))
                {
                    /* Valid ACK received, increment PNI. NFCForum-TS-DigitalProtocol-1.0 section 14.12.3.3. */
                    pDataParams->bPni = PHPAL_I18092MT_SW_INCREMENT_PNI(pDataParams->bPni);
                }
                else
                {
                    status = PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
                }
            }
            else if (PHPAL_I18092MT_SW_IS_ATTENTION(bPfb))
            {
              bSendAtn = PH_ON;

                /* Prepare Attention PDU. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_WritePduHeader(
                    pDataParams,
                    PHPAL_I18092MT_SW_PFB_SUPERVISORY_PDU_MASK,
                    PH_OFF,
                    0));

                pDataParams->bPni--;
            }
            else if (PHPAL_I18092MT_SW_IS_INFORMATION_PDU(bPfb))
            {
                /* During Tx-Chaining only ACKs are accepted. */
                if (((pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_MASK) == PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_TX) &&
                    (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT))
                {
                    status = PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
                }
                else
                {
                    /* Evaluate chaining bit */
                    if (PHPAL_I18092MT_SW_IS_CHAINING(bPfb))
                    {
                        /* Chaining bit set - ack to be sent, set chaining state. */
                        pDataParams->bStateNow |= PHPAL_I18092MT_SW_STATE_CHAINING_BIT;
                    }
                    else
                    {
                        /* Chaining bit not set - no ack to be sent, transmission is over. */
                        pDataParams->bStateNow &= (uint8_t)~(uint8_t)PHPAL_I18092MT_SW_STATE_CHAINING_BIT;
                    }

                    /* Valid information PDU received, increment PNI. NFCForum-TS-DigitalProtocol-1.0 section 14.12.3.3. */
                    //pDataParams->bPni = PHPAL_I18092MT_SW_INCREMENT_PNI(pDataParams->bPni);
                }
            }
            else 
            {
                /* Set error code */
                status = PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }
        }
        /* Invalid PDU received, stay in the current state and do not answer, ISO_IEC_18092-NFCIP-1 section 12.6.1.3.3(4) */
        else if (PHPAL_I18092MT_SW_IS_INVALID_BLOCK_STATUS(status))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
        else
        {
            /* MIFARE compliancy: force protocol error on NAK */
            if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_INCOMPLETE_BYTE)
            {
                return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }
        }
    }
    while (bRetransmit | bSendAtn);

    /* Set previous timeout value */
    if (!bTimeoutInMs)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,
            wTimeoutPrev));
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,
            wTimeoutPrev));
    }

    return status;
}

phStatus_t phpalI18092mT_Sw_StartRecv(
        phpalI18092mT_Sw_DataParams_t * pDataParams,
        void *ovrTask
        )
{
    phStatus_t status = PH_ERR_SUCCESS;

    pDataParams->ovrTask = ovrTask;

    /* Enable the receiver Interrupt and configure the ISR */
    pDataParams->pSet_Interrupt(PH_ON);

    PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

void phpalI18092mT_Sw_RtoxCallBck(uint32_t TimerId, void *pContext)
{
    uint16_t   PH_MEMLOC_REM  NormalizedResponseLength;
    uint8_t *  PH_MEMLOC_REM  pNormalizedResponse = NULL ;
    uint8_t *  PH_MEMLOC_REM  pPayload = NULL;
    uint16_t   PH_MEMLOC_REM  PayloadLength = 0;
    uint16_t   PH_MEMLOC_REM  wRtoxReqLen = 1;
    uint16_t   PH_MEMLOC_REM  wRtoxResLen = 0;
    uint8_t *  PH_MEMLOC_REM  pRtoxResBuf = NULL;
    phpalI18092mT_Sw_DataParams_t * PH_MEMLOC_REM  pDataParams = (phpalI18092mT_Sw_DataParams_t*)pContext;
    uint8_t *  PH_MEMLOC_REM  pRtoxReqBuf = &pDataParams->bRtox;

    pDataParams->rtoxStatus = phpalI18092mT_Sw_WritePduHeader(
                                    pDataParams,
                                    PHPAL_I18092MT_SW_PFB_SUPERVISORY_PDU_MASK,
                                    PH_ON,
                                    (uint8_t)wRtoxReqLen);

    if((pDataParams->rtoxStatus & PH_COMP_MASK) == PH_ERR_SUCCESS)
    {
        pDataParams->rtoxStatus = phhalHw_Exchange(
                                        pDataParams->pHalDataParams,
                                        PH_EXCHANGE_LEAVE_BUFFER_BIT,
                                        pRtoxReqBuf,
                                        wRtoxReqLen,
                                        &pRtoxResBuf,
                                        &wRtoxResLen);
    }
 
    if((pDataParams->rtoxStatus & PH_COMP_MASK) == PH_ERR_SUCCESS)
    {
        pDataParams->rtoxStatus = phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
                                        pDataParams,
                                        pRtoxResBuf,
                                        wRtoxResLen,
                                        PHPAL_I18092MT_SW_CMD_DEP_REQ,
                                        &pNormalizedResponse,
                                        &NormalizedResponseLength,
                                        &pPayload,
                                        &PayloadLength);
        if (pDataParams->rtoxStatus != PH_ERR_SUCCESS)
        {
             /*Restart transcieve*/
            phhalHw_Receive(
                 pDataParams->pHalDataParams,
                 PHHAL_HW_OPTION_START_CMD_ONLY | PHHAL_HW_OPTION_START_TRANSCIEVE,
                 NULL,
                 NULL);
        }
    }

    if(((pDataParams->rtoxStatus & PH_COMP_MASK) == PH_ERR_SUCCESS) && 
       (pPayload != NULL) && (pNormalizedResponse != NULL))     /* NULL check added to avoid QAC warning */
    {
        if((PayloadLength != wRtoxReqLen) || (pPayload[0] != pRtoxReqBuf[0]) ||
        (pNormalizedResponse[3] != (PHPAL_I18092MT_SW_PFB_OPTION_MASK | PHPAL_I18092MT_SW_PFB_SUPERVISORY_PDU_MASK)))
        {
            pDataParams->rtoxStatus = PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }
}

#endif  /* NXPBUILD__PHPAL_I18092MT_SW */

