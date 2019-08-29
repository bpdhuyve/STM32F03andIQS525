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
* $Author: nxp67988 $
* $Revision: 336 $
* $Date: 2013-08-08 20:14:50 +0530 (Thu, 08 Aug 2013) $
*/
#include <ph_Typedefs.h>
#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phacDiscLoop.h>
#include <phalMfdf.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW

#include "phalTop_Sw_Int_T4T.h"

phStatus_t phalTop_Sw_Int_T4T_CheckNdef(
                                        phalTop_T4T_t * pT4T,
                                        uint8_t * pNdefPresence
                                        )
{
    uint16_t PH_MEMLOC_REM status;
    uint16_t PH_MEMLOC_REM pBytesRead;
    uint8_t  PH_MEMLOC_REM *ppRxBuffer;
    uint8_t  PH_MEMLOC_REM aFid_MApp[2] = {0x00, 0x3F}; /* Master Application ISO File ID */
    uint8_t  PH_MEMLOC_REM aFid_CC[2] = {0x03, 0xE1};
    uint8_t  PH_MEMLOC_REM aNdefApp_Name[7] = {0xD2, 0x76, 0x00,        /**< DF Name buffer */
                                               0x00, 0x85, 0x01, 0x01};
    uint8_t  PH_MEMLOC_REM aNdefApp_Len = 0X07;
    uint8_t  PH_MEMLOC_REM *pFCI;
    uint16_t PH_MEMLOC_REM *pwFCILen = 0;
    uint16_t PH_MEMLOC_REM wNdef_Fid;
    uint16_t PH_MEMLOC_REM wErrorCode;
    phalMfdf_Sw_DataParams_t PH_MEMLOC_REM *pMfdf;
    pMfdf = (phalMfdf_Sw_DataParams_t *)(pT4T->phalT4TDataParams);

    pMfdf->bWrappedMode = 1;
    pT4T->bNdefSupport = PH_UNSUPPORTED;
    pT4T->bNdefFormatted = PH_UNSUPPORTED;
    pT4T->bNdefPresence = false;
    *pNdefPresence = PH_OFF;

    /* Select the NDEF Tag Application with ISO File id of the Application */
    status = phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x04,
        0,                                          /* Look for File ID of 0xE105 */
        aNdefApp_Name,
        aNdefApp_Len,
        &pFCI,
        pwFCILen
        );
    if((status & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
    {
        return status;
    }

    /* If No Valid Application is found then Format to make Tag as NDEF */
    wErrorCode = pMfdf->wAdditionalInfo;
    if(wErrorCode == 0x6A82)                             /* PHAL_MFDF_ISO7816_ERR_FILE_NOT_FOUND = 0x6A82*/
    {
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T4T_FormatNdef(pT4T));
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            PH_EXCHANGE_DEFAULT,
            0x04,
            0,
            aNdefApp_Name,
            aNdefApp_Len,
            &pFCI,
            pwFCILen
            ));
    }

    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
    	pT4T->bNdefFormatted = PH_SUPPORTED;

        /* Select the Capability Container (CC) file once NDEF Tag Application is selected */
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            0x0C,                              /* If bOption == 00 FCI(file control info) is returned. If 0x0C no FCI returned */
            0x00,
            aFid_CC,                                          /* Look for File ID of 0xE103 */
            NULL,
            0x00,
            &pFCI,
            pwFCILen
            ));

        /* Read the CC file and select the NDEF file */
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
            pT4T->phalT4TDataParams,
            PH_EXCHANGE_DEFAULT,
            0x00,                                             /* File offset where to start reading data */
            0x00,
            0x0F,                                             /* number of bytes to read. If 0, then entire file to be read */
            &ppRxBuffer,
            &pBytesRead
            ));

        pT4T->wCCLEN = ((uint16_t)ppRxBuffer[PHAL_TOP_T4T_CCLEN_OFFSET] << 8) | ppRxBuffer[PHAL_TOP_T4T_CCLEN_OFFSET + 1];
        if(pT4T->wCCLEN < 0x000F)
        {
            return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
        }

        if(ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET] == PHAL_TOP_T4T_NDEF_TLV)
        {
            pT4T->bRa = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 6];

            if(pT4T->bRa == PHAL_TOP_T4T_NDEF_FILE_READ)
            {
                pT4T->bWa = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 7];
                pT4T->bVno = ppRxBuffer[PHAL_TOP_T4T_VERSION_OFFSET];
                /* Read NDEF File ID from CC */
                pT4T->bNdef_FileID[0] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 3];    /* MSB of bNdef_FileID */
                pT4T->bNdef_FileID[1] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 2];    /* LSB of bNdef_FileID */
                pT4T->bMaxFileSize = ((uint16_t)ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 4] << 8) | ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 5];

                wNdef_Fid = ((uint16_t)pT4T->bNdef_FileID[1] << 8) | pT4T->bNdef_FileID[0];

                if(!((wNdef_Fid == 0x0000) ||
                    (wNdef_Fid == 0xE102) ||
                    (wNdef_Fid == 0xE103) ||
                    (wNdef_Fid == 0x3F00) ||
                    (wNdef_Fid == 0x3FFF)))
                {
                    /* select the NDEF file  */
                    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
                        pT4T->phalT4TDataParams,
                        0x0C,
                        0x00,
                        pT4T->bNdef_FileID,
                        NULL,
                        0x00,
                        &pFCI,
                        pwFCILen
                        ));

                    /* Read the NDEF file */
                    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
                        pT4T->phalT4TDataParams,
                        PH_EXCHANGE_DEFAULT,
                        0x00,                                 /* File offset where to start reading data */
                        0x00,
                        0x02,                                 /* number of bytes to read. If 0, then entire file to be read */
                        &ppRxBuffer,
                        &pBytesRead
                        ));

                    pT4T->bNdefSupport = PH_SUPPORTED;
                    pT4T->wNLEN = ((uint16_t)ppRxBuffer[0] << 8) | ppRxBuffer[1];

                    if((pT4T->wNLEN > 0x0000) && (pT4T->wNLEN <= (pT4T->bMaxFileSize - 2)))
                    {
                        pT4T->bNdefPresence = true;
                        *pNdefPresence = PH_ON;

                        if(pT4T->bWa == PHAL_TOP_T4T_NDEF_FILE_NO_WRITE_ACCESS)
                        {
                            pT4T->bTagState = PHAL_TOP_T4T_STATE_READONLY;
                        }
                        else if(pT4T->bWa == PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS)
                        {
                            pT4T->bTagState = PHAL_TOP_T4T_STATE_READWRITE;
                        }
                        else
                        {
                            pT4T->bTagState = PHAL_TOP_T4T_STATE_UNKNOWN;
                        }
                    }
                    else if((pT4T->wNLEN == 0x0000) || (pT4T->wNLEN > (pT4T->bMaxFileSize - 2)))
                    {
                        pT4T->bNdefSupport = PH_SUPPORTED;
                        pT4T->bTagState = PHAL_TOP_T4T_STATE_INITIALIZED;
                    }
                    else
                    {
                        /* Do Nothing */
                    }
                }
                else
                {
                    return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
                }
            }
            else
            {
                return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
            }
        }
        else
        {
            return PH_ADD_COMPCODE(PHAL_TOP_T4T_ERR_NO_NDEF_TLV, PH_COMP_AL_TOP);
        }

        /* Select the Master Tag Application with ISO File ID (0x3F00) of the Application */
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            0x0C,
            0x00,
            aFid_MApp,
            NULL,
            0x00,
            &pFCI,
            pwFCILen
            ));
    }
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T4T_FormatNdef(
    phalTop_T4T_t * pT4T
    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t PH_MEMLOC_REM aNDEF_msg[2] = {0x00, 0x00};
    uint8_t PH_MEMLOC_REM aFileSize[3] = {0x0F, 0x00, 0x00};        /**< File size specifier */
    uint8_t PH_MEMLOC_REM aNdefFileLen[3] = {0x04, 0x00, 0x00};     /**< Valid data buffer length */
    uint8_t PH_MEMLOC_REM aOffset[3] = {0x00, 0x00, 0x00};          /**< Offset for write data */
    uint8_t PH_MEMLOC_REM aAccessRights[2] = {0xEE, 0xEE};          /**< Access Rights */
    uint8_t PH_MEMLOC_REM aAid[3] = {0x01, 0x00, 0x00};
    uint8_t PH_MEMLOC_REM aFid_App[2] = {0x05, 0xE1};
    uint8_t PH_MEMLOC_REM aFid_MApp[2] = {0x00, 0x3F};              /**< Master Application ISO File ID */
    uint8_t PH_MEMLOC_REM aFid_CC[2] = {0x03, 0xE1};
    uint8_t  PH_MEMLOC_REM *pFCI;
    uint16_t PH_MEMLOC_REM *pwFCILen = 0;
    /* Default CC File contents with 0xE104 as NDEF FileID
    *  and with 1KBytes as Max NDEF File Size */
    uint8_t PH_MEMLOC_REM aData[15] = {0x00, 0x0F, 0x20, 0x00,
        0x3B, 0x00, 0x34, 0x04,
        0x06, 0xE1, 0x04, 0x04,
        0x00, 0x00, 0x00};
    uint8_t PH_MEMLOC_REM aNdefApp_Name[7] = {0xD2, 0x76, 0x00,        /**< DF Name buffer */
                                              0x00, 0x85, 0x01, 0x01};
    uint8_t  PH_MEMLOC_REM aNdefApp_Len = 0X07;
    uint16_t PH_MEMLOC_REM wNdef_Fid;
    phalMfdf_Sw_DataParams_t PH_MEMLOC_REM *pMfdf;
    pMfdf = (phalMfdf_Sw_DataParams_t *)(pT4T->phalT4TDataParams);

    pMfdf->bWrappedMode = 1;
    if(pT4T->bNdefSupport == PH_SUPPORTED)
    {
        /* CheckNDEF returned with Valid Card State. Erase the contents of Tag. */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T4T_EraseNdef(pT4T));
        pT4T->bNdefFormatted = true;
        return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
    }

    /* Check for valid NDEF File ID provided by user or default NDEF ID = 0xE104*/
    wNdef_Fid = ((uint16_t)pT4T->bNdef_FileID[1] << 8) | pT4T->bNdef_FileID[0];

    if(!((wNdef_Fid == 0x0000) ||
        (wNdef_Fid == 0xE102) ||
        (wNdef_Fid == 0xE103) ||
        (wNdef_Fid == 0x3F00) ||
        (wNdef_Fid == 0x3FFF)))
    {
        aData[0x0A] = pT4T->bNdef_FileID[0];
        aData[0x09] = pT4T->bNdef_FileID[1];
    }
    else
    {
        pT4T->bNdef_FileID[0] = aData[0x0A];
        pT4T->bNdef_FileID[1] = aData[0x09];
    }

    if(!((pT4T->bMaxFileSize < 0x0005) ||
        (pT4T->bMaxFileSize > 0x80FE)))
    {
        aData[0x0B] = (uint8_t)((uint16_t)(pT4T->bMaxFileSize & 0xFF00) >> 8);
        aData[0x0C] = (uint8_t)(pT4T->bMaxFileSize & 0x00FF);
    }

    if((pT4T->bWa == PHAL_TOP_T4T_NDEF_FILE_NO_WRITE_ACCESS) ||
        (pT4T->bWa == PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS))
    {
        aData[0x0E] = pT4T->bWa;
    }

    /* Create Application */
    status = phalMfdf_CreateApplication(
        pT4T->phalT4TDataParams,
        0x03,
        aAid,
        0x0F,
        0x21,
        aFid_App,
        aNdefApp_Name,
        aNdefApp_Len);

    if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
    {
        /* First call CheckNDEF before calling FormatNDEF */
        return PH_ADD_COMPCODE(PHAL_TOP_T4T_ERR_ALREADY_FORMATTED, PH_COMP_AL_TOP);
    }

    /* Select the AFC Application */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_SelectApplication(
        pT4T->phalT4TDataParams,
        aAid));

    /* Create Std File for CC */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_CreateStdDataFile(
        pT4T->phalT4TDataParams,
        0x01,
        0x01,
        aFid_CC,
        PHAL_MFDF_COMMUNICATION_PLAIN,
        aAccessRights,
        aFileSize));

    aNdefFileLen[0] = aData[0x0C];
    aNdefFileLen[1] = aData[0x0B];

    /* Create Std File for NDEF with user defined NdefID or with file ID 0xE104 */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_CreateStdDataFile(
        pT4T->phalT4TDataParams,
        0x01,
        0x02,
        pT4T->bNdef_FileID,                     /* NDEF ISO File Id */
        PHAL_MFDF_COMMUNICATION_PLAIN,
        aAccessRights,
        aNdefFileLen));

    /* Write data to CC file */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_WriteData(
        pT4T->phalT4TDataParams,
        PHAL_MFDF_COMMUNICATION_PLAIN,
        0x01,
        aOffset,
        aData,
        aFileSize));

    /* Now Initialize NDEF File (File No. 2) with NdefLength 0x0000 */
    aNdefFileLen[0] = 0x02;
    aNdefFileLen[1] = 0x00;

    /* Write data to NDEF file */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_WriteData(
        pT4T->phalT4TDataParams,
        PHAL_MFDF_COMMUNICATION_PLAIN,
        0x02,
        aOffset,
        aNDEF_msg,
        aNdefFileLen));

    pT4T->bNdefFormatted = true;
    pT4T->bNdefPresence = PH_OFF;
    pT4T->bTagState = PHAL_TOP_T4T_STATE_INITIALIZED;

    /* Select the Master Tag Application with ISO File ID (0x3F00) of the Application */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x00,
        aFid_MApp,
        NULL,
        0x00,
        &pFCI,
        pwFCILen
        ));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T4T_ReadNdef(
                                       phalTop_T4T_t * pT4T,
                                       uint8_t * pData,
                                       uint16_t * pLength
                                       )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t PH_MEMLOC_REM bLCount = 0;
    uint16_t PH_MEMLOC_REM *pwRxlen = 0;
    uint16_t PH_MEMLOC_REM pBytesRead;
    uint16_t PH_MEMLOC_REM wNdefLength;
    uint16_t PH_MEMLOC_REM wLocLength;
    uint16_t PH_MEMLOC_REM bOffset;
    uint8_t PH_MEMLOC_REM P1;
    uint8_t PH_MEMLOC_REM P2;
    uint8_t PH_MEMLOC_REM *ppRecv;
    uint8_t PH_MEMLOC_REM *ppRxBuffer;
    uint8_t PH_MEMLOC_REM aFid_Ndef[2];
    uint8_t PH_MEMLOC_REM aFid_MApp[2] = {0x00, 0x3F}; /* Master Application ISO File ID */
    uint8_t PH_MEMLOC_REM aFid_CC[2] = {0x03, 0xE1};
    uint8_t PH_MEMLOC_REM aNdefApp_Name[7] = {0xD2, 0x76, 0x00,        /**< DF Name buffer */
                                               0x00, 0x85, 0x01, 0x01};
    uint8_t PH_MEMLOC_REM aNdefApp_Len = 0X07;
    uint8_t PH_MEMLOC_REM *pFCI;
    uint16_t PH_MEMLOC_REM *pwFCILen = 0;
    uint16_t PH_MEMLOC_REM bcount;

    if(pT4T->bNdefPresence != PH_ON)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_TOP);
    }
    else
    {
        /* Select the NDEF Tag Application with ISO File id of the Application */
        status = phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            PH_EXCHANGE_DEFAULT,
            0x04,
            0,                                          /* Look for File ID of 0xE105 */
            aNdefApp_Name,
            aNdefApp_Len,
            &pFCI,
            pwFCILen
            );
        if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
        {
            return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
        }

        /* select CC file */
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            0x0C,
            0x00,
            aFid_CC,
            NULL,
            0x00,
            &ppRecv,
            pwRxlen
            ));

        /* Read the CC file and check for read access for NDEF file */
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
            pT4T->phalT4TDataParams,
            PH_EXCHANGE_DEFAULT,
            0x00,                                         /* File offset where to start reading data */
            0x00,
            0x0F,                                         /* number of bytes to read. If 0, then entire file to be read */
            &ppRxBuffer,
            &pBytesRead
            ));

        pT4T->bRa = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 6];

        if(pT4T->bRa == PHAL_TOP_T4T_NDEF_FILE_READ)
        {
            /* Read NDEF File ID from CC */
            aFid_Ndef[1] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 2];
            aFid_Ndef[0] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 3];

            /* select NDEF file */
            PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
                pT4T->phalT4TDataParams,
                0x0C,
                0x00,
                aFid_Ndef,
                NULL,
                0x00,
                &ppRecv,
                pwRxlen
                ));

            PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
                pT4T->phalT4TDataParams,
                PH_EXCHANGE_DEFAULT,
                0x00,                                        /* NDEF msg starts from offset 0x02 */
                0x00,
                0x02,
                &ppRxBuffer,
                &pBytesRead
                ));
            wNdefLength = ((uint16_t)ppRxBuffer[0] << 8) | ppRxBuffer[1];
            if(wNdefLength != pT4T->wNLEN)
            {
                /* Neither CheckNDEF is called nor misuse of SetConfig */
                return PH_ADD_COMPCODE(PHAL_TOP_T4T_ERR_INVALID_LENGHT, PH_COMP_AL_TOP);
            }

            wLocLength = pT4T->wNLEN;
            *pLength = pT4T->wNLEN;
            for(bcount = 0; wLocLength > 0x00FF; wLocLength -= 0x00FF, bcount += 0xFF)
            {
                bOffset = bcount + 2;                                       /* NDEF msg starts from offset 0x02 */
                P1 = (uint8_t)((bOffset & 0xFF00) >> 8);
                P2 = (uint8_t)(bOffset & 0x00FF);

                PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
                    pT4T->phalT4TDataParams,
                    PH_EXCHANGE_DEFAULT,
                    P2,
                    P1,
                    0xFF,
                    &ppRxBuffer,
                    &pBytesRead
                    ));

                for(bLCount = 0; bLCount < 0xFF; bLCount++)
                {
                    pData[bcount + bLCount] = ppRxBuffer[bLCount];
                }
            }
            bOffset = bcount + 2;                                          /* NDEF msg starts from offset 0x02 */
            P1 = (uint8_t)((bOffset & 0xFF00) >> 8);
            P2 = (uint8_t)(bOffset & 0x00FF);

            PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
                pT4T->phalT4TDataParams,
                PH_EXCHANGE_DEFAULT,
                P2,
                P1,
                (uint8_t)wLocLength,
                &ppRxBuffer,
                &pBytesRead
                ));
            for(bLCount = 0; bLCount < wLocLength; bLCount++)
            {
                pData[bcount + bLCount] = ppRxBuffer[bLCount];
            }
        }
    }

    /* Select the Master Tag Application with ISO File ID (0x3F00) of the Application */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x00,
        aFid_MApp,
        NULL,
        0x00,
        &pFCI,
        pwFCILen
        ));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T4T_WriteNdef(
                                        phalTop_T4T_t * pT4T,
                                        uint8_t * pData,
                                        uint16_t wLength
                                        )
{
    uint16_t PH_MEMLOC_REM status;
    uint16_t PH_MEMLOC_REM *pwFCILen = 0;
    uint8_t  PH_MEMLOC_REM *pFCI;
    uint16_t PH_MEMLOC_REM *pwRxlen = 0;
    uint16_t PH_MEMLOC_REM pBytesRead;
    uint16_t PH_MEMLOC_REM wLocLength;
    uint16_t PH_MEMLOC_REM bcount;
    uint16_t PH_MEMLOC_REM bOffset;
    uint8_t PH_MEMLOC_REM *ppRecv;
    uint8_t PH_MEMLOC_REM atemp[2] = {0x00, 0x00};
    uint8_t PH_MEMLOC_REM aFid_MApp[2] = {0x00, 0x3F}; /* Master Application ISO File ID */
    uint8_t PH_MEMLOC_REM aFid_CC[2] = {0x03, 0xE1};
    uint8_t PH_MEMLOC_REM *ppRxBuffer;
    uint8_t PH_MEMLOC_REM aFid_Ndef[2];
    uint8_t PH_MEMLOC_REM aNDEFLength[2];
    uint8_t PH_MEMLOC_REM aCCData[0x0F];
    uint8_t PH_MEMLOC_REM bWriteAccess;
    uint8_t PH_MEMLOC_REM P1;
    uint8_t PH_MEMLOC_REM P2;
    uint8_t PH_MEMLOC_REM aNdefApp_Name[7] = {0xD2, 0x76, 0x00,        /**< DF Name buffer */
                                               0x00, 0x85, 0x01, 0x01};
    uint8_t PH_MEMLOC_REM aNdefApp_Len = 0X07;

    aNDEFLength[0] = (uint8_t)((wLength & 0xFF00) >> 8);
    aNDEFLength[1] = (uint8_t)(wLength & 0x00FF);
    wLocLength = wLength;

    if(!((pT4T->bNdefFormatted == true) || (pT4T->bNdefSupport == PH_SUPPORTED)))
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
    }
    if(wLength == 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Select the NDEF Tag Application with ISO File id of the Application */
    status = phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x04,
        0,                                          /* Look for File ID of 0xE105 */
        aNdefApp_Name,
        aNdefApp_Len,
        &pFCI,
        pwFCILen
        );
    if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
    }

    /* select CC file */
    status = phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        0x0C,
        0x00,
        aFid_CC,
        NULL,
        0x00,
        &ppRecv,
        pwRxlen
        );
    if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
    }

    /* Read the CC file and check for write access for NDEF file */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x00,                                              /* File offset where to start reading data */
        0x00,
        0x0F,                                              /* number of bytes to read. If 0, then entire file to be read */
        &ppRxBuffer,
        &pBytesRead
        ));
    for(bcount = 0; bcount < 0x0F; bcount++)
    {
        aCCData[bcount] = ppRxBuffer[bcount];
    }

    bWriteAccess = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 7];
    if(bWriteAccess == PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS)
    {
        /* Read NDEF File ID from CC */
        aFid_Ndef[1] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 2];
        aFid_Ndef[0] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 3];

        /* If write access without any security is granted for NDEF file, Select the NDEF file*/
        status = phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            0x0C,
            0x00,
            aFid_Ndef,
            NULL,
            0x00,
            &ppRecv,
            pwRxlen
            );
        if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
        {
            return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
        }

        /* If NDEF length is bigger than (Max NDEF size)-2, the NDEF update procedure is aborted */
        if(wLength > (pT4T->bMaxFileSize - 2))
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        /* Write the value 0000h in the NLEN field */
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoUpdateBinary(
            pT4T->phalT4TDataParams,
            0x00,                                          /* NLEN(NDEF Length) offset is 0x00 */
            0x00,                                          /* SFid Parameter */
            atemp,                                         /* write 0x0000 to NDEF Length */
            0x02                                           /* Data length is 2 byte */
            ));

        /* Write the NDEF message from 2nd Byte in memory */
        for(bcount = 0; wLocLength > 0x00FF; wLocLength -= 0x00FF, bcount += 0xFF)
        {
            bOffset = bcount + 2;
            P1 = (uint8_t)((bOffset & 0xFF00) >> 8);
            P2 = (uint8_t)(bOffset & 0x00FF);
            PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoUpdateBinary(
                pT4T->phalT4TDataParams,
                P2,                                                         /* Offset - LSB of Offset */
                P1,                                                         /* sFid - MSB of Offset (7Bits ) = P1*/
                &pData[bcount],
                0xFF
                ));
        }

        /* Write remaining NDEF message */
        bOffset = bcount + 2;
        P1 = (uint8_t)((bOffset & 0xFF00) >> 8);
        P2 = (uint8_t)(bOffset & 0x00FF);

        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoUpdateBinary(
            pT4T->phalT4TDataParams,
            P2,                                                         /* Offset - LSB of Offset */
            P1,                                                         /* sFid - MSB of Offset (7Bits ) = P1*/
            &pData[bcount],
            (uint8_t)wLocLength
            ));

        /* Write the length of NDEF message in the NLEN field */
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoUpdateBinary(
            pT4T->phalT4TDataParams,
            0x00,                                          /* NLEN offset is 0x00 */
            0x00,
            aNDEFLength,                                   /* Update NDEF Length */
            0x02
            ));

        pT4T->wNLEN = wLength;
        pT4T->bNdefPresence = true;
    }
    else
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_READONLY_TAG, PH_COMP_AL_TOP);
    }

    /* Make ReadOnly Tag if the User write WriteAccess = 0xFF by updating CC */
    if((pT4T->bWa == PHAL_TOP_T4T_NDEF_FILE_NO_WRITE_ACCESS) && (pT4T->bWa != bWriteAccess))
    {
        /* Select CC File to right Write Access as 0xFF - ReadOnly*/
        status = phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            0x0C,
            0x00,
            aFid_CC,
            NULL,
            0x00,
            &ppRecv,
            pwRxlen
            );
        if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
        {
            return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
        }

        aCCData[0x0E] = PHAL_TOP_T4T_NDEF_FILE_NO_WRITE_ACCESS;
        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoUpdateBinary(
            pT4T->phalT4TDataParams,
            0x02,
            0x00,
            aCCData,
            0x0F
            ));
        pT4T->bTagState = PHAL_TOP_T4T_STATE_READONLY;
    }

    /* Select the Master Tag Application with ISO File ID (0x3F00) of the Application */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x00,
        aFid_MApp,
        NULL,
        0x00,
        &pFCI,
        pwFCILen
        ));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T4T_EraseNdef(
                                        phalTop_T4T_t * pT4T
                                        )
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wCount;
    uint8_t    PH_MEMLOC_REM pData[0xFF] = {0x00};
    uint8_t    PH_MEMLOC_REM aFid_MApp[2] = {0x00, 0x3F}; /* Master Application ISO File ID */
    uint8_t    PH_MEMLOC_REM aFid_Ndef[2];
    uint8_t    PH_MEMLOC_REM aFid_CC[2] = {0x03, 0xE1};
    uint8_t    PH_MEMLOC_REM aNdefApp_Name[7] = {0xD2, 0x76, 0x00,        /**< DF Name buffer */
                                                 0x00, 0x85, 0x01, 0x01};
    uint8_t    PH_MEMLOC_REM aNdefApp_Len = 0X07;
    uint16_t   PH_MEMLOC_REM pBytesRead;
    uint16_t   PH_MEMLOC_REM wLocLength;
    uint8_t    PH_MEMLOC_REM *pFCI;
    uint8_t    PH_MEMLOC_REM *ppRxBuffer;
    uint16_t   PH_MEMLOC_REM *pwFCILen = 0;

    /* Select the NDEF Tag Application with ISO File id of the Application */
    status = phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x04,
        0,                                          /* Look for File ID of 0xE105 */
        aNdefApp_Name,
        aNdefApp_Len,
        &pFCI,
        pwFCILen
        );
    if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
    }

    /* Select the Capability Container (CC) file */
    status = phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        0x0C,
        0x00,
        aFid_CC,
        NULL,
        0x00,
        &pFCI,
        pwFCILen
        );
    if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
    {
        return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
    }

    /* Read the CC file and select the NDEF file */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoReadBinary(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x00,                                              /* File offset where to start reading data */
        0x00,
        0x0F,                                             /* number of bytes to read. If 0, then entire file to be read */
        &ppRxBuffer,
        &pBytesRead
        ));

    if(pT4T->bRa == PHAL_TOP_T4T_NDEF_FILE_READ)
    {
        /* Read NDEF File ID from CC */
        aFid_Ndef[1] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 2];
        aFid_Ndef[0] = ppRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 3];

        /* select the NDEF file  */
        status = phalMfdf_IsoSelectFile(
            pT4T->phalT4TDataParams,
            PH_EXCHANGE_DEFAULT,
            0x00,
            aFid_Ndef,
            NULL,
            0x00,
            &pFCI,
            pwFCILen
            );
        if((status & 0x00F0) == PH_ERR_CUSTOM_BEGIN)
        {
            return PH_ADD_COMPCODE(PHAL_TOP_ERR_NOT_FORMATTED, PH_COMP_AL_TOP);
        }

        wLocLength = pT4T->wNLEN + 2;
        for(wCount = 0; wLocLength > 0x00FF; wLocLength -= 0x00FF, wCount += 0xFF)
        {
            PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoUpdateBinary(
                pT4T->phalT4TDataParams,
                (uint8_t)wCount,
                0x00,
                pData,
                0xFF
                ));
        }

        PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoUpdateBinary(
            pT4T->phalT4TDataParams,
            (uint8_t)wCount,
            0x00,
            pData,
            (uint8_t)wLocLength
            ));
        pT4T->bTagState = PHAL_TOP_T4T_STATE_INITIALIZED;
    }

    pT4T->bNdefPresence = PH_OFF;

    /* Select the Master Tag Application with ISO File ID (0x3F00) of the Application */
    PH_CHECK_SUCCESS_FCT(status, phalMfdf_IsoSelectFile(
        pT4T->phalT4TDataParams,
        PH_EXCHANGE_DEFAULT,
        0x00,
        aFid_MApp,
        NULL,
        0x00,
        &pFCI,
        pwFCILen
        ));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

#endif /* NXPBUILD__PHAL_TOP_SW */

