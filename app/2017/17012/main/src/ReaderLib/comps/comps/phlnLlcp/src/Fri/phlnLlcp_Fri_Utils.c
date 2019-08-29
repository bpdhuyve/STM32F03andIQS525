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
* \file  phlnLlcp_Fri_Utils.c
* \brief NFC LLCP core
*
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
*/
#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI

#include <phlnLlcp.h>
#include "phlnLlcp_Fri_Utils.h"
#include "phlnLlcp_Fri_Llcp.h"

phStatus_t phlnLlcp_Fri_Utils_DecodeTLV( phNfc_sData_t  *psRawData,
                                        uint32_t              *pOffset,
                                        uint8_t               *pType,
                                        phNfc_sData_t  *psValueBuffer )
{
   uint8_t type;
   uint8_t length;
   uint32_t offset = *pOffset;

   /* Check for NULL pointers */
   if ((psRawData == NULL) || (pOffset == NULL) || (pType == NULL) || (psValueBuffer == NULL))
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Check offset */
   if (offset > psRawData->length)
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Check if enough room for Type and Length (with overflow check) */
   if ((offset+2 > psRawData->length) && (offset+2 > offset))
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Get Type and Length from current TLV, and update offset */
   type = psRawData->buffer[offset];
   length = psRawData->buffer[offset+1];
   offset += 2;

   /* Check if enough room for Value with announced Length (with overflow check) */
   if ((offset+length > psRawData->length) && (offset+length > offset))
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Save response, and update offset */
   *pType = type;
   psValueBuffer->buffer = psRawData->buffer + offset;
   psValueBuffer->length = length;
   offset += length;

   /* Save updated offset */
   *pOffset = offset;

   return PH_ERR_SUCCESS;
}

phStatus_t phlnLlcp_Fri_Utils_EncodeTLV( phNfc_sData_t  *psValueBuffer,
                                   uint32_t       *pOffset,
                                   uint8_t        type,
                                   uint8_t        length,
                                   uint8_t        *pValue)
{
   uint32_t offset = *pOffset;
   uint32_t finalOffset = offset + 2 + length; /* 2 stands for Type and Length fields size */
   uint8_t i;

   /* Check for NULL pointers */
   if ((psValueBuffer == NULL) || (pOffset == NULL) || (pValue == NULL))
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Check offset */
   if (offset > psValueBuffer->length)
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Check if enough room for Type, Length and Value (with overflow check) */
   if ((finalOffset > psValueBuffer->length) || (finalOffset < offset))
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Set the TYPE */
   psValueBuffer->buffer[offset] = type;
   offset += 1;

   /* Set the LENGTH */
   psValueBuffer->buffer[offset] = length;
   offset += 1;

   /* Set the VALUE */
   for(i=0;i<length;i++,offset++)
   {
      psValueBuffer->buffer[offset]  = pValue[i];
   }

   /* Save updated offset */
   *pOffset = offset;

   return PH_ERR_SUCCESS;
}

phStatus_t phlnLlcp_Fri_Utils_AppendTLV( phNfc_sData_t  *psValueBuffer,
                                   uint32_t       nTlvOffset,
                                   uint32_t       *pCurrentOffset,
                                   uint8_t        length,
                                   uint8_t        *pValue)
{
   uint32_t offset = *pCurrentOffset;
   uint32_t finalOffset = offset + length;

   /* Check for NULL pointers */
   if ((psValueBuffer == NULL) || (pCurrentOffset == NULL) || (pValue == NULL))
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Check offset */
   if (offset > psValueBuffer->length)
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Check if enough room for Type and Length (with overflow check) */
   if ((finalOffset > psValueBuffer->length) || (finalOffset < offset))
   {
      return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP);
   }

   /* Update the LENGTH */
   psValueBuffer->buffer[nTlvOffset+1] += length;

   /* Set the VALUE */
   memcpy(psValueBuffer->buffer + offset, pValue, length);  /* PRQA S 3200 */
   offset += length;

   /* Save updated offset */
   *pCurrentOffset = offset;

   return PH_ERR_SUCCESS;
}

/* TODO: comment function EncodeRW */
void phlnLlcp_Fri_Utils_EncodeRW(uint8_t *pRw)
{
   /* MASK */
   *pRw = *pRw & PHLNLLCP_FRI_TLV_RW_MASK;
}

/**
* Initializes a Fifo Cyclic Buffer to point to some allocated memory.
*/
void phlnLlcp_Fri_Utils_CyclicFifoInit(pphlnLlcp_Fri_Util_Fifo_Buffer   pUtilFifo,
                                      uint8_t                    *pBuffStart,
                                      uint32_t                         buffLength)
{
   pUtilFifo->pBuffStart = pBuffStart;
   pUtilFifo->pBuffEnd   = pBuffStart + buffLength-1;
   pUtilFifo->pIn        = pBuffStart;
   pUtilFifo->pOut       = pBuffStart;
   pUtilFifo->bFull      = false;
}

/**
* Clears the Fifo Cyclic Buffer - loosing any data that was in it.
*/
void phlnLlcp_Fri_Utils_CyclicFifoClear(pphlnLlcp_Fri_Util_Fifo_Buffer pUtilFifo)
{
   pUtilFifo->pIn = pUtilFifo->pBuffStart;
   pUtilFifo->pOut = pUtilFifo->pBuffStart;
   pUtilFifo->bFull      = false;
}

/**
* Attempts to write dataLength bytes to the specified Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoWrite(pphlnLlcp_Fri_Util_Fifo_Buffer  pUtilFifo,
                                      uint8_t                         *pData,
                                      uint32_t                        dataLength)
{
   uint32_t dataLengthWritten = 0;
   volatile uint8_t * pNext;

   while(dataLengthWritten < dataLength)
   {
      pNext = pUtilFifo->pIn+1;

      if(pNext > pUtilFifo->pBuffEnd)
      {
         //Wrap around
         pNext = pUtilFifo->pBuffStart;
      }

      if(pUtilFifo->bFull)
      {
         //Full
         break;
      }

      if(pNext == pUtilFifo->pOut)
      {
         // Trigger Full flag
         pUtilFifo->bFull = true;
      }

      dataLengthWritten++;
      *pNext = *pData++;
      pUtilFifo->pIn = pNext;
   }

   return dataLengthWritten;
}

/**
* Attempts to read dataLength bytes from the specified  Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoFifoRead( pphlnLlcp_Fri_Util_Fifo_Buffer   pUtilFifo,
                                               uint8_t                          *pBuffer,
                                               uint32_t                         dataLength)
{
   uint32_t  dataLengthRead = 0;
    volatile uint8_t *pNext;

   while(dataLengthRead < dataLength)
   {
      if((pUtilFifo->pOut == pUtilFifo->pIn) && (pUtilFifo->bFull == false))
      {
         //No more bytes in buffer
         break;
      }
      else
      {
         dataLengthRead++;

         if(pUtilFifo->pOut == pUtilFifo->pBuffEnd)
         {
            /* Wrap around */
            pNext = pUtilFifo->pBuffStart;
         }
         else
         {
            pNext = pUtilFifo->pOut + 1;
         }

         *pBuffer++ = *pNext;

         pUtilFifo->pOut = pNext;

         pUtilFifo->bFull = false;
      }
   }

   return dataLengthRead;
}

/**
* Returns the number of bytes currently stored in Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoUsage(pphlnLlcp_Fri_Util_Fifo_Buffer pUtilFifo)
{
   uint32_t dataLength;
    volatile uint8_t *pIn   = pUtilFifo->pIn;
    volatile uint8_t *pOut  = pUtilFifo->pOut;

   if (pUtilFifo->bFull)
   {
      dataLength = (uint32_t)(pUtilFifo->pBuffEnd - pUtilFifo->pBuffStart + 1);
   }
   else
   {
      if(pIn >= pOut)
      {
         dataLength = (uint32_t)(pIn - pOut);
      }
      else
      {
         dataLength = (uint32_t)(pUtilFifo->pBuffEnd - pOut);
         dataLength += (uint32_t)((pIn+1) - pUtilFifo->pBuffStart);
      }
   }

   return dataLength;
}


/**
* Returns the available room for writing in Fifo Cyclic Buffer.
*/
uint32_t phlnLlcp_Fri_Utils_CyclicFifoAvailable(pphlnLlcp_Fri_Util_Fifo_Buffer pUtilFifo)
{
   uint32_t dataLength;
   uint32_t  size;
    volatile uint8_t  *pIn  = pUtilFifo->pIn;
    volatile uint8_t  *pOut = pUtilFifo->pOut;

   if (pUtilFifo->bFull)
   {
      dataLength = 0;
   }
   else
   {
      if(pIn >= pOut)
      {
         size = (uint32_t)(pUtilFifo->pBuffEnd - pUtilFifo->pBuffStart + 1);
         dataLength = size - (uint32_t)(pIn - pOut);
      }
      else
      {
         dataLength = (uint32_t)(pOut - pIn);
      }
   }

   return dataLength;
}

uint32_t phlnLlcp_Fri_Utils_Header2Buffer( phlnLlcp_Fri_sPacketHeader_t   *psHeader,
                                          uint8_t                         *pBuffer,
                                          uint32_t                         nOffset )
{
   uint32_t nOriginalOffset = nOffset;
   pBuffer[nOffset++] = (uint8_t)((psHeader->dsap << 2)  | (psHeader->ptype >> 2));
   pBuffer[nOffset++] = (uint8_t)((psHeader->ptype << 6) | psHeader->ssap);
   return nOffset - nOriginalOffset;
}

uint32_t phlnLlcp_Fri_Utils_Sequence2Buffer( phlnLlcp_Fri_sPacketSequence_t  *psSequence,
                                            uint8_t                          *pBuffer,
                                            uint32_t                          nOffset )
{
   uint32_t nOriginalOffset = nOffset;
   pBuffer[nOffset++] = (uint8_t)((psSequence->ns << 4) | (psSequence->nr));
   return nOffset - nOriginalOffset;
}

uint32_t phlnLlcp_Fri_Utils_Buffer2Header( uint8_t                      *pBuffer,
                                          uint32_t                       nOffset,
                                          phlnLlcp_Fri_sPacketHeader_t  *psHeader )
{
   psHeader->dsap  = (uint8_t)((pBuffer[nOffset] & 0xFC) >> 2);
   psHeader->ptype = (uint8_t)(((pBuffer[nOffset]  & 0x03) << 2) | ((pBuffer[nOffset+1] & 0xC0) >> 6));
   psHeader->ssap  = pBuffer[nOffset+1] & 0x3F;
   return PHLNLLCP_FRI_PACKET_HEADER_SIZE;
}

uint32_t phlnLlcp_Fri_Utils_Buffer2Sequence( uint8_t                        *pBuffer,
                                            uint32_t                         nOffset,
                                            phlnLlcp_Fri_sPacketSequence_t  *psSequence )
{
   psSequence->ns = pBuffer[nOffset] >> 4;
   psSequence->nr = pBuffer[nOffset] & 0x0F;
   return PHLNLLCP_FRI_PACKET_SEQUENCE_SIZE;
}
#endif /* NXPBUILD__PHLN_LLCP_FRI */

