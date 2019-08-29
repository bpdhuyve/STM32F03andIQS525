//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file implements a string composer
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STDSTRINGCOMPOSER_C
//================================================================================================//


//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDSTRINGCOMPOSER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STDSTRINGCOMPOSER_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include <stdarg.h>

#include "StdStringComposer.h"
//================================================================================================//


//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
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
    STRING          source_string;
    STRING          target_string;
    
    U16             target_space;
    
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
static DECODE_ACTION StdStringComposer_Decode(void);
static void StdStringComposer_ConvertParaDataToString(S16 base);
static void StdStringComposer_Padding(void);
static S16 StdStringComposer_GetNum(STRING* string_ptr);
static void StdStringComposer_CopyToTarget(STRING source_string, U8 len);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static ACTIVE_LOG_OBJECT            stringcomposer_item;
static DECODE_DATA                  stringcomposer_decode;
static CHAR*                        stringcomposer_num_string_ptr;
static U8                           stringcomposer_num_string_len;
static CHAR                         stringcomposer_num_string[12];
const CHAR                          stringcomposer_digits[] = "0123456789ABCDEF";
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
static DECODE_ACTION StdStringComposer_Decode(void)
{
    *(U8*)&stringcomposer_decode.flags = 0;
    stringcomposer_decode.pad_character = ' ';
    stringcomposer_decode.pad_len = 0;
    stringcomposer_decode.trim_len = 0x7FFF;
    
    while(1)
    {
        if(*stringcomposer_item.source_string == 0)
        {
            return NO_DECODE_ACTION;
        }
        else if(CoreString_IsDigit(*stringcomposer_item.source_string))
        {
            if(stringcomposer_decode.flags.f_dot)
            {
                stringcomposer_decode.trim_len = StdStringComposer_GetNum(&stringcomposer_item.source_string);
            }
            else
            {
                if(*stringcomposer_item.source_string == '0')
                {
                    stringcomposer_decode.pad_character = '0';
                }
                stringcomposer_decode.pad_len = StdStringComposer_GetNum(&stringcomposer_item.source_string);
                stringcomposer_decode.flags.f_pad = 1;
            }
        }
        else
        {
            switch(*stringcomposer_item.source_string++)
            {
            case '%':
                stringcomposer_decode.pad_character = '%';
                return PRINT_FIX_CHAR;
                
            case '-':
                stringcomposer_decode.flags.f_left = 1;
                break;
                
            case '.':
                stringcomposer_decode.flags.f_dot = 1;
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
static void StdStringComposer_ConvertParaDataToString(S16 base)
{
    BOOL negative = FALSE;

    stringcomposer_num_string_ptr = &stringcomposer_num_string[11];
    *stringcomposer_num_string_ptr-- = 0;
    
    if((base == 10) && (stringcomposer_item.para_type & LOG_BIT_SIGN))
    {
        switch(stringcomposer_item.para_type & LOG_SIZE_MASK)
        {
        case LOG_SIZE_U8:
            if((S8)(stringcomposer_item.para_data & 0x00FF) < 0)
            {
                negative = TRUE;
                stringcomposer_item.para_data = (U32)(-(S8)stringcomposer_item.para_data);
            }
            break;
        case LOG_SIZE_U16:
            if((S16)(stringcomposer_item.para_data & 0xFFFF) < 0)
            {
                negative = TRUE;
                stringcomposer_item.para_data = (U32)(-(S16)stringcomposer_item.para_data);
            }
            break;
        case LOG_SIZE_U32:
            if((S32)(stringcomposer_item.para_data) < 0)
            {
                negative = TRUE;
                stringcomposer_item.para_data = (U32)(-(S32)stringcomposer_item.para_data);
            }
            break;
        }
    }
    
    do
    {
        *stringcomposer_num_string_ptr-- = stringcomposer_digits[(U8)(stringcomposer_item.para_data % base)];
        stringcomposer_item.para_data /= base;
    }
    while(stringcomposer_item.para_data > 0);
    
    if(negative)
    {
        *stringcomposer_num_string_ptr = '-';
    }
    else
    {
        stringcomposer_num_string_ptr++;
    }
    
    stringcomposer_num_string_len = (U8)(&stringcomposer_num_string[11] - stringcomposer_num_string_ptr);
}
//------------------------------------------------------------------------------------------------//
static void StdStringComposer_Padding(void)
{
    if(stringcomposer_decode.flags.f_pad)
    {
        while(stringcomposer_num_string_len < stringcomposer_decode.pad_len)
        {
            StdStringComposer_CopyToTarget(&stringcomposer_decode.pad_character, 1);
            stringcomposer_num_string_len++;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static S16 StdStringComposer_GetNum(STRING* string_ptr)
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
static void StdStringComposer_CopyToTarget(STRING source_string, U8 len)
{
    while((stringcomposer_item.target_space > 0) && (len > 0))
    {
        *stringcomposer_item.target_string++ = *source_string++;
        stringcomposer_item.target_space--;
        len--;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
U16 StdStringComposer(STRING target_str, U16 target_len, STRING source_str, U8 len, ...)
{
    va_list     argp;
    U8          decode_pos;
    U8          used_len = 0;
    U8*         data_ptr;
    
    MEMSET((VPTR)&stringcomposer_decode, 0, SIZEOF(DECODE_DATA));
    MEMSET((VPTR)stringcomposer_num_string, 0, SIZEOF(stringcomposer_num_string));

    va_start(argp, len);
    
    stringcomposer_item.source_string = source_str;
    stringcomposer_item.target_string = target_str;
    stringcomposer_item.target_space = target_len;
    
    while(CoreString_GetLength(stringcomposer_item.source_string))
    {
        decode_pos = CoreString_FindChar(stringcomposer_item.source_string, '%');
        if(decode_pos == 255)
        {
            decode_pos = CoreString_GetLength(stringcomposer_item.source_string);
        }
        
        if(decode_pos > 0)
        {
            StdStringComposer_CopyToTarget(stringcomposer_item.source_string, decode_pos);
            stringcomposer_item.source_string += decode_pos;
        }
        else
        {
            stringcomposer_item.source_string++;
            stringcomposer_item.decode_action = StdStringComposer_Decode();
            
            if(used_len > len)
            {
                stringcomposer_item.decode_action = NO_DECODE_ACTION;
            }
            else if((stringcomposer_item.decode_action != NO_DECODE_ACTION) &&
                    (stringcomposer_item.decode_action != PRINT_FIX_CHAR))
            {
                // fetch parameter & data
                stringcomposer_item.para_type = (PTYPE)va_arg(argp, U32);
                if(stringcomposer_item.para_type & LOG_BIT_ARRAY)
                {
                    stringcomposer_item.para_len = (U8)va_arg(argp, U32);
                    data_ptr = (U8*)va_arg(argp, U32);
                    used_len += 3;
                }
                else
                {
                    stringcomposer_item.para_len = 1;
                    stringcomposer_item.para_data = (U32)va_arg(argp, U32);
                    used_len += 2;
                }
            }
            
            switch(stringcomposer_item.decode_action)
            {
            case PRINT_VAR_CHAR:
                if(stringcomposer_item.para_type & LOG_BIT_ARRAY)
                {
                    stringcomposer_num_string_len = stringcomposer_item.para_len;
                    if(!stringcomposer_decode.flags.f_left)
                    {
                        StdStringComposer_Padding();
                    }
                    StdStringComposer_CopyToTarget((STRING)data_ptr, stringcomposer_item.para_len);
                    if(stringcomposer_decode.flags.f_left)
                    {
                        StdStringComposer_Padding();
                    }
                }
                else
                {
                    stringcomposer_decode.pad_character = (CHAR)stringcomposer_item.para_data;
            case PRINT_FIX_CHAR:
                    StdStringComposer_CopyToTarget(&stringcomposer_decode.pad_character, 1);
                }
                break;
            case PRINT_VAR_HEX:
            case PRINT_VAR_DEC:
                while(stringcomposer_item.para_len)
                {
                    if(stringcomposer_item.para_type & LOG_BIT_ARRAY)
                    {
                        switch(stringcomposer_item.para_type & LOG_SIZE_MASK)
                        {
                        case LOG_SIZE_U8:
                            stringcomposer_item.para_data = (U32)*((U8*)data_ptr);
                            data_ptr++;
                            break;
                        case LOG_SIZE_U16:
                            stringcomposer_item.para_data = (U32)*((U16*)data_ptr);
                            data_ptr += 2;
                            break;
                        case LOG_SIZE_U32:
                            stringcomposer_item.para_data = (U32)*((U32*)data_ptr);
                            data_ptr += 4;
                            break;
                        }
                    }
                    stringcomposer_item.para_len--;
                    
                    if(stringcomposer_item.decode_action == PRINT_VAR_HEX)
                    {
                        StdStringComposer_ConvertParaDataToString(16);
                    }
                    else
                    {
                        StdStringComposer_ConvertParaDataToString(10);
                    }
                    
                    // padding
                    if(!stringcomposer_decode.flags.f_left)
                    {
                        StdStringComposer_Padding();
                    }
                    StdStringComposer_CopyToTarget(stringcomposer_num_string_ptr, CoreString_GetLength(stringcomposer_num_string_ptr));
                    if(stringcomposer_decode.flags.f_left)
                    {
                        StdStringComposer_Padding();
                    }
                    if(stringcomposer_item.para_len)
                    {
                        StdStringComposer_CopyToTarget(" ", 1);
                    }
                }
                break;
            case PRINT_STR:
                if(stringcomposer_item.para_type == PTYPE_CSTR)
                {
                    stringcomposer_num_string_len = CoreString_GetLength((STRING)stringcomposer_item.para_data);
                    if(!stringcomposer_decode.flags.f_left)
                    {
                        StdStringComposer_Padding();
                    }
                    StdStringComposer_CopyToTarget((STRING)stringcomposer_item.para_data, CoreString_GetLength((STRING)stringcomposer_item.para_data));
                    if(stringcomposer_decode.flags.f_left)
                    {
                        StdStringComposer_Padding();
                    }
                }
                break;
            }
            
            stringcomposer_item.decode_action = NO_DECODE_ACTION;
        }
    }
    
    va_end(argp);
    
    return (U16)(target_len - stringcomposer_item.target_space);
}
//================================================================================================//
