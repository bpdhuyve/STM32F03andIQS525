//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the SPI Master Device driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SPI__DRVSPIMASTERDEVICE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SPI__DRVSPIMASTERDEVICE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SPI__DRVSPIMASTERDEVICE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of SPI Master devices
#ifndef SPI_DEVICE_COUNT
	#define SPI_DEVICE_COUNT            6
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "DrvSpiMasterDevice.h"
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
    SPI_CHANNEL_HNDL    spi_channel_hndl;
    DRVGPIO_PIN_HNDL          pin_cs;
    SPI_CONFIG_STRUCT   config_struct;
    EVENT_CALLBACK      msg_complete;
    BOOL             selected;
    BOOL             terminate;
    BOOL             active;
}
SPI_DEVICE_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrvSpiMasterDevice_ChannelCallBack(SPI_CHANNEL_HNDL channel_hndl);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SPI_DEVICE_STRUCT                    spi_device_struct[SPI_DEVICE_COUNT];
static U8                           spi_device_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DrvSpiMasterDevice_ChannelCallBack(SPI_CHANNEL_HNDL channel_hndl)
{
    U8              device_id;
    SPI_DEVICE_STRUCT*      spi_dev_ptr;

    LOG_DEV("DrvSpiMasterDevice_ChannelCallBack");

    for(spi_dev_ptr = spi_device_struct, device_id = 0; spi_dev_ptr < &spi_device_struct[spi_device_count]; spi_dev_ptr++, device_id++)
    {
        if((spi_dev_ptr->spi_channel_hndl == channel_hndl) && (spi_dev_ptr->selected == TRUE))
        {
            break;
        }
    }

    if(spi_dev_ptr != &spi_device_struct[spi_device_count])
    {
        spi_dev_ptr->active = FALSE;

        if(spi_dev_ptr->terminate == TRUE)
        {
            DrvSpiMasterDevice_Deselect(device_id);
        }

        if(spi_dev_ptr->msg_complete != NULL)
        {
            spi_dev_ptr->msg_complete();
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSpiMasterDevice_Init(void)
{
    MODULE_INIT_ONCE();

    LOG_DEV("DrvSpiMasterDevice_Init");
    spi_device_count = 0;
    MEMSET((VPTR)spi_device_struct, 0, SIZEOF(spi_device_struct));

    DrvSpiMasterChannel_RegisterMsgComplete(DrvSpiMasterDevice_ChannelCallBack);

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
SPI_DEVICE_ID DrvSpiMasterDevice_Register(SPI_CHANNEL_HNDL spi_channel_hndl,
                                          DRVGPIO_PIN_HNDL pin_cs,
                                          SPI_CONFIG_STRUCT* config_struct_ptr,
                                          EVENT_CALLBACK msg_complete)
{
    MODULE_CHECK();

    SPI_DEVICE_STRUCT*      spi_dev_hndl = &spi_device_struct[spi_device_count];

    LOG_DEV("DrvSpiMasterDevice_Register");

    if(spi_channel_hndl == NULL)
    {
        LOG_WRN("illegal spi channel");
        return INVALID_SPI_DEVICE_ID;
    }

    if(spi_device_count < SPI_DEVICE_COUNT)
    {
        spi_dev_hndl->spi_channel_hndl = spi_channel_hndl;
        spi_dev_hndl->pin_cs = pin_cs;
        if(config_struct_ptr != NULL)
        {
            MEMCPY((VPTR)&(spi_dev_hndl->config_struct), (VPTR)config_struct_ptr, SIZEOF(SPI_CONFIG_STRUCT));
        }
        if(msg_complete != NULL)
        {
            spi_dev_hndl->msg_complete = msg_complete;
        }
        spi_device_count++;

        DrvSpiMasterDevice_Deselect(spi_device_count-1);

        return (spi_device_count-1);
    }

    LOG_ERR("SPI device count overrun");
    return INVALID_SPI_DEVICE_ID;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSpiMasterDevice_Config(SPI_DEVICE_ID device_id, SPI_CONFIG_STRUCT* config_struct_ptr)
{
    MODULE_CHECK();

    LOG_DEV("DrvSpiMasterDevice_Config");

    if(device_id < spi_device_count)
    {
        if(config_struct_ptr != NULL)
        {
            MEMCPY((VPTR)&spi_device_struct[device_id].config_struct, (VPTR)config_struct_ptr, SIZEOF(SPI_CONFIG_STRUCT));
            return TRUE;
        }
    }
    LOG_WRN("SPI illegal device config - %d", PU8(device_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSpiMasterDevice_MsgComplete(SPI_DEVICE_ID device_id, EVENT_CALLBACK msg_complete)
{
    MODULE_CHECK();

    LOG_DEV("DrvSpiMasterDevice_MsgComplete");

    if(device_id < spi_device_count)
    {
        spi_device_struct[device_id].msg_complete = msg_complete;
        return TRUE;
    }
    LOG_WRN("I2C illegal device msg complete - %d", PU8(device_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSpiMasterDevice_Select(SPI_DEVICE_ID device_id)
{
    MODULE_CHECK();

    SPI_DEVICE_STRUCT*      spi_dev_hndl = &spi_device_struct[device_id];
    SPI_DEVICE_STRUCT*      spi_dev_ptr;

    LOG_DEV("DrvSpiMasterDevice_Select");

    if(device_id < spi_device_count)
    {
        for(spi_dev_ptr = spi_device_struct; spi_dev_ptr < &spi_device_struct[spi_device_count]; spi_dev_ptr++)
        {
            if((spi_dev_ptr != spi_dev_hndl) &&
               (spi_dev_ptr->spi_channel_hndl == spi_dev_hndl->spi_channel_hndl) &&
               (spi_dev_ptr->selected == TRUE))
            {
                LOG_WRN("SPI channel is busy - %d", PU8(device_id));
                return FALSE;
            }
        }

        if(DrvSpiMasterChannel_Config(spi_dev_hndl->spi_channel_hndl, &(spi_dev_hndl->config_struct)))
        {
            DrvGpio_SetPin(spi_dev_hndl->pin_cs, FALSE);
            spi_dev_hndl->selected = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSpiMasterDevice_Deselect(SPI_DEVICE_ID device_id)
{
    MODULE_CHECK();

    SPI_DEVICE_STRUCT*      spi_dev_hndl = &spi_device_struct[device_id];

    LOG_DEV("DrvSpiMasterDevice_Deselect");

    if(device_id < spi_device_count)
    {
        DrvGpio_SetPin(spi_dev_hndl->pin_cs, TRUE);
        spi_dev_hndl->selected = FALSE;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSpiMasterDevice_Exchange(SPI_DEVICE_ID device_id, U8* write_byte_ptr, U8* read_byte_ptr, U16 count)
{
    MODULE_CHECK();

    SPI_DEVICE_STRUCT*      spi_dev_hndl = &spi_device_struct[device_id];

    LOG_DEV("DrvSpiMasterDevice_ExchangeData");

    if(device_id < spi_device_count)
    {
        if(spi_dev_hndl->selected == FALSE)
        {
            LOG_WRN("SPI device not selected - %d", PU8(device_id));
            return FALSE;
        }

        spi_dev_hndl->active = TRUE;
        spi_dev_hndl->terminate = FALSE;

        if(DrvSpiMasterChannel_Exchange(spi_dev_hndl->spi_channel_hndl, write_byte_ptr, read_byte_ptr, count))
        {
            if(spi_dev_hndl->msg_complete == NULL)
            {
                while(spi_dev_hndl->active == TRUE)
                {}
            }
            return TRUE;
        }

        spi_dev_hndl->active = FALSE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSpiMasterDevice_SelectExchange(SPI_DEVICE_ID device_id, U8* write_byte_ptr, U8* read_byte_ptr, U16 count)
{
    MODULE_CHECK();

    SPI_DEVICE_STRUCT*      spi_dev_hndl = &spi_device_struct[device_id];

    LOG_DEV("DrvSpiMasterDevice_SelectExchangeData");

    if(device_id < spi_device_count)
    {
        if(DrvSpiMasterDevice_Select(device_id))
        {
            spi_dev_hndl->active = TRUE;
            spi_dev_hndl->terminate = TRUE;

            if(DrvSpiMasterChannel_Exchange(spi_dev_hndl->spi_channel_hndl, write_byte_ptr, read_byte_ptr, count))
            {
                if(spi_dev_hndl->msg_complete == NULL)
                {
                    while(spi_dev_hndl->active == TRUE)
                    {}
                }
                return TRUE;
            }

            spi_dev_hndl->active = FALSE;
            DrvSpiMasterDevice_Deselect(device_id);
        }
    }
    return FALSE;
}
//================================================================================================//
