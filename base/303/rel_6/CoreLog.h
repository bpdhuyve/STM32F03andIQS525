//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file implements the debug logging
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORELOG_H
#define CORELOG_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the calling module
#ifndef CORELOG_LEVEL
    #error "CORELOG_LEVEL must be defined"
#endif
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the default
#ifndef LOG_LEVEL_DEFAULT
    #define LOG_LEVEL_DEFAULT           LOG_LEVEL_NONE
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the master log level
#ifndef CORELOG_LEVEL_MASTER
    #define CORELOG_LEVEL_MASTER        LOG_LEVEL_ALL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the tag format on the output strings
#ifndef CORELOG_TAG
    #define CORELOG_TAG                 TAG_FULL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the prompt to be displayed
#ifndef PROMPT
    #define PROMPT                      "  "
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//DRIVER lib include section
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @remark  log level definitions
#define LOG_LEVEL_DEVELOP       0x01
#define LOG_LEVEL_DEBUG         0x02
#define LOG_LEVEL_WARN          0x04
#define LOG_LEVEL_ERROR         0x08
#define LOG_LEVEL_TERM          0x10

#define LOG_LEVEL_ALL           0xFF
#define LOG_LEVEL_NONE          0x00
//------------------------------------------------------------------------------------------------//
// @remark  tag level definitions
#define TAG_NONE                0x00
#define TAG_MINIMAL             0x01
#define TAG_NORMAL              0x02
#define TAG_FULL                0x03
//------------------------------------------------------------------------------------------------//
// @remark  parameter passing definitions
#define PCHAR(x)                PTYPE_CHAR, (CHAR)(x)
#define PU8(x)                  PTYPE_U8, (U8)(x)
#define PU16(x)                 PTYPE_U16, (U16)(x)
#define PU32(x)                 PTYPE_U32, (U32)(x)
#define PS8(x)                  PTYPE_S8, (S8)(x)
#define PS16(x)                 PTYPE_S16, (S16)(x)
#define PS32(x)                 PTYPE_S32, (S32)(x)

#define PCHARA(x,y)             PTYPE_CHAR_ARRAY, (U8)(y), (CHAR*)(x)
#define PU8A(x,y)               PTYPE_U8_ARRAY, (U8)(y), (U8*)(x)
#define PU16A(x,y)              PTYPE_U16_ARRAY, (U8)(y), (U16*)(x)
#define PU32A(x,y)              PTYPE_U32_ARRAY, (U8)(y), (U32*)(x)
#define PS8A(x,y)               PTYPE_S8_ARRAY, (U8)(y), (S8*)(x)
#define PS16A(x,y)              PTYPE_S16_ARRAY, (U8)(y), (S16*)(x)
#define PS32A(x,y)              PTYPE_S32_ARRAY, (U8)(y), (S32*)(x)

#define PCSTR(x)                PTYPE_CSTR, (STRING)(x)                     //const string parameter(the pointer to the string is put in the log queue, use this for const strings) use with %s
#define PDSTR(x)                PCHARA(x,CoreString_GetLength(x))           //dynamic string parameter(all the chars are put in the log queue, use this for dynamic strings) use with %c
//------------------------------------------------------------------------------------------------//
// @remark  LOG_DEV definition
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & LOG_LEVEL_DEVELOP
#if CORELOG_TAG >= TAG_NORMAL
#define LOG_DEV(str, ...)       LOG("[DEV] " str "\r" PROMPT, LOG_LEVEL_DEVELOP, ##__VA_ARGS__)
#elif CORELOG_TAG == TAG_MINIMAL
#define LOG_DEV(str, ...)       LOG("P:" str "\r" PROMPT, LOG_LEVEL_DEVELOP, ##__VA_ARGS__)
#else
#define LOG_DEV(str, ...)       LOG(str "\r" PROMPT, LOG_LEVEL_DEVELOP, ##__VA_ARGS__)
#endif
#else
#define LOG_DEV(str, ...)
#endif
//------------------------------------------------------------------------------------------------//
// @remark  LOG_DBG definition
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & LOG_LEVEL_DEBUG
#if CORELOG_TAG >= TAG_NORMAL
#define LOG_DBG(str, ...)       LOG("[DBG] " str "\r" PROMPT, LOG_LEVEL_DEBUG, ##__VA_ARGS__)
#elif CORELOG_TAG == TAG_MINIMAL
#define LOG_DBG(str, ...)       LOG("D:" str "\r" PROMPT, LOG_LEVEL_DEBUG, ##__VA_ARGS__)
#else
#define LOG_DBG(str, ...)       LOG(str "\r" PROMPT, LOG_LEVEL_DEBUG, ##__VA_ARGS__)
#endif
#else
#define LOG_DBG(str, ...)
#endif
//------------------------------------------------------------------------------------------------//
// @remark  LOG_WRN definition
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & LOG_LEVEL_WARN
#if CORELOG_TAG >= TAG_FULL
#define LOG_WRN(str, ...)       LOG("[WRN] %s" CORE_CONVERT_TO_STRING(__LINE__) " : " str "\r" PROMPT, LOG_LEVEL_WARN, PCSTR(__FILE__" - line "), ##__VA_ARGS__)
#elif CORELOG_TAG == TAG_NORMAL
#define LOG_WRN(str, ...)       LOG("[WRN] " str "\r" PROMPT, LOG_LEVEL_WARN, ##__VA_ARGS__)
#elif CORELOG_TAG == TAG_MINIMAL
#define LOG_WRN(str, ...)       LOG("W:" str "\r" PROMPT, LOG_LEVEL_WARN, ##__VA_ARGS__)
#else
#define LOG_WRN(str, ...)       LOG(str "\r" PROMPT, LOG_LEVEL_WARN, ##__VA_ARGS__)
#endif
#else
#define LOG_WRN(str, ...)
#endif
//------------------------------------------------------------------------------------------------//
// @remark  LOG_ERR definition
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & LOG_LEVEL_ERROR
#if CORELOG_TAG >= TAG_FULL
#define LOG_ERR(str, ...)       LOG("[ERR] %s" CORE_CONVERT_TO_STRING(__LINE__) " : " str  "\r" PROMPT, LOG_LEVEL_ERROR, PCSTR(__FILE__" - line "), ##__VA_ARGS__)
#elif CORELOG_TAG == TAG_NORMAL
#define LOG_ERR(str, ...)       LOG("[ERR] " str "\r" PROMPT, LOG_LEVEL_ERROR, ##__VA_ARGS__)
#elif CORELOG_TAG == TAG_MINIMAL
#define LOG_ERR(str, ...)       LOG("E:" str "\r" PROMPT, LOG_LEVEL_ERROR, ##__VA_ARGS__)
#else
#define LOG_ERR(str, ...)       LOG(str "\r" PROMPT, LOG_LEVEL_ERROR, ##__VA_ARGS__)
#endif
#else
#define LOG_ERR(str, ...)       LOG("", LOG_LEVEL_ERROR, 0)
#endif
//------------------------------------------------------------------------------------------------//
// @remark  LOG_TRM definition
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & LOG_LEVEL_TERM
#define LOG_TRM(str, ...)       LOG(str "\r" PROMPT, LOG_LEVEL_TERM, ##__VA_ARGS__)
#else
#define LOG_TRM(str, ...)
#endif
//------------------------------------------------------------------------------------------------//
// @remark  general LOG definition
#define LOG(str, lvl, ...)      CoreLog_Print(str, lvl, COUNT_PARAMS(__VA_ARGS__), ##__VA_ARGS__)

#define COUNT_PARAMS_SUB(x1,  x2,  x3,  x4,  x5,  x6,  x7,  x8,  x9,  x10, \
                         x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, \
                         x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, \
                         x31, x32, x33, x34, x35, x36, x37, x38, x39, x40, \
                         x41, x42, x43, x44, x45, x46, x47, x48, x49, x50, \
                         x51, x52, x53, x54, x55, x56, x57, x58, x59, x60, \
                         x61, x62, x63, x64, x65, x66, x67, x68, x69, x70, \
                         x71, x72, x73, x74, x75, x76, x77, x78, x79, x80, \
                         x81, x82, x83, x84, x85, x86, x87, x88, x89, x90, \
                         x91, x92, x93, x94, x95, x96, x97, x98, x99, x100, x, ...)     x
#define COUNT_PARAMS(...)   COUNT_PARAMS_SUB(__VA_ARGS__, \
                                             100, 99, 98, 97, 96, 95, 94, 93, 92, 91, \
                                              90, 89, 88, 87, 86, 85, 84, 83, 82, 81, \
                                              80, 79, 78, 77, 76, 75, 74, 73, 72, 71, \
                                              70, 69, 68, 67, 66, 65, 64, 63, 62, 61, \
                                              60, 59, 58, 57, 56, 55, 54, 53, 52, 51, \
                                              50, 49, 48, 47, 46, 45, 44, 43, 42, 41, \
                                              40, 39, 38, 37, 36, 35, 34, 33, 32, 31, \
                                              30, 29, 28, 27, 26, 25, 24, 23, 22, 21, \
                                              20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
                                              10,  9,  8,  7,  6,  5,  4,  3,  2,  1, 0)

//------------------------------------------------------------------------------------------------//
// @remark  parameter type component definitions
#define LOG_BIT_PTR         0x80
#define LOG_BIT_ARRAY       0x40
#define LOG_BIT_SIGN        0x20
#define LOG_BIT_CHAR        0x10
#define LOG_SIZE_U8         0x01
#define LOG_SIZE_U16        0x02
#define LOG_SIZE_U32        0x04
#define LOG_SIZE_MASK       0x07
//------------------------------------------------------------------------------------------------//
// @brief    Macro to log the default statement of a switch-case structure as a warning
#define LOG_WRN_DEFAULT_CASE_U16(variable_u16)           \
        LOG_WRN("%s (line %d): Invalid %s (%d)", PCSTR(__FILE__), PU16(__LINE__), PCSTR(CORE_CONVERT_TO_STRING(variable_u16)), PU16(variable_u16))
// @brief    Macro to log the default statement of a switch-case structure as a warning
#define LOG_ERR_DEFAULT_CASE_U16(variable_u16)           \
        LOG_ERR("%s (line %d): Invalid %s (%d)", PCSTR(__FILE__), PU16(__LINE__), PCSTR(CORE_CONVERT_TO_STRING(variable_u16)), PU16(variable_u16))
#define LOG_TODO(todo_string)                            \
        LOG_WRN("TODO: %s(line %d): %s", PCSTR(__FILE__), PU16(__LINE__), PCSTR(todo_string))
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @remark  parameter type enumeration
typedef enum
{
    PTYPE_CHAR          = LOG_SIZE_U8 | LOG_BIT_CHAR,
    PTYPE_U8            = LOG_SIZE_U8,
    PTYPE_U16           = LOG_SIZE_U16,
    PTYPE_U32           = LOG_SIZE_U32,
    PTYPE_S8            = LOG_SIZE_U8 | LOG_BIT_SIGN,
    PTYPE_S16           = LOG_SIZE_U16 | LOG_BIT_SIGN,
    PTYPE_S32           = LOG_SIZE_U32 | LOG_BIT_SIGN,

    PTYPE_CSTR          = LOG_SIZE_U32 | LOG_BIT_PTR,

    PTYPE_CHAR_ARRAY    = LOG_SIZE_U8 | LOG_BIT_CHAR | LOG_BIT_ARRAY,
    PTYPE_U8_ARRAY      = LOG_SIZE_U8 | LOG_BIT_ARRAY,
    PTYPE_U16_ARRAY     = LOG_SIZE_U16 | LOG_BIT_ARRAY,
    PTYPE_U32_ARRAY     = LOG_SIZE_U32 | LOG_BIT_ARRAY,
    PTYPE_S8_ARRAY      = LOG_SIZE_U8 | LOG_BIT_SIGN | LOG_BIT_ARRAY,
    PTYPE_S16_ARRAY     = LOG_SIZE_U16 | LOG_BIT_SIGN | LOG_BIT_ARRAY,
    PTYPE_S32_ARRAY     = LOG_SIZE_U32 | LOG_BIT_SIGN | LOG_BIT_ARRAY,
}
PTYPE;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreLog_Init(SCI_CHANNEL_HNDL channel_hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreLog_SwitchSciChannel(SCI_CHANNEL_HNDL channel_hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreLog_Handler(void);
//------------------------------------------------------------------------------------------------//
// @remark  dont use this function directy but use the LOG macros instead.
//          these will fill in the level automaticly and lets you disable/enable this log level
//          available log macro's
//          LOG_DEV()
//          LOG_DBG()
//          LOG_WRN()
//          LOG_ERR()
//          LOG_TRM()
//
// @ param str: the string argument can have escape characters to inset data (just like the standard printf function)
//              %d = decimal number represenation
//                   -> encapsulate U8  arguments with "PU8(...)"
//                   -> encapsulate U16 arguments with "PU16(...)"
//                   -> encapsulate U32 arguments with "PU32(...)"
//                   -> encapsulate S8  arguments with "PS8(...)"
//                   -> encapsulate S16 arguments with "PS16(...)"
//                   -> encapsulate S32 arguments with "PS32(...)"
//
//              %s = string represenation
//                   -> encapsulate STRING arguments with "PCSTR(...)"   //cstr = const string, only pointer will be queued in the logueue
//
//              %c = direct character represenation
//                   -> encapsulate STRING arguments with "PDSTR(...)"   //dstr = dynamic string, all characters will be queued in the logqueue
//                   -> encapsulate char arrys with       "PCHARA(address of first char, length of chararry)"
//
// ... help uncomplete -> tirer votre plan
//
void CoreLog_Print(STRING str, U8 level, U8 len, ...);
//------------------------------------------------------------------------------------------------//
// @remark  stays in this function as long as the data Q is not empty
void CoreLog_Flush(void);
//------------------------------------------------------------------------------------------------//
// @remark  fills direct the outputbuffer
void CoreLog_PrintDirect(CHAR* data_ptr, U8 data_len);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif // CORELOG_H

