//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// collection of all sorts of math functions
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MATH__STDMATH_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MATH__STDMATH_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MATH__STDMATH_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "math\StdMath.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U32 Match_SineTableInterpolation(U16 index);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const U16    math_sine_tbl[257] = 
{
    0x0000, 0x00C9, 0x0192, 0x025B, 0x0324, 0x03ED, 0x04B6, 0x057F, 0x0648, 0x0711,
    0x07D9, 0x08A2, 0x096B, 0x0A33, 0x0AFB, 0x0BC4, 0x0C8C, 0x0D54, 0x0E1C, 0x0EE4,
    0x0FAB, 0x1073, 0x113A, 0x1201, 0x12C8, 0x138F, 0x1455, 0x151C, 0x15E2, 0x16A8,
    0x176E, 0x1833, 0x18F9, 0x19BE, 0x1A83, 0x1B47, 0x1C0C, 0x1CD0, 0x1D93, 0x1E57,
    0x1F1A, 0x1FDD, 0x209F, 0x2162, 0x2224, 0x22E5, 0x23A7, 0x2467, 0x2528, 0x25E8,
    0x26A8, 0x2768, 0x2827, 0x28E5, 0x29A4, 0x2A62, 0x2B1F, 0x2BDC, 0x2C99, 0x2D55,
    0x2E11, 0x2ECC, 0x2F87, 0x3042, 0x30FC, 0x31B5, 0x326E, 0x3327, 0x33DF, 0x3497,
    0x354E, 0x3604, 0x36BA, 0x3770, 0x3825, 0x38D9, 0x398D, 0x3A40, 0x3AF3, 0x3BA5,
    0x3C57, 0x3D08, 0x3DB8, 0x3E68, 0x3F17, 0x3FC6, 0x4074, 0x4121, 0x41CE, 0x427A,
    0x4326, 0x43D1, 0x447B, 0x4524, 0x45CD, 0x4675, 0x471D, 0x47C4, 0x486A, 0x490F,
    0x49B4, 0x4A58, 0x4AFB, 0x4B9E, 0x4C40, 0x4CE1, 0x4D81, 0x4E21, 0x4EC0, 0x4F5E,
    0x4FFB, 0x5098, 0x5134, 0x51CF, 0x5269, 0x5303, 0x539B, 0x5433, 0x54CA, 0x5560,
    0x55F6, 0x568A, 0x571E, 0x57B1, 0x5843, 0x58D4, 0x5964, 0x59F4, 0x5A82, 0x5B10,
    0x5B9D, 0x5C29, 0x5CB4, 0x5D3E, 0x5DC8, 0x5E50, 0x5ED7, 0x5F5E, 0x5FE4, 0x6068,
    0x60EC, 0x616F, 0x61F1, 0x6272, 0x62F2, 0x6371, 0x63EF, 0x646C, 0x64E9, 0x6564,
    0x65DE, 0x6657, 0x66D0, 0x6747, 0x67BD, 0x6832, 0x68A7, 0x691A, 0x698C, 0x69FD,
    0x6A6E, 0x6ADD, 0x6B4B, 0x6BB8, 0x6C24, 0x6C8F, 0x6CF9, 0x6D62, 0x6DCA, 0x6E31,
    0x6E97, 0x6EFB, 0x6F5F, 0x6FC2, 0x7023, 0x7083, 0x70E3, 0x7141, 0x719E, 0x71FA,
    0x7255, 0x72AF, 0x7308, 0x735F, 0x73B6, 0x740B, 0x7460, 0x74B3, 0x7505, 0x7556,
    0x75A6, 0x75F4, 0x7642, 0x768E, 0x76D9, 0x7723, 0x776C, 0x77B4, 0x77FB, 0x7840,
    0x7885, 0x78C8, 0x790A, 0x794A, 0x798A, 0x79C9, 0x7A06, 0x7A42, 0x7A7D, 0x7AB7,
    0x7AEF, 0x7B27, 0x7B5D, 0x7B92, 0x7BC6, 0x7BF9, 0x7C2A, 0x7C5A, 0x7C89, 0x7CB7,
    0x7CE4, 0x7D0F, 0x7D3A, 0x7D63, 0x7D8A, 0x7DB1, 0x7DD6, 0x7DFB, 0x7E1E, 0x7E3F,
    0x7E60, 0x7E7F, 0x7E9D, 0x7EBA, 0x7ED6, 0x7EF0, 0x7F0A, 0x7F22, 0x7F38, 0x7F4E,
    0x7F62, 0x7F75, 0x7F87, 0x7F98, 0x7FA7, 0x7FB5, 0x7FC2, 0x7FCE, 0x7FD9, 0x7FE2,
    0x7FEA, 0x7FF1, 0x7FF6, 0x7FFA, 0x7FFE, 0x7FFF, 0x8000
};
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
static U32 Match_SineTableInterpolation(U16 index)
{
    // index is between 0 and 0x4000 (14bit) (0° - 90°)
    // sine table has 256 entries (8bit) and returns 15-bit sine value (0 - 0x8000)
    
    U16*        tbl_ptr = (U16*)&math_sine_tbl[index >> 6];
    U16         fraction = index & 0x003F;
    U32         step = 0;
    
    if(fraction > 0)
    {
        step = ((U32)(*(tbl_ptr+1))) - ((U32)(*tbl_ptr));
        step *= (U32)fraction;
        step += 0x20;
        step >>= 6;
    }
    
    return ((U32)(*tbl_ptr) + (S32)step);
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief   function to calculate a rounddown SQRT value
//          The excel equivalent formula is: ROUNDDOWN(SQRT(value);0) and has been tested that way
U8 StdMath_Sqrt(U16 value)
{
    S32 a;
    S32 c;
    S32 temp_s32;
    U32 temp_u32 = value << 16;
    U8  shift = 0;
    
    while(temp_u32 > 0xFFFF)
    {
        temp_u32 >>= 2;
        shift++;
    }
    
    // a_0 = S
    // c_0 = S-1
    
    // a_n+1 = a_n - a_n*c_n/2
    // c_n+1 = c_n^2*(c_n - 3)/4
    
    // a_n --> SQRT(S)
    // c_n --> 0
    
    if(temp_u32 == 0)
    {
        return 0;
    }
    
    a = (S32)temp_u32;
    c = a;
    c -= 0x10000;
    
    while(c)
    {
        //LOG_DEV("a %d c %d", PS32(a), PS32(c));
        temp_s32 = (a * c) >> 17;
        a -= temp_s32;
        temp_s32 = (c - 0x30000) >> 3;
        temp_s32 *= c;
        temp_s32 >>= 16;
        temp_s32 *= c;
        c = (temp_s32 >> 15);
    }
    
    // Control mechanism to fix some faults in the algorithm
    temp_u32 = ((a << shift) >> 16);
    temp_u32 *= temp_u32;
    
    if(temp_u32 > (U32)value)
    {
        return (U8)(((a << shift) >> 16) - 1);
    }
    return (U8)((a << shift) >> 16);
}
//------------------------------------------------------------------------------------------------//
// @brief   function to calculate an SQRT from an U32
U16 StdMath_Sqrt32(U32 value)
{
    U32 c = 0x8000;
    U32 g = 0x8000;
    
    while(c > 0)
    {
        if((g*g) > value)
        {
            g ^= c;
        }
        c >>= 1;
        g |= c;
    }
    return g;
}
//------------------------------------------------------------------------------------------------//
// @brief   function to calculate the sine of an angle (0° = 0x0000 - 360° = 0x10000)
// @return  returns a 15-bit sine value between (-1 = -0x8000 and 1 = 0x8000)
S32 StdMath_Sine(U32 angle)
{
    // throw away cycle info
    angle &= 0x0000FFFF;
    
    // determine sine
    switch(angle & 0xC000)
    {
    case 0x0000:    // for   0 < x <  90 --> sin(x) = sin(x)
        return (S32)Match_SineTableInterpolation(angle);
    case 0x4000:    // for  90 < x < 180 --> sin(x) = sin(180 - x)
        return (S32)Match_SineTableInterpolation(0x8000 - angle);
    case 0x8000:    // for 180 < x < 270 --> sin(x) = -sin(x - 180)
        return -(S32)Match_SineTableInterpolation(angle - 0x8000);
    default:        // for 270 < x < 360 --> sin(x) = -sin(-x)
        return -(S32)Match_SineTableInterpolation(0 - angle);
    }
}
//------------------------------------------------------------------------------------------------//
// @brief   function to calculate the cosine of an angle (0° = 0x0000 - 360° = 0x10000)
// @return  returns a 15-bit cosine value between (-1 = -0x8000 and 1 = 0x8000)
S32 StdMath_Cosine(U32 angle)
{
    // throw away cycle info
    angle &= 0x0000FFFF;
    
    // determine cosine
    switch(angle & 0xC000)
    {
    case 0x0000:    // for   0 < x <  90 --> cos(x) = sin(90 - x)
        return (S32)Match_SineTableInterpolation(0x4000 - angle);
    case 0x4000:    // for  90 < x < 180 --> cos(x) = -sin(x - 90)
        return -(S32)Match_SineTableInterpolation(angle - 0x4000);
    case 0x8000:    // for 180 < x < 270 --> cos(x) = -sin(270 - x)
        return -(S32)Match_SineTableInterpolation(0xC000 - angle);
    default:        // for 270 < x < 360 --> cos(x) = sin(x - 270)
        return (S32)Match_SineTableInterpolation(angle - 0xC000);
    }
}
//------------------------------------------------------------------------------------------------//
// @brief   function to calculate the average of an unsigned 16 bit range,
//			The excel equivalent formula is: ROUND(SUM(table_ptr)/length;0) and has been tested that way
//------------------------------------------------------------------------------------------------//
U16 StdMath_AvgU16(U16* table_ptr, U16 length)
{
    U32 sum = 0;
    U32 i;
    
    for(i = 0; i < length; i++)
    {
        sum += table_ptr[i];
    }
    return (U16)((sum + (length / 2)) / length);
}
//------------------------------------------------------------------------------------------------//
// @brief   function to calculate the average of a signed 16 bit range,
//			The excel equivalent formula is: ROUND(SUM(table_ptr)/length;0) and has been tested that way
//------------------------------------------------------------------------------------------------//
S16 StdMath_AvgS16(S16* table_ptr, S16 length)
{
    S32 sum = 0;
    U32 i;
    
    for(i = 0; i < length; i++)
    {
        sum += table_ptr[i];
    }
    
    if(sum < 0)
    {
        sum -= (length / 2);
    }
    else
    {
        sum += (length / 2);
    }
    return (S16)(sum / length);
}
//================================================================================================//
