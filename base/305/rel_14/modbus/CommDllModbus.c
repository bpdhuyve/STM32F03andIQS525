//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the Data Link Layer of the MODBUS protocol.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MODBUS__COMMDLLMODBUS_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MODBUS__COMMDLLMODBUS_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               MODBUS__COMMDLLMODBUS_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief Defines on how many modbusses a modbus-client can be connected
#ifndef MODBUS_CHANNEL_COUNT
    #error "MODBUS_CHANNEL_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the CRC is done by calculation (save memory) or by CRC table (save time)
#ifndef STDDLL_CRC_TABLE_CALC
    #error "STDDLL_CRC_TABLE_CALC not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if background handler is registered automatically or not. If not, the application must do the call
#ifndef AUTO_REGISTER_BACKGROUND_HANDLER
    #define AUTO_REGISTER_BACKGROUND_HANDLER                1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//COMM lib include section
#include "modbus\CommDllModbus.h"

#include <stdint.h>
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel)     \
    if(((MODBUS_CHANNEL)(modbus_channel) == INVALID_MODBUS_CHANNEL) || (modbus_channel > dllmodbus_channel_count)) {LOG_ERR("[MODBUS DLL] Invalid MODBUS channel %d", PU8(modbus_channel));} \
    MODBUS_STRUCT* modbus_channel_ptr = &dllmodbus_struct[(MODBUS_CHANNEL)modbus_channel - 1];

//================================================================================================//

//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    DLL_MODBUS_STATE_RX,
    DLL_MODBUS_STATE_WAIT_BEFORE_TX,
    DLL_MODBUS_STATE_TX
}
DLLMODBUS_STATE;

typedef enum
{
    DLL_MODBUS_ENC_STATE_SOF,
    DLL_MODBUS_ENC_STATE_SENDBYTE,
    DLL_MODBUS_ENC_STATE_ALLBYTESSENT
}
DLLMODBUS_ENC_STATE;

typedef enum
{
    DLL_MODBUS_DEC_STATE_RECEIVE_SLAVE_ADDR,
    DLL_MODBUS_DEC_STATE_RECEIVE_DATA
}
DLLMODBUS_DEC_STATE;

typedef struct
{
    SCI_CHANNEL_HNDL                    commdllmodbus_sci_channel;
    MODBUS_CHANNEL                      commdllmodbus_modbus_channel;
    DLLMODBUS_STATE                     commdllmodbus_state;
    DLLMODBUS_ENC_STATE                 commdllmodbus_enc_state;
    DLLMODBUS_DEC_STATE                 commdllmodbus_dec_state;
    U8                                  commdllmodbus_frame_bytes[MODBUS_MAX_FRAME_LENGTH];
    U8                                  commdllmodbus_length;
    DLLMODBUS_ERROR                     commdllmodbus_error;      
    
    BOOL                                commdllmodbus_sci_tx_pipe_active;
    BOOL                                commdllmodbus_new_data_ready;
    
    DLLMODBUS_RECV_FRAME_HOOK           commdllmodbus_frame_hook;
    DLLMODBUS_ERROR_HANDLING_HOOK       commdllmodbus_error_handling_hook;
    EVENT_CALLBACK                      commdllmodbus_rxFeedback_hook;
    EVENT_CALLBACK                      commdllmodbus_txFeedback_hook;
    
    TASK_HNDL                           commdllmodbus_3_5_task;
}
MODBUS_STRUCT;
//================================================================================================//

//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
static U8 CommDllModbus_TxGetNextByte(MODBUS_STRUCT* modbus_channel_ptr, U8* byte_ptr, U8 length);
static U8 CommDllModbus_TxGetNextByte_Channel1(U8* byte_ptr, U8 length);
static U8 CommDllModbus_TxGetNextByte_Channel2(U8* byte_ptr, U8 length);
static void CommDllModbus_RxNewByte(MODBUS_STRUCT* modbus_channel_ptr, U8* byte_ptr, U8 length);
static void CommDllModbus_RxNewByte_Channel1(U8* byte_ptr, U8 length);
static void CommDllModbus_RxNewByte_Channel2(U8* byte_ptr, U8 length);

static void CommDllModbus_EndOfFrameDetection(VPTR data_ptr);

static U16 CRCGeneration(U8* value_ptr, U8 length);
static void CommDllModbus_CheckError(MODBUS_CHANNEL modbus_channel);
static void CommDllModbus_Handler_For_Channel(MODBUS_CHANNEL modbus_channel);
static BOOL CommDllModbus_SCIChannelIsFree(SCI_CHANNEL_HNDL* sci_channel);
static BOOL CommDllModbus_UpdateSCIChannelConfig_private(MODBUS_STRUCT* modbus_channel_ptr, SCI_CONFIG_STRUCT* sci_config);

//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static U8              dllmodbus_channel_count;
static MODBUS_STRUCT   dllmodbus_struct[MODBUS_CHANNEL_COUNT];

// Array containing the SCI tx hooks for all possible MODBUS channels
static U8 (*CommDllModbus_TxGetNextByte_table[MODBUS_CHANNEL_COUNT])(U8* byte_ptr, U8 length) =
{
    CommDllModbus_TxGetNextByte_Channel1 /* ...for first registered MODBUS channel */
    //CommDllModbus_TxGetNextByte_Channel2  /* ... second registered */
};

// Array containing the SCI rx hooks for all possible MODBUS channels
static void (*CommDllModbus_RxNewByte_table[MODBUS_CHANNEL_COUNT])(U8* byte_ptr, U8 length) =
{
    CommDllModbus_RxNewByte_Channel1     /* ...for first registered MODBUS channel */
    //CommDllModbus_RxNewByte_Channel2      /* ... second registered */
};

#if(STDDLL_CRC_TABLE_CALC == 1)
static const U8 auc_crc_hi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

static const U8 auc_crc_lo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//

// @brief   Function called by the SCI driver when he's ready to send the next character
// It fetches a new character (if there is one available) from the character array and paste the value into \p byte_ptr
// using the Frame Encoding Mechanism of Faiveley.
// @param   byte_ptr:   a pointer to the U8 variable where the character has to be filled in
// @return  TRUE if \p byte_ptr was filled in with a valid value which should be transmitted, otherwise returns FALSE
static U8 CommDllModbus_TxGetNextByte(MODBUS_STRUCT* modbus_channel_ptr, U8* byte_ptr, U8 length)
{
    static U8* output_byte_ptr;
    static U8  output_length;

    if(modbus_channel_ptr->commdllmodbus_state != DLL_MODBUS_STATE_TX)
    {
        LOG_WRN("Tried to TX while not in TX mode!");
        modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_TX_WHILE_NOT_IN_TX_MODE;
        return 0;
    }

    switch(modbus_channel_ptr->commdllmodbus_enc_state)
    {
    case DLL_MODBUS_ENC_STATE_SOF:
        output_byte_ptr = modbus_channel_ptr->commdllmodbus_frame_bytes;
            
        output_length = 0;
        *byte_ptr = *output_byte_ptr;
        output_length++;
        output_byte_ptr++;
        modbus_channel_ptr->commdllmodbus_enc_state = DLL_MODBUS_ENC_STATE_SENDBYTE;
        break;
    case DLL_MODBUS_ENC_STATE_SENDBYTE:
        *byte_ptr = *output_byte_ptr;
        output_length++;
        output_byte_ptr++;
        if(output_length >= modbus_channel_ptr->commdllmodbus_length)
        {
            modbus_channel_ptr->commdllmodbus_enc_state = DLL_MODBUS_ENC_STATE_ALLBYTESSENT;
        }
        break;
    case DLL_MODBUS_ENC_STATE_ALLBYTESSENT:
        modbus_channel_ptr->commdllmodbus_enc_state = DLL_MODBUS_ENC_STATE_SOF;
        modbus_channel_ptr->commdllmodbus_sci_tx_pipe_active = FALSE;
        modbus_channel_ptr->commdllmodbus_state = DLL_MODBUS_STATE_RX;      
        return 0;
    default:
        LOG_ERR("Unknown state");
        return 0;
    }
    
    LOG_DEV("TX %02h", PU8A(byte_ptr, length));
    
    return 1;
}
//------------------------------------------------------------------------------------------------//
static U8 CommDllModbus_TxGetNextByte_Channel1(U8* byte_ptr, U8 length)
{
    return CommDllModbus_TxGetNextByte(&dllmodbus_struct[0], byte_ptr, length);
}
//------------------------------------------------------------------------------------------------//
static U8 CommDllModbus_TxGetNextByte_Channel2(U8* byte_ptr, U8 length)
{
    return CommDllModbus_TxGetNextByte(&dllmodbus_struct[1], byte_ptr, length);
}
//------------------------------------------------------------------------------------------------//
// @brief   function for adding a byte to a Receive buffer array using the frame decoding mechanism.
// @param   byte:             the byte that should be decoded
static void CommDllModbus_RxNewByte(MODBUS_STRUCT* modbus_channel_ptr, U8* byte_ptr, U8 length)
{
    static U16 input_length;
    static U8* input_data_ptr;
    
    LOG_DEV("RX %02h", PU8A(byte_ptr, length));
    
    if(modbus_channel_ptr->commdllmodbus_state != DLL_MODBUS_STATE_RX)
    {
        LOG_WRN("RX char while not in RX mode");
        modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_RX_WHILE_NOT_IN_RX_MODE;
        return;
    }

    if(modbus_channel_ptr->commdllmodbus_new_data_ready)
    {
        LOG_WRN("DLL rx msg LOST!");
        modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_LOST;
        return;
    }
    if(!modbus_channel_ptr->commdllmodbus_sci_tx_pipe_active)
    {
        while(length)
        {
            //Frame decoding mechanism
            switch(modbus_channel_ptr->commdllmodbus_dec_state)
            {
            case DLL_MODBUS_DEC_STATE_RECEIVE_SLAVE_ADDR:
    
                input_length = 0;
                modbus_channel_ptr->commdllmodbus_dec_state = DLL_MODBUS_DEC_STATE_RECEIVE_DATA;
                input_data_ptr = modbus_channel_ptr->commdllmodbus_frame_bytes;
                input_length++;
                *input_data_ptr = *byte_ptr;
                input_data_ptr++;
                CoreTask_Start(modbus_channel_ptr->commdllmodbus_3_5_task);
                
                break;
            case DLL_MODBUS_DEC_STATE_RECEIVE_DATA: 
              
                if(input_length <= MODBUS_MAX_FRAME_LENGTH)
                {
                    input_length++;
                    *input_data_ptr = *byte_ptr;
                    input_data_ptr++;
                    modbus_channel_ptr->commdllmodbus_length = input_length;
                }
                else
                {
                    LOG_WRN("DLL error frame length too long");
                    modbus_channel_ptr->commdllmodbus_dec_state = DLL_MODBUS_DEC_STATE_RECEIVE_SLAVE_ADDR;
                    modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_TOO_LONG;
                }
                
                CoreTask_Stop(modbus_channel_ptr->commdllmodbus_3_5_task);
                CoreTask_Start(modbus_channel_ptr->commdllmodbus_3_5_task);
                
                break;
            default:
                LOG_ERR("Unknown state");
            }
            
            length--;
            byte_ptr++;
        }
    }
    else
    {
        LOG_WRN("Error: Tx Pipe is Active!!!");
        modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_RX_WHILE_TX_BUSY;
    }
    return;
}
//------------------------------------------------------------------------------------------------//
static void CommDllModbus_RxNewByte_Channel1(U8* byte_ptr, U8 length)
{
    CommDllModbus_RxNewByte(&dllmodbus_struct[0], byte_ptr, length);
}
//------------------------------------------------------------------------------------------------//
static void CommDllModbus_RxNewByte_Channel2(U8* byte_ptr, U8 length)
{
    CommDllModbus_RxNewByte(&dllmodbus_struct[1], byte_ptr, length);
}
//------------------------------------------------------------------------------------------------//
static void CommDllModbus_EndOfFrameDetection(VPTR data_ptr)
{
    LOG_DEV("3.5TO");
    
    MODBUS_STRUCT* modbus_channel_ptr = (MODBUS_STRUCT*)data_ptr;
        
    if (modbus_channel_ptr)
    {
        CoreTask_Stop(modbus_channel_ptr->commdllmodbus_3_5_task);
        modbus_channel_ptr->commdllmodbus_new_data_ready = TRUE;               
        modbus_channel_ptr->commdllmodbus_dec_state = DLL_MODBUS_DEC_STATE_RECEIVE_SLAVE_ADDR;
    }
}
//------------------------------------------------------------------------------------------------//
static U16 CRCGeneration(U8* value_ptr, U8 length)
{
#if(STDDLL_CRC_TABLE_CALC == 1)

    U8 crc_hi = 0xFF;
    U8 crc_lo = 0xFF;
    U8 index;
    U8 temp_hi;
    U8 temp_lo;

    while(length--)
    {
        index = crc_lo ^ (*(value_ptr++));
        MEMCPY(&temp_hi, (U8*)&auc_crc_hi[index], 1);
        MEMCPY(&temp_lo, (U8*)&auc_crc_lo[index], 1);
        crc_lo = crc_hi ^ temp_hi;
        crc_hi = temp_lo;
    }
    return (crc_hi << 8) | crc_lo;
#else
    static U16 modbus_crc;
    U8         crc_move;
    U8         lsb;

    modbus_crc = 0xFFFF;
    do
    {
        modbus_crc ^= (U16)(*value_ptr);

        crc_move = 0;

        do
        {
            lsb = modbus_crc & 0x0001;
            modbus_crc >>= 1;
            crc_move++;

            if(lsb == 1)
            {
                modbus_crc ^= 0xA001;
            }
        }
        while(crc_move < 8);
        length--;
        value_ptr++;
    }
    while(length > 0);

    return modbus_crc;
#endif
}
//------------------------------------------------------------------------------------------------//
static void CommDllModbus_CheckError(MODBUS_CHANNEL modbus_channel)
{
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    if((modbus_channel_ptr->commdllmodbus_error != DLLMODBUS_ERR_NO_ERROR) && (modbus_channel_ptr->commdllmodbus_error_handling_hook != NULL))
    {
        modbus_channel_ptr->commdllmodbus_error_handling_hook(modbus_channel, modbus_channel_ptr->commdllmodbus_error);
    }
    modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_NO_ERROR;
}
//------------------------------------------------------------------------------------------------//
static void CommDllModbus_Handler_For_Channel(MODBUS_CHANNEL modbus_channel)
{
    U16  crc_calculation;
    
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    CommDllModbus_CheckError(modbus_channel);
            
    switch(modbus_channel_ptr->commdllmodbus_state)
    {
    case DLL_MODBUS_STATE_RX:
        if(modbus_channel_ptr->commdllmodbus_new_data_ready)
        {
            /* comm feedback hook - frame received */
            if (modbus_channel_ptr->commdllmodbus_rxFeedback_hook != NULL)
            {
                modbus_channel_ptr->commdllmodbus_rxFeedback_hook();
            }
            
            LOG_DEV("PHY to DLL: %02h", PU8A(modbus_channel_ptr->commdllmodbus_frame_bytes, modbus_channel_ptr->commdllmodbus_length));

            if(modbus_channel_ptr->commdllmodbus_length < 4)
            {
                LOG_DBG("Frame too short.");
                modbus_channel_ptr->commdllmodbus_new_data_ready = FALSE;
                modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_TOO_SHORT;
                break;
            }
            //CRC check
            crc_calculation = CRCGeneration(&(modbus_channel_ptr->commdllmodbus_frame_bytes[0]), (modbus_channel_ptr->commdllmodbus_length - 2));

            if(((U8)(crc_calculation & 0x00FF)) != modbus_channel_ptr->commdllmodbus_frame_bytes[modbus_channel_ptr->commdllmodbus_length - 2])
            {
                LOG_WRN("CRC NOK");
                modbus_channel_ptr->commdllmodbus_new_data_ready = FALSE;
                modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_CRC_ERROR_1;
                break;
            }
            else if(((U8)(crc_calculation >> 8)) != modbus_channel_ptr->commdllmodbus_frame_bytes[modbus_channel_ptr->commdllmodbus_length - 1])
            {
                LOG_WRN("CRC NOK");
                modbus_channel_ptr->commdllmodbus_new_data_ready = FALSE;
                modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_CRC_ERROR_2;
                break;
            }

            if(modbus_channel_ptr->commdllmodbus_frame_hook != NULL)
            {
                LOG_DEV("DLL to NL: %02h", PU8A(modbus_channel_ptr->commdllmodbus_frame_bytes, modbus_channel_ptr->commdllmodbus_length-2));
                modbus_channel_ptr->commdllmodbus_frame_hook(modbus_channel, &(modbus_channel_ptr->commdllmodbus_frame_bytes[0]), modbus_channel_ptr->commdllmodbus_length-2);
            }
            else
            {
                LOG_WRN("DLL rx hook is NULL!");
                modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_NO_FRAME_HANDLER;
            }
            modbus_channel_ptr->commdllmodbus_new_data_ready = FALSE;
        }
        break;
    case DLL_MODBUS_STATE_WAIT_BEFORE_TX:

            LOG_DEV("DLL to PHY: %02h", PU8A(modbus_channel_ptr->commdllmodbus_frame_bytes, modbus_channel_ptr->commdllmodbus_length));
            
            modbus_channel_ptr->commdllmodbus_state = DLL_MODBUS_STATE_TX;
            modbus_channel_ptr->commdllmodbus_sci_tx_pipe_active = TRUE;
            DrvSciChannel_NotifyTxDataReady(modbus_channel_ptr->commdllmodbus_sci_channel);
            
            /* comm feedback hook - sending frame */
            if (modbus_channel_ptr->commdllmodbus_txFeedback_hook != NULL)
            {
                modbus_channel_ptr->commdllmodbus_txFeedback_hook();
            }

        break;
    case DLL_MODBUS_STATE_TX:
        break;
    default:
        LOG_ERR("DLL_MODBUS state error!");
        modbus_channel_ptr->commdllmodbus_sci_tx_pipe_active = FALSE;
        modbus_channel_ptr->commdllmodbus_new_data_ready = FALSE;
        modbus_channel_ptr->commdllmodbus_enc_state = DLL_MODBUS_ENC_STATE_SOF;
        modbus_channel_ptr->commdllmodbus_dec_state = DLL_MODBUS_DEC_STATE_RECEIVE_SLAVE_ADDR;
        modbus_channel_ptr->commdllmodbus_state = DLL_MODBUS_STATE_RX;
        modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_UNKNOWN_STATE;
        break;
    }
    
    CommDllModbus_CheckError(modbus_channel);
}
//------------------------------------------------------------------------------------------------//
static BOOL CommDllModbus_SCIChannelIsFree(SCI_CHANNEL_HNDL* sci_channel_ptr)
{
    for (MODBUS_STRUCT* ptr = dllmodbus_struct; ptr != &dllmodbus_struct[MODBUS_CHANNEL_COUNT]; ptr++)
    {
      if (ptr->commdllmodbus_sci_channel == *sci_channel_ptr)
      {
        return FALSE;
      }
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommDllModbus_UpdateSCIChannelConfig_private(MODBUS_STRUCT* modbus_channel_ptr, SCI_CONFIG_STRUCT* sci_config_ptr)
{    
    DrvSciChannel_Config(modbus_channel_ptr->commdllmodbus_sci_channel, sci_config_ptr);
    
    F32  interchar_delay;
    U32  time_3_5;
    // chartime = 11 bit / sci_speed (11 bit = 1 start + 8 data + even parity + 1 stop)
    if (sci_config_ptr->speed <= SCI_SPEED_19200_BPS)
    {
        interchar_delay = 11000000/sci_config_ptr->speed;
    }
    else
    {
        interchar_delay = 500;
    }
    
    time_3_5 = (U32)((interchar_delay * 35) / 10);
    time_3_5 = (time_3_5 - (time_3_5 % 1000)) + 1001;
    
    LOG_DEV("interframe delay (3.5) = %d", PU32(time_3_5));
        
    if (modbus_channel_ptr->commdllmodbus_3_5_task != NULL)
    {
        CoreTask_SetPeriod(modbus_channel_ptr->commdllmodbus_3_5_task, time_3_5); // this one also has a period of time1_5 !
    }
    else
    {
        return FALSE;
    }   
    
    return TRUE;
}

//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllModbus_Init(void)
{
    MODULE_INIT_ONCE();
    
    dllmodbus_channel_count = 0;
    MEMSET((VPTR)dllmodbus_struct, 0, sizeof(dllmodbus_struct));
    
#if AUTO_REGISTER_BACKGROUND_HANDLER
    {
        Core_RegisterModuleHandler(CommDllModbus_Handler);
    }
#endif
    
    MODULE_INIT_DONE();
    
    LOG_DBG("[MODBUS DLL] initialised");
}
//------------------------------------------------------------------------------------------------//
MODBUS_CHANNEL CommDllModbus_RegisterChannel(SCI_CHANNEL_HNDL channel_hndl, SCI_CONFIG_STRUCT* sci_config_ptr)
{
    MODULE_CHECK();
        
    if (dllmodbus_channel_count < MODBUS_CHANNEL_COUNT) 
    {
        if (CommDllModbus_SCIChannelIsFree(&channel_hndl))
        {         
            MODBUS_STRUCT* modbus_channel_ptr = &dllmodbus_struct[dllmodbus_channel_count];

            modbus_channel_ptr->commdllmodbus_sci_channel = channel_hndl;

            if (CommDllModbus_TxGetNextByte_table[dllmodbus_channel_count] != NULL && 
                CommDllModbus_RxNewByte_table[dllmodbus_channel_count] != NULL)
            {
                DrvSciChannel_RegisterTxHook(modbus_channel_ptr->commdllmodbus_sci_channel, CommDllModbus_TxGetNextByte_table[dllmodbus_channel_count]);
                DrvSciChannel_RegisterRxHook(modbus_channel_ptr->commdllmodbus_sci_channel, CommDllModbus_RxNewByte_table[dllmodbus_channel_count]);
            }
            else
            {
                LOG_WRN("[MODBUS DLL] Check CommDllModbus_TxGetNextByte_table[] and CommDllModbus_RxNewByte_table[] arrays");
            }
          
            modbus_channel_ptr->commdllmodbus_state              = DLL_MODBUS_STATE_RX;
            modbus_channel_ptr->commdllmodbus_enc_state          = DLL_MODBUS_ENC_STATE_SOF;
            modbus_channel_ptr->commdllmodbus_dec_state          = DLL_MODBUS_DEC_STATE_RECEIVE_SLAVE_ADDR;
            modbus_channel_ptr->commdllmodbus_sci_tx_pipe_active = FALSE;
            modbus_channel_ptr->commdllmodbus_new_data_ready     = FALSE;
          
            modbus_channel_ptr->commdllmodbus_3_5_task = CoreTask_RegisterTask(0, CommDllModbus_EndOfFrameDetection, modbus_channel_ptr, 255, "Modbus_3_5");
                       
            if (!CommDllModbus_UpdateSCIChannelConfig_private(modbus_channel_ptr, sci_config_ptr))
            {
                LOG_WRN("[MODBUS DLL] SCI configuration failed");
                return INVALID_MODBUS_CHANNEL;
            }
            
            //dllmodbus_channel 0 = INVALID_MODBUS_CHANNEL
            dllmodbus_channel_count++;
            modbus_channel_ptr->commdllmodbus_modbus_channel = dllmodbus_channel_count;
            
            LOG_DBG("[MODBUS DLL] channel %d registered", PU8(dllmodbus_channel_count));

            return (MODBUS_CHANNEL)(dllmodbus_channel_count);
        }
        else
        {
            LOG_WRN("[MODBUS DLL] SCI Channel already used for another MODBUS channel", PU8(dllmodbus_channel_count+1));
            return INVALID_MODBUS_CHANNEL;
        }
    }
    else
    {
      return INVALID_MODBUS_CHANNEL;
    }
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllModbus_RegisterFrameHook(MODBUS_CHANNEL modbus_channel, DLLMODBUS_RECV_FRAME_HOOK frame_hook)
{
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    modbus_channel_ptr->commdllmodbus_frame_hook = frame_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllModbus_RegisterTxFeedbackHook(MODBUS_CHANNEL modbus_channel, EVENT_CALLBACK txFeedback_hook)
{
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    modbus_channel_ptr->commdllmodbus_txFeedback_hook = txFeedback_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllModbus_RegisterRxFeedbackHook(MODBUS_CHANNEL modbus_channel, EVENT_CALLBACK rxFeedback_hook)
{
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    modbus_channel_ptr->commdllmodbus_rxFeedback_hook = rxFeedback_hook;
    return TRUE; 
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllModbus_RegisterErrorHandlingHook(MODBUS_CHANNEL modbus_channel, DLLMODBUS_ERROR_HANDLING_HOOK error_handling_hook)
{
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    modbus_channel_ptr->commdllmodbus_error_handling_hook = error_handling_hook;
    return TRUE;        
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllModbus_UpdateSCIChannelConfig(MODBUS_CHANNEL modbus_channel, SCI_CONFIG_STRUCT* sci_config)
{
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    if (!CommDllModbus_UpdateSCIChannelConfig_private(modbus_channel_ptr, sci_config))
    {
        LOG_WRN("[MODBUS DLL] SCI configuration failed");
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
void CommDllModbus_Handler(void)
{  
    //modbus_channel 0 is INVALID_MODBUS_CHANNEL
    for (MODBUS_CHANNEL modbus_channel = 1; modbus_channel <= dllmodbus_channel_count; modbus_channel++)
    {
        CommDllModbus_Handler_For_Channel(modbus_channel);
    }
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllModbus_SendFrame(MODBUS_CHANNEL modbus_channel, U8* frame_ptr, U8 length)
{
    U16 crc_calculation;
    U8  crc_low;
    U8  crc_high;
    
    VALIDATE_AND_GET_MODBUSCHANNEL_PTR(modbus_channel);
    
    CommDllModbus_CheckError(modbus_channel);
    
    if((modbus_channel_ptr->commdllmodbus_state == DLL_MODBUS_STATE_RX))
    {
        if((length + 2) <= MODBUS_MAX_FRAME_LENGTH) // +2 for CRC
        {
            modbus_channel_ptr->commdllmodbus_state = DLL_MODBUS_STATE_WAIT_BEFORE_TX;

            crc_calculation = CRCGeneration(frame_ptr, length);
            crc_low = (U8)(crc_calculation & 0x00FF);
            crc_high = (U8)(crc_calculation >> 8);

            modbus_channel_ptr->commdllmodbus_length = length + 2;
            MEMCPY((VPTR)(modbus_channel_ptr->commdllmodbus_frame_bytes), (VPTR)frame_ptr, modbus_channel_ptr->commdllmodbus_length); //in transmit buffer plaatsen
            modbus_channel_ptr->commdllmodbus_frame_bytes[length] = crc_low;
            modbus_channel_ptr->commdllmodbus_frame_bytes[length + 1] = crc_high;
            
            return TRUE;
        }
        else
        {
            LOG_WRN("SendFrame failed: length!");
            modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_FRAME_TX_TOO_LONG;
        }
    }
    else
    {
        LOG_WRN("SendFrame failed: tx-mode is not queued!");
        modbus_channel_ptr->commdllmodbus_error = DLLMODBUS_ERR_FRAME_TX_LOST;
    }
    CommDllModbus_CheckError(modbus_channel);
    
    return FALSE;
}
//================================================================================================//
