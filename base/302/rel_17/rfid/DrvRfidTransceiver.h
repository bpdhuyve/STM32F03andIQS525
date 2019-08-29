//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// General implementation of the RFID transceiver driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRVRFIDTRANSCEIVER_H
#define DRVRFIDTRANSCEIVER_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DrvRfidTransceiver_EnableRfField(t_hndl)            DrvRfidTransceiver_SetRfField(t_hndl, TRUE)
#define DrvRfidTransceiver_DisableRfField(t_hndl)           DrvRfidTransceiver_SetRfField(t_hndl, FALSE)
#define DrvRfidTransceiver_EnableCrc(t_hndl)                DrvRfidTransceiver_SetCrc(t_hndl, TRUE)
#define DrvRfidTransceiver_DisableCrc(t_hndl)               DrvRfidTransceiver_SetCrc(t_hndl, FALSE)
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                                  RFID_TRANSCEIVER_ID;

typedef enum
{
    RFID_PROTOCOL_ISO_14443A                = 0
}
RFID_PROTOCOL;

typedef enum
{
    RFID_NO_ERROR                           = 0,
    RFID_COLLISION_ERROR                    = 1,
    RFID_TIMEOUT_ERROR                      = 2,
    
    RFID_UNKNOWN_ERROR                      = 255
}
RFID_ERROR;

typedef BOOL (*RFID_TRANSCEIVER_SET_PROTOCOL)(RFID_TRANSCEIVER_ID transceiver_id, RFID_PROTOCOL protocol);

typedef BOOL (*RFID_TRANSCEIVER_SET_CRC)(RFID_TRANSCEIVER_ID transceiver_id, BOOL on_off);

typedef BOOL (*RFID_TRANSCEIVER_SET_RF_FIELD)(RFID_TRANSCEIVER_ID transceiver_id, BOOL on_off);

typedef RFID_ERROR (*RFID_TRANSCEIVER_TRANSCEIVE)(RFID_TRANSCEIVER_ID transceiver_id, U8* tx_buffer_ptr, U8 tx_buffer_length, U8* rx_buffer_ptr, U8* rx_buffer_length);

typedef BOOL (*RFID_TRANSCEIVER_ISO14443A_PREPARE_REQA)(RFID_TRANSCEIVER_ID transceiver_id);

typedef BOOL (*RFID_TRANSCEIVER_ISO14443A_PREPARE_SELECT)(RFID_TRANSCEIVER_ID transceiver_id);

typedef BOOL (*RFID_TRANSCEIVER_ISO14443A_EXIT_SELECT)(RFID_TRANSCEIVER_ID transceiver_id);

typedef BOOL (*RFID_TRANSCEIVER_ISO14443A_PREPARE_ANTICOLLISION)(RFID_TRANSCEIVER_ID transceiver_id);

typedef struct
{
    RFID_TRANSCEIVER_SET_PROTOCOL                       set_protocol_hook;
    RFID_TRANSCEIVER_SET_CRC                            set_crc_hook;             
    RFID_TRANSCEIVER_SET_RF_FIELD                       set_rf_field_hook;
    RFID_TRANSCEIVER_TRANSCEIVE                         transceive_hook;
    RFID_TRANSCEIVER_ISO14443A_PREPARE_REQA             iso14443a_prepare_reqa_hook;
    RFID_TRANSCEIVER_ISO14443A_PREPARE_SELECT           iso14443a_prepare_select_hook;
    RFID_TRANSCEIVER_ISO14443A_EXIT_SELECT              iso14443a_exit_select_hook;
    RFID_TRANSCEIVER_ISO14443A_PREPARE_ANTICOLLISION    iso14443a_prepare_anticollision_hook;
}
RFID_TRANSCEIVER_HOOK_LIST;

typedef struct
{
    RFID_TRANSCEIVER_HOOK_LIST*             hook_list_ptr;
    RFID_TRANSCEIVER_ID                     transceiver_id;
}
RFID_TRANSCEIVER_STRUCT;

typedef RFID_TRANSCEIVER_STRUCT*            RFID_TRANSCEIVER_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvRfidTransceiver_Init(void);

BOOL DrvRfidTransceiver_SetProtocol(RFID_TRANSCEIVER_HNDL transceiver_hndl, RFID_PROTOCOL protocol);

BOOL DrvRfidTransceiver_SetCrc(RFID_TRANSCEIVER_HNDL transceiver_hndl, BOOL on_off);

BOOL DrvRfidTransceiver_SetRfField(RFID_TRANSCEIVER_HNDL transceiver_hndl, BOOL on_off);

RFID_ERROR DrvRfidTransceiver_Transceive(RFID_TRANSCEIVER_HNDL transceiver_hndl, U8* tx_buffer_ptr, U8 tx_buffer_length, U8* rx_buffer_ptr, U8* rx_buffer_length);

//------------------------------------------------------------------------------------------------//
// ISO SPECIFIC FUNCTIONS
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidTransceiver_Iso14443aPrepareRequestA(RFID_TRANSCEIVER_HNDL transceiver_hndl);

BOOL DrvRfidTransceiver_Iso14443aPrepareSelect(RFID_TRANSCEIVER_HNDL transceiver_hndl);

BOOL DrvRfidTransceiver_Iso14443aExitSelect(RFID_TRANSCEIVER_HNDL transceiver_hndl);

BOOL DrvRfidTransceiver_Iso14443aPrepareAnticollision(RFID_TRANSCEIVER_HNDL transceiver_hndl);
//================================================================================================//



#endif /* DRVRFIDTRANSCEIVER_H */