//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// template header file for generating and handling a value masks
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
// @remark  Template usage:
//
// #define VALUEMASK_PREFIX(postfix)               Module_Function##postfix
// #define VALUEMASK_SIZE                          (100)
// #define VALUEMASK_MAXVALUE                      (10)
// #include "bitmask\StdValueMaskTpl.h"
//
// --> Functions become Module_Function_Set, Module_Function_Get,...
//------------------------------------------------------------------------------------------------//
// @remark  Defines the prefix name of the functions (and variables)
#ifndef VALUEMASK_PREFIX
    #error "VALUEMASK_PREFIX not defined"
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines the number of items in the mask
#ifndef VALUEMASK_SIZE
    #error "VALUEMASK_SIZE not defined"
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines the maximum value
#ifndef VALUEMASK_MAXVALUE
    #error "VALUEMASK_MAXVALUE not defined"
#elif (VALUEMASK_MAXVALUE) > 255
    #error "VALUEMASK_MAXVALUE cannot be larger than 255"
#elif (VALUEMASK_MAXVALUE) > 127
    #define VALUEMASK_ITEMWIDTH     8
    #define VALUEMASK_ITEMMASK      0xFF
#elif (VALUEMASK_MAXVALUE) > 63
    #define VALUEMASK_ITEMWIDTH     7
    #define VALUEMASK_ITEMMASK      0x7F
#elif (VALUEMASK_MAXVALUE) > 31
    #define VALUEMASK_ITEMWIDTH     6
    #define VALUEMASK_ITEMMASK      0x3F
#elif (VALUEMASK_MAXVALUE) > 15
    #define VALUEMASK_ITEMWIDTH     5
    #define VALUEMASK_ITEMMASK      0x1F
#elif (VALUEMASK_MAXVALUE) > 7
    #define VALUEMASK_ITEMWIDTH     4
    #define VALUEMASK_ITEMMASK      0x0F
#elif (VALUEMASK_MAXVALUE) > 3
    #define VALUEMASK_ITEMWIDTH     3
    #define VALUEMASK_ITEMMASK      0x07
#elif (VALUEMASK_MAXVALUE) > 1
    #define VALUEMASK_ITEMWIDTH     2
    #define VALUEMASK_ITEMMASK      0x03
#else
    #define VALUEMASK_ITEMWIDTH     1
    #define VALUEMASK_ITEMMASK      0x01
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static U8                       VALUEMASK_PREFIX(_data)[((VALUEMASK_SIZE * VALUEMASK_ITEMWIDTH) + 7) / 8];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void VALUEMASK_PREFIX(_SetAll)(U8 value);
static void VALUEMASK_PREFIX(_ClearAll)(void);

static BOOL VALUEMASK_PREFIX(_Set)(U16 item, U8 value);
static BOOL VALUEMASK_PREFIX(_Max)(U16 item);
static BOOL VALUEMASK_PREFIX(_Up)(U16 item);
static BOOL VALUEMASK_PREFIX(_Down)(U16 item);
static BOOL VALUEMASK_PREFIX(_Clear)(U16 item);

static BOOL VALUEMASK_PREFIX(_Get)(U16 item, U8* value_ptr);

static BOOL VALUEMASK_PREFIX(_IsMax)(U16 item);
static BOOL VALUEMASK_PREFIX(_IsZero)(U16 item);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N
//------------------------------------------------------------------------------------------------//
static void VALUEMASK_PREFIX(_SetAll)(U8 value)
{
    U16 item;
    
    if(value == 0)
    {
        VALUEMASK_PREFIX(_ClearAll)();
        return;
    }
    
    for(item = 0; item < (VALUEMASK_SIZE); item++)
    {
        VALUEMASK_PREFIX(_Set)(item, value);
    }
}
//------------------------------------------------------------------------------------------------//
static void VALUEMASK_PREFIX(_ClearAll)(void)
{
    MEMSET((VPTR)VALUEMASK_PREFIX(_data), 0, SIZEOF(VALUEMASK_PREFIX(_data)));
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_Set)(U16 item, U8 value)
{
    U16 index;
    U8  offset;
    U16 temp_u16;
    
    if(item < (VALUEMASK_SIZE))
    {
        // determine position
        index    = (item * (VALUEMASK_ITEMWIDTH)) >> 3;
        offset   = (item * (VALUEMASK_ITEMWIDTH)) & 0x07;
        
        // take data
        temp_u16 = ((U16)VALUEMASK_PREFIX(_data)[index + 1] << 8) | (U16)VALUEMASK_PREFIX(_data)[index];
        
        // update
        temp_u16 &= ~((U16)(VALUEMASK_ITEMMASK) << offset);
        temp_u16 |= ((U16)value << offset);
        
        // place data back
        VALUEMASK_PREFIX(_data)[index] = temp_u16 & 0xFF;
        VALUEMASK_PREFIX(_data)[index + 1] = temp_u16 >> 8;
        
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_Max)(U16 item)
{
    return VALUEMASK_PREFIX(_Set)(item, (VALUEMASK_MAXVALUE));
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_Up)(U16 item)
{
    U8 temp_u8;
    
    if(VALUEMASK_PREFIX(_Get)(item, &temp_u8) && (temp_u8 < (VALUEMASK_MAXVALUE)))
    {
        return VALUEMASK_PREFIX(_Set)(item, temp_u8 + 1);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_Down)(U16 item)
{
    U8 temp_u8;
    
    if(VALUEMASK_PREFIX(_Get)(item, &temp_u8) && (temp_u8 > 0))
    {
        return VALUEMASK_PREFIX(_Set)(item, temp_u8 - 1);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_Clear)(U16 item)
{
    return VALUEMASK_PREFIX(_Set)(item, 0);
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_Get)(U16 item, U8* value_ptr)
{
    U16 index;
    U8  offset;
    U16 temp_u16;
    
    if(item < (VALUEMASK_SIZE))
    {
        // determine position
        index    = (item * (VALUEMASK_ITEMWIDTH)) >> 3;
        offset   = (item * (VALUEMASK_ITEMWIDTH)) & 0x07;
        
        // take data
        temp_u16 = ((U16)VALUEMASK_PREFIX(_data)[index + 1] << 8) | (U16)VALUEMASK_PREFIX(_data)[index];
        
        // extract item
        *value_ptr = (U8)((temp_u16 >> offset) & (VALUEMASK_ITEMMASK));
        return TRUE;
    }
    *value_ptr = 0;
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_IsMax)(U16 item)
{
    U8 temp_u8;
    
    return (BOOL)(VALUEMASK_PREFIX(_Get)(item, &temp_u8) && (temp_u8 == (VALUEMASK_MAXVALUE)));
}
//------------------------------------------------------------------------------------------------//
static BOOL VALUEMASK_PREFIX(_IsZero)(U16 item)
{
    U8 temp_u8;
    
    return (BOOL)(VALUEMASK_PREFIX(_Get)(item, &temp_u8) && (temp_u8 == 0));
}
//================================================================================================//




//================================================================================================//
// C L E A R / U N D E F    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#undef VALUEMASK_PREFIX
#undef VALUEMASK_SIZE
#undef VALUEMASK_MAXVALUE
#undef VALUEMASK_ITEMWIDTH
#undef VALUEMASK_ITEMMASK
//================================================================================================//
