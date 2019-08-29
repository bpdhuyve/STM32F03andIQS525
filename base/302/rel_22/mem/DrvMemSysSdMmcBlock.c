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
// driver containing all SD Host Controller protocol
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRVMEMSYSSDMMCBLOCK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__DRVMEMSYSSDMMCBLOCK_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               MEM__DRVMEMSYSSDMMCBLOCK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMSYSSDMMCBLOCK_COUNT
	#define DRVMEMSYSSDMMCBLOCK_COUNT		        1
#endif
//------------------------------------------------------------------------------------------------//
// @remark trying a MMC setup after failing the SD card setup can take up some (unuseful) time
#ifndef DRVMEMSYSSDMMCBLOCK_SUPPORT_MMC
	#define DRVMEMSYSSDMMCBLOCK_SUPPORT_MMC		        0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section

//COM lib include section

//APP include section
#include "DrvMem.h"
#include "DrvMemSysSdMmcBlock.h"
#include "timer\DrvTimeStamp.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
/* Helper definition: all SD error conditions in the status word */
#define SD_INT_ERROR (MCI_INT_RESP_ERR | MCI_INT_RCRC | MCI_INT_DCRC | \
                      MCI_INT_RTO | MCI_INT_DTO | MCI_INT_HTO | MCI_INT_FRUN | MCI_INT_HLE | \
                      MCI_INT_SBE | MCI_INT_EBE)
//================================================================================================//


//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
 
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/**
 * @brief    execute a command sequence
 * @param    port : SDMMC peripheral selected
 * @param    cmd : the command to execute
 * @param    arg : the command arguments
 * @return   received status
 */
static S32 ExecuteCommand(SDMMC_PORT port, U32 cmd, U32 arg, U32 wait_status, U32 timeout);


// @remark returns True when a card was acquired
U32 CardAcquired(MEM_HNDL mem_hndl);

/**
 * @brief    Get card's current state (idle, transfer, program, etc.)
 * @param    port    : SDMMC peripheral selected
 * @return    Current SD card transfer state
 */
S32 GetState(SDMMC_PORT port);

/**
 * @brief    Function to enumerate the SD/MMC/SDHC/MMC+ cards
 * @param    port        : SDMMC peripheral selected
 * @param    pcardinfo    : Pointer to pre-allocated card info structure
 * @return    1 if a card is acquired, otherwise 0
 */
U32 Acquire(MEM_HNDL mem_hndl);

/**
 * @brief    Get the device size of SD/MMC card (after enumeration)
 * @param    port    : SDMMC peripheral selected
 * @return    Card size in number of bytes (capacity)
 */
S32 GetDeviceSize(SDMMC_PORT port);

/**
 * @brief    Get the number of device blocks of SD/MMC card (after enumeration)
 * Since GetDeviceSize is limited to 32 bits cards with greater than
 * 2 GBytes of data will not be correct, in such cases users can use this function
 * to get the size of the card in blocks.
 * @param    port    : SDMMC peripheral selected
 * @return    Number of 512 bytes blocks in the card
 */
S32 GetNumberOfDeviceBlocks(SDMMC_PORT port);

/**
 * @brief    Performs the read of data from the SD/MMC card
 * @param    port        : SDMMC peripheral selected
 * @param    buffer        : Pointer to data buffer to copy to
 * @param    start_block    : Start block number
 * @param    num_blocks    : Number of block to read
 * @return    Bytes read, or 0 on error
 */
BOOL ReadBlocks(MEM_ID mem_id, U32 start_block, U8* buffer, U16 num_blocks);

/**
 * @brief    Performs write of data to the SD/MMC card
 * @param    port        : SDMMC peripheral selected
 * @param    buffer        : Pointer to data buffer to copy to
 * @param    start_block    : Start block number
 * @param    num_blocks    : Number of block to write
 * @return    Number of bytes actually written, or 0 on error
 */
BOOL WriteBlocks(MEM_ID mem_id, U32 start_block, U8* buffer, U16 num_blocks);

/**
 * @brief    Wait till the card is ready
 */
BOOL Sync(MEM_ID mem_id);

/**
 * @brief    Get size of a single sector in the card
 */
U32 CardGetSectorSize(SDMMC_PORT port);


/**
 * @brief    Gets number of sectors in the card
 * LPC43xx/18xx implementation of the FSMCI adapter function, that
 * will get the number of sectors in the card.
 */
U32 CardGetSectorCount(SDMMC_PORT port);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
/* Global instance of the current card */
static MCI_CARD_STRUCT card_handles[DRVMEMSYSSDMMCBLOCK_COUNT];

static MEM_HOOK_LIST mem_hook_list;
static MEM_STRUCT    mem_struct[DRVMEMSYSSDMMCBLOCK_COUNT];

static U8            mem_count;

MODULE_DECLARE();
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static S32 ExecuteCommand(SDMMC_PORT port, U32 cmd, U32 arg, U32 wait_status, U32 timeout)
{
    S32 status = 0;
    U32 cmd_reg = 0;
    U8 retrycount;
    

    if (!wait_status)
    {
        wait_status = (cmd & CMD_MASK_RESP) ? MCI_INT_CMD_DONE : MCI_INT_DATA_OVER;
    }    
    
    for (retrycount = NUM_CMD_RETRIES; retrycount > 0; retrycount--)
    {
        

        /* Clear the interrupts & FIFOs*/
        if (cmd & CMD_BIT_DATA) {
            SysSdMmcBlock_SetClearIntFifo(port);
        }

        /* also check error conditions */
        wait_status |= MCI_INT_EBE | MCI_INT_SBE | MCI_INT_HLE |MCI_INT_RTO | MCI_INT_RCRC | MCI_INT_RESP_ERR; // 
        if (wait_status & MCI_INT_DATA_OVER) {
            wait_status |= MCI_INT_FRUN | MCI_INT_HTO | MCI_INT_DTO | MCI_INT_DCRC;
        }
        
        SysSdMmcBlock_SetClock(port, card_handles[(U8)port].card_info.speed);

        /* Clear the interrupts */
        SysSdMmcBlock_ClrIntStatus(port, 0xFFFFFFFF);

        SysSdMmcBlock_SetupWakeup(port, (void *) &wait_status);
        
        if (cmd & CMD_BIT_APP)
        {
            cmd_reg = MMC_APP_CMD | MCI_CMD_RESP_EXP |
              ((cmd & CMD_BIT_INIT)  ? MCI_CMD_INIT : 0) |
              MCI_CMD_START;        

            SysSdMmcBlock_SendCommand(port, cmd_reg, card_handles[(U8)port].card_info.rca << 16);
            
            /* wait for command response */
            status = SysSdMmcBlock_PollingDrivenWait(port, timeout);
            
            if (status & SD_INT_ERROR)
                continue;
            
            /* Get response if any */
            if (status & MCI_INT_CMD_DONE) {
                switch (cmd & CMD_MASK_RESP) {
                case 0:
                    break;

                case CMD_RESP_R1:
                case CMD_RESP_R3:
                case CMD_RESP_R2:
                    SysSdMmcBlock_GetResponse(port, &card_handles[(U8)port].card_info.response[0]);
                    break;
                }
            }
                
            SysSdMmcBlock_SetClock(port, card_handles[(U8)port].card_info.speed);

            /* Clear the interrupts */
            SysSdMmcBlock_ClrIntStatus(port, 0xFFFFFFFF);

            SysSdMmcBlock_SetupWakeup(port, (void *) &wait_status);
        }
        
        cmd_reg = ((cmd & CMD_MASK_CMD) >> CMD_SHIFT_CMD) |
                  ((cmd & CMD_BIT_INIT)  ? MCI_CMD_INIT : 0) |
                  ((cmd & CMD_BIT_DATA)  ? (MCI_CMD_DAT_EXP | MCI_CMD_PRV_DAT_WAIT) : 0) |
                  (((cmd & CMD_MASK_RESP) == CMD_RESP_R2) ? MCI_CMD_RESP_LONG : 0) |
                  ((cmd & CMD_MASK_RESP) ? MCI_CMD_RESP_EXP : 0) |
                  ((cmd & CMD_BIT_WRITE)  ? MCI_CMD_DAT_WR : 0) |
                  ((cmd & CMD_BIT_STREAM) ? MCI_CMD_STRM_MODE : 0) |
                  ((cmd & CMD_BIT_BUSY) ? MCI_CMD_STOP : 0) |
                  ((cmd & CMD_BIT_AUTO_STOP)  ? MCI_CMD_SEND_STOP : 0) |
                  MCI_CMD_START;

        /* wait for previos data finsh for select/deselect commands */
        if (((cmd & CMD_MASK_CMD) >> CMD_SHIFT_CMD) == MMC_SELECT_CARD) {
            cmd_reg |= MCI_CMD_PRV_DAT_WAIT;
        }
        
        /* wait for command to be accepted by CIU */
        SysSdMmcBlock_SendCommand(port, cmd_reg, arg);
            
        
        /* wait for command response */
        status = SysSdMmcBlock_PollingDrivenWait(port, timeout);
        
    
        if (!(status & SD_INT_ERROR))
            break;        
    }
    
    /* We return an error if there is a timeout, even if we've fetched  a response */
    if (status & SD_INT_ERROR) {
        return status;
    }

    /* Get response if any */
    if (status & MCI_INT_CMD_DONE) {
        switch (cmd & CMD_MASK_RESP) {
        case 0:
            break;

        case CMD_RESP_R1:
        case CMD_RESP_R3:
        case CMD_RESP_R2:
            SysSdMmcBlock_GetResponse(port, &card_handles[(U8)port].card_info.response[0]);
            break;
        }
    }
    
    return status;
}

//------------------------------------------------------------------------------------------------//
/* Helper function to get a bit field withing multi-word  buffer. Used to get
   fields with-in CSD & EXT-CSD */
static U32 GetBits(S32 start, S32 end, U32 *data)
{
    U32 v;
    U32 i = end >> 5;
    U32 j = start & 0x1f;

    if (i == (start >> 5)) {
        v = (data[i] >> j);
    }
    else {
        v = ((data[i] << (32 - j)) | (data[start >> 5] >> j));
    }

    return v & ((1 << (end - start + 1)) - 1);
}
//------------------------------------------------------------------------------------------------//
static void ProcessCsd(SDMMC_PORT port)
{
    S32 status = 0;
    S32 c_size = 0;
    S32 c_size_mult = 0;
    S32 mult = 0;

    /* compute block length based on CSD response */
    card_handles[(U8)port].card_info.block_len = 1 << GetBits(80, 83, card_handles[(U8)port].card_info.csd);

    if ((card_handles[(U8)port].card_info.card_type & CARD_TYPE_HC) && (card_handles[(U8)port].card_info.card_type & CARD_TYPE_SD)) {
        /* See section 5.3.3 CSD Register (CSD Version 2.0) of SD2.0 spec  an explanation for the calculation of these values */
        c_size = GetBits(48, 63, (U32 *) card_handles[(U8)port].card_info.csd) + 1;
        card_handles[(U8)port].card_info.blocknr = c_size << 10;    /* 512 byte blocks */
    }
    else {
        /* See section 5.3 of the 4.1 revision of the MMC specs for  an explanation for the calculation of these values */
        c_size = GetBits(62, 73, (U32 *) card_handles[(U8)port].card_info.csd);
        c_size_mult = GetBits(47, 49, (U32 *) card_handles[(U8)port].card_info.csd);
        mult = 1 << (c_size_mult + 2);
        card_handles[(U8)port].card_info.blocknr = (c_size + 1) * mult;

        /* adjust blocknr to 512/block */
        if (card_handles[(U8)port].card_info.block_len > MMC_SECTOR_SIZE) {
            card_handles[(U8)port].card_info.blocknr = card_handles[(U8)port].card_info.blocknr * (card_handles[(U8)port].card_info.block_len >> 9);
        }

        /* get extended CSD for newer MMC cards CSD spec >= 4.0*/
        if (((card_handles[(U8)port].card_info.card_type & CARD_TYPE_SD) == 0) &&
            (GetBits(122, 125, (U32 *) card_handles[(U8)port].card_info.csd) >= 4)) {
            /* put card in trans state */
            status = ExecuteCommand(port, CMD_SELECT_CARD, card_handles[(U8)port].card_info.rca << 16, 0, DEFAULT_COMMAND_TIMEOUT);

            /* set block size and byte count */
            SysSdMmcBlock_SetBlkSizeByteCnt(port, MMC_SECTOR_SIZE);

            /* send EXT_CSD command */
            SysSdMmcBlock_DmaSetup(port,
                              &card_handles[(U8)port].sdif_dev,
                              (U32) card_handles[(U8)port].card_info.ext_csd,
                              MMC_SECTOR_SIZE);

            status = ExecuteCommand(port, CMD_SEND_EXT_CSD, 0, 0 | MCI_INT_DATA_OVER, DEFAULT_COMMAND_TIMEOUT);
            if ((status & SD_INT_ERROR) == 0) {
                /* check EXT_CSD_VER is greater than 1.1 */
                if ((card_handles[(U8)port].card_info.ext_csd[48] & 0xFF) > 1) {
                    card_handles[(U8)port].card_info.blocknr = card_handles[(U8)port].card_info.ext_csd[53];/* bytes 212:215 represent sec count */

                }
                /* switch to 52MHz clock if card type is set to 1 or else set to 26MHz */
                if ((card_handles[(U8)port].card_info.ext_csd[49] & 0xFF) == 1) {
                    /* for type 1 MMC cards high speed is 52MHz */
                    card_handles[(U8)port].card_info.speed = MMC_HIGH_BUS_MAX_CLOCK;
                }
                else {
                    /* for type 0 MMC cards high speed is 26MHz */
                    card_handles[(U8)port].card_info.speed = MMC_LOW_BUS_MAX_CLOCK;
                }
            }
        }
    }

    card_handles[(U8)port].card_info.device_size = card_handles[(U8)port].card_info.blocknr << 9;    /* blocknr * 512 */
}
//------------------------------------------------------------------------------------------------//
static BOOL SetTransState(SDMMC_PORT port)
{
    U32 status;
    U32 timestamp_start;

    /* get current state of the card */
    status = ExecuteCommand(port, CMD_SEND_STATUS, card_handles[(U8)port].card_info.rca << 16, 0, DEFAULT_COMMAND_TIMEOUT);
    if (status & SD_INT_ERROR) {
        /* unable to get the card state. So return immediatly. */
        return FALSE;
    }

    /* check card state in response */
    status = R1_CURRENT_STATE(card_handles[(U8)port].card_info.response[0]);
    switch (status) {
    case SDMMC_STBY_ST:
        /* put card in 'Trans' state */
        status = ExecuteCommand(port, CMD_SELECT_CARD, card_handles[(U8)port].card_info.rca << 16, 0, DEFAULT_COMMAND_TIMEOUT);
        if ((status & SD_INT_ERROR) != 0) {
            /* unable to put the card in Trans state. So return immediatly. */
            return FALSE;
        }
        break;

    case SDMMC_TRAN_ST:
        /*do nothing */
        break;

    default:
        /* card shouldn't be in other states so return */
        return FALSE;
    }

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static S32 SetCardParams(SDMMC_PORT port)
{
    S32 status;

#if SDIO_BUS_WIDTH > 4
#error 8-bit mode not supported yet!
#elif SDIO_BUS_WIDTH > 1
    if (card_handles[(U8)port].card_info.card_type & CARD_TYPE_SD) {
        status = ExecuteCommand(port, CMD_SD_SET_WIDTH, 2, 0, DEFAULT_COMMAND_TIMEOUT);
        if (status & SD_INT_ERROR) {
            return -1;
        }

        /* if positive response */
        SysSdMmcBlock_SetCardType(port, MCI_CTYPE_4BIT);
    }
#endif

    /* set block length */
    SysSdMmcBlock_SetBlkSize(port, MMC_SECTOR_SIZE);
    status = ExecuteCommand(port, CMD_SET_BLOCKLEN, MMC_SECTOR_SIZE, 0, DEFAULT_COMMAND_TIMEOUT);
    if (status & SD_INT_ERROR) {
        return -1;
    }

    return 0;
}
//------------------------------------------------------------------------------------------------//
U32 CardAcquired(MEM_HNDL mem_hndl)
{
    MODULE_CHECK();
    
    return card_handles[(U8)(mem_hndl->mem_id)].card_info.cid[0] != 0;
}
//------------------------------------------------------------------------------------------------//
S32 GetState(SDMMC_PORT port)
{
    U32 status;
    S32 current_state;

    /* get current state of the card */
    status = ExecuteCommand(port, CMD_SEND_STATUS, card_handles[(U8)port].card_info.rca << 16, 0, 2*DEFAULT_COMMAND_TIMEOUT);
    if (status & SD_INT_ERROR || status == 0) {
        return -1;
    }

    /* check card state in response */
    current_state = R1_CURRENT_STATE(card_handles[(U8)port].card_info.response[0]);
    return (S32) current_state;
}
//------------------------------------------------------------------------------------------------//
U32 Acquire(MEM_HNDL mem_hndl)
{    
    S32 status;
    U32 response;
    SDMMC_PORT port = (SDMMC_PORT) mem_hndl->mem_id;
    
    System_EnableClock(CLOCK_BRANCH_SDIO);

    while (card_handles[(U8)port].acquire_info.state < 100) {
        switch (card_handles[(U8)port].acquire_info.state) {
        case 0:    /* Setup for SD */
            
            /* clear card type */
            SysSdMmcBlock_SetCardType(port, 0);

            /* set high speed for the card as 20MHz */
            card_handles[(U8)port].card_info.speed = MMC_MAX_CLOCK;   
            
            card_handles[(U8)port].acquire_info.ocr = OCR_VOLTAGE_RANGE_MSK;
            
            status = ExecuteCommand(port, CMD_IDLE, 0, MCI_INT_CMD_DONE, DEFAULT_COMMAND_TIMEOUT);
            /* check if it is SDHC card */
            status = ExecuteCommand(port, CMD_SD_SEND_IF_COND, SD_SEND_IF_ARG, 0, DEFAULT_COMMAND_TIMEOUT);
            if (!(status & MCI_INT_RTO)) {
                /* check response has same echo pattern */
                if ((card_handles[(U8)port].card_info.response[0] & SD_SEND_IF_ECHO_MSK) == SD_SEND_IF_RESP) {
                    card_handles[(U8)port].acquire_info.ocr |= OCR_HC_CCS;
                }
            }

            card_handles[(U8)port].acquire_info.state++;
            card_handles[(U8)port].acquire_info.command = CMD_SD_OP_COND;
            card_handles[(U8)port].acquire_info.tries = INIT_OP_RETRIES;

            /* assume SD card */
            card_handles[(U8)port].card_info.card_type |= CARD_TYPE_SD;
            card_handles[(U8)port].card_info.speed = SD_MAX_CLOCK;
            break;

#if DRVMEMSYSSDMMCBLOCK_SUPPORT_MMC > 0
        case 10:    /* Setup for MMC */
            /* start fresh for MMC crds */
            card_handles[(U8)port].acquire_info.ocr = OCR_VOLTAGE_RANGE_MSK;
            card_handles[(U8)port].card_info.card_type &= ~(U32)CARD_TYPE_SD;
            status = ExecuteCommand(port, CMD_IDLE, 0, MCI_INT_CMD_DONE, DEFAULT_COMMAND_TIMEOUT);
            card_handles[(U8)port].acquire_info.command = CMD_MMC_OP_COND;
            tries = INIT_OP_RETRIES;
            ocr |= OCR_HC_CCS;
            card_handles[(U8)port].acquire_info.state++;

            /* for MMC cards high speed is 20MHz */
            card_handles[(U8)port].card_info.speed = MMC_MAX_CLOCK;
            break;
#endif
        case 1:
        case 11:
            status = ExecuteCommand(port, card_handles[(U8)port].acquire_info.command, 0, 0, DEFAULT_COMMAND_TIMEOUT);
            if (status & MCI_INT_RTO) {
                card_handles[(U8)port].acquire_info.state += 9;    /* Mode unavailable */
            }
            else {
                card_handles[(U8)port].acquire_info.state++;
            }
            break;

        case 2:        /* Initial OCR check  */
        case 12:
            card_handles[(U8)port].acquire_info.ocr = card_handles[(U8)port].card_info.response[0] | (card_handles[(U8)port].acquire_info.ocr & OCR_HC_CCS);
            if (card_handles[(U8)port].acquire_info.ocr & OCR_ALL_READY) {
                card_handles[(U8)port].acquire_info.state++;
            }
            else {
                card_handles[(U8)port].acquire_info.state += 2;
            }
            break;

        case 3:        /* Initial wait for OCR clear */
        case 13:
            while ((card_handles[(U8)port].acquire_info.ocr & OCR_ALL_READY) && --card_handles[(U8)port].acquire_info.tries > 0) {
                SysSdMmcBlock_WaitMs(MS_ACQUIRE_DELAY);
                status = ExecuteCommand(port, card_handles[(U8)port].acquire_info.command, 0, 0, DEFAULT_COMMAND_TIMEOUT);
                card_handles[(U8)port].acquire_info.ocr = card_handles[(U8)port].card_info.response[0] | (card_handles[(U8)port].acquire_info.ocr & OCR_HC_CCS);
            }
            if (card_handles[(U8)port].acquire_info.ocr & OCR_ALL_READY) {
                card_handles[(U8)port].acquire_info.state += 7;
            }
            else {
                card_handles[(U8)port].acquire_info.state++;
            }
            break;


#if DRVMEMSYSSDMMCBLOCK_SUPPORT_MMC > 0            
        case 14:    /* Assign OCR */
            /* for MMC cards set high capacity bit */
            card_handles[(U8)port].acquire_info.ocr |= OCR_HC_CCS;
            /* no break */
#endif
        case 4:
            card_handles[(U8)port].acquire_info.tries = SET_OP_RETRIES;
            card_handles[(U8)port].acquire_info.ocr &= OCR_VOLTAGE_RANGE_MSK | OCR_HC_CCS;    /* Mask for the bits we care about */
            
            do
            {
            SysSdMmcBlock_WaitMs(MS_ACQUIRE_DELAY);
            status = ExecuteCommand(port, card_handles[(U8)port].acquire_info.command, card_handles[(U8)port].acquire_info.ocr, 0, 10*DEFAULT_COMMAND_TIMEOUT);
            response = card_handles[(U8)port].card_info.response[0];
            }
            while (!(response & OCR_ALL_READY) && --card_handles[(U8)port].acquire_info.tries > 0);
                        
            if (card_handles[(U8)port].acquire_info.tries <= 0)
                return FALSE; // yield

            if (response & OCR_ALL_READY) {
                /* is it high capacity card */
                card_handles[(U8)port].card_info.card_type |= (response & OCR_HC_CCS);
                card_handles[(U8)port].acquire_info.state++;
            }
            else {
                card_handles[(U8)port].acquire_info.state += 6;
            }
            break;

        case 5:    /* CID polling */
        case 15:
            status = ExecuteCommand(port, CMD_ALL_SEND_CID, 0, 0, DEFAULT_COMMAND_TIMEOUT);
            memcpy((VPTR)(&card_handles[(U8)port].card_info.cid), (VPTR)(&card_handles[(U8)port].card_info.response[0]), 16);
            ++card_handles[(U8)port].acquire_info.state;
            break;

        case 6:    /* RCA send, for SD get RCA */
            status = ExecuteCommand(port, CMD_SD_SEND_RCA, 0, 0, DEFAULT_COMMAND_TIMEOUT);
            card_handles[(U8)port].card_info.rca = (card_handles[(U8)port].card_info.response[0]) >> 16;
            card_handles[(U8)port].acquire_info.state++;
            break;

#if DRVMEMSYSSDMMCBLOCK_SUPPORT_MMC > 0
        case 16:    /* RCA assignment for MMC set to 1 */
            card_handles[(U8)port].card_info.rca = 1;
            status = ExecuteCommand(port, CMD_MMC_SET_RCA, card_handles[(U8)port].card_info.rca << 16, 0, DEFAULT_COMMAND_TIMEOUT);
            ++card_handles[(U8)port].acquire_info.state;
            break;
#endif

        case 7:
        case 17:
            status = ExecuteCommand(port, CMD_SEND_CSD, card_handles[(U8)port].card_info.rca << 16, 0, DEFAULT_COMMAND_TIMEOUT);
            memcpy((VPTR)(&card_handles[(U8)port].card_info.csd), (VPTR)(&card_handles[(U8)port].card_info.response[0]), 16);
            card_handles[(U8)port].acquire_info.state = 100;
            break;

        default:
            card_handles[(U8)port].acquire_info.state += 100;    /* break from while loop */
            break;
        }
    }

    /* Compute card size, block size and no. of blocks  based on CSD response recived. */
    if (CardAcquired(mem_hndl)) {
        ProcessCsd(port);

        /* Setup card data width and block size (once) */
        if (SetTransState(port) != TRUE) {
            return 0;
        }
        if (SetCardParams(port) != 0) {
            return 0;
        }
    }
    else
    {
        // acquire failed, reset state        
        LPC_SDMMC->CTRL |= 3;
        while (LPC_SDMMC->CTRL & 0x1) {}
        MEMSET((VPTR)&card_handles[(U8)port], 0, sizeof(card_handles[(U8)port]));        
        status = ExecuteCommand(port, CMD_IDLE, 0, MCI_INT_CMD_DONE, DEFAULT_COMMAND_TIMEOUT);
    }

    return CardAcquired(mem_hndl);
}
//------------------------------------------------------------------------------------------------//
S32 GetDeviceSize(SDMMC_PORT port)
{
    return card_handles[(U8)port].card_info.device_size;
}
//------------------------------------------------------------------------------------------------//
S32 GetNumberOfDeviceBlocks(SDMMC_PORT port)
{
    return card_handles[(U8)port].card_info.blocknr;
}
//------------------------------------------------------------------------------------------------//
BOOL ReadBlocks(MEM_ID mem_id, U32 start_block, U8* buffer, U16 num_blocks)
{
    S32 cbRead = (num_blocks) * MMC_SECTOR_SIZE;
    S32 status = 0;
    S32 index;
    U32 timestamp_start;
    U32 readblocks_delay;
    static U32 readblocks_delay_max;
    SDMMC_PORT port = (SDMMC_PORT)mem_id;

    /* if card is not acquired return immediately */
    if ((start_block + num_blocks) > card_handles[(U8)port].card_info.blocknr)
    {
        return FALSE;
    }

    /* put card in trans state */
    if (SetTransState(port) != TRUE) {
        return FALSE;
    }

    /* set number of bytes to read */
    SysSdMmcBlock_SetByteCnt(port, cbRead);

    /* if high capacity card use block indexing */
    if (card_handles[(U8)port].card_info.card_type & CARD_TYPE_HC) {
        index = start_block;
    }
    else 
    {    /*fix at 512 bytes*/
        index = start_block << 9;    // \* card_handles[(U8)port].card_info.block_len;

    }
    SysSdMmcBlock_DmaSetup(port, &card_handles[(U8)port].sdif_dev, (U32) buffer, cbRead);

    /* Select single or multiple read based on number of blocks */
    if (num_blocks == 1) {
        status = ExecuteCommand(port, CMD_READ_SINGLE, index, 0 | MCI_INT_DATA_OVER, DEFAULT_COMMAND_TIMEOUT);
    }
    else {
        status = ExecuteCommand(port, CMD_READ_MULTIPLE, index, 0 | MCI_INT_DATA_OVER, DEFAULT_COMMAND_TIMEOUT);
    }

    if (status & SD_INT_ERROR > 0)
        return FALSE;
    
    /*Wait for card program to finish*/
    timestamp_start = GET_TIMESTAMP();
    while (GetState(port) != SDMMC_TRAN_ST && TIMEOUT_STILL_PENDING(timestamp_start, DEFAULT_COMMAND_TIMEOUT)) {} 
    readblocks_delay = GET_TIMESTAMP() - timestamp_start;
    if (readblocks_delay > readblocks_delay_max)
    {
        readblocks_delay_max = readblocks_delay;
        LOG_DEV("new readblocks max: %d ns", PU32(TIMESTAMP_TO_NS(readblocks_delay_max)));
    }

    return (BOOL)(GetState(port) == SDMMC_TRAN_ST);
}
//------------------------------------------------------------------------------------------------//
BOOL WriteBlocks(MEM_ID mem_id, U32 start_block, U8* buffer, U16 num_blocks)
{
    S32 cbWrote = num_blocks *  MMC_SECTOR_SIZE;
    S32 status;
    S32 index;
    U32 timestamp_start;
    U32 writeblocks_delay;
    static U32 writeblocks_delay_max;
    SDMMC_PORT port = (SDMMC_PORT)mem_id;

    /* if card is not acquired return immediately */
    if ((start_block + num_blocks) > card_handles[(U8)port].card_info.blocknr)
    {
        return FALSE;
    }

    /* put card in trans state */
    if (SetTransState(port) != TRUE) {
        return FALSE;
    }

    /* set number of bytes to write */
    SysSdMmcBlock_SetByteCnt(port, cbWrote);

    /* if high capacity card use block indexing */
    if (card_handles[(U8)port].card_info.card_type & CARD_TYPE_HC) {
        index = start_block;
    }
    else {    /*fix at 512 bytes*/
        index = start_block << 9;    // * card_handles[(U8)port].card_info.block_len;

    }

    SysSdMmcBlock_DmaSetup(port, &card_handles[(U8)port].sdif_dev, (U32) buffer, cbWrote);

    /* Select single or multiple write based on number of blocks */
    if (num_blocks == 1) {
        status = ExecuteCommand(port, CMD_WRITE_SINGLE, index, 0 | MCI_INT_DATA_OVER, DEFAULT_COMMAND_TIMEOUT);
    }
    else {
        status = ExecuteCommand(port, CMD_WRITE_MULTIPLE, index, 0 | MCI_INT_DATA_OVER, DEFAULT_COMMAND_TIMEOUT);
    }

    if (status & SD_INT_ERROR > 0)
        return FALSE;
    
    /*Wait for card program to finish*/
    timestamp_start = GET_TIMESTAMP();
    S32 state = 0;
    while (state != SDMMC_TRAN_ST && TIMEOUT_STILL_PENDING(timestamp_start, DEFAULT_COMMAND_TIMEOUT))
    { 
        state = GetState(port);
    }
    writeblocks_delay = GET_TIMESTAMP() - timestamp_start;
    if (writeblocks_delay > writeblocks_delay_max)
    {
        writeblocks_delay_max = writeblocks_delay;
        LOG_DEV("new writeblocks max: %d ns", PU32(writeblocks_delay_max));
    }
    
    if (state != SDMMC_TRAN_ST) 
	{
        return FALSE;
    }

    if (cbWrote == 0)
        return FALSE;

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL Sync(MEM_ID mem_id)
{
    U32 start = GET_TIMESTAMP();
    SDMMC_PORT port = (SDMMC_PORT)mem_id;
    

    while (GetState(port) == -1 && TIMEOUT_STILL_PENDING(start, 20e6)) {};

    return (BOOL)(GetState(port) != -1);
}
//------------------------------------------------------------------------------------------------//
U32 CardGetSectorSize(SDMMC_PORT port)
{
    return card_handles[(U8)port].card_info.block_len;
}
//------------------------------------------------------------------------------------------------//
U32 CardGetSectorCount(SDMMC_PORT port)
{
    return card_handles[(U8)port].card_info.blocknr;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvMemSysSdMmcBlock_Init()
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)card_handles, 0, SIZEOF(card_handles));

    mem_hook_list.mem_clear_hook = NULL;
    mem_hook_list.mem_read_hook = ReadBlocks;
    mem_hook_list.mem_write_hook = WriteBlocks;
    mem_hook_list.mem_verify_hook = NULL;
    mem_hook_list.mem_flush_hook = Sync;

    MEMSET((VPTR)mem_struct, 0, SIZEOF(mem_struct));
    mem_count = 0;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void DrvMemSysSdMmcBlock_Handler()
{
    U8 i = 0;
    U32 timestamp_start;
    static U32 retrycounter = 0;
    
    MODULE_CHECK();
    
    for (i = 0; i < mem_count; i++)
    {    
        SDMMC_PORT port = (SDMMC_PORT) mem_struct[i].mem_id;
        
        if (SysSdMmcBlock_CardDetected(port))
        {
            if (! CardAcquired(&mem_struct[i]))
            {
                SysSdMmcBlock_Init(port);
                U32 success = Acquire(&mem_struct[i]);
                if (!success)
                {
					// reset controller & data structures to reset for next try to acquire 
                    SysSdMmcBlock_DeInit(port);
                    MEMSET((VPTR)&card_handles[(U8)port], 0, sizeof(card_handles[(U8)port]));
                }
                //S32 state = GetState(port);
            }
            
//            // still busy after read/write
//            timestamp_start = GET_TIMESTAMP();
//            while (GetState(port) == SDMMC_DATA_ST && TIMEOUT_STILL_PENDING(timestamp_start, 5*DEFAULT_COMMAND_TIMEOUT)) {}
//            
//            if (GetState(port) == SDMMC_DATA_ST)
//            {
//                // workaround: start over
//                MEMSET((VPTR)&card_handles[(U8)port], 0, sizeof(card_handles[(U8)port]));
//                retrycounter++;
//            }
        }
        else
        {
            SysSdMmcBlock_DeInit(port);
            MEMSET((VPTR)&card_handles[(U8)port], 0, sizeof(card_handles[(U8)port]));
        }
    }
}
//------------------------------------------------------------------------------------------------//
MEM_HNDL DrvMemSysSdMmcBlock_RegisterChannel(SDMMC_PORT port)
{    
    MEM_HNDL handle;
    U8 i;
    
    MODULE_CHECK();
    
    for (i = 0; i < mem_count; i++)
    {
        handle = &mem_struct[i];
        if (handle->mem_id == (MEM_ID)port)
            return handle;        
    }

    if (mem_count < DRVMEMSYSSDMMCBLOCK_COUNT)
    {
        handle = &mem_struct[mem_count];

        handle->mem_id = (MEM_ID)port;
        handle->hook_list_ptr = &mem_hook_list;

        mem_count++;
        return handle;
    }
    
    LOG_WRN("DrvMemSysSdMmcBlock: insufficient number of handles!");
    return NULL;
}
//================================================================================================//

