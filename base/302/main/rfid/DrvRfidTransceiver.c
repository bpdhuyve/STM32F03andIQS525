//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// General implementation of the RFID transceiver driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRVRFIDTRANSCEIVER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRVRFIDTRANSCEIVER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               DRVRFIDTRANSCEIVER_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "rfid\DrvRfidTransceiver.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
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
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRfidTransceiver_Init(void)
{
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidTransceiver_SetProtocol(RFID_TRANSCEIVER_HNDL transceiver_hndl, RFID_PROTOCOL protocol)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->set_protocol_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->set_protocol_hook(transceiver_hndl->transceiver_id, protocol);
    }
    LOG_WRN("TRANSCEIVER set protocol function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidTransceiver_SetCrc(RFID_TRANSCEIVER_HNDL transceiver_hndl, BOOL on_off)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->set_crc_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->set_crc_hook(transceiver_hndl->transceiver_id, on_off);
    }
    LOG_WRN("TRANSCEIVER set crc function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidTransceiver_CalculateCrc(RFID_TRANSCEIVER_HNDL transceiver_hndl, U8 *data, U8 data_length, U8 *crc_buffer)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->calculate_crc_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->calculate_crc_hook(transceiver_hndl->transceiver_id, data, data_length, crc_buffer);
    }
    LOG_WRN("TRANSCEIVER set crc function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidTransceiver_SetRfField(RFID_TRANSCEIVER_HNDL transceiver_hndl, BOOL on_off)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->set_rf_field_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->set_rf_field_hook(transceiver_hndl->transceiver_id, on_off);
    }
    LOG_WRN("TRANSCEIVER set rf field function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	Writing in and reading from the FIFO buffer and send/receive via RF to/from the PICC
*       eventually ends up in NxpPn512Transceive(); a local function in DrvRfidTransceiverNxpPn512.c
*/        
RFID_ERROR DrvRfidTransceiver_Transceive(RFID_TRANSCEIVER_HNDL transceiver_hndl, U8* tx_buffer_ptr, U8 tx_buffer_length, U8* rx_buffer_ptr, U8* rx_buffer_length)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->transceive_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->transceive_hook(transceiver_hndl->transceiver_id, tx_buffer_ptr, tx_buffer_length, rx_buffer_ptr, rx_buffer_length);
    }
    LOG_WRN("TRANSCEIVER transceive function is NULL");
    return RFID_UNKNOWN_ERROR;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	Writing in and reading from the FIFO buffer and send/receive via RF to/from the PICC
*       Made for reading card data. Seems to need more delay or something
*       eventually ends up in NxpPn512TransceiveRx(); a local function in DrvRfidTransceiverNxpPn512.c
*/        
RFID_ERROR DrvRfidTransceiver_TransceiveRx(RFID_TRANSCEIVER_HNDL transceiver_hndl, U8* tx_buffer_ptr, U8 tx_buffer_length, U8* rx_buffer_ptr, U8* rx_buffer_length)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->transceiveRx_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->transceiveRx_hook(transceiver_hndl->transceiver_id, tx_buffer_ptr, tx_buffer_length, rx_buffer_ptr, rx_buffer_length);
    }
    LOG_WRN("TRANSCEIVER transceive function is NULL");
    return RFID_UNKNOWN_ERROR;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	Authenticate the PCD with the PICC
*       eventually ends up in DrvRfidTransceiverNxpPn512_Authenticate 
*       (originally: NxpPn512Authenticate(); a local function in DrvRfidTransceiverNxpPn512.c)
*/ 
RFID_ERROR DrvRfidTransceiver_Authenticate(RFID_TRANSCEIVER_HNDL transceiver_hndl, U8 block_address, U8* key_ptr, U8* uid_ptr, U8 uid_length)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->authenticate_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->authenticate_hook(transceiver_hndl->transceiver_id, block_address, key_ptr, uid_ptr, uid_length);
    }
    LOG_WRN("TRANSCEIVER authenticate function is NULL");
    return RFID_UNKNOWN_ERROR;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	Stops encrypted communication and go back to non-encryption when there is communication
*/ 
RFID_ERROR DrvRfidTransceiver_Stop_Encryption(RFID_TRANSCEIVER_HNDL transceiver_hndl)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->stop_encryption_hook != NULL))
    {
        transceiver_hndl->hook_list_ptr->stop_encryption_hook(transceiver_hndl->transceiver_id);
        return RFID_NO_ERROR;
    }
    LOG_WRN("TRANSCEIVER Stop_Encryption function is NULL");
    return RFID_UNKNOWN_ERROR;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	Sets a hardware timer to prevent an endless loop
*/ 
BOOL DrvRfidTransceiver_SetTimer(RFID_TRANSCEIVER_HNDL transceiver_hndl, U16 micro_seconds)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->set_timer_hook != NULL))
    {
        transceiver_hndl->hook_list_ptr->set_timer_hook(transceiver_hndl->transceiver_id, micro_seconds);
        return TRUE;
    }
    LOG_WRN("TRANSCEIVER SetTimer function is NULL");
    return FALSE;    
}
//------------------------------------------------------------------------------------------------//

/**
*@brief Eventually ends up in calling DrvRfidTransceiverNxpPn512_Iso14443aPrepareReqA
*/
BOOL DrvRfidTransceiver_Iso14443aPrepareRequestA(RFID_TRANSCEIVER_HNDL transceiver_hndl)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->iso14443a_prepare_reqa_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->iso14443a_prepare_reqa_hook(transceiver_hndl->transceiver_id);
    }
    LOG_WRN("TRANSCEIVER prepare REQA function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief Eventually ends up in calling DrvRfidTransceiverNxpPn512_Iso14443aPrepareSelect
*/
BOOL DrvRfidTransceiver_Iso14443aPrepareSelect(RFID_TRANSCEIVER_HNDL transceiver_hndl)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->iso14443a_prepare_select_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->iso14443a_prepare_select_hook(transceiver_hndl->transceiver_id);
    }
    LOG_WRN("TRANSCEIVER prepare SELECT function is NULL");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidTransceiver_Iso14443aPrepareAnticollision(RFID_TRANSCEIVER_HNDL transceiver_hndl)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->iso14443a_prepare_anticollision_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->iso14443a_prepare_anticollision_hook(transceiver_hndl->transceiver_id);
    }
    LOG_WRN("TRANSCEIVER prepare ANTICOLLISION function is NULL");
    return FALSE; 
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidTransceiver_Iso14443aExitSelect(RFID_TRANSCEIVER_HNDL transceiver_hndl)
{
    if((transceiver_hndl != NULL) && (transceiver_hndl->hook_list_ptr != NULL) && (transceiver_hndl->hook_list_ptr->iso14443a_exit_select_hook != NULL))
    {
        return transceiver_hndl->hook_list_ptr->iso14443a_exit_select_hook(transceiver_hndl->transceiver_id);
    }
    LOG_WRN("TRANSCEIVER exit SELECT function is NULL");
    return FALSE;
}
//================================================================================================//