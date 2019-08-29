//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module for converting data
// This module allows you to convert data from one data type to another
// The Functions are always defines as CoreConvert_XXXXToXXXX where XXXX can be
// Bool                 : BOOL
// U8                   : U8
// U16                  : U16
// U32                  : U32
// S8                   : S8
// S16                  : S16
// S32                  : S32
// Float                : F32
// String               : STRING formatted as normal ascii text
// HexString            : STRING formatted as hex ("0x123")
// BinString			: STRING formatted as binair ("0b01111011" or "0b0111_1011")
// DecimalString        : STRING formatted as decimal number ("123")
// CommaString          : STRING formatted as comma decimal number ("1,23")
// BoolString           : STRING formatted as boolean ("TRUE" or "true")
// Char                 : U8 that contains the ascii code of 1 character
// U8Array              : *U8
// U16Array             : *U16
// U32Array             : *U32
// String               : STRING (pointer has to be supplied by user)
// DString              : STRING (pointer is send to user)
// Rgb                  : color defined in the rgb color format
// Hsb                  : color defined in the hsb color format


// functions now available:
// ========================
// from ... to string:
// --------------------
// void CoreConvert_U8ToDecimalString(U8 in, STRING out, BOOL leading_zeros);
// void CoreConvert_U16ToDecimalString(U16 in, STRING out, BOOL leading_zeros);
// void CoreConvert_U32ToDecimalString(U32 in, STRING out, BOOL leading_zeros);
// void CoreConvert_S8ToDecimalString(S8 in, STRING out);
// void CoreConvert_S16ToDecimalString(S16 in, STRING out);
// void CoreConvert_S32ToDecimalString(S32 in, STRING out);
// void CoreConvert_U8ToHexString(U8 in, STRING out, BOOL prefix, BOOL leading_zeros);
// void CoreConvert_U16ToHexString(U16 in, STRING out, BOOL prefix, BOOL leading_zeros);
// void CoreConvert_U32ToHexString(U32 in, STRING out, BOOL prefix, BOOL leading_zeros);
// void CoreConvert_U8ToBinString(U16 in, STRING out, BOOL prefix, BOOL nibbleSplit);
// void CoreConvert_U16ToBinString(U16 in, STRING out, BOOL prefix, BOOL nibbleSplit);
// void CoreConvert_U32ToBinString(U16 in, STRING out, BOOL prefix, BOOL nibbleSplit);
// void CoreConvert_BoolToBoolString(BOOL in, STRING out); * void CoreConvert_BoolToBoolString(BOOL in, STRING out);
// void CoreConvert_U16ToCommaString(U16 in, STRING out, U8 digits_after_comma);
// void CoreConvert_U8ArryToHexString(U8* in, U8 arry_length, STRING out);

// from string to ...
// -------------------
// U32 CoreConvert_StringToU32(STRING in); //try to always use this one (auto detect string type)
// U32 CoreConvert_Ipv4StringToU32(STRING in);
// U32 CoreConvert_DecimalStringToU32(STRING in);
// U32 CoreConvert_HexStringToU32(STRING in);
// U32 CoreConvert_BinStringToU32(STRING in);
// BOOL CoreConvert_BoolStringToBool(STRING in);
// S32 CoreConvert_DecimalStringToS32(STRING in);

// array conversions
// ----------------------
// U16 CoreConvert_U8ArrayToU16(U8* data_ptr);
// U32 CoreConvert_U8ArrayToU32(U8* data_ptr);
// void CoreConvert_U16ToU8Array(U16 in, U8* data_ptr);
// void CoreConvert_U32ToU8Array(U32 in, U8* data_ptr);

// color space conversions
// ----------------------
//COLOR_HSV CoreConvert_RgbToHsv(COLOR_RGB rgb);
//COLOR_RGB CoreConvert_HsvToRgb(COLOR_HSV hsv);

// specials
// ---------
// U8 CoreConvert_HexCharToU8(CHAR in);
// U8 CoreConvert_CharTo7segmentU8(char in);
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORECONVERT_H
#define CORECONVERT_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @remark  convert to string
//          use this to convert a define value to string
//example:
//#define TEST_DEFINE 1234
//CORE_CONVERT_TO_STRING(TEST_DEFINE) wil be changed in "1234" by the precompiler
#define CORE_CONVERT_TO_STRING(x)       CORE_CONVERT_TO_STRING_SUB(x)
#define CORE_CONVERT_TO_STRING_SUB(x)   #x
//------------------------------------------------------------------------------------------------//
// @remark  macro's for MIN, MAX and ABS
#define MAX(x,y)                        (((x) > (y)) ? (x) : (y))
#define MIN(x,y)                        (((x) < (y)) ? (x) : (y))
#define ABS(x)                          (((x) >= 0) ? (x) : -(x))
//------------------------------------------------------------------------------------------------//
// @remark  macro's for MSW and LSW
#define MSW(x)                          ((x >> 16) & 0xffff)
#define LSW(x)                          (x & 0xffff)
//------------------------------------------------------------------------------------------------//
// @remark  macro's for MSB and LSB
#define MSB(x)                          ((x >> 8) & 0xff)
#define LSB(x)                          (x & 0xff)
//------------------------------------------------------------------------------------------------//
// @remark  macro's for nibbles
#define NIBBLE0(x)                      ((x) & 0x000F)
#define NIBBLE1(x)                      (((x) & 0x00F0) >> 4)
#define NIBBLE2(x)                      (((x) & 0x0F00) >> 8)
#define NIBBLE3(x)                      ((x) >> 12)
//------------------------------------------------------------------------------------------------//
// @remark  macro to make word out of bytes
#define BYTES2WORD(high,low)            (((low) & 0xFF) | ((high) << 8))
//------------------------------------------------------------------------------------------------//
// @remark  macro's to convert between different angle formats
#define DEGREES_TO_FRACTIONAL(degrees)  (((((U32)(degrees)) << 16) + 180) / 360) //watch out: 360° == 0x10000
#define FRACTIONAL_TO_DEGREES(fract)    (((((U32)(fract)) * 360) + (65536/2)) >> 16)
#define DEGREES_TO_PT1024(degrees)      (((((U32)(degrees)) << 10) + 180) / 360) //watch out: 360° == 0x0400
#define PT1024_TO_DEGREES(pt1024)       (((((U32)(pt1024)) * 360) + (1024/2)) >> 10)
//------------------------------------------------------------------------------------------------//
// @remark  macro's to convert between percentage and fractional
#define PERCENTAGE_TO_FRACTIONAL(percent)  (((((U32)(percent)) * 65535) + 50) / 100) //watch out: not shifting but multiplying with 0xFFFF because 100% == 0xFFFF  and not 0x10000
#define FRACTIONAL_TO_PERCENTAGE(fract)    (((((U32)(fract)) * 100) + (65536/2)) >> 16) //here we can shift iso divide by 65536, the result is the same !!
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   struct to define a color in hsb format
//hsv = hsb zie http://en.wikipedia.org/wiki/HSL_and_HSV
//uitleg over eenheden hsv = http://colorizer.org/
typedef struct
{
    U16 hue;             //in graden 0-360
    U8 saturation;       //in % 0-100
    U8 value;           //(= brightness) in % 0-100
}
COLOR_HSV;

// @brief   struct to define a color in rgb format
typedef struct
{
    U8 red;       //0 = no red, 0xFF = max red
    U8 green;
    U8 blue;
}
COLOR_RGB;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief	                Converts an integer value (319) into an integer string ("319")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param leading_zeros     if this var is set TRUE then the string will be padded with leading zero's until 10 digits are reached
void CoreConvert_U8ToDecimalString(U8 in, STRING out, BOOL leading_zeros);
// @brief	                Converts an integer value (319) into an integer string ("319")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param leading_zeros     if this var is set TRUE then the string will be padded with leading zero's until 10 digits are reached
void CoreConvert_U16ToDecimalString(U16 in, STRING out, BOOL leading_zeros);

// @brief	                Converts an integer value (319) into an integer string ("319")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param leading_zeros     if this var is set TRUE then the string will be padded with leading zero's until 10 digits are reached
void CoreConvert_U32ToDecimalString(U32 in, STRING out, BOOL leading_zeros);//old duco name: int2str(int a, char *s)

// @brief	                Converts an float value (3,19) into an integer string ("3,19")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param digits_after_comma    specifys how manny digits you want after the comma
void CoreConvert_FloatToDecimalString(F32 in, STRING out, U8 digits_after_comma);

// @brief	                Converts an integer value (14) into hexadecimal string ("0x14")
// @param in                input unsigned 32
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param prefix            if this is set TRUE then the prefix "0x" will be added to the outputstring
// @param leading_zeros     if this var is set TRUE then the string will be padded with leading zero's until 8 digits are reached
void CoreConvert_U8ToHexString(U8 in, STRING out, BOOL prefix, BOOL leading_zeros);

// @brief	                Converts an integer value (14) into hexadecimal string ("0x14")
// @param in                input unsigned 32
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param prefix            if this is set TRUE then the prefix "0x" will be added to the outputstring
// @param leading_zeros     if this var is set TRUE then the string will be padded with leading zero's until 8 digits are reached
void CoreConvert_U16ToHexString(U16 in, STRING out, BOOL prefix, BOOL leading_zeros);

// @brief	                Converts an integer value (14) into hexadecimal string ("0x14")
// @param in                input unsigned 32
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param prefix            if this is set TRUE then the prefix "0x" will be added to the outputstring
// @param leading_zeros     if this var is set TRUE then the string will be padded with leading zero's until 8 digits are reached
void CoreConvert_U32ToHexString(U32 in, STRING out, BOOL prefix, BOOL leading_zeros);

// @brief	                Converts an integer value (319) into the binairy representation string ("0b0000_0001_0011_1111")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param prefix            if this is set TRUE then the prefix "0b" will be added to the outputstring
// @param nibbleSplit       if this is set TRUE then the sting will be split up in nibbles by means of "_" characters to make it more readable
void CoreConvert_U8ToBinString(U8 in, STRING out, BOOL prefix, BOOL nibbleSplit);

// @brief	                Converts an integer value (319) into the binairy representation string ("0b0000_0001_0011_1111")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param prefix            if this is set TRUE then the prefix "0b" will be added to the outputstring
// @param nibbleSplit       if this is set TRUE then the sting will be split up in nibbles by means of "_" characters to make it more readable
void CoreConvert_U16ToBinString(U16 in, STRING out, BOOL prefix, BOOL nibbleSplit);

// @brief	                Converts an integer value (319) into the binairy representation string ("0b0000_0001_0011_1111")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param prefix            if this is set TRUE then the prefix "0b" will be added to the outputstring
// @param nibbleSplit       if this is set TRUE then the sting will be split up in nibbles by means of "_" characters to make it more readable
void CoreConvert_U32ToBinString(U32 in, STRING out, BOOL prefix, BOOL nibbleSplit);

// @brief	                Converts an boolean value (true) into the binairy representation string ("true")
// @param in                input integer
// @param out               pointer to a string where the output string (null terminated) will be placed
void CoreConvert_BoolToBoolString(BOOL in, STRING out);

// @brief	                Converts an integer value (316) into comma string ("31,6")
// @param in                input unsigned 16
// @param out               pointer to a string where the output string (null terminated) will be placed
// @param digits_after_comma    specifys how manny digits you want after the comma
void CoreConvert_U16ToCommaString(U16 in, STRING out, U8 digits_after_comma);

// @brief	                Converts an string contain numeric data ("444" or "0x1BC") to an integer value (444)
// @remark                  this function auto detects the input format of the string by looking at
//                          the prefix (0x for hex, 0b for binair, nothing for decimal)
// @param in                input string (zero terminated)
// @return                  decimal output
U32 CoreConvert_StringToU32(STRING in);

// @brief	                Converts an string contain numeric data ("444" or "0x1BC") to an integer value (444)
// @remark                  this function auto detects the input format of the string by looking at
//                          the prefix (0x for hex, 0b for binair, nothing for decimal)
// @param in                input string (zero terminated)
// @return                  decimal output
S32 CoreConvert_StringToS32(STRING in);

// @brief	                Converts a ipv4 address string ("192.168.100.1") to an integer value (0xc0a86401)
// @remark                  string may contain some prefix letters, convertion will start from first number
//                          its recommended not to use strings with letters in it
// @param in                input string (zero terminated)
// @return                  decimal output
U32 CoreConvert_Ipv4StringToU32(STRING in);

// @brief	                Converts an integer string ("319") to an integer value (319)
// @remark                  string may contain some prefix letters, convertion will start from first number
//                          its recommended not to use strings with letters in it
// @param in                input string (zero terminated)
// @return                  decimal output
U32 CoreConvert_DecimalStringToU32(STRING in);//old duco name: IntStrToInt

// @brief	                Converts an hex string ("0x1BC") to an integer value (444)
// @remark                  string must start with "0x"
// @param in                input string (zero terminated)
// @return                  decimal output
U32 CoreConvert_HexStringToU32(STRING in);

// @brief	                Converts an binair string ("0b0000_0101") to an integer value (5)
// @remark                  string must start with "0b and may contain nibble splitters ("_")
// @param in                input string (zero terminated)
// @return                  decimal output
U32 CoreConvert_BinStringToU32(STRING in);

// @brief	                Converts an boolean string ("true") to an Boolean value (TRUE)
// @param in                input string (zero terminated)
// @return                  boolean output
BOOL CoreConvert_BoolStringToBool(STRING in);

// @brief	                Converts an ascii character ('A') into integer value (10)
// @param in                input char
// @return                  output integer
U8 CoreConvert_HexCharToU8(CHAR in);

// @brief	                Converts an signed integer string ("-319") to an integer value (-319)
// @param in                input string (zero terminated)
// @return                  signed decimal output
S32 CoreConvert_DecimalStringToS32(STRING in);

// @brief	                Converts an ascii char to the 7segment representative symbol
// @param in                input char
// @return                  7segmentU8, bits are in following sequence ".gfedcba"
// @remark                   A  A  A
//                          F       B
//                          F       B
//                          F       B
//                           G  G  G
//                          E       C
//                          E       C
//                          E       C
//                           D  D  D   .
U8 CoreConvert_CharTo7segmentU8(char in);

// @brief   Function to get an U16 of 2 bytes of a byte array
// @param   data_ptr :      the io port of the LED
// @return   The U16 composed of the 2 bytes of the byte array
U16 CoreConvert_U8ArrayToU16(U8* data_ptr);

// @brief   Function to get an U32 of 4 bytes of a byte array
// @param   data_ptr :      the io port of the LED
// @return   The U32 composed of the 4 bytes of the byte array
U32 CoreConvert_U8ArrayToU32(U8* data_ptr);

// @brief   Function to fill a byte array with an U16
// @param   data_ptr :      The pointer to the first byte of the byte array
// @param   in :           	The U16 you want to fill in the byte array
void CoreConvert_U16ToU8Array(U16 in, U8* data_ptr);

// @brief   Function to fill a byte array with an U32
// @param   data_ptr :      The pointer to the first byte of the byte array
// @param   in :           	The U32 you want to fill in the byte array
void CoreConvert_U32ToU8Array(U32 in, U8* data_ptr);

// @brief   Function to convert Pt1024 U16 to degree U16
// @param   in :     		Pt1024 U16 (0-1023)
// return   return :        degree U16 (*10 to provide 0.1 degree resolution)
U16 CoreConvert_Pt1024U16ToDegreeU16(U16 in);

// @brief   Function to convert degree U16 to Pt1024 U16
// @param   in :     		degree U16 (*10 to provide 0.1 degree resolution)
// return   return :        Pt1024 U16 (0-1023)
U16 CoreConvert_DegreeU16ToPt1024U16(U16 in);

// @brief   Function to convert a color defined in the rgb color space to a color defined in the hsv color space
// @param   in :     		color defined using the COLOR_RGB struct
// return   return :        color defined using the COLOR_HSV struct
COLOR_HSV CoreConvert_RgbToHsv(COLOR_RGB rgb);

// @brief   Function to convert a color defined in the hsv color space to a color defined in the rgb color space
// @param   in :     		color defined using the COLOR_HSV struct
// return   return :        color defined using the COLOR_RGB struct
COLOR_RGB CoreConvert_HsvToRgb(COLOR_HSV hsv);

// @brief	                Converts an signed integer value (-319) into an integer string ("-319")
// @param in                input signed integer
// @param out               pointer to a string where the output string (null terminated) will be placed
void CoreConvert_S8ToDecimalString(S8 in, STRING out);

// @brief	                Converts an signed integer value (-319) into an integer string ("-319")
// @param in                input signed integer
// @param out               pointer to a string where the output string (null terminated) will be placed
void CoreConvert_S16ToDecimalString(S16 in, STRING out);

// @brief	                Converts an signed integer value (-319) into an integer string ("-319")
// @param in                input signed integer
// @param out               pointer to a string where the output string (null terminated) will be placed
void CoreConvert_S32ToDecimalString(S32 in, STRING out);

// @brief	                Converts an U8 arry ({0,6,126,234}) into an visual readable string ("00 06 7E EA")
// @param in                pointer to arry
// @param arry_length       length of arry in bytes
// @param out               pointer to a string where the output string (null terminated) will be placed
void CoreConvert_U8ArryToHexString(U8* in, U8 arry_length, STRING out);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//

#endif /* CORECONVERT_H */
