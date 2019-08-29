/***********************************************************************************************
 *   config.h:  config file for example for NXP LPC122x Family
 *   Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2012.07.20  ver 1.00    First Release
 *
 ***********************************************************************************************/


/*----------------------------------------------------------------------------------------------
 * Includes
 ---------------------------------------------------------------------------------------------*/
#ifndef __PHHW_CONFIG_H__
#define __PHHW_CONFIG_H__

//#include <LPC122x.h>
#include <phhalHw.h>

//#define BOARD_PRH601
//#define TUSA

#if defined(BOARD_PRH601) & !defined(NXPBUILD__PHHAL_HW_RC663)
#error ** Mismatch in hardware configuration - PRH601 board includes RC663 reader IC **
#endif

#if defined(TUSA) & !defined(NXPBUILD__PHHAL_HW_RC663)
#error ** Mismatch in hardware configuration - TUSA board includes RC663 reader IC **
#endif
/***********************************************************************************************
 **	Global macros and definitions
 ***********************************************************************************************/
#define SPI_USED
//#define I2C_USED

#if defined (SPI_USED) && defined (I2C_USED)
#error ** Mismatch interface definition - I2C and SPI not alloved together **
#endif

//#ifdef SPI_USED
#define PHHAL_HW_BUS	PHHAL_HW_BAL_CONNECTION_SPI
//#endif /* SPI_USED */
//#ifdef I2C_USED
//	#ifdef TUSA
//	#error ** Mismatch in hardware configuration - I2C interface not supported on TUSA board **
//	#endif /* TUSA */
//	#ifdef BOARD_PRH601
//	#error ** Mismatch in hardware configuration - I2C interface not supported on BOARD_PRH601 board **
//	#endif /* BOARD_PRH601 */
//	#define PHHAL_HW_BUS	PHHAL_HW_BAL_CONNECTION_I2C
//#endif /* I2C_USED */
//
//#define PORT0			  0
//#define PORT1			  1
//#define PORT2			  2
//#define PORT3			  3
//#define	INT_PORT		  PORT0	//
//#define LED_PORT          PORT0     // Port for led
//
//#define SET_OUT           1     // a pin of chip is configured to output direction
//#define SET_IN            0     // a pin of chip is configured to input direction
//#define SET_HIGH          1
//#define SET_LOW           0
//
//#ifndef BOARD_PRH601 // Blueboard
//#define LED_BIT           7     // Bit on port for led
//#define LED_ON            1     // Level to set port to turn on led
//#define LED_OFF           0     // Level to set port to turn off led
//#else // BOARD_PRH601
//#define LED_RED       	  5     // Bit on port 0 for RED color LED
//#define LED_GRN           4     // Bit on port 0 for GREEN color LED
//#define LED_ON            0     // Level to set port to turn on led
//#define LED_OFF           1     // Level to set port to turn off led
//#endif /* BOARD_PRH601 */
/* ------------------------------------------------ */
/* pins definition                                  */
/* ------------------------------------------------ */
#define PIN_LED            LED_BIT     // Bit on port for led
#define PIN_AD0            3
#define PIN_AD1            9

//#ifdef NXPBUILD__PHHAL_HW_RC663
//	#if !defined(TUSA) && !defined(BOARD_PRH601)
//		#define PIN_RESET	   		4      // for reset pin for Blueboard CLEV663B
//		#define PIN_SSEL       		11    // SSEL pin for SSP periphery for Blueboard CLEV663B
//		#define SSP_SSEL	   		PIO0_11
//		#define PIN_IFSEL0          21    // select pin to define com. interface type for RC663
//		#define PIN_IFSEL1	        24    // select pin to define com. interface type for RC663
//	#elif defined(TUSA)
//		#define PIN_RESET   		10		// for TUSA board
//		#define PIN_SSEL			11     	// SSEL pin for SSP periphery
//		#define SSP_SSEL    		PIO0_11	/* SSP SSEL is a GPIO pin */
//
//		#define EMUL_SPI_MOSI		PIO0_7
//		#define EMUL_SPI_MISO		PIO0_3
//		#define EMUL_SPI_CLK		PIO0_9
//		#define EMUL_SPI_SSEL		PIO0_11
//
//		#define EMUL_SPI_MOSI_PIN	7
//		#define EMUL_SPI_MISO_PIN	3
//		#define EMUL_SPI_CLK_PIN	9
//		#define EMUL_SPI_SSEL_PIN	11
//
//		#define EMUL_SPI_MOSI_PORT	0
//		#define EMUL_SPI_MISO_PORT	0
//		#define EMUL_SPI_CLK_PORT	0
//		#define EMUL_SPI_SSEL_PORT	0
//	#elif defined(BOARD_PRH601)
//		#define PIN_IRQ_FROM_663    21	   // interrupt input from RC663 !!! port 0 !!!
//		#define PIN_LED           	LED_RED       // Bit on port for led
//		#define LED_BIT           	PIN_LED       // Bit on port for led
//		#define PIN_SSEL          	15    /* SSEL pin for SSP periphery, but setting is like for
//	 	 	 	 	 	 	 	 	 	   * standard GPIO */
//		#define PIN_IFSEL0		  	18	   // select pin to define com. interface type for RC663
//		#define PIN_IFSEL1		  	19	   // select pin to define com. interface type for RC663
//		#define PIN_RESET	     	20	   // PDOWN - pin for power down/reset of RC663
//		#define SSP_SSEL	   	  	PIO0_15
//	#else /* no board */
//	#endif
//#endif /* NXPBUILD__PHHAL_HW_RC663 */

#ifdef NXPBUILD__PHHAL_HW_RC523
	#define PIN_RESET	   28    // for reset pin for Blueboard PNEV512B
	#define INT_PIN		   29	 // pin that is connected to IRQ pin of attached reader IC
	#define PIN_SSEL       15	 // SSEL pin for SSP periphery for Blueboard PNEV512B
	#define SSP_SSEL	   PIO0_15
#endif /* NXPBUILD__PHHAL_HW_RC523 */

#define SSEL_ASR           0
#define SSEL_DEASR         1
#define ADDR_CLEAR		   0
#define ADDR_SET		   1
#define READER_CHIP_ADDR 0x50


/***********************************************************************************************
 **	Global variables
 ***********************************************************************************************/

/**
 * @brief SPI Data configuration structure definitions
 */
typedef struct {
	void *tx_data;				/**< Pointer to transmit data */
	uint32_t tx_cnt;			/**< Transmit counter */
	void *rx_data;				/**< Pointer to transmit data */
	uint32_t rx_cnt;			/**< Receive counter */
	uint32_t length;			/**< Length of transfer data */
	uint32_t status;			/**< Current status of SSP activity */
} SSP_DATA_SETUP_Type;

///**
// * @brief I2C Data configuration structure definitions
// */
//typedef struct
//{
//	uint8_t *rxData;
//	uint8_t *txData;
//	uint16_t rxLength;
//	uint16_t txLength;
//} I2C_DATA_SETUP_Type;

/***********************************************************************************************
 **	Global function prototypes
 ***********************************************************************************************/
//void Set_Port(void);
//void Set_Interface_Link(void);
//void Reset_reader_device(void);
#ifdef NXPBUILD__PHHAL_HW_RC523
//void appDataInit( void *pHal, void *pOsal, void *pData);
extern int32_t SSP_ReadWrite (SSP_DATA_SETUP_Type *dataCfg);
#endif /* NXPBUILD__PHHAL_HW_RC523 */


/******************************************************************************************
 * Reader IC specific soft reset function - command
 ******************************************************************************************/
#ifdef NXPBUILD__PHHAL_HW_RC523
	#define SoftReset_reader_device(pHal)	phhalHw_Rc523_Cmd_SoftReset(pHal)
#endif
#ifdef NXPBUILD__PHHAL_HW_RC663
	//#define SoftReset_reader_device(pHal)	phhalHw_Rc663_Cmd_SoftReset(pHal)
#endif

#endif	// __PHHW_CONFIG_H__
/***********************************************************************************************
 **                            End Of File
 ***********************************************************************************************/





