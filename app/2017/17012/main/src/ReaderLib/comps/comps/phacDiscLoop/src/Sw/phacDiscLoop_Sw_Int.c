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
* Internal functions for Discovery Loop Activities Component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 452 $
* $Date: 2013-12-19 20:09:00 +0530 (Thu, 19 Dec 2013) $
*
* History:
*  PC: Generated 23. Aug 2012
*
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phacDiscLoop.h>
#include <phpalI14443p3a.h>
#include <phpalI14443p4a.h>
#include <phhalHw.h>
#include <phpalFelica.h>
#include <phpalI14443p3b.h>
#include <phpalI18092mPI.h>
#include <phalT1T.h>


#ifdef NXPBUILD__PHAC_DISCLOOP_SW
#include "phacDiscLoop_Sw_Int.h"

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS
phStatus_t phacDiscLoop_Sw_Int_DetectA(
    phacDiscLoop_Sw_DataParams_t * pDataParams
    )
{
    uint8_t    PH_MEMLOC_REM bLoop; 
    uint8_t    PH_MEMLOC_REM aAtqa[2];
    phStatus_t PH_MEMLOC_REM status;
    uint16_t   PH_MEMLOC_REM wLength;
    uint8_t    PH_MEMLOC_REM aUid[4];
    
    bLoop = 0;

    pDataParams->sTypeATargetInfo.bTotalTagsFound_A = 0;
    status = PH_ERR_INTERNAL_ERROR;
    
    while (bLoop < pDataParams->sTypeATargetInfo.bLoopLimit_A)
    {
        status = phpalI14443p3a_RequestA(
            pDataParams->pPal1443p3aDataParams, 
            aAtqa
            );

        if ((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
        {
            /* Multiple type A cards in field */
            memcpy(    pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa, aAtqa, 2);  /* PRQA S 3200 */
            break;
        }
        else if (status == PH_ERR_SUCCESS)
        {
            /* Only one card in the filed */
            memcpy(pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa, aAtqa, 2);  /* PRQA S 3200 */
            if ((aAtqa[0] & PHAC_DISCLOOP_TYPEA_TYPE1_TAG_ATQA_BYTE1_CONFIG_MASK) == 0x00 &&
               ((aAtqa[1] & PHAC_DISCLOOP_TYPEA_TYPE1_TAG_ATQA_BYTE2_CONFIG_MASK) == 0x0C))
            {
                /* Detected type 1 tag */
                pDataParams->wTagsFound |=  PHAC_DISCLOOP_TYPEA_DETECTED_TAG_TYPE1;
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS               
                /* This is a Jewel tag, Send RID */
                status = phalT1T_ReadUID(
                    pDataParams->pAlT1TDataParams,
                    aUid,
                    &wLength
                    );
                if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
                {
                    memcpy(pDataParams->sTypeATargetInfo.sType_Jewel.aUid, aUid, 4);  /* PRQA S 3200 */
                    memcpy(pDataParams->sTypeATargetInfo.sType_Jewel.abHr, ((phalT1T_Sw_DataParams_t *)(pDataParams->pAlT1TDataParams))->abHR, 2);  /* PRQA S 3200 */
                }
                else
                {
                    /* Set collision pending flag */
                }
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS */
                
            }
            break;
        }
        else if (status != PH_ERR_IO_TIMEOUT)
        {
            /* This is an error condition that we cannot handle */
            return status;
        }
        else
        {
            /* Nothing to Do */
        }
        bLoop++;
    }
    
    if (((status & PH_ERR_MASK) != PH_ERR_COLLISION_ERROR) 
        && status != PH_ERR_SUCCESS)
    {
        /* No card detected in field, return with "No card found" message */ 
        return PH_ADD_COMPCODE(
            PHAC_DISCLOOP_ERR_TYPEA_NO_TAG_FOUND, 
            PH_COMP_AC_DISCLOOP
            );
    }
    else
    {
        /* One or more Type A tag detected in field */
        pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEA_DETECTED;
    }
    
    return status;
}

phStatus_t phacDiscLoop_Sw_Int_CollisionResolutionA(
    phacDiscLoop_Sw_DataParams_t * pDataParams
    )
{
    uint8_t    PH_MEMLOC_REM bTypeANvbUid;                   /* NVB UID */
    uint8_t    PH_MEMLOC_REM aTypeAUid[7];                   /* UID for Type A */
    uint8_t    PH_MEMLOC_REM bLoop;
    uint8_t    PH_MEMLOC_REM aTypeAUidComplete[10] = {0};
    uint8_t    PH_MEMLOC_REM aTypeAUidLen;
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint8_t    PH_MEMLOC_REM bailOutFlag;
    uint8_t    PH_MEMLOC_REM bTagPlat;
    uint8_t    PH_MEMLOC_REM bTypeATagIdx;                    /* Type a Tag index */
    uint8_t    PH_MEMLOC_REM aCascadeCodes[] = { 
        PHPAL_I14443P3A_CASCADE_LEVEL_1, 
        PHPAL_I14443P3A_CASCADE_LEVEL_2,     
        PHPAL_I14443P3A_CASCADE_LEVEL_3
    };
    uint8_t    PH_MEMLOC_REM bCascadeCodeIdx;
    uint8_t    PH_MEMLOC_REM bTypeAMoreCrads;                /* More than 1 type A 
                                                                cards available in field */
    uint8_t    PH_MEMLOC_REM aAtqa[2];

    bLoop = 0;
    bTypeATagIdx = 0;
    
    while (bLoop < pDataParams->sTypeATargetInfo.bDeviceLimit_A)
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P3_TAGS
        /* Anticollision stage */
        if (bTypeATagIdx < PHAC_DISCLOOP_TYPEA_I3P3_NR)
        {
            aTypeAUidLen = 0;
            bTypeAMoreCrads = 0;
            bailOutFlag = 0;
            /* Go through all cascade levels */
            for (bCascadeCodeIdx=0; bCascadeCodeIdx<3; bCascadeCodeIdx++)
            {
                bTypeANvbUid = 0;
                
                /* Anticollision loop */
                while (bTypeANvbUid != 0x40)
                {
                    status = phpalI14443p3a_Anticollision(
                        pDataParams->pPal1443p3aDataParams,
                        aCascadeCodes[bCascadeCodeIdx],      /* Cascade code */
                        aTypeAUid,                           /* Input UID */
                        bTypeANvbUid,                        /* UID len = 0 */
                        aTypeAUid,                           /* UID out */
                        &bTypeANvbUid                        /* UID out size */
                        );
       
                    if ((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
                    {
        
                        /* Collision has occurred */
                        bTypeAMoreCrads = 1;
                        if ((bTypeANvbUid & 0x07) < 7)
                        {
                            bTypeANvbUid++;
                        }
                        /* This is more difficult if we have to 
                           increment the bytecount */
                        else
                        {
                            bTypeANvbUid = (uint8_t)((((bTypeANvbUid & 0xF0) >> 4) + 1) << 4);
                        }
                    }
                    else if (status == PH_ERR_SUCCESS)
                    {
                        /* Copy the partial UID */
                        if (bCascadeCodeIdx == 0)
                        {
                            if (bTypeANvbUid != 0x40)
                            {
                                /* cascade level 1, last 3 bytes are valid */
                                memcpy(aTypeAUidComplete, &aTypeAUid[1], 3);  /* PRQA S 3200 */   
                                aTypeAUidLen += 3;
                            }
                            else
                            {
                                memcpy(&aTypeAUidComplete[aTypeAUidLen], aTypeAUid,4);  /* PRQA S 3200 */
                                aTypeAUidLen += 4;
                            }
                            
                        }
                        else
                        {
                            memcpy(&aTypeAUidComplete[aTypeAUidLen], aTypeAUid, 4);  /* PRQA S 3200 */
                            aTypeAUidLen += 4;
                        }
                        
                        continue;
                    }
                    else
                    {
                        /* Some error that can't be handled */
                        bailOutFlag = 1;
                        break;
                    }
                }
                if (bailOutFlag == 1)
                {
                    break;
                }
                
                status = phpalI14443p3a_Select(
                    pDataParams->pPal1443p3aDataParams,
                    aCascadeCodes[bCascadeCodeIdx],           /* Cascade code */
                    aTypeAUid,
                    pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak     /* SAK */
                    );
                if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
                {
                    if (!(pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak[0] & 0x04))
                    {
                        /* A card UID has been resolved */
                        if ((pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak[0] & (uint8_t) ~0xFB) == 0)
                        {
                            /* Bit b3 is set to zero, [Digital] 4.8.2 */
                            /* Mask out all other bits except for b7 and b6 */
                            bTagPlat = (pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak[0] & 0x60);
                            bTagPlat = bTagPlat >> 5;

                            switch(bTagPlat)
                            {
                            case PHAC_DISCLOOP_TYPEA_TYPE2_TAG_CONFIG_MASK:
                                pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEA_DETECTED_TAG_TYPE2;
                                break;
                            case PHAC_DISCLOOP_TYPEA_TYPE4A_TAG_CONFIG_MASK:
                                pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEA_DETECTED_TAG_TYPE4A;
                                break;
                            case PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TAG_CONFIG_MASK:
                                pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEA_DETECTED_TAG_P2P;
                                break;
                            case PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TYPE4A_TAG_CONFIG_MASK:
                                pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEA_DETECTED_TAG_NFC_DEP_TYPE4A;
                                break;
                            default:
                                break;
                            }
                        }
                        break;
                    }
                }
                else
                {
                    return status;
                }

            }
    
            /* If select was successful */
            if (status == PH_ERR_SUCCESS)
            {
                /* Copy the UID into the proper struct represening 
                   a TYPE A I3p3 tag */
                if (bCascadeCodeIdx == 0)
                {
                    memcpy( pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aUid, aTypeAUidComplete, aTypeAUidLen);  /* PRQA S 3200 */
                    pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bUidSize = aTypeAUidLen;
                }
                else
                {
                    memcpy(pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aUid, &aTypeAUidComplete[1],aTypeAUidLen);  /* PRQA S 3200 */
                    pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bUidSize = aTypeAUidLen-1;
                }

                /* Move onto next card */
                bTypeATagIdx++;

                if((bTypeAMoreCrads != 0) &&
                   (!(pDataParams->bBailOut & PHAC_DISCLOOP_CON_BAIL_OUT_A)))
                {
                    /* Halt the card */
                    status = phpalI14443p3a_HaltA(pDataParams->pPal1443p3aDataParams);
                    if (status != PH_ERR_SUCCESS)
                    {
                        /* Halt A has failed, can't count this as detected tag */
                        return status;
                    }

                    /* In preparation of next anticollision loop, do a REQ A */
                    status = phpalI14443p3a_RequestA(
                        pDataParams->pPal1443p3aDataParams,
                        aAtqa
                        );
                    if (status != PH_ERR_SUCCESS)
                    {
                        /* Cannot proceed with the Anticollision if REQA fails */
                        return status;
                    }
                }
                else
                {
                    break;
                }
            }
        } 
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P3_TAGS */
        bLoop++;
    }
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P3_TAGS
    pDataParams->sTypeATargetInfo.bTotalTagsFound_A = bTypeATagIdx;
#endif

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AC_DISCLOOP);
}


phStatus_t phacDiscLoop_Sw_Int_ActivateA(
    phacDiscLoop_Sw_DataParams_t * pDataParams,
    uint8_t                        bTypeATagIdx
    )
{
    phStatus_t   PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint8_t      PH_MEMLOC_REM aUid[10];
    uint8_t      PH_MEMLOC_REM bUidLen;
    uint8_t      PH_MEMLOC_REM bMoreCardsAvailable;
    
    if (bTypeATagIdx > pDataParams->sTypeATargetInfo.bTotalTagsFound_A)
    {
        /* Out of range or no such card found yet */
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    /* If bail out is set, layer 3 activation is already done during collision resolution */
    if(!(pDataParams->bBailOut & PHAC_DISCLOOP_CON_BAIL_OUT_A) &&
       (pDataParams->sTypeATargetInfo.bTotalTagsFound_A == 1))
    {
        status = phhalHw_ApplyProtocolSettings(
            pDataParams->pHalDataParams,
            PHHAL_HW_CARDTYPE_ISO14443A
            );
        if (status != PH_ERR_SUCCESS)
        {
            return status;
        }

        /* Activate the card in sleep with given UID */
        status = phpalI14443p3a_ActivateCard(
            pDataParams->pPal1443p3aDataParams,
            pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aUid,
            pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bUidSize,
            aUid,
            &bUidLen,
            pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak,
            &bMoreCardsAvailable);
    }

    if (status == PH_ERR_SUCCESS)
    {
#ifdef  NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS
        
        if (PHAC_DISCLOOP_CHECK_ANDMASK(pDataParams->wTagsFound, PHAC_DISCLOOP_TYPEA_DETECTED_TAG_TYPE4A))
        {
            status = phpalI14443p4a_ActivateCard(
                pDataParams->pPal1443p4aDataParams,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bFsdi,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bCid,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bDri,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bDsi,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.aAts
                );
        }
#endif /*  NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS */

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_TAGS

        if (PHAC_DISCLOOP_CHECK_ANDMASK(pDataParams->wTagsFound, PHAC_DISCLOOP_TYPEA_DETECTED_TAG_P2P))
        {
            /* Type A P2P tag */
            status = phpalI18092mPI_Atr(
                pDataParams->pPal18092mPIDataParams,
                pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aUid,
                pDataParams->sTypeATargetInfo.sTypeA_P2P.bDid,
                pDataParams->sTypeATargetInfo.sTypeA_P2P.bLri,
                pDataParams->sTypeATargetInfo.sTypeA_P2P.bNadEnable,
                pDataParams->sTypeATargetInfo.sTypeA_P2P.bNad,
                pDataParams->sTypeATargetInfo.sTypeA_P2P.pGi,
                pDataParams->sTypeATargetInfo.sTypeA_P2P.bGiLength,
                pDataParams->sTypeATargetInfo.sTypeA_P2P.pAtrRes,
                &(pDataParams->sTypeATargetInfo.sTypeA_P2P.bAtrResLength)
                );
        }
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_TAGS */

        /* Type A tag activated */
        pDataParams->sTypeATargetInfo.bActivatedTagNumber_A++;
    }
    if (status == PH_ERR_SUCCESS)
    {
        /*pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bState = PHAC_DISCLOOP_TYPEA_STATE_ACTIVATED;*/
    }
    else
    {
        /*pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bState = PHAC_DISCLOOP_STATE_UNKNOWN;*/
    }

    return status;
                                                                
}
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS
phStatus_t phacDiscLoop_Sw_Int_DetectB(
    phacDiscLoop_Sw_DataParams_t * pDataParams
    )
 {
    uint8_t    aAtqb[13];
    uint8_t    bAtqbLen;
    phStatus_t status;
    uint8_t    bLoop;
    uint8_t    bTypeBTagIdx;

    bLoop = 0;
    bTypeBTagIdx = 0;
    status = PH_ERR_INTERNAL_ERROR;


    /* Reset Type B field in tags found to 0, indicating no tag found */
    pDataParams->wTagsFound = pDataParams->wTagsFound & ~(PHAC_DISCLOOP_TYPEB_DETECTED);

    /* No Type B tag found yet */
    pDataParams->sTypeBTargetInfo.bTotalTagsFound_B = 0;
    
    while (bLoop <  pDataParams->sTypeBTargetInfo.bLoopLimit_B)
    {
        status = phpalI14443p3b_RequestB(
            pDataParams->pPal1443p3bDataParams,
            pDataParams->sTypeBTargetInfo.bNCodingSlotNumber,
            pDataParams->sTypeBTargetInfo.bAfiReq,
            pDataParams->sTypeBTargetInfo.bExtendedAtqBbit,
            aAtqb,
            &bAtqbLen
            );
        if (status == PH_ERR_INTEGRITY_ERROR)
        {
            /* More cards in the field */
            pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEB_DETECTED;
            break;
        }
        else if (status == PH_ERR_SUCCESS)
        {
            /* Only one Type B tag in the field */   
            memcpy( pDataParams->sTypeBTargetInfo.aI3P3B[bTypeBTagIdx].aPupi, &aAtqb[1], 4);  /* PRQA S 3200 */
            pDataParams->sTypeBTargetInfo.aI3P3B[bTypeBTagIdx].bAtqBLength = bAtqbLen;
            pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEB_DETECTED;
            pDataParams->sTypeBTargetInfo.bTotalTagsFound_B++;
            break;
        }
        else
        {
            /* Nothing to Do */
        }
        bLoop++;
    }

    if (status != PH_ERR_SUCCESS && status != PH_ERR_INTEGRITY_ERROR)
    {
        /* No tag was found */
        return PH_ADD_COMPCODE(
            PHAC_DISCLOOP_ERR_TYPEB_NO_TAG_FOUND, 
            PH_COMP_AC_DISCLOOP
            );
    }
    else
    {
        /* One or more Type B tag detected in field */
        pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEB_DETECTED;
    }
    

    return status;
}

phStatus_t phacDiscLoop_Sw_Int_CollisionResolutionB(
    phacDiscLoop_Sw_DataParams_t * pDataParams
    )
{
    phStatus_t   status;
    uint8_t      aAtqb[13];
    uint8_t      bAtqbLen;
    /*uint8_t      bDeviceCounter; */
    uint8_t      bNumOfSlots;
    uint8_t      bCurrentSlotNum; 

    /* 9.3.5.1 set device counter to zero and number of slots to 1 */
    bNumOfSlots = 1;
    

  
    /* 9.3.5.2 the device must read GRE_POLL_B[1], containing most recent 
       SENSB_RES and determine wether SENSB_RES is valid. If not valid,
       go to symbol 2, 9.3.5.3 or else go to symbol 3, 9.3.5.4 */
   
    if (pDataParams->sTypeBTargetInfo.bTotalTagsFound_B == 0 && 
        PHAC_DISCLOOP_QUERY_TAG_FOUND(
            pDataParams, 
            PHAC_DISCLOOP_TYPEB_DETECTED))
        {
            pDataParams->sTypeBTargetInfo.bTotalTagsFound_B = 0;
            while(bNumOfSlots <= PHAC_DISCLOOP_TYPEB_MAX_SLOT_NUM)
            {
                /* If type B tag(s) are found, as indicated by 
                   the bTagsFound filed and still the if bTotalTagsFound 
                   field is zero, then there was a collision.
                   Implements 9.3.5.4 */
                /* The NFC device must set current slot number to 
                   zero and initialize INT_COL_PEND with zero */
                bCurrentSlotNum = 0;
                pDataParams->sTypeBTargetInfo.bCollPend = 0;
                
                while (bCurrentSlotNum <= (1 << bNumOfSlots))
                {
                    
                    /* 9.3.5.5 */
                    if (bCurrentSlotNum != 0)
                    {
                        /* 9.3.5.21 */
                        /* With the current slot number, we 
                           try to get the response */
                        status = phpalI14443p3b_SlotMarker(
                            pDataParams->pPal1443p3bDataParams,
                            bCurrentSlotNum,
                            aAtqb,
                            &bAtqbLen
                            );
                        
                        /* 9.3.5.6, if a valid SENSB_RES obtained */
                        if (status == PH_ERR_SUCCESS)
                        {
                            /* NFC forum device must store the ID 9.3.5.9 */
                            memcpy(pDataParams->sTypeBTargetInfo.aI3P3B[pDataParams->sTypeBTargetInfo.bTotalTagsFound_B].aPupi,&aAtqb[1],bAtqbLen);  /* PRQA S 3200 */
                            pDataParams->sTypeBTargetInfo.aI3P3B[pDataParams->sTypeBTargetInfo.bTotalTagsFound_B].bAtqBLength = bAtqbLen;
                            /* Proceed to symbol 7, 
                               9.3.5.8, inrement the device counter */
                            pDataParams->sTypeBTargetInfo.bTotalTagsFound_B++;
                            /* bDeviceCounter++; */
                            /* pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEB_DETECTED_TAG_TYPEB; */
                            
                            /* Is it required to resolve more tags ?
                               9.3.5.10 */
                            if (pDataParams->sTypeBTargetInfo.bTotalTagsFound_B 
                                < pDataParams->sTypeBTargetInfo.bDeviceLimit_B)
                            {
                                /* The NFC forum device puts the 
                                   tag in sleep state if more 
                                   tags are to be resolved, 9.3.5.12 */
                                status = phpalI14443p3b_HaltB(
                                    pDataParams->pPal1443p3bDataParams
                                    );
                                if (status == PH_ERR_SUCCESS)
                                {
                                    pDataParams->sTypeBTargetInfo.aI3P3B[pDataParams->sTypeBTargetInfo.bTotalTagsFound_B-1].bTagState 
                                        = PHAC_DISCLOOP_TYPEB_STATE_SLEEP_B;
                                }
                                else
                                {
                                    pDataParams->sTypeBTargetInfo.aI3P3B[pDataParams->sTypeBTargetInfo.bTotalTagsFound_B-1].bTagState 
                                        = PHAC_DISCLOOP_STATE_UNKNOWN;
                                }
                            }
                            else
                            {
                                /* 9.3.5.10 The NFC device must 
                                   conculde the NFC-B collision */
                                return PH_ADD_COMPCODE(
                                    PH_ERR_SUCCESS, 
                                    PH_COMP_AC_DISCLOOP
                                    );
                            }
                            /* Proceed to 9.3.5.12 */
                        }
                        else
                        {
                            /* 9.3.5.7, The NFC device must 
                               set INT_COLL_PEND to 1 */
                            pDataParams->sTypeBTargetInfo.bCollPend = 1;
                            /* and proceed to symbol 11, 9.3.5.12 */
                        }
                    }
                    /* 9.3.5.12 increment the current slot number */
                    bCurrentSlotNum++;
                }
                
                /* 9.3.5.14, check int_coll_pend flag */
                if (pDataParams->sTypeBTargetInfo.bCollPend == 1)
                {
                    /* If set, proceed to symbol 14, 9.3.5.15 */
                    
                    /* If subsequent to the last SENSB_REQ Command, the 
                       NFC Forum Device resolved an identifier of a responding
                       device (i.e., the identifier of the responding device 
                       has been memorized), then the NFC Forum 
                       Device MUST proceed to Symbol 17, 9.3.5.16 */
                    
                    /* We know that a tag has responded and we have its 
                       pupi is by looking at the  
                       pDataParams->sTypeBTargetInfo.bTotalTagsFound 
                       which is non-zero if SENS_B went successfully in 
                       detect phase or SENS_B command in collision 
                       resolution phase */
                    if ( pDataParams->sTypeBTargetInfo.bTotalTagsFound_B > 0)
                    {
                        /* Atleast one card has been resolved */
                        /* Proceed to symbol 17, 9.3.5.18 */
                        if (pDataParams->sTypeBTargetInfo.bTotalTagsFound_B 
                            >= pDataParams->sTypeBTargetInfo.bDeviceLimit_B)
                        {
                            /* 9.3.5.18, If the device counter is greater 
                               than device limit, the NFC forum device must 
                               conclude the NFC-B collision resolution 
                               activity */
                            return PH_ADD_COMPCODE(
                                PH_ERR_SUCCESS, 
                                PH_COMP_AC_DISCLOOP
                                );
                        }
                    }
                    else
                    {
                        /* 9.3.5.16 if number of slots is equal to 
                           the maximum value allowed then the device must 
                           conclude the NFC-B collision resolution activity */
                        if (bNumOfSlots == PHAC_DISCLOOP_TYPEB_MAX_SLOT_NUM)
                        {
                            return PH_ADD_COMPCODE(
                                PH_ERR_SUCCESS, 
                                PH_COMP_AC_DISCLOOP);
                    }
                        
                        /* 9.3.5.17, The NFC forum device must 
                           increase the number of slots 
                           to the next value allowed */
                        bNumOfSlots++;
                        
                        /* 9.3.5.18, if the device 
                           counter is lower than device limit */
                        
                        if (pDataParams->sTypeBTargetInfo.bTotalTagsFound_B 
                            < pDataParams->sTypeBTargetInfo.bDeviceLimit_B)
                        {
                            /* Proceed to symbol 18 */
                            /* The NFC forum device must send SENS_B command */
                            status = phpalI14443p3b_RequestB(
                                pDataParams->pPal1443p3bDataParams,
                                bNumOfSlots,
                                pDataParams->sTypeBTargetInfo.bAfiReq,
                                pDataParams->sTypeBTargetInfo.bExtendedAtqBbit,
                                aAtqb,
                                &bAtqbLen
                                );
                            if (status == PH_ERR_SUCCESS)
                            {
                                /* Only one Type B tag in the field */   
                                memcpy(pDataParams->sTypeBTargetInfo.aI3P3B[pDataParams->sTypeBTargetInfo.bTotalTagsFound_B].aPupi,&aAtqb[1],bAtqbLen);  /* PRQA S 3200 */
                                pDataParams->sTypeBTargetInfo.aI3P3B[pDataParams->sTypeBTargetInfo.bTotalTagsFound_B].bAtqBLength = bAtqbLen;
                                pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEB_DETECTED;
                                pDataParams->sTypeBTargetInfo.bTotalTagsFound_B++;
                            }
                        }
                        else
                        {
                            /* 9.3.5.18, the device must 
                               conclude anticollision activity */
                            return PH_ADD_COMPCODE(
                                PH_ERR_SUCCESS, 
                                PH_COMP_AC_DISCLOOP
                                );
                        }
                    }
                }
            }
        }
        else
        {
            /* 9.3.5.20 */
           pDataParams->sTypeBTargetInfo.bCollPend = 1;
        }
        
        return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AC_DISCLOOP);
}
                                       
                       
        
phStatus_t phacDiscLoop_Sw_Int_ActivateB(
    phacDiscLoop_Sw_DataParams_t * pDataParams,
    uint8_t                        bTypeBTagIdx)
{
    phStatus_t status;
    uint8_t    aAtqb[14];
    uint8_t    bMoreCardsAvailable;
    uint8_t    bPupiLength;

    if (bTypeBTagIdx > pDataParams->sTypeBTargetInfo.bTotalTagsFound_B)
    {
        /* Out of range or no such card found yet */
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }
    status = phhalHw_ApplyProtocolSettings(
        pDataParams->pHalDataParams, 
        PHHAL_HW_CARDTYPE_ISO14443B);

    if (status != PH_ERR_SUCCESS)
    {
        return status;
    }

    if((pDataParams->bBailOut & PHAC_DISCLOOP_CON_BAIL_OUT_B) &&
       (pDataParams->sTypeBTargetInfo.bDeviceLimit_B == 1))
    {
        bPupiLength = 0;
    }
    else
    {
        bPupiLength = 4;
    }

    status = phpalI14443p3b_ActivateCard(
        pDataParams->pPal1443p3bDataParams,
        pDataParams->sTypeBTargetInfo.aI3P3B[bTypeBTagIdx].aPupi,
        bPupiLength,
        pDataParams->sTypeBTargetInfo.bNCodingSlotNumber,
        pDataParams->sTypeBTargetInfo.bAfiReq,
        pDataParams->sTypeBTargetInfo.bExtendedAtqBbit,
        pDataParams->sTypeBTargetInfo.bFsdi,
        pDataParams->sTypeBTargetInfo.bCid,
        pDataParams->sTypeBTargetInfo.bDri,
        pDataParams->sTypeBTargetInfo.bDsi,
        aAtqb,
        &pDataParams->sTypeBTargetInfo.aI3P3B[bTypeBTagIdx].bAtqBLength,
        &pDataParams->sTypeBTargetInfo.sI3P4B.bMbli,
        &bMoreCardsAvailable
        );

    return status;
}
    
    
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS */

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS
phStatus_t phacDiscLoop_Sw_Int_DetectF(
    phacDiscLoop_Sw_DataParams_t  * pDataParams)
{                                               
    phStatus_t status;
    uint8_t    bLoop;
    uint8_t    aRxBuf[18];
    uint8_t    bTypeFTagIdx;

    bTypeFTagIdx = 0;
    status = PH_ERR_INTERNAL_ERROR;
    pDataParams->sTypeFTargetInfo.bTotalTagsFound_F = 0;
    bLoop = pDataParams->sTypeFTargetInfo.bLoopLimit_F;
    while (bLoop > 0)
    {
        status = phpalFelica_ReqC(pDataParams->pPalFelicaDataParams,
                                  pDataParams->sTypeFTargetInfo.aSystemCode,
                                  pDataParams->sTypeFTargetInfo.bTimeSlot,
                                  aRxBuf);

        if (status == PH_ERR_SUCCESS)
        {
            /* In case all went well, copy the IDm and PMm */
            memcpy(pDataParams->sTypeFTargetInfo.aTypeF[bTypeFTagIdx].aIDmPMm, aRxBuf, (PHAC_DISCLOOP_FELICA_IDM_LENGTH + PHAC_DISCLOOP_FELICA_PMM_LENGTH));  /* PRQA S 3200 */
            pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEF_DETECTED;
            break;
        }

        bLoop--;
    }
    
    if (((status & PH_ERR_MASK) != PH_ERR_COLLISION_ERROR) 
        && status != PH_ERR_SUCCESS)
    {
        /* No tag was found */
        return PH_ADD_COMPCODE(
            PHAC_DISCLOOP_ERR_TYPEB_NO_TAG_FOUND, 
            PH_COMP_AC_DISCLOOP
            );
    }
    else
    {
        /* One or more Type F tag detected in field */
        pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEF_DETECTED;
        /* TODO: Only one Type F tag for now */
        pDataParams->sTypeFTargetInfo.bTotalTagsFound_F = 1;
    }

    if ((pDataParams->sTypeFTargetInfo.aTypeF[bTypeFTagIdx].aIDmPMm[0] == 0x01) &&
        (pDataParams->sTypeFTargetInfo.aTypeF[bTypeFTagIdx].aIDmPMm[1] == 0xFE))
    {
        /* This is Type F tag with P2P capablities */
        pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEF_DETECTED_TAG_P2P;
    }
    else
    {
        /* This is Type F T3T tag */
        pDataParams->wTagsFound |= PHAC_DISCLOOP_TYPEF_DETECTED_TAG_TYPE3;
    }

    return status;

}
                              
                                                                                            
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS */


void phacDiscLoop_Sw_Int_ClearDataParams(phacDiscLoop_Sw_DataParams_t * pDataParams)
{
    pDataParams->bState                  = PHAC_DISCLOOP_STATE_IDLE;
    pDataParams->wTagsFound              = PHAC_DISCLOOP_NO_TAGS_FOUND;
    
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS
    pDataParams->sTypeATargetInfo.bTotalTagsFound_A = 0;
    pDataParams->sTypeATargetInfo.bActivatedTagNumber_A = 0;
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS
    pDataParams->sTypeBTargetInfo.bTotalTagsFound_B = 0;
    pDataParams->sTypeBTargetInfo.bActivatedTagNumber_B = 0;
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS */

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS
    pDataParams->sTypeFTargetInfo.bTotalTagsFound_F = 0;
    pDataParams->sTypeFTargetInfo.bActivatedTagNumber_F = 0;
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS */
}


#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
