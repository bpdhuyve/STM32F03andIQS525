//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to compose JSON strings
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STDJSON_H
#define STDJSON_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include <stdio.h>
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//ExampleUse
//{
//    U16 temp_u16 = 0;
//    const STRING names[] = {"1","2","3"};
//    
//    JSON_INIT();
//    
//    JSON_OPEN()
//    {
//        JSON_ADDKEY("Key1");
//        JSON_ADDVAL(VAL_U16(temp_u16));
//        JSON_ADDKEYVAL("Key2", VAL_U16(temp_u16));
//        JSON_ADDKEY_DYNSTR(names[temp_u16]);
//        JSON_OPEN()
//        {
//            JSON_ADDKEYVAL("Key2", VAL_X16(temp_u16));
//        }
//        JSON_CLOSE()
//    }
//    JSON_CLOSE()
//}
//------------------------------------------------------------------------------------------------//
// @remark  Re-inits the JSON parameter to start a new string
#define JSON_INIT()                     StdJson_Init()

// @remark  Sets the opening bracket (if still allowed)
#define JSON_OPEN()                     if(StdJson_Open()){

// @remark  Sets the closing bracket
#define JSON_CLOSE()                    StdJson_Close();}

// @remark  Adds the combination of a key and a value to the JSON
#define JSON_ADDKEYVAL(key,val,...)     JSON_ADDKEY(key); JSON_ADDVAL(val,##__VA_ARGS__)

// @remark  Adds a key to the JSON string, including comma (if needed) and semicolomn
#define JSON_ADDKEY(key,...)            StdJson_AddComma(); JSON_ADD("\""key"\":",##__VA_ARGS__)

// @remark  Adds a dynamic string as a key
#define JSON_ADDKEY_DYNSTR(str,...)     JSON_ADDKEY("%s",str,##__VA_ARGS__)

// @remark  Adds a value to the JSON
#define JSON_ADDVAL(val,...)            JSON_ADD(val,##__VA_ARGS__)

// @remark  Helper defines to output values
#define VAL_U8(x)                       "%d", (U8)x
#define VAL_S8(x)                       "%d", (S8)x
#define VAL_U16(x)                      "%d", (U16)x
#define VAL_S16(x)                      "%d", (S16)x
#define VAL_X16(x)                      "\"0x%04X\"", (U16)x
#define VAL_U32(x)                      "%lu", (U32)x
#define VAL_S32(x)                      "%ld", (S32)x
#define VAL_X32(x)                      "\"0x%08X\"", x
#define VAL_BOOL(x)                     "%s", true_false_string[MIN(x,1)]
#define VAL_STR(x)                      "\"%s\"", x

//------------------------------------------------------------------------------------------------//
// @remark  Actual macro that adds to the JSON
#define JSON_ADD(...)                   StdJson_AddCheck(snprintf(StdJson_GetBufferPosition(), StdJson_GetBufferSpace(), ##__VA_ARGS__))
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern const STRING true_false_string[];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void StdJson_Init(void);

// @remark  Function to get the JSON buffer pointer and length
void StdJson_GetBuffer(STRING* json_string_ptr, U16* json_len_ptr);

//------------------------------------------------------------------------------------------------//
// HELPER FUNCTIONS, DO NOT USE DIRECTLY, USE MACRO'S ABOVE
BOOL StdJson_Open(void);
void StdJson_Close(void);
void StdJson_AddComma(void);
STRING StdJson_GetBufferPosition(void);
U32 StdJson_GetBufferSpace(void);
void StdJson_AddCheck(U32 add_len);
//================================================================================================//



#endif /* STDJSON_H */
