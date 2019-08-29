//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the common part of the memory driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MEM__DRVMEMSD_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__DRVMEMSD_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MEM__DRVMEMSD_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVMEMSD_COUNT
	#define DRVMEMSD_COUNT		        1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SD_USE_CRC
    #define SD_USE_CRC                  1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SD_CARD_MIN_SPEED
    #define SD_CARD_MIN_SPEED           400000
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SD_CARD_MAX_SPEED
    #define SD_CARD_MAX_SPEED           10000000
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "mem\DrvMemSD.h"

//STD
#include "crc\StdCrc.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SD_CARD_WRITE_BLOCKSIZE         512

#define COMMAND_FRAME_MASK              0x7F
#define RESPONSE_MSK_IDLE               0x01
#define SD_CMD_TIMEOUT                  100
#define SD_IDLE_WAIT_MAX                10000
#define BIT_ILLEGAL_COMMAND             0x04
#define SD_RETRY_COUNT                  50

#define NEC                             100000
#define NCR                             8       //minimum command response time in bytes
#define NWR                             1       // minimum 1
//================================================================================================//


//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    NOT_PRESENT,
    NOT_INITIALISED,
    INITIALISING,
    AVAILABLE,
    BUSY,
}
SD_STATUS;

typedef struct
{
    SPI_DEVICE_ID       spi_device_id;
    DRVGPIO_PIN_HNDL    pin_card_detect_hndl;
    DRVGPIO_PIN_HNDL    pin_write_protect_hndl;
    SD_STATUS           card_status;
    U32                 read_blocksize;
	U32					spi_channel_speed;
    U32                 card_size;
    U32                 nac;
}
MEM_CTRL_STRUCT;

typedef enum
{
    CMD_GO_IDLE_STATE =             0,
    CMD_SEND_OP_COND =              1,
    CMD_SEND_CSD =                  9,
    CMD_STOP_TRANSMISSION =         12,
    CMD_SEND_STATUS =               13,
    CMD_SET_BLOCKLEN =              16,
    CMD_READ_SINGLE_BLOCK =         17,
    CMD_READ_MULTIPLE_BLOCK =       18,
    CMD_WRITE_SINGLE_BLOCK=         24,
    CMD_WRITE_MULTIPLE_BLOCK =      25,
    CMD_APP_CMD =                   55,
    CMD_READ_OCR =                  58,
    CMD_CRC_ON_OFF =                59,

    //application specific commands
    ACMD_SEND_OP_COND =             41,

    //version 2.0 commands
    CMD_SEND_IF_COND =              8,

    //this command does not exists
    CMD_ILLEGAL =                   2,
}
SD_COMMAND;

typedef enum
{
    DATA_TOKEN_CMD17 =              0xFE,
    DATA_TOKEN_CMD18 =              0xFE,
    DATA_TOKEN_CMD24 =              0xFE,
    DATA_TOKEN_CMD25 =              0xFC,
}
DATA_TOKENS;

typedef enum
{
    R1,
    R1B,
    R2,
    R3
}
RESPONSE_TYPE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrvMemSD_GetCardStatus(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr);
static BOOL DrvMemSD_GetWriteProtected(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr);

static void DrvMemSD_SetDelay(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U8 delay);
static BOOL DrvMemSD_SetReadBlocksize(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U32 new_block_size);
static BOOL DrvMemSD_SendCommand(SPI_DEVICE_ID spi_device_id, SD_COMMAND command, RESPONSE_TYPE response_type, U32 argument, U8* response_ptr);
static BOOL DrvMemSD_ConfigSPIMode(MEM_CTRL_STRUCT* ctrl_struct_ptr);

static BOOL DrvMemSD_Read(MEM_ID mem_id, U32 address, U8* data_ptr, U16 length);
static BOOL DrvMemSD_ReadBlock(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U32 address, U8* data_ptr);
static BOOL DrvMemSD_Write(MEM_ID mem_id, U32 address, U8* data_ptr, U16 length);
static BOOL DrvMemSD_WriteBlock(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U32 address, U8* data_ptr);

static void DrvMemSD_Handler(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static MEM_HOOK_LIST                        mem_hook_list;
static MEM_STRUCT                           mem_struct[DRVMEMSD_COUNT];
static MEM_CTRL_STRUCT                      mem_ctrl_struct[DRVMEMSD_COUNT];
static U8                                   mem_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DrvMemSD_GetCardStatus(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr)
{
    U32         i = 0;
    U8          reply_byte;
    
    // CHECK IF SD CARD IS PRESENT
    if(DrvGpio_GetPin(mem_ctrl_struct_ptr->pin_card_detect_hndl) == TRUE)
    {
        mem_ctrl_struct_ptr->card_status = NOT_PRESENT;
    }
    // IF SO, CHECK IF INITIALISATION IS NEEDED
    else if(mem_ctrl_struct_ptr->card_status <= NOT_INITIALISED)
    {
        mem_ctrl_struct_ptr->card_status = INITIALISING;
        if(DrvMemSD_ConfigSPIMode(mem_ctrl_struct_ptr))
        {
            mem_ctrl_struct_ptr->card_status = AVAILABLE;
        }
        else
        {
            mem_ctrl_struct_ptr->card_status = NOT_INITIALISED;
        }
    }
    // OTHERWISE CHECK IF BUSY OR AVAILABLE
    else
    {
        do
        {
            DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
            DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, &reply_byte, 1);
            DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
            if(reply_byte != 0xFF)
            {
                // Card is busy
                mem_ctrl_struct_ptr->card_status = BUSY;
            }
            else
            {
                // Card is not busy
                mem_ctrl_struct_ptr->card_status = AVAILABLE;
            }
            i++;
        }
        while((i <= NEC) && (mem_ctrl_struct_ptr->card_status == BUSY));
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_GetWriteProtected(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr)
{
    return DrvGpio_GetPin(mem_ctrl_struct_ptr->pin_write_protect_hndl);
}
//------------------------------------------------------------------------------------------------//
static void DrvMemSD_SetDelay(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U8 delay)
{
    U8  write_ff = 0xFF;
    
    while(delay > 0)
    { 
        DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, &write_ff, 1);
        delay--;
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_SetReadBlocksize(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U32 new_block_size)
{
    U8  reply_byte;
    
    if(mem_ctrl_struct_ptr->read_blocksize == new_block_size)
    {
        return TRUE;
    }
    if((new_block_size <= 512) && (new_block_size > 0))
    {
        // Set Block length
        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_SET_BLOCKLEN, R1, new_block_size, &reply_byte);
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
        if(reply_byte != 0x00)
        {
            LOG_WRN("BLOCKLEN CMD didn't work");
            return FALSE;
        }
        mem_ctrl_struct_ptr->read_blocksize = new_block_size;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_SendCommand(SPI_DEVICE_ID spi_device_id, SD_COMMAND command, RESPONSE_TYPE response_type, U32 argument, U8* response_ptr)
{
    U8      command_frame[7];
    U8      i = 0;
    U8      response_length;
    #if SD_USE_CRC
    U8      crc_temp;
    U8      crc = 0;
    U8*     crc_data;
    U8      j;
    #endif
    
	switch(response_type)
    {
	case R1:
	case R1B:
		response_length = 1;
		break;
	case R2:
		response_length = 2;
		break;
	case R3:
		response_length = 5;
		break;
	default:
        return FALSE;
	}
    command_frame[0] = 0xFF;
    command_frame[1] = (U8)((command | 0x40) & COMMAND_FRAME_MASK);
    command_frame[2] = (U8)((argument >> 24) & 0x000000FF);
    command_frame[3] = (U8)((argument >> 16) & 0x000000FF);
    command_frame[4] = (U8)((argument >> 8) & 0x000000FF);
    command_frame[5] = (U8)(argument & 0x000000FF);
    
    // Implement CRC here
    #if SD_USE_CRC
    {
        crc_data = &command_frame[1];
        for (i = 0; i < 5; i++, crc_data++)
        {
           crc_temp = *crc_data;
     
           for (j = 0; j < 8; j++)
           {
              crc = crc << 1;
              if ((crc_temp ^ crc) & 0x80)
              {
                  crc = crc ^ 0x09;
              }
              crc_temp = crc_temp << 1;
           }
     
            crc = crc & 0x7F;
        }
        crc <<= 1;
        crc |= 1;
        command_frame[6] = crc;
    }
    #else
    {
        if(command == CMD_GO_IDLE_STATE)
        {
            command_frame[6] = 0x95;
        }
        else
        {
            command_frame[6] = 0x00;
        }
    }
    #endif
    
    // Write the command with SPI
    DrvSpiMasterDevice_WriteData(spi_device_id, &command_frame[0], 7);
    i = 0;
    response_ptr[0] = 0xFF;
    while((response_ptr[0] == 0xFF) && (i <= NCR))
    {
		DrvSpiMasterDevice_ReadData(spi_device_id, &response_ptr[0], 1);
		i++;
	}
    
    // check for timeout
    if(i > NCR)
    {
		return FALSE;
	}
    
    // get additional bytes
    if((response_type != R1) && (response_type != R1B))
    {
        DrvSpiMasterDevice_ReadData(spi_device_id, &response_ptr[1], response_length - 1);
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_ConfigSPIMode(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr)
{   
    U32         i = 0;
    U8          data_in[18];
    U32         csd_taac;
    U32         csd_tran_speed;
    BOOL        cmd_complete = FALSE;
    U8          cmd_response[5];
    const U8    coefficients[16] = {0,10,12,13,15,20,25,30,35,40,45,50,55,60,70,80};
    
    // Init SD card peripherals by enabling the clock
    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvMemSD_SetDelay(mem_ctrl_struct_ptr, 12);
    DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
    
    // Start in SPI mode
    MEMSET((VPTR)cmd_response, 0, SIZEOF(cmd_response));
    i = 0;
    while(((cmd_response[0] & RESPONSE_MSK_IDLE) != RESPONSE_MSK_IDLE))
    {
        // check for loop timeout
        if(i++ > SD_IDLE_WAIT_MAX)
        {
            LOG_WRN("Not in SPI mode");
            return FALSE;
        }
        
        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_GO_IDLE_STATE, R1, 0, cmd_response);
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
    }
    
    // Check idle state
    i = 0;
    while((cmd_response[0] & RESPONSE_MSK_IDLE) == RESPONSE_MSK_IDLE)
    {
        // check for loop timeout
        if(i++ > SD_IDLE_WAIT_MAX)
        {
            LOG_WRN("Card not in idle state");
            return FALSE;
        }
        
        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, ACMD_SEND_OP_COND, R1, 0, cmd_response);
        DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_APP_CMD, R1, 0, cmd_response);
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
    }
    
    // Check if the operating voltage is correct
    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_READ_OCR, R3, 0, cmd_response);
    DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
    if(((cmd_response[1] == 0x00) && ((cmd_response[2] & 0x80) == 0x00)))
    {
        LOG_WRN("Supply Voltage not OK");
        return FALSE;
    }
    
    // Set CRC mode on/off
    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_CRC_ON_OFF, R1, (U32)(SD_USE_CRC > 0), cmd_response);
    DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
    if(cmd_response[0] != 0x00)
    {
        LOG_WRN("CRC CMD didn't work");
        return FALSE;
    }
    
    // Set Block length
    mem_ctrl_struct_ptr->read_blocksize = 0;
    if(DrvMemSD_SetReadBlocksize(mem_ctrl_struct_ptr, SD_CARD_WRITE_BLOCKSIZE) == FALSE)
    {
        LOG_WRN("Failed to set block size");
        return FALSE;
    }
    
    // Check CSD register (Card Specific Data)
    DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
    DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_SEND_CSD, R1, 0, cmd_response);
    if(cmd_response[0] != 0x00)
    {
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
        LOG_WRN("CSD CMD didn't work");
        return FALSE;
    }
    MEMSET((VPTR)data_in, 0xFF, SIZEOF(data_in));
    i = 0;
    while(data_in[0] == 0xFF)
    {
        if(i++ > 100)
        {
            DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
            LOG_WRN("Read Timeout");
            return FALSE;
        }
        
        DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, &data_in[0], 1);
    }
    if(data_in[0] != DATA_TOKEN_CMD17)
    {
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
        LOG_WRN("Bad csd token received");
        return FALSE;
    }
    DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, &data_in[0], (16 + 2)); // 128 bit register + 2 CRC bytes
    DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
    
    LOG_DBG("%02x", PU8A(data_in, 16));
    
    // Check Max speed, TAAC and NSAC and calculate Nac
    // Check max speed
    if(data_in[3] & 0x84)
    {
        LOG_WRN("Invalid max speed");
        return FALSE;
    }
    csd_tran_speed = (U32)coefficients[((data_in[3] & 0x78) >> 3)] * 100000;
    for (i = 0; i < ((data_in[3] & 0x07) - 1); i++)
    {
        csd_tran_speed *= 10;
    }
    if(csd_tran_speed > mem_ctrl_struct_ptr->spi_channel_speed)
    {
        //LOG_WRN("SPI speed could be higher [MAX %d kbit/s]", PU32(csd_tran_speed));
    }
    else
    {
        LOG_WRN("SPI speed too high, reconfigure [MAX %d kbit/s]", PU32(csd_tran_speed));
        return FALSE;
    }

    // Calculate TAAC in 10ns
    csd_taac = coefficients[((data_in[1] & 0x78) >> 3)];
    for (i = 0; i < ((data_in[1] & 0x07)-1); i++)
    {
        csd_taac *= 10;
    }
    
    // Calculate NSAC in multiples of 100 clock cycles
    mem_ctrl_struct_ptr->nac = (((((csd_taac >> 10) * mem_ctrl_struct_ptr->spi_channel_speed) >> 20) + ((U32)data_in[2] * 100)) >> 1);
    
    // Check Size of the SD card
    mem_ctrl_struct_ptr->card_size = (((U32)data_in[6] & 0x03) << 10) | ((U32)data_in[7] << 2) | (((U32)data_in[8] & 0xC0) >> 6);   // csd_c_size
    mem_ctrl_struct_ptr->card_size <<= (data_in[5] & 0x0F);                                                                         // blockcsd_read_bl_lensize;
    mem_ctrl_struct_ptr->card_size <<= ((((data_in[9] & 0x03) << 1) | ((data_in[10] & 0x80) >> 7)) + 2);                            // csd_c_size_mult

    LOG_DBG("Card size %x", PU32(mem_ctrl_struct_ptr->card_size));
    
    // Perform an ultimate test
    i = 0;
    do
    {
        DrvMemSD_GetCardStatus(mem_ctrl_struct_ptr);
        if(mem_ctrl_struct_ptr->card_status == BUSY)
        {
            LOG_WRN("Init not properly done");
            return FALSE;
        }
        else
        {
            DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
            cmd_complete = DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_SEND_STATUS, R2, 0, cmd_response);
            DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
        }
        i++;
    }
    while((i <= SD_CMD_TIMEOUT) && (cmd_complete == FALSE));
    if(cmd_response[0] == 0x00)
    {
        i = 0;
        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        do
        {
            cmd_complete = DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_ILLEGAL, R2, 0, cmd_response);
            i++;
        }
        while((i <= SD_CMD_TIMEOUT) && !(cmd_complete));
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
        if(i > SD_CMD_TIMEOUT)
        {
            LOG_WRN("Timeout");
            return FALSE;
        }
        if((cmd_response[0] & BIT_ILLEGAL_COMMAND) != BIT_ILLEGAL_COMMAND)
        {
            LOG_WRN("Wrong response");
            return FALSE;
        }
        LOG_DBG("SD card init done!");
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_Read(MEM_ID mem_id, U32 address, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*        mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    U16                     read_size;
    
    if(mem_id >= mem_count)
    {
        LOG_WRN("Illegal mem id");
        return FALSE;
    }
    
    while(length > 0)
    {
        if(length < 512)
        {
            read_size = length;
        }
        else
        {
            read_size = 512;
        }
        
        DrvMemSD_GetCardStatus(mem_ctrl_struct_ptr);
        if(mem_ctrl_struct_ptr->card_status != AVAILABLE)
        {
            LOG_DBG("SD Card is not available");
            return FALSE;
        }
        
        if(DrvMemSD_SetReadBlocksize(mem_ctrl_struct_ptr, read_size) == FALSE)
        {
            LOG_DBG("Failed to set blocksize");
            return FALSE;
        }
        
        if(DrvMemSD_ReadBlock(mem_ctrl_struct_ptr, address, data_ptr) == FALSE)
        {
            LOG_DBG("Failed to read block");
            return FALSE;
        }
        
        address += read_size;
        data_ptr += read_size;
        length -= read_size;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_ReadBlock(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U32 address, U8* data_ptr)
{
    U16             try_counter = 0;
    U32             i = 0;
    U8              reply_byte;
    BOOL            success = FALSE;
    U8              data_in[2];
    #if SD_USE_CRC
    U16             crc_generate;
    U16             crc_from_card;
    #endif
    
    // Start communicating with SD card
    while(success == FALSE)
    {
        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        
        // Send command
        DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_READ_SINGLE_BLOCK, R1, address, &reply_byte);
        
        // check response
        if(reply_byte != 0)
        {
            LOG_DBG("Read CMD failed");
        }
        else
        {
            // Send dummy
            DrvMemSD_SetDelay(mem_ctrl_struct_ptr, 1);
            // Receive token
            reply_byte = 0xFF;
            i = 0;
            while(reply_byte == 0xFF)
            {
                DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, &reply_byte, 1);
                if(i++ > mem_ctrl_struct_ptr->nac)
                {
                    LOG_DBG("No token received");
                    break;
                }
            }
            
            if(reply_byte != DATA_TOKEN_CMD17)
            {
                LOG_DBG("Wrong token");
            }
            else if(DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, data_ptr, mem_ctrl_struct_ptr->read_blocksize)) //read in data + crc
            {
                DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, data_in, 2);
                #if SD_USE_CRC
                {
                    crc_generate = StdCrcGenerateCrc16CCITT(data_ptr, mem_ctrl_struct_ptr->read_blocksize);
                    crc_from_card = ((data_in[1]) | (data_in[0] << 8));
                    success = (BOOL)(crc_generate == crc_from_card);
                }
                #else
                {
                    success = TRUE;
                }
                #endif
            }
        }
        
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
        
        if(try_counter++ > SD_RETRY_COUNT)
        {
            break;
        }
    }
    
    return success;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_Write(MEM_ID mem_id, U32 address, U8* data_ptr, U16 length)
{
    MEM_CTRL_STRUCT*        mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_id];
    
    if(mem_id >= mem_count)
    {
        LOG_WRN("Illegal mem id");
        return FALSE;
    }
    
    if((address & 0x01FF) > 0)
    {
        LOG_WRN("Address must be a multiple of 512");
        return FALSE;
    }
    if((length & 0x01FF) > 0)
    {
        LOG_WRN("Length must be a multiple of 512");
        return FALSE;
    }
    
    while(length > 0)
    {
        DrvMemSD_GetCardStatus(mem_ctrl_struct_ptr);
        if(mem_ctrl_struct_ptr->card_status != AVAILABLE)
        {
            LOG_DBG("SD Card is not available");
            return FALSE;
        }
        
        if(DrvMemSD_GetWriteProtected(mem_ctrl_struct_ptr))
        {
            LOG_DBG("SD card is write protected");
            return FALSE;
        }
        
        if(DrvMemSD_SetReadBlocksize(mem_ctrl_struct_ptr, SD_CARD_WRITE_BLOCKSIZE) == FALSE)
        {
            LOG_DBG("Failed to set blocksize");
            return FALSE;
        }
        
        if(DrvMemSD_WriteBlock(mem_ctrl_struct_ptr, address, data_ptr) == FALSE)
        {
            LOG_DBG("Failed to read block");
            return FALSE;
        }
        
        address += SD_CARD_WRITE_BLOCKSIZE;
        data_ptr += SD_CARD_WRITE_BLOCKSIZE;
        length -= SD_CARD_WRITE_BLOCKSIZE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvMemSD_WriteBlock(MEM_CTRL_STRUCT* mem_ctrl_struct_ptr, U32 address, U8* data_ptr)
{
    U8          data_out[2];
    U16         try_counter = 0;
    U8          reply_byte;
    BOOL        success = FALSE;
    U16         crc_generate = 0;
    
    // CRC
    #if SD_USE_CRC
    {
        crc_generate = StdCrcGenerateCrc16CCITT(data_ptr, SD_CARD_WRITE_BLOCKSIZE);
    }
    #endif
    
    while(success == FALSE)
    {
        // Start communicating with SD card
        DrvSpiMasterDevice_Select(mem_ctrl_struct_ptr->spi_device_id);
        
        // Send command
        DrvMemSD_SendCommand(mem_ctrl_struct_ptr->spi_device_id, CMD_WRITE_SINGLE_BLOCK, R1, address, &reply_byte);
        
        // check response
        if(reply_byte != 0)
        {
            LOG_DBG("Write CMD failed");
        }
        else
        {
            // Start Sending data
            data_out[0] = 0xFF; // Nwr time
            data_out[1] = DATA_TOKEN_CMD24;
            DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, data_out, 2);
            DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, data_ptr, SD_CARD_WRITE_BLOCKSIZE);
            data_out[0] = (U8)((crc_generate >> 8) & 0x000000FF);
            data_out[1] = (U8)(crc_generate & 0x000000FF);
            DrvSpiMasterDevice_WriteData(mem_ctrl_struct_ptr->spi_device_id, data_out, 2);
            
            // Receive response
            DrvSpiMasterDevice_ReadData(mem_ctrl_struct_ptr->spi_device_id, &reply_byte, 1);
            success = (BOOL)((reply_byte & 0x1F) == 0x05);
        }
        
        DrvSpiMasterDevice_Deselect(mem_ctrl_struct_ptr->spi_device_id);
        
        if(try_counter++ > SD_RETRY_COUNT)
        {
            break;
        }
    }
    
    return success;
}
//------------------------------------------------------------------------------------------------//
static void DrvMemSD_Handler(void)
{
    MEM_CTRL_STRUCT*        mem_ctrl_struct_ptr;
    
    for(mem_ctrl_struct_ptr = mem_ctrl_struct; mem_ctrl_struct_ptr < &mem_ctrl_struct[mem_count]; mem_ctrl_struct_ptr++)
    {
        if(DrvGpio_GetPin(mem_ctrl_struct_ptr->pin_card_detect_hndl) == TRUE)
        {
            mem_ctrl_struct_ptr->card_status = NOT_PRESENT;
        }
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvMemSD_Init(void)
{
    mem_hook_list.mem_clear_hook = NULL;
    mem_hook_list.mem_read_hook = DrvMemSD_Read;
    mem_hook_list.mem_write_hook = DrvMemSD_Write;
    mem_hook_list.mem_verify_hook = NULL;
    mem_hook_list.mem_flush_hook = NULL;

    MEMSET((VPTR)mem_struct, 0, SIZEOF(mem_struct));
    MEMSET((VPTR)mem_ctrl_struct, 0, SIZEOF(mem_ctrl_struct));
    mem_count = 0;
    
    Core_RegisterModuleHandler(DrvMemSD_Handler);
}
//------------------------------------------------------------------------------------------------//
MEM_HNDL DrvMemSD_Register(SPI_DEVICE_ID spi_device_id, DRVGPIO_PIN_HNDL pin_cd_hndl, DRVGPIO_PIN_HNDL pin_wp_hndl, U32 spi_speed)
{
    MEM_HNDL                mem_hndl;
    MEM_CTRL_STRUCT*        mem_ctrl_struct_ptr;
    SPI_CONFIG_STRUCT       spi_config_struct;
    
    for(mem_hndl = mem_struct, mem_ctrl_struct_ptr = mem_ctrl_struct; mem_hndl < &mem_struct[mem_count]; mem_hndl++, mem_ctrl_struct_ptr++)
    {
        if(mem_ctrl_struct_ptr->spi_device_id == spi_device_id)
        {
            return mem_hndl;
        }
    }
    
    if(mem_count < DRVMEMSD_COUNT)
    {
        if((spi_speed < SD_CARD_MIN_SPEED) || (spi_speed > SD_CARD_MAX_SPEED))
        {
            LOG_WRN("Speed is outside the boundaries of [%d, %d]", PU32((U32)SD_CARD_MIN_SPEED), PU32((U32)SD_CARD_MAX_SPEED));
            return NULL;
        }
        spi_config_struct.speed = spi_speed;
        spi_config_struct.mode = MODE_3;
        spi_config_struct.bitcount = 8;
        spi_config_struct.lsb_first = FALSE;
        if(DrvSpiMasterDevice_Config(spi_device_id, &spi_config_struct) == FALSE)
        {
            LOG_WRN("Configuration of SPI Master Device failed");
            return NULL;
        }            
        mem_ctrl_struct_ptr->spi_device_id = spi_device_id;
        mem_ctrl_struct_ptr->pin_card_detect_hndl = pin_cd_hndl;
        mem_ctrl_struct_ptr->pin_write_protect_hndl = pin_wp_hndl;
        mem_ctrl_struct_ptr->spi_channel_speed = spi_speed;
        mem_ctrl_struct_ptr->card_status = NOT_PRESENT;
        mem_ctrl_struct_ptr->read_blocksize = SD_CARD_WRITE_BLOCKSIZE;
        mem_hndl->hook_list_ptr = &mem_hook_list;
        mem_hndl->mem_id = mem_count;
        mem_count++;
        return mem_hndl;
    }
    return NULL;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvMemSD_IsPresent(MEM_HNDL mem_hndl)
{
    MEM_CTRL_STRUCT*        mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_hndl->mem_id];
    
    if((mem_hndl != NULL) && (mem_hndl->mem_id < mem_count))
    {
        DrvMemSD_GetCardStatus(mem_ctrl_struct_ptr);
        return (BOOL)(mem_ctrl_struct_ptr->card_status > NOT_INITIALISED);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U32 DrvMemSD_GetCardSize(MEM_HNDL mem_hndl)
{
    MEM_CTRL_STRUCT*        mem_ctrl_struct_ptr = &mem_ctrl_struct[mem_hndl->mem_id];
    
    if((mem_hndl != NULL) && (mem_hndl->mem_id < mem_count))
    {
        DrvMemSD_GetCardStatus(mem_ctrl_struct_ptr);
        if(mem_ctrl_struct_ptr->card_status > NOT_INITIALISED)
        {
            return mem_ctrl_struct_ptr->card_size;
        }
    }
    return 0;
}
//================================================================================================//