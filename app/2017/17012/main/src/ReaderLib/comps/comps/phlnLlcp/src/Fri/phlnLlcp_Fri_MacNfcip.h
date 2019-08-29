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
* \file  phlnLlcp_Fri_MacNfcip.h
* \brief NFC LLCP MAC Mapping for NFCIP.
*
* $Author: nxp40786 $
* $Revision: 452 $
* $Date: 2013-12-19 20:09:00 +0530 (Thu, 19 Dec 2013) $
*
*/

#ifndef PHLNLLCP_FRI_MACNFCIP_H
#define PHLNLLCP_FRI_MACNFCIP_H

#include <ph_Status.h>
#include <phlnLlcp.h>

/** 
* \brief Internal definitions.
* @{
*/

/**
* \brief Register MacNFCip.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phlnLlcp_Fri_MacNfcip_Register( phlnLlcp_Fri_Mac_t  *LlcpMac );

#endif /* PHLNLLCP_FRI_MACNFCIP_H */
