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
* \file  phlnLlcp_Fri_Mac.c
* \brief NFC LLCP MAC Mappings For Different RF Technologies.
*
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP_FRI

/*include files*/
#include "phlnLlcp_Fri_Mac.h"
#include "phlnLlcp_Fri_MacNfcip.h"
#include <phpalI18092mPI.h>
#include <phpalI18092mT.h>


phStatus_t phlnLlcp_Fri_Mac_Reset (phlnLlcp_Fri_Mac_t                 *LlcpMac,
                                   void                               *LowerDevice,
                                   phlnLlcp_Fri_Mac_LinkStatus_CB_t    LinkStatus_Cb,
                                   void                               *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    /* Store the Linkstatus callback function of the upper layer */
    LlcpMac->LinkStatus_Cb = LinkStatus_Cb;

    /* Store a pointer to the upper layer context */
    LlcpMac->LinkStatus_Context = pContext;

    /* Set the LinkStatus variable to the default state */
    LlcpMac->LinkState = phlnLlcp_Fri_Mac_eLinkDefault;

    /* Store a pointer to the lower layer */
    LlcpMac->LowerDevice =  LowerDevice; 

    LlcpMac->psRemoteDevInfo         = NULL;
    LlcpMac->PeerRemoteDevType       = phlnLlcp_Fri_Mac_ePeerTypeInitiator;
    LlcpMac->MacReceive_Cb           = NULL;
    LlcpMac->MacSend_Cb              = NULL;
    LlcpMac->psSendBuffer            = NULL;
    LlcpMac->RecvPending             = 0;
    LlcpMac->SendPending             = 0;

    return status;
}

phStatus_t phlnLlcp_Fri_Mac_ChkLlcp (phlnLlcp_Fri_Mac_t               *LlcpMac,
                                     phHal_sRemoteDevInformation_t    *psRemoteDevInfo,
                                     phlnLlcp_Fri_Mac_Chk_CB_t         ChkLlcpMac_Cb,
                                     void                             *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;
    PH_ASSERT_NULL(LlcpMac);
    PH_ASSERT_NULL(psRemoteDevInfo);

    /* Store the Remote Device info received from Device Discovery  */
    LlcpMac->psRemoteDevInfo = psRemoteDevInfo;

    if(LlcpMac->psRemoteDevInfo->RemDevType == PHLNLLCP_FRI_HAL_ENFCIP1_INITIATOR)
    {
        /* Set the PeerRemoteDevType variable to the Target type */
        LlcpMac->PeerRemoteDevType = phlnLlcp_Fri_Mac_ePeerTypeTarget;
    }
    else if(LlcpMac->psRemoteDevInfo->RemDevType == PHLNLLCP_FRI_HAL_ENFCIP1_TARGET)
    {
        /* Set the PeerRemoteDevType variable to the Initiator type */
        LlcpMac->PeerRemoteDevType = phlnLlcp_Fri_Mac_ePeerTypeInitiator;
    }
    else
    {
        /* Nothing to Do */
    }
    switch(LlcpMac->psRemoteDevInfo->RemDevType)
    {
    case PHLNLLCP_FRI_HAL_ENFCIP1_INITIATOR:
    case PHLNLLCP_FRI_HAL_ENFCIP1_TARGET:
        {
            /* Register the lower layer to the MAC mapping component */
            status = phlnLlcp_Fri_MacNfcip_Register (LlcpMac);
            if(status == PH_ERR_SUCCESS)
            {
                status  = LlcpMac->LlcpMacInterface.chk(LlcpMac,ChkLlcpMac_Cb,pContext);
            }
            else
            {
                status = PH_ADD_COMPCODE(PH_ERR_FAILED, CID_FRI_NFC_LLCP_MAC);
            }
        }break;
    default:
        {
            status = PH_ADD_COMPCODE(PH_ERR_INVALID_DEVICE, CID_FRI_NFC_LLCP_MAC);
        }break;
    }

    if(status == PH_ERR_SUCCESS)
    {
        if(LlcpMac->psRemoteDevInfo->RemDevType == PHLNLLCP_FRI_HAL_ENFCIP1_INITIATOR)
        {
            status = phpalI18092mT_SetConfig(LlcpMac->LowerDevice,PHPAL_I18092MT_CONFIG_RTOX,0);
        }
        else if(LlcpMac->psRemoteDevInfo->RemDevType == PHLNLLCP_FRI_HAL_ENFCIP1_TARGET)
        {
            status = phpalI18092mPI_SetConfig(LlcpMac->LowerDevice, PHPAL_I18092MPI_CONFIG_ATN,0);
        }
        else
        {
            /* Nothing to Do */
        }
    }

    return status;
}

phStatus_t phlnLlcp_Fri_Mac_Activate (phlnLlcp_Fri_Mac_t   *LlcpMac)
{
    phStatus_t status = PH_ERR_SUCCESS;

    if(LlcpMac->LlcpMacInterface.activate == NULL)
    {  
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        status = LlcpMac->LlcpMacInterface.activate(LlcpMac);
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Mac_Deactivate (phlnLlcp_Fri_Mac_t   *LlcpMac)
{
    phStatus_t status = PH_ERR_SUCCESS;
    if(LlcpMac->LlcpMacInterface.deactivate == NULL)
    {  
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        status = LlcpMac->LlcpMacInterface.deactivate(LlcpMac);
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Mac_Send( phlnLlcp_Fri_Mac_t          *LlcpMac,
                                 phNfc_sData_t                *psData,
                                 phlnLlcp_Fri_Mac_Send_CB_t    LlcpMacSend_Cb,
                                 void                         *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    if(NULL== LlcpMac->LlcpMacInterface.send || NULL==psData || NULL==LlcpMacSend_Cb || NULL==pContext)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        status = LlcpMac->LlcpMacInterface.send(LlcpMac,psData,LlcpMacSend_Cb,pContext);
    }
    return status;
}

phStatus_t phlnLlcp_Fri_Mac_Receive (phlnLlcp_Fri_Mac_t               *LlcpMac,
                                     phNfc_sData_t                    *psData,
                                     phlnLlcp_Fri_Mac_Reveive_CB_t     ReceiveLlcpMac_Cb,
                                     void                             *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    if(LlcpMac->LlcpMacInterface.receive == NULL || NULL==psData || NULL==ReceiveLlcpMac_Cb || NULL==pContext)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, CID_FRI_NFC_LLCP_MAC);
    }
    else
    {
        status = LlcpMac->LlcpMacInterface.receive(LlcpMac,psData,ReceiveLlcpMac_Cb,pContext);
    }
    return status;

}
#endif /* NXPBUILD__PHLN_LLCP_FRI */

