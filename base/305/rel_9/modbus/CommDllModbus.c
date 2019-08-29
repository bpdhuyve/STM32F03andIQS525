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
// @brief  Defines the time in 탎, before this time there must be an answer to a request ohterwhise you get timeout
#ifndef T_TIMEOUT
    #error "T_TIMEOUT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the device is master
#ifndef DEVICE_IS_MASTER
    #error "DEVICE_IS_MASTER not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max length of one Modbus ADU (Application Data Unit)
#ifndef STDDLL_FRAME_LENGTH
    #error "STDDLL_FRAME_LENGTH not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the CRC is done by calculation (save memory) or by CRC table (save time)
#ifndef STDDLL_CRC_TABLE_CALC
    #error "STDDLL_CRC_TABLE_CALC not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//COMM lib include section
#include "modbus\CommDllModbus.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
static U8 CommDllModbus_TxGetNextByte(U8* byte_ptr, U8 length);
static void CommDllModbus_RxNewByte(U8* byte_ptr, U8 length);
static void CommDllModbus_Timeout(VPTR data_ptr);
static void CommDllModbus_1_5Task(VPTR data_ptr);
static void CommDllModbus_3_5Task(VPTR data_ptr);
static U16 CRCGeneration(U8* value_ptr, U8 length);
static void StdDllCheckError(void);
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   enumeration typedef for the MODBUS data link layer <em>state</em>
typedef enum
{
    DLL_MODBUS_STATE_RX,
    DLL_MODBUS_STATE_WAIT_BEFORE_TX,
    DLL_MODBUS_STATE_TX
}
DLL_MODBUS_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SCI_CHANNEL_HNDL                 commdllmodbus_sci_channel;
static DLLMODBUS_RECV_FRAME_HOOK        commdllmodbus_frame_hook         = NULL;
#if(DEVICE_IS_MASTER == 1)
static EVENT_CALLBACK                   commdllmodbus_timeout_hook       = NULL;
#endif
static EVENT_CALLBACK				    commdllmodbus_txcomplete_hook    = NULL;
static DLLMODBUS_ERROR_HANDLING_HOOK	commdllmodbus_error_handeling_hook      = NULL;

static U8                               commdllmodbus_length;
static U8                               commdllmodbus_frame_bytes[STDDLL_FRAME_LENGTH];

static BOOL                             commdllmodbus_sci_tx_pipe_active;
static BOOL                             commdllmodbus_new_data_ready;

static U8                               commdllmodbus_enc_state;
static U8                               commdllmodbus_dec_state;

static DLL_MODBUS_STATE                 commdllmodbus_state;
static BOOL                             commdllmodbus_tx_allowed; //flag which controls the timing issue
                                                                 //between RX and TX frames
static TASK_HNDL                        commdllmodbus_timeout_task;

static TASK_HNDL                        commdllmodbus_1_5_task;
static TASK_HNDL                        commdllmodbus_3_5_task;

static U8                               commdllmodbus_frame_nok;
static DLLMODBUS_ERROR						commdllmodbus_error;

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
static U8 CommDllModbus_TxGetNextByte(U8* byte_ptr, U8 length)
{
    static U8* output_byte_ptr;
    static U8  output_length;

    if(commdllmodbus_state != DLL_MODBUS_STATE_TX)
    {
        LOG_WRN("Tried to TX while not in TX mode!");
        commdllmodbus_error = DLLMODBUS_ERR_TX_WHILE_NOT_IN_TX_MODE;
        return 0;
    }

    switch(commdllmodbus_enc_state)
    {
    case 0://start of frame
        output_byte_ptr = commdllmodbus_frame_bytes;
        output_length = 0;
        *byte_ptr = *output_byte_ptr;
        output_length++;
        output_byte_ptr++;
        #if(DEVICE_IS_MASTER == 0)
			CoreTask_Stop(commdllmodbus_timeout_task);
        #endif
        commdllmodbus_enc_state = 1;
        break;
    case 1:
        *byte_ptr = *output_byte_ptr;
        output_length++;
        output_byte_ptr++;
        if(output_length >= commdllmodbus_length)
        {
            commdllmodbus_enc_state = 2;
        }
        break;
    case 2:
        commdllmodbus_enc_state = 0;
        commdllmodbus_sci_tx_pipe_active = FALSE;
        commdllmodbus_state = DLL_MODBUS_STATE_RX;
        #if(DEVICE_IS_MASTER == 1)
			CoreTask_Stop(commdllmodbus_timeout_task);
			CoreTask_Start(commdllmodbus_timeout_task);
        #endif
        
        if(commdllmodbus_txcomplete_hook != NULL)
        {
        	commdllmodbus_txcomplete_hook();
        }
        
        return 0;
    default:
        LOG_ERR("Unknown state");
        return 0;
    }
    return 1;
}
//------------------------------------------------------------------------------------------------//
// @brief   function for adding a byte to a Receive buffer array using the frame decoding mechanism.
// @param   byte:             the byte that should be decoded
static void CommDllModbus_RxNewByte(U8* byte_ptr, U8 length)
{
    static U16 input_length;
    static U8* input_data_ptr;
    
    LOG_DEV("RX %02h", PU8A(byte_ptr, length));

    if(commdllmodbus_state != DLL_MODBUS_STATE_RX)
    {
        LOG_WRN("RX char while not in RX mode");
        commdllmodbus_error = DLLMODBUS_ERR_RX_WHILE_NOT_IN_RX_MODE;
        return;
    }

    if(commdllmodbus_new_data_ready)
    {
        LOG_WRN("DLL rx mssg LOST!");
        commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_LOST;
        return;
    }
    if(!commdllmodbus_sci_tx_pipe_active)
    {
        while(length)
        {
            //Frame decoding mechanism
            switch(commdllmodbus_dec_state)
            {
            case 0://start of frame: receive slave address
                commdllmodbus_frame_nok = 0;
    
                input_length = 0;
                commdllmodbus_dec_state = 1;
                input_data_ptr = commdllmodbus_frame_bytes;
                #if(DEVICE_IS_MASTER == 1)
                    CoreTask_Stop(commdllmodbus_timeout_task);
                #endif
                input_length++;
                *input_data_ptr = *byte_ptr;
                input_data_ptr++;
                CoreTask_Start(commdllmodbus_1_5_task);
                CoreTask_Start(commdllmodbus_3_5_task);
                break;
            case 1://data frame: receive data
                if(input_length <= STDDLL_FRAME_LENGTH)
                {
                    CoreTask_Stop(commdllmodbus_1_5_task);
                    CoreTask_Stop(commdllmodbus_3_5_task);
                    if(commdllmodbus_frame_nok == 0)
                    {
                        input_length++;
                        *input_data_ptr = *byte_ptr;
                        input_data_ptr++;
                        commdllmodbus_length = input_length;
                    }
                    CoreTask_Start(commdllmodbus_1_5_task);
                    CoreTask_Start(commdllmodbus_3_5_task);
                }
                else
                {
                    LOG_WRN("DLL error frame length too long");
                    commdllmodbus_dec_state = 0;
                    commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_TOO_LONG;
                }
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
        commdllmodbus_error = DLLMODBUS_ERR_RX_WHILE_TX_BUSY;
    }
    return;
}
//------------------------------------------------------------------------------------------------//
// @brief   Function to start the timeout communication.
// @param   data_ptr :           not used here.
static void CommDllModbus_Timeout(VPTR data_ptr)
{
	CoreTask_Stop(commdllmodbus_timeout_task);
    commdllmodbus_state = DLL_MODBUS_STATE_RX;

    //oproepen van timeout functie
    #if(DEVICE_IS_MASTER == 1)
        commdllmodbus_tx_allowed = TRUE;
        if(commdllmodbus_timeout_hook != NULL)
        {
            commdllmodbus_timeout_hook();
        }
    #else
        commdllmodbus_error = DLLMODBUS_ERR_INTERFRAME_TIMEOUT_REPLY_NOT_SENT;
    #endif
}
//------------------------------------------------------------------------------------------------//
static void CommDllModbus_1_5Task(VPTR data_ptr)
{
    LOG_DEV("1.5TO");
	CoreTask_Stop(commdllmodbus_1_5_task);
    commdllmodbus_frame_nok++;
}
//------------------------------------------------------------------------------------------------//
static void CommDllModbus_3_5Task(VPTR data_ptr)
{
	CoreTask_Stop(commdllmodbus_3_5_task);
    if(commdllmodbus_frame_nok == 1)
    {
        commdllmodbus_frame_nok--;

        commdllmodbus_new_data_ready = TRUE;
        #if(DEVICE_IS_MASTER == 0)
			CoreTask_Stop(commdllmodbus_timeout_task);
			CoreTask_Start(commdllmodbus_timeout_task);
        #endif
        commdllmodbus_dec_state = 0;
    }
    else
    {
        commdllmodbus_dec_state = 0;
        LOG_DEV("The message contains silent intervals >1.5 char");
        commdllmodbus_error = DLLMODBUS_ERR_INFRAME_SILENT_INTERVALS;
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
static void StdDllCheckError(void)
{
    if((commdllmodbus_error != DLLMODBUS_ERR_NO_ERROR) &&  (commdllmodbus_error_handeling_hook != NULL))
    {
        commdllmodbus_error_handeling_hook(commdllmodbus_error);
    }
    commdllmodbus_error = DLLMODBUS_ERR_NO_ERROR;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllModbus_Init(SCI_CHANNEL_HNDL channel_hndl, SCI_SPEED sci_speed)
{
    U32  time_1_5;
    U32  time_3_5;
    SCI_CONFIG_STRUCT   sci_cfg_config_struct;

    MODULE_INIT_ONCE();
    
    commdllmodbus_sci_channel = channel_hndl;
    
    sci_cfg_config_struct.speed = sci_speed;
    sci_cfg_config_struct.parity = SCI_PARITY_NONE;
    sci_cfg_config_struct.stopbit = SCI_STOPBIT_1;
    sci_cfg_config_struct.data_length = SCI_DATA_LENGTH_8_BITS;
    
    DrvSciChannel_Config(commdllmodbus_sci_channel, &sci_cfg_config_struct);
    
    DrvSciChannel_RegisterRxHook(commdllmodbus_sci_channel, CommDllModbus_RxNewByte);
    DrvSciChannel_RegisterTxHook(commdllmodbus_sci_channel, CommDllModbus_TxGetNextByte);

    commdllmodbus_sci_tx_pipe_active = FALSE;
    commdllmodbus_new_data_ready = FALSE;

    commdllmodbus_frame_nok = 0;

    commdllmodbus_enc_state = 0;
    commdllmodbus_dec_state = 0;

    commdllmodbus_state = DLL_MODBUS_STATE_RX;
    #if(DEVICE_IS_MASTER == 1)
        commdllmodbus_tx_allowed = TRUE; //set on TRUE for masters, slaves should be FALSE
    #else
        commdllmodbus_tx_allowed = FALSE;
    #endif
        
    // interchar delay = 11 bit / sci_speed (11 bit = 1 start + 8 data + even parity + 1 stop)
    switch(sci_speed)
    {
    case SCI_SPEED_9600_BPS:
        // interchar delay = 1146탎
        time_1_5 = 2001;    // 1146 * 1.5 = 1719 - if timer tick is 1000탎, timeout will be between ~1001탎 and ~1999탎 which is possibly not enough, therefore use 2001 --> (~2001 - ~2999탎)
        time_3_5 = 4011;    // 1146 * 3.5 = 4011 - if timer tick is 1000탎, timeout will be between ~4001탎 and ~4999탎, which is OK
        break;
    case SCI_SPEED_19200_BPS:   // and higher
    default:
        // interchar delay = 573탎
        time_1_5 = 1001;    // 573 * 1.5 = 860   - if timer tick is 1000탎, timeout will be between ~1탎 and ~999탎, therefor use 1001 --> (~1001 - ~1999탎)
        time_3_5 = 2005;    // 573 * 3.5 = 2006  - if timer tick is 1000탎, timeout will be between ~2001탎 and ~2999탎, which is OK
        break;
    }
    commdllmodbus_1_5_task = CoreTask_RegisterTask(time_1_5, CommDllModbus_1_5Task, NULL, 255, "Modbus_1_5"); //1.5 char time
    commdllmodbus_3_5_task = CoreTask_RegisterTask(time_3_5, CommDllModbus_3_5Task, NULL, 255, "Modbus_3_5"); //3.5 char time
    commdllmodbus_timeout_task = CoreTask_RegisterTask(T_TIMEOUT, CommDllModbus_Timeout, NULL, 255, "ModbusTimeOut");
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommDllModbus_RegisterFrameHook(DLLMODBUS_RECV_FRAME_HOOK frame_hook)
{
    commdllmodbus_frame_hook = frame_hook;
}
//------------------------------------------------------------------------------------------------//
void CommDllModbus_RegisterTimeoutHook(EVENT_CALLBACK timeout_hook)
{
#if(DEVICE_IS_MASTER == 1)
    commdllmodbus_timeout_hook = timeout_hook;
#endif
}
//------------------------------------------------------------------------------------------------//
void CommDllModbus_RegisterTxCompleteHook(EVENT_CALLBACK tx_complete_hook)
{
    commdllmodbus_txcomplete_hook = tx_complete_hook;
}
//------------------------------------------------------------------------------------------------//
void CommDllModbus_RegisterErrorHandlingHook(DLLMODBUS_ERROR_HANDLING_HOOK error_handeling_hook)
{
    commdllmodbus_error_handeling_hook = error_handeling_hook;
}
//------------------------------------------------------------------------------------------------//
void CommDllModbus_Handler(void)
{
    U16  crc_calculation;

    StdDllCheckError();
    switch(commdllmodbus_state)
    {
    case DLL_MODBUS_STATE_RX:
        if(commdllmodbus_new_data_ready)
        {
            //handle the required delay time between RX and TX frames
            //this should/can be different for masters and slaves

            commdllmodbus_tx_allowed = TRUE;

            LOG_DEV("PHY to DLL: %02h", PU8A(commdllmodbus_frame_bytes, commdllmodbus_length));

            if(commdllmodbus_length < 4)
            {
                LOG_DBG("Frame too short.");
                commdllmodbus_new_data_ready = FALSE;
                commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_TOO_SHORT;
                break;
            }
            //CRC check
            crc_calculation = CRCGeneration(&commdllmodbus_frame_bytes[0], (commdllmodbus_length - 2));

            if(((U8)(crc_calculation & 0x00FF)) != commdllmodbus_frame_bytes[commdllmodbus_length - 2])
            {
                LOG_DBG("CRC NOK");
                commdllmodbus_new_data_ready = FALSE;
                commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_CRC_ERROR_1;
                break;
            }
            else if(((U8)(crc_calculation >> 8)) != commdllmodbus_frame_bytes[commdllmodbus_length - 1])
            {
                LOG_DBG("CRC NOK");
                commdllmodbus_new_data_ready = FALSE;
                commdllmodbus_error = DLLMODBUS_ERR_FRAME_RX_CRC_ERROR_2;
                break;
            }

            if(commdllmodbus_frame_hook != NULL)
            {
                LOG_DEV("DLL to NL: %02h", PU8A(commdllmodbus_frame_bytes, commdllmodbus_length-2));
                commdllmodbus_frame_hook(&commdllmodbus_frame_bytes[0], commdllmodbus_length - 2);
            }
            else
            {
                LOG_WRN("DLL rx hook is NULL!");
                commdllmodbus_error = DLLMODBUS_ERR_NO_FRAME_HANDLER;
            }
            commdllmodbus_new_data_ready = FALSE;
        }
        break;
    case DLL_MODBUS_STATE_WAIT_BEFORE_TX:
        if(commdllmodbus_tx_allowed)
        {
	        LOG_DEV("DLL to PHY: %02h", PU8A(commdllmodbus_frame_bytes, commdllmodbus_length));
	        
            commdllmodbus_state = DLL_MODBUS_STATE_TX;
            commdllmodbus_sci_tx_pipe_active = TRUE;
            DrvSciChannel_NotifyTxDataReady(commdllmodbus_sci_channel);
            commdllmodbus_tx_allowed = FALSE;
        }
        break;
    case DLL_MODBUS_STATE_TX:
        break;
    default:
        LOG_ERR("DLL_MODBUS state error!");
        commdllmodbus_sci_tx_pipe_active = FALSE;
        commdllmodbus_new_data_ready = FALSE;
        commdllmodbus_enc_state = 0;
        commdllmodbus_dec_state = 0;
        commdllmodbus_state = DLL_MODBUS_STATE_RX;
        commdllmodbus_error = DLLMODBUS_ERR_UNKNOWN_STATE;
        break;
    }
    StdDllCheckError();
}
//------------------------------------------------------------------------------------------------//
void CommDllModbus_SendFrame(U8* data_ptr, U8 length)
{
    U16 crc_calculation;
    U8  crc_low;
    U8  crc_high;

    StdDllCheckError();
    if((commdllmodbus_state == DLL_MODBUS_STATE_RX) && (commdllmodbus_tx_allowed == TRUE))
    {
        if((length + 2) <= STDDLL_FRAME_LENGTH) // +2 for CRC
        {
            commdllmodbus_state = DLL_MODBUS_STATE_WAIT_BEFORE_TX;

            crc_calculation = CRCGeneration(data_ptr, length);
            crc_low = (U8)(crc_calculation & 0x00FF);
            crc_high = (U8)(crc_calculation >> 8);

            commdllmodbus_length = length + 2;
            MEMCPY((VPTR)commdllmodbus_frame_bytes, (VPTR)data_ptr, commdllmodbus_length); //in transmit buffer plaatsen
            commdllmodbus_frame_bytes[length] = crc_low;
            commdllmodbus_frame_bytes[length + 1] = crc_high;
        }
        else
        {
            LOG_WRN("SendFrame failed: length!");
            commdllmodbus_error = DLLMODBUS_ERR_FRAME_TX_TOO_LONG;
        }
    }
    else
    {
        LOG_WRN("SendFrame failed: tx-mode is not queued!");
        commdllmodbus_error = DLLMODBUS_ERR_FRAME_TX_LOST;
    }
    StdDllCheckError();
}
//================================================================================================//
