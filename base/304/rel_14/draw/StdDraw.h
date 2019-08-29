//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to create a screenbuffer and modify it
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRAW__STDDRAW_H
#define DRAW__STDDRAW_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the buffer format std draw has to to operate on
// note: formats where the colors dont have the same number of bits can make AA fonts ugly because some shades of gray will be shown in a light tint of color
#ifndef STD_DRAW_BUFFER_FORMAT
    #error "STD_DRAW_BUFFER_FORMAT not defined in AppConfig"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//possible buffer formats
//all buffer streams start with the top left pixel (x=0,y=0), the next pixel in the stream is x=1,y=0
#define BUFFER_FORMAT_GRAY4    	1
#define BUFFER_FORMAT_RGB332    2
#define BUFFER_FORMAT_RGB565    3

// other future formats
// - BUFFER_FORMAT_GRAY8
// - BUFFER_FORMAT_RGB444
// - BUFFER_FORMAT_RGB888

//old buffer format names, must be placed here for backwards compatibility
#define BUFFER_FORMAT_RGB8_R3G3B2     						BUFFER_FORMAT_RGB332
#define BUFFER_FORMAT_RGB16_R5G6B5    						BUFFER_FORMAT_RGB565
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
#if (STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB332)
typedef union
{
    struct
    {
        U8  blue:2;
        U8  green:3;
        U8  red:3;
    }
    colors;
    U8  rgb;
}
PIXEL;

#elif (STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_RGB565)
typedef union
{
    struct
    {
        U16 blue:5;
        U16 green:6;
        U16 red:5;
    }
    colors;
    U16 rgb;
}
PIXEL;

#elif STD_DRAW_BUFFER_FORMAT == BUFFER_FORMAT_GRAY4
	#warning BUFFER_FORMAT_GRAY4 not supported yet

#else
    #error "unknown buffer format"
#endif
//------------------------------------------------------------------------------------------------//
//different image formats with different bpp (bits per pixel)
typedef enum
{
    IMAGE_FORMAT_MONO,			//1bpp: black/white	8 pixels next to each other are merged in one byte, lsb = most left pixel, msb = most right pixel
    IMAGE_FORMAT_GRAY4,      	//4bpp:  4bit grayscale, 2 pixels next to each other are merged in one byte, the low nibble is the left pixel, high nibble the right one
    IMAGE_FORMAT_GRAY8,      	//8bpp:  8bit grayscale
    IMAGE_FORMAT_RGB332,     	//8bpp:  8bit color
    IMAGE_FORMAT_RGB565,    	//16bpp: 16bit color

    //not suported yet
    //IMAGE_FORMAT_RGB444		//12bpp: 12bit color
    //IMAGE_FORMAT_RGBA4444  	//16bpp: 12bit color with transparency
    //IMAGE_FORMAT_RGBA6666  	//24bpp: 18bit color with transparency
    //IMAGE_FORMAT_RGBA8888  	//32bpp: 24bit color with transparency
}
IMAGE_FORMAT;

// gray4 image(or buffer) layout
// 2 pixels next to eachother (with the same y value) are merged into one byte as following
// high nible = right pixel, low nible = left pixel
//
// example an image of 5 pixels wide and 3 high will be stored in memory as following
// [pix02,pix01],[pix04,pix03],[0,pix05],
// [pix07,pix06],[pix09,pix08],[0,pix10],
// [pix12,pix11],[pix14,pix13],[0,pix15],
//------------------------------------------------------------------------------------------------//
typedef enum
{
    //todo: dit field wordt eigenlijk niet gebruikt aangezien een font gewoon een reference stream is en een image stream, het is de image format die die bepaald hoe een letter getekend wordt, dit veld moet wijzigen naar een soort OPTION FIELD waarbij een eerste optie UTF8 kan zijn
    FONT_FORMAT_MONOCHROME,         //(1bit per pixel)
    FONT_FORMAT_GRAYSCALE,          //(8bits per pixel) experimental anti aliased font
}
FONT_FORMAT;
//------------------------------------------------------------------------------------------------//
typedef enum
{
    HORIZONTAL,
    VERTICAL,
}
DIRECTION;
//------------------------------------------------------------------------------------------------//
// @brief   struct to define a point on a 2d plane
// coordinate {x=0,y=0} is the top left side of the screen
// x = column value, y = row value
//
// 	            X
//   +------------------------>
//   |
// Y | screen as user sees it
//   |
//   v
//
typedef struct
{
    S16 x;
    S16 y;
}
POINT;
//------------------------------------------------------------------------------------------------//
// @brief   struct to define the size of something
typedef struct
{
    U16 width;
    U16 height;
}
SIZE;
//------------------------------------------------------------------------------------------------//
// @brief   struct to define a region on a 2d plane (combination of point and size)
typedef struct
{
    POINT location;
    SIZE size;
}
REGION;
//------------------------------------------------------------------------------------------------//
// @brief   struct to define a color, if no alpha channel is required set alpha to 0xFF
typedef struct
{
    U8 red;       //0 = no red, 0xFF = max red
    U8 green;
    U8 blue;
    U8 alpha;     //0 = fully transparent, 0xFF = fully opaque(visble)
}
COLOR;
//------------------------------------------------------------------------------------------------//
// @brief   struct to define a image
typedef struct
{
    IMAGE_FORMAT format;
    SIZE size;
    const U8* location_in_flash;
}
IMAGE;
//------------------------------------------------------------------------------------------------//
/// @brief	struct to define a font
typedef struct
{
     FONT_FORMAT   format;
     U8            char_count;
     const U8*     reference_stream_pointer;      //this points to an arry (of size size "chars_count") which contains the ascii value of the represented image that can be found at the same position in the image stream
     IMAGE**       image_stream_pointer;          //this points to an arry (of size size "chars_count") which contains pointers to images, each image represents one character
}
FONT;
//------------------------------------------------------------------------------------------------//
/// @brief	struct to define a style, you can use this in the code to create a style which is basicly saying your going to use that font with those colours
typedef struct
{
    const FONT* font_ptr;
    COLOR front_color;
    COLOR back_color;
}
STYLE;
//------------------------------------------------------------------------------------------------//
typedef struct
{
    SIZE    size;
    STYLE   style;
    STRING  text;
}
TEXTBOX;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//standard color list, source http://www.opinionatedgeek.com/DotNet/Tools/Colors/
static const COLOR color_Transparent			= {0x00,0x00,0x00,0x00};    //special one
static const COLOR color_AliceBlue				= {0xF0,0xF8,0xFF,0xFF};
static const COLOR color_AntiqueWhite			= {0xFA,0xEB,0xD7,0xFF};
static const COLOR color_Aqua					= {0x00,0xFF,0xFF,0xFF};
static const COLOR color_Aquamarine				= {0x7F,0xFF,0xD4,0xFF};
static const COLOR color_Azure					= {0xF0,0xFF,0xFF,0xFF};
static const COLOR color_Beige					= {0xF5,0xF5,0xDC,0xFF};
static const COLOR color_Bisque					= {0xFF,0xE4,0xC4,0xFF};
static const COLOR color_Black					= {0x00,0x00,0x00,0xFF};
static const COLOR color_BlanchedAlmond			= {0xFF,0xEB,0xCD,0xFF};
static const COLOR color_Blue					= {0x00,0x00,0xFF,0xFF};
static const COLOR color_BlueViolet				= {0x8A,0x2B,0xE2,0xFF};
static const COLOR color_Brown					= {0xA5,0x2A,0x2A,0xFF};
static const COLOR color_BurlyWood				= {0xDE,0xB8,0x87,0xFF};
static const COLOR color_CadetBlue				= {0x5F,0x9E,0xA0,0xFF};
static const COLOR color_Chartreuse				= {0x7F,0xFF,0x00,0xFF};
static const COLOR color_Chocolate				= {0xD2,0x69,0x1E,0xFF};
static const COLOR color_Coral					= {0xFF,0x7F,0x50,0xFF};
static const COLOR color_CornflowerBlue			= {0x64,0x95,0xED,0xFF};
static const COLOR color_Cornsilk				= {0xFF,0xF8,0xDC,0xFF};
static const COLOR color_Crimson				= {0xDC,0x14,0x3C,0xFF};
static const COLOR color_Cyan					= {0x00,0xFF,0xFF,0xFF};
static const COLOR color_DarkBlue				= {0x00,0x00,0x8B,0xFF};
static const COLOR color_DarkCyan				= {0x00,0x8B,0x8B,0xFF};
static const COLOR color_DarkGoldenrod			= {0xB8,0x86,0x0B,0xFF};
static const COLOR color_DarkGray				= {0xA9,0xA9,0xA9,0xFF};
static const COLOR color_DarkGreen				= {0x00,0x64,0x00,0xFF};
static const COLOR color_DarkKhaki				= {0xBD,0xB7,0x6B,0xFF};
static const COLOR color_DarkMagenta			= {0x8B,0x00,0x8B,0xFF};
static const COLOR color_DarkOliveGreen			= {0x55,0x6B,0x2F,0xFF};
static const COLOR color_DarkOrange				= {0xFF,0x8C,0x00,0xFF};
static const COLOR color_DarkOrchid				= {0x99,0x32,0xCC,0xFF};
static const COLOR color_DarkRed				= {0x8B,0x00,0x00,0xFF};
static const COLOR color_DarkSalmon				= {0xE9,0x96,0x7A,0xFF};
static const COLOR color_DarkSeaGreen			= {0x8F,0xBC,0x8B,0xFF};
static const COLOR color_DarkSlateBlue			= {0x48,0x3D,0x8B,0xFF};
static const COLOR color_DarkSlateGray			= {0x2F,0x4F,0x4F,0xFF};
static const COLOR color_DarkTurquoise			= {0x00,0xCE,0xD1,0xFF};
static const COLOR color_DarkViolet				= {0x94,0x00,0xD3,0xFF};
static const COLOR color_DeepPink				= {0xFF,0x14,0x93,0xFF};
static const COLOR color_DeepSkyBlue			= {0x00,0xBF,0xFF,0xFF};
static const COLOR color_DimGray				= {0x69,0x69,0x69,0xFF};
static const COLOR color_DodgerBlue				= {0x1E,0x90,0xFF,0xFF};
static const COLOR color_Firebrick				= {0xB2,0x22,0x22,0xFF};
static const COLOR color_FloralWhite			= {0xFF,0xFA,0xF0,0xFF};
static const COLOR color_ForestGreen			= {0x22,0x8B,0x22,0xFF};
static const COLOR color_Fuchsia				= {0xFF,0x00,0xFF,0xFF};
static const COLOR color_Gainsboro				= {0xDC,0xDC,0xDC,0xFF};
static const COLOR color_GhostWhite				= {0xF8,0xF8,0xFF,0xFF};
static const COLOR color_Gold					= {0xFF,0xD7,0x00,0xFF};
static const COLOR color_Goldenrod				= {0xDA,0xA5,0x20,0xFF};
static const COLOR color_Gray					= {0x80,0x80,0x80,0xFF};
static const COLOR color_Green					= {0x00,0x80,0x00,0xFF};
static const COLOR color_GreenYellow			= {0xAD,0xFF,0x2F,0xFF};
static const COLOR color_Honeydew				= {0xF0,0xFF,0xF0,0xFF};
static const COLOR color_HotPink				= {0xFF,0x69,0xB4,0xFF};
static const COLOR color_IndianRed				= {0xCD,0x5C,0x5C,0xFF};
static const COLOR color_Indigo					= {0x4B,0x00,0x82,0xFF};
static const COLOR color_Ivory					= {0xFF,0xFF,0xF0,0xFF};
static const COLOR color_Khaki					= {0xF0,0xE6,0x8C,0xFF};
static const COLOR color_Lavender				= {0xE6,0xE6,0xFA,0xFF};
static const COLOR color_LavenderBlush			= {0xFF,0xF0,0xF5,0xFF};
static const COLOR color_LawnGreen				= {0x7C,0xFC,0x00,0xFF};
static const COLOR color_LemonChiffon			= {0xFF,0xFA,0xCD,0xFF};
static const COLOR color_LightBlue				= {0xAD,0xD8,0xE6,0xFF};
static const COLOR color_LightCoral				= {0xF0,0x80,0x80,0xFF};
static const COLOR color_LightCyan				= {0xE0,0xFF,0xFF,0xFF};
static const COLOR color_LightGoldenrodYellow	= {0xFA,0xFA,0xD2,0xFF};
static const COLOR color_LightGray				= {0xD3,0xD3,0xD3,0xFF};
static const COLOR color_LightGreen				= {0x90,0xEE,0x90,0xFF};
static const COLOR color_LightPink				= {0xFF,0xB6,0xC1,0xFF};
static const COLOR color_LightSalmon			= {0xFF,0xA0,0x7A,0xFF};
static const COLOR color_LightSeaGreen			= {0x20,0xB2,0xAA,0xFF};
static const COLOR color_LightSkyBlue			= {0x87,0xCE,0xFA,0xFF};
static const COLOR color_LightSlateGray			= {0x77,0x88,0x99,0xFF};
static const COLOR color_LightSteelBlue			= {0xB0,0xC4,0xDE,0xFF};
static const COLOR color_LightYellow			= {0xFF,0xFF,0xE0,0xFF};
static const COLOR color_Lime					= {0x00,0xFF,0x00,0xFF};
static const COLOR color_LimeGreen				= {0x32,0xCD,0x32,0xFF};
static const COLOR color_Linen					= {0xFA,0xF0,0xE6,0xFF};
static const COLOR color_Magenta				= {0xFF,0x00,0xFF,0xFF};
static const COLOR color_Maroon					= {0x80,0x00,0x00,0xFF};
static const COLOR color_MediumAquamarine		= {0x66,0xCD,0xAA,0xFF};
static const COLOR color_MediumBlue				= {0x00,0x00,0xCD,0xFF};
static const COLOR color_MediumOrchid			= {0xBA,0x55,0xD3,0xFF};
static const COLOR color_MediumPurple			= {0x93,0x70,0xDB,0xFF};
static const COLOR color_MediumSeaGreen			= {0x3C,0xB3,0x71,0xFF};
static const COLOR color_MediumSlateBlue		= {0x7B,0x68,0xEE,0xFF};
static const COLOR color_MediumSpringGreen		= {0x00,0xFA,0x9A,0xFF};
static const COLOR color_MediumTurquoise		= {0x48,0xD1,0xCC,0xFF};
static const COLOR color_MediumVioletRed		= {0xC7,0x15,0x85,0xFF};
static const COLOR color_MidnightBlue			= {0x19,0x19,0x70,0xFF};
static const COLOR color_MintCream				= {0xF5,0xFF,0xFA,0xFF};
static const COLOR color_MistyRose				= {0xFF,0xE4,0xE1,0xFF};
static const COLOR color_Moccasin				= {0xFF,0xE4,0xB5,0xFF};
static const COLOR color_NavajoWhite			= {0xFF,0xDE,0xAD,0xFF};
static const COLOR color_Navy					= {0x00,0x00,0x80,0xFF};
static const COLOR color_OldLace				= {0xFD,0xF5,0xE6,0xFF};
static const COLOR color_Olive					= {0x80,0x80,0x00,0xFF};
static const COLOR color_OliveDrab				= {0x6B,0x8E,0x23,0xFF};
static const COLOR color_Orange					= {0xFF,0xA5,0x00,0xFF};
static const COLOR color_OrangeRed				= {0xFF,0x45,0x00,0xFF};
static const COLOR color_Orchid					= {0xDA,0x70,0xD6,0xFF};
static const COLOR color_PaleGoldenrod			= {0xEE,0xE8,0xAA,0xFF};
static const COLOR color_PaleGreen				= {0x98,0xFB,0x98,0xFF};
static const COLOR color_PaleTurquoise			= {0xAF,0xEE,0xEE,0xFF};
static const COLOR color_PaleVioletRed			= {0xDB,0x70,0x93,0xFF};
static const COLOR color_PapayaWhip				= {0xFF,0xEF,0xD5,0xFF};
static const COLOR color_PeachPuff				= {0xFF,0xDA,0xB9,0xFF};
static const COLOR color_Peru					= {0xCD,0x85,0x3F,0xFF};
static const COLOR color_Pink					= {0xFF,0xC0,0xCB,0xFF};
static const COLOR color_Plum					= {0xDD,0xA0,0xDD,0xFF};
static const COLOR color_PowderBlue				= {0xB0,0xE0,0xE6,0xFF};
static const COLOR color_Purple					= {0x80,0x00,0x80,0xFF};
static const COLOR color_Red					= {0xFF,0x00,0x00,0xFF};
static const COLOR color_RosyBrown				= {0xBC,0x8F,0x8F,0xFF};
static const COLOR color_RoyalBlue				= {0x41,0x69,0xE1,0xFF};
static const COLOR color_SaddleBrown			= {0x8B,0x45,0x13,0xFF};
static const COLOR color_Salmon					= {0xFA,0x80,0x72,0xFF};
static const COLOR color_SandyBrown				= {0xF4,0xA4,0x60,0xFF};
static const COLOR color_SeaGreen				= {0x2E,0x8B,0x57,0xFF};
static const COLOR color_SeaShell				= {0xFF,0xF5,0xEE,0xFF};
static const COLOR color_Sienna					= {0xA0,0x52,0x2D,0xFF};
static const COLOR color_Silver					= {0xC0,0xC0,0xC0,0xFF};
static const COLOR color_SkyBlue				= {0x87,0xCE,0xEB,0xFF};
static const COLOR color_SlateBlue				= {0x6A,0x5A,0xCD,0xFF};
static const COLOR color_SlateGray				= {0x70,0x80,0x90,0xFF};
static const COLOR color_Snow					= {0xFF,0xFA,0xFA,0xFF};
static const COLOR color_SpringGreen			= {0x00,0xFF,0x7F,0xFF};
static const COLOR color_SteelBlue				= {0x46,0x82,0xB4,0xFF};
static const COLOR color_Tan					= {0xD2,0xB4,0x8C,0xFF};
static const COLOR color_Teal					= {0x00,0x80,0x80,0xFF};
static const COLOR color_Thistle				= {0xD8,0xBF,0xD8,0xFF};
static const COLOR color_Tomato					= {0xFF,0x63,0x47,0xFF};
static const COLOR color_Turquoise				= {0x40,0xE0,0xD0,0xFF};
static const COLOR color_Violet					= {0xEE,0x82,0xEE,0xFF};
static const COLOR color_Wheat					= {0xF5,0xDE,0xB3,0xFF};
static const COLOR color_White					= {0xFF,0xFF,0xFF,0xFF};
static const COLOR color_WhiteSmoke				= {0xF5,0xF5,0xF5,0xFF};
static const COLOR color_Yellow					= {0xFF,0xFF,0x00,0xFF};
static const COLOR color_YellowGreen			= {0x9A,0xCD,0x32,0xFF};

//debug text style
static const STYLE debug_style = {NULL, {0xFF,0xFF,0xFF,0xFF}, {0x00,0x00,0x00,0xFF}};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/// @brief	screen buffer must be inited before you call this function
void StdDraw_Init(U8* screenbuffer_ptr, SIZE screensize, EVENT_CALLBACK flush_callback);

/// @brief	calls the flush function, this actually moves all the data to the screen, if this function is never called nothing will be shown on the screen :)
void StdDraw_Flush(void);

/// @brief	clears the entire buffer
void StdDraw_Clear(void);

/// @brief	clears a region in the buffer
void StdDraw_ClearRegion(REGION region);

/// @brief	fills a region in the buffer with a color
void StdDraw_FillRegion(REGION region, COLOR color);

/// @brief	calculates the intersection between 2 regions.
/// @param "intersect_result_ptr" pointer to a valid region struct where the intersection info will be placed, the intersect_result_ptr will always be filled in!
/// @return TRUE if region 1 & 2 intersect, FALSE if they do not
BOOL StdDraw_CalculateIntersection(REGION region1, REGION region2, REGION* intersect_result_ptr);

/// @brief	draws an image @ location in the buffer
void StdDraw_DrawImage(POINT location, IMAGE image);

/// @brief	draws a specific region of a image at the location, where the location is the origin of the image
void StdDraw_DrawImageCropped(POINT location, IMAGE image, REGION crop_info);

/// @brief	draws a text @ location in the screen buffer
/// @remark if the style is = "debug_style" then a low flash cost font debug font is used (background color is black and front color is white)
void StdDraw_DrawText(POINT location, STYLE style, STRING text);

/// @brief	move a region in the display buffer to a new target location
void StdDraw_MoveRegion(REGION source_region, POINT target_location);
//================================================================================================//



//================================================================================================//
// REVERSE COMPATIBLE DEFINITION LIST
//------------------------------------------------------------------------------------------------//
#define StdDraw_DrawImageRegion(location, image, region)   	StdDraw_DrawImageCropped(location, image, region)

#define IMAGE_FORMAT_MONOCHROME								IMAGE_FORMAT_MONO
#define IMAGE_FORMAT_GRAYSCALE8    							IMAGE_FORMAT_GRAY8
#define IMAGE_FORMAT_RGB8_R3G3B2   							IMAGE_FORMAT_RGB332
#define IMAGE_FORMAT_RGB16_R5G6B5  							IMAGE_FORMAT_RGB565
//================================================================================================//

#endif /* DRAW__STDDRAW_H */
