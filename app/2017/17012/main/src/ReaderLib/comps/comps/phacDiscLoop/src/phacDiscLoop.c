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
* Generic Discovery Loop Activities Component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 161 $
* $Date: 2013-06-05 14:04:36 +0530 (Wed, 05 Jun 2013) $
*
* History:
*  PC: Generated 23. Aug 2012
*
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phacDiscLoop.h>

#ifdef NXPBUILD__PHAC_DISCLOOP_SW
#include "Sw/phacDiscLoop_Sw.h"
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
#ifdef NXPBUILD__PHAC_DISCLOOP

phStatus_t phacDiscLoop_Start(void * pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_Start");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams);*/
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
	
    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    /* Select the active layer to perform the action */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
        case PHAC_DISCLOOP_SW_ID:
            status = phacDiscLoop_Sw_Start(pDataParams);
            break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
        default:
            status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
            break;
    }
    return status;
}

phStatus_t phacDiscLoop_GetConfig(void * pDataParams,
                                  uint16_t wConfig,
                                  uint16_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_GetConfig");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams); */
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pValue);

     /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }
    
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
    case PHAC_DISCLOOP_SW_ID:
        status = phacDiscLoop_Sw_GetConfig(pDataParams, wConfig, pValue);
        break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}



phStatus_t phacDiscLoop_SetConfig(
    void * pDataParams,
    uint16_t wConfig,
    uint16_t wValue
    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_SetConfig");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams); */
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wValue_log, &wValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);


    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
    case PHAC_DISCLOOP_SW_ID:
        status = phacDiscLoop_Sw_SetConfig(pDataParams, wConfig, wValue);
        break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phacDiscLoop_ActivateCard(
    void * pDataParams,
    uint8_t bTagType,
    uint8_t bTagIndex
    )
{
    phStatus_t PH_MEMLOC_REM status;
    
    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_ActivateCard");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams); */
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bTagType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bTagIndex);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bTagType_log, &bTagType);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bTagIndex_log, &bTagIndex);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    
    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }
    
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
    case PHAC_DISCLOOP_SW_ID:
        status = phacDiscLoop_Sw_ActivateCard(pDataParams, bTagType, bTagIndex);
        break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}
#ifdef __DEBUG
phStatus_t phacDiscLoop_SetPtr(
    void * pDataParams,
    void * pI14443p3a,
    void * pI14443p4a,
    void * pI14443p3b,
    void * pFeilca,
    void * pI18092mPI
    
    )
{
    phStatus_t PH_MEMLOC_REM status;
    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_SetPtr");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams); */
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pI14443p3a);
    PH_ASSERT_NULL (pI14443p4a);
    PH_ASSERT_NULL (pI14443p3b);
    PH_ASSERT_NULL (pFeilca);
    PH_ASSERT_NULL (pI18092mPI);
    
    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }
    
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
    case PHAC_DISCLOOP_SW_ID:
        status = phacDiscLoop_Sw_SetPtr(pDataParams,
            pI14443p3a,
            pI14443p4a,
            pI14443p3b,
            pFeilca,
            pI18092mPI
            );
        break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        break;
    }
    
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;    
}
#endif /* #if __DEBUG */
#endif /* NXPBUILD__PHAC_DISCLOOP */

