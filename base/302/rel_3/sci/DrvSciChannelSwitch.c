//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the interrupt driven SCI Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SCI__DRVSCICHANNELSWITCH_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SCI__DRVSCICHANNELSWITCH_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SCI__DRVSCICHANNELSWITCH_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of interrupt driven SCI channels
#ifndef DRVSCICHANNELSWITCH_MAX_CHANNELS
	#define DRVSCICHANNELSWITCH_MAX_CHANNELS			SCI_CHANNEL_COUNT
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the size of the buffer used to cache the TX bytes
#ifndef DRVSCICHANNELSWITCH_BUFFER_LENGTH
	#define DRVSCICHANNELSWITCH_BUFFER_LENGTH			1024
#endif

//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sci\DrvSciChannel.h"
#include "sci\DrvSciChannelSwitch.h"
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
   SCI_CHANNEL_HNDL                 registered_drvscichannelswitch_channel_hndl;
   U32                              read_index;
}
DRVSCICHANNELSWITCH_DATA;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSwitch_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook);
static BOOL DrvSciChannelSwitch_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook);
static BOOL DrvSciChannelSwitch_NotityTxDataReady(SCI_CHANNEL_ID channel_id);
#ifdef INCLUDE_MPCM
static BOOL DrvSciChannelSwitch_ConfigAsMpcm(SCI_CHANNEL_ID channel_id, SCI_SPEED speed, BOOL allow_rx);
static BOOL DrvSciChannelSwitch_SetMpcmFilter(SCI_CHANNEL_ID channel_id, BOOL enable);
#endif

static U8 DrvSciChannelSwitch_GetNextByte(U8 channel, U8* data_ptr, U8 length);
static void DrvSciChannelSwitch_OnNewByteFromSci(U8* byte_ptr, U8 length);

// we need to define as many functions as there are channels... so we define 5
// because in the general SCI_TX_GET_NEXT_BYTE_HOOK, no channel id is passed...

#if DRVSCICHANNELSWITCH_MAX_CHANNELS > 5
    #error too many channels in switch!
#endif
static U8 DrvSciChannelSwitch_GetNextByte0(U8* data_ptr, U8 length);
static U8 DrvSciChannelSwitch_GetNextByte1(U8* data_ptr, U8 length);
static U8 DrvSciChannelSwitch_GetNextByte2(U8* data_ptr, U8 length);
static U8 DrvSciChannelSwitch_GetNextByte3(U8* data_ptr, U8 length);
static U8 DrvSciChannelSwitch_GetNextByte4(U8* data_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SCI_CHANNEL_HOOK_LIST                drvscichannelswitch_channel_hook_list;
static SCI_CHANNEL_STRUCT                   drvscichannelswitch_channel_hndl;

static DRVSCICHANNELSWITCH_DATA             registered_channels_data[DRVSCICHANNELSWITCH_MAX_CHANNELS];
static U8                                   num_registered_channels;

static U8*                                  tx_circ_buffer;
static U32                                  write_index;

static SCI_RX_NEW_BYTE_HOOK                 registered_new_byte_hook;
static SCI_TX_GET_NEXT_BYTE_HOOK            registered_get_next_byte_hook;

static SCI_TX_GET_NEXT_BYTE_HOOK            list_of_drvsciswitch_get_next_byte_hooks[] = {DrvSciChannelSwitch_GetNextByte0,
                                                                                          DrvSciChannelSwitch_GetNextByte1,
                                                                                          DrvSciChannelSwitch_GetNextByte2,
                                                                                          DrvSciChannelSwitch_GetNextByte3,
                                                                                          DrvSciChannelSwitch_GetNextByte4 };

MODULE_DECLARE();
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSwitch_RegisterRxHook(SCI_CHANNEL_ID channel_id, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook)
{
    registered_new_byte_hook = rx_new_byte_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSwitch_RegisterTxHook(SCI_CHANNEL_ID channel_id, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook)
{
    registered_get_next_byte_hook = tx_get_next_byte_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSciChannelSwitch_NotityTxDataReady(SCI_CHANNEL_ID channel_id)
{
    U8 i = 0;
    for (; i < num_registered_channels; i++)
    {
        DrvSciChannel_NotifyTxDataReady(registered_channels_data[i].registered_drvscichannelswitch_channel_hndl);
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static U8 DrvSciChannelSwitch_GetNextByte(U8 channel_id, U8* data_ptr, U8 length)
{
    // so, this is the trick. Whenever a SCI channel asks a byte (because it's notified that there are bytes available),
    // we check if he's already received the latest byte. If so, we fetch several new ones for him
  
    U8 num_bytes_read = 0;    
    U8 i = 0;
    U8 num_bytes_fetched = 0; 
    
    if (length == 0)
        return 0;
    
    if (registered_get_next_byte_hook == NULL)
    {
        LOG_WRN("sci_channel_intercepted_tx_hook is NULL!!");
    }
    else 
    {
        Core_CriticalEnter();
        if (registered_channels_data[channel_id].read_index == write_index)
        {
            // loop as long as there are bytes available in the master, but no longer than the requested length, we can't return them anyways
            i = 0;
            do
            {
                num_bytes_fetched = registered_get_next_byte_hook(tx_circ_buffer + write_index, MIN(DRVSCICHANNELSWITCH_BUFFER_LENGTH - write_index, length));
                write_index += num_bytes_fetched;                

                if (write_index >= DRVSCICHANNELSWITCH_BUFFER_LENGTH)
                    write_index = 0;  
                
                i++;
                
            } while (i < length && num_bytes_fetched > 0);
            
        }
        
        Core_CriticalExit();
        
        // no critical section here, it may be that the write index gets pushed forward, but this is no problem for the while loop
        
        // if the read index is smaller than the write index, reading is allowed
        // if the read index is higher than the write index, he's behind and the write index already has wrapped around, so reading is allowed
        // only if the read index is equal to the write index then there's no new data
        while (registered_channels_data[channel_id].read_index != write_index
               && num_bytes_read < length)
        {
            *(data_ptr++) = *(tx_circ_buffer + registered_channels_data[channel_id].read_index);
            registered_channels_data[channel_id].read_index++;
            num_bytes_read++;      
        
            if (registered_channels_data[channel_id].read_index >= DRVSCICHANNELSWITCH_BUFFER_LENGTH)
                registered_channels_data[channel_id].read_index = 0;
        }
    }
    
    return num_bytes_read;
    
}
//------------------------------------------------------------------------------------------------//
static void DrvSciChannelSwitch_OnNewByteFromSci(U8* byte_ptr, U8 length)
{
    // any slave can call this function to push a new byte to the master
    // when 2 slaves are sending at the same time, their data will be mixed
    // a messaging interface could be implemented here, but that might be some work to get right
    // so for now, don't do it!
    if (registered_new_byte_hook != NULL)
    {
        registered_new_byte_hook(byte_ptr, length);
    }
}
//------------------------------------------------------------------------------------------------//
// we need to define as many functions as there are channels 
// because in the general SCI_TX_GET_NEXT_BYTE_HOOK, no channel id is passed...
// so the trick is to register a separate function on each connected SCI channel
// then we know which channel called the GetNextByte function
static U8 DrvSciChannelSwitch_GetNextByte0(U8* data_ptr, U8 length)
{
    return DrvSciChannelSwitch_GetNextByte(0, data_ptr, length);
}
//------------------------------------------------------------------------------------------------//
static U8 DrvSciChannelSwitch_GetNextByte1(U8* data_ptr, U8 length)
{
    return DrvSciChannelSwitch_GetNextByte(1, data_ptr, length);
}
//------------------------------------------------------------------------------------------------//
static U8 DrvSciChannelSwitch_GetNextByte2(U8* data_ptr, U8 length)
{
    return DrvSciChannelSwitch_GetNextByte(2, data_ptr, length);
}
//------------------------------------------------------------------------------------------------//
static U8 DrvSciChannelSwitch_GetNextByte3(U8* data_ptr, U8 length)
{
    return DrvSciChannelSwitch_GetNextByte(3, data_ptr, length);
}
//------------------------------------------------------------------------------------------------//
static U8 DrvSciChannelSwitch_GetNextByte4(U8* data_ptr, U8 length)
{
    return DrvSciChannelSwitch_GetNextByte(4, data_ptr, length);
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSciChannelSwitch_Init(void)
{
    LOG_DEV("DrvSciChannelSwitch_Init");
    MODULE_INIT_ONCE();
    
    // all hooks
    drvscichannelswitch_channel_hook_list.config_hook = NULL; // not supported, because the configurations of the different slaves will all be different
    drvscichannelswitch_channel_hook_list.register_rx_hook = DrvSciChannelSwitch_RegisterRxHook;
    drvscichannelswitch_channel_hook_list.register_tx_hook = DrvSciChannelSwitch_RegisterTxHook;
    drvscichannelswitch_channel_hook_list.notify_tx_data_ready = DrvSciChannelSwitch_NotityTxDataReady;
    drvscichannelswitch_channel_hook_list.config_as_mpcm_hook = NULL;
    drvscichannelswitch_channel_hook_list.set_mpcm_filter_hook = NULL;
    
    // the switch handle to register on
    drvscichannelswitch_channel_hndl.hook_list_ptr = &drvscichannelswitch_channel_hook_list;
    
    // remembering the handle & read index
    MEMSET((VPTR)registered_channels_data, 0, SIZEOF(registered_channels_data));    
    num_registered_channels = 0;
    
    // the callbacks to the master's
    registered_new_byte_hook = NULL;
    registered_get_next_byte_hook = NULL;
    
    // and the circus buffer
    tx_circ_buffer = CoreBuffer_CreateStaticU8(DRVSCICHANNELSWITCH_BUFFER_LENGTH, "DrvSciChannelSwitch");
    write_index = 0;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSciChannelSwitch_AddChannel(SCI_CHANNEL_HNDL drvscichannelswitch_channel_hndl)
{
    U8 i;
    
    LOG_DEV("DrvSciChannelSwitch_AddChannel");
    MODULE_CHECK();    
    
    for (i = 0; i < DRVSCICHANNELSWITCH_MAX_CHANNELS; i++)
    {
        if (registered_channels_data[i].registered_drvscichannelswitch_channel_hndl == drvscichannelswitch_channel_hndl)
        {
           // already registered
          return TRUE;
        }
        if (registered_channels_data[i].registered_drvscichannelswitch_channel_hndl == NULL)
        {
            registered_channels_data[i].registered_drvscichannelswitch_channel_hndl = drvscichannelswitch_channel_hndl;
            registered_channels_data[i].read_index = 0;
            
            DrvSciChannel_RegisterTxHook(drvscichannelswitch_channel_hndl, list_of_drvsciswitch_get_next_byte_hooks[i]);
            DrvSciChannel_RegisterRxHook(drvscichannelswitch_channel_hndl, DrvSciChannelSwitch_OnNewByteFromSci);
            
            num_registered_channels++;
            return TRUE;
        }
    }

    LOG_ERR("DRVSCICHANNELSWITCH_MAX_CHANNELS overrun!");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
SCI_CHANNEL_HNDL DrvSciChannelSwitch_GetHandle()
{
    MODULE_CHECK();
    return &drvscichannelswitch_channel_hndl;
}
//================================================================================================//
