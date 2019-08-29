
#ifndef PHALT1T_SW_H
#define PHALT1T_SW_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

#define PHAL_T1T_READUID_RESP_LEN              8U                       /**< T1T RID response len */

#define PHAL_T1T_READALL_RESP_LEN              124U                     /**< T1T RALL response len */
#define PHAL_T1T_READBYTE_RESP_LEN             4U                       /**< T1T Read response len */
#define PHAL_T1T_WRITEERASEBYTE_RESP_LEN       4U                       /**< T1T Write_E response len */
#define PHAL_T1T_WRITENOERASEBYTE_RESP_LEN     4U                       /**< T1T Write_NE response len */

#define PHAL_T1T_READSEG_RESP_LEN              131U                     /**< T1T RSEG response len */
#define PHAL_T1T_READBLOCK_RESP_LEN            11U                      /**< T1T Read8 response len */
#define PHAL_T1T_WRITEERASEBLOCK_RESP_LEN      11U                      /**< T1T Write_E8 response len */
#define PHAL_T1T_WRITENOERASEBLOCK_RESP_LEN    11U                      /**< T1T Write_NE8 response len */

#define PHAL_T1T_CMD_READUID                   0x78U                    /**< T1T RID command byte */

#define PHAL_T1T_CMD_READALL                   0x00U                    /**< T1T RALL command byte */
#define PHAL_T1T_CMD_READBYTE                  0x01U                    /**< T1T Read command byte */
#define PHAL_T1T_CMD_WRITEERASEBYTE            0x53U                    /**< T1T Write_E8 command byte */
#define PHAL_T1T_CMD_WRITENOERASEBYTE          0x1AU                    /**< T1T Write_NE8 command byte */

#define PHAL_T1T_CMD_READSEG                   0x10U                    /**< T1T RID command byte */
#define PHAL_T1T_CMD_READBLOCK                 0x02U                    /**< T1T Read8 command byte */
#define PHAL_T1T_CMD_WRITEERASEBLOCK           0x54U                    /**< T1T Write_E8 command byte */
#define PHAL_T1T_CMD_WRITENOERASEBLOCK         0x1BU                    /**< T1T Write_NE8 command byte */

/** @} */

phStatus_t phalT1T_Sw_RequestA  (
                                 phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                 uint8_t * pAtqa                            /**< [Out] */
                                 );

phStatus_t phalT1T_Sw_ReadUID  (
                                phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                uint8_t * pUid,                            /**< [Out */
                                uint16_t * pLength                         /**< [Out] */
                                );

phStatus_t phalT1T_Sw_ReadAll  (
                                phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                uint8_t * pUid,                            /**< [In] */
                                uint8_t * pData,                           /**< [Out] */
                                uint16_t * pLength                         /**< [Out] */
                                );

phStatus_t phalT1T_Sw_ReadByte  (
                                 phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                 uint8_t * pUid,                            /**< [In] */
                                 uint8_t   bAddress,                        /**< [In] */
                                 uint8_t * pData,                           /**< [Out] */
                                 uint16_t * pLength                         /**< [Out] */
                                 );

phStatus_t phalT1T_Sw_WriteEraseByte  (
                                       phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                       uint8_t * pUid,                            /**< [In] */
                                       uint8_t   bAddress,                        /**< [In] */
                                       uint8_t   bTxData,                         /**< [In] */
                                       uint8_t * pRxData,                         /**< [Out] */
                                       uint16_t * pLength                         /**< [Out] */
                                       );

phStatus_t phalT1T_Sw_WriteNoEraseByte  (
    phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
    uint8_t * pUid,                            /**< [In] */
    uint8_t   bAddress,                        /**< [In] */
    uint8_t   bTxData,                         /**< [In] */
    uint8_t * pRxData,                         /**< [Out] */
    uint16_t * pLength                         /**< [Out] */
    );

phStatus_t phalT1T_Sw_ReadSegment  (
                                    phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                    uint8_t * pUid,                            /**< [In] */
                                    uint8_t   bAddress,                        /**< [In] */
                                    uint8_t * pData,                           /**< [Out] */
                                    uint16_t * pLength                         /**< [Out] */
                                    );

phStatus_t phalT1T_Sw_ReadBlock  (
                                  phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                  uint8_t * pUid,                            /**< [In] */
                                  uint8_t   bAddress,                        /**< [In] */
                                  uint8_t * pData,                           /**< [Out] */
                                  uint16_t * pLength                         /**< [Out] */
                                  );

phStatus_t phalT1T_Sw_WriteEraseBlock  (
                                        phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
                                        uint8_t * pUid,                            /**< [In] */
                                        uint8_t   bAddress,                        /**< [In] */
                                        uint8_t * pTxData,                         /**< [In] */
                                        uint8_t * pRxData,                         /**< [Out] */
                                        uint16_t * pLength                         /**< [Out] */
                                        );

phStatus_t phalT1T_Sw_WriteNoEraseBlock  (
    phalT1T_Sw_DataParams_t * pDataParams,     /**< [In] */
    uint8_t * pUid,                            /**< [In] */
    uint8_t   bAddress,                        /**< [In] */
    uint8_t * pTxData,                         /**< [In] */
    uint8_t * pRxData,                         /**< [Out] */
    uint16_t * pLength                         /**< [Out] */
    );

#ifdef __DEBUG

phStatus_t phalT1T_Sw_SetPtr(
                             phalT1T_Sw_DataParams_t * pDataParams,
                             void * pI14443p3a
                             );
#endif /* __DEBUG */

#endif /* PHALT1T_SW_H */
