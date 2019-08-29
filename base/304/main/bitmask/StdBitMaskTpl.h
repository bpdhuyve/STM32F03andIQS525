//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// template header file for generating and handling a bitmask
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
// @remark  Template usage:
//
// #define BITMASK_PREFIX(postfix)                 Module_Function##postfix
// #define BITMASK_SIZE                            (100)
// #include "bitmask\StdBitMaskTpl.h"
//
// --> Functions become Module_Function_Set, Module_Function_Get,...
//------------------------------------------------------------------------------------------------//
// @remark  Defines the prefix name of the functions (and variables)
#ifndef BITMASK_PREFIX
    #error "BITMASK_PREFIX not defined"
#endif
//------------------------------------------------------------------------------------------------//
// @remark  Defines the number of bits to be used in the mask
#ifndef BITMASK_SIZE
    #error "BITMASK_SIZE not defined"
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
static U8                       BITMASK_PREFIX(_mask)[((BITMASK_SIZE) + 7) / 8];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void BITMASK_PREFIX(_UpdateAll)(BOOL set_on);
static void BITMASK_PREFIX(_SetAll)(void);
static void BITMASK_PREFIX(_ClearAll)(void);

static BOOL BITMASK_PREFIX(_Update)(U16 bit, BOOL set_on);
static BOOL BITMASK_PREFIX(_Set)(U16 bit);
static BOOL BITMASK_PREFIX(_Clear)(U16 bit);

static BOOL BITMASK_PREFIX(_Get)(U16 bit);

static BOOL BITMASK_PREFIX(_GetFirst)(BOOL set_on, U16* bit_ptr);
static BOOL BITMASK_PREFIX(_GetFirstSet)(U16* bit_ptr);
static BOOL BITMASK_PREFIX(_GetFirstClear)(U16* bit_ptr);

static U16 BITMASK_PREFIX(_GetCount)(BOOL set_on);
static U16 BITMASK_PREFIX(_GetCountSet)(void);
static U16 BITMASK_PREFIX(_GetCountClear)(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N
//------------------------------------------------------------------------------------------------//
static void BITMASK_PREFIX(_UpdateAll)(BOOL set_on)
{
    if(set_on)
    {
        BITMASK_PREFIX(_SetAll)();
        return;
    }
    BITMASK_PREFIX(_ClearAll)();
}
//------------------------------------------------------------------------------------------------//
static void BITMASK_PREFIX(_SetAll)(void)
{
    MEMSET((VPTR)BITMASK_PREFIX(_mask), 0xFF, SIZEOF(BITMASK_PREFIX(_mask)));
}
//------------------------------------------------------------------------------------------------//
static void BITMASK_PREFIX(_ClearAll)(void)
{
    MEMSET((VPTR)BITMASK_PREFIX(_mask), 0, SIZEOF(BITMASK_PREFIX(_mask)));
}
//------------------------------------------------------------------------------------------------//
static BOOL BITMASK_PREFIX(_Update)(U16 bit, BOOL set_on)
{
    if(set_on)
    {
        return BITMASK_PREFIX(_Set)(bit);
    }
    return BITMASK_PREFIX(_Clear)(bit);
}
//------------------------------------------------------------------------------------------------//
static BOOL BITMASK_PREFIX(_Set)(U16 bit)
{
    if(bit < (BITMASK_SIZE))
    {
        BITMASK_PREFIX(_mask)[bit >> 3] |= (0x01 << (bit & 0x07));
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL BITMASK_PREFIX(_Clear)(U16 bit)
{
    if(bit < (BITMASK_SIZE))
    {
        BITMASK_PREFIX(_mask)[bit >> 3] &= ~(0x01 << (bit & 0x07));
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL BITMASK_PREFIX(_Get)(U16 bit)
{
    if(bit < (BITMASK_SIZE))
    {
        return (BOOL)((BITMASK_PREFIX(_mask)[bit >> 3] & (0x01 << (bit & 0x07))) > 0);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL BITMASK_PREFIX(_GetFirst)(BOOL set_on, U16* bit_ptr)
{
    for(*bit_ptr = 0; *bit_ptr < (BITMASK_SIZE); (*bit_ptr)++)
    {
        if(BITMASK_PREFIX(_Get)(*bit_ptr) == set_on)
        {
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL BITMASK_PREFIX(_GetFirstSet)(U16* bit_ptr)
{
    return BITMASK_PREFIX(_GetFirst)(TRUE, bit_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL BITMASK_PREFIX(_GetFirstClear)(U16* bit_ptr)
{
    return BITMASK_PREFIX(_GetFirst)(FALSE, bit_ptr);
}
//------------------------------------------------------------------------------------------------//
static U16 BITMASK_PREFIX(_GetCount)(BOOL set_on)
{
    U16 bit;
    U16 count = 0;
    
    for(bit = 0; bit < (BITMASK_SIZE); bit++)
    {
        if(BITMASK_PREFIX(_Get)(bit) == set_on)
        {
            count++;
        }
    }
    return count;
}
//------------------------------------------------------------------------------------------------//
static U16 BITMASK_PREFIX(_GetCountSet)(void)
{
    return BITMASK_PREFIX(_GetCount)(TRUE);
}
//------------------------------------------------------------------------------------------------//
static U16 BITMASK_PREFIX(_GetCountClear)(void)
{
    return BITMASK_PREFIX(_GetCount)(FALSE);
}
//================================================================================================//




//================================================================================================//
// C L E A R / U N D E F    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#undef BITMASK_PREFIX
#undef BITMASK_SIZE
//================================================================================================//
