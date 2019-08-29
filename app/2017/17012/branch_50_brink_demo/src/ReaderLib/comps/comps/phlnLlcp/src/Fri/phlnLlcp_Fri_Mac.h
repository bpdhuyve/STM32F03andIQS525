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
* \file  phlnLlcp_Fri_Mac.h
* \brief NFC LLCP MAC Mappings For Different RF Technologies.
*
* $Author: nxp40786 $
* $Revision: 452 $
* $Date: 2013-12-19 20:09:00 +0530 (Thu, 19 Dec 2013) $
*
*/
#ifndef PHLNLLCP_FRI_MAC_H
#define PHLNLLCP_FRI_MAC_H


/*include files*/
#include <ph_Status.h>
#include <phlnLlcp.h>

/**
* \name LLCP MAC Mapping
*
*/


/**
*
*  This component implements the different MAC mapping for a Logical Link Control Protocol communication,
*  as defined by the NFC Forum LLCP specifications.\n
*  The MAC component handles the mapping for the different technologies supported by LLCP
*  This component provides an API to the upper layer with the following features:\n\n
*  - Reset the MAC mapping component
*  - Check the LLCP Compliancy
*  - Activate the LLCP link
*  - Deactivate the LLCP link
*  - Register the MAC component Interface with a specific technologie (NFCIP/ISO14443)
*  - Send packets through the LLCP link
*  - Receive packets through the LLCP link
*
*/


/*
################################################################################
********************** MAC Interface Function Prototype  ***********************
################################################################################
*/

/**
*
* \brief
*/
phStatus_t phlnLlcp_Fri_Mac_Reset (phlnLlcp_Fri_Mac_t                 *LlcpMac,
                                   void                               *LowerDevice,
                                   phlnLlcp_Fri_Mac_LinkStatus_CB_t    LinkStatus_Cb,
                                   void                               *pContext);
/**
*
* \brief
*/
phStatus_t phlnLlcp_Fri_Mac_ChkLlcp (phlnLlcp_Fri_Mac_t               *LlcpMac,
                                     phHal_sRemoteDevInformation_t    *psRemoteDevInfo,
                                     phlnLlcp_Fri_Mac_Chk_CB_t         ChkLlcpMac_Cb,
                                     void                             *pContext);
/**
*
* \brief
*/
phStatus_t phlnLlcp_Fri_Mac_Activate (phlnLlcp_Fri_Mac_t   *LlcpMac);

/**
*
* \brief
*/
phStatus_t phlnLlcp_Fri_Mac_Deactivate (phlnLlcp_Fri_Mac_t    *LlcpMac);

/**
*
* \brief
*/
phStatus_t phlnLlcp_Fri_Mac_Send( phlnLlcp_Fri_Mac_t          *LlcpMac,
                                 phNfc_sData_t                *psData,
                                 phlnLlcp_Fri_Mac_Send_CB_t    LlcpMacSend_Cb,
                                 void                         *pContext);

/**
*
* \brief
*/
phStatus_t phlnLlcp_Fri_Mac_Receive( phlnLlcp_Fri_Mac_t              *LlcpMac,    /* PRQA S 3208 */
                                    phNfc_sData_t                    *psData,
                                    phlnLlcp_Fri_Mac_Reveive_CB_t     ReceiveLlcpMac_Cb,
                                    void                             *pContext);

#endif /* PHLNLLCP_FRI_MAC_H */
