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
* Generic LLCP Link layer Component of Reader Library Framework.
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
* History:
*  PC: Generated 23. Aug 2012
*
*/
#include <ph_Status.h>

#ifdef NXPBUILD__PHLN_LLCP
#include <phlnLlcp.h>
#include <ph_RefDefs.h>
#include "Fri/phlnLlcp_Fri_Llcp.h"
#include "Fri/phlnLlcp_Fri_Transport.h"

phStatus_t phlnLlcp_Reset(  
                          void                            *pDataParams,
                          phlnLlcp_Fri_LinkStatus_CB_t     pfLink_CB,
                          void                            *pContext
                          )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Reset");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pfLink_CB);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Reset(
                                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp,
                                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLowerDevice,
                                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLinkParams,
                                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pRxBuffer,
                                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->wRxBufferLength,
                                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pTxBuffer,
                                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->wTxBufferLength,
                                pfLink_CB,
                                pContext);
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_ChkLlcp(
                            void                       *pDataParams,
                            phlnLlcp_Fri_Check_CB_t     pfCheck_CB,
                            void                       *pContext
                            )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_ChkLlcp");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pfCheck_CB);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_ChkLlcp(
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp,
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pRemoteDevInfo,
            pfCheck_CB,
            pContext);
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Activate(void  *pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Activate");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Activate(((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp);
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Deactivate(void  *pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Deactivate");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Deactivate(((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp);
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_GetLocalInfo(void  *pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_GetLocalInfo");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_GetLocalInfo(
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp,
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLinkParams
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_GetRemoteInfo(void  *pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_GetRemoteInfo");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_GetRemoteInfo(
                                    ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp,
                                    ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLinkParams
                                    );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Send(
                         void                            *pDataParams,
                         phlnLlcp_Fri_sPacketHeader_t    *pHeader,
                         phlnLlcp_Fri_sPacketSequence_t  *pSequence,
                         phNfc_sData_t                   *pInfo,
                         phlnLlcp_Fri_Send_CB_t           pfSend_CB,
                         void                            *pContext
                         )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Send");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pHeader);
    PH_ASSERT_NULL (pSequence);
    PH_ASSERT_NULL (pInfo);
    PH_ASSERT_NULL (pfSend_CB);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Send(
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp,
            pHeader,
            pSequence,
            pInfo,
            pfSend_CB,
            pContext);
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Recv(
                         void                     *pDataParams,
                         phlnLlcp_Fri_Recv_CB_t    pfRecv_CB,
                         void                     *pContext
                         )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Recv");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pfRecv_CB);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Recv(
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp,
            pfRecv_CB,
            pContext);
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Reset(void  *pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Reset");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Reset(
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcpSocketTable,
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcp
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_CloseAll(void  *pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_CloseAll");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_CloseAll(
                ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcpSocketTable
                );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_SocketGetLocalOptions(
                                void                                      *pDataParams,
                                phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                phlnLlcp_Fri_Transport_sSocketOptions_t   *pLocalOptions
                                )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_SocketGetLocalOptions");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_SocketGetLocalOptions(
                pLlcpSocket,
                pLocalOptions
                );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_SocketGetRemoteOptions(
                                void                                      *pDataParams,
                                phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                phlnLlcp_Fri_Transport_sSocketOptions_t   *pLocalOptions
                                )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_SocketGetRemoteOptions");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_SocketGetRemoteOptions(
            pLlcpSocket,
            pLocalOptions
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Socket(
                                     void                                       *pDataParams,
                                     phlnLlcp_Transport_eSocketType_t            eType,
                                     phlnLlcp_Fri_Transport_sSocketOptions_t    *pOptions,
                                     phNfc_sData_t                              *pWorkingBuffer,
                                     phlnLlcp_Fri_Transport_Socket_t           **pLlcpSocket,
                                     phlnLlcp_Fri_TransportSocketErrCb_t         pErr_Cb,
                                     void                                       *pContext  
                                     )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Socket");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pOptions);
    PH_ASSERT_NULL (pWorkingBuffer);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pErr_Cb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Socket(
            ((phlnLlcp_Fri_DataParams_t *)pDataParams)->pLlcpSocketTable,
            eType,
            pOptions,
            pWorkingBuffer,
            pLlcpSocket,
            pErr_Cb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Close(
                                    void                              *pDataParams,
                                    phlnLlcp_Fri_Transport_Socket_t   *pLlcpSocket
                                    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Close");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Close(pLlcpSocket);
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Bind(
                                   void                               *pDataParams,
                                   phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket,
                                   uint8_t                             nSap,
                                   phNfc_sData_t                      *psServiceName
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Bind");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(nSap);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_INFO, nSap_log, &nSap);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (psServiceName);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Bind(
            pLlcpSocket,
            nSap,
            psServiceName
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Listen(
                                     void                                    *pDataParams,
                                     phlnLlcp_Fri_Transport_Socket_t         *pLlcpSocket,
                                     phlnLlcp_Fri_TransportSocketListenCb_t   pListen_Cb,
                                     void                                    *pContext
                                     )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Listen");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pListen_Cb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Listen(
            pLlcpSocket,
            pListen_Cb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Accept(
                                     void                                      *pDataParams,
                                     phlnLlcp_Fri_Transport_Socket_t           *pLlcpSocket,
                                     phlnLlcp_Fri_Transport_sSocketOptions_t   *pOptions,
                                     phNfc_sData_t                             *pWorkingBuffer,
                                     phlnLlcp_Fri_TransportSocketErrCb_t        pErr_Cb,
                                     phlnLlcp_Fri_TransportSocketAcceptCb_t     pAccept_RspCb,
                                     void                                      *pContext
                                     )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Accept");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pOptions);
    PH_ASSERT_NULL (pWorkingBuffer);
    PH_ASSERT_NULL (pErr_Cb);
    PH_ASSERT_NULL (pAccept_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Accept(
            pLlcpSocket,
            pOptions,
            pWorkingBuffer,
            pErr_Cb,
            pAccept_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Reject(
                                     void                                    *pDataParams,
                                     phlnLlcp_Fri_Transport_Socket_t         *pLlcpSocket,
                                     phlnLlcp_Fri_TransportSocketRejectCb_t   pReject_RspCb,
                                     void                                    *pContext
                                     )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Reject");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pReject_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Reject(
            pLlcpSocket,
            pReject_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Connect(
                                      void                                       *pDataParams,
                                      phlnLlcp_Fri_Transport_Socket_t            *pLlcpSocket,
                                      uint8_t                                     nSap,
                                      phlnLlcp_Fri_TransportSocketConnectCb_t     pConnect_RspCb,
                                      void                                       *pContext
                                      )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Connect");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(nSap);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_INFO, nSap_log, &nSap);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pConnect_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Connect(
            pLlcpSocket,
            nSap,
            pConnect_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_ConnectByUri(
    void                                     *pDataParams,
    phlnLlcp_Fri_Transport_Socket_t          *pLlcpSocket,
    phNfc_sData_t                            *pUri,
    phlnLlcp_Fri_TransportSocketConnectCb_t   pConnect_RspCb,
    void                                     *pContext
    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_ConnectByUri");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pUri);
    PH_ASSERT_NULL (pConnect_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_ConnectByUri(
            pLlcpSocket,
            pUri,
            pConnect_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Disconnect(
    void                               *pDataParams,
    phlnLlcp_Fri_Transport_Socket_t    *pLlcpSocket,
    phlnLlcp_SocketDisconnectCb_t       pDisconnect_RspCb,
    void                               *pContext
    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Disconnect");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pDisconnect_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Disconnect(
            pLlcpSocket,
            pDisconnect_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Send(
                                   void                                  *pDataParams,
                                   phlnLlcp_Fri_Transport_Socket_t       *pLlcpSocket,
                                   phNfc_sData_t                         *pBuffer,
                                   phlnLlcp_Fri_TransportSocketSendCb_t   pSend_RspCb,
                                   void                                  *pContext
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Send");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pBuffer);
    PH_ASSERT_NULL (pSend_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Send(
            pLlcpSocket,
            pBuffer,
            pSend_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_Recv(
                                   void                                  *pDataParams,
                                   phlnLlcp_Fri_Transport_Socket_t       *pLlcpSocket,
                                   phNfc_sData_t                         *pBuffer,
                                   phlnLlcp_Fri_TransportSocketRecvCb_t   pRecv_RspCb,
                                   void                                  *pContext
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_Recv");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pBuffer);
    PH_ASSERT_NULL (pRecv_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_Recv(
            pLlcpSocket,
            pBuffer,
            pRecv_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_SendTo(
                                     void                                   *pDataParams,
                                     phlnLlcp_Fri_Transport_Socket_t        *pLlcpSocket,
                                     uint8_t                                 nSap,
                                     phNfc_sData_t                          *pBuffer,
                                     phlnLlcp_Fri_TransportSocketSendCb_t    pSend_RspCb,
                                     void                                   *pContext
                                     )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_SendTo");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(nSap);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_INFO, nSap_log, &nSap);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pBuffer);
    PH_ASSERT_NULL (pSend_RspCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_SendTo(
            pLlcpSocket,
            nSap,
            pBuffer,
            pSend_RspCb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phlnLlcp_Transport_RecvFrom(
                                       void                                        *pDataParams,
                                       phlnLlcp_Fri_Transport_Socket_t             *pLlcpSocket,
                                       phNfc_sData_t                               *pBuffer,
                                       phlnLlcp_Fri_TransportSocketRecvFromCb_t     pRecv_Cb,
                                       void                                        *pContext
                                       )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phlnLlcp_Transport_RecvFrom");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pLlcpSocket);
    PH_ASSERT_NULL (pBuffer);
    PH_ASSERT_NULL (pRecv_Cb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_LN_LLCP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHLN_LLCP_FRI
    case PHLN_LLCP_FRI_ID:
        status = phlnLlcp_Fri_Transport_RecvFrom(
            pLlcpSocket,
            pBuffer,
            pRecv_Cb,
            pContext
            );
        break;
#endif /* NXPBUILD__PHLN_LLCP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_LN_LLCP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

#endif /* NXPBUILD__PHLN_LLCP */

