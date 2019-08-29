/*
*                  Copyright (c), NXP Semiconductors
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

/**
* \file  phlnLlcp_Fri_Utils.h
* \brief NFC LLCP utils
*
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
*/

#ifndef PHLNLLCP_FRI_UTILS_H
#define PHLNLLCP_FRI_UTILS_H

/*include files*/

#include <ph_Status.h>
#include <phlnLlcp.h>

/** 
* NFC Forum Logical Link Control Protocol Utils
*
* File: \ref phlnLlcp_Fri_Utils.h
*
*/

phStatus_t phlnLlcp_Fri_Utils_DecodeTLV( phNfc_sData_t  *psRawData,
                                         uint32_t       *pOffset,
                                         uint8_t        *pType,
                                         phNfc_sData_t  *psValueBuffer );

phStatus_t phlnLlcp_Fri_Utils_EncodeTLV( phNfc_sData_t  *psValueBuffer,
                                   uint32_t       *pOffset,
                                   uint8_t        type,
                                   uint8_t        length,
                                   uint8_t        *pValue);

phStatus_t phlnLlcp_Fri_Utils_AppendTLV( phNfc_sData_t  *psValueBuffer,
                                   uint32_t       nTlvOffset,
                                   uint32_t       *pCurrentOffset,
                                   uint8_t        length,
                                   uint8_t        *pValue);

void phlnLlcp_Fri_Utils_EncodeRW(uint8_t *pRw);

/**
* Initializes a Fifo Cyclic Buffer to point to some allocated memory.
*/
void phlnLlcp_Fri_Utils_CyclicFifoInit(pphlnLlcp_Fri_Util_Fifo_Buffer     sUtilFifo,
                                       uint8_t                     *pBuffStart,
                                       uint32_t                          buffLength);

/**
* Clears the Fifo Cyclic Buffer - loosing any data that was in it.
*/
void phlnLlcp_Fri_Utils_CyclicFifoClear(pphlnLlcp_Fri_Util_Fifo_Buffer sUtilFifo);


/**
* Attempts to write dataLength bytes to the specified Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoWrite(pphlnLlcp_Fri_Util_Fifo_Buffer     sUtilFifo,
                                       uint8_t              *pData,
                                       uint32_t             dataLength);

/**
* Attempts to read dataLength bytes from the specified  Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoFifoRead(pphlnLlcp_Fri_Util_Fifo_Buffer     sUtilFifo,
                                               uint8_t                           *pBuffer,
                                               uint32_t                          dataLength);

/**
* Returns the number of bytes currently stored in Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoUsage(pphlnLlcp_Fri_Util_Fifo_Buffer sUtilFifo);

/**
* Returns the available room for writing in Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoAvailable(pphlnLlcp_Fri_Util_Fifo_Buffer sUtilFifo);

uint32_t phlnLlcp_Fri_Utils_Header2Buffer( phlnLlcp_Fri_sPacketHeader_t *psHeader,
                                           uint8_t *pBuffer,
                                           uint32_t nOffset );

uint32_t phlnLlcp_Fri_Utils_Sequence2Buffer( phlnLlcp_Fri_sPacketSequence_t *psSequence,
                                             uint8_t *pBuffer,
                                             uint32_t nOffset );

uint32_t phlnLlcp_Fri_Utils_Buffer2Header( uint8_t *pBuffer,                                                                  /* PRQA S 3209 */
                                           uint32_t nOffset,
                                           phlnLlcp_Fri_sPacketHeader_t *psHeader );

uint32_t phlnLlcp_Fri_Utils_Buffer2Sequence( uint8_t *pBuffer,                                                                 /* PRQA S 3209 */
                                             uint32_t nOffset,
                                             phlnLlcp_Fri_sPacketSequence_t *psSequence );

#endif /* PHLNLLP_FRI_UTILS_H */
