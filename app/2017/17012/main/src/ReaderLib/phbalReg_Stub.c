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
* BAL Stub Component of Reader Library Framework.
* $Author: christopher.hubmann $
* $Revision: 1.17 $
* $Date: Thu Jan 20 14:43:02 2011 $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#include <ph_NxpBuild.h>

#ifdef NXPBUILD__PHBAL_REG_STUB
//#include <driver_config.h>
//#include <ssp.h>
//#include <gpio.h>
//#include <i2c.h>
//#include <i2cx.h>

#include <ph_Status.h>
#include <phbalReg.h>
#include <ph_RefDefs.h>
#include "phhwConfig.h"

#include <comps\comps\phbalReg\src\Stub\phbalReg_Stub.h>

#define PHBAL_REG_HAL_HW_RC523              0x0000U     /**< Rc523 HW */
#define PHBAL_REG_HAL_HW_RC663              0x0001U     /**< Rc663 HW */

/**********************************************************************************************/
phStatus_t phbalReg_Stub_Init(
                              phbalReg_Stub_DataParams_t * pDataParams,
                              uint16_t wSizeOfDataParams
                              )
{
    if (sizeof(phbalReg_Stub_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_BAL);
    }
    PH_ASSERT_NULL (pDataParams);

    pDataParams->wId = PH_COMP_BAL | PHBAL_REG_STUB_ID;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
}

/**********************************************************************************************/
phStatus_t phbalReg_Stub_GetPortList(
    phbalReg_Stub_DataParams_t * pDataParams,
    uint16_t wPortBufSize,
    uint8_t * pPortNames,
    uint16_t * pNumOfPorts
    )
{
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
}

/**********************************************************************************************/
phStatus_t phbalReg_Stub_SetPort(
                            phbalReg_Stub_DataParams_t * pDataParams,
                            uint8_t * pPortName
                            )
{
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
}

/**********************************************************************************************/
phStatus_t phbalReg_Stub_OpenPort(
                                  phbalReg_Stub_DataParams_t * pDataParams
                                  )
{
//#ifdef SPI_USED
//
//
//#ifndef TUSA
//	/* Config. the SPI pins */
//	SSP_IOConfig();
//	/* Select the SSP for SPI interface */
//	SSP_Init();
//#else /* TUSA */
//	SSP_Emul_GPIO_Config();
//#endif /* TUSA */
//#endif /* SPI_USED */
//
//#ifdef I2C_USED
//    I2CInit(I2CMASTER);
//#endif /* I2C_USED */
//    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
}

/**********************************************************************************************/
phStatus_t phbalReg_Stub_ClosePort(
                                   phbalReg_Stub_DataParams_t * pDataParams
                                   )
{
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
}

/**********************************************************************************************/
phStatus_t phbalReg_Stub_Exchange(
                                  phbalReg_Stub_DataParams_t * pDataParams,
                                  uint16_t wOption,
                                  uint8_t * pTxBuffer,
                                  uint16_t wTxLength,
                                  uint16_t wRxBufSize,
                                  uint8_t * pRxBuffer,
                                  uint16_t * pRxLength
                                  )
{
#ifdef SPI_USED
	uint16_t xferLen;
    SSP_DATA_SETUP_Type xferConfig;

    xferConfig.length = wTxLength;
    xferConfig.rx_data = pRxBuffer;
    xferConfig.tx_data = pTxBuffer;

//    GPIOSetValue(PORT0, PIN_SSEL, SSEL_ASR);  // CS on
//#ifndef TUSA
    xferLen = SSP_ReadWrite (&xferConfig);
//#else /* TUSA */
//    xferLen = SSP_Emul_GPIO_ReadWrite (LPC_GPIO0, &xferConfig);
//#endif /* TUSA */
//    GPIOSetValue(PORT0, PIN_SSEL, SSEL_DEASR); // CS off

    if (xferLen != wTxLength)
    {
        return PH_ADD_COMPCODE(PH_ERR_INTERFACE_ERROR, PH_COMP_BAL);
    }

    *pRxLength = xferLen;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
#endif /* SPI_USED */

#ifdef I2C_USED

    I2C_M_SETUP_Type transferMCfg;

    /* Only 7Bits of slave address should be initialized as Last bit(R/W) will be appended automatically
     * based on the operation done. */
    transferMCfg.sl_addr7bit		= 0x28;
    transferMCfg.tx_data			= pTxBuffer;
    transferMCfg.tx_length			= (uint32_t)wTxLength;
    transferMCfg.rx_data			= pRxBuffer;
    transferMCfg.rx_length			= (uint32_t)wRxBufSize;
    transferMCfg.retransmissions_max = 3;
    if(SUCCESS != I2C_MasterTransferData(&transferMCfg))
    {
    	return PH_ADD_COMPCODE(PH_ERR_INTERFACE_ERROR, PH_COMP_BAL);
    }

    if(pRxBuffer != NULL && pRxLength != NULL)
    {
    	*pRxLength = (uint16_t)(transferMCfg.rx_count & 0x0000FFFF);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);

#endif /* I2C_USED */
}

/**********************************************************************************************/
phStatus_t phbalReg_Stub_SetConfig(
                                   phbalReg_Stub_DataParams_t * pDataParams,
                                   uint16_t wConfig,
                                   uint16_t wValue
                                   )
{
    switch (wConfig)
    {

    case PHBAL_REG_CONFIG_HAL_HW_TYPE:
        if((wValue != PHBAL_REG_HAL_HW_RC523) && (wValue != PHBAL_REG_HAL_HW_RC663))
        {
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_BAL);
        }
        pDataParams->wHalType = (uint8_t)wValue;

        break;

    default:
        return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_BAL);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
}

/**********************************************************************************************/
phStatus_t phbalReg_Stub_GetConfig(
                                   phbalReg_Stub_DataParams_t * pDataParams,
                                   uint16_t wConfig,
                                   uint16_t * pValue
                                   )
{
    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_BAL);
}

#endif /* NXPBUILD__PHBAL_REG_STUB */
