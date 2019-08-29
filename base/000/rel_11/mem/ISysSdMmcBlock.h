/*
 * @brief LPC18xx/43xx SD/SDIO driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// SD Card interface for LPC1830
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ISYS_SD_MMC_BLOCK
#define ISYS_SD_MMC_BLOCK
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/**
 * @brief    Set block size for the transfer
 * @param    port    : SDMMC peripheral selected
 * @param    bytes    : block size in bytes
 * @return    None
 */
void SysSdMmcBlock_SetBlkSize(SDMMC_PORT port, U32 bytes);

/**
 * @brief    Reset card in slot
 * @param    port    : SDMMC peripheral selected
 * @param    reset    : Sets SD_RST to passed state
 * @return    None
 * @note    Reset card in slot, must manually de-assert reset after assertion
 * (Uses SD_RST pin, set per reset parameter state)
 */
void SysSdMmcBlock_Reset(SDMMC_PORT port, S32 reset);

/**
 * @brief    Detect if an SD card is inserted
 * @param    port    : SDMMC peripheral selected
 * @return    Returns TRUE if a card is detected, otherwise FALSE
 * @note    Detect if an SD card is inserted
 */
BOOL SysSdMmcBlock_CardDetected(SDMMC_PORT port);

/**
 * @brief    Detect if write protect is enabled
 * @param    port    : SDMMC peripheral selected
 * @return    Returns 1 if card is write protected, otherwise 0
 * @note    Detect if write protect is enabled
 * (uses SD_WP pin, returns 1 if card is write protected)
 */
S32 SysSdMmcBlock_CardWpOn(SDMMC_PORT port);

/**
 * @brief    Disable slot power
 * @param    port    : SDMMC peripheral selected
 * @return    None
 * @note    Uses SD_POW pin, set to low.
 */
void SysSdMmcBlock_PowerOff(SDMMC_PORT port);

/**
 * @brief    Enable slot power
 * @param    port    : SDMMC peripheral selected
 * @return    None
 * @note    Uses SD_POW pin, set to high.
 */
void SysSdMmcBlock_PowerOn(SDMMC_PORT port);

/**
 * @brief    Function to set card type
 * @param    port    : SDMMC peripheral selected
 * @param    ctype    : card type
 * @return    None
 */
void SysSdMmcBlock_SetCardType(SDMMC_PORT port, U32 ctype);

/**
 * @brief    Returns the raw SD interface interrupt status
 * @param    port    : SDMMC peripheral selected
 * @return    None
 */
U32 SysSdMmcBlock_GetIntStatus(SDMMC_PORT port);

/**
 * @brief    Clears the raw SD interface interrupt status
 * @param    port    : SDMMC peripheral selected
 * @param    iVal    : Interrupts to be cleared, Or'ed values MCI_INT_*
 * @return    None
 */
void SysSdMmcBlock_ClrIntStatus(SDMMC_PORT port, U32 iVal);

/**
 * @brief    Sets the SD interface interrupt mask
 * @param    port    : SDMMC peripheral selected
 * @param    iVal    : Interrupts to enable, Or'ed values MCI_INT_*
 * @return    None
 */
void SysSdMmcBlock_SetIntMask(SDMMC_PORT port, U32 iVal);

/**
 * @brief    Set block size and byte count for transfer
 * @param    port    : SDMMC peripheral selected
 * @param    blk_size: block size and byte count in bytes
 * @return    None
 */
void SysSdMmcBlock_SetBlkSizeByteCnt(SDMMC_PORT port, U32 blk_size);

/**
 * @brief    Set byte count for transfer
 * @param    port    : SDMMC peripheral selected
 * @param    bytes    : block size and byte count in bytes
 * @return    None
 */
void SysSdMmcBlock_SetByteCnt(SDMMC_PORT port, U32 bytes);

/**
 * @brief    Initializes the SD/MMC card controller
 * @param    port    : SDMMC peripheral selected
 * @return    None
 */
void SysSdMmcBlock_Init(SDMMC_PORT port);

/**
 * @brief    Shutdown the SD/MMC card controller
 * @param    port    : SDMMC peripheral selected
 * @return    None
 */
void SysSdMmcBlock_DeInit(SDMMC_PORT port);

/**
 * @brief    Function to send command to Card interface unit (CIU)
 * @param    port    : SDMMC peripheral selected
 * @param    cmd        : Command with all flags set
 * @param    arg        : Argument for the command
 * @return    TRUE on times-out, otherwise FALSE
 */
S32 SysSdMmcBlock_SendCommand(SDMMC_PORT port, U32 cmd, U32 arg);

/**
 * @brief    Read the response from the last command
 * @param    port    : SDMMC peripheral selected
 * @param    resp    : Pointer to response array to fill
 * @return    None
 */
void SysSdMmcBlock_GetResponse(SDMMC_PORT port, U32 *resp);

/**
 * @brief    Sets the SD bus clock speed
 * @param    port    : SDMMC peripheral selected
 * @param    clk_rate    : Input clock rate into the IP block
 * @param    speed        : Desired clock speed to the card
 * @return    None
 */
void SysSdMmcBlock_SetClock(SDMMC_PORT port, U32 speed);

/**
 * @brief    Function to clear interrupt & FIFOs
 * @param    port    : SDMMC peripheral selected
 * @return    None
 */
void SysSdMmcBlock_SetClearIntFifo(SDMMC_PORT port);

/*
 * @brief   Delay callback for timed SDIF/SDMMC functions
 */
void SysSdMmcBlock_WaitMs(U32 time);

/**
 * @brief    Sets up the SD event driven wakeup
 * @param    bits : Status bits to poll for command completion
 * @return    Nothing
 */
void SysSdMmcBlock_SetupWakeup(SDMMC_PORT port, void *bits);

/**
 * @brief    A polling wait callback for SDMMC
 * @return    the status of the register when a command is completed, or a timeout occurred
 */
U32 SysSdMmcBlock_PollingDrivenWait(SDMMC_PORT port, U32 timeout_in_ns);
//================================================================================================//



#endif /* ISYS_SD_MMC_BLOCK */
