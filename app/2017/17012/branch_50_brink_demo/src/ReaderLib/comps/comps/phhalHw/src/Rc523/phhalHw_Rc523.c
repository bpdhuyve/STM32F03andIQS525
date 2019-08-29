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
* $Revision: 486 $
* $Date: 2014-01-31 09:38:54 +0530 (Fri, 31 Jan 2014) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#include <ph_Status.h>
#include <phbalReg.h>
#include <phhalHw.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_KEYSTORE
#include <phKeyStore.h>
#endif

//#ifdef NXPBUILD__PHHAL_HW_RC523

#include "phhalHw_Rc523.h"
#include "phhalHw_Rc523_Int.h"
#include <phhalHw_Rc523_Reg.h>
#include "phhalHw_Rc523_Config.h"

/* Default shadow for ISO14443-3A Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_I14443a[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_ON},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_OFF},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_OFF},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0008},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_MODINDEX,              0x3FU},
    {PHHAL_HW_CONFIG_ASK100,                PH_ON}
};

/* Default shadow for ISO14443-3B Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_I14443b[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_ON},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0008},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_MODINDEX,              PHHAL_HW_RC523_MODINDEX_I14443B},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for FeliCa Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_Felica[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0003},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_212},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_212},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_MODINDEX,              PHHAL_HW_RC523_MODINDEX_FELICA},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for FeliCa Mode at 424 baud rate */
static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_Felica_424[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0003},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_424},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_424},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_MODINDEX,              PHHAL_HW_RC523_MODINDEX_FELICA},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for ISO18092 Passive Initiator Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_I18092mPI[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_ON},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0003},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_MODINDEX,              PHHAL_HW_RC523_MODINDEX_FELICA},
    {PHHAL_HW_CONFIG_ASK100,                PH_ON}
};

static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_I18092mPI_212[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0003},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_212},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_212},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_MODINDEX,              PHHAL_HW_RC523_MODINDEX_FELICA},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_I18092mPI_424[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0003},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_424},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_424},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_MODINDEX,              PHHAL_HW_RC523_MODINDEX_FELICA},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for ISO18092 Passive Target Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wRc523_DefaultShadow_I18092mPT[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_ON},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXDEAFBITS,            0x0008},
    {PHHAL_HW_CONFIG_TXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_RXDATARATE,            PHHAL_HW_RF_DATARATE_106},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,      PHHAL_HW_RC523_DEFAULT_TIMEOUT},
    {PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS,      0x0000},
    {PHHAL_HW_CONFIG_ASK100,                PH_ON}
};

phStatus_t phhalHw_Rc523_Init(
                              phhalHw_Rc523_DataParams_t * pDataParams,
                              uint16_t wSizeOfDataParams,
                              void * pBalDataParams,
                              void * pKeyStoreDataParams,
                              uint8_t * pTxBuffer,
                              uint16_t wTxBufSize,
                              uint8_t * pRxBuffer,
                              uint16_t wRxBufSize
                              )
{
    if (sizeof(phhalHw_Rc523_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }
    if (wTxBufSize == 0 || wRxBufSize == 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pBalDataParams);
    PH_ASSERT_NULL (pTxBuffer);
    PH_ASSERT_NULL (pRxBuffer);

    /* We need to reserve 1 byte at the beginning of the buffers for SPI operation */
    ++pTxBuffer;
    --wTxBufSize;
    ++pRxBuffer;
    --wRxBufSize;

    /* Init. private data */
    pDataParams->wId                    = PH_COMP_HAL | PHHAL_HW_RC523_ID;
    pDataParams->pBalDataParams         = pBalDataParams;
    pDataParams->pKeyStoreDataParams    = pKeyStoreDataParams;
    pDataParams->pTxBuffer              = pTxBuffer;
    pDataParams->wTxBufSize             = wTxBufSize;
    pDataParams->wTxBufLen              = 0;
    pDataParams->pRxBuffer              = pRxBuffer;
    pDataParams->wRxBufSize             = wRxBufSize;
    pDataParams->wRxBufLen              = 0;
    pDataParams->wRxBufStartPos         = 0;
    pDataParams->bCardType              = PHHAL_HW_CARDTYPE_ISO14443A;
    pDataParams->wTimingMode            = PHHAL_HW_TIMING_MODE_OFF;
    pDataParams->bTimeoutUnit           = PHHAL_HW_TIME_MICROSECONDS;
    pDataParams->dwTimingUs             = 0;
    pDataParams->wFieldOffTime          = PHHAL_HW_FIELD_OFF_DEFAULT;
    pDataParams->wFieldRecoveryTime     = PHHAL_HW_FIELD_RECOVERY_DEFAULT;
    pDataParams->wMaxPrecachedBytes     = PHHAL_HW_RC523_PRECACHED_BYTES;
    pDataParams->wAdditionalInfo        = 0;
    pDataParams->bBalConnectionType     = 0;
    pDataParams->bRfResetAfterTo        = PH_OFF;
    pDataParams->bRxMultiple            = PH_OFF;
    pDataParams->bActiveMode            = PH_OFF;
    pDataParams->bAutocollTimer         = PH_ON;

    return phbalReg_SetConfig(pDataParams->pBalDataParams, PHBAL_REG_CONFIG_HAL_HW_TYPE, PHBAL_REG_HAL_HW_RC523);
}

phStatus_t phhalHw_Rc523_Exchange(
                                  phhalHw_Rc523_DataParams_t * pDataParams,
                                  uint16_t wOption,
                                  uint8_t * pTxBuffer,
                                  uint16_t wTxLength,
                                  uint8_t ** ppRxBuffer,
                                  uint16_t * pRxLength
                                  )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t *   PH_MEMLOC_REM pTmpBuffer;
    uint16_t    PH_MEMLOC_REM wTmpBufferLen;
    uint16_t    PH_MEMLOC_REM wTmpBufferSize;

    /* Check options */
    if (wOption & (uint16_t)~(uint16_t)(PH_EXCHANGE_BUFFERED_BIT | PH_EXCHANGE_LEAVE_BUFFER_BIT))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* clear internal buffer if requested */
    if (!(wOption & PH_EXCHANGE_LEAVE_BUFFER_BIT))
    {
        pDataParams->wTxBufLen = 0;
    }

    /* set the receive length */
    if (pRxLength != NULL)
    {
        *pRxLength = 0;
    }

    /* Fill the global TxBuffer */
    /* Note: We always need to buffer for SPI, else the input buffer would get overwritten! */
    if ((wOption & PH_EXCHANGE_BUFFERED_BIT) ||
        (pDataParams->bBalConnectionType == PHHAL_HW_BAL_CONNECTION_SPI) ||
        (pDataParams->bBalConnectionType == PHHAL_HW_BAL_CONNECTION_I2C))
    {
        /*
        In target mode it's not allowed to have same buffers for transmission and reception
        since the data to transmit would get overwritten by received data.
        */
        if ((pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT) &&
            (pDataParams->pTxBuffer == pDataParams->pRxBuffer))
        {
            return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_HAL);
        }

        /* retrieve transmit buffer */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetTxBuffer(pDataParams, PH_ON, &pTmpBuffer, &wTmpBufferLen, &wTmpBufferSize));

        if (wTxLength != 0)
        {
            /* TxBuffer overflow check */
            if (wTxLength > (wTmpBufferSize - wTmpBufferLen))
            {
                pDataParams->wTxBufLen = 0;
                return PH_ADD_COMPCODE(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
            }

            /* copy data */
            memcpy(&pTmpBuffer[wTmpBufferLen], pTxBuffer, wTxLength);  /* PRQA S 3200 */
            pDataParams->wTxBufLen = pDataParams->wTxBufLen + wTxLength;
        }

        /* Buffer operation -> finished */
        if (wOption & PH_EXCHANGE_BUFFERED_BIT)
        {
            return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
        }
        /* Else reset the input length for SPI and continue */
        else
        {
            wTxLength = 0;
        }
    }

    if (pDataParams->bCardType != PHHAL_HW_CARDTYPE_I18092MPT)
    {
        /* Terminate a possibly running command */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_IDLE));
    }

    /* Flush FiFo */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FlushFifo(pDataParams));

    /* clear all IRQ0 flags */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(
        pDataParams,
        PHHAL_HW_RC523_REG_COMMIRQ,
        (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_SET));

    /* clear all IRQ1 flags */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(
        pDataParams,
        PHHAL_HW_RC523_REG_DIVIRQ,
        (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_SET));

    /* perform transmission */
    status = phhalHw_Rc523_ExchangeTransmit(pDataParams, PHHAL_HW_RC523_CMD_TRANSCEIVE, pTxBuffer, wTxLength);

    /* perform receive operation */
    if (((status & PH_ERR_MASK) == PH_ERR_SUCCESS) &&
        (((pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT) && (NULL != ppRxBuffer)) ||
        (pDataParams->bCardType != PHHAL_HW_CARDTYPE_I18092MPT)))
    {
        status = phhalHw_Rc523_ExchangeReceive(pDataParams, wOption, ppRxBuffer, pRxLength);
    }

    return status;
}

phStatus_t phhalHw_Rc523_WriteRegister(
                                       phhalHw_Rc523_DataParams_t * pDataParams,
                                       uint8_t bAddress,
                                       uint8_t bValue
                                       )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bDataBuffer[2];
    uint16_t    PH_MEMLOC_REM wBytesRead;
    uint8_t     PH_MEMLOC_REM bNumExpBytes;

    /* SPI protocol */

        /* shift address and clear RD/NWR bit to indicate write operation */
        bAddress = (uint8_t)(bAddress << 1);
        bNumExpBytes = 2;

    /* Write the address and data */
    bDataBuffer[0] = bAddress;
    bDataBuffer[1] = bValue;
    PH_CHECK_SUCCESS_FCT(statusTmp, phbalReg_Exchange(
        pDataParams->pBalDataParams,
        PH_EXCHANGE_DEFAULT,
        bDataBuffer,
        2,
        bNumExpBytes,
        bDataBuffer,
        &wBytesRead));

    /* Check number of received bytes */
    if (wBytesRead != bNumExpBytes)
    {
        return PH_ADD_COMPCODE(PH_ERR_INTERFACE_ERROR, PH_COMP_HAL);
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_ReadRegister(
                                      phhalHw_Rc523_DataParams_t * pDataParams,
                                      uint8_t bAddress,
                                      uint8_t * pValue
                                      )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bTxBuffer[2];
    uint16_t    PH_MEMLOC_REM wTxLength;
    uint16_t    PH_MEMLOC_REM wBytesRead;
    uint8_t        PH_MEMLOC_REM bNumExpBytes;


        /* set RD/NWR bit to indicate read operation */
        bTxBuffer[0] = (uint8_t)(bAddress << 1) | 0x80U;
        bTxBuffer[1] = 0x00;
        wTxLength = 2;
        bNumExpBytes = 2;


    /* Write the address and retrieve the register content */
    PH_CHECK_SUCCESS_FCT(statusTmp, phbalReg_Exchange(
        pDataParams->pBalDataParams,
        PH_EXCHANGE_DEFAULT,
        bTxBuffer,
        wTxLength,
        bNumExpBytes,
        bTxBuffer,
        &wBytesRead));

    /* Check number of received bytes */
    if (wBytesRead != bNumExpBytes)
    {
        return PH_ADD_COMPCODE(PH_ERR_INTERFACE_ERROR, PH_COMP_HAL);
    }

    /* in case of SPI 2 bytes are recieved from a read */
    if (pDataParams->bBalConnectionType == PHHAL_HW_BAL_CONNECTION_SPI)
    {
        *pValue = bTxBuffer[1];
    }
    else
    {
        *pValue = bTxBuffer[0];
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_ApplyProtocolSettings(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t bCardType
    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bValue;
    uint16_t    PH_MEMLOC_COUNT wIndex;
    uint16_t *  PH_MEMLOC_REM pShadowDefault;
    uint16_t    PH_MEMLOC_REM wShadowCount;
    uint8_t     PH_MEMLOC_REM bUseDefaultShadow;
    uint16_t    PH_MEMLOC_REM wConfig;

    /* Store new card type */
    if (bCardType != PHHAL_HW_CARDTYPE_CURRENT)
    {
        pDataParams->bCardType = bCardType;
        pDataParams->bTimeoutUnit = PHHAL_HW_TIME_MICROSECONDS;
        bUseDefaultShadow = 1;

        /* Initialize config shadow */
        memset(pDataParams->wCfgShadow, 0x00, PHHAL_HW_RC523_SHADOW_COUNT);  /* PRQA S 3200 */
    }
    else
    {
        bUseDefaultShadow = 0;
    }

    /* configure reader IC for current card */
    switch (pDataParams->bCardType)
    {
        /* configure hardware for ISO14443A */
    case PHHAL_HW_CARDTYPE_ISO14443A:

        /* configure Tx path for ISO14443A */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_MIFARE));
        /* configure Rx path for ISO14443A */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_MIFARE));
        /* configurate default TxModWidth */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODWIDTH, 0x26));
        /* configure the RxThreshold for ISO14443A */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_I14443A));
        /* Set initiator mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR));

        /* Use 14443a default shadow */
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I14443a;
        wShadowCount = sizeof(wRc523_DefaultShadow_I14443a) / (sizeof(uint16_t) * 2);
        break;

        /* configure hardware for ISO14443B */
    case PHHAL_HW_CARDTYPE_ISO14443B:

        /* configure Tx path for ISO14443B */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_TYPEB));
        /* configure Rx path for ISO14443B */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_TYPEB));
        /* configure the RxThreshold for ISO14443B */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_I14443B));
        /* Set TypeB register to default */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TYPEB, 0x00));
        /* Set initiator mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR));

        /* Use 14443b shadow */
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I14443b;
        wShadowCount = sizeof(wRc523_DefaultShadow_I14443b) / (sizeof(uint16_t) * 2);
        break;

        /* configure hardware for Felica */

    case PHHAL_HW_CARDTYPE_FELICA:

        //configure Tx path for Felica
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_FELICA));
        // configure Rx path for Felica
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_FELICA));
        // configure the RxThreshold for Felica
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_FELICA));
        //Set initiator mode
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR));

        // Use Felica shadow
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_Felica;
        wShadowCount = sizeof(wRc523_DefaultShadow_Felica) / (sizeof(uint16_t) * 2);
        break;



        /* configure hardware for I18092mPI at 106 */
    case PHHAL_HW_CARDTYPE_I18092MPI:

        /* configure Tx path for I18092mPI */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_MIFARE));
        /* configure Rx path for I18092mPI */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_MIFARE));
        /* configurate default TxModWidth */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODWIDTH, 0x26));
        /* configure the RxThreshold for I18092mPI  */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_I14443A));
        /* Set initiator mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR));

        /* Use I18092mPI shadow */
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I18092mPI;
        wShadowCount = sizeof(wRc523_DefaultShadow_I18092mPI) / (sizeof(uint16_t) * 2);
        break;

        /* configure hardware for I18092mPI at 212 */
    case PHHAL_HW_CARDTYPE_I18092MPI_212:

        /* configure Tx path for I18092mPI */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_FELICA));
        /* configure Rx path for I18092mPI */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_FELICA));
        /* configurate default TxModWidth */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODWIDTH, 0x26));
        /* configure the RxThreshold for I18092mPI  */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_FELICA));
        /* Set initiator mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR));

        /* Use I18092mPI 212 shadow */
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I18092mPI_212;
        wShadowCount = sizeof(wRc523_DefaultShadow_I18092mPI_212) / (sizeof(uint16_t) * 2);
        break;

        /* configure hardware for I18092mPI at 424 */
    case PHHAL_HW_CARDTYPE_I18092MPI_424:

        /* configure Tx path for I18092mPI */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_FELICA));
        /* configure Rx path for I18092mPI */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_FELICA));
        /* configurate default TxModWidth */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODWIDTH, 0x26));
        /* configure the RxThreshold for I18092mPI  */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_FELICA));
        /* Set initiator mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR));

        /* Use I18092mPI 424 shadow */
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I18092mPI_424;
        wShadowCount = sizeof(wRc523_DefaultShadow_I18092mPI_424) / (sizeof(uint16_t) * 2);
        break;

        /* configure hardware for I18092mPT */
    case PHHAL_HW_CARDTYPE_I18092MPT:

        /* configure Tx path for TypeA */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_MIFARE));
        /* configure Rx path for TypeA */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_MIFARE));
        /* configurate default TxModWidth */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODWIDTH, 0x26));
        /* configure the RxThreshold for TypeA  */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_I14443A));
        /* Set target mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, 0x00));
        /* Switch off the field */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, 0x00));
        /* Use TypeA shadow by default */
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I18092mPT;
        wShadowCount = sizeof(wRc523_DefaultShadow_I18092mPT) / (sizeof(uint16_t) * 2);
        break;

    case PHHAL_HW_CARDTYPE_FELICA_424:

        /* configure Tx path for Felica with baud rate 424 */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, PHHAL_HW_RC523_BIT_FELICA));
        /* configure Rx path for Felica with 424 baud rate */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, PHHAL_HW_RC523_BIT_FELICA ));
        /* configure the RxThreshold for Felica  */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_FELICA));
        /* Set initiator mode */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR));

        /* Use Felica shadow */
        pShadowDefault = (uint16_t*)wRc523_DefaultShadow_Felica_424;
        wShadowCount = sizeof(wRc523_DefaultShadow_Felica_424) / (sizeof(uint16_t) * 2);
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* */
    /* Generic initialization */
    /* */

    /* configure the gain factor to 23dB for Target and 38dB for Initiator*/

    if(pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RFCFG, 0x59));
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RFCFG, 0x59));
    }

    /* configure TX2 inverted of but do not change the field status */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, &bValue));
    bValue  = PHHAL_HW_RC523_BIT_TX2RFEN | PHHAL_HW_RC523_BIT_TX1RFEN;
    bValue |= PHHAL_HW_RC523_BIT_INVTX2ON | PHHAL_HW_RC523_BIT_CHECKRF;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, bValue));

    /* configure the RxSel Register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXSEL, 0x80));

    /* configure general settings */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODE, 0x00));

    /* configure the conductance if no modulation is active */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_GSN, 0xFF));
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CWGSP, 0x3F));

    /* configure the conductance for LoadModulation */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_GSNOFF, PHHAL_HW_RC523_MODINDEX_TARGET));

    /* reset bitframing register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_BITFRAMING, 0x00));

    if(pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT)
    {
        /* configure the timer */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TMODE, 0x00));
    }
    else
    {
        /* configure the timer */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TMODE, PHHAL_HW_RC523_BIT_TAUTO));
    }

    /* configure the water level */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_WATERLEVEL, PHHAL_HW_RC523_FIFOSIZE - 8));

    /* Apply shadowed registers */
    for (wIndex = 0; wIndex < wShadowCount; ++wIndex)
    {
        /* Get wConfig */
        wConfig = pShadowDefault[wIndex << 1];

        /* Apply only one the correct timeout unit */
        if (!(((wConfig == PHHAL_HW_CONFIG_TIMEOUT_VALUE_US) &&
            (pDataParams->bTimeoutUnit != PHHAL_HW_TIME_MICROSECONDS)) ||
            ((wConfig == PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS) &&
            (pDataParams->bTimeoutUnit != PHHAL_HW_TIME_MILLISECONDS))))
        {
            /* Default shadow: */
            if (bUseDefaultShadow)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetConfig(pDataParams, wConfig, pShadowDefault[(wIndex << 1) + 1]));
            }
            /* Current shadow: */
            else
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetConfig(pDataParams, wConfig, pDataParams->wCfgShadow[wConfig]));
            }
        }
    }

    /* MIFARE Crypto1 state is disabled by default */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, PHHAL_HW_CONFIG_DISABLE_MF_CRYPTO1, PH_ON));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_SetConfig(
                                   phhalHw_Rc523_DataParams_t * pDataParams,
                                   uint16_t wConfig,
                                   uint16_t wValue
                                   )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bRegister;
    uint8_t     PH_MEMLOC_REM bValue;
    uint16_t    PH_MEMLOC_REM wIndex;
    uint8_t     PH_MEMLOC_REM bModWidthReg;
    uint8_t *   PH_MEMLOC_REM pBuffer;
    uint16_t    PH_MEMLOC_REM wBufferSize;
    uint16_t    PH_MEMLOC_REM wConfigShadow;
    uint16_t *  PH_MEMLOC_REM pShadowDefault = NULL;
    uint16_t    PH_MEMLOC_REM wBufferLen = 0;
    uint8_t     PH_MEMLOC_REM bSpeed;

    switch (wConfig)
    {
    case PHHAL_HW_CONFIG_PARITY:

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_MFRX, &bRegister));

        if (wValue == PH_OFF)
        {
            bValue = bRegister | PHHAL_HW_RC523_BIT_PARITYDISABLE;
        }
        else
        {
            bValue = bRegister & (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_PARITYDISABLE;
        }

        /* Only perform the operation, if the new value is different */
        if (bValue != bRegister)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MFRX, bValue));
        }

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_TXCRC:

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, &bRegister));

        if (wValue == PH_OFF)
        {
            bValue = bRegister & (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_CRCEN;
        }
        else
        {
            bValue = bRegister | PHHAL_HW_RC523_BIT_CRCEN;
        }

        /* Only perform the operation, if the new value is different */
        if (bValue != bRegister)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, bValue));
        }

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_RXCRC:

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, &bRegister));

        if (wValue == PH_OFF)
        {
            bValue = bRegister & (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_CRCEN;
        }
        else
        {
            bValue = bRegister | PHHAL_HW_RC523_BIT_CRCEN;
        }

        /* Only perform the operation, if the new value is different */
        if (bValue != bRegister)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, bValue));
        }

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_TXLASTBITS:

        /* check parameter */
        if (wValue > PHHAL_HW_RC523_MASK_TXBITS)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_BITFRAMING, &bRegister));
        bRegister &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_TXBITS;
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_BITFRAMING, bRegister | (uint8_t)wValue));

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_RXALIGN:

        /* check parameter */
        if (wValue > 7)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* adjust parameter */
        wValue = (uint16_t)(wValue << 4);

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_BITFRAMING, &bRegister));
        bRegister &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_RXALIGN;
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_BITFRAMING, bRegister |  (uint8_t)wValue));

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_RXDEAFBITS:

        /* check parameter */
        if (wValue > PHHAL_HW_RC523_MASK_RXWAIT)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* configure the Rx Deaf Time in bits */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXSEL, &bRegister));
        bRegister &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_RXWAIT;
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXSEL, bRegister |  (uint8_t)wValue));

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_CLEARBITSAFTERCOLL:

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_TXDATARATE:

        /* Retrieve TxMode register */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, &bRegister));
        bValue = bRegister & (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_SPEED;

        /* update current data rate */
        switch (wValue)
        {
        case PHHAL_HW_RF_DATARATE_106:
            bValue |= PHHAL_HW_RC523_BIT_106KBPS;
            bModWidthReg = 0x26;
            break;
        case PHHAL_HW_RF_DATARATE_212:
            bValue |= PHHAL_HW_RC523_BIT_212KBPS;
            bModWidthReg = 0x15;
            break;
        case PHHAL_HW_RF_DATARATE_424:
            bValue |= PHHAL_HW_RC523_BIT_424KBPS;
            bModWidthReg = 0x0A;
            break;
        case PHHAL_HW_RF_DATARATE_848:
            bValue |= PHHAL_HW_RC523_BIT_848KBPS;
            bModWidthReg = 0x05;
            break;
        default:
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* protocol specific handling */
        switch (pDataParams->bCardType)
        {
            /* Felica */
        case PHHAL_HW_CARDTYPE_FELICA:
            /* Only allow 212kbit/s and 424kbit/s */
            if ((wValue != PHHAL_HW_RF_DATARATE_212) && (wValue != PHHAL_HW_RF_DATARATE_424))
            {
                return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            break;
            /* I18000p3m3 */
        case PHHAL_HW_CARDTYPE_I18092MPI:
        case PHHAL_HW_CARDTYPE_I18092MPI_212:
        case PHHAL_HW_CARDTYPE_I18092MPI_424:
        case PHHAL_HW_CARDTYPE_I18092MPT:
            /* TypeA mode: 106kbit/s */
            if (wValue == PHHAL_HW_RF_DATARATE_106)
            {
                /* If the Card mode changes, we need to update the configuration */
                if ((bRegister & PHHAL_HW_RC523_MASK_SPEED) != PHHAL_HW_RF_DATARATE_106)
                {
                    /* Update RxThreshold*/
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_I14443A));

                    /* Update Framing */
                    bValue &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_FRAMING;
                    bValue |= PHHAL_HW_RC523_BIT_MIFARE;

                    /* Set shadow */
                    pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I14443a;
                    wBufferLen = sizeof(wRc523_DefaultShadow_I14443a) / (sizeof(uint16_t) * 2);
                }
            }
            /* Felica Mode: 212kbit/s or 424kbit/s */
            else if (wValue < PHHAL_HW_RF_DATARATE_848)
            {
                /* If the Card mode changes, we need to update the registers */
                if ((bRegister & PHHAL_HW_RC523_MASK_SPEED) == PHHAL_HW_RF_DATARATE_106)
                {
                    /* Update RxThreshold*/
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_FELICA));

                    /* Update Framing */
                    bValue &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_FRAMING;
                    bValue |= PHHAL_HW_RC523_BIT_FELICA;

                    /* Set shadow */
                    pShadowDefault = (uint16_t*)wRc523_DefaultShadow_Felica;
                    wBufferLen = sizeof(wRc523_DefaultShadow_Felica) / (sizeof(uint16_t) * 2);
                }
            }
            /* 848kbit/s is unsupported */
            else
            {
                return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            /* Also Update registers from default shadow */
            if (pShadowDefault != NULL)
            {
                for (wIndex = 0; wIndex < wBufferLen; ++wIndex)
                {
                    /* Get wConfigShadow */
                    wConfigShadow = pShadowDefault[wIndex << 1];

                    /* Apply configuration */
                    if ((wConfigShadow != PHHAL_HW_CONFIG_TXDATARATE) &&
                        (wConfigShadow != PHHAL_HW_CONFIG_RXDATARATE) &&
                        (wConfigShadow != PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS))
                    {
                        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, wConfigShadow, pShadowDefault[(wIndex << 1) + 1]));
                    }
                }
            }
            break;
            /* Others */
        default:
            break;
        }

        /* Only apply the settings if the speed has changed */
        if (bValue != bRegister)
        {
            /* Apply data rate */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXMODE, bValue));

            /* Configure the modulation width for ISO 14443-3A */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODWIDTH, bModWidthReg));
        }

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;

        /* Felica only support symmetric baudrates */
        if (pDataParams->bCardType == PHHAL_HW_CARDTYPE_FELICA)
        {
            /* Adjust RxDatarate if it does not match */
            if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_RXDATARATE] != wValue)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, PHHAL_HW_CONFIG_RXDATARATE, wValue));
            }
        }
        break;

    case PHHAL_HW_CONFIG_RXDATARATE:

        /* Retrieve RxMode register */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, &bRegister));
        bValue = bRegister & (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_SPEED;

        /* update current data rate */
        switch (wValue)
        {
        case PHHAL_HW_RF_DATARATE_106:
            bValue |= PHHAL_HW_RC523_BIT_106KBPS;
            break;
        case PHHAL_HW_RF_DATARATE_212:
            bValue |= PHHAL_HW_RC523_BIT_212KBPS;
            break;
        case PHHAL_HW_RF_DATARATE_424:
            bValue |= PHHAL_HW_RC523_BIT_424KBPS;
            break;
        case PHHAL_HW_RF_DATARATE_848:
            bValue |= PHHAL_HW_RC523_BIT_848KBPS;
            break;
        default:
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* protocol specific handling */
        switch (pDataParams->bCardType)
        {
            /* Felica */
        /*
        case PHHAL_HW_CARDTYPE_FELICA:
            // Only allow 212kbit/s and 424kbit/s
            if ((wValue != PHHAL_HW_RF_DATARATE_212) && (wValue != PHHAL_HW_RF_DATARATE_424))
            {
                return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            break;
            */
            /* I18000p3m3 */
        /*
        case PHHAL_HW_CARDTYPE_I18092MPI:
        case PHHAL_HW_CARDTYPE_I18092MPI_212:
        case PHHAL_HW_CARDTYPE_I18092MPI_424:
        case PHHAL_HW_CARDTYPE_I18092MPT:
            // TypeA mode: 106kbit/s
            if (wValue == PHHAL_HW_RF_DATARATE_106)
            {
                // If the Card mode changes, we need to update the configuration
                if ((bRegister & PHHAL_HW_RC523_MASK_SPEED) != PHHAL_HW_RF_DATARATE_106)
                {
                    // Update RxThreshold
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_I14443A));

                    // Update Framing
                    bValue &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_FRAMING;
                    bValue |= PHHAL_HW_RC523_BIT_MIFARE;

                    // Set shadow
                    pShadowDefault = (uint16_t*)wRc523_DefaultShadow_I14443a;
                    wBufferLen = sizeof(wRc523_DefaultShadow_I14443a) / (sizeof(uint16_t) * 2);
                }
            }
            */
            /* Felica Mode: 212kbit/s or 424kbit/s */
            /*
            else if (wValue < PHHAL_HW_RF_DATARATE_848)
            {
                // If the Card mode changes, we need to update the registers
                if ((bRegister & PHHAL_HW_RC523_MASK_SPEED) == PHHAL_HW_RF_DATARATE_106)
                {
                    // Update RxThreshold
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXTHRESHOLD, PHHAL_HW_RC523_RXTHRESHOLD_FELICA));

                    // Update Framing
                    bValue &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_FRAMING;
                    bValue |= PHHAL_HW_RC523_BIT_FELICA;

                    // Set shadow
                    pShadowDefault = (uint16_t*)wRc523_DefaultShadow_Felica;
                    wBufferLen = sizeof(wRc523_DefaultShadow_Felica) / (sizeof(uint16_t) * 2);
                }
            }

            // 848kbit/s is unsupported
            else
            {
                return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            */

            /* Also Update registers from default shadow */
            if (pShadowDefault != NULL)
            {
                for (wIndex = 0; wIndex < wBufferLen; ++wIndex)
                {
                    /* Get wConfigShadow */
                    wConfigShadow = pShadowDefault[wIndex << 1];

                    /* Apply configuration */
                    if ((wConfigShadow != PHHAL_HW_CONFIG_TXDATARATE) &&
                        (wConfigShadow != PHHAL_HW_CONFIG_RXDATARATE) &&
                        (wConfigShadow != PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS))
                    {
                        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, wConfigShadow, pShadowDefault[(wIndex << 1) + 1]));
                    }
                }
            }
            break;
            /* Others */
        default:
            break;
        }

        /* Apply data rate */
        if (bValue != bRegister)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, bValue));
        }

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;

        /* Felica only support symmetric baudrates */
        /*
        if (pDataParams->bCardType == PHHAL_HW_CARDTYPE_FELICA)
        {
            // Adjust TxDatarate if it does not match
            if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXDATARATE] != wValue)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, PHHAL_HW_CONFIG_TXDATARATE, wValue));
            }
        }
        */
        break;

    case PHHAL_HW_CONFIG_MODINDEX:

        if (wValue & (uint8_t)~(uint8_t)PHHAL_HW_RC523_MASK_MODGSP)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* Set ModGsP register */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_MODGSP, (uint8_t)wValue));

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_ASK100:

        /* switch off 100% ASK */
        if (wValue == PH_OFF)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXASK, 0x00));
        }
        /* switch on 100% ASK */
        else
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXASK, PHHAL_HW_RC523_BIT_FORCE100ASK));
        }

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_TIMEOUT_VALUE_US:
    case PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS:

        /* Calculate values for Microsecond values */
        if (wConfig == PHHAL_HW_CONFIG_TIMEOUT_VALUE_US)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetFdt(pDataParams, PHHAL_HW_TIME_MICROSECONDS, wValue));
            pDataParams->bTimeoutUnit = PHHAL_HW_TIME_MICROSECONDS;
        }
        /* Calculate values for Millisecond values */
        else
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetFdt(pDataParams, PHHAL_HW_TIME_MILLISECONDS, wValue));
            pDataParams->bTimeoutUnit = PHHAL_HW_TIME_MILLISECONDS;
        }

        /* Write config data into shadow */
        pDataParams->wCfgShadow[wConfig] = wValue;
        break;

    case PHHAL_HW_CONFIG_TIMING_MODE:

        /* Check supported option bits */
        switch (wValue & PHHAL_HW_TIMING_MODE_OPTION_MASK)
        {
        case PHHAL_HW_TIMING_MODE_OPTION_DEFAULT:
        case PHHAL_HW_TIMING_MODE_OPTION_AUTOCLEAR:
            break;
        default:
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* Check supported timing modes */
        switch (wValue & (uint16_t)~(uint16_t)PHHAL_HW_TIMING_MODE_OPTION_MASK)
        {
        case PHHAL_HW_TIMING_MODE_OFF:
        case PHHAL_HW_TIMING_MODE_FDT:
            pDataParams->dwTimingUs = 0;
            pDataParams->wTimingMode = wValue;
            break;
        case PHHAL_HW_TIMING_MODE_COMM:
            return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
        default:
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        break;

    case PHHAL_HW_CONFIG_FIELD_OFF_TIME:

        /* Parameter Check */
        if (wValue == 0)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* Store config data */
        pDataParams->wFieldOffTime = wValue;
        break;

    case PHHAL_HW_CONFIG_FIELD_RECOVERY_TIME:

        /* Store config data */
        pDataParams->wFieldRecoveryTime = wValue;
        break;

    case PHHAL_HW_CONFIG_DISABLE_MF_CRYPTO1:

        /* Disable crypto, enabling is not supported */
        if (wValue != PH_OFF)
        {
            /* Retrieve Status2 register */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_STATUS2, &bRegister));

            /* Clear Crypto1On bit */
            bValue = bRegister & (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_CRYPTO1ON;

            /* Only perform the operation, if the new value is different */
            if (bValue != bRegister)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_STATUS2, bValue));
            }
        }
        break;

    case PHHAL_HW_CONFIG_ADDITIONAL_INFO:

        /* Modify additional info parameter */
        pDataParams->wAdditionalInfo = wValue;
        break;

    case PHHAL_HW_CONFIG_RXBUFFER_STARTPOS:

        /* Boundary check */
        if (wValue >= pDataParams->wRxBufSize)
        {
            return PH_ADD_COMPCODE(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
        }

        /* Set start position */
        pDataParams->wRxBufStartPos = wValue;
        break;

    case PHHAL_HW_CONFIG_TXBUFFER_LENGTH:

        /* Retrieve TxBuffer parameters */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetTxBuffer(pDataParams, PH_ON, &pBuffer, &wBufferLen, &wBufferSize));

        /* Check parameter, must not exceed TxBufferSize */
        if (wValue > wBufferSize)
        {
            return PH_ADD_COMPCODE(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
        }

        /* set buffer length */
        pDataParams->wTxBufLen = wValue;
        break;

    case PHHAL_HW_CONFIG_TXBUFFER:

        /* Retrieve TxBuffer parameters */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetTxBuffer(pDataParams, PH_ON, &pBuffer, &wBufferLen, &wBufferSize));

        /* Check parameter, must not exceed TxBufferSize */
        if (pDataParams->wAdditionalInfo >= wBufferSize)
        {
            return PH_ADD_COMPCODE(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
        }

        /* Modify TxBuffer byte */
        pBuffer[pDataParams->wAdditionalInfo] = (uint8_t)wValue;
        break;

    case PHHAL_HW_CONFIG_MAX_PRECACHED_BYTES:

        if (wValue > PHHAL_HW_RC523_FIFOSIZE)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        /* Store parameter*/
        pDataParams->wMaxPrecachedBytes = wValue;
        break;

    case PHHAL_HW_CONFIG_BAL_CONNECTION:

        /* parameter check */
        if (wValue > PHHAL_HW_BAL_CONNECTION_I2C)
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        pDataParams->bBalConnectionType = (uint8_t)wValue;
        break;

        /*
    case PHHAL_HW_CONFIG_SERIAL_BITRATE:

        switch (wValue)
        {
        case PHHAL_HW_RS232_BITRATE_9600:
            bValue = PHHAL_HW_RC523_SERIALSPEED_9600;
            break;
        case PHHAL_HW_RS232_BITRATE_19200:
            bValue = PHHAL_HW_RC523_SERIALSPEED_19200;
            break;
        case PHHAL_HW_RS232_BITRATE_38400:
            bValue = PHHAL_HW_RC523_SERIALSPEED_38400;
            break;
        case PHHAL_HW_RS232_BITRATE_57600:
            bValue = PHHAL_HW_RC523_SERIALSPEED_57600;
            break;
        case PHHAL_HW_RS232_BITRATE_115200:
            bValue = PHHAL_HW_RC523_SERIALSPEED_115200;
            break;
        case PHHAL_HW_RS232_BITRATE_230400:
            bValue = PHHAL_HW_RC523_SERIALSPEED_230400;
            break;
        case PHHAL_HW_RS232_BITRATE_460800:
            bValue = PHHAL_HW_RC523_SERIALSPEED_460800;
            break;
        default:
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        // Set the register value
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_SERIALSPEED, bValue));
        break;
        */

    case PHHAL_HW_CONFIG_RFRESET_ON_TIMEOUT:

        if (wValue == PH_OFF)
        {
            pDataParams->bRfResetAfterTo = PH_OFF;
        }
        else
        {
            pDataParams->bRfResetAfterTo = PH_ON;
        }        
        break;

    case PHHAL_HW_CONFIG_SETMINFDT:

        phhalHw_Rc523_SetMinFDT(pDataParams, wValue);
        break;

    case PHHAL_HW_CONFIG_RXMULTIPLE:

        if (wValue == PH_ON)
        {
            pDataParams->bRxMultiple = PH_ON;
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_ReadRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_RXMODE,
                &bRegister));
            bRegister |= PHHAL_HW_RC523_BIT_RXMULTIPLE;

            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Rc523_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_RXMODE,
                bRegister
                ));
        }
        else
        {
            pDataParams->bRxMultiple = PH_OFF;
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_ReadRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_RXMODE,
                &bRegister));
            bRegister &= ~PHHAL_HW_RC523_BIT_RXMULTIPLE;

            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_RXMODE,
                bRegister
                ));
        }
        break;

    case PHHAL_HW_CONFIG_ACTIVEMODE:
        if (wValue == PH_ON)
        {
            pDataParams->bActiveMode = PH_ON;
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, &bRegister));

            /* Get the Rx data rate*/
            bSpeed = (bRegister & PHHAL_HW_RC523_MASK_SPEED);
            if (bSpeed == PHHAL_HW_RF_DATARATE_106)
            {
                /* Its 106 */
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXBITPHASE,
                    0x0F));

                /* Set Force100ASK */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXASK, &bRegister));

                bRegister |= 0x40;
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXASK,
                    bRegister));
            }
            else
            {
                /* Clear Force100ASK */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXASK, &bRegister));

                bRegister &= (~(0x40));
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXASK,
                    bRegister));

            }
            /* Enable Detect Sync Bit */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_ReadRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_MODE,
                &bRegister));
            bRegister |= PHHAL_HW_RC523_BIT_DETECT_SYNC;
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_MODE,
                bRegister));

            if ((pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPI) ||
                (pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPI_212) ||
                (pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPI_424))
            {
                /* Enable active Tx mode */
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_ReadRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXMODE,
                    &bRegister));

                bRegister &= 0xFC;
                bRegister |= PHHAL_HW_RC523_BIT_ACTIVE;
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXMODE,
                    bRegister));

                /* Enable active Rx mode */
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_ReadRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_RXMODE,
                    &bRegister));

                bRegister &= 0xFC;
                bRegister |= PHHAL_HW_RC523_BIT_ACTIVE;
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_RXMODE,
                    bRegister));

                /* CAoN has to be cleared */
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_ReadRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXASK,
                    &bRegister));
                bRegister &= ~(PHHAL_HW_RC523_BIT_CA_ON);

                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXASK,
                    bRegister));
                /* Enable AutoRFoff, Tx1/2AutoRFON*/
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_ReadRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXASK,
                    &bRegister));

                bRegister |= (PHHAL_HW_RC523_BIT_TX2RFAutoEN | PHHAL_HW_RC523_BIT_TX1RFAutoEN
                    | PHHAL_HW_RC523_BIT_AUTORF_OFF);
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXASK,
                    bRegister));
            }
            else
            {
                /* Enable active Tx mode and speed */
                bRegister = (PHHAL_HW_RC523_BIT_ACTIVE | bSpeed | PHHAL_HW_RC523_BIT_CRCEN);
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_TXMODE,
                    bRegister));
                /* Enable active Rx mode and speed */
                bRegister = (PHHAL_HW_RC523_BIT_ACTIVE | bSpeed | PHHAL_HW_RC523_BIT_CRCEN);
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_RXMODE,
                    bRegister));
               
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_ReadRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_MODE,
                    &bRegister));
                
                bRegister |= PHHAL_HW_RC523_BIT_TXWAITRF;
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_WriteRegister(
                    pDataParams,
                    PHHAL_HW_RC523_REG_MODE,
                    bRegister));
            }
        }
        else
        {
            pDataParams->bActiveMode = PH_OFF;
            /* Enable active Tx mode */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_ReadRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_TXMODE,
                &bRegister));
            bRegister &= ~PHHAL_HW_RC523_BIT_ACTIVE;
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_TXMODE,
                bRegister));

            /* Disable active Rx mode */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_ReadRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_RXMODE,
                &bRegister));
            bRegister &= ~PHHAL_HW_RC523_BIT_ACTIVE;
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_RXMODE,
                bRegister));

            /* Disable RX wait on RF detection */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_ReadRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_MODE,
                &bRegister));
            bRegister &= ~PHHAL_HW_RC523_BIT_TXWAITRF;
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_MODE,
                bRegister));
        }   
        break;

    case PHHAL_HW_CONFIG_AUTOCOLL_TIMER:
        if((wValue != PH_ON) && (wValue != PH_OFF))
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }
        pDataParams->bAutocollTimer = (uint8_t)wValue;
        break;

    case PHHAL_HW_CONFIG_MFHALTED:
        if((wValue != PH_ON) && (wValue != PH_OFF))
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }

        PH_CHECK_SUCCESS_FCT(statusTmp,
            phhalHw_ReadRegister(
            pDataParams,
            PHHAL_HW_RC523_REG_MFTX,
            &bRegister));

        if(wValue == PH_ON)
        {
            bRegister |= PHHAL_HW_RC523_BIT_MFHALTED;
        }
        else
        {
            bRegister &= ~PHHAL_HW_RC523_BIT_MFHALTED;
        }

        PH_CHECK_SUCCESS_FCT(statusTmp,
            phhalHw_WriteRegister(
            pDataParams,
            PHHAL_HW_RC523_REG_MFTX,
            bRegister));
        break;

    default:

        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_GetConfig(
                                   phhalHw_Rc523_DataParams_t * pDataParams,
                                   uint16_t wConfig,
                                   uint16_t * pValue
                                   )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bRegister;
    uint8_t *   PH_MEMLOC_REM pBuffer;
    uint16_t    PH_MEMLOC_REM wBufferLen;
    uint16_t    PH_MEMLOC_REM wBufferSize;

    switch (wConfig)
    {
    case PHHAL_HW_CONFIG_PARITY:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_TXCRC:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_RXCRC:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_TXLASTBITS:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_ADDITIONAL_INFO:
    case PHHAL_HW_CONFIG_RXLASTBITS:

        *pValue = pDataParams->wAdditionalInfo;
        break;

    case PHHAL_HW_CONFIG_RXDEAFBITS:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_CLEARBITSAFTERCOLL:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_TXDATARATE:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_RXDATARATE:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_MODINDEX:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_ASK100:

        /* Read config from shadow */
        *pValue = pDataParams->wCfgShadow[wConfig];
        break;

    case PHHAL_HW_CONFIG_TIMEOUT_VALUE_US:

        if (pDataParams->bTimeoutUnit == PHHAL_HW_TIME_MICROSECONDS)
        {
            *pValue = pDataParams->wCfgShadow[wConfig];
        }
        else
        {
            if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS] > (0xFFFF / 1000))
            {
                return PH_ADD_COMPCODE(PH_ERR_PARAMETER_OVERFLOW, PH_COMP_HAL);
            }
            *pValue = pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS] * 1000;
        }
        break;

    case PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS:

        if (pDataParams->bTimeoutUnit == PHHAL_HW_TIME_MILLISECONDS)
        {
            *pValue = pDataParams->wCfgShadow[wConfig];
        }
        else
        {
            *pValue = pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_US] / 1000;
        }
        break;

    case PHHAL_HW_CONFIG_TIMING_MODE:

        *pValue = pDataParams->wTimingMode;
        break;

    case PHHAL_HW_CONFIG_TIMING_US:

        if (pDataParams->dwTimingUs > 0xFFFF)
        {
            return PH_ADD_COMPCODE(PH_ERR_PARAMETER_OVERFLOW, PH_COMP_HAL);
        }

        *pValue = (uint16_t)pDataParams->dwTimingUs;
        pDataParams->dwTimingUs = 0;
        break;

    case PHHAL_HW_CONFIG_TIMING_MS:

        if (pDataParams->dwTimingUs > (0xFFFF * 1000))
        {
            pDataParams->dwTimingUs = 0;
            return PH_ADD_COMPCODE(PH_ERR_PARAMETER_OVERFLOW, PH_COMP_HAL);
        }

        *pValue = (uint16_t)(pDataParams->dwTimingUs / 1000);
        pDataParams->dwTimingUs = 0;
        break;

    case PHHAL_HW_CONFIG_FIELD_OFF_TIME:

        *pValue = pDataParams->wFieldOffTime;
        break;

    case PHHAL_HW_CONFIG_FIELD_RECOVERY_TIME:

        *pValue = pDataParams->wFieldRecoveryTime;
        break;

    case PHHAL_HW_CONFIG_DISABLE_MF_CRYPTO1:

        /* Retrieve Status register */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_STATUS2, &bRegister));

        /* Check Crypto1On bit */
        if (bRegister & PHHAL_HW_RC523_BIT_CRYPTO1ON)
        {
            *pValue = PH_OFF; /* OFF in this case means "Crypto1 not disabled --> enabled" */
        }
        else
        {
            *pValue = PH_ON; /* ON in this case means "Crypto1 is disabled" */
        }
        break;

    case PHHAL_HW_CONFIG_RXBUFFER_STARTPOS:

        /* Return parameter */
        *pValue = pDataParams->wRxBufStartPos;
        break;

    case PHHAL_HW_CONFIG_RXBUFFER_BUFSIZE:

        /* Retrieve RxBuffer parameters */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetRxBuffer(pDataParams, PH_ON, &pBuffer, &wBufferLen, &wBufferSize));

        /* Return parameter */
        *pValue = wBufferSize;
        break;

    case PHHAL_HW_CONFIG_TXBUFFER_BUFSIZE:

        /* Retrieve TxBuffer parameters */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetTxBuffer(pDataParams, PH_ON, &pBuffer, &wBufferLen, &wBufferSize));

        /* Return parameter */
        *pValue = wBufferSize;
        break;

    case PHHAL_HW_CONFIG_TXBUFFER_LENGTH:

        /* Retrieve TxBuffer parameters */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetTxBuffer(pDataParams, PH_ON, &pBuffer, &wBufferLen, &wBufferSize));

        /* Return parameter */
        *pValue = wBufferLen;
        break;

    case PHHAL_HW_CONFIG_TXBUFFER:

        /* Retrieve TxBuffer parameters */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetTxBuffer(pDataParams, PH_ON, &pBuffer, &wBufferLen, &wBufferSize));

        /* Check additional info parameter */
        if (pDataParams->wAdditionalInfo >= wBufferSize)
        {
            return PH_ADD_COMPCODE(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
        }

        /* Retrieve TxBuffer byte */
        *pValue = (uint16_t)pBuffer[pDataParams->wAdditionalInfo];
        break;

    case PHHAL_HW_CONFIG_MAX_PRECACHED_BYTES:

        /* Return parameter */
        *pValue = pDataParams->wMaxPrecachedBytes;
        break;

    case PHHAL_HW_CONFIG_BAL_CONNECTION:

        /* Return parameter */
        *pValue = (uint16_t)pDataParams->bBalConnectionType;
        break;

        /*
    case PHHAL_HW_CONFIG_SERIAL_BITRATE:

        // Read the register value
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_SERIALSPEED, &bRegister));

        switch (bRegister)
        {
        case PHHAL_HW_RC523_SERIALSPEED_9600:
            *pValue = PHHAL_HW_RS232_BITRATE_9600;
            break;
        case PHHAL_HW_RC523_SERIALSPEED_19200:
            *pValue = PHHAL_HW_RS232_BITRATE_19200;
            break;
        case PHHAL_HW_RC523_SERIALSPEED_38400:
            *pValue = PHHAL_HW_RS232_BITRATE_38400;
            break;
        case PHHAL_HW_RC523_SERIALSPEED_57600:
            *pValue = PHHAL_HW_RS232_BITRATE_57600;
            break;
        case PHHAL_HW_RC523_SERIALSPEED_115200:
            *pValue = PHHAL_HW_RS232_BITRATE_115200;
            break;
        case PHHAL_HW_RC523_SERIALSPEED_230400:
            *pValue = PHHAL_HW_RS232_BITRATE_230400;
            break;
        case PHHAL_HW_RC523_SERIALSPEED_460800:
            *pValue = PHHAL_HW_RS232_BITRATE_460800;
            break;
        default:
            return PH_ADD_COMPCODE(PH_ERR_INTERNAL_ERROR, PH_COMP_HAL);
        }
        break;
        */

    case PHHAL_HW_CONFIG_RFRESET_ON_TIMEOUT:

        /* Return parameter */
        *pValue = (uint16_t)pDataParams->bRfResetAfterTo;
        break;

    case PHHAL_HW_CONFIG_EXT_RF_ON:
        /* Retrieve Status register */
        PH_CHECK_SUCCESS_FCT(statusTmp, 
            phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_DIVIRQ, &bRegister));

        if (bRegister & PHHAL_HW_RC523_BIT_EXT_RF_OFF)
        {
            /* Clear the EXT_RF_ON and EXT_RF_OFF bits*/
            bRegister = 0x03;

            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_DIVIRQ, bRegister));
        }

        /* Retrieve Status register */
        PH_CHECK_SUCCESS_FCT(statusTmp,
            phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_DIVIRQ, &bRegister));

        /* Check EXT_RF_ON bit */
        if (bRegister & PHHAL_HW_RC523_BIT_EXT_RF_ON)
        {
            *pValue = PH_ON; /* ON if external RF field is detected */
        }
        else
        {
            *pValue = PH_OFF; /* OFF if no external RF field is detected */
        }
        break;

    case PHHAL_HW_CONFIG_RXMULTIPLE:
        *pValue = (uint16_t)pDataParams->bRxMultiple;
        break;

    case PHHAL_HW_CONFIG_ACTIVEMODE:

        *pValue = (uint16_t)pDataParams->bActiveMode;
        break;

    case PHHAL_HW_CONFIG_RX_FRAME_MODE:
        /* Read the RX frame mode from RXModeRegister */
        PH_CHECK_SUCCESS_FCT(statusTmp,
            phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, &bRegister));
        *pValue = bRegister & 0x03;
        break;

    case PHHAL_HW_CONFIG_AUTOCOLL_TIMER:
        *pValue = (uint16_t)pDataParams->bAutocollTimer;
        break;

    case PHHAL_HW_CONFIG_RXSPEED:
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, &bRegister));
        *pValue = bRegister & PHHAL_HW_RC523_MASK_SPEED;
        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_FieldOn(
                                 phhalHw_Rc523_DataParams_t * pDataParams
                                 )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t PH_MEMLOC_REM bTxControlReg;

    /* Not allowed in passive target mode */
    if ((pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT) && (pDataParams->bActiveMode != PH_ON))
    {
        return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_HAL);
    }

    /* Retrieve the content of the TxControl register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, &bTxControlReg));

    /* Switch on the field again */
    bTxControlReg |= PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN;
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, bTxControlReg));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_FieldOff(
                                  phhalHw_Rc523_DataParams_t * pDataParams
                                  )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bTxControlReg;

    /* Retrieve the content of the TxControl register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, &bTxControlReg));

    /* Switch off the field */
    bTxControlReg &= (uint8_t)~(uint8_t)(PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN);
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, bTxControlReg));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_FieldReset(
                                    phhalHw_Rc523_DataParams_t * pDataParams
                                    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;

    /* Switch off the field */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FieldOff(pDataParams));

    /* wait for field-off timeout */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_Wait(
        pDataParams,
        PHHAL_HW_TIME_MILLISECONDS,
        pDataParams->wFieldOffTime));

    /* switch on the field again */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FieldOn(pDataParams));

    /* wait for field-recovery timeout */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_Wait(
        pDataParams,
        PHHAL_HW_TIME_MILLISECONDS,
        pDataParams->wFieldRecoveryTime));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_Wait(
                              phhalHw_Rc523_DataParams_t * pDataParams,
                              uint8_t bUnit,
                              uint16_t wTimeout
                              )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bRegister;
    uint8_t        PH_MEMLOC_REM bIrq0Wait;
    uint8_t        PH_MEMLOC_REM bIrq1Wait;
    uint16_t    PH_MEMLOC_REM wTimerShift;
    uint16_t    PH_MEMLOC_REM wTimeoutNew;

    /* Parameter check */
    if ((bUnit != PHHAL_HW_TIME_MICROSECONDS) && (bUnit != PHHAL_HW_TIME_MILLISECONDS))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Terminate a probably running command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_IDLE));

    /* Retrieve RxDataRate */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetConfig(pDataParams, PHHAL_HW_CONFIG_RXDATARATE, &wTimerShift));
    ++wTimerShift;

    /* Calculate timershift */
    wTimerShift = (uint16_t)(PHHAL_HW_RC523_TIMER_SHIFT * (PHHAL_HW_RC523_ETU_106 / (float32_t)wTimerShift));

    /* do as long as we have an overflow in the IC timer */
    do
    {
        /* Set temporary timeout */
        if (bUnit == PHHAL_HW_TIME_MICROSECONDS)
        {
            wTimeoutNew = (wTimeout > wTimerShift) ? (wTimeout - wTimerShift) : 0;
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetFdt(pDataParams, PHHAL_HW_TIME_MICROSECONDS, wTimeoutNew));
            wTimeout = 0;
        }
        else
        {
            /* Timer would overflow -> use maximum value and decrement overall value for next iteration */
            if (wTimeout > ((PHHAL_HW_RC523_TIMER_MAX_VALUE_US / 1000) - 1))
            {
                wTimeoutNew = (PHHAL_HW_RC523_TIMER_MAX_VALUE_US / 1000) - 1;
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetFdt(pDataParams, PHHAL_HW_TIME_MILLISECONDS, wTimeoutNew));
                wTimeout = wTimeout - wTimeoutNew;
            }
            /* No overflow -> set the complete time */
            else
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetFdt(pDataParams, PHHAL_HW_TIME_MILLISECONDS, wTimeout));
                wTimeout = 0;
            }
        }

        /* retrieve content of Control register */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, &bRegister));

        /* enable timer interrupt */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIEN, PHHAL_HW_RC523_BIT_TIMERI));

        /* clear all irq flags */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_SET));

        /* start timer */
        bRegister |= PHHAL_HW_RC523_BIT_TSTARTNOW;
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, bRegister));

        /* wait for timer interrupt */
        bIrq0Wait = PHHAL_HW_RC523_BIT_ERRI | PHHAL_HW_RC523_BIT_TIMERI;
        bIrq1Wait = 0x00;
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WaitIrq(pDataParams, bIrq0Wait, bIrq1Wait, &bRegister, &bRegister));

        /* clear all irq flags */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_SET));
    }
    while (wTimeout > 0);

    /* Restore previous timeout */
    if (pDataParams->bTimeoutUnit == PHHAL_HW_TIME_MICROSECONDS)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetFdt(
            pDataParams,
            PHHAL_HW_TIME_MICROSECONDS,
            pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_US]));
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_SetFdt(
            pDataParams,
            PHHAL_HW_TIME_MILLISECONDS,
            pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS]));
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_MfcAuthenticateKeyNo(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t bBlockNo,
    uint8_t bKeyType,
    uint16_t wKeyNo,
    uint16_t wKeyVersion,
    uint8_t * pUid
    )
{
#ifdef NXPBUILD__PH_KEYSTORE
    phStatus_t  PH_MEMLOC_REM status;
    uint8_t     PH_MEMLOC_REM aKey[PHHAL_HW_MFC_KEY_LENGTH * 2];
    uint8_t *   PH_MEMLOC_REM pKey;
    uint16_t    PH_MEMLOC_REM bKeystoreKeyType;

    /* Bail out if we haven't got a keystore */
    if (pDataParams->pKeyStoreDataParams == NULL)
    {
        return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_HAL);
    }

    /* retrieve KeyA & KeyB from keystore */
    status = phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVersion,
        sizeof(aKey),
        aKey,
        &bKeystoreKeyType);

    /* Change component code for invalid paramter */
    if ((status & PH_ERR_MASK) == PH_ERR_INVALID_PARAMETER)
    {
        return PH_ADD_COMPCODE((status & PH_ERR_MASK), PH_COMP_HAL);
    }
    else
    {
        PH_CHECK_SUCCESS(status);
    }

    /* check key type */
    if (bKeystoreKeyType != PH_KEYSTORE_KEY_TYPE_MIFARE)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Evaluate which key to use */
    if (bKeyType == PHHAL_HW_MFC_KEYA)
    {
        /* Use KeyA */
        pKey = aKey;
    }
    else if (bKeyType == PHHAL_HW_MFC_KEYB)
    {
        /* Use KeyB */
        pKey = &aKey[PHHAL_HW_MFC_KEY_LENGTH];
    }
    else
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Call the authentication function using the key */
    return phhalHw_Rc523_MfcAuthenticate(pDataParams, bBlockNo, bKeyType, aKey, pUid);
}

phStatus_t phhalHw_Rc523_MfcAuthenticate(
    phhalHw_Rc523_DataParams_t * pDataParams,
    uint8_t bBlockNo,
    uint8_t bKeyType,
    uint8_t * pKey,
    uint8_t * pUid
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bFifoData[6 + PHHAL_HW_MFC_KEY_LENGTH];
    uint16_t    PH_MEMLOC_REM wFifoDataLen = 0;
    uint8_t     PH_MEMLOC_REM bRegister;
    uint8_t        PH_MEMLOC_REM bIrq1Rq;
    uint8_t     PH_MEMLOC_REM bIrq0Wait = 0x0;
    uint8_t     PH_MEMLOC_REM bIrq1Wait = 0x0;

    /* Evaluate which key to use */
    if (bKeyType == PHHAL_HW_MFC_KEYA)
    {
        /* Set authentication command code */
        bFifoData[wFifoDataLen++] = PHHAL_HW_RC523_MFC_AUTHA_CMD;
    }
    else if (bKeyType == PHHAL_HW_MFC_KEYB)
    {
        /* Set authentication command code */
        bFifoData[wFifoDataLen++] = PHHAL_HW_RC523_MFC_AUTHB_CMD;
    }
    else
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* set block number */
    bFifoData[wFifoDataLen++] = bBlockNo;

    /* copy the sector key */
    memcpy(&bFifoData[wFifoDataLen], pKey, PHHAL_HW_MFC_KEY_LENGTH);  /* PRQA S 3200 */
    wFifoDataLen += PHHAL_HW_MFC_KEY_LENGTH;

    /* set serial number key */
    memcpy(&bFifoData[wFifoDataLen], pUid, 4);  /* PRQA S 3200 */
    wFifoDataLen += 4;

    /* enable timer, and idle interrupt pins */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIEN, PHHAL_HW_RC523_BIT_TIMERI | PHHAL_HW_RC523_BIT_IDLEI));

    /* write command data into FiFo buffer */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteFifo(pDataParams, bFifoData, wFifoDataLen, &wFifoDataLen));

    /* clear all irq flags */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_SET));

    /* write the command into the command register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_AUTHENT));

    /* wait until command is finished */
    bIrq0Wait = PHHAL_HW_RC523_BIT_RXI | PHHAL_HW_RC523_BIT_ERRI | PHHAL_HW_RC523_BIT_TIMERI | PHHAL_HW_RC523_BIT_IDLEI;
    status = phhalHw_Rc523_WaitIrq(pDataParams, bIrq0Wait, bIrq1Wait,&bRegister,&bIrq1Rq);

    /* stop the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_IDLE));

    /* Flush FiFo */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FlushFifo(pDataParams));

    /* Status check */
    PH_CHECK_SUCCESS(status);

    /* Check for timeout */
    if (bRegister & PHHAL_HW_RC523_BIT_TIMERI)
    {
        return PH_ADD_COMPCODE(PH_ERR_IO_TIMEOUT, PH_COMP_HAL);
    }

    /* Check auth success */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_STATUS2, &bRegister));
    if ((bRegister & PHHAL_HW_RC523_BIT_CRYPTO1ON) != PHHAL_HW_RC523_BIT_CRYPTO1ON)
    {
        return PH_ADD_COMPCODE(PH_ERR_AUTH_ERROR, PH_COMP_HAL);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_HAL);
}

phStatus_t phhalHw_Rc523_SetMinFDT(
                                   phhalHw_Rc523_DataParams_t * pDataParams,
                                   uint16_t wValue
                                   )
{
    phStatus_t    PH_MEMLOC_REM status = 0;
    uint16_t    PH_MEMLOC_REM wTimer = 0;
    uint16_t    PH_MEMLOC_REM wTxRate = 0;

    if (wValue == PH_ON)
    {
        /*Backup the old Timer values and set min FDT*/
        PH_CHECK_SUCCESS_FCT(status, phhalHw_Rc523_GetConfig(pDataParams, 
            PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS, &wTimer));
        pDataParams->dwFdtPc = wTimer;
        /* Get the data rate */
        PH_CHECK_SUCCESS_FCT(status, phhalHw_Rc523_GetConfig(pDataParams, 
            PHHAL_HW_CONFIG_TXDATARATE, &wTxRate));
        switch(wTxRate)
        {
        case PHHAL_HW_RF_DATARATE_106:
            wTimer = PHHAL_HW_MINFDT_106_US;
            break;
        case PHHAL_HW_RF_DATARATE_212:
            wTimer = PHHAL_HW_MINFDT_212_US;
            break;
        case PHHAL_HW_RF_DATARATE_424:
            wTimer = PHHAL_HW_MINFDT_424_US;
            break;
        case PHHAL_HW_RF_DATARATE_848:
            wTimer = PHHAL_HW_MINFDT_848_US;
            break;
        default:
            break;
        }
        /* Calculate values for Microsecond values */
        PH_CHECK_SUCCESS_FCT(status, phhalHw_Rc523_SetConfig(pDataParams, 
            PHHAL_HW_CONFIG_TIMEOUT_VALUE_US, wTimer));
    }
    else if (wValue == PH_OFF)
    {
        PH_CHECK_SUCCESS_FCT(status, phhalHw_Rc523_SetConfig(pDataParams, 
            PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS, pDataParams->dwFdtPc));
    }
    else
    {
        /* Do nothing*/
    }
    return status;
}

phStatus_t phhalHw_Rc523_Autocoll(
                                  phhalHw_Rc523_DataParams_t * pDataParams,
                                  uint8_t ** ppRxBuffer,
                                  uint16_t * pRxLength
                                  )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wFifoBytes;
    uint8_t     PH_MEMLOC_REM bIrq0WaitFor;
    uint8_t     PH_MEMLOC_REM bIrq1WaitFor;
    uint8_t     PH_MEMLOC_REM bIrq0Rq;
    uint8_t     PH_MEMLOC_REM bIrq1Rq;
    uint8_t     PH_MEMLOC_REM bStatus2;
    uint8_t     PH_MEMLOC_REM bReg;
    uint8_t     PH_MEMLOC_REM bRegister;
    uint8_t     PH_MEMLOC_REM bError;
    uint8_t *   PH_MEMLOC_REM pTmpBuffer;
    uint16_t    PH_MEMLOC_REM wTmpBufferLen;
    uint16_t    PH_MEMLOC_REM wTmpBufferSize;
    uint8_t     PH_MEMLOC_REM offsetPos = 0;
    uint8_t     PH_MEMLOC_REM offsetLen = 0;

    /* set the receive length */
    if (pRxLength != NULL)
    {
        *pRxLength = 0;
    }

    /* Terminate a probably running command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_IDLE));

    /* Flush FiFo */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FlushFifo(pDataParams));

    /* clear all IRQ0 flags */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(
        pDataParams,
        PHHAL_HW_RC523_REG_COMMIRQ,
        (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_SET));

    /* clear all IRQ1 flags */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WriteRegister(
        pDataParams,
        PHHAL_HW_RC523_REG_DIVIRQ,
        (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_SET));

    if(pDataParams->bAutocollTimer == PH_ON)
    {
        /* retrieve content of Timer Control register */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, &bRegister));
        /* manually start Timer */
        bRegister |= PHHAL_HW_RC523_BIT_TSTARTNOW;
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, bRegister));
    }

    /* start the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_AUTOCOLL));

    PH_CHECK_SUCCESS_FCT(statusTmp,
        phhalHw_ReadRegister(
        pDataParams,
        PHHAL_HW_RC523_REG_TXASK,
        &bRegister));
    bReg = bRegister;
    bRegister |= (PHHAL_HW_RC523_BIT_TX2RFAutoEN | PHHAL_HW_RC523_BIT_TX1RFAutoEN
        | PHHAL_HW_RC523_BIT_AUTORF_OFF | PHHAL_HW_RC523_BIT_CA_ON);

    PH_CHECK_SUCCESS_FCT(statusTmp,
        phhalHw_WriteRegister(
        pDataParams,
        PHHAL_HW_RC523_REG_TXASK,
        bRegister));

    if(pDataParams->bAutocollTimer == PH_OFF)
    {
        bIrq0WaitFor = PHHAL_HW_RC523_BIT_IDLEI | PHHAL_HW_RC523_BIT_ERRI;
    }
    else
    {
        bIrq0WaitFor = PHHAL_HW_RC523_BIT_IDLEI | PHHAL_HW_RC523_BIT_ERRI | PHHAL_HW_RC523_BIT_TIMERI;
    }
    bIrq1WaitFor = 0x00;

    /* Wait until the command is finished */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_WaitIrq(
        pDataParams,
        bIrq0WaitFor,
        bIrq1WaitFor,
        &bIrq0Rq,
        &bIrq1Rq));

    /* Timeout handling */
    if (bIrq0Rq & PHHAL_HW_RC523_BIT_TIMERI)
    {
        status = PH_ERR_IO_TIMEOUT;
        return status;
    }

    if (pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT)
    {
        /* we need to check if target is activated */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_STATUS2, &bStatus2));

        if (bStatus2 & PHHAL_HW_RC523_BIT_TARGET_ACTIVATED)
        {
            /* passive communication */
            status = PH_ERR_SUCCESS;
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_WriteRegister(
                pDataParams,
                PHHAL_HW_RC523_REG_TXASK,
                bReg));
        }
        /* if target is not activated, then it would be active mode communication */
        else
        {
            status = PH_ERR_SUCCESS;
        }
    }
    else
    {
        /* Timeout handling */
        if (bIrq0Rq & PHHAL_HW_RC523_BIT_TIMERI)
        {
            status = PH_ERR_IO_TIMEOUT;
        }
        else
        {
            status = PH_ERR_SUCCESS;
        }
    }

    /* Reset receive buffer length */
    pDataParams->wRxBufLen = 0;

    /* Retrieve receive buffer properties */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_GetRxBuffer(pDataParams, PH_ON, &pTmpBuffer, &wTmpBufferLen, &wTmpBufferSize));

    /* Do the following if no timeout occured */
    if (status == PH_ERR_SUCCESS)
    {
        /* mask out high-alert */
        bIrq0WaitFor &= (uint8_t)~(uint8_t)PHHAL_HW_RC523_BIT_HIALERTI;

        if (pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18092MPT)
        {
            do
            {
                /* check if there is an error or of modem status went to Wait for StartSend */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, &bIrq0Rq));
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_STATUS2, &bStatus2));
            }
            while ((!(bIrq0Rq & bIrq0WaitFor)) && !((bStatus2 & 0x07) == 0x01));
        }

        /* retrieve fifo bytes */
        do
        {
            /* retrieve bytes from FiFo */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_ReadFifo(
                pDataParams,
                wTmpBufferSize,
                &pTmpBuffer[wTmpBufferLen],
                &wFifoBytes));

            /* advance receive buffer */
            wTmpBufferLen = wTmpBufferLen + wFifoBytes;
            wTmpBufferSize = wTmpBufferSize - wFifoBytes;

            /* read interrupt status */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_COMMIRQ, &bIrq0Rq));
        }
        while ((!(bIrq0Rq & bIrq0WaitFor)) || (wFifoBytes != 0));

        if(!(bStatus2 & PHHAL_HW_RC523_BIT_TARGET_ACTIVATED))
        {
            /*TODO : Check if time delay is required before checking PHHAL_HW_RC523_REG_TXCONTROL Reg values for TX2RFEN and TX1RFEN */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_TXCONTROL, &bReg));
            if (bReg & (PHHAL_HW_RC523_BIT_TX2RFEN | PHHAL_HW_RC523_BIT_TX1RFEN))
            {
                PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams, PHHAL_HW_CONFIG_ACTIVEMODE, PH_ON));
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, &bRegister));
                /* If DetectSync bit is set for active mode(106 baud rate), Hardware removes the "F0"
                ** But incase of target, "Detectsync" would be enabled after receiving ATR, so to
                ** normalize the response, "F0" is offset here
                */
                if((bRegister & PHHAL_HW_RC523_MASK_SPEED) == PHHAL_HW_RF_DATARATE_106)
                {
                    offsetPos = 1;
                    /* offset for startByte and CRC as it included if active communication */
                    offsetLen = 3;
                }
            }
            else
            {
                /* its a protocol error*/
                status = PH_ERR_PROTOCOL_ERROR;
                return status;
            }
        }
        /* Check for errors */
        if (bIrq0Rq & PHHAL_HW_RC523_BIT_ERRI)
        {
            /* read the error register */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_ERROR, &bError));

            /* Fifo write error */
            if (bError & PHHAL_HW_RC523_BIT_WRERR)
            {
                status = PH_ERR_READ_WRITE_ERROR;
            }
            /* temperature error */
            else if (bError & PHHAL_HW_RC523_BIT_TEMPERR)
            {
                status = PH_ERR_TEMPERATURE_ERROR;
            }
            /* RF error (should not happen) */
            /*
            if (bError & PHHAL_HW_RC523_BIT_RFERR)
            {
            status = PH_ERR_RF_ERROR;
            }
            */
            /* buffer overflow */
            else if (bError & PHHAL_HW_RC523_BIT_BUFFEROVFL)
            {
                status = PH_ERR_BUFFER_OVERFLOW;
            }
            /* protocol error */
            else if (bError & PHHAL_HW_RC523_BIT_PROTERR)
            {
                status = PH_ERR_PROTOCOL_ERROR;
            }
            /* CRC / parity error */
            else if ((bError & PHHAL_HW_RC523_BIT_CRCERR) || (bError & PHHAL_HW_RC523_BIT_PARITYERR))
            {
                status = PH_ERR_INTEGRITY_ERROR;
            }
            /* No error */
            else
            {
                status = PH_ERR_SUCCESS;
            }
        }
    }

    /* Receive was successfull */
    if (status == PH_ERR_SUCCESS)
    {
        /* No bytes received -> timeout */
        if (wTmpBufferLen == 0)
        {
            status = PH_ERR_IO_TIMEOUT;
        }
        /* Else retrieve valid bits of last byte */
        else
        {
            /* Retrieve RxBits */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_CONTROL, &bRegister));

            /* Mask out valid bits of last byte */
            bRegister &= PHHAL_HW_RC523_MASK_RXBITS;

            /* Set RxLastBits */
            pDataParams->wAdditionalInfo = bRegister;

            /* Set incomplete byte status if applicable */
            if (bRegister != 0x00)
            {
                status = PH_ERR_SUCCESS_INCOMPLETE_BYTE;
            }
        }
    }

    if (pDataParams->bCardType != PHHAL_HW_CARDTYPE_I18092MPT)
    {
        /* stop the command */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_WriteRegister(pDataParams, PHHAL_HW_RC523_REG_COMMAND, PHHAL_HW_RC523_CMD_IDLE));
    }

    /* Flush the FIFO on error */
    if ((status != PH_ERR_SUCCESS) &&
        (status != PH_ERR_SUCCESS_INCOMPLETE_BYTE))
    {
        /* Flush FiFo */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Rc523_FlushFifo(pDataParams));
    }
    /* Switch on CRC after successfull activation */
    else
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, PHHAL_HW_CONFIG_TXCRC, PH_ON));
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, PHHAL_HW_CONFIG_RXCRC, PH_ON));
    }

    /* Store received data length in dataparams */
    pDataParams->wRxBufLen = pDataParams->wRxBufStartPos + wTmpBufferLen;

    /* Return RxBuffer pointer */
    if (ppRxBuffer != NULL)
    {
        /* Offset "F0" incase of Active communication */
        *ppRxBuffer = pDataParams->pRxBuffer + offsetPos;
    }

    /* Return RxBuffer length */
    if (pRxLength != NULL)
    {
        /* Offset "F0" + CRC incase of Active communication */
        *pRxLength = pDataParams->wRxBufLen - offsetLen;
    }

    /* Read RX Mode register */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_ReadRegister(pDataParams, PHHAL_HW_RC523_REG_RXMODE, &bRegister));
    /* Check if received data is ATR */
    /* TODO: Check for RATS */
    if((bRegister & 0x03) == PHHAL_HW_I14443A_106_FRAMING)
    {
        if((pDataParams->pRxBuffer[2] != 0xD4) || (pDataParams->pRxBuffer[3] != 0x00))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_HAL);
        }
    }
    else 
    {
        if((pDataParams->pRxBuffer[1 + offsetPos] != 0xD4) || (pDataParams->pRxBuffer[2 + offsetPos] != 0x00))
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_HAL);
        }
    }
    return PH_ADD_COMPCODE(status, PH_COMP_HAL);

}

phStatus_t phhalHw_Rc523_Config(
                                phhalHw_Rc523_DataParams_t * pDataParams,
                                uint8_t * pSensRes,
                                uint8_t * pNfcId1,
                                uint8_t SelRes,
                                uint8_t * pPollingResp,
                                uint8_t bNfcId3
                                )
{
    uint8_t PH_MEMLOC_BUF aCmd[25];

    memcpy(&aCmd[0], pSensRes, 2);      /* PRQA S 3200 */
    memcpy(&aCmd[2], pNfcId1, 3);       /* PRQA S 3200 */
    memcpy(&aCmd[5], &SelRes, 1);       /* PRQA S 3200 */
    memcpy(&aCmd[6], pPollingResp, 18); /* PRQA S 3200 */
    memcpy(&aCmd[24], &bNfcId3, 1);     /* PRQA S 3200 */

    return phhalHw_Rc523_Cmd_Mem(pDataParams, aCmd, 25, NULL);
}
#endif /* NXPBUILD__PHHAL_HW_RC523 */
