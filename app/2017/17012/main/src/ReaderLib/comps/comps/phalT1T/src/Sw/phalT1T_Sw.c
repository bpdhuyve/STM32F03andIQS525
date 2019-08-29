#include <ph_Status.h>
#include <phalT1T.h>
#include <phpalI14443p3a.h>
#include <ph_RefDefs.h>
#include <phTools.h>

#ifdef NXPBUILD__PHAL_T1T_SW

#include "phalT1T_Sw.h"

phStatus_t phalT1T_Sw_Init(
                           phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                           uint16_t wSizeOfDataParams,                /**< [In] */
                           void * pPalI14443p3aDataParams             /**< [In] Pointer to the parameter structure of the underlying ISO14443-3 layer. */
                           )
{
    if (sizeof(phalT1T_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_T1T);
    }

    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pPalI14443p3aDataParams);

    /* init private data */
    pDataParams->wId                    = PH_COMP_AL_T1T | PHAL_T1T_SW_ID ;
    pDataParams->pPalI14443p3aDataParams   = pPalI14443p3aDataParams;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFUL);
}

phStatus_t phalT1T_Sw_RequestA  (
                                 phalT1T_Sw_DataParams_t * pDataParams,      /**< [In] */
                                 uint8_t * pAtqa                             /**< [Out] */
                                 )
{
    return phpalI14443p3a_RequestA( pDataParams->pPalI14443p3aDataParams , pAtqa );
}

phStatus_t phalT1T_Sw_ReadUID  (
                                phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                uint8_t * pUid,                            /**< [Out */
                                uint16_t * pLength                          /**< [Out] */
                                )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[] = { PHAL_T1T_CMD_READUID , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t     PH_MEMLOC_REM abOutBuffer[10] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wTempLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_READUID_RESP_LEN;               /*Rid response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 7; /* Command sequence len without Crc_B */
    uint16_t    PH_MEMLOC_REM wUIDLength = 4;

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) ;

    /*Send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT , &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && (statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        abOutBuffer ,
        &wTempLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (wTempLength != PHAL_T1T_READUID_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( &abOutBuffer[0], wOutBufferSize-2, aCrc));
    if(aCrc[0] == abOutBuffer[6] && aCrc[1] ==  abOutBuffer[7] )
    {
        /*update the Header ROM bytes*/
        pDataParams->abHR[0] = abOutBuffer[0];
        pDataParams->abHR[1] = abOutBuffer[1];

        /*update the UID*/
        for( bCountLoop = 0 ; bCountLoop < 4 ; bCountLoop++ )
        {
            pDataParams->abUid[bCountLoop] = abOutBuffer[bCountLoop+2];
            pUid[bCountLoop] = pDataParams->abUid[bCountLoop];
        }
        (*pLength) = wUIDLength;   /*UID length*/

        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_ReadAll  (
                                phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                uint8_t * pUid,                            /**< [In] */
                                uint8_t * pData,                           /**< [Out] */
                                uint16_t * pLength                         /**< [Out] */
                                )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[10] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_READALL_RESP_LEN;                    /*Rall response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 7;   /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    abTxBuffer[0] = PHAL_T1T_CMD_READALL;   /*cmd*/
    abTxBuffer[1] = 0x00;                   /*address*/
    abTxBuffer[2] = 0x00;                   /*data*/

    for( bCountLoop = 3; bCountLoop < 7 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        pData ,
        pLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (*pLength != PHAL_T1T_READALL_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pData, wOutBufferSize-2, aCrc));
    if(aCrc[0] == pData[122] && aCrc[1] ==  pData[123] )
    {

        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_ReadByte  (
                                 phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                 uint8_t * pUid,                            /**< [In] */
                                 uint8_t   bAddress,                        /**< [In] */
                                 uint8_t * pData,                           /**< [Out] */
                                 uint16_t * pLength                         /**< [Out] */
                                 )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[10] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_READBYTE_RESP_LEN;                      /*Read response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 7;   /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    /*get RALL cmd and uid*/
    abTxBuffer[0] = PHAL_T1T_CMD_READBYTE;         /*cmd*/
    abTxBuffer[1] = bAddress;                      /*address*/
    abTxBuffer[2] = 0x00;                          /*data*/
    for( bCountLoop = 3; bCountLoop < 7 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize,
        pData ,
        pLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (*pLength != PHAL_T1T_READBYTE_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pData , wOutBufferSize-2 , aCrc));
    if(aCrc[0] == pData[2] && aCrc[1] ==  pData[3] )
    {
        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_WriteEraseByte  (
                                       phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                       uint8_t * pUid,                            /**< [In] */
                                       uint8_t   bAddress,                        /**< [In] */
                                       uint8_t   bTxData,                         /**< [In] */
                                       uint8_t * pRxData,                         /**< [Out] */
                                       uint16_t * pLength                         /**< [Out] */
                                       )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[10] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_WRITEERASEBYTE_RESP_LEN;                      /*Write_E response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 7;    /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    /*get RALL cmd and uid*/
    abTxBuffer[0] = PHAL_T1T_CMD_WRITEERASEBYTE;      /*cmd*/
    abTxBuffer[1] = bAddress;                         /*address*/
    abTxBuffer[2] = bTxData;                          /*data*/
    for( bCountLoop = 3; bCountLoop < 7 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        pRxData ,
        pLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (*pLength != PHAL_T1T_WRITEERASEBYTE_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pRxData, wOutBufferSize-2, aCrc));
    if(aCrc[0] == pRxData[2] && aCrc[1] ==  pRxData[3] )
    {
        if(bAddress != pRxData[0] || bTxData != pRxData[1])
        {
            return  PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
        }

        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_WriteNoEraseByte  (
    phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
    uint8_t * pUid,                            /**< [In] */
    uint8_t   bAddress,                        /**< [In] */
    uint8_t   bTxData,                         /**< [In] */
    uint8_t * pRxData,                         /**< [Out] */
    uint16_t * pLength                         /**< [Out] */
    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[10] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_WRITENOERASEBYTE_RESP_LEN;                     /*Write_NE response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 7;    /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    /*get RALL cmd and uid*/
    abTxBuffer[0] = PHAL_T1T_CMD_WRITENOERASEBYTE;   /*cmd*/
    abTxBuffer[1] = bAddress;                        /*address*/
    abTxBuffer[2] = bTxData;                         /*data*/
    for( bCountLoop = 3; bCountLoop < 7 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        pRxData ,
        pLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (*pLength != PHAL_T1T_WRITENOERASEBYTE_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pRxData, wOutBufferSize-2, aCrc));
    if(aCrc[0] == pRxData[2] && aCrc[1] ==  pRxData[3] )
    {
        if(bAddress != pRxData[0] || bTxData != pRxData[1])
        {
            return  PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
        }

        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_ReadSegment  (
                                    phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                    uint8_t * pUid,                            /**< [In] */
                                    uint8_t   bAddress,                        /**< [In] */
                                    uint8_t * pData,                           /**< [Out] */
                                    uint16_t * pLength                         /**< [Out] */
                                    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[14] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_READSEG_RESP_LEN;                    /*Rseg response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 14;    /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    abTxBuffer[0] = PHAL_T1T_CMD_READSEG;        /*cmd*/
    abTxBuffer[1] = bAddress ;                   /*address*/
    for( bCountLoop = 2 ; bCountLoop < 10 ; bCountLoop++)
        abTxBuffer[bCountLoop] = 0x00;               /*data*/

    for( ; bCountLoop < 14 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        pData ,
        pLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (*pLength != PHAL_T1T_READSEG_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pData, wOutBufferSize-2, aCrc));
    if(aCrc[0] == pData[129] && aCrc[1] ==  pData[130] )
    {

        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_ReadBlock  (
                                  phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                  uint8_t * pUid,                            /**< [In] */
                                  uint8_t   bAddress,                        /**< [In] */
                                  uint8_t * pData,                           /**< [Out] */
                                  uint16_t * pLength                         /**< [Out] */
                                  )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[14] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_READBLOCK_RESP_LEN;                    /*Read8 response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 14;    /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    abTxBuffer[0] = PHAL_T1T_CMD_READBLOCK;       /*cmd*/
    abTxBuffer[1] = bAddress ;                    /*address*/
    for( bCountLoop = 2 ; bCountLoop < 10 ; bCountLoop++)
        abTxBuffer[bCountLoop] = 0x00;                 /*data*/

    for( ; bCountLoop < 14 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        pData ,
        pLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (*pLength != PHAL_T1T_READBLOCK_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pData, wOutBufferSize-2, aCrc));
    if(aCrc[0] == pData[9] && aCrc[1] ==  pData[10] )
    {
        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_WriteEraseBlock  (
                                        phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                        uint8_t * pUid,                            /**< [In] */
                                        uint8_t   bAddress,                        /**< [In] */
                                        uint8_t * pTxData,                         /**< [In] */
                                        uint8_t * pRxData,                         /**< [Out] */
                                        uint16_t * pLength                         /**< [Out] */
                                        )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[14] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_WRITEERASEBLOCK_RESP_LEN;                 /*Write_E8 response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 14;    /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    abTxBuffer[0] = PHAL_T1T_CMD_WRITEERASEBLOCK;       /*cmd*/
    abTxBuffer[1] = bAddress;                           /*address*/
    for( bCountLoop = 2 ; bCountLoop < 10 ; bCountLoop++)
        abTxBuffer[bCountLoop] = pTxData[bCountLoop-2];     /*data*/

    for( ; bCountLoop < 14 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        pRxData ,
        pLength ,
        &bBitlen
        )
        );

    /* check received length */
    if (*pLength != PHAL_T1T_WRITEERASEBLOCK_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pRxData, wOutBufferSize-2, aCrc));
    if(aCrc[0] == pRxData[9] && aCrc[1] ==  pRxData[10] )
    {
        /*verify Address of the recieved data*/
        if (bAddress != pRxData[0])
        {
            return  PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
        }

        /*verify the data written */
        for( bCountLoop = 1 ; bCountLoop < 9 ; bCountLoop++)
        {
            if (pTxData[bCountLoop-1] != pRxData[bCountLoop])
            {
                return  PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
            }
        }

        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

phStatus_t phalT1T_Sw_WriteNoEraseBlock  (
    phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
    uint8_t * pUid,                            /**< [In] */
    uint8_t   bAddress,                        /**< [In] */
    uint8_t * pTxData,                         /**< [In] */
    uint8_t * pRxData,                         /**< [Out] */
    uint16_t * pLength                         /**< [Out] */
    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp ;
    uint8_t     PH_MEMLOC_REM bCountLoop;
    uint8_t     PH_MEMLOC_REM abTxBuffer[14] ;
    uint8_t *   PH_MEMLOC_REM pRxBuffer ;
    uint16_t    PH_MEMLOC_REM wRxLength ;
    uint8_t     PH_MEMLOC_REM bBitlen ;
    uint8_t     PH_MEMLOC_REM aCrc[2] ;
    uint16_t    PH_MEMLOC_REM wOutBufferSize = PHAL_T1T_WRITENOERASEBLOCK_RESP_LEN;                   /*Write_NE8 response len*/
    uint16_t    PH_MEMLOC_REM wInBufferLength ;
    uint8_t     PH_MEMLOC_REM bInBufferBits = (uint8_t)(wOutBufferSize % 8);
    uint16_t    PH_MEMLOC_REM wTxLength = 14;    /* Command sequence len without Crc_B */

    wInBufferLength =  (wOutBufferSize) + (wOutBufferSize/8) + 1 ;
    abTxBuffer[0] = PHAL_T1T_CMD_WRITENOERASEBLOCK;     /*cmd*/
    abTxBuffer[1] = bAddress;                           /*address*/
    for( bCountLoop = 2 ; bCountLoop < 10 ; bCountLoop++)
        abTxBuffer[bCountLoop] = pTxData[bCountLoop-2];     /*data*/

    for( ; bCountLoop < 14 ; bCountLoop++ )
    {
        abTxBuffer[bCountLoop] = (*pUid);
        pUid++;
    }

    /*send and receive the data*/
    statusTmp = phpalI14443p3a_T1T_Exchange(
        pDataParams->pPalI14443p3aDataParams ,
        PH_EXCHANGE_DEFAULT ,
        &abTxBuffer[0] ,
        wTxLength  ,
        &pRxBuffer ,
        &wRxLength
        );
    if ( statusTmp != PH_ERR_SUCCESS && ( statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_INCOMPLETE_BYTE)
    {
        return statusTmp ;
    }

    /*remove parity of the recieved data-as harware's parity is off*/
    PH_CHECK_SUCCESS_FCT(statusTmp,
        phTools_DecodeParity(
        PH_TOOLS_PARITY_OPTION_ODD ,
        pRxBuffer ,
        wInBufferLength ,
        bInBufferBits ,
        wOutBufferSize ,
        pRxData ,
        pLength ,
        &bBitlen
        )
        );
    /* check received length */
    if (*pLength != PHAL_T1T_WRITENOERASEBLOCK_RESP_LEN)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
    }

    /*verify Crc_B for the recieved data*/
    PH_CHECK_SUCCESS_FCT(statusTmp,phTools_ComputeCrc_B( pRxData, wOutBufferSize-2, aCrc));
    if(aCrc[0] == pRxData[9] && aCrc[1] ==  pRxData[10] )
    {
        /*verify Address of the recieved data*/
        if (bAddress != pRxData[0])
        {
            return  PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
        }

        /*verify the data written */
        for( bCountLoop = 1 ; bCountLoop < 9 ; bCountLoop++)
        {
            if (pTxData[bCountLoop-1] != pRxData[bCountLoop])
            {
                return  PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_T1T);
            }
        }

        return  PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
    }
    else
    {
        return  PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_T1T);
    }
}

#ifdef __DEBUG
/* This API would be available only for debugging purpose */
phStatus_t phalT1T_Sw_SetPtr(
                             phalT1T_Sw_DataParams_t * pDataParams,
                             void * pI14443p3a
                             )
{
    pDataParams->pPalI14443p3aDataParams = pI14443p3a;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_T1T);
}
#endif /* __DEBUG */

#endif /* NXPBUILD__PHAL_T1T_SW */

