//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Low level disk interface module include file
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRVFAT_H
#define DRVFAT_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "mem\DrvMem.h"
#include "rtc\DrvRtc.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
/* Disk Status Bits (DSTATUS) */
#define STA_OK            0x00
#define STA_NOINIT        0x01    /* Drive not initialized */
#define STA_NODISK        0x02    /* No medium in the drive */
#define STA_PROTECT       0x04    /* Write protected */


/* Command code for disk_ioctrl fucntion */
#define CTRL_SYNC            0    /* Flush disk cache (for write functions) */
#define GET_SECTOR_COUNT    1    /* Get media size (for only f_mkfs()) */
#define GET_SECTOR_SIZE        2    /* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define GET_BLOCK_SIZE        3    /* Get erase block size (for only f_mkfs()) */
#define CTRL_ERASE_SECTOR    4    /* Force erased a block of sectors (for only _USE_ERASE) */
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
/* Status of Disk Functions */
typedef U8    DSTATUS;

/* Results of Disk Functions */
typedef enum {
    RES_OK = 0,        /* 0: Successful */
    RES_ERROR,        /* 1: R/W Error */
    RES_WRPRT,        /* 2: Write Protected */
    RES_NOTRDY,        /* 3: Not Ready */
    RES_PARERR        /* 4: Invalid Parameter */
} DRESULT;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvFat_Init(void);

U8 disk_register(MEM_HNDL mem_handle, U32 sector_count, U16 sector_size, U32 block_size);

DSTATUS disk_initialize (U8 pdrv);
DSTATUS disk_status (U8 pdrv);
DRESULT disk_read (U8 pdrv, U8* buff, U32 sector, U32 count);
DRESULT disk_write (U8 pdrv, const U8* buff, U32 sector, U32 count);
DRESULT disk_ioctl (U8 pdrv, U8 cmd, void* buff);

void DrvFat_setRtcHndl (RTC_HNDL rtc_hndl);
U32 get_fattime (void);
//================================================================================================//



#endif /* DRVFAT_H */
