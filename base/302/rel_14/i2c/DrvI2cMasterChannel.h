//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the I2C Master Channel driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef I2C__DRVI2CMASTERCHANNEL_H
#define I2C__DRVI2CMASTERCHANNEL_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "i2c\ISysI2c.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                  I2C_CHANNEL_ID;

typedef BOOL (*I2C_WRITE_HOOK)(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count);

typedef BOOL (*I2C_READ_HOOK)(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count);

typedef BOOL (*I2C_CONFIG_HOOK)(I2C_CHANNEL_ID channel_id, I2C_CONFIG_STRUCT* config_struct_ptr);

typedef struct
{
    I2C_WRITE_HOOK                  write_hook;
    I2C_READ_HOOK                   read_hook;
    I2C_CONFIG_HOOK                 config_hook;
}
I2C_CHANNEL_HOOK_LIST;

typedef struct
{
    I2C_CHANNEL_HOOK_LIST*	        hook_list_ptr;
    I2C_CHANNEL_ID                  channel_id;
}
I2C_CHANNEL_STRUCT;

typedef I2C_CHANNEL_STRUCT*         I2C_CHANNEL_HNDL;

typedef void (*DRVI2C_MSG_COMPLETE)(I2C_CHANNEL_HNDL channel_hndl, BOOL success);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern DRVI2C_MSG_COMPLETE         drvi2cmasterchannel_msg_complete_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvI2cMasterChannel_Init(void);

BOOL DrvI2cMasterChannel_RegisterMsgComplete(DRVI2C_MSG_COMPLETE msg_complete_hook);

BOOL DrvI2cMasterChannel_WriteData(I2C_CHANNEL_HNDL channel_hndl, U8 address, U8* write_data_ptr, U16 count);

BOOL DrvI2cMasterChannel_ReadData(I2C_CHANNEL_HNDL channel_hndl, U8 address, U8* read_data_ptr, U16 count);

BOOL DrvI2cMasterChannel_Config(I2C_CHANNEL_HNDL channel_hndl, I2C_CONFIG_STRUCT* config_struct_ptr);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* I2C__DRVI2CMASTERCHANNEL_H */
