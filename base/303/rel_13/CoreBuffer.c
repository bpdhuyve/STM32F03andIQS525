//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file helps the users to manage the buffers
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COREBUFFER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COREBUFFER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COREBUFFER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the total buffer space
#ifndef TOTAL_BUFFER_SPACE
    #error "TOTAL_BUFFER_SPACE not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SAFETY_BYTE         0x23
#define UNUSED_BYTE         0xAD
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreBufferInfo(void);
#endif

STRING Buffer_GetName(U8 buffer_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static U8           corebuffer_space[TOTAL_BUFFER_SPACE];
static U8*          corebuffer_ptr;
static U16          corebuffer_id_counter;
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
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CoreBufferInfo(void)
{
    U16 i;
    U8* buffer_ptr = corebuffer_space;
    U8* buffer_ptr2;
    U16 length;
    U16 count;
    #if (INCLUDE_INFO_STRING == 1)
    STRING      buffer_name;
    #endif

    LOG_TRM("CoreBuffer");
    for(i = 0; i < corebuffer_id_counter; i++)
    {
        length = CoreConvert_U8ArrayToU16(buffer_ptr);
        buffer_ptr += 2;
        #if (INCLUDE_INFO_STRING == 1)
        {
            buffer_name = (STRING)CoreConvert_U8ArrayToU32(buffer_ptr);
            buffer_ptr += length + 4;
        }
        #else
        {
            buffer_ptr += length;
        }
        #endif

        buffer_ptr2 = buffer_ptr-1;
        count = length;
        while((count > 0) && (*buffer_ptr2 == UNUSED_BYTE))
        {
            count--;
            buffer_ptr2--;
        }

        #if (INCLUDE_INFO_STRING == 1)
        {
            LOG_TRM("%2d - %4d/%-4d - %s", PU8(i), PU16(count), PU16(length), PCSTR(buffer_name));
        }
        #else
        {
            LOG_TRM("%2d - %4d/%-4d", PU8(i), PU16(count), PU16(length));
        }
        #endif
        buffer_ptr++;
        CoreLog_Flush();
    }
    LOG_TRM("Use: %4d/%-4d", PU16(buffer_ptr - corebuffer_space), PU16(TOTAL_BUFFER_SPACE));
}
#endif
//------------------------------------------------------------------------------------------------//
STRING Buffer_GetName(U8 buffer_id)
{
    #if (INCLUDE_INFO_STRING == 1)
        U16 i;
        U16 length;
        U8* buffer_ptr = corebuffer_space;
        for(i = 0; i < buffer_id; i++)  //loop all buffers until you get start of the buffer you want to know somthign about
        {
            length = CoreConvert_U8ArrayToU16(buffer_ptr);
            buffer_ptr += 2;    //skip the length
            buffer_ptr += 4;    //skip the stringpointer
            buffer_ptr += length ;
            buffer_ptr++;
        }

        buffer_ptr += 2; //skip the length
        return (STRING)CoreConvert_U8ArrayToU32(buffer_ptr);
    #else
        return NULL;
    #endif
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CoreBuffer_Init(void)
{
    MODULE_INIT_ONCE();

    corebuffer_id_counter = 0;
    corebuffer_ptr = corebuffer_space;
    MEMSET((VPTR)corebuffer_space, UNUSED_BYTE, TOTAL_BUFFER_SPACE);

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
U8* CoreBuffer_CreateStaticU8(U16 array_length, STRING name)
{
    U8*  return_buffer_ptr;

    //check if there is enough room for this new buffer
    #if (INCLUDE_INFO_STRING == 1)
    {
        if((corebuffer_ptr + array_length + 7) >= &corebuffer_space[TOTAL_BUFFER_SPACE])
        {
            LOG_ERR("BUFFER space issue - %s [%db > %db]",
                    PCSTR(name),
                    PU16(array_length + 7),
                    PU16(&corebuffer_space[TOTAL_BUFFER_SPACE] - corebuffer_ptr));
            return NULL;
        }
    }
    #else
    {
        if((corebuffer_ptr + array_length + 3) >= &corebuffer_space[TOTAL_BUFFER_SPACE])
        {
            LOG_ERR("BUFFER space issue - %s [%db > %db]",
                    PCSTR(name),
                    PU16(array_length + 3),
                    PU16(&corebuffer_space[TOTAL_BUFFER_SPACE] - corebuffer_ptr));
            return NULL;
        }
    }
    #endif

    CoreConvert_U16ToU8Array(array_length, corebuffer_ptr);
    corebuffer_ptr += 2;
    #if (INCLUDE_INFO_STRING == 1)
    {
        CoreConvert_U32ToU8Array((U32)name, corebuffer_ptr);
        corebuffer_ptr += 4;
    }
    #endif
    return_buffer_ptr = corebuffer_ptr;
    corebuffer_ptr += array_length;
    *corebuffer_ptr++ = SAFETY_BYTE;

    //set vars correct for next new buffer
    corebuffer_id_counter++;

    return return_buffer_ptr;
}
//------------------------------------------------------------------------------------------------//
U16* CoreBuffer_CreateStaticU16(U16 array_length, STRING name)
{
    // create U8 buffer with 1 extra byte. return first aligned address;
    return (U16*)(((U32)CoreBuffer_CreateStaticU8((array_length<<1) + 1, name) + 1) & 0xFFFFFFFE);
}
//------------------------------------------------------------------------------------------------//
U32* CoreBuffer_CreateStaticU32(U16 array_length, STRING name)
{
    // create U8 buffer with 3 extra bytes. return first aligned address;
    return (U32*)(((U32)CoreBuffer_CreateStaticU8((array_length<<2) + 3, name) + 3) & 0xFFFFFFFC);
}
//------------------------------------------------------------------------------------------------//
VPTR CoreBuffer_CreateStaticStruct(U16 struct_size, U16 array_length, STRING name)
{
    // create U8 buffer with 3 extra bytes. return first aligned address;
    return (VPTR)(((U32)CoreBuffer_CreateStaticU8((array_length * struct_size) + 3, name) + 3) & 0xFFFFFFFC);
}
//------------------------------------------------------------------------------------------------//
STRING CoreBuffer_NewString(U16 string_length, STRING name)
{
    STRING temp = (STRING)CoreBuffer_CreateStaticU8(string_length + 1, name); //add one for 0-terminator
    temp[0] = 0;
    temp[string_length] = 0;
    return temp;
}
//------------------------------------------------------------------------------------------------//
void CoreBuffer_SafetyChecker(void)//checks all the safe bits, if a safetybyte is gone throw exception
{
    U16 i;
    U8* buffer_ptr = corebuffer_space;
    U16 length;

    for(i = 0; i < corebuffer_id_counter; i++)
    {
        length = CoreConvert_U8ArrayToU16(buffer_ptr);
        #if (INCLUDE_INFO_STRING == 1)
        {
            buffer_ptr += length + 6;
        }
        #else
        {
            buffer_ptr += length + 2;
        }
        #endif
        if(*buffer_ptr != SAFETY_BYTE)
        {
            #if (INCLUDE_INFO_STRING == 1)
                LOG_ERR("Buffer overflow - %s", PCSTR(Buffer_GetName(i)));
            #else
                LOG_ERR("Buffer overflow - %d", PU16(i));
            #endif
        }
        buffer_ptr++;
    }
}
//------------------------------------------------------------------------------------------------//
void CoreBuffer_Info(void)
{
    CoreTerm_RegisterCommand("CoreBufferInfo", "CORE buffer info", 0, Command_CoreBufferInfo, FALSE);
}
//================================================================================================//
