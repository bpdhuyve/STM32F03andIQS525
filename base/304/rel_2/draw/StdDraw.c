//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to create a screenbuffer and modify it
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STD_DRAW_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef GPIO__DRVGPIO_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               GPIO__DRVGPIO_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the buffer format std draw has to to operate on
// note: formats where the colors dont have the same number of bits can make AA fonts ugly because some shades of gray will be shown in a light tint of color
//
// the possibilities are:
// - BUFFER_FORMAT_RGB8_R3G3B2   for now only this mode is supported
//
// other future modes
// - BUFFER_FORMAT_MONOCHROME_1BPP
// - BUFFER_FORMAT_GRAYSCALE_8BPP
// - BUFFER_FORMAT_COLOR_12BPP_R4G4B4   //hiervan kunnen we nog een 16b aligned variant maken ook indien nodig(+4 ignored bits)
// - BUFFER_FORMAT_RGB16_R5G6B5
// - BUFFER_FORMAT_COLOR_24BPP_R8G8B8   //hiervan kunnen we nog een 32b aligned variant maken ook indien nodig(+8 ignored bits)

#ifndef STD_DRAW_BUFFER_FORMAT
    #error "STD_DRAW_BUFFER_FORMAT not defined in AppConfig"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYSTEM include section
//DRIVER include section
//STANDARD include section
//APPLICATION include section

#include "draw/StdDraw.h"
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
static void DrawPixel(U8* pixel_ptr, COLOR color);
static void DrawImage_MonochromeReColored(POINT location, IMAGE image, COLOR white_replacement_color, COLOR black_replacement_color);
static void DrawImage_GrayscaleReColored(POINT location, IMAGE image, COLOR white_replacement_color, COLOR black_replacement_color);
static void DrawImageRegion_8b(POINT location, IMAGE image, REGION image_region);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static U8*              screen_buffer = NULL;
static REGION           screen_region;

static EVENT_CALLBACK   flush_function = NULL;

//96 chars van 17 pixels hoog op 8 pixels breed, iedere byte is 1 lijn van het karakter
static const U16   debug_font[96][17] ={
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Char ' ' dec 32  (0x20)
    {0x00,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00}, //Char '!' dec 33  (0x21)
    {0x00,0x00,0x00,0x6c,0x6c,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Char '"' dec 34  (0x22)
    {0x00,0x00,0x00,0x00,0x44,0x44,0xfe,0x44,0x44,0x44,0xfe,0x44,0x44,0x00,0x00,0x00,0x00}, //Char '#' dec 35  (0x23)
    {0x00,0x00,0x00,0x00,0x10,0x7c,0x90,0x90,0x7c,0x12,0x12,0x7c,0x10,0x00,0x00,0x00,0x00}, //Char '$' dec 36  (0x24)
    {0x00,0x00,0x00,0x00,0x00,0xc2,0xc6,0x0c,0x18,0x30,0x60,0xc6,0x86,0x00,0x00,0x00,0x00}, //Char '%' dec 37  (0x25)
    {0x00,0x00,0x00,0x00,0x38,0x44,0x48,0x30,0x30,0x4a,0x84,0x8a,0x70,0x00,0x00,0x00,0x00}, //Char '&' dec 38  (0x26)
    {0x00,0x00,0x00,0x18,0x08,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Char ''' dec 39  (0x27)
    {0x00,0x00,0x00,0x04,0x08,0x10,0x10,0x30,0x30,0x30,0x10,0x10,0x08,0x04,0x00,0x00,0x00}, //Char '(' dec 40  (0x28)
    {0x00,0x00,0x00,0x20,0x10,0x08,0x08,0x0c,0x0c,0x0c,0x08,0x08,0x10,0x20,0x00,0x00,0x00}, //Char ')' dec 41  (0x29)
    {0x00,0x00,0x00,0x00,0x00,0x10,0xd6,0x38,0x38,0x38,0xd6,0x10,0x00,0x00,0x00,0x00,0x00}, //Char '*' dec 42  (0x2a)
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7e,0x7e,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00}, //Char '+' dec 43  (0x2b)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x04,0x08,0x00,0x00}, //Char ',' dec 44  (0x2c)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x7e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Char '-' dec 45  (0x2d)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00}, //Char '.' dec 46  (0x2e)
    {0x00,0x00,0x00,0x00,0x00,0x02,0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x00,0x00,0x00,0x00}, //Char '/' dec 47  (0x2f)
    {0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xce,0xd6,0xe6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,0x00}, //Char '0' dec 48  (0x30)
    {0x00,0x00,0x00,0x0c,0x1c,0x2c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e,0x00,0x00,0x00}, //Char '1' dec 49  (0x31)
    {0x00,0x00,0x00,0x7c,0xc6,0xc6,0x06,0x0c,0x18,0x30,0x60,0xc0,0xc6,0xfe,0x00,0x00,0x00}, //Char '2' dec 50  (0x32)
    {0x00,0x00,0x00,0x7c,0xc6,0xc6,0x06,0x06,0x1c,0x06,0x06,0xc6,0xc6,0x7c,0x00,0x00,0x00}, //Char '3' dec 51  (0x33)
    {0x00,0x00,0x00,0x0c,0x1c,0x2c,0x4c,0xcc,0xcc,0xfe,0x0c,0x0c,0x0c,0x0c,0x00,0x00,0x00}, //Char '4' dec 52  (0x34)
    {0x00,0x00,0x00,0xfe,0xc2,0xc0,0xc0,0xdc,0xe6,0x06,0x06,0x06,0xc6,0x7c,0x00,0x00,0x00}, //Char '5' dec 53  (0x35)
    {0x00,0x00,0x00,0x7c,0xc6,0xc0,0xc0,0xc0,0xfc,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,0x00}, //Char '6' dec 54  (0x36)
    {0x00,0x00,0x00,0xfe,0xc6,0x06,0x06,0x0c,0x18,0x30,0x60,0xc0,0xc0,0xc0,0x00,0x00,0x00}, //Char '7' dec 55  (0x37)
    {0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0x7c,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,0x00}, //Char '8' dec 56  (0x38)
    {0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0x06,0x06,0xc6,0x7c,0x00,0x00,0x00}, //Char '9' dec 57  (0x39)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00}, //Char ':' dec 58  (0x3a)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x08,0x10,0x00,0x00}, //Char ';' dec 59  (0x3b)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x18,0x30,0x60,0x30,0x18,0x0c,0x00,0x00,0x00,0x00}, //Char '<' dec 60  (0x3c)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x7e,0x00,0x00,0x7e,0x7e,0x00,0x00,0x00,0x00,0x00}, //Char '=' dec 61  (0x3d)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x18,0x0c,0x06,0x0c,0x18,0x30,0x00,0x00,0x00,0x00}, //Char '>' dec 62  (0x3e)
    {0x00,0x00,0x00,0x7c,0xc6,0xc6,0x0c,0x0c,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00}, //Char '?' dec 63  (0x3f)
    {0x00,0x00,0x00,0x00,0x7c,0x82,0x8a,0xba,0xaa,0xaa,0xbc,0x80,0x78,0x00,0x00,0x00,0x00}, //Char '@' dec 64  (0x40)
    {0x00,0x00,0x00,0x38,0x6c,0xc6,0xc6,0xc6,0xc6,0xfe,0xc6,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'A' dec 65  (0x41)
    {0x00,0x00,0x00,0xfc,0x66,0x66,0x66,0x66,0x7c,0x66,0x66,0x66,0x66,0xfc,0x00,0x00,0x00}, //Char 'B' dec 66  (0x42)
    {0x00,0x00,0x00,0x38,0x6c,0xc6,0xc0,0xc0,0xc0,0xc0,0xc0,0xc6,0x6c,0x38,0x00,0x00,0x00}, //Char 'C' dec 67  (0x43)
    {0x00,0x00,0x00,0xf8,0x6c,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x6c,0xf8,0x00,0x00,0x00}, //Char 'D' dec 68  (0x44)
    {0x00,0x00,0x00,0xfe,0x62,0x60,0x60,0x60,0x78,0x60,0x60,0x60,0x62,0xfe,0x00,0x00,0x00}, //Char 'E' dec 69  (0x45)
    {0x00,0x00,0x00,0xfe,0x62,0x60,0x60,0x60,0x78,0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00}, //Char 'F' dec 70  (0x46)
    {0x00,0x00,0x00,0x38,0x6c,0xc6,0xc0,0xc0,0xce,0xc6,0xc6,0xc6,0x6c,0x38,0x00,0x00,0x00}, //Char 'G' dec 71  (0x47)
    {0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xfe,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'H' dec 72  (0x48)
    {0x00,0x00,0x00,0x3c,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,0x00}, //Char 'I' dec 73  (0x49)
    {0x00,0x00,0x00,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0xcc,0xcc,0x78,0x00,0x00,0x00}, //Char 'J' dec 74  (0x4a)
    {0x00,0x00,0x00,0x66,0x66,0x66,0x6c,0x78,0x70,0x78,0x6c,0x66,0x66,0x66,0x00,0x00,0x00}, //Char 'K' dec 75  (0x4b)
    {0x00,0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x62,0x7e,0x00,0x00,0x00}, //Char 'L' dec 76  (0x4c)
    {0x00,0x00,0x00,0x82,0xc6,0xee,0xd6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'M' dec 77  (0x4d)
    {0x00,0x00,0x00,0x86,0xc6,0xe6,0xd6,0xce,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'N' dec 78  (0x4e)
    {0x00,0x00,0x00,0x38,0x6c,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x6c,0x38,0x00,0x00,0x00}, //Char 'O' dec 79  (0x4f)
    {0x00,0x00,0x00,0xfc,0x66,0x66,0x66,0x66,0x7c,0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00}, //Char 'P' dec 80  (0x50)
    {0x00,0x00,0x00,0x38,0x6c,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xce,0x6c,0x3a,0x00,0x00,0x00}, //Char 'Q' dec 81  (0x51)
    {0x00,0x00,0x00,0xfc,0x66,0x66,0x66,0x66,0x7c,0x78,0x6c,0x64,0x66,0x66,0x00,0x00,0x00}, //Char 'R' dec 82  (0x52)
    {0x00,0x00,0x00,0x7c,0xc6,0xc0,0xc0,0xc0,0x7c,0x06,0x06,0x06,0xc6,0x7c,0x00,0x00,0x00}, //Char 'S' dec 83  (0x53)
    {0x00,0x00,0x00,0x7e,0x5a,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00}, //Char 'T' dec 84  (0x54)
    {0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,0x00}, //Char 'U' dec 85  (0x55)
    {0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x6c,0x6c,0x38,0x38,0x10,0x00,0x00,0x00}, //Char 'V' dec 86  (0x56)
    {0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xd6,0xee,0xc6,0x82,0x00,0x00,0x00}, //Char 'W' dec 87  (0x57)
    {0x00,0x00,0x00,0xc6,0xc6,0xc6,0x6c,0x38,0x10,0x38,0x6c,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'X' dec 88  (0x58)
    {0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x3c,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00}, //Char 'Y' dec 89  (0x59)
    {0x00,0x00,0x00,0xfe,0x06,0x06,0x0c,0x18,0x30,0x60,0xc0,0xc0,0xc0,0xfe,0x00,0x00,0x00}, //Char 'Z' dec 90  (0x5a)
    {0x00,0x00,0x00,0x7c,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x7c,0x00,0x00,0x00}, //Char '[' dec 91  (0x5b)
    {0x00,0x00,0x00,0x00,0x80,0xc0,0x60,0x30,0x18,0x0c,0x06,0x03,0x01,0x00,0x00,0x00,0x00}, //Char '\' dec 92  (0x5c)
    {0x00,0x00,0x00,0x7c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x7c,0x00,0x00,0x00}, //Char ']' dec 93  (0x5d)
    {0x00,0x00,0x00,0x10,0x38,0x6c,0xc6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Char '^' dec 94  (0x5e)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xfe,0x00,0x00}, //Char '_' dec 95  (0x5f)
    {0x00,0x00,0x00,0x60,0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Char '`' dec 96  (0x60)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0x06,0x06,0x7e,0xc6,0xc6,0xc6,0x7e,0x00,0x00,0x00}, //Char 'a' dec 97  (0x61)
    {0x00,0x00,0x00,0xc0,0xc0,0xc0,0xfc,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xfc,0x00,0x00,0x00}, //Char 'b' dec 98  (0x62)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0xc0,0xc0,0xc0,0xc0,0xc6,0x7c,0x00,0x00,0x00}, //Char 'c' dec 99  (0x63)
    {0x00,0x00,0x00,0x06,0x06,0x06,0x7e,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7e,0x00,0x00,0x00}, //Char 'd' dec 100 (0x64)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0xc6,0xfe,0xc0,0xc0,0xc0,0x7c,0x00,0x00,0x00}, //Char 'e' dec 101 (0x65)
    {0x00,0x00,0x00,0x3c,0x66,0x60,0x60,0x60,0xf0,0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00}, //Char 'f' dec 102 (0x66)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0xc6,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0x7c,0x00,0x00}, //Char 'g' dec 103 (0x67)
    {0x00,0x00,0x00,0xc0,0xc0,0xc0,0xfc,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'h' dec 104 (0x68)
    {0x00,0x00,0x00,0x00,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,0x00}, //Char 'i' dec 105 (0x69)
    {0x00,0x00,0x00,0x00,0x0c,0x00,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0xf8,0x00,0x00}, //Char 'j' dec 106 (0x6a)
    {0x00,0x00,0x00,0xc0,0xc0,0xc0,0xc6,0xcc,0xd8,0xf0,0xf0,0xd8,0xcc,0xc6,0x00,0x00,0x00}, //Char 'k' dec 107 (0x6b)
    {0x00,0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,0x00}, //Char 'l' dec 108 (0x6c)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xcc,0xfe,0xd6,0xd6,0xd6,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'm' dec 109 (0x6d)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,0x00}, //Char 'n' dec 110 (0x6e)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,0x00}, //Char 'o' dec 111 (0x6f)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0xc6,0xc6,0xc6,0xc6,0xc6,0xfc,0xc0,0xc0,0x00,0x00}, //Char 'p' dec 112 (0x70)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0xc6,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0x06,0x00,0x00}, //Char 'q' dec 113 (0x71)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xdc,0xe6,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x00,0x00,0x00}, //Char 'r' dec 114 (0x72)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc0,0xc0,0x7c,0x06,0x06,0x06,0x7c,0x00,0x00,0x00}, //Char 's' dec 115 (0x73)
    {0x00,0x00,0x00,0x60,0x60,0x60,0xf8,0x60,0x60,0x60,0x60,0x60,0x66,0x3c,0x00,0x00,0x00}, //Char 't' dec 116 (0x74)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7e,0x00,0x00,0x00}, //Char 'u' dec 117 (0x75)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0xc6,0x6c,0x6c,0x38,0x38,0x10,0x00,0x00,0x00}, //Char 'v' dec 118 (0x76)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xd6,0xee,0x44,0x00,0x00,0x00}, //Char 'w' dec 119 (0x77)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0x6c,0x38,0x38,0x6c,0xc6,0xc6,0x00,0x00,0x00}, //Char 'x' dec 120 (0x78)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0xfc,0x00,0x00}, //Char 'y' dec 121 (0x79)
    {0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x06,0x0c,0x18,0x30,0x60,0xc0,0xfe,0x00,0x00,0x00}, //Char 'z' dec 122 (0x7a)
    {0x00,0x00,0x00,0x0c,0x10,0x10,0x10,0x10,0x60,0x10,0x10,0x10,0x10,0x0c,0x00,0x00,0x00}, //Char '{' dec 123 (0x7b)
    {0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, //Char '|' dec 124 (0x7c)
    {0x00,0x00,0x00,0x60,0x10,0x10,0x10,0x10,0x0c,0x10,0x10,0x10,0x10,0x60,0x00,0x00,0x00}, //Char '}' dec 125 (0x7d)
    {0x00,0x00,0x00,0x62,0x92,0x8c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //Char '~' dec 126 (0x7e)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x28,0x44,0x82,0x82,0x82,0xfe,0x00,0x00,0x00,0x00}  //Char ' ' dec 127 (0x7f)
};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//implementation for BUFFER_FORMAT_RGB8_R3G3B2
static void DrawPixel(U8* pixel_ptr, COLOR color)
{
    if (color.alpha == 255) //no alpha channel dus moeten we geen rekening houden met de oude pixel value, zet gewoon kleur in de buffer
    {
        *pixel_ptr = (color.red & 0xE0) | ((color.green & 0xE0) >> 3) | ((color.blue & 0xC0) >> 6);
        return;
    }
    else if (color.alpha == 0) //volledig transparent dus return doe gewoon niets
    {
        return;
    }
    else //wel alpha dus neem oude pixel value samen met requested value en verdeel dit volgens de alpha waarde
    {
        U8 old_pixel_value_red =            (*pixel_ptr >> 5);
        U8 old_pixel_value_green =          (*pixel_ptr >> 2) & 0x07;
        U8 old_pixel_value_blue =           (*pixel_ptr) & 0x03;
        U8 requested_pixel_value_red =      (color.red & 0xE0) >> 5;
        U8 requested_pixel_value_green =    (color.green & 0xE0) >> 5;
        U8 requested_pixel_value_blue =     (color.blue & 0xC0) >> 6;
        U16 new_pixel_value_red =           (((old_pixel_value_red * (255-color.alpha))/255) + ((requested_pixel_value_red * color.alpha)/255));
        U16 new_pixel_value_green =         (((old_pixel_value_green * (255-color.alpha))/255) + ((requested_pixel_value_green * color.alpha)/255));
        U16 new_pixel_value_blue =          (((old_pixel_value_blue * (255-color.alpha))/255) + ((requested_pixel_value_blue * color.alpha)/255));

        if (new_pixel_value_red > 0x07)    //waarde kan niet groter zijn dan 7 omdat er maar 3 bits zijn
        {
            new_pixel_value_red = 0x7;
        }
        if (new_pixel_value_green > 0x07)  //waarde kan niet groter zijn dan 7 omdat er maar 3 bits zijn
        {
            new_pixel_value_green = 0x7;
        }
        if (new_pixel_value_blue > 0x03)   //waarde kan niet groter zijn dan 3 omdat er maar 2 bits zijn
        {
            new_pixel_value_blue = 0x3;
        }

        *pixel_ptr = ((new_pixel_value_red << 4) | (new_pixel_value_green << 2) |(new_pixel_value_blue));
        return;
    }
}
//------------------------------------------------------------------------------------------------//
static void DrawImage_MonochromeReColored(POINT location, IMAGE image, COLOR white_replacement_color, COLOR black_replacement_color)
{
    POINT buffer_coordinates;
    U32 image_stream_byte_offset = 0;
    U8 image_stream_bit_offset = 0;

    if (image.format == IMAGE_FORMAT_MONOCHROME)
    {
        for(buffer_coordinates.y = location.y; buffer_coordinates.y < (location.y + image.size.height); buffer_coordinates.y++)
        {
            for(buffer_coordinates.x = location.x; buffer_coordinates.x < (location.x + image.size.width); buffer_coordinates.x ++)
            {
                if ((image.location_in_flash[image_stream_byte_offset]) & (0x80>>(image_stream_bit_offset)))
                {
                    DrawPixel(&screen_buffer[buffer_coordinates.x + (buffer_coordinates.y * screen_region.size.width)],white_replacement_color);
                }
                else
                {
                    DrawPixel(&screen_buffer[buffer_coordinates.x + (buffer_coordinates.y * screen_region.size.width)],black_replacement_color);
                }

                image_stream_bit_offset++;

                if(image_stream_bit_offset >= 8)    //er zijn al 8 bits overlopen geweest van de image, dus ga naar de volgende byte (increase byte counter en reset bit counter)
                {
                    image_stream_byte_offset++;     //so increase byte ofset counter
                    image_stream_bit_offset = 0;    //and reset bit ofset
                }
            }

            if(image_stream_bit_offset != 0)    //we beginen aan de volgende regel van de image, indien de bit niet nul is wilt dit zeggen dat ie nog ergens in de midden van een byte zat dus ga door naar de volgende byte want de nieuwe regel van de image begint altijd met een nieuwe byte
            {
                image_stream_byte_offset++;     //y coordinate increased, so go direct to next byte
                image_stream_bit_offset = 0;    //and reset bit ofset ofcource
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void DrawImage_GrayscaleReColored(POINT location, IMAGE image, COLOR white_replacement_color, COLOR black_replacement_color)
{
    //made this function as a test to try to acief AA font but does not work good on 8bits buffer because we can not show many grays,this is because we have diferent nr of bits for the colors so some grays will show blueisch en therfore the font will look like crap, yet to trie what the effect is on a 16bit buffer
    POINT buffer_coordinates;
    U32 image_stream_byte_offset = 0;
    COLOR color = color_White;

    if (image.format == IMAGE_FORMAT_GRAYSCALE8)
    {
        for(buffer_coordinates.y = location.y; buffer_coordinates.y < (location.y + image.size.height); buffer_coordinates.y++)
        {
            for(buffer_coordinates.x = location.x; buffer_coordinates.x < (location.x + image.size.width); buffer_coordinates.x ++)
            {
                //warning UNCOMPLETE EXPERIMENTAL IMPLEMENTATION, replacement colors ar not applied
                color.alpha = image.location_in_flash[image_stream_byte_offset];
                DrawPixel(&screen_buffer[buffer_coordinates.x + (buffer_coordinates.y * screen_region.size.width)],color);

                image_stream_byte_offset++;     //so byte ofset counter to check next pixel
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void DrawImageRegion_8b(POINT location, IMAGE image, REGION image_region)
{
    U16 rows;
    U16 pixels;
    U8* base_ptr;
    const U8* source_ptr;
    REGION  source_region = {0, 0, image.size.width, image.size.height};
    REGION  target_region = {location.x, location.y, image_region.size.width, image_region.size.height};
    
    // crop target region to screen region
    if(StdDraw_RegionIntersect(&target_region, &screen_region, &target_region) == FALSE)
    {
        // target region not within screen region
        return;
    }
    
    // update image region with cropped target region
    image_region.location.x += target_region.location.x - location.x;
    image_region.location.y += target_region.location.y - location.y;
    image_region.size.width = target_region.size.width;
    image_region.size.height = target_region.size.height;
    
    // crop source region to image region
    if(StdDraw_RegionIntersect(&source_region, &image_region, &source_region) == FALSE)
    {
        // requested image region not within image
        return;
    }
    
    // update target region with cropped source (image) region
    target_region.location.x += source_region.location.x - image_region.location.x;
    target_region.location.y += source_region.location.y - image_region.location.y;
    target_region.size.width = source_region.size.width;
    target_region.size.height = source_region.size.height;
    
    // copy data
    base_ptr = &screen_buffer[(target_region.location.y * screen_region.size.width) + target_region.location.x];
    source_ptr = &image.location_in_flash[(source_region.location.y * image.size.width) + source_region.location.x];
    rows = source_region.size.height;
    while(rows-- > 0)
    {
        pixels = source_region.size.width;
        while(pixels-- > 0)
        {
            *base_ptr++ = *source_ptr++;
        }
        base_ptr += screen_region.size.width - source_region.size.width;
        source_ptr += image.size.width - source_region.size.width;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdDraw_Init(U8* screenbuffer, SIZE screensize, EVENT_CALLBACK flush_callback)
{
    flush_function = flush_callback;
    screen_buffer = screenbuffer;
    screen_region.location.x = 0;
    screen_region.location.y = 0;
    screen_region.size = screensize;

    StdDraw_Clear();
}
//------------------------------------------------------------------------------------------------//
void StdDraw_Flush(void)
{
    if(flush_function != NULL)
    {
        flush_function();
    }
}
//------------------------------------------------------------------------------------------------//
void StdDraw_Clear(void)
{
    MEMSET((VPTR)screen_buffer, 0, screen_region.size.width * screen_region.size.height);
}
//------------------------------------------------------------------------------------------------//
void StdDraw_ClearRegion(REGION region)
{
    U8*     base_ptr;
    U16     rows;
    
    if(StdDraw_RegionIntersect(&region, &screen_region, &region) == TRUE)
    {
        base_ptr = &screen_buffer[(region.location.y * screen_region.size.width) + region.location.x];
        rows = region.size.height;
        while(rows > 0)
        {
            MEMSET((VPTR)base_ptr, 0, region.size.width);
            base_ptr += screen_region.size.width;
            rows--;
        }
    }
}
//------------------------------------------------------------------------------------------------//
void StdDraw_FillRegion(REGION region, COLOR color)
{
    U16 rows;
    U16 pixels;
    U8* base_ptr;
    
    if(StdDraw_RegionIntersect(&region, &screen_region, &region) == TRUE)
    {
        base_ptr = &screen_buffer[(region.location.y * screen_region.size.width) + region.location.x];
        rows = region.size.height;
        while(rows-- > 0)
        {
            pixels = region.size.width;
            while(pixels-- > 0)
            {
                DrawPixel(base_ptr++, color);
            }
            base_ptr += screen_region.size.width - region.size.width;
        }
    }
}
//------------------------------------------------------------------------------------------------//
BOOL StdDraw_RegionIntersect(REGION* region1_ptr, REGION* region2_ptr, REGION* intersect_ptr)
{
    S16 x1 = MAX(region1_ptr->location.x, region2_ptr->location.x);
    S16 x2 = MIN(region1_ptr->location.x + region1_ptr->size.width, region2_ptr->location.x + region2_ptr->size.width);
    S16 y1 = MAX(region1_ptr->location.y, region2_ptr->location.y);
    S16 y2 = MIN(region1_ptr->location.y + region1_ptr->size.height, region2_ptr->location.y + region2_ptr->size.height);
    
    if((x2 > x1) && (y2 > y1))
    {
        intersect_ptr->location.x = x1;
        intersect_ptr->location.y = y1;
        intersect_ptr->size.width = x2 - x1;
        intersect_ptr->size.height = y2 - y1;
        return TRUE;
    }
    return FALSE;
}//------------------------------------------------------------------------------------------------//
void StdDraw_DrawImage(POINT location, IMAGE image)
{
    REGION  image_region = {0, 0, image.size.width, image.size.height};
    
    switch(image.format)
    {
    case IMAGE_FORMAT_MONOCHROME:
        DrawImage_MonochromeReColored(location, image, color_White, color_White);
        break;
    case IMAGE_FORMAT_GRAYSCALE8:
        DrawImage_GrayscaleReColored(location, image, color_White, color_White);
        break;
    case IMAGE_FORMAT_RGB8_R3G3B2:
        DrawImageRegion_8b(location, image, image_region);
        break;
    default:
        break;
    }
}
//------------------------------------------------------------------------------------------------//
void StdDraw_DrawImageRegion(POINT location, IMAGE image, REGION region)
{
    switch(image.format)
    {
    case IMAGE_FORMAT_MONOCHROME:
        break;
    case IMAGE_FORMAT_GRAYSCALE8:
        break;
    case IMAGE_FORMAT_RGB8_R3G3B2:
        DrawImageRegion_8b(location, image, region);
        break;
    default:
        break;
    }
}
//------------------------------------------------------------------------------------------------//
void StdDraw_DrawText(POINT location, STRING text)
{
    //todo maken dat je niet buiten het scherm kan schrijven want nu flowed dat over naar de volgende lijn
    //todo maken als er een \r\n in zit dat ie overgaat naar de volgende lijn
    U8 textCharCounter = 0;
    while(*text !=0)
    {
        U8 j;
        for(j = 0; j < 17; j++)//char height loop, chars zijn 17 pixels hoog
        {
            U8 i;
            for(i = 0; i < 8; i++)//char width loop, chars zijn 8 pixels breed
            {
                if ((debug_font[*text - 32][j])& 0x80>>i) //check if that pixel bit is set (min 32 omdat de eerste 32 chars niet in de font zitten
                {
                    screen_buffer[(i+location.x)+((j+location.y)*screen_region.size.width)+(textCharCounter*8)] = 0xFF; //if yes color pixel zwart
                }
                else
                {
                    screen_buffer[(i+location.x)+((j+location.y)*screen_region.size.width)+(textCharCounter*8)] = 0x00;   //if no color pixel wit
                }
            }
        }
        text++;
        textCharCounter++;
    }
}
//------------------------------------------------------------------------------------------------//
void StdDraw_DrawTextStyled(POINT location, STRING text, STYLE style)
{
    //todo maken dat je niet buiten het scherm kan schrijven want nu flowed dat over naar de volgende lijn
    //todo maken als er een \r\n in zit dat ie overgaat naar de volgende lijn
    U8 textCharCounter = 0;
    POINT next_char_coordinates = location;
    while(*text !=0)
    {
        //lookup char in reference table
        U8 reference_number;
        for(reference_number = 0; reference_number < style.font.char_count; reference_number++)
        {
            if (style.font.reference_stream_pointer[reference_number] == *text)
            {
                break;
            }
        }

        switch(style.font.format)
        {
            case FONT_FORMAT_MONOCHROME:
                DrawImage_MonochromeReColored(next_char_coordinates, *style.font.image_stream_pointer[reference_number], style.back_color, style.front_color);
                break;
            case FONT_FORMAT_GRAYSCALE:
                DrawImage_GrayscaleReColored(next_char_coordinates, *style.font.image_stream_pointer[reference_number], style.back_color, style.front_color);
        }

        next_char_coordinates.x += style.font.image_stream_pointer[reference_number]->size.width;

        text++;
        textCharCounter++;
    }
}
//------------------------------------------------------------------------------------------------//
void StdDraw_ShiftRegion(REGION source_region, POINT target)
{
    U16 rows;
    U16 pixels;
    U8* base_ptr;
    U8* target_ptr;
    
    S32 target_offset;
    
    //target = base + offset --> offset = target - base
    target_offset = (S32)((target.y * screen_region.size.width) + target.x);
    target_offset -= (S32)((source_region.location.y * screen_region.size.width) + source_region.location.x);
    
    base_ptr = &screen_buffer[(source_region.location.y * screen_region.size.width) + source_region.location.x];
    rows = source_region.size.height;
    
    if(target.y <= source_region.location.y)
    {
        // van boven naar onder
        if(target.x <= source_region.location.x)
        {
            // van links naar rechts
            while(rows-- > 0)
            {
                target_ptr = base_ptr + target_offset;
                pixels = source_region.size.width;
                while(pixels-- > 0)
                {
                    *target_ptr++ = *base_ptr++;
                }
                base_ptr += screen_region.size.width - source_region.size.width;
            }
        }
        else
        {
            // van rechts naar links
            base_ptr += source_region.size.width - 1;
            while(rows-- > 0)
            {
                target_ptr = base_ptr + target_offset;
                pixels = source_region.size.width;
                while(pixels-- > 0)
                {
                    *target_ptr-- = *base_ptr--;
                }
                base_ptr += screen_region.size.width + source_region.size.width;
            }
        }
    }
    else
    {
        // onder naar boven
        base_ptr += (source_region.size.height - 1) * screen_region.size.width;
        if(target.x <= source_region.location.x)
        {
            // van links naar rechts
            while(rows-- > 0)
            {
                target_ptr = base_ptr + target_offset;
                pixels = source_region.size.width;
                while(pixels-- > 0)
                {
                    *target_ptr++ = *base_ptr++;
                }
                base_ptr -= screen_region.size.width + source_region.size.width;
            }
        }
        else
        {
            // van rechts naar links
            base_ptr += source_region.size.width - 1;
            while(rows-- > 0)
            {
                target_ptr = base_ptr + target_offset;
                pixels = source_region.size.width;
                while(pixels-- > 0)
                {
                    *target_ptr-- = *base_ptr--;
                }
                base_ptr -= screen_region.size.width - source_region.size.width;
            }
        }
    }
}
//================================================================================================//

