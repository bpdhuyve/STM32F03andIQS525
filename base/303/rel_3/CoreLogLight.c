//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file implements the debug logging
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORELOG_C
//================================================================================================//


//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORELOG_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CORELOG_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the initial log level (default is ALL)
#ifndef LOG_LEVEL_INIT
    #define LOG_LEVEL_INIT              LOG_LEVEL_ALL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include <stdarg.h>
//================================================================================================//


//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define LOG_DATA_LOST_STRING                "LOG DATA LOST (%d)\r"##PROMPT
#define POWER_UP_STRING                     "\r\rPOWER UP\r"##PROMPT
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    NO_DECODE_ACTION = 0,
    PRINT_FIX_CHAR,
    PRINT_VAR_CHAR,
    PRINT_VAR_HEX,
    PRINT_VAR_DEC,
    PRINT_STR,
}
DECODE_ACTION;

typedef struct
{
    STRING          string;
    U8              data_size;

    DECODE_ACTION   decode_action;
    
    PTYPE           para_type;
    U8              para_len;
    U32             para_data;
}
ACTIVE_LOG_OBJECT;

typedef struct
{
    U8      f_left : 1;
    U8      f_pad : 1;
    U8      f_dot : 1;
}
FLAGS;

typedef struct
{
    FLAGS     flags;
    CHAR      pad_character;
    
    S16       pad_len;
    S16       trim_len;
}
DECODE_DATA;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if CORELOG_LEVEL_MASTER
static DECODE_ACTION CoreLog_Decode(void);
static void CoreLog_ConvertParaDataToString(S16 base);
static void CoreLog_Padding(void);
static S16 CoreLog_GetNum(STRING* string_ptr);
static U8 CoreLogLight_SciTxGetNextByteHook(U8* byte_ptr, U8 length);
static void CoreLogLight_PrintString(STRING string, U8 length);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
#if CORELOG_LEVEL_MASTER
MODULE_DECLARE();
static SCI_CHANNEL_HNDL             corelog_tx_channel_hndl = NULL;
static U8*                          log_tx_bytes_ptr;
static U8                           log_tx_bytes_len;
static ACTIVE_LOG_OBJECT            corelog_item;
static DECODE_DATA                  corelog_decode;
static CHAR*                        corelog_num_string_ptr;
static U8                           corelog_num_string_len;
static CHAR                         corelog_num_string[12];
const CHAR                          digits[] = "0123456789ABCDEF";


/*
static Q_HNDL                       corelog_q_hndl;
static U8                           corelog_loglevel;
static U16                          corelog_data_lost_count;*/
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if CORELOG_LEVEL_MASTER
static DECODE_ACTION CoreLog_Decode(void)
{
    *(U8*)&corelog_decode.flags = 0;
    corelog_decode.pad_character = ' ';
    corelog_decode.pad_len = 0;
    corelog_decode.trim_len = 0x7FFF;
    
    while(1)
    {
        if(*corelog_item.string == 0)
        {
            return NO_DECODE_ACTION;
        }
        else if(CoreString_IsDigit(*corelog_item.string))
        {
            if(corelog_decode.flags.f_dot)
            {
                corelog_decode.trim_len = CoreLog_GetNum(&corelog_item.string);
            }
            else
            {
                if(*corelog_item.string == '0')
                {
                    corelog_decode.pad_character = '0';
                }
                corelog_decode.pad_len = CoreLog_GetNum(&corelog_item.string);
                corelog_decode.flags.f_pad = 1;
            }
        }
        else
        {
            switch(*corelog_item.string++)
            {
            case '%':
                corelog_decode.pad_character = '%';
                return PRINT_FIX_CHAR;
                
            case '-':
                corelog_decode.flags.f_left = 1;
                break;
                
            case '.':
                corelog_decode.flags.f_dot = 1;
                break;
                
            case 'D':
            case 'd':
                return PRINT_VAR_DEC;   // signed-unsigned is determined by type
                
            case 'H':
            case 'h':
            case 'X':
            case 'x':
                return PRINT_VAR_HEX;
                
            case 'S':
            case 's':
                return PRINT_STR;
                
            case 'C':
            case 'c':
                return PRINT_VAR_CHAR;
                
            default:
                return NO_DECODE_ACTION;
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void CoreLog_ConvertParaDataToString(S16 base)
{
    BOOL negative = FALSE;

    corelog_num_string_ptr = &corelog_num_string[11];
    *corelog_num_string_ptr-- = 0;
    
    if((base == 10) && (corelog_item.para_type & LOG_BIT_SIGN))
    {
        switch(corelog_item.para_type & LOG_SIZE_MASK)
        {
        case LOG_SIZE_U8:
            if((S8)(corelog_item.para_data & 0x00FF) < 0)
            {
                negative = TRUE;
                corelog_item.para_data = (U32)(-(S8)corelog_item.para_data);
            }
            break;
        case LOG_SIZE_U16:
            if((S16)(corelog_item.para_data & 0xFFFF) < 0)
            {
                negative = TRUE;
                corelog_item.para_data = (U32)(-(S16)corelog_item.para_data);
            }
            break;
        case LOG_SIZE_U32:
            if((S32)(corelog_item.para_data) < 0)
            {
                negative = TRUE;
                corelog_item.para_data = (U32)(-(S32)corelog_item.para_data);
            }
            break;
        }
    }
    
    do
    {
        *corelog_num_string_ptr-- = digits[(U8)(corelog_item.para_data % base)];
        corelog_item.para_data /= base;
    }
    while(corelog_item.para_data > 0);
    
    if(negative)
    {
        *corelog_num_string_ptr = '-';
    }
    else
    {
        corelog_num_string_ptr++;
    }
    
    corelog_num_string_len = (U8)(&corelog_num_string[11] - corelog_num_string_ptr);
}
//------------------------------------------------------------------------------------------------//
static void CoreLog_Padding(void)
{
    if(corelog_decode.flags.f_pad)
    {
        while(corelog_num_string_len < corelog_decode.pad_len)
        {
            CoreLogLight_PrintString(&corelog_decode.pad_character, 1);
            corelog_num_string_len++;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static S16 CoreLog_GetNum(STRING* string_ptr)
{
    S16   num = 0;
    
    while(CoreString_IsDigit(**string_ptr))
    {
        num *= 10;
        num += **string_ptr - '0';
        (*string_ptr)++;
    }
    
    return num;
}
//------------------------------------------------------------------------------------------------//
static U8 CoreLogLight_SciTxGetNextByteHook(U8* byte_ptr, U8 length)
{
    if(log_tx_bytes_len)
    {
        *byte_ptr = *log_tx_bytes_ptr;
        log_tx_bytes_ptr++;
        log_tx_bytes_len--;
        return 1;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
static void CoreLogLight_PrintString(STRING string, U8 length)
{
    log_tx_bytes_ptr = (U8*)string;
    log_tx_bytes_len = length;
    
    DrvSciChannel_NotifyTxDataReady(corelog_tx_channel_hndl);
    
    while(log_tx_bytes_len)
    {}
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CoreLog_Init(SCI_CHANNEL_HNDL channel_hndl)
{
#if CORELOG_LEVEL_MASTER
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)&corelog_decode, 0, SIZEOF(DECODE_DATA));
    MEMSET((VPTR)corelog_num_string, 0, SIZEOF(corelog_num_string));
    corelog_item.string = (STRING)&corelog_num_string[11];
    //corelog_loglevel = LOG_LEVEL_INIT;
    //corelog_data_lost_count = 0;
    CoreLog_SwitchSciChannel(channel_hndl);
    
    MODULE_INIT_DONE();
#endif
}
//------------------------------------------------------------------------------------------------//
void CoreLog_SwitchSciChannel(SCI_CHANNEL_HNDL channel_hndl)
{
#if CORELOG_LEVEL_MASTER
    DrvSciChannel_RegisterTxHook(corelog_tx_channel_hndl, NULL);
    corelog_tx_channel_hndl = channel_hndl;
    DrvSciChannel_RegisterTxHook(corelog_tx_channel_hndl, CoreLogLight_SciTxGetNextByteHook);
    
    CoreLog_Print(POWER_UP_STRING, LOG_LEVEL_TERM, 0);
#endif
}
//------------------------------------------------------------------------------------------------//
void CoreLog_Handler(void)
{
    // nothing to do
}
//------------------------------------------------------------------------------------------------//
void CoreLog_Print(STRING str, U8 level, U8 len, ...)
{
#if CORELOG_LEVEL_MASTER
    va_list     argp;
    U8          decode_pos;
    U8          used_len = 0;
    U8*         data_ptr;
#endif
    BOOL        goto_error;
    
    if(level & LOG_LEVEL_ERROR)
    {
        goto_error = Core_OnErrorReport();
    }
    
#if CORELOG_LEVEL_MASTER
    if((corelog_tx_channel_hndl != NULL) && ((LOG_LEVEL_INIT & level) > 0))
    {
        va_start(argp, len);
        
        corelog_item.string = str;
        
        while(CoreString_GetLength(corelog_item.string))
        {
            decode_pos = CoreString_FindChar(corelog_item.string, '%');
            if(decode_pos == 255)
            {
                decode_pos = CoreString_GetLength(corelog_item.string);
            }
            
            if(decode_pos > 0)
            {
                CoreLogLight_PrintString(corelog_item.string, decode_pos);
                corelog_item.string += decode_pos;
            }
            else
            {
                corelog_item.string++;
                corelog_item.decode_action = CoreLog_Decode();
                
                if(used_len > len)
                {
                    corelog_item.decode_action = NO_DECODE_ACTION;
                }
                else if((corelog_item.decode_action != NO_DECODE_ACTION) &&
                        (corelog_item.decode_action != PRINT_FIX_CHAR))
                {
                    // fetch parameter & data
                    corelog_item.para_type = (PTYPE)va_arg(argp, U8);
                    if(corelog_item.para_type & LOG_BIT_ARRAY)
                    {
                        corelog_item.para_len = (U8)va_arg(argp, U8);
                        data_ptr = (U8*)va_arg(argp, U32);
                        used_len += 3;
                    }
                    else
                    {
                        corelog_item.para_len = 1;
                        switch(corelog_item.para_type & LOG_SIZE_MASK)
                        {
                        case LOG_SIZE_U8:
                            corelog_item.para_data = (U32)va_arg(argp, U8);
                            break;
                        case LOG_SIZE_U16:
                            corelog_item.para_data = (U32)va_arg(argp, U16);
                            break;
                        case LOG_SIZE_U32:
                            corelog_item.para_data = (U32)va_arg(argp, U32);
                            break;
                        }
                        used_len += 2;
                    }
                }
                
                switch(corelog_item.decode_action)
                {
                case PRINT_VAR_CHAR:
                    if(corelog_item.para_type & LOG_BIT_ARRAY)
                    {
                        corelog_num_string_len = corelog_item.para_len;
                        if(!corelog_decode.flags.f_left)
                        {
                            CoreLog_Padding();
                        }
                        CoreLogLight_PrintString((STRING)data_ptr, corelog_item.para_len);
                        if(corelog_decode.flags.f_left)
                        {
                            CoreLog_Padding();
                        }
                    }
                    else
                    {
                        corelog_decode.pad_character = (CHAR)corelog_item.para_data;
                case PRINT_FIX_CHAR:
                        CoreLogLight_PrintString(&corelog_decode.pad_character, 1);
                    }
                    break;
                case PRINT_VAR_HEX:
                case PRINT_VAR_DEC:
                    while(corelog_item.para_len)
                    {
                        if(corelog_item.para_type & LOG_BIT_ARRAY)
                        {
                            switch(corelog_item.para_type & LOG_SIZE_MASK)
                            {
                            case LOG_SIZE_U8:
                                corelog_item.para_data = (U32)*((U8*)data_ptr);
                                data_ptr++;
                                break;
                            case LOG_SIZE_U16:
                                corelog_item.para_data = (U32)*((U16*)data_ptr);
                                data_ptr += 2;
                                break;
                            case LOG_SIZE_U32:
                                corelog_item.para_data = (U32)*((U32*)data_ptr);
                                data_ptr += 4;
                                break;
                            }
                        }
                        corelog_item.para_len--;
                        
                        if(corelog_item.decode_action == PRINT_VAR_HEX)
                        {
                            CoreLog_ConvertParaDataToString(16);
                        }
                        else
                        {
                            CoreLog_ConvertParaDataToString(10);
                        }
                        
                        // padding
                        if(!corelog_decode.flags.f_left)
                        {
                            CoreLog_Padding();
                        }
                        CoreLogLight_PrintString(corelog_num_string_ptr, CoreString_GetLength(corelog_num_string_ptr));
                        if(corelog_decode.flags.f_left)
                        {
                            CoreLog_Padding();
                        }
                        if(corelog_item.para_len)
                        {
                            CoreLogLight_PrintString(" ", 1);
                        }
                    }
                    break;
                case PRINT_STR:
                    if(corelog_item.para_type == PTYPE_CSTR)
                    {
                        corelog_num_string_len = CoreString_GetLength((STRING)corelog_item.para_data);
                        if(!corelog_decode.flags.f_left)
                        {
                            CoreLog_Padding();
                        }
                        CoreLogLight_PrintString((STRING)corelog_item.para_data, CoreString_GetLength((STRING)corelog_item.para_data));
                        if(corelog_decode.flags.f_left)
                        {
                            CoreLog_Padding();
                        }
                    }
                    break;
                }
                
                corelog_item.decode_action = NO_DECODE_ACTION;
            }
        }
        
        va_end(argp);
    }
#endif
    
    if((level & LOG_LEVEL_ERROR) && goto_error)
    {
        Core_OnErrorHandler();
    }
}
//------------------------------------------------------------------------------------------------//
// @remark  stays in this function as long as the data Q is not empty
void CoreLog_Flush(void)
{
    // nothing to do
}
//================================================================================================//
