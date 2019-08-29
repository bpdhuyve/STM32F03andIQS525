/*
*         Copyright (c), NXP Semiconductors Gratkorn / Austria
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
* Generic BAL Component of Reader Library Framework.
* $Author: nxp40786 $
* $Revision: 467 $
* $Date: 2014-01-16 16:04:47 +0530 (Thu, 16 Jan 2014) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHBALREG_H
#define PHBALREG_H

#include <ph_Status.h>

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */

#ifdef NXPBUILD__PHBAL_REG_STUB

/** \defgroup phbalReg_Stub Component : Stub
* \brief Component without functionality to ease implementation of additional busses.
* @{
*/

#define PHBAL_REG_STUB_ID               0x08U       /**< ID for Stub BAL component */

/**
* \brief BAL Stub parameter structure
*/
typedef struct
{
    uint16_t       wId;         /**< Layer ID for this BAL component, NEVER MODIFY! */
    uint16_t       wHalType;	/**< HAL HW type (Rc523, Rc663, ... ) */
    uint16_t	   ic;			/* 663 or 523*/
    /* TODO: Place private parameters which are needed across different functions here. */

} phbalReg_Stub_DataParams_t;

/**
* \brief Initialise the BAL Stub layer.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS Parameter structure size is invalid.
*/
phStatus_t phbalReg_Stub_Init(
                              phbalReg_Stub_DataParams_t * pDataParams, /**< [In] Pointer to this layer's parameter structure. */
                              uint16_t wSizeOfDataParams                /**< [In] Specifies the size of the data parameter structure. */
                              );

/** @} */
#endif /* NXPBUILD__PHBAL_REG_STUB */

#ifdef NXPBUILD__PHBAL_REG

/** \defgroup phbalReg Bus Abstraction Layer
* \brief These Components implement the interfaces between physical Host-Device and physical Reader-Device.
* @{
*/

/**
* \name Generic BAL Configs
*/
/*@{*/
#define PHBAL_REG_CONFIG_WRITE_TIMEOUT_MS   0x0000U /**< Configure transmission timeout. */
#define PHBAL_REG_CONFIG_READ_TIMEOUT_MS    0x0001U /**< Configure reception timeout. */
#define PHBAL_REG_CONFIG_HAL_HW_TYPE        0x0002U /**< Configure HW type */
#define PHBAL_REG_STUBI2C_CONFIG_IC  		0x0003U       /**< Configure */
/*@}*/

/**
* \name BAL Configuration Values
*/
/*@{*/
#define PHBAL_REG_HAL_HW_RC523              0x0000U     /**< Rc523 HW */
#define PHBAL_REG_HAL_HW_RC663              0x0001U     /**< Rc663 HW */
#define PHBAL_REG_STUBI2C_CONFIG_IC_RC523   0x0000U     /**< peridot 1.5*/
#define PHBAL_REG_STUBI2C_CONFIG_IC_RC663   0x0001U     /**< peridot 2.0 */
/*@}*/

/**
* \brief List all available ports.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_BUFFER_OVERFLOW Given enumeration buffer is too small.
* \retval #PH_ERR_INTERFACE_ERROR Error while enumerating devices.
*/
phStatus_t phbalReg_GetPortList(
                                void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                                uint16_t wPortBufSize,  /**< [In] Buffer Size of Port Name String. */ 
                                uint8_t * pPortNames,   /**< [Out] Port Name as Multi-String. */
                                uint16_t * pNumOfPorts  /**< [Out] Number of found port strings. */ 
                                );

/**
* \brief Select Port to be used.
*
* <em>Example SerialWin:</em> The caller has to ensure that \c pPortName
* is valid throughout the whole lifetime of \c pDataParams.\n
* Furthermore, the caller is responsible for prepending "\\.\" if COM ports above
* COM9 need to be accessed.\n\n
* \b Example:
\code
strcpy(pPortName, "COM9");         <-- correct
strcpy(pPortName, "\\\\.\\COM9");  <-- correct
strcpy(pPortName, "\\\\.\\COM10"); <-- correct
strcpy(pPortName, "COM10");        <-- wrong
\endcode
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phbalReg_SetPort(
                            void * pDataParams, /**< [In] Pointer to this layer's parameter structure. */
                            uint8_t * pPortName /**< [In] Port Name as String. */
                            );

/**
* \brief Open communication port.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_USE_CONDITION Communication port is already open.
* \retval #PH_ERR_INTERFACE_ERROR Error while opening port.
*/
phStatus_t phbalReg_OpenPort(
                             void * pDataParams /**< [In] Pointer to this layer's parameter structure. */
                             );

/**
* \brief Close communication port.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_USE_CONDITION Communication port is not open.
* \retval #PH_ERR_INTERFACE_ERROR Error while closing port.
*/
phStatus_t phbalReg_ClosePort(
                              void * pDataParams    /**< [In] Pointer to this layer's parameter structure. */
                              );

/**
* \brief Perform Data Exchange on the bus.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER \b wOption is invalid.
* \retval #PH_ERR_IO_TIMEOUT No response received within given time frame.
* \retval #PH_ERR_BUFFER_OVERFLOW Response is too big for either given receive buffer or internal buffer.
* \retval #PH_ERR_INTERFACE_ERROR Communication error.
*/
phStatus_t phbalReg_Exchange(
                             void * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                             uint16_t wOption,      /**< [In] Option parameter. */
                             uint8_t * pTxBuffer,   /**< [In] Data to transmit. */
                             uint16_t wTxLength,    /**< [In] Number of bytes to transmit. */
                             uint16_t wRxBufSize,   /**< [In] Size of receive buffer / Number of bytes to receive (depending on implementation). */
                             uint8_t * pRxBuffer,   /**< [Out] Received data. */
                             uint16_t * pRxLength   /**< [Out] Number of received data bytes. */
                             );

/**
* \brief Set configuration parameter.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_UNSUPPORTED_PARAMETER Configuration is not supported or invalid.
* \retval #PH_ERR_INVALID_PARAMETER Parameter value is invalid.
* \retval #PH_ERR_INTERFACE_ERROR Communication error.
*/
phStatus_t phbalReg_SetConfig(
                              void * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                              uint16_t wConfig,     /**< [In] Configuration Identifier. */
                              uint16_t wValue       /**< [In] Configuration Value. */
                              );
/**
* \brief Get configuration parameter.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_UNSUPPORTED_PARAMETER Configuration is not supported or invalid.
* \retval #PH_ERR_INTERFACE_ERROR Communication error.
*/
phStatus_t phbalReg_GetConfig(
                              void * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                              uint16_t wConfig,     /**< [In] Configuration Identifier. */
                              uint16_t * pValue     /**< [Out] Configuration Value. */
                              );

/** @} */
#endif /* NXPBUILD__PHBAL_REG */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHBALREG_H */
