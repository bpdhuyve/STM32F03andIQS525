//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Low level disk interface module include file
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRVFAT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRVFAT_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               DRVFAT_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "ffconf.h"


//DRV lib include section
#include "mem\DrvMem.h"

//STD lib include section

//COM lib include section

//APP include section
#include "mem\DrvFat.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U8 drive_number;
    MEM_HNDL mem_handle;
    U32 sector_count;
    U16 sector_size;
    U32 block_size;
} FAT_DRIVE_INFO;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
FAT_DRIVE_INFO drive_info[_VOLUMES];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvFat_Init()
{
    MEMSET((VPTR)&drive_info, 0, sizeof(drive_info));
}
//------------------------------------------------------------------------------------------------//
/* connect a Drive to a drvMem handle */
U8 disk_register(MEM_HNDL mem_handle, U32 sector_count, U16 sector_size, U32 block_size)
{
    U8 i;
    for (i = 0; i < _VOLUMES; i++)
    {        
        // if already registered, update the data
        // if not, alloate a new slot
        if (drive_info[i].mem_handle == mem_handle ||
            ! drive_info[i].mem_handle)
        {
            drive_info[i].mem_handle = mem_handle;
            drive_info[i].sector_size = sector_size; 
            drive_info[i].sector_count = sector_count; 
            drive_info[i].block_size = block_size;
            drive_info[i].drive_number = i;           
            
            return drive_info[i].drive_number;
        }
    }
    
    return 0xFF; // no more drives available
}
//------------------------------------------------------------------------------------------------//
/* Initialize Disk Drive */
DSTATUS disk_initialize(U8 drv)
{
    /* if the disk is registered, we consider it initialized */    
    return disk_status(drv);
}
//------------------------------------------------------------------------------------------------//
DRESULT disk_ioctl(U8 drv, U8 ctrl, void *buff)
{
    DRESULT res;

    if (drv >= _VOLUMES)
        return RES_PARERR;
    
    if (! drive_info[drv].mem_handle)
        return RES_NOTRDY;

    res = RES_ERROR;

    switch (ctrl) {
    case CTRL_SYNC:    /* Make sure that no pending write process */
        if (DrvMem_Flush(drive_info[drv].mem_handle)) 
        {
            res = RES_OK;
        }
        break;
    case GET_SECTOR_COUNT:    /* Get number of sectors on the disk (U32) */
        if (drive_info[drv].sector_count > 0)
        {
            *(U32 *) buff = drive_info[drv].sector_count;
            res = RES_OK;
        }
        break;
    case GET_SECTOR_SIZE:    /* Get R/W sector size (U16) */
        if (drive_info[drv].sector_size > 0)
        {
            *(U16 *) buff = drive_info[drv].sector_size;
            res = RES_OK;
        }
        break;
    case GET_BLOCK_SIZE:/* Get erase block size in unit of sector (U32) */
        if (drive_info[drv].block_size > 0)
        {
            *(U32 *) buff = drive_info[drv].block_size;
            res = RES_OK;
        }
        break;
    default:
        res = RES_PARERR;
        break;
    }

    return res;
}
//------------------------------------------------------------------------------------------------//
DRESULT disk_read(U8 drv, U8* buff, U32 sector, U32 count)
{
    if (drv >= _VOLUMES)
        return RES_PARERR;
    
    if (! drive_info[drv].mem_handle)
        return RES_NOTRDY;

    if (DrvMem_ReadData(drive_info[drv].mem_handle, sector, buff, count)) {
        return RES_OK;
    }

    return RES_ERROR;
}
//------------------------------------------------------------------------------------------------//
DSTATUS disk_status(U8 drv)
{
    if (drv >= _VOLUMES)
        return STA_NODISK;
    
    if (! drive_info[drv].mem_handle)
        return STA_NOINIT;
    
    return STA_OK; // all ok
}
//------------------------------------------------------------------------------------------------//
DRESULT disk_write(U8 drv, const U8* buff, U32 sector, U32 count)
{

    if (drv >= _VOLUMES)
        return RES_PARERR;
    
    if (! drive_info[drv].mem_handle)
        return RES_NOTRDY;

    if (DrvMem_WriteData(drive_info[drv].mem_handle, sector, (void *) buff, count)) {
        return RES_OK;
    }

    return RES_ERROR;
}
//------------------------------------------------------------------------------------------------//
U32 get_fattime (void)
{
    // return 01/01/2015 00:00:00
    return ((U32)(2015 - 1980) << 25) |  // year
           ((U32) 1 << 21) | // month
           ((U32) 1 << 16) | // day
           ((U32) 0 << 11) | // hour
           ((U32) 0 << 5 ) | // minute
           ((U32) 0 ); // seconds
}
//================================================================================================//
