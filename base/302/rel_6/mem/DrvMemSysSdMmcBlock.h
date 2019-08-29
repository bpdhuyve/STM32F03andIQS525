/*
 * @brief LPC18xx/43xx SD/MMC card driver
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
#ifndef DRVMEMSYSSDMMCBLOCK_H
#define DRVMEMSYSSDMMCBLOCK_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "mem/DrvMem.h"

#include "mem/SysSdMmcBlock.h"
//================================================================================================//


//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define CMD_MASK_RESP       (0x3UL << 28)
#define CMD_RESP(r)         (((r) & 0x3) << 28)
#define CMD_RESP_R0         (0 << 28)
#define CMD_RESP_R1         (1 << 28)
#define CMD_RESP_R2         (2 << 28)
#define CMD_RESP_R3         (3 << 28)
#define CMD_BIT_AUTO_STOP   (1 << 24)
#define CMD_BIT_APP         (1 << 23)
#define CMD_BIT_INIT        (1 << 22)
#define CMD_BIT_BUSY        (1 << 21)
#define CMD_BIT_LS          (1 << 20)    /* Low speed, used during acquire */
#define CMD_BIT_DATA        (1 << 19)
#define CMD_BIT_WRITE       (1 << 18)
#define CMD_BIT_STREAM      (1 << 17)
#define CMD_MASK_CMD        (0xff)
#define CMD_SHIFT_CMD       (0)

#define CMD(c, r)        ( ((c) &  CMD_MASK_CMD) | CMD_RESP((r)) )

#define CMD_IDLE            CMD(MMC_GO_IDLE_STATE, 0) | CMD_BIT_LS    | CMD_BIT_INIT
#define CMD_SD_OP_COND      CMD(SD_APP_OP_COND, 1)      | CMD_BIT_LS | CMD_BIT_APP
#define CMD_SD_SEND_IF_COND CMD(SD_CMD8, 1)      | CMD_BIT_LS
#define CMD_MMC_OP_COND     CMD(MMC_SEND_OP_COND, 3)    | CMD_BIT_LS | CMD_BIT_INIT
#define CMD_ALL_SEND_CID    CMD(MMC_ALL_SEND_CID, 2)    | CMD_BIT_LS
#define CMD_MMC_SET_RCA     CMD(MMC_SET_RELATIVE_ADDR, 1) | CMD_BIT_LS
#define CMD_SD_SEND_RCA     CMD(SD_SEND_RELATIVE_ADDR, 1) | CMD_BIT_LS
#define CMD_SEND_CSD        CMD(MMC_SEND_CSD, 2) | CMD_BIT_LS
#define CMD_SEND_EXT_CSD    CMD(MMC_SEND_EXT_CSD, 1) | CMD_BIT_LS | CMD_BIT_DATA
#define CMD_DESELECT_CARD   CMD(MMC_SELECT_CARD, 0)
#define CMD_SELECT_CARD     CMD(MMC_SELECT_CARD, 1)
#define CMD_SET_BLOCKLEN    CMD(MMC_SET_BLOCKLEN, 1)
#define CMD_SEND_STATUS     CMD(MMC_SEND_STATUS, 1)
#define CMD_READ_SINGLE     CMD(MMC_READ_SINGLE_BLOCK, 1) | CMD_BIT_DATA
#define CMD_READ_MULTIPLE   CMD(MMC_READ_MULTIPLE_BLOCK, 1) | CMD_BIT_DATA | CMD_BIT_AUTO_STOP
#define CMD_SD_SET_WIDTH    CMD(SD_APP_SET_BUS_WIDTH, 1) | CMD_BIT_APP
#define CMD_STOP            CMD(MMC_STOP_TRANSMISSION, 1) | CMD_BIT_BUSY
#define CMD_WRITE_SINGLE    CMD(MMC_WRITE_BLOCK, 1) | CMD_BIT_DATA | CMD_BIT_WRITE
#define CMD_WRITE_MULTIPLE  CMD(MMC_WRITE_MULTIPLE_BLOCK, 1) | CMD_BIT_DATA | CMD_BIT_WRITE | CMD_BIT_AUTO_STOP

/**
 * @brief OCR Register definitions
 */
/** Support voltage range 2.0-2.1 (this bit is reserved in SDC)*/
#define SDC_OCR_20_21               (((U32) 1) << 8)
/** Support voltage range 2.1-2.2 (this bit is reserved in SDC)*/
#define SDC_OCR_21_22               (((U32) 1) << 9)
/** Support voltage range 2.2-2.3 (this bit is reserved in SDC)*/
#define SDC_OCR_22_23               (((U32) 1) << 10)
/** Support voltage range 2.3-2.4 (this bit is reserved in SDC)*/
#define SDC_OCR_23_24               (((U32) 1) << 11)
/** Support voltage range 2.4-2.5 (this bit is reserved in SDC)*/
#define SDC_OCR_24_25               (((U32) 1) << 12)
/** Support voltage range 2.5-2.6 (this bit is reserved in SDC)*/
#define SDC_OCR_25_26               (((U32) 1) << 13)
/** Support voltage range 2.6-2.7 (this bit is reserved in SDC)*/
#define SDC_OCR_26_27               (((U32) 1) << 14)
/** Support voltage range 2.7-2.8 */
#define SDC_OCR_27_28               (((U32) 1) << 15)
/** Support voltage range 2.8-2.9*/
#define SDC_OCR_28_29               (((U32) 1) << 16)
/** Support voltage range 2.9-3.0 */
#define SDC_OCR_29_30               (((U32) 1) << 17)
/** Support voltage range 3.0-3.1 */
#define SDC_OCR_30_31               (((U32) 1) << 18)
/** Support voltage range 3.1-3.2 */
#define SDC_OCR_31_32               (((U32) 1) << 19)
/** Support voltage range 3.2-3.3 */
#define SDC_OCR_32_33               (((U32) 1) << 20)
/** Support voltage range 3.3-3.4 */
#define SDC_OCR_33_34               (((U32) 1) << 21)
/** Support voltage range 3.4-3.5 */
#define SDC_OCR_34_35               (((U32) 1) << 22)
/** Support voltage range 3.5-3.6 */
#define SDC_OCR_35_36               (((U32) 1) << 23)
/** Support voltage range 2.7-3.6 */
#define SDC_OCR_27_36               ((U32) 0x00FF8000)
/** Card Capacity Status (CCS). (this bit is reserved in MMC) */
#define SDC_OCR_HC_CCS              (((U32) 1) << 30)
/** Card power up status bit */
#define SDC_OCR_IDLE                (((U32) 1) << 31)
#define SDC_OCR_BUSY                (((U32) 0) << 31)

/* SD/MMC commands - this matrix shows the command, response types, and
   supported card type for that command.
   Command                 Number Resp  SD  MMC
   ----------------------- ------ ----- --- ---
   Reset (go idle)         CMD0   NA    x   x
   Send op condition       CMD1   R3        x
   All send CID            CMD2   R2    x   x
   Send relative address   CMD3   R1        x
   Send relative address   CMD3   R6    x
   Program DSR             CMD4   NA        x
   Select/deselect card    CMD7   R1b       x
   Select/deselect card    CMD7   R1    x
   Send CSD                CMD9   R2    x   x
   Send CID                CMD10  R2    x   x
   Read data until stop    CMD11  R1    x   x
   Stop transmission       CMD12  R1/b  x   x
   Send status             CMD13  R1    x   x
   Go inactive state       CMD15  NA    x   x
   Set block length        CMD16  R1    x   x
   Read single block       CMD17  R1    x   x
   Read multiple blocks    CMD18  R1    x   x
   Write data until stop   CMD20  R1        x
   Setblock count          CMD23  R1        x
   Write single block      CMD24  R1    x   x
   Write multiple blocks   CMD25  R1    x   x
   Program CID             CMD26  R1        x
   Program CSD             CMD27  R1    x   x
   Set write protection    CMD28  R1b   x   x
   Clear write protection  CMD29  R1b   x   x
   Send write protection   CMD30  R1    x   x
   Erase block start       CMD32  R1    x
   Erase block end         CMD33  R1    x
   Erase block start       CMD35  R1        x
   Erase block end         CMD36  R1        x
   Erase blocks            CMD38  R1b       x
   Fast IO                 CMD39  R4        x
   Go IRQ state            CMD40  R5        x
   Lock/unlock             CMD42  R1b       x
   Application command     CMD55  R1        x
   General command         CMD56  R1b       x

 *** SD card application commands - these must be preceded with ***
 *** MMC CMD55 application specific command first               ***
   Set bus width           ACMD6  R1    x
   Send SD status          ACMD13 R1    x
   Send number WR blocks   ACMD22 R1    x
   Set WR block erase cnt  ACMD23 R1    x
   Send op condition       ACMD41 R3    x
   Set clear card detect   ACMD42 R1    x
   Send CSR                ACMD51 R1    x */

/**
 * @brief SD/MMC commands, arguments and responses
 * Standard SD/MMC commands (3.1)       type  argument     response
 */
/* class 1 */
#define MMC_GO_IDLE_STATE         0        /* bc                          */
#define MMC_SEND_OP_COND          1        /* bcr  [31:0]  OCR        R3  */
#define MMC_ALL_SEND_CID          2        /* bcr                     R2  */
#define MMC_SET_RELATIVE_ADDR     3        /* ac   [31:16] RCA        R1  */
#define MMC_SET_DSR               4        /* bc   [31:16] RCA            */
#define MMC_SELECT_CARD           7        /* ac   [31:16] RCA        R1  */
#define MMC_SEND_EXT_CSD          8        /* bc                      R1  */
#define MMC_SEND_CSD              9        /* ac   [31:16] RCA        R2  */
#define MMC_SEND_CID             10        /* ac   [31:16] RCA        R2  */
#define MMC_STOP_TRANSMISSION    12        /* ac                      R1b */
#define MMC_SEND_STATUS          13        /* ac   [31:16] RCA        R1  */
#define MMC_GO_INACTIVE_STATE    15        /* ac   [31:16] RCA            */

/* class 2 */
#define MMC_SET_BLOCKLEN         16        /* ac   [31:0]  block len  R1  */
#define MMC_READ_SINGLE_BLOCK    17        /* adtc [31:0]  data addr  R1  */
#define MMC_READ_MULTIPLE_BLOCK  18        /* adtc [31:0]  data addr  R1  */

/* class 3 */
#define MMC_WRITE_DAT_UNTIL_STOP 20        /* adtc [31:0]  data addr  R1  */

/* class 4 */
#define MMC_SET_BLOCK_COUNT      23        /* adtc [31:0]  data addr  R1  */
#define MMC_WRITE_BLOCK          24        /* adtc [31:0]  data addr  R1  */
#define MMC_WRITE_MULTIPLE_BLOCK 25        /* adtc                    R1  */
#define MMC_PROGRAM_CID          26        /* adtc                    R1  */
#define MMC_PROGRAM_CSD          27        /* adtc                    R1  */

/* class 6 */
#define MMC_SET_WRITE_PROT       28        /* ac   [31:0]  data addr  R1b */
#define MMC_CLR_WRITE_PROT       29        /* ac   [31:0]  data addr  R1b */
#define MMC_SEND_WRITE_PROT      30        /* adtc [31:0]  wpdata addr R1  */

/* class 5 */
#define MMC_ERASE_GROUP_START    35        /* ac   [31:0]  data addr  R1  */
#define MMC_ERASE_GROUP_END      36        /* ac   [31:0]  data addr  R1  */
#define MMC_ERASE                37        /* ac                      R1b */
#define SD_ERASE_WR_BLK_START    32        /* ac   [31:0]  data addr  R1  */
#define SD_ERASE_WR_BLK_END      33        /* ac   [31:0]  data addr  R1  */
#define SD_ERASE                 38        /* ac                      R1b */

/* class 9 */
#define MMC_FAST_IO              39        /* ac   <Complex>          R4  */
#define MMC_GO_IRQ_STATE         40        /* bcr                     R5  */

/* class 7 */
#define MMC_LOCK_UNLOCK          42        /* adtc                    R1b */

/* class 8 */
#define MMC_APP_CMD              55        /* ac   [31:16] RCA        R1  */
#define MMC_GEN_CMD              56        /* adtc [0]     RD/WR      R1b */

/* SD commands                           type  argument     response */
/* class 8 */
/* This is basically the same command as for MMC with some quirks. */
#define SD_SEND_RELATIVE_ADDR     3        /* ac                      R6  */
#define SD_CMD8                   8        /* bcr  [31:0]  OCR        R3  */

/* Application commands */
#define SD_APP_SET_BUS_WIDTH      6        /* ac   [1:0]   bus width  R1   */
#define SD_APP_OP_COND           41        /* bcr  [31:0]  OCR        R1 (R4)  */
#define SD_APP_SEND_SCR          51        /* adtc                    R1   */

/**
 * @brief MMC status in R1<br>
 * Type<br>
 *   e : error bit<br>
 *   s : status bit<br>
 *   r : detected and set for the actual command response<br>
 *   x : detected and set during command execution. the host must poll
 *       the card by sending status command in order to read these bits.
 * Clear condition<br>
 *   a : according to the card state<br>
 *   b : always related to the previous command. Reception of
 *       a valid command will clear it (with a delay of one command)<br>
 *   c : clear by read<br>
 */

#define R1_OUT_OF_RANGE         (1UL << 31)    /* er, c */
#define R1_ADDRESS_ERROR        (1 << 30)    /* erx, c */
#define R1_BLOCK_LEN_ERROR      (1 << 29)    /* er, c */
#define R1_ERASE_SEQ_ERROR      (1 << 28)    /* er, c */
#define R1_ERASE_PARAM          (1 << 27)    /* ex, c */
#define R1_WP_VIOLATION         (1 << 26)    /* erx, c */
#define R1_CARD_IS_LOCKED       (1 << 25)    /* sx, a */
#define R1_LOCK_UNLOCK_FAILED   (1 << 24)    /* erx, c */
#define R1_COM_CRC_ERROR        (1 << 23)    /* er, b */
#define R1_ILLEGAL_COMMAND      (1 << 22)    /* er, b */
#define R1_CARD_ECC_FAILED      (1 << 21)    /* ex, c */
#define R1_CC_ERROR             (1 << 20)    /* erx, c */
#define R1_ERROR                (1 << 19)    /* erx, c */
#define R1_UNDERRUN             (1 << 18)    /* ex, c */
#define R1_OVERRUN              (1 << 17)    /* ex, c */
#define R1_CID_CSD_OVERWRITE    (1 << 16)    /* erx, c, CID/CSD overwrite */
#define R1_WP_ERASE_SKIP        (1 << 15)    /* sx, c */
#define R1_CARD_ECC_DISABLED    (1 << 14)    /* sx, a */
#define R1_ERASE_RESET          (1 << 13)    /* sr, c */
#define R1_STATUS(x)            (x & 0xFFFFE000)
#define R1_CURRENT_STATE(x)     ((x & 0x00001E00) >> 9)    /* sx, b (4 bits) */
#define R1_READY_FOR_DATA       (1 << 8)    /* sx, a */
#define R1_APP_CMD              (1 << 5)    /* sr, c */

/**
 * @brief SD/MMC card OCR register bits
 */
#define OCR_ALL_READY           (1UL << 31)    /* Card Power up status bit */
#define OCR_HC_CCS              (1 << 30)    /* High capacity card */
#define OCR_VOLTAGE_RANGE_MSK   (0x00FF8000)

#define SD_SEND_IF_ARG          0x000001AA
#define SD_SEND_IF_ECHO_MSK     0x000000FF
#define SD_SEND_IF_RESP         0x000000AA

/**
 * @brief R3 response definitions
 */
#define CMDRESP_R3_OCR_VAL(n)           (((U32) n) & 0xFFFFFF)
#define CMDRESP_R3_S18A                 (((U32) 1 ) << 24)
#define CMDRESP_R3_HC_CCS               (((U32) 1 ) << 30)
#define CMDRESP_R3_INIT_COMPLETE        (((U32) 1 ) << 31)

/**
 * @brief R6 response definitions
 */
#define CMDRESP_R6_RCA_VAL(n)           (((U32) (n >> 16)) & 0xFFFF)
#define CMDRESP_R6_CARD_STATUS(n)       (((U32) (n & 0x1FFF)) | \
                                         ((n & (1 << 13)) ? (1 << 19) : 0) | \
                                         ((n & (1 << 14)) ? (1 << 22) : 0) | \
                                         ((n & (1 << 15)) ? (1 << 23) : 0))

/**
 * @brief R7 response definitions
 */
/** Echo-back of check-pattern */
#define CMDRESP_R7_CHECK_PATTERN(n)     (((U32) n ) & 0xFF)
/** Voltage accepted */
#define CMDRESP_R7_VOLTAGE_ACCEPTED     (((U32) 1 ) << 8)

/**
 * @brief CMD3 command definitions
 */
/** Card Address */
#define CMD3_RCA(n)         (((U32) (n & 0xFFFF) ) << 16)

/**
 * @brief CMD7 command definitions
 */
/** Card Address */
#define CMD7_RCA(n)         (((U32) (n & 0xFFFF) ) << 16)

/**
 * @brief CMD8 command definitions
 */
/** Check pattern */
#define CMD8_CHECKPATTERN(n)            (((U32) (n & 0xFF) ) << 0)
/** Recommended pattern */
#define CMD8_DEF_PATTERN                    (0xAA)
/** Voltage supplied.*/
#define CMD8_VOLTAGESUPPLIED_27_36     (((U32) 1 ) << 8)

/**
 * @brief CMD9 command definitions
 */
#define CMD9_RCA(n)         (((U32) (n & 0xFFFF) ) << 16)

/**
 * @brief CMD13 command definitions
 */
#define CMD13_RCA(n)            (((U32) (n & 0xFFFF) ) << 16)

/**
 * @brief APP_CMD command definitions
 */
#define CMD55_RCA(n)            (((U32) (n & 0xFFFF) ) << 16)

/**
 * @brief ACMD41 command definitions
 */
#define ACMD41_OCR(n)                   (((U32) n) & 0xFFFFFF)
#define ACMD41_S18R                     (((U32) 1 ) << 24)
#define ACMD41_XPC                      (((U32) 1 ) << 28)
#define ACMD41_HCS                      (((U32) 1 ) << 30)

/**
 * @brief ACMD6 command definitions
 */
#define ACMD6_BUS_WIDTH(n)              ((U32) n & 0x03)
#define ACMD6_BUS_WIDTH_1               (0)
#define ACMD6_BUS_WIDTH_4               (2)

/** @brief Card type defines
 */
#define CARD_TYPE_SD    (1 << 0)
#define CARD_TYPE_4BIT  (1 << 1)
#define CARD_TYPE_8BIT  (1 << 2)
#define CARD_TYPE_HC    (OCR_HC_CCS)/*!< high capacity card > 2GB */

/**
 * @brief SD/MMC sector size in bytes
 */
#define MMC_SECTOR_SIZE     512

#define NUM_CMD_RETRIES     2
#define DEFAULT_COMMAND_TIMEOUT 1000000
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
/**
 * @brief  SD/MMC application specific commands for SD cards only - these
 * must be preceded by the SDMMC CMD55 to work correctly
 */
typedef enum {
    SD_SET_BUS_WIDTH,        /*!< Set the SD bus width */
    SD_SEND_STATUS,            /*!< Send the SD card status */
    SD_SEND_WR_BLOCKS,        /*!< Send the number of written clocks */
    SD_SET_ERASE_COUNT,        /*!< Set the number of blocks to pre-erase */
    SD_SENDOP_COND,            /*!< Send the OCR register (init) */
    SD_CLEAR_CARD_DET,        /*!< Set or clear the 50K detect pullup */
    SD_SEND_SCR,            /*!< Send the SD configuration register */
    SD_INVALID_APP_CMD        /*!< Invalid SD application command */
} SD_APP_CMD_T;

/**
 * @brief  Possible SDMMC response types
 */
typedef enum {
    SDMMC_RESPONSE_R1,        /*!< Typical status */
    SDMMC_RESPONSE_R1B,        /*!< Typical status with busy */
    SDMMC_RESPONSE_R2,        /*!< CID/CSD registers (CMD2 and CMD10) */
    SDMMC_RESPONSE_R3,        /*!< OCR register (CMD1, ACMD41) */
    SDMMC_RESPONSE_R4,        /*!< Fast IO response word */
    SDMMC_RESPONSE_R5,        /*!< Go IRQ state response word */
    SDMMC_RESPONSE_R6,        /*!< Published RCA response */
    SDMMC_RESPONSE_NONE        /*!< No response expected */
} SDMMC_RESPONSE_T;

/**
 * @brief  Possible SDMMC card state types
 */
typedef enum {
    SDMMC_IDLE_ST = 0,    /*!< Idle state */
    SDMMC_READY_ST,        /*!< Ready state */
    SDMMC_IDENT_ST,        /*!< Identification State */
    SDMMC_STBY_ST,        /*!< standby state */
    SDMMC_TRAN_ST,        /*!< transfer state */
    SDMMC_DATA_ST,        /*!< Sending-data State */
    SDMMC_RCV_ST,        /*!< Receive-data State */
    SDMMC_PRG_ST,        /*!< Programming State */
    SDMMC_DIS_ST        /*!< Disconnect State */
} SDMMC_STATE_T;

/**
 * @brief SD/MMC Card specific setup data structure
 */
typedef struct {
    U32 response[4];                        /*!< Most recent response */
    U32 cid[4];                            /*!< CID of acquired card  */
    U32 csd[4];                            /*!< CSD of acquired card */
    U32 ext_csd[512 / 4];                    /*!< Ext CSD */
    U32 card_type;                            /*!< Card Type */
    U16 rca;                                /*!< Relative address assigned to card */
    U32 speed;                                /*!< Speed */
    U32 block_len;                            /*!< Card sector size */
    U32 device_size;                        /*!< Device Size */
    U32 blocknr;                            /*!< Block Number */
    U32 clk_rate;                            /*! Clock rate */
} SDMMC_CARD_T;

typedef struct 
{
    U32 command;
    U32 state;
    U32 ocr;
    S32 tries;
} SDMMC_ACQUIRE_INFO;   

/* Card specific setup data */
typedef struct _mci_card_struct {
    sdif_device sdif_dev;
    SDMMC_CARD_T card_info;
    SDMMC_ACQUIRE_INFO acquire_info;
} MCI_CARD_STRUCT;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvMemSysSdMmcBlock_Init(void);

void DrvMemSysSdMmcBlock_Handler(void);

MEM_HNDL DrvMemSysSdMmcBlock_RegisterChannel(SDMMC_PORT port);
//================================================================================================//


#endif /* SD_SDMMC_H */
