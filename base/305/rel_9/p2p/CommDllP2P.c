//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the DLL communication layer between the BASE and IO processor
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define P2P__COMMDLLP2P_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMDLLP2P_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMMDLLP2P_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMDLLP2P_SPEED
    #define COMMDLLP2P_SPEED                                SCI_SPEED_57600_BPS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMDLLP2P_PARITY
    #define COMMDLLP2P_PARITY                               SCI_PARITY_NONE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMDLLP2P_DATA_LENGTH
    #define COMMDLLP2P_DATA_LENGTH                          SCI_DATA_LENGTH_8_BITS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMDLLP2P_STOPBIT
    #define COMMDLLP2P_STOPBIT                              SCI_STOPBIT_1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMDLLP2P_RX_BUFFER_LENGTH
    #define COMMDLLP2P_RX_BUFFER_LENGTH                     130
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMDLLP2P_RX_QUEUE_SIZE
    #define COMMDLLP2P_RX_QUEUE_SIZE                        256
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMDLLP2P_TX_QUEUE_SIZE
    #define COMMDLLP2P_TX_QUEUE_SIZE                        256
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// STD
#include "crc\StdCrc.h"

// COM
#include "CommDllP2P.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define ESCAPE_CHARACTER                0xAA
#define START_OF_FRAME_CHARACTER        0x55
#define VALUE_0XAA_CHARACTER            0x01
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
	DLL_RX_START_OF_FRAME	    = 0,
	DLL_RX_LENGTH				= 1,
	DLL_RX_DATA					= 2,
	DLL_RX_CRC				    = 3
}
DLL_RX_STATE;

typedef enum
{
	DLL_TX_WAIT_BEFORE_TX		= 0,
    DLL_TX_START_OF_FRAME       = 1,
    DLL_TX_Q_DATA               = 2
}
DLL_TX_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void CommDllP2P_RxNewByte(U8* byte_ptr, U8 length);
static U8   CommDllP2P_TxGetNextByte(U8* byte_ptr, U8 length);
static BOOL CommDllP2P_Decode(U8* byte_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SCI_CHANNEL_HNDL                 commdllp2p_sci_channel_hndl;

static COMMDLLP2P_RECV_FRAME_HOOK       commdllp2p_rx_frame_hook;
// Added one byte to commdllp2p_rx_buffer_bytes to avoid data alignment mismatch. commdllp2p_rx_buffer_bytes[0] is never used
#pragma data_alignment=2
static U8                               commdllp2p_rx_buffer_bytes[COMMDLLP2P_RX_BUFFER_LENGTH + 2];

static Q_HNDL                           commdllp2p_rx_queue;
static DLL_RX_STATE                     commdllp2p_rx_state;

static Q_HNDL                           commdllp2p_tx_queue;
static DLL_TX_STATE                     commdllp2p_tx_state;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CommDllP2P_RxNewByte(U8* byte_ptr, U8 length)
{
    LOG_DEV("P2P DLL RX: 0x%02h", PU8(*byte_ptr));
    
    if(CoreQ_Write(commdllp2p_rx_queue, byte_ptr, length) == FALSE)
    {
        LOG_WRN("Writing RX queue failed");
    }
}
//------------------------------------------------------------------------------------------------//
// @brief   Function called by the SCI driver when he's ready to send the next character
// It fetches a new character (if there is one available) from the character array and paste the value into \p byte_ptr
// @param   byte_ptr:   a pointer to the U8 variable where the character has to be filled in
// @return  TRUE if \p byte_ptr was filled in with a valid value which should be transmitted, otherwise returns FALSE
static U8 CommDllP2P_TxGetNextByte(U8* byte_ptr, U8 length)
{
    static U8   output_byte;
    static U16  output_length;
    static BOOL escape_active = FALSE;
    
    switch(commdllp2p_tx_state)
    {
    case DLL_TX_WAIT_BEFORE_TX:
        // first item in Q is the length of the data of the frame without itself and the crc
        if(CoreQ_Peek(commdllp2p_tx_queue, (VPTR)&output_byte, 1))
        {
            output_length       = (U16)output_byte + 3; // +1 for the data length byte + 2 crc bytes
            *byte_ptr           = ESCAPE_CHARACTER;     // send first byte of START-OF-FRAME 0xAA
            commdllp2p_tx_state = DLL_TX_START_OF_FRAME;
        }
        else
        {
            return 0;
        }
        break;
        
    case DLL_TX_START_OF_FRAME:
        *byte_ptr               = START_OF_FRAME_CHARACTER;
        commdllp2p_tx_state     = DLL_TX_Q_DATA;
        break;
        
    case DLL_TX_Q_DATA:
        if(escape_active == TRUE)
        {
            *byte_ptr           = VALUE_0XAA_CHARACTER;
            escape_active       = FALSE;
            output_length--;
        }
        else if(CoreQ_Read(commdllp2p_tx_queue, &output_byte, 1))
        {
            *byte_ptr           = output_byte;
            if(output_byte == ESCAPE_CHARACTER)
            {
                escape_active   = TRUE;
            }
            else
            {
                output_length--;
            }
        }
        else
        {
            CoreQ_DropAll(commdllp2p_tx_queue);
            commdllp2p_tx_state = DLL_TX_WAIT_BEFORE_TX;
            LOG_WRN("Unexpected frame termination");
            return 0;
        }
        
        // check if data is complete
        if(output_length == 0)
        {
            LOG_DBG("P2P TX complete");
            commdllp2p_tx_state = DLL_TX_WAIT_BEFORE_TX;
        }
        break;
        
    default:
        LOG_ERR("Unknown TX state");
        return 0;
    }
    
    LOG_DEV("TX DLL: 0x%02h", PU8(*byte_ptr));
    return 1;
}
//------------------------------------------------------------------------------------------------//
// function returns TRUE if the value of byte_ptr is decoded and can be used in the state machine
static BOOL CommDllP2P_Decode(U8* byte_ptr)
{
    static BOOL escape_active = FALSE;
    
    if(escape_active == TRUE)
    {
        escape_active = FALSE;
        
        switch(*byte_ptr)
        {
        case VALUE_0XAA_CHARACTER:
            *byte_ptr = 0xAA;
            return TRUE;
            
        case START_OF_FRAME_CHARACTER:
            commdllp2p_rx_state = DLL_RX_LENGTH;
            return FALSE;
            
        default:
            commdllp2p_rx_state = DLL_RX_START_OF_FRAME;
            LOG_WRN("Invalid ESC character %02x", PU8(*byte_ptr));
            return FALSE;
        }
    }
    else if(*byte_ptr == ESCAPE_CHARACTER)
    {
        escape_active = TRUE;
        return FALSE;
    }
    return TRUE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllP2P_Init(SCI_CHANNEL_HNDL channel_hndl)
{
    SCI_CONFIG_STRUCT sci_cfg_struct = {COMMDLLP2P_SPEED, COMMDLLP2P_PARITY, COMMDLLP2P_STOPBIT, COMMDLLP2P_DATA_LENGTH};
    
    MODULE_INIT_ONCE();
    
    if(channel_hndl == NULL)
    {
        LOG_ERR("channel_hndl is NULL");
        return;
    }
    
    commdllp2p_sci_channel_hndl = channel_hndl;
    commdllp2p_rx_frame_hook    = NULL;
    DrvSciChannel_Config(commdllp2p_sci_channel_hndl, &sci_cfg_struct);
    
    commdllp2p_rx_queue = CoreQ_Register(COMMDLLP2P_RX_QUEUE_SIZE, 1, "P2P DLL Rx");
    commdllp2p_tx_queue = CoreQ_Register(COMMDLLP2P_TX_QUEUE_SIZE, 1, "P2P DLL Tx");
    
    DrvSciChannel_RegisterRxHook(commdllp2p_sci_channel_hndl, CommDllP2P_RxNewByte);
    DrvSciChannel_RegisterTxHook(commdllp2p_sci_channel_hndl, CommDllP2P_TxGetNextByte);
    
    commdllp2p_rx_state = DLL_RX_START_OF_FRAME;
    commdllp2p_tx_state = DLL_TX_WAIT_BEFORE_TX;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommDllP2P_Handler(void)
{
    static U8  receive_data_length;
    static U8* input_data_ptr;
    static U16 input_length;
    static U16 input_crc;
    U8 data;
    
    // Handle received messages
    while(CoreQ_Read(commdllp2p_rx_queue, &data, 1))
    {
        if(CommDllP2P_Decode(&data))
        {
            switch(commdllp2p_rx_state)
            {
            case DLL_RX_START_OF_FRAME:
                break;
                
            case DLL_RX_LENGTH:
#if (COMMDLLP2P_RX_BUFFER_LENGTH) < 255
                if(data > COMMDLLP2P_RX_BUFFER_LENGTH)
                {
                    commdllp2p_rx_state = DLL_RX_START_OF_FRAME;
                    LOG_WRN("RX length invalid");
                    break;
                }
#endif
                
                input_data_ptr      = &commdllp2p_rx_buffer_bytes[1];
                receive_data_length = data;
                *input_data_ptr     = data;
                input_length        = 1;
                input_data_ptr++;
                
                commdllp2p_rx_state = DLL_RX_DATA;
                break;
                
            case DLL_RX_DATA:
                *input_data_ptr = data;
                input_data_ptr++;
                input_length++;

                if(--receive_data_length == 0)
                {
                    commdllp2p_rx_state = DLL_RX_CRC;
                    receive_data_length = 2; // 2 CRC bytes should be received
                    input_crc = 0;
                }
                break;
                
            case DLL_RX_CRC:
                input_crc |= (data << ((2 - receive_data_length) * 8));
                
                if(--receive_data_length == 0)
                {
                    // complete message received
                    LOG_DBG("RX crc: %04h", PU16(input_crc));
                    if(StdCrcGenerateCrc16IBM(&commdllp2p_rx_buffer_bytes[1], input_length) == input_crc)
                    {
                        LOG_DEV("CRC correct, writing message to Q");
                        if(commdllp2p_rx_frame_hook != NULL)
                        {
                            commdllp2p_rx_frame_hook(&commdllp2p_rx_buffer_bytes[2], commdllp2p_rx_buffer_bytes[1]);
                        }
                    }
                    else
                    {
                        LOG_WRN("CRC error %04h - %04h", PU16(StdCrcGenerateCrc16IBM(&commdllp2p_rx_buffer_bytes[1], input_length)), PU16(input_crc));
                    }
                    
                    commdllp2p_rx_state = DLL_RX_START_OF_FRAME;
                    return;
                }
                break;
                
            default:
                LOG_ERR("Unknown RX state");
                return;
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
void CommDllP2P_RegisterFrameReceivedHook(COMMDLLP2P_RECV_FRAME_HOOK frame_hook)
{
    commdllp2p_rx_frame_hook = frame_hook;
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllP2P_SendFrame(U8* frame_ptr, U8 frame_length)
{
    U16 start_index;
    U16 crc;
    
    // check on inputs
#if (COMMDLLP2P_TX_QUEUE_SIZE) < 258
    if((frame_length + 3) > COMMDLLP2P_TX_QUEUE_SIZE)
    {
        LOG_WRN("DLL TX msg too long, frame not send");
        return FALSE;
    }
#endif
    if(frame_length == 0)
    {
        LOG_WRN("Illegal frame length");
        return FALSE;
    }
    if(frame_ptr == NULL)
    {
        LOG_WRN("Illegal frame pointer");
        return FALSE;
    }
    
    // We will add the crc to the message in the Q here
    // The frame_length is also part of data to be sent, so it needs to be included in the CRC calculation
    // The format of the bytestream that will be added to the Q looks like:
    // [ L |    L data bytes    | CRC (16-bit ]
    crc = StdCrcGenerateCrc16IBM(&frame_length, 1);
    crc = StdCrcGenerateCrc16IBMPart(frame_ptr, frame_length, crc);
    LOG_DBG("TX crc: %04h", PU16(crc));
    
    if(CoreQ_WriteAlloc(commdllp2p_tx_queue, frame_length + 3, &start_index)) // 3 = 1 of frame_length + 2 of CRC
    {
        // if allocation succeeded, write parts
        CoreQ_WritePart(commdllp2p_tx_queue, start_index, &frame_length, 1);                    // length
        CoreQ_WritePart(commdllp2p_tx_queue, start_index + 1, frame_ptr, frame_length);         // data
        CoreQ_WritePart(commdllp2p_tx_queue, start_index + frame_length + 1, (VPTR)&crc, 2);    // crc
        
        // conclude writing to queue
        CoreQ_WriteDone(commdllp2p_tx_queue, frame_length + 3);
        
        DrvSciChannel_NotifyTxDataReady(commdllp2p_sci_channel_hndl);
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//
