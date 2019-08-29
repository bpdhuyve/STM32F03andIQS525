//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of ISO/IEC14443 RFID protocol
// Asumption: The specific transceiver driver should implement timeouts.
//            Multiple chips have a timer on board to do this, so that's why
//            this isn't implemented in this module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMMRFIDISO14443A_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMRFIDISO14443A_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMMRFIDISO14443A_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef LEAVE_IDLE_STATE_WITH_REQ_A
    #define LEAVE_IDLE_STATE_WITH_REQ_A         0
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
#include "CommRfidIso14443a.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define CASCADE_LEVEL_0             0x93
#define CASCADE_LEVEL_1             0x95
#define CASCADE_LEVEL_2             0x97
#define CASCADE_TAG                 0x88
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443a_RequestA(void);
static BOOL Iso14443a_CardAnticollisionLoop(U8 cascade_code, U8 number_of_valid_bits);
static BOOL Iso14443a_Select(U8 cascade_code, U8 number_of_valid_bits, U8* uid_ptr, U8* sak);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static RFID_ISO14443A_CALLBACK_HOOK     callback_hook;
static RFID_TRANSCEIVER_HNDL            transceiver_hndl;
static U8                               tx_buffer[50];
static U8                               tx_buffer_length;
static U8                               rx_buffer[50];
static U8                               rx_buffer_length;
static U8                               complete_uid[10];
static U8                               complete_uid_length;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443a_RequestA(void)
{
    // Is used to start the ISO14443A Anticollision/Activation Loop.
    // PROCESS:	[1] send REQA or WUPA command
    //			[2] receive ATQA
    //			[3] perform bit frame anticollison loop
    // -------------------------------------------------------------
    
    // [1] send REQA or WUPA command
    // Prepare the driver if for initiating the REQA / WUPA command
    DrvRfidTransceiver_Iso14443aPrepareRequestA(transceiver_hndl);
    
    // switch off CRC
    DrvRfidTransceiver_DisableCrc(transceiver_hndl);
    
    // REQA = CMD 0x26 | WUPA = CMD 0x52
#if LEAVE_IDLE_STATE_WITH_REQ_A
    tx_buffer[0] = 0x26;
#else
    tx_buffer[0] = 0x52;
#endif
    tx_buffer_length = 1;
    
    switch(DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length))
    {
    case RFID_NO_ERROR:
        if(rx_buffer_length == 2)
        {
            LOG_DBG("ATQA: %02h", PU8A(rx_buffer, rx_buffer_length));
            break;
        }
        else if(rx_buffer_length != 0)
        {
            LOG_DBG("[RFID]: Invalid ATQA length - %d", PU8(rx_buffer_length));
        }
        return FALSE;
    case RFID_COLLISION_ERROR:
        return FALSE;
    case RFID_TIMEOUT_ERROR:
        return FALSE;
    default:
        return FALSE;
    }
    
    if(callback_hook != NULL)
    {
        callback_hook(RFID_MSSG_TYPE_ATQA, rx_buffer, rx_buffer_length);
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443a_CardAnticollisionLoop(U8 cascade_code, U8 number_of_valid_bits)
{
	U8	i = 0, length = 0;
	U8	nvbytes = 0, nvbits = 0, xbits = 0;
    U8 sak;
    
    
    DrvRfidTransceiver_Iso14443aPrepareAnticollision(transceiver_hndl);
    
    // switch off CRC
    DrvRfidTransceiver_DisableCrc(transceiver_hndl);
    
    length = number_of_valid_bits >> 4;
	if((number_of_valid_bits & 0x0F) != 0x00)
	{
		length++;
		nvbytes = (number_of_valid_bits >> 4) - 2;			    // the number of known valid bytes
		xbits = number_of_valid_bits & 0x07;					// the number of known valid bits
        
		// Both are used in the UID calculation
		for(i = 0; i < xbits; i++)
		{
            // if xbits == 3 => nvbits = b111
			nvbits = nvbits << 1;
			nvbits = nvbits + 1;
		}
	}
    
    tx_buffer[0] = cascade_code;
    tx_buffer[1] = number_of_valid_bits;
    tx_buffer_length = 2;
    
    switch(DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length))
    {
    case RFID_NO_ERROR:
        if(rx_buffer_length != 5)
        {
            LOG_WRN("[RFID]: Invalid length - %d", PU8(rx_buffer_length));
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    
    LOG_DBG("[RFID]: ANTICOLLISION FRAME %02h", PU8A(rx_buffer, rx_buffer_length));
    
    switch(cascade_code)
    {
    case CASCADE_LEVEL_0:
        if(rx_buffer[0] == CASCADE_TAG)
        {
            // UID not complete
            for(i = 0; i < 3; i++)
            {
                complete_uid[i] = rx_buffer[i + 1];
            }
            complete_uid_length = 3;
            
            if((!Iso14443a_Select(CASCADE_LEVEL_0, 0x50, rx_buffer, &sak)) || ((sak & 0x04) != 0x04))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
            return Iso14443a_CardAnticollisionLoop(CASCADE_LEVEL_1, 0x20);
        }
        else
        {
            // UID complete
            for(i = 0; i < 4; i++)
            {
                complete_uid[i] = rx_buffer[i];
            }
            complete_uid_length = 4;
            
            if(!Iso14443a_Select(CASCADE_LEVEL_0, 0x50, rx_buffer, &sak))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
        }
        break;
    case CASCADE_LEVEL_1:
        if(rx_buffer[0] == CASCADE_TAG)
        {
            // UID not complete
            for(i = 0; i < 3; i++)
            {
                complete_uid[i + 3] = rx_buffer[i + 1];
            }
            complete_uid_length += 3;
            
            if((!Iso14443a_Select(CASCADE_LEVEL_1, 0x50, rx_buffer, &sak)) || ((sak & 0x04) != 0x04))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
            return Iso14443a_CardAnticollisionLoop(CASCADE_LEVEL_2, 0x20);
        }
        else
        {
            // UID complete
            for(i = 0; i < 4; i++)
            {
                complete_uid[i + 3] = rx_buffer[i];
            }
            complete_uid_length = 7;
            
            if(!Iso14443a_Select(CASCADE_LEVEL_1, 0x50, rx_buffer, &sak))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
        }
        break;
    case CASCADE_LEVEL_2:
        if(rx_buffer[0] == CASCADE_TAG)
        {
            LOG_WRN("[RFID]: Invalid CASCADE level");
            return FALSE;
        }
        else
        {
            // UID complete
            for(i = 0; i < 4; i++)
            {
                complete_uid[i + 6] = rx_buffer[i];
            }
            complete_uid_length = 10;
            
            if(!Iso14443a_Select(CASCADE_LEVEL_2, 0x50, rx_buffer, &sak))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
        }
        break;
    default:
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443a_Select(U8 cascade_code,
                             U8 number_of_valid_bits,
                             U8* uid_ptr,
                             U8* sak)
{
    // uid_ptr is used for in- and output of this function
    tx_buffer[0] = cascade_code;
    tx_buffer[1] = 0x70;
    MEMSET(&tx_buffer[2], 0x00, 5);
    // Determine tx_buffer[1], the number of valid bits
    tx_buffer_length = (U8)((number_of_valid_bits & 0xF0) >> 4);
    if((number_of_valid_bits & 0x0F) > 0)
    {
        tx_buffer_length++;
    }
    // Set UID data
	MEMCPY(&tx_buffer[2], uid_ptr, tx_buffer_length);
	tx_buffer_length += 2;
    // Add BCC
    tx_buffer[6] = uid_ptr[0] ^ uid_ptr[1] ^ uid_ptr[2] ^ uid_ptr[3];
    
    LOG_DBG("[RFID] SELECT FRAME: %02h", PU8A(tx_buffer, tx_buffer_length));
    
    DrvRfidTransceiver_Iso14443aPrepareSelect(transceiver_hndl);
    
    // CRC needs to be enabled when performing SELECT command
    DrvRfidTransceiver_EnableCrc(transceiver_hndl);
    
    switch(DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length))
    {
    case RFID_NO_ERROR:
        LOG_DBG("SAK: %02h", PU8A(rx_buffer, rx_buffer_length));
        if(rx_buffer_length != 1)
        {
            LOG_WRN("[RFID]: Invalid SAK length");
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    
    DrvRfidTransceiver_Iso14443aExitSelect(transceiver_hndl);
    
    if((rx_buffer[0] & 0x04) == 0x04)
    {
        // another cascade level is coming, except if the current cascade level
        // is equal to 0x97
        if((cascade_code == CASCADE_LEVEL_2) && (uid_ptr[0] == 0x88))
        {
            LOG_WRN("[RFID]: Protocol error");
            return FALSE;
        }
    }
    else
    {
        if(uid_ptr[0] == 0x88)
        {
            LOG_WRN("[RFID]: Protocol error");
            return FALSE;
        }
    }
    
    // Copy received SAK
    *sak = rx_buffer[0];
    
    if(callback_hook != NULL)
    {
        callback_hook(RFID_MSSG_TYPE_SAK, rx_buffer, 1);
    }
    return TRUE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommRfidIso14443a_Init(RFID_TRANSCEIVER_HNDL rfid_transceiver_hndl)
{
    MODULE_INIT_ONCE();
    
    callback_hook    = NULL;
    transceiver_hndl = rfid_transceiver_hndl;
    
    tx_buffer_length = 0;
    rx_buffer_length = 0;
    
    MEMSET(tx_buffer, 0, SIZEOF(tx_buffer));
    MEMSET(rx_buffer, 0, SIZEOF(rx_buffer));
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommRfidIso14443a_Handler(void)
{
    BOOL tag_found = FALSE;
    
    // Is used to detect ISO14443A conform tags
    // PROCESS:	[1] load ISO14443A protocol
    //			[2] turn on RF driver
    //			[3] do a complete anticollision sequence
    //			[4] turn off RF driver
    // --------------------------------------------------
    
    // [1] load ISO14443A protocol
    DrvRfidTransceiver_SetProtocol(transceiver_hndl, RFID_PROTOCOL_ISO_14443A);
    
    // [2] turn on RF driver
    DrvRfidTransceiver_SetRfField(transceiver_hndl, TRUE);
    
    // [3] do a complete anticollision sequence
    if(Iso14443a_RequestA())
    {
        if(Iso14443a_CardAnticollisionLoop(CASCADE_LEVEL_0, 0x20))
        {
            tag_found = TRUE;
            LOG_DBG("[RFID]: Complete UID: %02h", PU8A(complete_uid, complete_uid_length));
            if(callback_hook != NULL)
            {
                callback_hook(RFID_MSSG_TYPE_ID, complete_uid, complete_uid_length);
            }
        }
    }
    
    if(callback_hook != NULL)
    {
        callback_hook(RFID_MSSG_TYPE_TAG_PRESENT, (U8*)(&tag_found), 1);
    }
    
    // [4] turn off RF driver
    DrvRfidTransceiver_SetRfField(transceiver_hndl, FALSE);
}
//------------------------------------------------------------------------------------------------//
void CommRfidIso14443a_RegisterCallbackHook(RFID_ISO14443A_CALLBACK_HOOK tag_found_hook)
{
    callback_hook = tag_found_hook;
}
//------------------------------------------------------------------------------------------------//
BOOL CommRfidIso14443a_ReadCardData(U8 address, U8* card_data_ptr, U8* card_data_length)
{
    tx_buffer[0] = 0x30;
    tx_buffer[1] = address;
    tx_buffer_length = 2;
    
    DrvRfidTransceiver_EnableCrc(transceiver_hndl);
    
    switch(DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length))
    {
    case RFID_NO_ERROR:
        MEMCPY(card_data_ptr, rx_buffer, rx_buffer_length);
        *card_data_length = rx_buffer_length;
        LOG_DBG("[RFID]: Page %d: %02h", PU8(address), PU8A(card_data_ptr, *card_data_length));
        break;
    case RFID_COLLISION_ERROR:
        return FALSE;
    case RFID_TIMEOUT_ERROR:
        return FALSE;
    default:
        LOG_WRN("[RFID]: Unknown ERROR");
        return FALSE;
    }
    
    return TRUE;
}
//================================================================================================//