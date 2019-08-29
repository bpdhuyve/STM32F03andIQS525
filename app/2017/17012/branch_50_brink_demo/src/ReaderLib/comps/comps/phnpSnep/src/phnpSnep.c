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
* Generic SNEP protocol layer Component of Reader Library Framework.
* $Author: nxp62726 $
* $Revision: 416 $
* $Date: 2013-11-13 14:38:02 +0530 (Wed, 13 Nov 2013) $
*
* History:
*  PC: Generated 3. May 2013
*
*/
#include <ph_Status.h>

#ifdef NXPBUILD__PHNP_SNEP
#include <phnpSnep.h>

#ifdef NXPBUILD__PHNP_SNEP_FRI
#include "Fri/phnpSnep_Fri.h"
#endif /* NXPBUILD__PHNP_SNEP_FRI */

/* SNEP Client Initialization API */
phStatus_t phnpSnep_Client_Init( void                           *pDataParams,
                                phnpSnep_Fri_Config_t           *pConfigInfo,
                                ph_NfcHandle                     hRemDevHandle,
                                pphnpSnep_Fri_ConnectCB_t        pConnClientCb,
                                phnpSnep_Fri_ClientSession_t    *pClient,
                                void                            *pContext)
{
    phStatus_t status = PH_ERR_SUCCESS;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_Client_Init");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pConfigInfo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pConnClientCb);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pClientSession);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pContext);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pConfigInfo);
    PH_ASSERT_NULL (pConnClientCb);
    PH_ASSERT_NULL (pClient);
    PH_ASSERT_NULL (pContext);


    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_NP_SNEP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHNP_SNEP_FRI
    case PHNP_SNEP_FRI_ID:
    status = phnpSnep_Fri_Client_Init( pDataParams,
                                       pConfigInfo,
                                       hRemDevHandle,
                                       pConnClientCb,
                                       pClient,
                                       pContext
                                      );
        break;
#endif /* NXPBUILD__PHNP_SNEP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);


    return status;

}

/* SNEP Client PUT request API */
phStatus_t phnpSnep_ClientReqPut( void                   *pDataParams,
                                 ph_NfcHandle             ConnHandle,
                                 phNfc_sData_t           *pPutData,
                                 pphnpSnep_Fri_ReqCb_t    fCbPut,
                                 void                    *cbContext
                                 )

{
    phStatus_t status = PH_ERR_SUCCESS;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_ClientReqPut");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(fCbPut);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(cbContext);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (fCbPut);
    PH_ASSERT_NULL (cbContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_NP_SNEP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHNP_SNEP_FRI
    case PHNP_SNEP_FRI_ID:

    status =  phnpSnep_Fri_ClientReqPut( ((phnpSnep_Fri_DataParams_t *) pDataParams),
                                         ConnHandle,
                                         pPutData,
                                         fCbPut,
                                         cbContext
                                       );

        break;
#endif /* NXPBUILD__PHNP_SNEP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
        break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status ;

}

/* SNEP Client GET request API */
phStatus_t phnpSnep_ClientReqGet( void                    *pDataParams,
                                  ph_NfcHandle             ConnHandle,
                                  phNfc_sData_t           *pGetData,
                                  uint32_t                 Acceptable_length,
                                  pphnpSnep_Fri_ReqCb_t    fCbGet,
                                  void                    *cbContext)
{
    phStatus_t status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_ClientReqGet");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(fCbGet);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(cbContext);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (fCbGet);
    PH_ASSERT_NULL (cbContext);
    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHNP_SNEP_FRI
    case PHNP_SNEP_FRI_ID:

    status = phnpSnep_Fri_ClientReqGet( (phnpSnep_Fri_DataParams_t *) pDataParams,
                                         ConnHandle,
                                         pGetData,
                                         Acceptable_length,
                                         fCbGet,
                                         cbContext);

        break;
#endif /* NXPBUILD__PHNP_SNEP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

/* SNEP Client De-Initialization API */
phStatus_t phnpSnep_Client_DeInit( void           *pDataParams,
                                  ph_NfcHandle    ConnHandle
                                  )
{
    phStatus_t status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_Client_DeInit");
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    status =  phnpSnep_Fri_Client_DeInit( (phnpSnep_Fri_DataParams_t *) pDataParams,
                                          ConnHandle
                                         );
    return status;
}


//================================= SNEP SERVER ===========================================================

/* SNEP Server Initialization API */
phStatus_t phnpSnep_Server_Init( void                          *pDataParams,
                                phnpSnep_Fri_Config_t          *pConfigInfo,
                                pphnpSnep_Fri_ConnectCB_t       pConnCb,
                                ph_NfcHandle                   *pServerHandle,
                                phnpSnep_Fri_ServerSession_t   *pServerSession,
                                void                           *pContext)
{
    phStatus_t status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_ClientReqGet");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pConfigInfo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pConnCb);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pConnCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_NP_SNEP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHNP_SNEP_FRI
    case PHNP_SNEP_FRI_ID:

    status = phnpSnep_Fri_Server_Init( (phnpSnep_Fri_DataParams_t *) pDataParams,
                                       pConfigInfo,
                                       pConnCb,
                                       pServerHandle,
                                       pServerSession,
                                       pContext
                                     );
        break;
#endif /* NXPBUILD__PHNP_SNEP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}


/* SNEP Server Accept Connection API */
phStatus_t phnpSnep_Server_Accept( void                             *pDataParams,
                                  phNfc_sData_t                     *pDataInbox,
                                  phlnLlcp_Fri_Transport_sSocketOptions_t   *pSockOps,
                                  ph_NfcHandle                       ServerHandle,
                                  ph_NfcHandle                       ConnHandle,
                                  pphnpSnep_Fri_Put_ntf_t            pPutNtfCb,
                                  pphnpSnep_Fri_Get_ntf_t            pGetNtfCb,
                                  void                              *pContext )
{
    phStatus_t    status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_Server_Accept");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataInbox);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pSockOps);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(ServerHandle);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pPutNtfCb);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pGetNtfCb);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pContext);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pDataInbox);
    PH_ASSERT_NULL (pSockOps);
    PH_ASSERT_NULL (ServerHandle);
    PH_ASSERT_NULL (pPutNtfCb);
    PH_ASSERT_NULL (pGetNtfCb);
    PH_ASSERT_NULL (pContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_NP_SNEP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHNP_SNEP_FRI
    case PHNP_SNEP_FRI_ID:

    status = phnpSnep_Fri_Server_Accept( pDataParams,
                                         pDataInbox,
                                         pSockOps,
                                         ServerHandle,
                                         ConnHandle,
                                         pPutNtfCb,
                                         pGetNtfCb,
                                         pContext );
        break;
#endif /* NXPBUILD__PHNP_SNEP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);


    return status;
}



/* SNEP Server De-Initialization API */
phStatus_t phnpSnep_Server_DeInit ( void           *pDataParams,
                                    ph_NfcHandle    ServerHandle )
{
    phStatus_t status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_Server_DeInit");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_NP_SNEP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHNP_SNEP_FRI
    case PHNP_SNEP_FRI_ID:

    status = phnpSnep_Fri_Server_DeInit( pDataParams,
                                         ServerHandle );

        break;
#endif /* NXPBUILD__PHNP_SNEP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

/* SNEP Server Server Send Response API */
phStatus_t phnpSnep_ServerSendResponse( void            *pDataParams,
                                        ph_NfcHandle     ConnHandle,
                                        phNfc_sData_t   *pResponseData,
                                        phStatus_t       responseStatus,
                                        ph_NfcHandle     fSendCompleteCb,
                                        void            *cbContext
                                      )
{
    phStatus_t status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phnpSnep_ServerSendResponse");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pResponseData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(responseStatus);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(fSendCompleteCb);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(cbContext);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pResponseData);
    PH_ASSERT_NULL (responseStatus);
    PH_ASSERT_NULL (fSendCompleteCb);
    PH_ASSERT_NULL (cbContext);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_NP_SNEP)
    {
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHNP_SNEP_FRI
    case PHNP_SNEP_FRI_ID:

    status = phnpSnep_Fri_ServerSendResponse( (phnpSnep_Fri_DataParams_t *) pDataParams,
                                               ConnHandle,
                                              (phNfc_sData_t *) pResponseData,
                                               responseStatus,
                                              (pphnpSnep_Fri_Protocol_SendRspCb_t) fSendCompleteCb,
                                              (void *) cbContext
                                             );

        break;
#endif /* NXPBUILD__PHNP_SNEP_FRI */

    default:
        status = PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_NP_SNEP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}


#endif /* NXPBUILD__PHNP_SNEP */

