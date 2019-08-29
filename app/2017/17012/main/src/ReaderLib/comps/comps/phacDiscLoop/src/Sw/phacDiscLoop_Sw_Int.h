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
* Software Discovery Loop Activities Component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 452 $
* $Date: 2013-12-19 20:09:00 +0530 (Thu, 19 Dec 2013) $
*
* History:
*  PC: Generated 23. Aug 2012
*
*/
#ifndef PHACDICLOOP_SW_INT_H
#define PHACDICLOOP_SW_INT_H
#ifdef NXPBUILD__PHAC_DISCLOOP_SW

/** This macro calls the error hadnling call back function */
#define PHAC_DISCLOOP_EXEC_ERR_CALL_BACK(pDataParams, status) \
    if ((pDataParams)->pErrHandler != NULL)                     \
    {                                                         \
        (*((pDataParams)->pErrHandler))(status);                  \
    }                                                         \

/**
 * This macro handles the error condition within dicovery
 * loop.  First, it calls the call back funcition and then 
 * returns from discovery loop.
 */
#define PHAC_DISCLOOP_CHECK_STATUS(pDataParams, status)        \
    if (((status) & PH_ERR_MASK) != PH_ERR_SUCCESS )             \
    {                                                          \
        PHAC_DISCLOOP_EXEC_ERR_CALL_BACK(pDataParams, status); \
        return (status);                                         \
    }                                                          \

/** 
 * This macro checks for the discovery loop stop flag, if set
 * exits the discovery loop with success code 
 */
#define PHAC_DISCLOOP_CHECK_STOP(pDataParams)                   \
if ((pDataParams)->bStopFlag == PH_ON)                            \
{                                                               \
    (pDataParams)->bState = PHAC_DISCLOOP_STATE_IDLE;             \
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AC_DISCLOOP);\
}


#define PHAC_DISCLOOP_TYPEA_TYPE1_TAG_ATQA_BYTE1_CONFIG_MASK       0x1FU
#define PHAC_DISCLOOP_TYPEA_TYPE1_TAG_ATQA_BYTE2_CONFIG_MASK       0x0FU                /**< Mask for determining whether a Type A tag is T1. Used along with byte 2 of ATQZ */
#define PHAC_DISCLOOP_TYPEA_TYPE2_TAG_CONFIG_MASK                  0x00U
#define PHAC_DISCLOOP_TYPEA_TYPE4A_TAG_CONFIG_MASK                 0x01U
#define PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TAG_CONFIG_MASK           0x02U
#define PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TYPE4A_TAG_CONFIG_MASK    0x03U


#define  PHAC_DISCLOOP_TYPEB_MAX_SLOT_NUM                          0x04U  /**< Maximum slot numbers allowed for Type B tags */
/** 
 * This macro evaluates to non zero value if a particular tag type was 
 * found which is indicated by it's respective bit or else zero is returned 
 */ 
#define PHAC_DISCLOOP_QUERY_TAG_FOUND(pDataParams, tagType) \
    ((pDataParams)->wTagsFound & (tagType)) 


#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS


/**
 *  \brief    Detects if there is any Type A tag in the field.
 * 
 *  \return   Error code
 *
 *  \retval   #PH_ERR_SUCCESS on success
 *  \retval   Error code depending on the underlaying components
 *
 */
phStatus_t phacDiscLoop_Sw_Int_DetectA(
    phacDiscLoop_Sw_DataParams_t * pDataParams                               /**<[In] Pointer to this layer's parameter data structure */
    );

/**
 * \brief    Does collision resolution for Type A tags.
 *
 * \return   Error code 
 * 
 * \retval   #PH_ERR_SUCCESS on success
 * \retval   Error code depending on the underlaying components
 */

phStatus_t phacDiscLoop_Sw_Int_CollisionResolutionA(
    phacDiscLoop_Sw_DataParams_t * pDataParams                              /**<[In] Pointer to this layer's parameter data structure */
    );

/**
 * \brief    Activates type A tag with given index
 * 
 * \return   Error code 
 * 
 * \retval   #PH_ERR_SUCCESS      on success
 * \retval   #PH_ERR_INVALID_ARG  If invalid index was passed 
 * \retval   Error code depending on the underlaying components
 * 
 */
 

phStatus_t phacDiscLoop_Sw_Int_ActivateA(
    phacDiscLoop_Sw_DataParams_t * pDataParams,                            /**<[In] Pointer to this layer's parameter data structure */
    uint8_t                        bTypeATagIdx                            /**<[In] Index of the tag to be activtaed */
    );
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS

/**
 * \brief    Detects Type B tags in the field
 *
 * \retuan   Error code 
 * 
 * \retval   #PH_ERR_SUCCESS     on success
 * \retval   Error code depending on the underlaying componente
 *
 */

phStatus_t phacDiscLoop_Sw_Int_DetectB(
    phacDiscLoop_Sw_DataParams_t * pDataParams                            /**<[In] Pointer to this layer's parameter data structure */
    );

/**
 * \brief  Performs collision resolution on Type B tags
 * 
 * \return  error code
 * \retval  #PH_ERR_SUCCESS on success
 * \retval  other error code depending on underlaying components 
 */
phStatus_t phacDiscLoop_Sw_Int_CollisionResolutionB(phacDiscLoop_Sw_DataParams_t * pDataParams);

/**
 * \brief   Activates the given Type B tag
 * 
 * \return  Error code
 * \retval  #PH_ERR_SUCCESS on success
 * \retval  other error code depending on underlaying components
 */

phStatus_t phacDiscLoop_Sw_Int_ActivateB(
    phacDiscLoop_Sw_DataParams_t * pDataParams,                           /**<[In] Pointer to this layer's parameter data structure */
    uint8_t                        bTypeBTagIdx                           /**<[In] Index of the tag to be activated */
    ); 
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS */

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS

/**
 * \brief  Tries to detect Type F tags in the field 
 * 
 * \return  Error code
 * \retval  #PH_ERR_SUCCESS on success
 * \retval  other error code depending on underlaying components
 */
phStatus_t phacDiscLoop_Sw_Int_DetectF(
    phacDiscLoop_Sw_DataParams_t  * pDataParams                          /**<[In] Pointer to this layer's parameter data structure */
    );


#endif  /* NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS */

/**
 * \brief  Clears the information that was collected during greedy
 *         collection algorithim
 */

void phacDiscLoop_Sw_Int_ClearDataParams(
    phacDiscLoop_Sw_DataParams_t * pDataParams                           /**<[In] Pointer to this layer's data parameters */
    );

#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
#endif /* PHACDICLOOP_H */

