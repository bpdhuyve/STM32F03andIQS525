/*
*                    Copyright (c), NXP Semiconductors
*
*                       (C)NXP Electronics N.V.2005
*         All rights are reserved. Reproduction in whole or in part is
*        prohibited without the written consent of the copyright owner.
*    NXP reserves the right to make changes without notice at any time.
*   NXP makes no warranty, expressed, implied or statutory, including but
*   not limited to any implied warranty of merchantability or fitness for any
*  particular purpose, or that the use will not infringe any third party patent,
*   copyright or trademark. NXP must not be liable for any loss or damage
*                            arising from its use.
*
*/

/**
* \file  phlnLlcp_Fri_OvrHal.h
* \brief Overlapped HAL
*
*
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
* Changed by: $Author: nxp62726 $
* $Revision: 416 $
* $Aliases:  $
*
*/

#ifndef PHLNLLCP_FRI_OVRHAL_H
#define PHLNLLCP_FRI_OVRHAL_H


#include <ph_Status.h>
#include <phlnLlcp.h>

/**
*  \name Overlapped HAL
*
* File: \ref phlnLlcp_Fri_OvrHal.h
*
*/
/*@{*/

void phlnLlcp_Fri_OvrHal_Init(void);
void phlnLlcp_Fri_OvrHal_DeInit(void);

/**
* 
*
* \brief Transceive Data to/from a Remote Device
*
* \param[in]      OvrHal               Component Context.
* \param[in]      CompletionInfo       \copydoc phFriNfc_OvrHal_t::TemporaryCompletionInfo
* \param[in,out]  RemoteDevInfo        Remote Device Information.
* \param[in]      Cmd                  Command to perform.
* \param[out]     DepAdditionalInfo    Protocol Information.
* \param[in]      SendBuf              Pointer to the data to send.
* \param[in]      SendLength           Length, in bytes, of the Send Buffer.
* \param[out]     RecvBuf              Pointer to the buffer that receives the data.
* \param[in,out]  RecvLength           Length, in bytes, of the received data.
*
*
*/

phStatus_t phlnLlcp_Fri_OvrHal_Transceive(void                          *OvrHal,
                                          phlnLlcp_Fri_CplRt_t           *CompletionInfo,
                                          uint8_t                        *SendBuf,
                                          uint16_t                        SendLength,
                                          uint8_t                       **RecvBuf,
                                          uint16_t                       *RecvLength);

/**
* 
*
* \brief TODO
*
*/
phStatus_t phlnLlcp_Fri_OvrHal_Receive(void                             *OvrHal,
                                       phlnLlcp_Fri_CplRt_t              *CompletionInfo,
                                       uint8_t                          **RecvBuf,
                                       uint16_t                          *RecvLength);

/**
* 
*
* \brief TODO
*
*/
phStatus_t phlnLlcp_Fri_OvrHal_Send(void                    *OvrHal,
                                    phlnLlcp_Fri_CplRt_t    *CompletionInfo,
                                    uint8_t                 *SendBuf,
                                    uint16_t                 SendLength);

void phlnLlcp_Fri_StartTicks(void   *pContext);

void phlnLlcp_Fri_StopTicks(void   *pContext);

void phlnLlcp_Fri_OvrHal_Task(uint32_t timerId, void    *pOvrHalTask);

/** @}
* end of phlnLlcp_Fri_OvrHal group
*/

#endif /* PHLNLLCP_FRI_OVRHAL_H */
