#ifndef PHALT1T_H
#define PHALT1T_H

#include <ph_Status.h>

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

#define __DEBUG
#ifdef NXPBUILD__PHAL_T1T_SW

/** \defgroup phalT1T_Sw Component : Software
* @{
*/

#define PHAL_T1T_SW_ID      0x01U                                    /**< ID for Software T1T layer */

/**
* \brief Private parameter structure
*/
typedef struct
{
    uint16_t wId;                                                    /**< Layer ID for this component, NEVER MODIFY! */
    void * pPalI14443p3aDataParams;                                  /**< Pointer to pal parameter structure. */
    uint8_t abHR[2];                                                 /**< Header Rom bytes. */
    uint8_t abUid[4];                                                /**< UID of selected tag. */
} phalT1T_Sw_DataParams_t;

/**
* \brief Initialise this layer.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalT1T_Sw_Init(
    phalT1T_Sw_DataParams_t * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
    uint16_t wSizeOfDataParams,              /**< [In] Specifies the size of the data parameter structure */
    void * pPalI14443p3aDataParams           /**< [In] Pointer to the parameter structure of the underlying ISO14443-3 layer. */
    );

    /** @} */

#ifdef __DEBUG
/* This API would be available only for debugging purpose and test bench */
/**
* \brief   Assigns the component data paramter to one of the filed in 
*          T1T parameter based on the option
*
* \return  Error code
* \retval  #PH_ERR_INVALID_PARAMTER  If invalid option was passed
* \retval  #PH_ERR_SUCCESS           on success
*
*/
phStatus_t phalT1T_SetPtr(
    void * pDataParams,                  /**< [In] Pointer to this layer's paramter structure */
    void * pI14443p3a/**< [In] Pointer to 14443a paramter structure */
    );
#endif /* __DEBUG */

#endif /* NXPBUILD__PHAL_T1T_SW */

#ifdef NXPBUILD__PHAL_T1T
/** \defgroup phalT1T Type 1 tag
* \brief These Components implement the Type 1 Tag commands.
* @{
*/

/**
* \brief Perform ISO14443-4A Request A commands.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_RequestA(
    void * pDataParams,                      /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pAtqa                          /**< [Out] AtqA; uint8_t[2]. */
    ) ;

/**
* \brief Perform Jewel/Topaz Read UID command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_ReadUID(
    void    * pDataParams,                     /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pUid,                            /**< [Out] Known Uid , 4 bytes. */
    uint16_t * pLength                         /**< [Out] Number of received data bytes. */
    ) ;

/**
* \brief Perform Jewel/Topaz Read All command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_ReadAll(
    void * pDataParams,                        /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                            /**< [In]  Known Uid , 4 bytes. */
    uint8_t * pData,                           /**< [Out] pData containing data returned from the Picc. */
    uint16_t * pLength                         /**< [Out] Number of received data bytes. */
    ) ;

/**
* \brief Perform Jewel/Topaz Read Byte command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_ReadByte(
    void * pDataParams,                       /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                           /**< [In]  Known Uid , 4 bytes. */
    uint8_t   bAddress,                       /**< [In]  Address of a byte on Picc to read from. */
    uint8_t * pData,                          /**< [Out] pData containing data returned from the Picc. */
    uint16_t * pLength                        /**< [Out] Number of received data bytes. */
    ) ;

/**
* \brief Perform Jewel/Topaz Write Erase Byte command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_WriteEraseByte(
    void * pDataParams,                 /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                     /**< [In]  Known Uid , 4 bytes. */
    uint8_t   bAddress,                 /**< [In]  Address of a byte on Picc to write to. */
    uint8_t   bTxData,                  /**< [In]  pData containing data to be written to the Picc.*/
    uint8_t * pRxData,                  /**< [Out] pRxData containing data returned from the Picc. */
    uint16_t * pLength                  /**< [Out] Number of received data bytes. */
    );

/**
* \brief Perform Jewel/Topaz Write No Erase Byte command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_WriteNoEraseByte(
    void * pDataParams,               /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                   /**< [In]  Known Uid , 4 bytes. */
    uint8_t   bAddress,               /**< [In]  Address of a byte on Picc to write to. */
    uint8_t   bTxData,                /**< [In]  pData containing data to be written to the Picc. */
    uint8_t * pRxData,                /**< [Out] pRxData containing data returned from the Picc.*/
    uint16_t * pLength                /**< [Out] Number of received data bytes. */
    );

/**
* \brief Perform Jewel/Topaz Read Segment command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_ReadSegment(
    void * pDataParams,                    /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                        /**< [In]  Known Uid , 4 bytes. */
    uint8_t   bAddress,                    /**< [In]  Address of a segment on Picc to read from. */
    uint8_t * pData,                       /**< [Out] pData containing data returned from the Picc. */
    uint16_t * pLength                     /**< [Out] Number of received data bytes. */
    );

/**
* \brief Perform Jewel/Topaz Read Block command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_ReadBlock(
    void * pDataParams,                      /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                          /**< [In]  Known Uid , 4 bytes. */
    uint8_t   bAddress,                      /**< [In]  Address of a block on Picc to read from. */
    uint8_t * pData,                         /**< [Out] pData containing data returned from the Picc. */
    uint16_t * pLength                       /**< [Out] Number of received data bytes. */
    );

/**
* \brief Perform Jewel/Topaz Write Erase Block command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_WriteEraseBlock(
    void * pDataParams,                /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                    /**< [In]  Known Uid , 4 bytes. */
    uint8_t   bAddress,                /**< [In]  Address of a block on Picc to write to. */
    uint8_t * pTxData,                 /**< [In]  pTxData containing data to be written to the Picc. */
    uint8_t * pRxData,                 /**< [Out] pRxData containing data returned from the Picc. */
    uint16_t * pLength                 /**< [Out] Number of received data bytes. */
    );

/**
* \brief Perform Jewel/Topaz Write No Erase Block command.
*
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval #PH_ERR_FRAMING_ERROR Bcc invalid.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phalT1T_WriteNoEraseBlock(
    void * pDataParams,              /**< [In]  Pointer to this layer's parameter structure. */
    uint8_t * pUid,                  /**< [In]  Known Uid , 4 bytes. */
    uint8_t   bAddress,              /**< [In]  Address of a block on Picc to write to. */
    uint8_t * pTxData,               /**< [In]  pTxData containing data to be written to the Picc. */
    uint8_t * pRxData,               /**< [Out] pRxData containing data returned from the Picc. */
    uint16_t * pLength               /**< [Out] Number of received data bytes. */
    ) ;
/** @} */

#endif /* NXPBUILD__PHAL_T1T */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHALT1T_H */
