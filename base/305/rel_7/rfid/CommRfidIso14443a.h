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
#ifndef COMMRFIDISO14443A_H
#define COMMRFIDISO14443A_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "rfid\DrvRfidTransceiver.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    RFID_MSSG_TYPE_ATQA         = 0,
    RFID_MSSG_TYPE_SAK          = 1,
    RFID_MSSG_TYPE_ID           = 2,
    RFID_MSSG_TYPE_TAG_PRESENT  = 3
}
RFID_MSSG_TYPE;

typedef void (*RFID_ISO14443A_CALLBACK_HOOK)(RFID_MSSG_TYPE mssg_type, U8* data_ptr, U8 data_length);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommRfidIso14443a_Init(RFID_TRANSCEIVER_HNDL rfid_transceiver_hndl);

// @remark  Background handler
void CommRfidIso14443a_Handler(void);

// @remark  Register function for hook that will be called when an iso14443a compliant tag
//          enters the RF-field of the reader
void CommRfidIso14443a_RegisterCallbackHook(RFID_ISO14443A_CALLBACK_HOOK hook);

// @remark  Get the data out of the card
BOOL CommRfidIso14443a_ReadCardData(U8 address, U8* card_data_ptr, U8* card_data_length);

// @remark  Write data to the card
BOOL CommRfidIso14443a_WriteCardData(U8 address, U8* data_ptr, U8 data_length);
//================================================================================================//



#endif /* COMMRFIDISO14443A_H */