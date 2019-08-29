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
// @brief  Defines the size of the data queue
#ifndef LOG_DATA_QUEUE_SIZE
    #error "LOG_DATA_QUEUE_SIZE not defined in AppConfig"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the size of the out queue
#ifndef LOG_OUT_QUEUE_SIZE
    #error "LOG_OUT_QUEUE_SIZE not defined in AppConfig"
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

#if CORELOG_LEVEL_MASTER
//DRV lib include section
#define Q_PREFIX(postfix)                   CoreLog_##postfix
#define Q_SIZE                              LOG_OUT_QUEUE_SIZE
#include "sci\DrvSciQTxTpl.h"
#endif
//================================================================================================//


//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define LOG_DATA_LOST_STRING                "LOG DATA LOST (%d)\r" PROMPT
#define POWER_UP_STRING                     "\r\rPOWER UP\r" PROMPT
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
    U16             data_size;

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
    FLAGS   flags;
    CHAR    pad_character;

    S16     pad_len;
    S16     trim_len;
}
DECODE_DATA;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if CORELOG_LEVEL_MASTER
static void CoreLog_FillOutput(void);
static DECODE_ACTION CoreLog_Decode(void);
static BOOL CoreLog_FetchPara(void);
static BOOL CoreLog_FetchData(void);
static void CoreLog_ConvertParaDataToString(S16 base);
static BOOL CoreLog_Padding(void);
static S16 CoreLog_GetNum(STRING* string_ptr);
static void CoreLog_PrintSub(STRING str, U8 len, va_list va_arg_in);
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreLogLevel(void);
static void Command_CoreGetError(void);
#endif
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
#if CORELOG_LEVEL_MASTER
static Q_HNDL                       corelog_q_hndl;
static ACTIVE_LOG_OBJECT            corelog_item;
static DECODE_DATA                  corelog_decode;
static CHAR*                        corelog_num_string_ptr;
static U8                           corelog_num_string_len;
static CHAR                         corelog_num_string[12];
const CHAR                          digits[] = "0123456789ABCDEF";
static U8                           corelog_loglevel;
static U16                          corelog_data_lost_count;
static STRING                       corelog_errorlog_str;
static U8                           corelog_errorlog_len;
static va_list                      corelog_error_va_arg;
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
static void CoreLog_FillOutput(void)
{
    U8      write_len;
    U8      print_len;
    CHAR    space = ' ';

    // get space in any case.
    write_len = CoreLog_SciQTx_GetSpace();

    // decoderen, maar oppassen dat er genoeg plaats is in output Q
    while(*corelog_item.string != 0)
    {
        if(corelog_item.decode_action == NO_DECODE_ACTION)
        {
            // print non-coded string
            if(*corelog_item.string != '%')
            {
                write_len = CoreLog_SciQTx_GetSpace();
                if(write_len == 0)
                {
                    return; // no more space: abort
                }
                if(CoreString_CountChar(corelog_item.string, '%') > 0)
                {
                    print_len = CoreString_SearchChar(corelog_item.string, '%');
                }
                else
                {
                    print_len = CoreString_GetLength(corelog_item.string);
                }
                write_len = MIN(write_len, print_len);
                CoreLog_SciQTx_Write((VPTR)corelog_item.string, write_len);
                corelog_item.string += write_len;
            }

            // determine decode action
            if(*corelog_item.string == '%')
            {
                corelog_item.string++;
                corelog_item.decode_action = CoreLog_Decode();          // advances corelog_item.string till just after code string

                if((corelog_item.decode_action != NO_DECODE_ACTION) &&
                   (corelog_item.decode_action != PRINT_FIX_CHAR))
                {
                    if(CoreLog_FetchPara() && CoreLog_FetchData())      // fills (para_type & para_len) and para_data
                    {
                        switch(corelog_item.decode_action)
                        {
                        case PRINT_VAR_CHAR:
                            corelog_decode.pad_character = (CHAR)corelog_item.para_data;
                            break;
                        case PRINT_VAR_HEX:
                            CoreLog_ConvertParaDataToString(16);
                            break;
                        case PRINT_VAR_DEC:
                            CoreLog_ConvertParaDataToString(10);
                            break;
                        case PRINT_STR:
                            if(corelog_item.para_type != PTYPE_CSTR)
                            {
                                corelog_item.decode_action = NO_DECODE_ACTION;
                            }
                            else
                            {
                                corelog_num_string_len = CoreString_GetLength((STRING)corelog_item.para_data);
                            }
                            break;
                        }
                    }
                    else
                    {
                        corelog_item.decode_action = NO_DECODE_ACTION;
                    }
                }
            }
        }

        // determine action based on decoding
        switch(corelog_item.decode_action)
        {
        case PRINT_FIX_CHAR:
        case PRINT_VAR_CHAR:
            if(CoreLog_SciQTx_Write((VPTR)&corelog_decode.pad_character, 1) == FALSE)
            {
                return; // no more space: abort
            }
            if(corelog_item.para_len > 0)
            {
                if(CoreLog_FetchData())
                {
                    corelog_decode.pad_character = (CHAR)corelog_item.para_data;
                    continue;
                }
            }
            break;
        case PRINT_VAR_HEX:
        case PRINT_VAR_DEC:
            if(!corelog_decode.flags.f_left && (CoreLog_Padding() == FALSE))
            {
                return; // no more space: abort
            }
            while(*corelog_num_string_ptr != 0)
            {
                write_len = CoreLog_SciQTx_GetSpace();
                if(write_len == 0)
                {
                    return;  // no more space: abort
                }
                print_len = CoreString_GetLength(corelog_num_string_ptr);
                write_len = MIN(write_len, print_len);
                CoreLog_SciQTx_Write((VPTR)corelog_num_string_ptr, write_len);
                corelog_num_string_ptr += write_len;
            }
            if(corelog_decode.flags.f_left && (CoreLog_Padding() == FALSE))
            {
                return; // no more space: abort
            }
            if(corelog_item.para_len > 0)
            {
                if(CoreLog_SciQTx_Write((VPTR)&space, 1) == FALSE)
                {
                    return; // no more space: abort
                }
                if(CoreLog_FetchData())
                {
                    if(corelog_item.decode_action == PRINT_VAR_HEX)
                    {
                        CoreLog_ConvertParaDataToString(16);
                    }
                    else
                    {
                        CoreLog_ConvertParaDataToString(10);
                    }
                    continue;
                }
            }
            break;
        case PRINT_STR:
            if(!corelog_decode.flags.f_left && (CoreLog_Padding() == FALSE))
            {
                return; // no more space: abort
            }
            while((*((STRING)corelog_item.para_data) != 0) && (corelog_decode.trim_len > 0))
            {
                write_len = CoreLog_SciQTx_GetSpace();
                if(write_len == 0)
                {
                    return;  // no more space: abort
                }
                print_len = MIN(CoreString_GetLength((STRING)(corelog_item.para_data)), corelog_decode.trim_len);
                write_len = MIN(write_len, print_len);
                CoreLog_SciQTx_Write((VPTR)(corelog_item.para_data), write_len);
                corelog_item.para_data += write_len;
                corelog_decode.trim_len -= write_len;
            }
            if(corelog_decode.flags.f_left && (CoreLog_Padding() == FALSE))
            {
                return; // no more space: abort
            }
            break;
        }
        corelog_item.decode_action = NO_DECODE_ACTION;
    }

    // check if there is still something in the queue, drop the rest
    if(corelog_item.data_size > 0)
    {
        CoreQ_Drop(corelog_q_hndl, corelog_item.data_size);
        corelog_item.data_size = 0;
        corelog_item.para_len = 0;
    }
}
//------------------------------------------------------------------------------------------------//
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
static BOOL CoreLog_FetchPara(void)
{
    U16 count = (corelog_item.para_type & LOG_SIZE_MASK) * corelog_item.para_len;

    if(corelog_item.para_len > 0)
    {
        CoreQ_Drop(corelog_q_hndl, count);
        corelog_item.data_size -= count;
        corelog_item.para_len = 0;
    }
    if(corelog_item.data_size > 0)
    {
        CoreQ_Read(corelog_q_hndl, (U8*)&corelog_item.para_type, 1);
        corelog_item.data_size--;
        if(corelog_item.para_type & LOG_BIT_ARRAY)
        {
            CoreQ_Read(corelog_q_hndl, (U8*)&corelog_item.para_len, 1);
            corelog_item.data_size--;
            corelog_item.para_type &= ~LOG_BIT_ARRAY;
        }
        else
        {
            corelog_item.para_len = 1;
        }
        return TRUE;
    }
    corelog_item.para_len = 0;
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CoreLog_FetchData(void)
{
    U8  buffer[4];
    U8  count = (corelog_item.para_type & LOG_SIZE_MASK);

    if((corelog_item.data_size > 0) && (corelog_item.para_len > 0))
    {
        CoreQ_Read(corelog_q_hndl, buffer, count);
        switch(count)
        {
        case LOG_SIZE_U8:
            corelog_item.para_data = (U32)buffer[0];
            break;
        case LOG_SIZE_U16:
            corelog_item.para_data = (U32)CoreConvert_U8ArrayToU16(buffer);
            break;
        case LOG_SIZE_U32:
            corelog_item.para_data = (U32)CoreConvert_U8ArrayToU32(buffer);
            break;
        }
        corelog_item.data_size -= count;
        corelog_item.para_len--;
        return TRUE;
    }
    corelog_item.para_len = 0;
    return FALSE;
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
static BOOL CoreLog_Padding(void)
{
    if(corelog_decode.flags.f_pad)
    {
        while(corelog_num_string_len < corelog_decode.pad_len)
        {
            if(CoreLog_SciQTx_Write((VPTR)&corelog_decode.pad_character, 1) == FALSE)
            {
                return FALSE; // no more space: abort
            }
            corelog_num_string_len++;
        }
    }
    return TRUE;
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
static void CoreLog_PrintSub(STRING str, U8 len, va_list va_arg_in)
{
#if CORELOG_LEVEL_MASTER
    va_list     argp;
    U8          i;
    U8          j;
    U16         hdr_qsize;
    U16         data_qsize;
    U8          buffer[8];
    U16         start_index;
    U32         data_ptr;
    U8          add;
    BOOL        add_lost_log_data = (BOOL)(corelog_data_lost_count > 0);
    
    // first determine size needed in buffer
    argp = va_arg_in;
    i = 0;
    hdr_qsize = 5;  // header size
    data_qsize = 0; // data size
    if(len == 1)    // if no varargs are supplied, the length will be 1
    {
        len = 0;
    }
    while(i < len)
    {
        buffer[0] = va_arg(argp, U32);
        add = buffer[0] & LOG_SIZE_MASK;
        if(buffer[0] & LOG_BIT_ARRAY)
        {
            data_qsize += (va_arg(argp, U32) * add) + 2;    // 2 + n*m bytes: type + count (n) + n * m data
            i += 3;                                         // 3 input parameters
        }
        else
        {
            data_qsize += add + 1;                          // 1 + m bytes: type + m data
            i += 2;                                         // 2 input parameters
        }
        va_arg(argp, U32);     // dummy readout U32 data
    }
    va_end(argp);

    if(data_qsize >= 0xFF)
    {
        hdr_qsize += 2;
    }

    if(add_lost_log_data == TRUE)
    {
        hdr_qsize += 8;
    }

    // try to allocate qsize = data_qsize + hdr_qsize
    if(CoreQ_WriteAlloc(corelog_q_hndl, data_qsize + hdr_qsize , &start_index))
    {
        // write lost data
        if(add_lost_log_data == TRUE)
        {
            // add log data lost statement
            buffer[0] = 3;
            CoreConvert_U32ToU8Array((U32)(LOG_DATA_LOST_STRING), &buffer[1]);
            buffer[5] = (U8)PTYPE_U16;
            CoreConvert_U16ToU8Array(corelog_data_lost_count, &buffer[6]);
            CoreQ_WritePart(corelog_q_hndl, start_index, buffer, 8);
            start_index += 8;
            corelog_data_lost_count = 0;
        }

        // write header
        CoreConvert_U32ToU8Array((U32)str, &buffer[1]);
        if(data_qsize < 0xFF)
        {
            buffer[0] = (U8)data_qsize;
            CoreQ_WritePart(corelog_q_hndl, start_index, buffer, 5);
            start_index += 5;
        }
        else
        {
            buffer[0] = 0xFF;
            CoreConvert_U16ToU8Array(data_qsize, &buffer[5]);
            CoreQ_WritePart(corelog_q_hndl, start_index, buffer, 7);
            start_index += 7;
        }

        // write data
        argp = va_arg_in;
        i = 0;
        while(i < len)
        {
            buffer[0] = (PTYPE)va_arg(argp, U32);
            add = buffer[0] & LOG_SIZE_MASK;
            if(buffer[0] & LOG_BIT_ARRAY)
            {
                buffer[1] = (U8)va_arg(argp, U32);
                data_ptr = (U32)va_arg(argp, U32);
                CoreQ_WritePart(corelog_q_hndl, start_index, buffer, 2);
                start_index += 2;
                i += 3;
                for(j=0; j<buffer[1]; j++)
                {
                    switch(add)
                    {
                    case LOG_SIZE_U8:
                        buffer[2] = *((U8*)data_ptr);
                        break;
                    case LOG_SIZE_U16:
                        CoreConvert_U16ToU8Array(*((U16*)data_ptr), &buffer[2]);
                        break;
                    case LOG_SIZE_U32:
                        CoreConvert_U32ToU8Array(*((U32*)data_ptr), &buffer[2]);
                        break;
                    }
                    CoreQ_WritePart(corelog_q_hndl, start_index, &buffer[2], add);
                    start_index += add;
                    data_ptr += add;
                }
            }
            else
            {
                switch(add)
                {
                case LOG_SIZE_U8:
                    buffer[1] = (U8)va_arg(argp, U32);
                    break;
                case LOG_SIZE_U16:
                    CoreConvert_U16ToU8Array((U16)va_arg(argp, U32), &buffer[1]);
                    break;
                case LOG_SIZE_U32:
                    CoreConvert_U32ToU8Array((U32)va_arg(argp, U32), &buffer[1]);
                    break;
                }
                CoreQ_WritePart(corelog_q_hndl, start_index, buffer, add + 1);
                start_index += add + 1;
                i += 2;
            }
        }
        va_end(argp);

        // complete writing
        CoreQ_WriteDone(corelog_q_hndl, data_qsize + hdr_qsize);
    }
    else
    {
        corelog_data_lost_count++;
    }
#endif
}
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreLogLevel(void)
{
    corelog_loglevel = (U8)CoreTerm_GetArgumentAsU32(0);
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_CoreGetError(void)
{
    if(Core_GetCoreState() & CORE_STATE_ERROR)
    {
        CoreLog_PrintSub(corelog_errorlog_str, corelog_errorlog_len, corelog_error_va_arg);
    }
    else
    {
        LOG_TRM("No error");
    }
    CoreTerm_PrintAcknowledge();
}
#endif
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CoreLog_Init(SCI_CHANNEL_HNDL channel_hndl)
{
    MODULE_INIT_ONCE();

#if CORELOG_LEVEL_MASTER
    corelog_q_hndl = CoreQ_Register(LOG_DATA_QUEUE_SIZE, SIZEOF(U8), "CoreLog_DataQ");
    CoreLog_SciQTx_Create(channel_hndl);
    MEMSET((VPTR)&corelog_item, 0, SIZEOF(ACTIVE_LOG_OBJECT));
    MEMSET((VPTR)&corelog_decode, 0, SIZEOF(DECODE_DATA));
    MEMSET((VPTR)corelog_num_string, 0, SIZEOF(corelog_num_string));
    corelog_item.string = (STRING)&corelog_num_string[11];
    corelog_loglevel = LOG_LEVEL_INIT;
    corelog_data_lost_count = 0;
    CoreLog_Print(POWER_UP_STRING, LOG_LEVEL_TERM, 0);
    CoreTerm_RegisterCommand("CoreLogLevel", "CORE set log level", 1, Command_CoreLogLevel, FALSE);
    CoreTerm_RegisterCommand("CoreGetError", "CORE get error", 0, Command_CoreGetError, FALSE);
#endif

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CoreLog_SwitchSciChannel(SCI_CHANNEL_HNDL channel_hndl)
{
#if CORELOG_LEVEL_MASTER
    CoreLog_SciQTx_SwitchSciChannel(channel_hndl);
#endif
}
//------------------------------------------------------------------------------------------------//
void CoreLog_Handler(void)
{
#if CORELOG_LEVEL_MASTER
    U8  buffer[8];

    if((corelog_data_lost_count > 0) && (CoreQ_GetCount(corelog_q_hndl) == 0))
    {
        buffer[0] = 3;
        CoreConvert_U32ToU8Array((U32)(LOG_DATA_LOST_STRING), &buffer[1]);
        buffer[5] = (U8)PTYPE_U16;
        CoreConvert_U16ToU8Array(corelog_data_lost_count, &buffer[6]);
        if(CoreQ_Write(corelog_q_hndl, buffer, 8))
        {
            corelog_data_lost_count = 0;
        }
    }

    // check if log object is active
    if((*corelog_item.string == 0) && (CoreQ_GetCount(corelog_q_hndl) >= 5))
    {
        CoreQ_Read(corelog_q_hndl, buffer, 5);
        corelog_item.data_size = buffer[0];
        corelog_item.string = (STRING)CoreConvert_U8ArrayToU32(&buffer[1]);
        if(corelog_item.data_size == 0xFF)
        {
            CoreQ_Read(corelog_q_hndl, buffer, 2);
            corelog_item.data_size = CoreConvert_U8ArrayToU16(&buffer[0]);
        }
    }

    CoreLog_FillOutput();
#endif
}
//------------------------------------------------------------------------------------------------//
void CoreLog_Print(STRING str, U8 level, U8 len, ...)
{
#if CORELOG_LEVEL_MASTER
    va_list     argp;
#endif

    #if CORELOG_LEVEL_MASTER
    {
        if((corelog_loglevel & level) > 0)
        {
            va_start(argp, len);
            CoreLog_PrintSub(str, len, argp);
        }
    }
    #endif
    
    if(level & LOG_LEVEL_ERROR)
    {
        if(Core_OnErrorReport())
        {
            #if CORELOG_LEVEL_MASTER
            {
                corelog_errorlog_str = str;
                corelog_errorlog_len = len;
                va_start(corelog_error_va_arg, len);
            }
            #endif
            Core_OnErrorHandler();
        }
    }
}
//------------------------------------------------------------------------------------------------//
// @remark  stays in this function as long as the data Q is not empty
void CoreLog_Flush(void)
{
#if CORELOG_LEVEL_MASTER
    System_KickDog();
    while(CoreQ_GetCount(corelog_q_hndl) > 0)
    {
        #ifdef USE_FREERTOS
        if(((Core_GetCoreState() & (CORE_STATE_ERROR|CORE_STATE_ACTIVE)) == CORE_STATE_ACTIVE) &&
           (xTaskGetCurrentTaskHandle() != xTaskGetIdleTaskHandle()))
        {
            vTaskDelay(1);
        }
        else
        #endif
        {
            CoreLog_Handler();
        }
    }
#endif
}
//------------------------------------------------------------------------------------------------//
void CoreLog_PrintDirect(CHAR* data_ptr, U8 data_len)
{
    CoreLog_SciQTx_Write((VPTR)data_ptr, data_len);
}
//================================================================================================//
