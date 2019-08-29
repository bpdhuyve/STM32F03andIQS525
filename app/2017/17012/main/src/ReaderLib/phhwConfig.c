/*==============================================================================================
 *         Copyright (c), NXP Semiconductors
 *
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 *particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 */

/*==============================================================================================
 *
 *   File name:  hw_config.c
 *
 *  Created on:
 *      Author:
 *
 *     Hystory:
 */

/*----------------------------------------------------------------------------------------------
 * Includes
 ---------------------------------------------------------------------------------------------*/

#include <ph_TypeDefs.h>
#include <ph_Status.h>

//#include <gpio.h>
//#include <ssp.h>
//#include <i2c.h>

#include <ph_NxpBuild.h>
#include <phbalReg.h>
#include <phhalHw.h>

#include "phhwConfig.h"

#include <phOsal.h>
#include <phpalI18092mT.h>
#include <phlnLlcp.h>
#include <phhalHw_Rc523_Reg.h>


/*----------------------------------------------------------------------------------------------
 * Local macros and definitions
 ---------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------
 * Global variables
 * -------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------
 * Local variables
 * -------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------
 * Local function declaration
 ---------------------------------------------------------------------------------------------*/
#if defined (NXPBUILD__PHHAL_HW_RC523) && defined (NXPBUILD__PHHAL_HW_RC663)
#error ** Mismatch reader device - PN512 and RC663 are alloved together **
#endif


/*----------------------------------------------------------------------------------------------
 * Global function prototypes
 ---------------------------------------------------------------------------------------------*/
/* SSP Status Implementation definitions */
#define SSP_STAT_DONE		(1UL<<8)		/**< Done */
#define SSP_STAT_ERROR		(1UL<<9)		/**< Error */

/* **************************************
 * Initialize interface link            *
 * **************************************/

/* =============================================================================================
 * Function:	Set_Interface_Link
 *
 * brief:
 *   Initialize interface link
 *   GPIOSetDir sets the direction in GPIO port.
 *   1 out, 0 input
 *
 *   GPIOSetValue sets/clears a bitvalue in a specific bit position
 *   in GPIO portX(X is the port number.)
 *   Our RC523 will act in slave mode. START and STOP conditions are
 *   generated by the master.
 *
 * -------------------------------------------------------------------------------------------*/
//void Set_Interface_Link(void)
//    {
//	GPIOInit();
//#if defined NXPBUILD__PHHAL_HW_RC523
//#ifdef SPI_USED
//	/* not any pinout setting from reason the PNEV512B has interface setting realized
//	 * by soldere resistors on the PCB */
//#endif /* SPI_USED */
//#ifdef I2C_USED
//	/* Select PIO function of the pins */
//	LPC_IOCON->R_PIO0_31 |= 0x01;
//	LPC_IOCON->R_PIO0_30 |= 0x01;
//	LPC_IOCON->R_PIO1_0  |= 0x01;
//
//	/* Set direction of pins for I2C address */
//	GPIOSetDir(PORT0, 31, SET_OUT);
//	GPIOSetDir(PORT0, 30, SET_OUT);
//	GPIOSetDir(PORT1,  0, SET_OUT);
//
//	/* Set PN512 I2C address to 0x50 */
//	GPIOSetValue(PORT0, 31, SET_LOW);  		// 16 ADR0 = D5
//	GPIOSetValue(PORT0, 30, SET_LOW);  		// 15 ADR1 = D6
//	GPIOSetValue(PORT1,  0, SET_LOW);      	// 17 ADR2 = D4
//#endif /* I2C_USED */
//#endif /* NXPBUILD__PHHAL_HW_RC523 */
//
//#if defined NXPBUILD__PHHAL_HW_RC663
//#ifdef SPI_USED
//	#ifndef TUSA
//	/* Set port pin P0.21 - IFSEL0 to output */
//	GPIOSetDir(PORT0, PIN_IFSEL0, SET_OUT );
//
//	/* Set port pin P0.24 - IFSEL1 to output */
//	GPIOSetDir(PORT0, PIN_IFSEL1, SET_OUT );
//
//	/* Select SPI link -> IFSEL0 = 0 & IFSEL1 = 1 */
//	/* IFSEL0 = 0 */
//	GPIOSetValue(PORT0, PIN_IFSEL0, SET_LOW);
//
//	/* IFSEL1 = 1 */
//	GPIOSetValue(PORT0, PIN_IFSEL1, SET_HIGH);
//	#endif /* TUSA */
//
//#endif /* SPI_USED */
//
//#ifdef I2C_USED
//    /* Set port pin P0.21 - IFSEL0 to output */
//    GPIOSetDir(PORT0, PIN_IFSEL0, SET_OUT );
//
//    /* Set port pin P0.24 - IFSEL1 to output */
//    GPIOSetDir(PORT0, PIN_IFSEL1, SET_OUT );
//
//    /* Select I2C link -> IFSEL0 = 1 & IFSEL1 = 0 */
//    /* IFSEL0 = 1 */
//    GPIOSetValue(PORT0, PIN_IFSEL0, SET_HIGH );
//
//    /* IFSEL1 = 0 */
//    GPIOSetValue(PORT0, PIN_IFSEL1, SET_LOW);
//
//    /* Set port pin P0.3 - I2C address bit AD0 to output */
//    GPIOSetDir(PORT0, PIN_AD0, SET_OUT);
//
//    /* Set port pin P0.9 - I2C address bit AD1 to output */
//    GPIOSetDir(PORT0, PIN_AD1, SET_OUT);
//
//    /* Select I2C adresss -> AD0 = AD1 = 0 */
//    /* AD0 = 0 */
//    GPIOSetValue(PORT0, PIN_AD0, ADDR_CLEAR);
//
//    /* AD1 = 0 */
//    GPIOSetValue(PORT0, PIN_AD1, ADDR_CLEAR);
//#endif /* I2C_USED */
//#endif /* NXPBUILD__PHHAL_HW_RC663 */
//

//    }

/* **************************************
 * handle Reset of the reader chip      *
 * **************************************/

/*==============================================================================================
 * Function:   Reset_reader_device
 *
 * brief:   reset the reader chip
 *
 * -------------------------------------------------------------------------------------------*/
//void Reset_reader_device(void)
//{
//    uint32_t volatile i;
//
//#ifdef NXPBUILD__PHHAL_HW_RC523
//
//    /* Set port pin - PDOWN to output */
//    GPIOSetDir(PORT0, PIN_RESET, SET_OUT );
//
//    /* send the reset pulse 1-0-1 to reset device */
//    /* RSET signal high - to '1' */
//    GPIOSetValue(PORT0, PIN_RESET, SET_HIGH);
//
//    /* delay of ~1,2 ms */
//    for (i = 0x1000; i > 0; i --);
//
//    /* RSET signal low - to '0' */
//    GPIOSetValue(PORT0, PIN_RESET, SET_LOW);
//
//    /* delay of ~1,2 ms */
//    for (i = 0x1000; i > 0; i --);
//
//    /* RSET signal high - to '1' */
//    GPIOSetValue(PORT0, PIN_RESET, SET_HIGH);
//
//    /* delay of ~1,2 ms */
//    for (i = 0x1000; i > 0; i --);
//
//#endif /* NXPBUILD__PHHAL_HW_RC523 */
//#ifdef NXPBUILD__PHHAL_HW_RC663
//
//	/* Set port pin - PDOWN to output */
//	GPIOSetDir(PORT0, PIN_RESET, SET_OUT );
//
//	/* send the reset pulse 0-1-0 to reset device */
//	/* RSET signal low - PDOWN to '0' */
//	GPIOSetValue(PORT0, PIN_RESET, SET_LOW);
//
//	/* delay of ~1,2 ms */
//	for (i = 0x1000; i > 0; i --);
//
//	/* RSET signal high to reset the RC663 IC - PDOWN to '1' */
//	GPIOSetValue(PORT0, PIN_RESET, SET_HIGH);
//
//	/* delay of ~1,2 ms */
//	for (i = 0x1000; i > 0; i --);
//
//	/* RSET signal low - PDOWN to '0' */
//	GPIOSetValue(PORT0, PIN_RESET, SET_LOW);
//
//	/* delay of ~1,2 ms */
//	for (i = 0x1000; i > 0; i --);
//#endif /* NXPBUILD__PHHAL_HW_RC663 */
//}
///*==============================================================================================
// * Function:	Set_Port
// *
// * brief:
// *
// * -------------------------------------------------------------------------------------------*/
//void Set_Port(void)
//{
//#ifndef TUSA
//	/* Set LED port pin to output */
//	GPIOSetDir(LED_PORT, LED_BIT, SET_OUT);
//#endif
//}
/*==============================================================================================
 * Function:	SSP_ReadWrite
 *
 * brief 		SSP Read write data function
 * param[in]	SSPx 	Pointer to SSP peripheral, should be
 * 						- LPC_SSP0: SSP0 peripheral
 * 						- LPC_SSP1: SSP1 peripheral
 * param[in]	dataCfg	Pointer to a SSP_DATA_SETUP_Type structure that
 * 						contains specified information about transmit
 * 						data configuration.
 * param[in]	xfType	Transfer type, should be:
 * 						- SSP_TRANSFER_POLLING: Polling mode
 * 						- SSP_TRANSFER_INTERRUPT: Interrupt mode
 * return 		Actual Data length has been transferred in polling mode.
 * 				In interrupt mode, always return (0)
 * 				Return (-1) if error.
 *
 * Note: This function can be used in both master and slave mode.
 *
 * -------------------------------------------------------------------------------------------*/
int32_t SSP_ReadWrite (SSP_DATA_SETUP_Type *dataCfg)
    {
    volatile uint8_t *rdata8 = NULL;
    volatile uint8_t *wdata8 = NULL;
    volatile uint16_t *rdata16 = NULL;
    volatile uint16_t *wdata16 = NULL;
    volatile uint32_t stat;
    volatile uint32_t tmp;
    volatile int32_t dataword;

    dataCfg->rx_cnt = 0;
    dataCfg->tx_cnt = 0;
    dataCfg->status = 0;

    rdata8 = (uint8_t *)dataCfg->rx_data;
    wdata8 = (uint8_t *)dataCfg->tx_data;

    AppRf_SpiReadWrite((uint8_t*) dataCfg->tx_data,(uint8_t*) dataCfg->rx_data ,(uint8_t)dataCfg->length);

    dataCfg->rx_cnt = dataCfg->length;
    dataCfg->tx_cnt = dataCfg->length;
    
    // save status
    dataCfg->status = SSP_STAT_DONE;

    return dataCfg->tx_cnt;

    }
/*----------------------------------------------------------------------------------------------
 * End of file
 ---------------------------------------------------------------------------------------------*/

