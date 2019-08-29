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
* $Author: nxp40786 $
* $Revision: 486 $
* $Date: 2014-01-31 09:38:54 +0530 (Fri, 31 Jan 2014) $
*
* History:
*  CEn: Generated 07. September 2010
*/

#include <ph_Status.h>
#include <phhalHw.h>
#include <phpalI18092mT.h>
#include <ph_RefDefs.h>
#include <phOsal.h>

#ifdef NXPBUILD__PHPAL_I18092MT_SW

#include "phpalI18092mT_Sw.h"
#include "phpalI18092mT_Sw_Int.h"

/* static const uint16_t PH_MEMLOC_CONST_ROM aI18092_FsTable[4] = {64, 128, 192, 253};
 *  We need to add one because for calculating the maximum payload length, we should have byte for length field
 *  which will be added as a part of payload...The Lower tester confirms the payload is the string without length
 *  field.
 *  */
static const uint16_t PH_MEMLOC_CONST_ROM aI18092_FsTable[4] = {65, 129, 193, 255};
static const uint16_t PH_MEMLOC_CONST_ROM aI18092_DrTable[3] = {PHHAL_HW_RF_DATARATE_106, PHHAL_HW_RF_DATARATE_212, PHHAL_HW_RF_DATARATE_424};


phStatus_t phpalI18092mT_Sw_Init(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint16_t wSizeOfDataParams,
    void * pHalDataParams,
    pInterruptSetCallback pSetInterrupt
    )
{
    if (sizeof(phpalI18092mT_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_I18092MT);
    }
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pHalDataParams);

    /* Init private data */
    pDataParams->wId            = PH_COMP_PAL_I18092MT | PHPAL_I18092MT_SW_ID;
    pDataParams->pHalDataParams = pHalDataParams;
    pDataParams->pSet_Interrupt = pSetInterrupt;
 

    /* Reset protocol to defaults */
    return phpalI18092mT_Sw_ResetProtocol(pDataParams);
}

phStatus_t phpalI18092mT_Sw_ResetProtocol(
    phpalI18092mT_Sw_DataParams_t * pDataParams
    )
{
    /* Apply default parameters */
    pDataParams->bNfcIdValid        = PH_OFF;
    pDataParams->bStateNow          = PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_TX;
    pDataParams->bDid               = 0;    /* set DID to 0, i.e. disable usage of DID */
    pDataParams->bNadEnabled        = PH_OFF;
    pDataParams->bNad               = 0;
    pDataParams->bWt                = PHPAL_I18092MT_WT_MAX;
    pDataParams->bFsl               = PHPAL_I18092MT_FRAMESIZE_64;
    pDataParams->bPni               = 0;    /* NFCForum-TS-DigitalProtocol-1.0 section 14.12.3.1 */
    pDataParams->bDst               = PHPAL_I18092MT_DATARATE_106;
    pDataParams->bDrt               = PHPAL_I18092MT_DATARATE_106;
    pDataParams->bBsi               = 0;
    pDataParams->bBri               = 0;
    pDataParams->bLri               = PHPAL_I18092MT_FRAMESIZE_64;
    pDataParams->bBst               = 0;
    pDataParams->bBrt               = 0;
    pDataParams->bLrt               = PHPAL_I18092MT_FRAMESIZE_64;
    pDataParams->bMaxRetryCount     = PHPAL_I18092MT_SW_MAX_RETRIES_DEFAULT;
    pDataParams->pGt                = NULL;
    pDataParams->bGtLength          = 0;
    pDataParams->bTo                = 0;
    pDataParams->bTargetState       = PHPAL_I18092MT_STATE_NONE;
    pDataParams->bRtoxDisabled      = PH_OFF;
    pDataParams->ovrTask            = NULL;
    pDataParams->bCmdtype           = PHPAL_I18092MT_SW_CMD_ATR_REQ;
    pDataParams->rtoxStatus         = PH_ERR_SUCCESS;
    pDataParams->bRtox              = PHPAL_I18092MT_SW_MIN_RTOX_VALUE;
    pDataParams->bActiveMode        = PH_OFF;
    pDataParams->bAutocollRetryCount= 0;

    if((pDataParams->phOsal != NULL ) &&(pDataParams->dwTimerId != PH_OSAL_INVALID_TIMER_ID))
    {
        (void)phOsal_Timer_Delete(pDataParams->phOsal, pDataParams->dwTimerId);
    }

    pDataParams->dwTimerId      = PH_OSAL_INVALID_TIMER_ID;
    pDataParams->phOsal         = NULL;
    memset(pDataParams->aNfcid3i, 0, PHPAL_I18092MT_NFCID3_LENGTH);     /* PRQA S 3200 */
    memset(pDataParams->aNfcid3t, 0, PHPAL_I18092MT_NFCID3_LENGTH);     /* PRQA S 3200 */

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_AtrRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pAtr,
    uint16_t wAtrLength
    )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM *pFrame;
    uint16_t    PH_MEMLOC_REM wFrameLength;
    uint8_t     PH_MEMLOC_REM *pPayload;
    uint16_t    PH_MEMLOC_REM wPayloadLength;
    uint8_t     PH_MEMLOC_REM aCmd[100];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;


    /* Make NfcID invalid */
    pDataParams->bNfcIdValid = PH_OFF;

    statusTmp = phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
            pDataParams,
            pAtr,
            wAtrLength,
            PHPAL_I18092MT_SW_CMD_ATR_REQ,
            &pFrame,
            &wFrameLength,
            &pPayload,
            &wPayloadLength
            );

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

    /* Ignore length byte */
    ++pFrame;
    --wFrameLength;

    /* Check for minimum and maximum frame size. NFCForum-TS-DigitalProtocol-1.0 section 14.6.1.3. */
    if ((wFrameLength > PHPAL_I18092MT_SW_MAX_ACTIVATION_FRAME_LENGTH) ||
        (wFrameLength < PHPAL_I18092MT_SW_MIN_ACTIVATION_FRAME_LENGTH))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
    }
    /* The format for ATR_REQ, 12.5.1.1 */
    /*
      +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
      | CMD 0  | CMD 1  | Byte 0 | Byte 9 | Byte 10| Byte 11| Byte 12| Byte 13| Byte 14| Byte n |
      +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
      |  (D4)  |  (00)  |nfcid3i0|nfcid3i9| DIDi   |  BSi   |  BRi   |  PPi   | [Gi[0]]| [Gi[n]|
      +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+

    */

    /* Skip the command bytes, CMD0 and CMD1
       and copy the NFCID3 of the initiator store that info */
    memmove(pDataParams->aNfcid3i, &pFrame[2], PHPAL_I18092MT_NFCID3_LENGTH);  /* PRQA S 3200 */
    /* Copy the DID, BSI and BRI */
    pDataParams->bDid = pFrame[PHPAL_I18092MT_SW_ATR_REQ_DID_POS];
    pDataParams->bBsi = pFrame[PHPAL_I18092MT_SW_ATR_REQ_BSI_POS] & 0x0F;
    pDataParams->bBri = pFrame[PHPAL_I18092MT_SW_ATR_REQ_BRI_POS] & 0x0F;

    /* Extract LRi bits,  Gi and NADen bit , 12.5.1.1

       +--------+--------+--------+--------+--------+--------+--------+--------+
       | bit 7  | bit 6  | bit 5  | bit 4  | bit 3  | bit 2  | bit 1  | bit 0  |
       +--------+--------+--------+--------+--------+--------+--------+--------+
       |   0    |   0    |  LRi   |  LRi   |   0    |   0    |   Gi   |  NAD   |
       +--------+--------+--------+--------+--------+--------+--------+--------+

    */

    pDataParams->bLri = (pFrame[PHPAL_I18092MT_SW_ATR_REQ_PPI_POS] >> 4) & 0x03;
    if((pFrame[PHPAL_I18092MT_SW_ATR_REQ_PPI_POS] & PHPAL_I18092MT_SW_ATR_REQ_PPI_NAD_USED))
    {
        pDataParams->bNadEnabled = PH_ON;
    }
    else
    {
        /* No NAD support */
        pDataParams->bNad = 0;
        pDataParams->bNadEnabled = PH_OFF;
    }

    /* Check Gi presence/absence */
    if (pFrame[PHPAL_I18092MT_SW_ATR_REQ_PPI_POS] & PHPAL_I18092MT_SW_ATR_REQ_PPI_GT_AVAILABLE)
    {
        /* Gi should be present */
        if (wFrameLength <= PHPAL_I18092MT_SW_MIN_ACTIVATION_FRAME_LENGTH)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }
    else
    {
        /* Gi should be absent */
        if (wFrameLength > PHPAL_I18092MT_SW_MIN_ACTIVATION_FRAME_LENGTH)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
     }

    /* Form the AtrRes frame */

    /* The format of ATR RES

       +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
       | CMD 0  | CMD 1  | Byte 0...Byte 9 | Byte 10| Byte 11| Byte 12| Byte 13| Byte 14| Byte 15| Byte n |
       +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
       |  (D5)  |  (01)  |nfcid3i0|nfcid3i9| DIDt   |  BSt   |  BRt   |   TO   |  PPt   | [Gi[0]]| [Gi[n] |
       +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+

    */
    memcpy(&aCmd[bCmdLen], pDataParams->aNfcid3t, PHPAL_I18092MT_NFCID3_LENGTH);  /* PRQA S 3200 */
    bCmdLen = PHPAL_I18092MT_NFCID3_LENGTH;
    aCmd[bCmdLen++] = pDataParams->bDid;
    aCmd[bCmdLen++] = pDataParams->bBst;
    aCmd[bCmdLen++] = pDataParams->bBrt;
    aCmd[bCmdLen++] = pDataParams->bTo;

    /* Configure PPt field */
    aCmd[bCmdLen++] = (pDataParams->bLrt << PHPAL_I18092MT_SW_ATR_PPT_LRT_BIT);

    if (pDataParams->bNadEnabled != PH_OFF)
    {
        aCmd[bCmdLen-1] |= 0x01;
    }
    if (pDataParams->bGtLength > 0)
    {
        aCmd[bCmdLen-1] |= PHPAL_I18092MT_SW_ATR_RES_PPT_GT_AVAILABLE;
    }


    /*
    Find the frame size setting which is supported by both, initiator and target.
    May be overwritten by optional PSL request.
    NFCForum-TS-DigitalProtocol-1.0 section 14.12.2.3.
    */
    pDataParams->bFsl = (pDataParams->bLrt < pDataParams->bLri) ? pDataParams->bLrt : pDataParams->bLri;

    /* Write frame header. */
    PH_CHECK_SUCCESS_FCT(
        statusTmp,
        phpalI18092mT_Sw_WriteFrameHeader(
            pDataParams,
            PH_EXCHANGE_BUFFER_FIRST,
            PHPAL_I18092MT_SW_CMD_ATR_RES,
            (bCmdLen + pDataParams->bGtLength)));

    if (pDataParams->bGtLength > 0)
    {
        /* Write data to internal buffer. */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Exchange(
                                 pDataParams->pHalDataParams,
                                 PH_EXCHANGE_BUFFER_CONT,
                                 aCmd,
                                (uint16_t)bCmdLen,
                                 NULL,
                                 NULL));

        /* Write general bytes, transmit the whole buffer and receive response. */
         PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Exchange(pDataParams->pHalDataParams,
            PH_EXCHANGE_BUFFER_LAST,
            pDataParams->pGt,
            (pDataParams->bGtLength),
            NULL,
            NULL));
                   
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Exchange(
            pDataParams->pHalDataParams,
            PH_EXCHANGE_BUFFER_LAST,
            aCmd,
            (uint16_t)bCmdLen,
            NULL,
            NULL));

    }

    return status;
}

phStatus_t phpalI18092mT_Sw_DepSend(
        phpalI18092mT_Sw_DataParams_t * pDataParams,
        uint16_t wOption,
        uint8_t * pTxBuffer,
        uint16_t wTxLength)
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;

    /* Used to build block frames */
    uint16_t    PH_MEMLOC_REM wTxBufferSize;
    uint16_t    PH_MEMLOC_REM wTxBufferLen;
    uint16_t    PH_MEMLOC_REM wMaxFrameSize;
    uint16_t    PH_MEMLOC_REM wBytesToSend;
    uint8_t     PH_MEMLOC_REM bHeaderLength;
   

    /* Used for Exchange */
    uint16_t    PH_MEMLOC_REM wHalExchangeOption;
    uint16_t    PH_MEMLOC_REM wRxBufferSize;
    uint8_t   * PH_MEMLOC_REM pRawResponse;
    uint16_t    PH_MEMLOC_REM wRawResponseLength;
    uint8_t   * PH_MEMLOC_REM pNormalizedResponse;
    uint16_t    PH_MEMLOC_REM wNormalizedResponseLength;
    uint8_t   * PH_MEMLOC_REM pPayloadResponse;
    uint16_t    PH_MEMLOC_REM wPayloadLength;

    /* Disable the receiver Interrupt and configure the ISR */
    pDataParams->pSet_Interrupt(PH_OFF);
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RXMULTIPLE, PH_OFF));

    /* Option parameter check */
    if (wOption &  (uint16_t)~(uint16_t)
        (
        PH_EXCHANGE_BUFFERED_BIT | PH_EXCHANGE_LEAVE_BUFFER_BIT |
        PH_EXCHANGE_TXCHAINING | PH_EXCHANGE_RXCHAINING | PH_EXCHANGE_RXCHAINING_BUFSIZE
        ))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }

    if((pDataParams->dwTimerId != PH_OSAL_INVALID_TIMER_ID))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Stop(pDataParams->phOsal, pDataParams->dwTimerId));
        PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Reset(pDataParams->phOsal, pDataParams->dwTimerId));
    }

    if((pDataParams->dwTimerId != PH_OSAL_INVALID_TIMER_ID) && ((pDataParams->rtoxStatus & PH_COMP_MASK) != PH_ERR_SUCCESS))
    {
        return pDataParams->rtoxStatus;
    }

    /* Retrieve HAL buffer sizes. */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RXBUFFER_BUFSIZE, &wRxBufferSize));
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_TXBUFFER_BUFSIZE, &wTxBufferSize));

    /* Find the biggest possible frame size before chaining is needed. */
    wMaxFrameSize = (wTxBufferSize < aI18092_FsTable[pDataParams->bFsl]) ? wTxBufferSize : aI18092_FsTable[pDataParams->bFsl];

    /* Transfer frame size should be one more than the FSL for 106 baud rate
     * as SOF is added by the protocol 
     */
    /* In Active mode the start byte would be handled by Hardware */
    if ((pDataParams->bDrt == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
    {
        wMaxFrameSize++;
    }

    /* ACK transmission in case of Rx-Chaining */
    if (((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING) ||
        ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING_BUFSIZE))
    {
        pDataParams->bStateNow = PHPAL_I18092MT_SW_STATE_ACK_PDU_TX | PHPAL_I18092MT_SW_STATE_CHAINING_BIT;
    }
    /* PDU transmission */
    else
    {
        /* Reset to default state if not in Tx-Mode */
        if ((pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_MASK) != PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_TX)
        {
            pDataParams->bStateNow = PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_TX;
        }
    }

    /* Reset Header length */
    bHeaderLength = 0;

    /* Reset BufferOverflow flag */
    
    /* ******************************** */
    /*       PDU TRANSMISSION LOOP      */
    /* ******************************** */
    do
    {
        /* Set exchange option */
        wHalExchangeOption = (uint16_t)(wOption & (uint16_t)~(uint16_t)PH_EXCHANGE_MODE_MASK);

        /* Init. number of bytes to send */
        wBytesToSend = wTxLength;

        switch (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_MASK)
        {
        case PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_TX:

            /* Forecast header length */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_GetPduHeaderLength(pDataParams, &bHeaderLength));

            /* Retrieve Number of preloaded bytes */
            if (wOption & PH_EXCHANGE_LEAVE_BUFFER_BIT)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
                    pDataParams->pHalDataParams,
                    PHHAL_HW_CONFIG_TXBUFFER_LENGTH,
                    &wTxBufferLen));

                /* Header is already included in TxBufferLen, so subtract it */
                wTxBufferLen = wTxBufferLen - (uint16_t)bHeaderLength;
            }
            else
            {
                /* Nothing has been buffered */
                wTxBufferLen = 0;
            }

            /* Check if we need to chain -> Data to be sent exceeds maximum frame size or HAL buffer */
            if (((uint16_t)bHeaderLength + wTxBufferLen + wBytesToSend) > wMaxFrameSize)
            {
                /* If this is the first frame */
                if (!(wOption & PH_EXCHANGE_LEAVE_BUFFER_BIT))
                {
                    /* TxBuffer needs space for at least the header */
                    if ((bHeaderLength + wTxBufferLen + 1) > wMaxFrameSize)
                    {
                        return PH_ADD_COMPCODE(PH_ERR_BUFFER_OVERFLOW, PH_COMP_PAL_I18092MT);
                    }

                    /* Limit bytes to send */
                    wBytesToSend = wMaxFrameSize - (uint16_t)(wTxBufferLen + bHeaderLength);
                }
                /* Else send the buffered frame and transmit this frame afterwards */
                else
                {
                    wBytesToSend = 0;
                }

                /* If there was no chaining indicate that this is the first chain */
                if (!(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT))
                {
                    /* Indicate first chaining */
                    pDataParams->bStateNow |= (PHPAL_I18092MT_SW_STATE_CHAINING_BIT | PHPAL_I18092MT_SW_STATE_CHAINING_FIRST_BIT);
                }

                /* Force sending. */
                wHalExchangeOption = PH_EXCHANGE_BUFFER_LAST;
            }

            /* Check if we are forced to chain */
            if ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_TXCHAINING)
            {
                /* If there was no chaining indicate that this is the first chain */
                if (!(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT))
                {
                    /* Indicate first chaining */
                    pDataParams->bStateNow |= (PHPAL_I18092MT_SW_STATE_CHAINING_BIT | PHPAL_I18092MT_SW_STATE_CHAINING_FIRST_BIT);
                }

                /* Force sending. */
                wHalExchangeOption = PH_EXCHANGE_BUFFER_LAST;
            }
            /* We are not forced to chain */
            else
            {
                /* Nor do we need to chain */
                if ((wBytesToSend == wTxLength) && !(wOption & PH_EXCHANGE_BUFFERED_BIT))
                {
                    /* But chaining has been performed -> this is the last chain */
                    if (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT)
                    {
                        pDataParams->bStateNow &= (uint8_t)~(uint8_t)PHPAL_I18092MT_SW_STATE_CHAINING_BIT;
                        pDataParams->bStateNow |= PHPAL_I18092MT_SW_STATE_CHAINING_LAST_BIT;
                    }
                }
            }

            /* Write PDU header */
            if (!(wOption & PH_EXCHANGE_LEAVE_BUFFER_BIT))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_WritePduHeader(
                    pDataParams,
                    PHPAL_I18092MT_SW_PFB_INFORMATION_PDU_MASK,
                    (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT) ? PH_ON : PH_OFF,
                    (uint8_t)(wBytesToSend + wTxBufferLen)));

                /* Preserve buffered data from now on */
                wHalExchangeOption |= PH_EXCHANGE_LEAVE_BUFFER_BIT;
            }
            /* PDU Header has already been written */
            else
            {
                /* Exchange is imminent, update length byte in HAL buffer */
                if (!(wHalExchangeOption & PH_EXCHANGE_BUFFERED_BIT))
                {
                    /* At a datarate of 106kbps an additional start byte needs to be added. ISO/IEC 18092:2004(E) section 12.1 */
                    /* In Active mode the start byte would be handled by Hardware */
                    if ((pDataParams->bDst == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
                    {
                        --wTxBufferLen;
                    }
                    PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_UpdatePduHeader(pDataParams, (uint8_t)(wBytesToSend + wTxBufferLen)));
                }
            }
            break;

        case PHPAL_I18092MT_SW_STATE_ACK_PDU_TX:

            /* Write PDU header to internal HAL buffer. Don't set the option bit (timeout in this case). */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_WritePduHeader(
                pDataParams,
                PHPAL_I18092MT_SW_PFB_ACK_PDU_MASK,
                PH_OFF,
                0));

            /* No additional data */
            wBytesToSend = 0;

            /* Force sending. */
            wHalExchangeOption = PH_EXCHANGE_BUFFER_LAST;
            break;

        default:
            return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_PAL_I18092MT);
        }

        if (!(pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT))
        {
           /* If no chaining, just do a transmit */
            status = phhalHw_Exchange(
                  pDataParams->pHalDataParams,
                  wHalExchangeOption,
                  pTxBuffer,
                  wBytesToSend,
                  NULL,
                  NULL
                  );
        }
        else
        {
            /* Transmit and receive data since we are doing chaining */
            status = phpalI18092mT_Sw_TransceivePdu(
                pDataParams,
                wHalExchangeOption,
                pTxBuffer,
                wBytesToSend,
                &pRawResponse,
                &wRawResponseLength,
                &pNormalizedResponse,
                &wNormalizedResponseLength,
                &pPayloadResponse,
                &wPayloadLength);
        }

        /* Complete chaining if buffer is full */
        if (((status & PH_ERR_MASK) == PH_ERR_BUFFER_OVERFLOW) &&
            ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING_BUFSIZE))
        {
            /* Indicate Buffer Overflow */
            
            /* Valid information PDU received, increment PNI. NFCForum-TS-DigitalProtocol-1.0 section 14.12.3.3. */
            pDataParams->bPni = PHPAL_I18092MT_SW_INCREMENT_PNI(pDataParams->bPni);
        }
        /* Else bail out on error */
        else
        {
            PH_CHECK_SUCCESS(status);
        }

        if (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_RETRANSMIT_BIT)
        {
            /* Retransmission requested. */
            pDataParams->bStateNow &= (uint8_t)~(uint8_t)PHPAL_I18092MT_SW_STATE_RETRANSMIT_BIT;
        }
        else
        {
            /* Advance input data buffer. */
            pTxBuffer += wBytesToSend;
            wTxLength = wTxLength - wBytesToSend;

            /* Clean-up state */
            pDataParams->bStateNow &= (uint8_t)~(uint8_t)(PHPAL_I18092MT_SW_STATE_CHAINING_FIRST_BIT | PHPAL_I18092MT_SW_STATE_CHAINING_LAST_BIT);

            /* TX is finished if no data is left to send */
            if (wTxLength == 0)
            {
                /* Buffered / TxChaining mode -> finished after sending */
                if ((wOption & PH_EXCHANGE_BUFFERED_BIT) ||
                    ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_TXCHAINING))
                {
                    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
                }

                /* Advance to RX state */
                pDataParams->bStateNow &= (uint8_t)~(uint8_t)PHPAL_I18092MT_SW_STATE_MASK;
                pDataParams->bStateNow |= PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_RX;

                /* Chained frame received, set first-chain bit */
                if (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT)
                {
                    pDataParams->bStateNow |= PHPAL_I18092MT_SW_STATE_CHAINING_FIRST_BIT;
                }
            }
            /* TX is not finished -> reset leave-buffer bit */
            else
            {
                wOption &= (uint16_t)~(uint16_t)PH_EXCHANGE_LEAVE_BUFFER_BIT;
            }
        }
    }
    /* Loop as long as the state does not transit to RX mode. */
    while ((pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_MASK) != PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_RX);

    /* PNI should be incremented after a successful send response by the target */
    pDataParams->bPni = PHPAL_I18092MT_SW_INCREMENT_PNI(pDataParams->bPni);

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_HandleChainedData(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t ** ppRxBuffer,
    uint16_t * pRxLength
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;

    /* Used to build block frames */
   
    uint16_t    PH_MEMLOC_REM wMaxFrameSize;
 
    uint8_t     PH_MEMLOC_REM bHeaderLength;

    /* Used for Exchange */
   
    uint8_t     PH_MEMLOC_REM aRxOverlapped[7];
    uint16_t    PH_MEMLOC_REM wRxOverlappedStartPos;
    uint16_t    PH_MEMLOC_REM wRxBufferSize;
    uint8_t     PH_MEMLOC_REM bBufferOverflow;
    uint8_t   * PH_MEMLOC_REM pRawResponse;
    uint16_t    PH_MEMLOC_REM wRawResponseLength;
    uint8_t   * PH_MEMLOC_REM pNormalizedResponse;
    uint16_t    PH_MEMLOC_REM wNormalizedResponseLength;
    uint8_t   * PH_MEMLOC_REM pPayloadResponse;
    uint16_t    PH_MEMLOC_REM wPayloadLength;
    uint8_t     PH_MEMLOC_REM bPrePni;
    uint16_t    PH_MEMLOC_REM wPreRawResponseLength;
    uint16_t    PH_MEMLOC_REM wPrePayloadLength;

    /* Retrieve HAL RX buffer size. */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RXBUFFER_BUFSIZE, &wRxBufferSize));

    /* Reset Header length */
    bHeaderLength = 0;

    /* Reset BufferOverflow flag */
    bBufferOverflow = 0;

    /* Reset Previous Payload length */
    wPrePayloadLength = 0;
    
    /* Reset wRxOverlappedStartPos */
    wRxOverlappedStartPos = 0;

    /* We are in receive state and also since this function is called, chaning must been already enabled */
    pDataParams->bStateNow = PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_RX | PHPAL_I18092MT_SW_STATE_CHAINING_BIT;
    /* Get the biggest possible frame size we'll receive. */
    wMaxFrameSize = aI18092_FsTable[pDataParams->bFsl];

    /* Forecast header length */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_GetPduHeaderLength(pDataParams, &bHeaderLength));

    wRawResponseLength = *pRxLength;
    pRawResponse = *ppRxBuffer;
    
    /* ******************************** */
    /*        PDU RECEPTION LOOP        */
    /* ******************************** */
    do
    {
        /* Only allow receive state at this point */
        if ((pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_MASK) != PHPAL_I18092MT_SW_STATE_INFORMATION_PDU_RX)
        {
            return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_PAL_I18092MT);
        }

        /* Retransmission requested. */
        if (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_RETRANSMIT_BIT)
        {
            pDataParams->bStateNow &= (uint8_t)~(uint8_t)PHPAL_I18092MT_SW_STATE_RETRANSMIT_BIT;
        }
        /* We're not done yet, chaining is in progress */
        else if (pDataParams->bStateNow & PHPAL_I18092MT_SW_STATE_CHAINING_BIT)
        {
            /* Skip overlapping / SUCCESS_CHAINING checks in case of BufferOverflow */
            if (!bBufferOverflow)
            {              
                /* Return with chaining status if the next chain may not fit into our buffer */
                if ((*pRxLength + wMaxFrameSize) > wRxBufferSize)
                {
                    /* Reset RxBuffer Start Position */
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RXBUFFER_STARTPOS, 0));
                    
                    return PH_ADD_COMPCODE(PH_ERR_SUCCESS_CHAINING, PH_COMP_PAL_I18092MT);
                }
                
                /* Calculate start-position for overlapping */
                wRxOverlappedStartPos = wRawResponseLength - (uint16_t)bHeaderLength;

                /* Backup overlapped bytes */
                memcpy(aRxOverlapped, &pRawResponse[wRxOverlappedStartPos], bHeaderLength);  /* PRQA S 3200 */

                /* Adjust RX buffer start position. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
                    pDataParams->pHalDataParams,
                    PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
                    wRxOverlappedStartPos));
            }

            /* Chaining information PDU received, send ACK. */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_WritePduHeader(
                pDataParams,
                PHPAL_I18092MT_SW_PFB_ACK_PDU_MASK,
                PH_OFF,
                0));

            if((pDataParams->dwTimerId != PH_OSAL_INVALID_TIMER_ID))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Stop(pDataParams->phOsal, pDataParams->dwTimerId));
                PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Reset(pDataParams->phOsal, pDataParams->dwTimerId));
            }

            pDataParams->bPni = PHPAL_I18092MT_SW_INCREMENT_PNI(pDataParams->bPni);

            bPrePni = pDataParams->bPni;
            wPreRawResponseLength = wRawResponseLength;

            /* Perform actual exchange */
            status = phpalI18092mT_Sw_TransceivePdu(
                pDataParams,
                PH_EXCHANGE_BUFFER_LAST,
                NULL,
                0,
                &pRawResponse,
                &wRawResponseLength,
                &pNormalizedResponse,
                &wNormalizedResponseLength,
                &pPayloadResponse,
                &wPayloadLength);

            /* Complete chaining if buffer is full */
            if ((status & PH_ERR_MASK) == PH_ERR_BUFFER_OVERFLOW)
            {
                /* Reset wRxStartPos */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
                    pDataParams->pHalDataParams,
                    PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
                    0));

                /* Indicate Buffer Overflow */
                bBufferOverflow = 1;
            }
            /* Next chunk of PDU has been received */
            else
            {
                /* Bail out on error */
                if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
                {
                    /* Reset output length */
                    *pRxLength = 0;

                    /* Reset RxBuffer Start Position */
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RXBUFFER_STARTPOS, 0));

                    return status;
                }

                if (!bBufferOverflow)
                {
                    /* Restore overlapped bytes */
                    memcpy(&pRawResponse[wRxOverlappedStartPos], aRxOverlapped, bHeaderLength);  /* PRQA S 3200 */ /* PRQA S 3354 */

                    /* If after ATN, same payload received before ATN is received again, them overwrite the previous */
                    if(pDataParams->bPni == --bPrePni)
                    {
                        wRawResponseLength = wPreRawResponseLength;
                        *pRxLength = *pRxLength - wPrePayloadLength;
                        memcpy(&pRawResponse[wRawResponseLength - wPrePayloadLength], pPayloadResponse, wPayloadLength);    /* PRQA S 3200 */
                    }

                    /* Increment length */
                    *pRxLength = *pRxLength + wPayloadLength;

                    /* Backup previous length */
                    wPrePayloadLength = wPayloadLength;
                }
            }
        }
        /* No chaining -> reception finished */
        else
        {
            /* Reset RxBuffer Start Position */
            if (wRxOverlappedStartPos != 0)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RXBUFFER_STARTPOS, 0));
            }

            /* Return data */
            if (!bBufferOverflow)
            {
                pDataParams->bStateNow = PHPAL_I18092MT_SW_STATE_FINISHED;
            }
            /* do not return any data in case of Buffer Overflow */
            else
            {
                *pRxLength = 0;
                return PH_ADD_COMPCODE(PH_ERR_BUFFER_OVERFLOW, PH_COMP_PAL_I18092MT);
            }
        }
    }
    while (pDataParams->bStateNow != PHPAL_I18092MT_SW_STATE_FINISHED);

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_DslRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pDslReq,
    uint8_t wDslReqLength
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM *pFrame;
    uint16_t    PH_MEMLOC_REM wFrameLength;
    uint8_t     PH_MEMLOC_REM *pPayload;
    uint16_t    PH_MEMLOC_REM wPayloadLength;
    uint8_t     PH_MEMLOC_REM aCmd[1+3];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t     PH_MEMLOC_REM bDid;

     statusTmp = phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
            pDataParams,
            pDslReq,
            wDslReqLength,
            PHPAL_I18092MT_SW_CMD_DSL_REQ,
            &pFrame,
            &wFrameLength,
            &pPayload,
            &wPayloadLength
            );

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

    /* Ignore the length byte */
    ++pFrame;
    --wFrameLength;

    if(pDataParams->bDid == 0)
    {
        if (wFrameLength != PHPAL_I18092MT_SW_DSL_REQ_LENGTH)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }
    else
    {
        if (wFrameLength != (PHPAL_I18092MT_SW_DSL_REQ_LENGTH + 1))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }

    bDid = pFrame[PHPAL_I18092MT_SW_DSL_REQ_DID_POS];

    if (pDataParams->bDid != bDid && pDataParams->bDid != 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
    }

    /* Make NfcID invalid */
    pDataParams->bNfcIdValid = PH_OFF;

    if(pDataParams->bDid > 0)
    {
        aCmd[bCmdLen++] = pDataParams->bDid;
    }

    /* Write frame header. */
    PH_CHECK_SUCCESS_FCT(
        statusTmp,
        phpalI18092mT_Sw_WriteFrameHeader(
            pDataParams,
            PH_EXCHANGE_BUFFER_FIRST,
            PHPAL_I18092MT_SW_CMD_DSL_RES,
            bCmdLen));

    status = phhalHw_Exchange(pDataParams->pHalDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        aCmd,
        (uint16_t)bCmdLen,
        NULL,
        NULL);

    if((PH_ERR_MASK & status) == PH_ERR_SUCCESS)
    {
        pDataParams->bTargetState = PHPAL_I18092MT_STATE_NONE;
    }

    /* SLEEP_AF state (Respond only to ALL_REQ ("52"), not to SENS_REQ ("26")), so set MFHalted bit */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_MFHALTED, PH_ON));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_PslRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pPslReq,
    uint16_t  wPslReqLength
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM *pFrame;
    uint16_t    PH_MEMLOC_REM wFrameLength;
    uint8_t     PH_MEMLOC_REM *pPayload;
    uint16_t    PH_MEMLOC_REM wPayloadLength;
    uint8_t     PH_MEMLOC_REM aCmd[1+3];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t     PH_MEMLOC_REM bBrs;
    uint8_t     PH_MEMLOC_REM bDst;
    uint8_t     PH_MEMLOC_REM bDrt;
    uint8_t     PH_MEMLOC_REM bFsl;

    bCmdLen = 0;

    statusTmp = phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
            pDataParams,
            pPslReq,
            wPslReqLength,
            PHPAL_I18092MT_SW_CMD_PSL_REQ,
            &pFrame,
            &wFrameLength,
            &pPayload,
            &wPayloadLength
            );

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

    /* Ignore the length byte */
    ++pFrame;
    --wFrameLength;

    /* Check the frame length of the PSL command */
    if (wFrameLength != (PHPAL_I18092MT_SW_PSL_REQ_LENGTH))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
    }

    /*
      Format of the PSL_REQ command, 12.5.3.1
      +--------+--------+--------+--------+--------+
      | CMD 0  | CMD 1  | Byte 0 | Byte 1 | Byte 2 |
      +--------+--------+--------+--------+--------+
      |  (D4)  |  (04)  |   DID  |   BRS  |  FSL   |
      +--------+--------+--------+--------+--------+

    */

    if(pDataParams->bDid != 0)
    {
        if(pDataParams->bDid != pFrame[PHPAL_I18092MT_SW_PSL_REQ_DID_POS])
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }

    pDataParams->bDid = pFrame[PHPAL_I18092MT_SW_PSL_REQ_DID_POS];
    bBrs = pFrame[PHPAL_I18092MT_SW_PSL_REQ_BRS_POS];
    bDst = (bBrs >> 3)&0x07;
    bDrt = bBrs & 0x07;
    bFsl = pFrame[PHPAL_I18092MT_SW_PSL_REQ_FSL_POS];

    /* Parameter check */
    if ((bDst > PHPAL_I18092MT_DATARATE_424) ||
        (bDrt > PHPAL_I18092MT_DATARATE_424) ||
        (bFsl > PHPAL_I18092MT_FRAMESIZE_254))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }

    /* Form the response and send back the response */

    /*
       Format of the PSL_REQ command, 12.5.3.2
       +--------+--------+--------+
       | CMD 0  | CMD 1  | Byte 0 |
       +--------+--------+--------+
       |  (D5)  |  (05)  |   DID  |
       +--------+--------+--------+

     */
    aCmd[bCmdLen++] = pDataParams->bDid;
    /* Write frame header. */
    PH_CHECK_SUCCESS_FCT(
        statusTmp,
        phpalI18092mT_Sw_WriteFrameHeader(
            pDataParams,
            PH_EXCHANGE_BUFFER_FIRST,
            PHPAL_I18092MT_SW_CMD_PSL_RES,
            bCmdLen));

    status = phhalHw_Exchange(pDataParams->pHalDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        aCmd,
        (uint16_t)bCmdLen,
        NULL,
        NULL);

    /* Apply Tx-datarate to HAL. */
   PH_CHECK_SUCCESS_FCT(statusTmp,
                        phhalHw_SetConfig(
                            pDataParams->pHalDataParams,
                            PHHAL_HW_CONFIG_TXDATARATE,
                            aI18092_DrTable[bDst]));

   /* Apply Rx-datarate to HAL. */
   PH_CHECK_SUCCESS_FCT(statusTmp,
                        phhalHw_SetConfig(
                            pDataParams->pHalDataParams,
                            PHHAL_HW_CONFIG_RXDATARATE,
                            aI18092_DrTable[bDrt]));

   /* All OK, store the Dsi, Dri and FSL value */
   pDataParams->bDst = bDst;
   pDataParams->bDrt = bDrt;
   pDataParams->bFsl = bFsl;

    return status;
}


phStatus_t phpalI18092mT_Sw_RlsRes(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint8_t * pRlsReq,
    uint16_t wRlsReqLength
    )
{
    phStatus_t PH_MEMLOC_REM status;
    phStatus_t PH_MEMLOC_REM statusTmp;
    uint8_t    PH_MEMLOC_REM *pFrame;
    uint16_t   PH_MEMLOC_REM wFrameLength;
    uint8_t    PH_MEMLOC_REM *pPayload;
    uint16_t   PH_MEMLOC_REM wPayloadLength;
    uint8_t    PH_MEMLOC_REM aCmd[1+3];
    uint8_t    PH_MEMLOC_REM bCmdLen = 0;
    uint8_t    PH_MEMLOC_REM bDid;

    statusTmp = phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
            pDataParams,
            pRlsReq,
            wRlsReqLength,
            PHPAL_I18092MT_SW_CMD_RLS_REQ,
            &pFrame,
            &wFrameLength,
            &pPayload,
            &wPayloadLength
            );

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

    /* Ignore the length byte */
    ++pFrame;
    --wFrameLength;

    if(pDataParams->bDid == 0)
    {
        if (wFrameLength != PHPAL_I18092MT_SW_RLS_REQ_LENGTH)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }
    else
    {
        if (wFrameLength != (PHPAL_I18092MT_SW_RLS_REQ_LENGTH + 1))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
        }
    }

    bDid = pFrame[PHPAL_I18092MT_SW_RLS_REQ_DID_POS];
    
    if (pDataParams->bDid != bDid && pDataParams->bDid != 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
    }
    
    /*
       The frame for RL_RES, 12.7.2.2
       +--------+--------+--------+
       | CMD 0  | CMD 1  | Byte 0 |
       +--------+--------+--------+
       |  (D5)  |  (0B)  |   DID  |
       +--------+--------+--------+

    */

    /* Make NfcID invalid */
    pDataParams->bNfcIdValid = PH_OFF;

    if(pDataParams->bDid > 0)
    {
        aCmd[bCmdLen++] = pDataParams->bDid;
    }

    /* Write frame header. */
    PH_CHECK_SUCCESS_FCT(
        statusTmp,
        phpalI18092mT_Sw_WriteFrameHeader(
            pDataParams,
            PH_EXCHANGE_BUFFER_FIRST,
            PHPAL_I18092MT_SW_CMD_RLS_RES,
            bCmdLen));
    status = phhalHw_Exchange(pDataParams->pHalDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        aCmd,
        (uint16_t)bCmdLen,
        NULL,
        NULL);
    if((PH_ERR_MASK & status) == PH_ERR_SUCCESS)
    {
        pDataParams->bTargetState = PHPAL_I18092MT_STATE_NONE;
    }

    return status;

}
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
    )
{
   /* Copy the NFC ID3 */
    memmove(pDataParams->aNfcid3t, pNfcid3t, PHPAL_I18092MT_NFCID3_LENGTH);  /* PRQA S 3200 */

    if ((bLrt & (uint8_t)~0x03) != 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }
    else
    {
        pDataParams->bLrt = bLrt;
    }

    if ((bNadEnable != PH_ON) && (bNadEnable != PH_OFF))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }
    else
    {
        pDataParams->bNadEnabled = bNadEnable;
    }

    if ((bBst & (uint8_t)~0x0F) != 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }
    else
    {
        pDataParams->bBst = bBst;
    }

    if ((bBrt & (uint8_t)~0x0F) != 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }
    else
    {
        pDataParams->bBrt = bBrt;
    }

    if (bTo > 0x0F)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }
    else
    {
        pDataParams->bTo = bTo;
    }

    /* TODO: check the maximum length of the GT, check the right value */
    if (bGtLength > 0xef)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
    }

    pDataParams->bGtLength = bGtLength;
    if (bGtLength == 0)
    {
        pDataParams->pGt = NULL;
    }
    else
    {
        pDataParams->pGt = pGt;
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_Listen(
    phpalI18092mT_Sw_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t ** ppRxBuffer,
    uint16_t * pRxLength,
    void * context
    )
{
    phStatus_t PH_MEMLOC_REM status;
    phStatus_t PH_MEMLOC_REM statusTmp;
    uint8_t *  PH_MEMLOC_REM pRxBuf;
    uint16_t   PH_MEMLOC_REM wRxLen;
    uint16_t   PH_MEMLOC_REM wFraming;
    uint8_t    * PH_MEMLOC_REM pAutoCollResBuf;
    uint16_t   PH_MEMLOC_REM pAutoCollResLen;
    uint8_t    PH_MEMLOC_REM bOffset = 0;
    uint8_t    PH_MEMLOC_REM bPfb;
    uint8_t    PH_MEMLOC_REM *pPayload;
    uint16_t   PH_MEMLOC_REM wPayloadLength;
    uint16_t   PH_MEMLOC_REM wHeaderLen;
    uint8_t    PH_MEMLOC_REM bLoop = 0;
    uint16_t   PH_MEMLOC_REM wRxSpeed = 0;

    if (pDataParams->bTargetState == PHPAL_I18092MT_STATE_NONE)
    {
        do
        {
            status = phhalHw_Listen(
                pDataParams->pHalDataParams,
                &pAutoCollResBuf,
                &pAutoCollResLen
                );
            bLoop++;
        }
        while (((status & PH_ERR_MASK) != PH_ERR_SUCCESS) && (pDataParams->bAutocollRetryCount >= bLoop));
        PH_CHECK_SUCCESS(status);

        /* Get the framing mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RX_FRAME_MODE,
            &wFraming));
        /* After the auto coll, check the framing mode that was detected so as to
           know the baud rate */
        if (wFraming == PHHAL_HW_I14443A_106_FRAMING)
        {
            pDataParams->bDst = PHPAL_I18092MT_DATARATE_106;
            pDataParams->bDrt = PHPAL_I18092MT_DATARATE_106;
        }
        else if (wFraming == PHHAL_HW_ACTIVE_MODE_FRAMING)
        {
             pDataParams->bActiveMode = PH_ON;
             /* Get the RxSpeed */
             PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
                 pDataParams->pHalDataParams,
                 PHHAL_HW_CONFIG_RXSPEED,
                 &wRxSpeed));
             if(wRxSpeed == PHHAL_HW_I14443A_106_FRAMING)
             {
                 pDataParams->bDst = PHPAL_I18092MT_DATARATE_106;
                 pDataParams->bDrt = PHPAL_I18092MT_DATARATE_106;
             }
             else
             {
                 pDataParams->bDst = PHPAL_I18092MT_DATARATE_212_424;
                 pDataParams->bDrt = PHPAL_I18092MT_DATARATE_212_424;
             }
        }
        else
        {
            pDataParams->bDst = PHPAL_I18092MT_DATARATE_212_424;
            pDataParams->bDrt = PHPAL_I18092MT_DATARATE_212_424;
        }
        /* Once we are out of the autocoll, we have received the next command
           from the initiator. */
        /* In response to ATR_REQ, send ATR_RES */
        status = phpalI18092mT_Sw_AtrRes(
                 pDataParams,
                 pAutoCollResBuf,
                 pAutoCollResLen                
                 );
        
        if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Receive(
                             pDataParams->pHalDataParams,
                             PHHAL_HW_OPTION_RXTX_TIMER_START | PHHAL_HW_OPTION_START_TRANSCIEVE,
                             &pAutoCollResBuf,
                             &pAutoCollResLen));
        
            /* Get the framing mode */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
                    pDataParams->pHalDataParams,
                    PHHAL_HW_CONFIG_RX_FRAME_MODE,
                    &wFraming));
            /* After the auto coll, check the framing mode that was detected so as to
               know the baud rate */
            if (wFraming == PHHAL_HW_I14443A_106_FRAMING)
            {
                pDataParams->bDst = PHPAL_I18092MT_DATARATE_106;
                pDataParams->bDrt = PHPAL_I18092MT_DATARATE_106;
            }
            else
            {
                pDataParams->bDst = PHPAL_I18092MT_DATARATE_212_424;
                pDataParams->bDrt = PHPAL_I18092MT_DATARATE_212_424;
            }

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_AtrRes(
                                pDataParams,
                                pAutoCollResBuf,
                                pAutoCollResLen
                                ));
        }

        if ((pDataParams->bDrt == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
        {
            /* Remove the SOF incase of datarate 106*/
            *ppRxBuffer =  pAutoCollResBuf + 1;
            *pRxLength = pAutoCollResLen - 1;
        }
        else
        {
            *ppRxBuffer =  pAutoCollResBuf;
            *pRxLength = pAutoCollResLen;
        }
    
        pDataParams->bTargetState = PHPAL_I18092MT_STATE_DEP_ACTIVATED;
    }
    else if(pDataParams->bTargetState == PHPAL_I18092MT_STATE_DEP_ACTIVATED)
    {
        if (wOption == PHHAL_HW_OPTION_START_CMD_ONLY)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_StartRecv(pDataParams, context));
        }
        else if (wOption == PHHAL_HW_OPTION_FIFO_READ)
        {
            /* Start the first receive action */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Receive(
                 pDataParams->pHalDataParams,
                 PHHAL_HW_OPTION_FIFO_READ,
                 &pRxBuf,
                 &wRxLen));

            /* Populate the Drt here */
            if ((pDataParams->bDrt == PHPAL_I18092MT_DATARATE_106) && (pDataParams->bActiveMode == PH_OFF))
            {
               bOffset = 3;
            }
            else
            {
               bOffset = 2;
            }

            if(wRxLen <= bOffset)
            {
                pDataParams->pSet_Interrupt(PH_OFF);
                /* Restart transcieve */
                PH_CHECK_SUCCESS_FCT(status, phhalHw_Receive(
                     pDataParams->pHalDataParams,
                     PHHAL_HW_OPTION_START_CMD_ONLY | PHHAL_HW_OPTION_START_TRANSCIEVE,
                     NULL,
                     NULL));

                return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_PAL_I18092MT);
            }

            switch(pRxBuf[bOffset])
            {
                case PHPAL_I18092MT_SW_CMD_DSL_REQ:
                    pDataParams->bCmdtype = PHPAL_I18092MT_SW_CMD_DSL_REQ;
                    *ppRxBuffer = pRxBuf;
                    *pRxLength = wRxLen;

                    break;

                case PHPAL_I18092MT_SW_CMD_RLS_REQ:
                    pDataParams->bCmdtype = PHPAL_I18092MT_SW_CMD_RLS_REQ;
                    *ppRxBuffer = pRxBuf;
                    *pRxLength = wRxLen;

                    break;

                case PHPAL_I18092MT_SW_CMD_PSL_REQ:
                    pDataParams->bCmdtype = PHPAL_I18092MT_SW_CMD_PSL_REQ;
                    *ppRxBuffer = pRxBuf;
                    *pRxLength = wRxLen;
                    break;

                case PHPAL_I18092MT_SW_CMD_DEP_REQ:
                    pDataParams->bCmdtype = PHPAL_I18092MT_SW_CMD_DEP_REQ;
                    /* No chaining enabled */
                    statusTmp = phpalI18092mT_Sw_ValidateAndNormalizeResponseFrame(
                            pDataParams,
                            pRxBuf,
                            wRxLen,
                            PHPAL_I18092MT_SW_CMD_DEP_REQ,
                            &pPayload,
                            &wPayloadLength,
                            ppRxBuffer,
                            pRxLength);

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
                    wHeaderLen = wRxLen - *pRxLength;
                    bPfb = pRxBuf[bOffset+1];

                    if((!PHPAL_I18092MT_SW_IS_ATTENTION(bPfb)) && (!PHPAL_I18092MT_SW_IS_TIMEOUT(bPfb)))
                    {
                        pDataParams->bPni = bPfb & 0x03;
                    }

                    if(!pDataParams->bRtoxDisabled)
                    {
                        if(pDataParams->dwTimerId == PH_OSAL_INVALID_TIMER_ID)
                        {
                            PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Create(pDataParams->phOsal,  &pDataParams->dwTimerId));
                        }

                        if(pDataParams->dwTimerId != PH_OSAL_INVALID_TIMER_ID)
                        {
                            PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Reset(pDataParams->phOsal,  pDataParams->dwTimerId));
                            PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Start(
                                    pDataParams->phOsal,
                                    pDataParams->dwTimerId,
                                    PHPAL_I18092MT_SW_CALCULATE_RWT_US(pDataParams->bTo),
                                    PH_OSAL_TIMER_UNIT_US,
                                    &phpalI18092mT_Sw_RtoxCallBck,
                                    (void *)pDataParams));
                        }
                    }

                    if (PHPAL_I18092MT_SW_IS_ATTENTION(bPfb))
                    {
                        /* Stop timer */
                        if((pDataParams->dwTimerId != PH_OSAL_INVALID_TIMER_ID))
                        {
                            PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Stop(pDataParams->phOsal, pDataParams->dwTimerId));
                            PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_Timer_Reset(pDataParams->phOsal, pDataParams->dwTimerId));
                        }

                        /* Send the ATN response */
                        PH_CHECK_SUCCESS_FCT(statusTmp, phpalI18092mT_Sw_WritePduHeader(
                                pDataParams,
                                PHPAL_I18092MT_SW_PFB_SUPERVISORY_PDU_MASK,
                                PH_OFF,
                                0));
                        /* If no chaining, just do a transmit */
                        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Exchange(
                                pDataParams->pHalDataParams,
                                PH_EXCHANGE_LEAVE_BUFFER_BIT,
                                NULL,
                                0,
                                NULL,
                                NULL
                                ));
                        pDataParams->bPni--;
                        pDataParams->bCmdtype = PHPAL_I18092MT_SW_CMD_ATN;
                        /* Restart receive incase of ATN response */
                        pDataParams->pSet_Interrupt(PH_ON);
                        break;
                    }

                    if (!PHPAL_I18092MT_SW_IS_CHAINING(bPfb))
                    {
                        break;
                    }
                    else
                    {
                        PH_CHECK_SUCCESS_FCT(
                              statusTmp,
                              phpalI18092mT_Sw_HandleChainedData(
                                  pDataParams,
                                  &pRxBuf,
                                  &wRxLen
                                  ));
                        *pRxLength =  wRxLen - wHeaderLen;
                        break;
                    }
                    
                default:
                    return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_PAL_I18092MT);
            }
        }
        else
        {
            /* Error state */
            return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_PAL_I18092MT);
        }
    }
    else
    {
        /* Error state */
        return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_PAL_I18092MT);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_SetConfig(
                                       phpalI18092mT_Sw_DataParams_t * pDataParams,
                                       uint16_t wConfig,
                                       uint16_t wValue
                                       )
{
    switch (wConfig)
    {

    case PHPAL_I18092MT_CONFIG_RTOX:
        if((wValue != PH_ON) && (wValue != PH_OFF))
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
        }
        pDataParams->bRtoxDisabled = (uint8_t)(wValue ^ 0x0001);
        break;
        
    case PHPAL_I18092MT_CONFIG_RTOX_VALUE:
        if((wValue < PHPAL_I18092MT_SW_MIN_RTOX_VALUE) || (wValue > PHPAL_I18092MT_SW_MAX_RTOX_VALUE))
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
        }
        pDataParams->bRtox = (uint8_t)(wValue);
        break;

    case PHPAL_I18092MT_CONFIG_AUTOCOLL_RETRY_COUNT:
        if(wValue > PHPAL_I18092MT_SW_MAX_AUTOCOLL_RETRY_COUNT)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_I18092MT);
        }
        pDataParams->bAutocollRetryCount = (uint8_t)(wValue);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_PAL_I18092MT);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}

phStatus_t phpalI18092mT_Sw_GetConfig(
                                       phpalI18092mT_Sw_DataParams_t * pDataParams,
                                       uint16_t wConfig,
                                       uint16_t * pValue
                                       )
{
    switch (wConfig)
    {

    case PHPAL_I18092MT_CONFIG_RTOX:
        *pValue = (uint16_t)(pDataParams->bRtoxDisabled ^ 0x01);
        break;

    case PHPAL_I18092MT_CONFIG_AUTOCOLL_RETRY_COUNT:
        *pValue = (uint16_t)(pDataParams->bAutocollRetryCount);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_PAL_I18092MT);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_I18092MT);
}
#endif /* NXPBUILD__PHPAL_I18092MT_SW */
