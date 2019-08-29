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