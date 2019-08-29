//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to create a screenbuffer and modify it
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRAW__STDDRAW_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRAW__STDDRAW_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               DRAW__STDDRAW_LOG_LEVEL
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
#ifdef RUN_FROM_RAM
    #pragma diag_suppress=Ta022   //(Possible rom access from within a __ramfunc function)              = static strings
    #pragma diag_suppress=Ta023   //(Call to a non __ramfunc function from within a __ramfunc function) = because prototypes are not marked ramfunc !
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrawPixel(PIXEL* pixel_ptr, COLOR color);
static void DrawImage_MonochromeReColored(POINT location, IMAGE image, REGION image_region, COLOR white_replacement_color, COLOR black_replacement_color);
static void DrawImage_GrayscaleReColored(POINT location, IMAGE image, REGION image_region, COLOR white_replacement_color, COLOR black_replacement_color);
static void DrawImage_8bColor(POINT location, IMAGE image, REGION image_region);
static void DrawImage_16bColor(POINT location, IMAGE image, REGION image_region);
static BOOL StdDraw_CropRegionToScreen(POINT* location_ptr, REGION* region_ptr);
static void DrawTextDebug(POINT location, STRING text);
static void DrawTextStyled(POINT location, STYLE style, STRING text);
static void DrawImageRegionRecolored(POINT location, IMAGE image, REGION region, COLOR white_replacement_color, COLOR black_replacement_color);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static PIXEL*           screen_buffer = NULL;

static REGION           screen_region;

static EVENT_CALLBACK   flush_function = NULL;

//96 chars van 17 pixels hoog op 8 pixels breed, iedere byte is 1 lijn van het karakter
static const U8         debug_font[96][17] ={
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
#ifdef RUN_FROM_RAM
__ramfunc
#endif
static void DrawPixel(PIXEL* pixel_ptr, COLOR color)
{
    COLOR   pixel_color;

    if(color.alpha == 0) //volledig transparent = niets tekenen
    {
        return;
    }

    if(color.alpha < 255)
    {
        #if(STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB8_R3G3B2)
        {
            pixel_color.red = (U8)pixel_ptr->colors.red << 5;
            pixel_color.green = (U8)pixel_ptr->colors.green << 5;
            pixel_color.blue = (U8)pixel_ptr->colors.blue << 6;
        }
        #elif(STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB16_R5G6B5)
        {
            pixel_color.red = (U8)pixel_ptr->colors.red << 3;
            pixel_color.green = (U8)pixel_ptr->colors.green << 2;
            pixel_color.blue = (U8)pixel_ptr->colors.blue << 3;
        }
        #else
        {
            #error "unsupported buffer format"
        }
        #endif

        color.red   = pixel_color.red   + (U8)(((U16)(color.red - pixel_color.red) * ((U16)color.alpha + 1)) >> 8);
        color.green = pixel_color.green + (U8)(((U16)(color.green - pixel_color.green) * ((U16)color.alpha + 1)) >> 8);
        color.blue  = pixel_color.blue  + (U8)(((U16)(color.blue - pixel_color.blue) * ((U16)color.alpha + 1)) >> 8);
    }

    #if(STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB8_R3G3B2)
    {
        pixel_ptr->rgb = (color.red & 0xE0) | ((color.green & 0xE0) >> 3) | ((color.blue & 0xC0) >> 6);
    }
    #elif(STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB16_R5G6B5)
    {
        pixel_ptr->rgb = (((U16)color.red & 0xF8) << 8) | (((U16)color.green & 0xFC) << 3) | (((U16)color.blue & 0xF8) >> 3);
    }
    #else
    {
        #error "unsupported buffer format"
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
static void DrawImage_MonochromeReColored(POINT location, IMAGE image, REGION image_region, COLOR white_replacement_color, COLOR black_replacement_color)
{
    U16         rows;
    U16         pixels;
    U8          bit;
    U8          byte;
    PIXEL*      target_ptr;
    const U8*   image_ptr;

    target_ptr = &screen_buffer[(location.y * screen_region.size.width) + location.x];
    image_ptr = image.location_in_flash + (image_region.location.y * ((image.size.width + 7) >> 3) + (image_region.location.x >> 3));

    rows = image_region.size.height;

    while(rows-- > 0)
    {
        pixels = image_region.size.width;
        bit = image_region.location.x & 0x07;
        byte = 0;
        while(pixels-- > 0)
        {
            if(image_ptr[byte] & (0x80>>bit))
            {
                DrawPixel(target_ptr, white_replacement_color);
            }
            else
            {
                DrawPixel(target_ptr, black_replacement_color);
            }

            if(++bit > 0x07)
            {
                bit = 0;
                byte++;
            }
            target_ptr++;
        }
        target_ptr += screen_region.size.width - image_region.size.width;
        image_ptr += ((image.size.width + 7) >> 3);
    }
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
static void DrawImage_GrayscaleReColored(POINT location, IMAGE image, REGION image_region, COLOR white_replacement_color, COLOR black_replacement_color)
{
    U16         rows;
    U16         pixels;
    PIXEL*      target_ptr;
    const U8*   image_ptr;

    target_ptr = &screen_buffer[(location.y * screen_region.size.width) + location.x];
    image_ptr = image.location_in_flash + (image_region.location.y * image.size.width) + image_region.location.x;

    rows = image_region.size.height;

    while(rows-- > 0)
    {
        pixels = image_region.size.width;
        while(pixels-- > 0)
        {
            DrawPixel(target_ptr, black_replacement_color);
            white_replacement_color.alpha = (U8)*image_ptr++;
            DrawPixel(target_ptr, white_replacement_color);
            target_ptr++;
        }
        target_ptr += screen_region.size.width - image_region.size.width;
        image_ptr += image.size.width - image_region.size.width;
    }
}
//------------------------------------------------------------------------------------------------//
//warning deze functie mag niet gecalled worden met location of region die buiten het scherm ligt, de hogergelegen functie StdDraw_DrawImageRegion moet hier voor zorgen
#ifdef RUN_FROM_RAM
__ramfunc
#endif
static void DrawImage_8bColor(POINT location, IMAGE image, REGION image_region)
{
    U16         rows;
    U16         pixels;
    PIXEL*      target_ptr;
    const U8*   image_ptr;

    target_ptr = &screen_buffer[(location.y * screen_region.size.width) + location.x];
    image_ptr = image.location_in_flash + (image_region.location.y * image.size.width) + image_region.location.x;

    rows = image_region.size.height;

    while(rows-- > 0)
    {
        pixels = image_region.size.width;
        while(pixels-- > 0)
        {
            #if STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB8_R3G3B2
            {
                target_ptr->rgb = *image_ptr;
            }
            #elif STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB16_R5G6B5
            {
                target_ptr->rgb = (((*image_ptr & 0xE0) << 8) | ((*image_ptr & 0x1C) << 6) | ((*image_ptr & 0x03) << 3));  //scale image up to 16b
            }
            #else
            {
                LOG_WRN("Unsupported image format");
                return;
            }
            #endif
            image_ptr++;
            target_ptr++;
        }
        target_ptr += screen_region.size.width - image_region.size.width;
        image_ptr += image.size.width - image_region.size.width;
    }
}
//------------------------------------------------------------------------------------------------//
//warning deze functie mag niet gecalled worden met location of region die buiten het scherm ligt, de hogergelegen functie StdDraw_DrawImageRegion moet hier voor zorgen
#ifdef RUN_FROM_RAM
__ramfunc
#endif
static void DrawImage_16bColor(POINT location, IMAGE image, REGION image_region)
{
    U16         rows;
    U16         pixels;
    PIXEL*      target_ptr;
    const U8*   image_ptr;

    target_ptr = &screen_buffer[(location.y * screen_region.size.width) + location.x];
    image_ptr = image.location_in_flash + ((image_region.location.y * image.size.width + image_region.location.x) << 1);

    rows = image_region.size.height;

    while(rows-- > 0)
    {
        pixels = image_region.size.width;
        while(pixels-- > 0)
        {
            #if(STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB8_R3G3B2)
            {
                target_ptr->rgb = (U8)(*image_ptr & 0xE0);
                target_ptr->rgb |= (U8)(*image_ptr++ & 0x07) << 2;
                target_ptr->rgb |= (U8)(*image_ptr++ & 0x18) << 3;
            }
            #elif(STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB16_R5G6B5)
            {
                //deze shift zit er hier in omdat de buffer in litte endian is en de images in big endian zijn, in de displaydriver wordt dit dan terug omgewisseld
                target_ptr->rgb = (U16)(*(image_ptr++)) << 8;
                target_ptr->rgb |= (U16)(*image_ptr++);
            }
            #else
            {
                LOG_WRN("Unsupported image format");
                return;
            }
            #endif
            target_ptr++;
        }
        target_ptr += screen_region.size.width - image_region.size.width;
        image_ptr += (image.size.width - image_region.size.width) << 1;
    }
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
static BOOL StdDraw_CropRegionToScreen(POINT* location_ptr, REGION* region_ptr)
{
    REGION  target_region = {location_ptr->x, location_ptr->y, region_ptr->size.width, region_ptr->size.height};

    if(StdDraw_CalculateIntersection(target_region, screen_region, &target_region) == FALSE)
    {
        return FALSE;
    }

    // set region location to zero-point
    region_ptr->location.x -= location_ptr->x;
    region_ptr->location.y -= location_ptr->y;

    // take over new target location
    location_ptr->x = target_region.location.x;
    location_ptr->y = target_region.location.y;

    // reset region location
    region_ptr->location.x += location_ptr->x;
    region_ptr->location.y += location_ptr->y;

    // take over region size
    region_ptr->size.width = target_region.size.width;
    region_ptr->size.height = target_region.size.height;

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void DrawTextDebug(POINT location, STRING text)
{
    //todo maken dat je niet buiten het scherm kan schrijven want nu flowed dat over naar de volgende lijn
    //todo maken als er een \r\n in zit dat ie overgaat naar de volgende lijn

    U16         rows;
    U16         pixels;
    PIXEL*      target_ptr = &screen_buffer[(location.y * screen_region.size.width) + location.x];
    const U8*   source_ptr;

    while(*text != 0)
    {
        source_ptr = &debug_font[*text - 32][0];
        rows = 17;
        while(rows-- > 0)
        {
            pixels = 8;
            while(pixels-- > 0)
            {
                if(*source_ptr & (1<<pixels))
                {
                    DrawPixel(target_ptr, color_White);
                }
                else
                {
                    DrawPixel(target_ptr, color_Black);
                }
                target_ptr++;
            }
            target_ptr += screen_region.size.width - 8;
            source_ptr++;
        }
        target_ptr -= (screen_region.size.width * 17) - 8;
        text++;
    }
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void DrawTextStyled(POINT location, STYLE style, STRING text)
{
    POINT           char_location = location;
    IMAGE**         char_image_ptr_ptr;
    const U8*       char_ref_ptr;
    const FONT*     font_ptr = style.font_ptr;
    REGION          region = {0, 0, 0, 0};

    while(*text != 0)
    {
        if((*text == '\r') || (*text == '\n'))
        {
            char_location.x = location.x;
            char_location.y += (*font_ptr->image_stream_pointer)->size.height + 1;
        }
        else
        {
            char_image_ptr_ptr = &font_ptr->image_stream_pointer[font_ptr->char_count - 1];
            char_ref_ptr   = &font_ptr->reference_stream_pointer[font_ptr->char_count - 1];

            while((*char_ref_ptr != *text) && (char_ref_ptr > font_ptr->reference_stream_pointer))
            {
                char_ref_ptr--;
                char_image_ptr_ptr--;
            }
            if(*char_ref_ptr != *text)
            {
                // character not found
                continue;
            }

            region.size.width = (*char_image_ptr_ptr)->size.width;
            region.size.height = (*char_image_ptr_ptr)->size.height;

            DrawImageRegionRecolored(char_location, **char_image_ptr_ptr, region, style.back_color, style.front_color);

            char_location.x += (*char_image_ptr_ptr)->size.width;
        }
        text++;
    }
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
//this function is used to draw the letters of fonts
void DrawImageRegionRecolored(POINT location, IMAGE image, REGION region, COLOR white_replacement_color, COLOR black_replacement_color)
{
    //pas de region en location aan zodading dat deze binnen de image en binnen het scherm vallen
    REGION  image_region = {0, 0, image.size.width, image.size.height};

    if(StdDraw_CalculateIntersection(image_region, region, &region) == FALSE)
    {
        return; //requested region not within image region
    }

    if(StdDraw_CropRegionToScreen(&location, &region) == FALSE)
    {
        return; //target region not within screen region
    }

    switch(image.format)
    {
        case IMAGE_FORMAT_MONOCHROME:
            DrawImage_MonochromeReColored(location, image, region, white_replacement_color, black_replacement_color);   //voorlopig voor deze 2 nog geen drawregion
            break;
        case IMAGE_FORMAT_GRAYSCALE8:
            DrawImage_GrayscaleReColored(location, image, region, white_replacement_color, black_replacement_color);    //voorlopig voor deze 2 nog geen drawregion
            break;
        default:
            LOG_WRN("not supported");
            break;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdDraw_Init(U8* screenbuffer, SIZE screensize, EVENT_CALLBACK flush_callback)
{
    flush_function = flush_callback;

    screen_buffer = (PIXEL*)screenbuffer;
    screen_region.location.x = 0;
    screen_region.location.y = 0;
    screen_region.size = screensize;

    StdDraw_Clear();
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_Flush(void)
{
    if(flush_function != NULL)
    {
        flush_function();
    }
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_Clear(void)
{
    StdDraw_ClearRegion(screen_region);
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_ClearRegion(REGION region)
{
    StdDraw_FillRegion(region, color_Black);
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_FillRegion(REGION region, COLOR color)
{
    U16     rows;
    U16     pixels;
    PIXEL*  base_ptr;

    if(StdDraw_CalculateIntersection(region, screen_region, &region) == TRUE)
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
#ifdef RUN_FROM_RAM
__ramfunc
#endif
BOOL StdDraw_CalculateIntersection(REGION region1_ptr, REGION region2_ptr, REGION* intersect_result_ptr)
{
    S16 x1 = MAX(region1_ptr.location.x, region2_ptr.location.x);
    S16 x2 = MIN(region1_ptr.location.x + region1_ptr.size.width, region2_ptr.location.x + region2_ptr.size.width);
    S16 y1 = MAX(region1_ptr.location.y, region2_ptr.location.y);
    S16 y2 = MIN(region1_ptr.location.y + region1_ptr.size.height, region2_ptr.location.y + region2_ptr.size.height);

    if((x2 > x1) && (y2 > y1))
    {
        intersect_result_ptr->location.x = x1;
        intersect_result_ptr->location.y = y1;
        intersect_result_ptr->size.width = x2 - x1;
        intersect_result_ptr->size.height = y2 - y1;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_DrawImage(POINT location, IMAGE image)
{
    REGION  image_region = {0, 0, image.size.width, image.size.height};

    StdDraw_DrawImageRegion(location, image, image_region);
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_DrawImageRegion(POINT location, IMAGE image, REGION region)
{
    //pas de region en location aan zodading dat deze binnen de image en binnen het scherm vallen
    REGION  image_region = {0, 0, image.size.width, image.size.height};

    if(StdDraw_CalculateIntersection(image_region, region, &region) == FALSE)
    {
        return; //requested region not within image region
    }
    if(StdDraw_CropRegionToScreen(&location, &region) == FALSE)
    {
        return; //target region not within screen region
    }

    switch(image.format)
    {
    case IMAGE_FORMAT_MONOCHROME:
        DrawImage_MonochromeReColored(location, image, region, color_White, color_Black);   //voorlopig voor deze 2 nog geen drawregion
        break;
    case IMAGE_FORMAT_GRAYSCALE8:
        DrawImage_GrayscaleReColored(location, image, region, color_White, color_Black);    //voorlopig voor deze 2 nog geen drawregion
        break;
    case IMAGE_FORMAT_RGB8_R3G3B2:
        DrawImage_8bColor(location, image, region);
        break;
    case IMAGE_FORMAT_RGB16_R5G6B5:
        DrawImage_16bColor(location, image, region);
        break;
    default:
        LOG_WRN("image format not supported");
        break;
    }
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_DrawText(POINT location, STYLE style, STRING text)
{
    if (style.font_ptr == NULL) //if no font pointer is found, draw debug text
    {
        DrawTextDebug(location, text);
    }
    else
    {
        DrawTextStyled(location, style, text);
    }
}
//------------------------------------------------------------------------------------------------//
#ifdef RUN_FROM_RAM
__ramfunc
#endif
void StdDraw_MoveRegion(REGION source_region, POINT target)
{
    U16     rows;
    U16     pixels;
    PIXEL*  source_ptr;
    PIXEL*  target_ptr;

    S32     target_offset;

    //target = source + offset --> offset = target - source
    target_offset = (S32)((target.y * screen_region.size.width) + target.x);
    target_offset -= (S32)((source_region.location.y * screen_region.size.width) + source_region.location.x);

    source_ptr = &screen_buffer[(source_region.location.y * screen_region.size.width) + source_region.location.x];
    rows = source_region.size.height;

    // if target is above the source or at the same height and left: copy from top to bottom and from left to right
    // otherwise: copy from bottom to top and from right to left
    if((target.y < source_region.location.y) || ((target.y == source_region.location.y) && (target.x < source_region.location.x)))
    {
        // work way to last pixel of source region
        while(rows-- > 0)
        {
            target_ptr = source_ptr + target_offset;
            pixels = source_region.size.width;
            while(pixels-- > 0)
            {
                *target_ptr++ = *source_ptr++;
            }
            source_ptr += screen_region.size.width - source_region.size.width;
        }
    }
    else
    {
        // jump to last pixel of source region and work way back to first
        source_ptr += (source_region.size.height - 1) * screen_region.size.width + (source_region.size.width - 1);
        while(rows-- > 0)
        {
            target_ptr = source_ptr + target_offset;
            pixels = source_region.size.width;
            while(pixels-- > 0)
            {
                *target_ptr-- = *source_ptr--;
            }
            source_ptr -= screen_region.size.width - source_region.size.width;
        }
    }
}
//================================================================================================//
