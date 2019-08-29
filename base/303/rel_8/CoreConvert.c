//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module for converting data
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORECONVERT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORECONVERT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CORECONVERT_LOG_LEVEL
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
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL CoreConvert_IsCharacterNumber(U8 character);
static void CoreConvert_UnsignedIntegerToBinString(U32 in, STRING out, BOOL prefix, BOOL nibbleSplit,U8 integerLength);
static void CoreConvert_UnsignedIntegerToHexString(U32 in, STRING out, BOOL prefix, BOOL leading_zeros,U8 integerLength);
static void CoreConvert_UnsignedIntegerToDecimalString(U32 in, STRING out, BOOL leading_zeros,U8 integerLength);
static void CoreConvert_SignedIntegerToDecimalString(S32 in, STRING out, U8 integerLength);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL CoreConvert_IsCharacterNumber(U8 character)
{
    if (character >= '0' && character <= '9')
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
static void CoreConvert_UnsignedIntegerToBinString(U32 in, STRING out, BOOL prefix, BOOL nibbleSplit,U8 integerLength)
{
	U8	i;
	U8  extra = 0;

	if (prefix)
	{
		//add 0xb prefix
	   	out[0] = '0';
		out[1] = 'b';
		extra = 2;
	}

	for (i = 0; i<integerLength;i++)
	{
		if (i%4 == 0 && i != 0  && nibbleSplit)
		{
			out[i + extra] = '_';//add _ every 4 bits to improve readability
			extra++;
		}

		if (in & (1 << (integerLength-1-i)))
		{
			out[i+ extra] = '1';
		}
		else
		{
		   	out[i+ extra] = '0';
		}

	}
   	out[i+ extra] = 0;  //null terminate string
}
//------------------------------------------------------------------------------------------------//
static void CoreConvert_UnsignedIntegerToHexString(U32 in, STRING out, BOOL prefix, BOOL leading_zeros,U8 integerLength)
{
    U8 temp;
    U8 i;
    U8 string_position = 0;
    U8 first_meaningfull_digit_passed = FALSE;

    U8 totalStringDigits = integerLength/4;

    if (prefix)
    {
        out[0] = '0';
        out[1] = 'x';
        string_position = 2;
    }

    for(i = 0;i<totalStringDigits;i++)
    {
        temp = (in >>(4*(totalStringDigits-1-i))) & 0xF;

        if(temp == 0 && i != (totalStringDigits-1) && first_meaningfull_digit_passed == FALSE) //never do this on lsb number or if a meaningfull digit has passed allready
        {
            if (leading_zeros)
            {
                out[string_position] = '0';
                string_position++;
            }
        }
        else if(temp < 10)
        {
            first_meaningfull_digit_passed = TRUE;
            out[string_position] = '0' + temp;
            string_position++;
        }
        else
        {
            first_meaningfull_digit_passed = TRUE;
            out[string_position] = 'A' + temp - 10;
            string_position++;
        }
    }

    out[string_position] = 0; //null terminate string
}
//------------------------------------------------------------------------------------------------//
static void CoreConvert_UnsignedIntegerToDecimalString(U32 in, STRING out, BOOL leading_zeros,U8 integerLength)
{
    S8 i = 0;
    U8 tempCharBuffer[10];
    S8 numberOfPaddingChars = 0;

    //fill in temp char buffer with ascii characters (reversed)
    do
    {
        tempCharBuffer[i++] = '0' + in%10;
        in = in/10;
    }
    while (in);

    if (leading_zeros)
    {
        if (integerLength == 8)
        {
            numberOfPaddingChars = 3-i;
        }
        else if (integerLength == 16)
        {
            numberOfPaddingChars = 5-i;
        }
        else
        {
            numberOfPaddingChars = 10-i;
        }
        while(numberOfPaddingChars>0)
        {
            *out = '0';//pad with zero's
            out++;
            numberOfPaddingChars--;
        }
    }

    //copy buffer in to string
    while(i>0)
    {
        *out = tempCharBuffer[i-1];
        out++;
        i--;
    }

    //null terminate string
    *out++ = 0;
}
//------------------------------------------------------------------------------------------------//
static void CoreConvert_SignedIntegerToDecimalString(S32 in, STRING out, U8 integerLength)
{
    if (in >= 0)
    {
        CoreConvert_UnsignedIntegerToDecimalString(in,out,FALSE,integerLength);
    }
    else
    {
        out[0] = '-';
        CoreConvert_UnsignedIntegerToDecimalString(in*-1,&out[1],FALSE,integerLength);
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
U32 CoreConvert_DecimalStringToU32(STRING in)
{
	U32 result = 0;

    //Find first digit in case the string does contains letters also
	while(!CoreConvert_IsCharacterNumber(*in))
	{
		if(*in == 0) return 0;
		in++;
	}
    //keep on decoding while there are numbers
	while(CoreConvert_IsCharacterNumber(*in))
	{
		result *= 10;
		result += (*in - '0');
		in++;
	}
	return result;
}
//------------------------------------------------------------------------------------------------//
U32 CoreConvert_Ipv4StringToU32(STRING in)
{
	U32 result = 0;
    U8 temp = 0;
    U8 shift = 0;

    //Find first digit in case the string does contains letters also
	while(!CoreConvert_IsCharacterNumber(*in))
	{
		if(*in == 0) return 0;
		in++;
	}
    //decode 4 sets of digits
    for (shift = 0; shift < 32; shift += 8)
    {
        temp = 0;
        while(CoreConvert_IsCharacterNumber(*in))
        {
            temp *= 10;
            temp += (*in - '0');
            in++;
        }
        result += temp << (24 - shift);

        if (*in != '.')
            break;
        in++;
    }
	return result;
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U8ToDecimalString(U8 in, STRING out, BOOL leading_zeros)
{
    CoreConvert_UnsignedIntegerToDecimalString(in, out, leading_zeros,8);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U16ToDecimalString(U16 in, STRING out, BOOL leading_zeros)
{
    CoreConvert_UnsignedIntegerToDecimalString(in, out, leading_zeros,16);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U32ToDecimalString(U32 in, STRING out, BOOL leading_zeros)
{
    CoreConvert_UnsignedIntegerToDecimalString(in, out, leading_zeros,32);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U8ToBinString(U8 in, STRING out, BOOL prefix, BOOL nibbleSplit)
{
	CoreConvert_UnsignedIntegerToBinString(in, out, prefix, nibbleSplit,8);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U16ToBinString(U16 in, STRING out, BOOL prefix, BOOL nibbleSplit)
{
	CoreConvert_UnsignedIntegerToBinString(in, out, prefix, nibbleSplit,16);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U32ToBinString(U32 in, STRING out, BOOL prefix, BOOL nibbleSplit)
{
	CoreConvert_UnsignedIntegerToBinString(in, out, prefix, nibbleSplit,32);
}
//------------------------------------------------------------------------------------------------//
U8 CoreConvert_HexCharToU8(CHAR in)
{
    if((in >= '0') && (in <= '9'))
    {
        return (in - '0');
    }
    else if((in >= 'a') && (in <= 'f'))
    {
        return (in - 'a' + 10);
    }
    else if((in >= 'A') && (in <= 'F'))
    {
        return (in - 'A' + 10);
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U32 CoreConvert_HexStringToU32(STRING in)
{
    CHAR* ptr_ptr;
    U32 value  = 0;

    ptr_ptr = &in[2]; //dispose of "0x"
    for(;;)
    {
        if((*ptr_ptr >= '0') && (*ptr_ptr <= '9'))
        {
            value <<= 4;
            value += (*ptr_ptr) - '0';
        }
        else if((*ptr_ptr >= 'a') && (*ptr_ptr <= 'f'))
        {
            value <<= 4;
            value += (*ptr_ptr) - 'a' + 10;
        }
        else if((*ptr_ptr >= 'A') && (*ptr_ptr <= 'F'))
        {
            value <<= 4;
            value += (*ptr_ptr) - 'A' + 10;
        }
        else
        {
            return value;
        }
        ptr_ptr++;
    }
}
//------------------------------------------------------------------------------------------------//
U32 CoreConvert_StringToU32(STRING in)
{
	if(in[1] == 'x')
    {
        return CoreConvert_HexStringToU32(in);
    }
	else if(in[1] == 'b')
    {
        return CoreConvert_BinStringToU32(in);
    }
    else if (CoreConvert_IsCharacterNumber(in[0])) //if it is a number do conversion, else try boolean
    {
        return CoreConvert_DecimalStringToU32(in);
    }
    else
    {
        return (U32) CoreConvert_BoolStringToBool(in);
    }
}
//------------------------------------------------------------------------------------------------//
S32 CoreConvert_StringToS32(STRING in)
{
    if(in[0] == '-')
    {
    	return -(S32)CoreConvert_StringToU32(&in[1]);
    }
    else if(in[0] == '+')
    {
    	return (S32)CoreConvert_StringToU32(&in[1]);
    }
    else
    {
    	return (S32)CoreConvert_StringToU32(in);
    }
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_BoolToBoolString(BOOL in, STRING out)
{
	STRING str_true = "TRUE";
	STRING str_false = "FALSE";
	U8 i = 0;

	if (in == TRUE)
	{
		while(str_true[i] != 0)
		{
			out[i] = str_true[i];
			i++;
		}
	}
	else
	{
		while(str_false[i] != 0)
		{
			out[i] = str_false[i];
			i++;
		}
	}
	out[i] = 0;	// null termination

}
//------------------------------------------------------------------------------------------------//
BOOL CoreConvert_BoolStringToBool(STRING in)
{
    //if true or TRUE or treu or High or hgh or 1, it will all reconise it as true,check only first letter
	if ((in[0] == 'E')||(in[0] == 'e')||    //enable,ENABLE,enablle, ...
        (in[0] == 'T')||(in[0] == 't')||    //True,TRUE,treu,..
        (in[0] == 'H')||(in[0] == 'h'))    //High,HIGH,hgh,...
	{
	    return TRUE;
	}
	else if (CoreConvert_IsCharacterNumber(in[0]))   //if number try number
	{
        if (CoreConvert_StringToU32(in)>0)
        {
            return TRUE;
        }
	}
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U8ToHexString(U8 in, STRING out, BOOL prefix, BOOL leading_zeros)
{
    CoreConvert_UnsignedIntegerToHexString(in,out,prefix,leading_zeros,8);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U16ToHexString(U16 in, STRING out, BOOL prefix, BOOL leading_zeros)
{
    CoreConvert_UnsignedIntegerToHexString(in,out,prefix,leading_zeros,16);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U32ToHexString(U32 in, STRING out, BOOL prefix, BOOL leading_zeros)
{
    CoreConvert_UnsignedIntegerToHexString(in,out,prefix,leading_zeros,32);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U16ToCommaString(U16 in, STRING out, U8 digits_after_comma)
{
    U8 digits = 0;

    U8 i = 0;
    U8 j = 0;
    U8 k = 0;

    CoreConvert_U16ToDecimalString(in,out,FALSE);//convert first to normal string

    if (digits_after_comma > 0)
    {
        for(digits = 0;out[digits]!=0;digits++){} //check how many digits it has without comma

        if(digits <= digits_after_comma)//if there are to few digits for comma number
        {
            k = digits_after_comma + 1 +1; //+ 1 null before comma
            j = digits +1;
            while(k > 0)
            {
                if (j > 0)
                {
                    out[k-1] = out[j-1]; //shift chars right
                    j--;
                }
                else
                {
                    out[k-1] = '0'; // pad left with zeros
                }
                k--;

            }
            digits = digits_after_comma + 1; //if extra zeors have been added then the new number of difernt is changed
        }

        i = digits+1;
        while(j < digits_after_comma+1)
        {
            out[i] = out[i-1]; //shift char further to make place for ','
            i--;
            j++;
        }
        out[i] = ','; //place comma
    }
}
//------------------------------------------------------------------------------------------------//
U32 CoreConvert_BinStringToU32(STRING in)
{
    CHAR* ptr_ptr;
    U32 value  = 0;

    ptr_ptr = &in[2]; //dispose of "0b"
    for(;;)
    {
        if((*ptr_ptr >= '0') && (*ptr_ptr <= '1'))
        {
            value <<= 1;
            value += (*ptr_ptr) - '0';
        }
        else if(*ptr_ptr == '_') //ignore underscores
        {
        }
        else //some other character reached, stop for loop and return
        {
            return value;
        }
        ptr_ptr++;
    }
}
//------------------------------------------------------------------------------------------------//
S32 CoreConvert_DecimalStringToS32(STRING in)
{
    S32 value;

    if(in[0] == '-')
    {
    	value = -(S32)CoreConvert_DecimalStringToU32(&in[1]);
    }
    else
    {
    	value = (S32)CoreConvert_DecimalStringToU32(in);
    }
	return value;
}
//------------------------------------------------------------------------------------------------//
U8 CoreConvert_CharTo7segmentU8(char in)
{
    // 7 segment bits are in following sequence ".gfedcba"
    //     A  A  A
    //    F       B
    //    F       B
    //    F       B
    //     G  G  G
    //    E       C
    //    E       C
    //    E       C
    //     D  D  D   .

    switch(in)
    {
        case 'A' :
        case 'a' : return 0x77; //A
        case 'B' :
        case 'b' : return 0x7C; //b
        case 'C' : return 0x39; //C
        case 'c' : return 0x58; //c
        case 'D' :
        case 'd' : return 0x5E; //d
        case 'E' :
        case 'e' : return 0x79; //E
        case 'F' :
        case 'f' : return 0x71; //F
        case 'G' :
        case 'g' : return 0x3D; //g
        case 'H' :
        case 'h' : return 0x74; //h
        case 'I' : return 0x06; //I
        case 'i' : return 0x10; //i
        case 'J' :
        case 'j' : return 0x1E; //j
        case 'K' :
        case 'k' : return 0x75; //k
        case 'L' :
        case 'l' : return 0x38; //L
        case 'M' :
        case 'm' : return 0x55; //m
        case 'N' :
        case 'n' : return 0x54; //n
        case 'O' :
        case 'o' : return 0x5C; //o
        case 'P' :
        case 'p' : return 0x73; //P
        case 'Q' :
        case 'q' : return 0x67; //q
        case 'R' :
        case 'r' : return 0x50; //r
        case 'S' :
        case 's' : return 0x6D; //S
        case 'T' :
        case 't' : return 0x78; //t
        case 'U' :
        case 'u' : return 0x1C; //u
        case 'V' :
        case 'v' : return 0x2A; //v
        case 'W' :
        case 'w' : return 0x6A; //w
        case 'X' :
        case 'x' : return 0x49; //x
        case 'Y' :
        case 'y' : return 0x6E; //y
        case 'Z' :
        case 'z' : return 0x52; //z
        case ' ' : return 0x00; //space
        case '_' : return 0x08; //_
        case '0' : return 0x3F; //0
        case '1' : return 0x06; //1
        case '2' : return 0x5B; //2
        case '3' : return 0x4F; //3
        case '4' : return 0x66; //4
        case '5' : return 0x6D; //5
        case '6' : return 0x7D; //6
        case '7' : return 0x07; //7
        case '8' : return 0x7F; //8
        case '9' : return 0x6F; //9
        case '.' : return 0x80; //.
        default  : return 0x00; //space
    }
}
//------------------------------------------------------------------------------------------------//
U16 CoreConvert_U8ArrayToU16(U8* data_ptr)
{
    return (((data_ptr[0]) << 8) | (data_ptr[1] & 0xFF));
}
//------------------------------------------------------------------------------------------------//
U32 CoreConvert_U8ArrayToU32(U8* data_ptr)
{
    return ((((U32)data_ptr[0]) << 24) |
            (((U32)data_ptr[1]) << 16) |
            (((U32)data_ptr[2]) << 8)  |
            ((U32)data_ptr[3]) & 0xFF);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U16ToU8Array(U16 in, U8* data_ptr)
{
    data_ptr[0] = (in >>  8) & 0x00FF;
    data_ptr[1] = (in >>  0) & 0x00FF;
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U32ToU8Array(U32 in, U8* data_ptr)
{
    data_ptr[0] = (in >> 24) & 0x00FF;
    data_ptr[1] = (in >> 16) & 0x00FF;
    data_ptr[2] = (in >>  8) & 0x00FF;
    data_ptr[3] = (in >>  0) & 0x00FF;
}
//------------------------------------------------------------------------------------------------//
U16 CoreConvert_Pt1024U16ToDegreeU16(U16 in)
{
    return (U16)((in * 3600) >> 10);
}
//------------------------------------------------------------------------------------------------//
U16 CoreConvert_DegreeU16ToPt1024U16(U16 in)
{
    return (U16)((in << 10)/ 3600);
}
//------------------------------------------------------------------------------------------------//
COLOR_RGB CoreConvert_HsvToRgb(COLOR_HSV hsv)
{
    //code found at http://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both    - fins answer
    //hsv komt binnen als 0-360,0-100,0-100 en dit algoritme verwacht 0-255,0-255,0-255 dus zet om
    U8 hsv_255_hue = (U8)((((F32)hsv.hue)/360)*255);
    U8 hsv_255_saturation = (U8)(((F32)hsv.saturation/100)*255);
    U8 hsv_255_value = (U8)(((F32)hsv.value/100)*255);
    COLOR_RGB rgb;
    U8 region, remainder, p, q, t;

    if (hsv_255_saturation == 0)
    {
        rgb.red = hsv_255_value;
        rgb.green = hsv_255_value;
        rgb.blue = hsv_255_value;
        return rgb;
    }

    region = hsv_255_hue / 43;
    remainder = (hsv_255_hue - (region * 43)) * 6;

    p = (hsv_255_value * (255 - hsv_255_saturation)) >> 8;
    q = (hsv_255_value * (255 - ((hsv_255_saturation * remainder) >> 8))) >> 8;
    t = (hsv_255_value * (255 - ((hsv_255_saturation * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.red = hsv_255_value; rgb.green = t; rgb.blue = p;
            break;
        case 1:
            rgb.red = q; rgb.green = hsv_255_value; rgb.blue = p;
            break;
        case 2:
            rgb.red = p; rgb.green = hsv_255_value; rgb.blue = t;
            break;
        case 3:
            rgb.red = p; rgb.green = q; rgb.blue = hsv_255_value;
            break;
        case 4:
            rgb.red = t; rgb.green = p; rgb.blue = hsv_255_value;
            break;
        default:
            rgb.red = hsv_255_value; rgb.green = p; rgb.blue = q;
            break;
    }

    return rgb;
}
//------------------------------------------------------------------------------------------------//
COLOR_HSV CoreConvert_RgbToHsv(COLOR_RGB rgb)
{
    COLOR_HSV hsv;
    U8 hsv_255_hue;
    U8 hsv_255_saturation;
    U8 hsv_255_value;
    U8 rgbMin, rgbMax;

    rgbMin = rgb.red < rgb.green ? (rgb.red < rgb.blue ? rgb.red : rgb.blue) : (rgb.green < rgb.blue ? rgb.green : rgb.blue);
    rgbMax = rgb.red > rgb.green ? (rgb.red > rgb.blue ? rgb.red : rgb.blue) : (rgb.green > rgb.blue ? rgb.green : rgb.blue);

    hsv_255_value = rgbMax;
    if (hsv_255_value == 0)
    {
        hsv_255_hue = 0;
        hsv_255_saturation = 0;
    }
    else
    {
        long temp = rgbMax - rgbMin;
        hsv_255_saturation = 255 * temp / hsv_255_value;
        if (hsv_255_saturation == 0)
        {
            hsv_255_hue = 0;
        }
        else
        {
            if (rgbMax == rgb.red)
            {
                hsv_255_hue = 0 + 43 * (rgb.green - rgb.blue) / (rgbMax - rgbMin);
            }
            else if (rgbMax == rgb.green)
            {
                hsv_255_hue = 85 + 43 * (rgb.blue - rgb.red) / (rgbMax - rgbMin);
            }
            else
            {
                hsv_255_hue = 171 + 43 * (rgb.red - rgb.green) / (rgbMax - rgbMin);
            }
        }
    }

    //hsv komt eruit 0-255,0-255,0-255 en de rest van de code verwacht 0-360,0-100,0-100 dus zet om
    hsv.hue = (U16)((((F32)hsv_255_hue)/255)*360);
    hsv.saturation = (U8)(((F32)hsv_255_saturation/255)*100);
    hsv.value = (U8)(((F32)hsv_255_value/255)*100);

    //failsafes
    if (hsv.hue >360)
    {
        hsv.hue = 360;
    }
    if (hsv.saturation >100)
    {
        hsv.saturation = 100;
    }
    if (hsv.value > 100)
    {
        hsv.value = 100;
    }

    return hsv;
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_S8ToDecimalString(S8 in, STRING out)
{
    CoreConvert_SignedIntegerToDecimalString(in, out, 8);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_S16ToDecimalString(S16 in, STRING out)
{
    CoreConvert_SignedIntegerToDecimalString(in, out, 16);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_S32ToDecimalString(S32 in, STRING out)
{
    CoreConvert_SignedIntegerToDecimalString(in, out, 32);
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_U8ArryToHexString(U8* in, U8 arry_length, STRING out)
{
    U8 i;
    for(i = 0; i < arry_length; i++)
    {
        CoreConvert_U8ToHexString(in[i], out, FALSE, TRUE);
        out[2] = ' ';
        //out[3] = 0;
        out+=3;
    }
    out[-1] = 0;
}
//------------------------------------------------------------------------------------------------//
void CoreConvert_FloatToDecimalString(F32 in, STRING out, U8 digits_after_comma)
{
    //TODO this function needs better testing, what will happen with very big numbers ? option show exponent needed ? ect, it can also be optimised

    U8 i;
    for(i=0;i<digits_after_comma;i++)
    {
        in*=10;
    }
    CoreConvert_U16ToCommaString((U16)in,out,digits_after_comma);
}
//================================================================================================//
