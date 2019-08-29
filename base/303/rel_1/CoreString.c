//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This entity bundles basic STRING operations
// These bundled basic STRING operations are grouped together in a DRIVER entity because of the different
// representations of strings on the different platforms.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORESTRING_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORESTRING_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CORESTRING_LOG_LEVEL
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
U8 CoreString_GetLength(STRING string)
{
    STRING base = string;
    while(*string != 0)
    {
        string++;
    }
    return (U8)(string - base);
}
//------------------------------------------------------------------------------------------------//
BOOL CoreString_Equals(STRING string1, STRING string2)
{
    while((*string1 != 0) && (*string1 == *string2))
    {
        string1++;
        string2++;
    }
    return (BOOL)(*string1 == *string2);
}
//------------------------------------------------------------------------------------------------//
BOOL CoreString_StartsWith(STRING string1, STRING string2)
{
    while((*string2 != 0) && (*string1 == *string2))
    {
        string1++;
        string2++;
    }
    return (BOOL)(*string2 == 0);
}
//------------------------------------------------------------------------------------------------//
void CoreString_ToLowerCase(STRING string)
{
    while(*string != 0)
    {
		if(*string >= 'A' && *string <= 'Z')
		{
			*string += 'a' - 'A';
		}
		string++;
    }
}
//------------------------------------------------------------------------------------------------//
void CoreString_ToUpperCase(STRING string)
{
    while(*string != 0)
    {
		if(*string >= 'a' && *string <= 'z')
		{
			*string += 'A' - 'a';
		}
		string++;
    }
}
//------------------------------------------------------------------------------------------------//
U8 CoreString_SearchChar(STRING string, CHAR character)
{
    STRING base = string;
    while(*string != 0)
    {
        if(*string == character)
        {
            return (U8)(string - base);
        }
        string++;
    }
	return 255;
}
//------------------------------------------------------------------------------------------------//
U8 CoreString_CountChar(STRING string, CHAR character)
{
    U8 i = 0;
    while(*string != 0)
    {
        if(*string == character)
        {
            i++;
        }
        string++;
    }
    return i;
}
//------------------------------------------------------------------------------------------------//
void CoreString_PadEnd(STRING string, U8 length, CHAR character)
{
    U8  string_len = CoreString_GetLength(string);
    if(string_len < length)
    {
        MEMSET((VPTR)(string + string_len), character, length - string_len);
    }
    string[length] = 0;
}
//------------------------------------------------------------------------------------------------//
void CoreString_PadStart(STRING string, U8 length, CHAR character)
{
    U8  string_len = CoreString_GetLength(string);
    STRING      string1 = &string[string_len - 1];
    STRING      string2 = &string[length - 1];
    if(string_len < length)
    {
        while(string1 >= string)
        {
            *string2-- = *string1--;
        }
        MEMSET((VPTR)string, character, length - string_len);
    }
    string[length] = 0;
}
//------------------------------------------------------------------------------------------------//
void CoreString_CopyString(STRING source_string, STRING destination_string)
{
    while(*source_string != 0)
    {
        *destination_string++ = *source_string++;
    }
    *destination_string = 0;
}
//------------------------------------------------------------------------------------------------//
void CoreString_Concat(STRING string1, STRING string2)
{
    CoreString_CopyString(string2, string1 + CoreString_GetLength(string1));
}
//------------------------------------------------------------------------------------------------//
void CoreString_SubString(STRING source_string, STRING destination_string, U8 start_index, U8 length)
{
    U8  string_len;
    if(length == 0)
    {
        CoreString_CopyString(source_string + start_index, destination_string);
    }
    else
    {
        string_len = CoreString_GetLength(source_string);
        if((string_len - start_index) < length)
        {
            length = string_len - start_index;
        }
        MEMCPY((VPTR)destination_string, (VPTR)(source_string + start_index), length);
        destination_string[length] = 0;
    }
}
//------------------------------------------------------------------------------------------------//
U8 CoreString_Search(STRING source_string, STRING search_string)
{
    STRING  base = source_string;
    while(*source_string != 0)
    {
        if(CoreString_StartsWith(source_string, search_string))
        {
            return (U8)(source_string - base);
        }
        source_string++;
    }
    return 255;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreString_Contains(STRING source_string, STRING search_string)
{
    return (BOOL)(CoreString_Search(source_string,search_string) != 255);
}
//------------------------------------------------------------------------------------------------//
BOOL CoreString_IsAlphabeticBefore(STRING string1, STRING string2)
{
    while((*string1 != 0) && (*string2 != 0) && (*string1 == *string2))
    {
        string1++;
        string2++;
    }
    return (BOOL)(*string1 < *string2);
}
//------------------------------------------------------------------------------------------------//
void CoreString_Clear(STRING string, U8 length)
{
    MEMSET((VPTR)string, 0, length);
}
//------------------------------------------------------------------------------------------------//
BOOL CoreString_IsDigit(CHAR character)
{
    return (BOOL)((character >= '0') && (character <= '9'));
}
//------------------------------------------------------------------------------------------------//
void CoreString_Trim(STRING string)
{
    CoreString_TrimStart(string);
    CoreString_TrimEnd(string);
}
//------------------------------------------------------------------------------------------------//
void CoreString_TrimStart(STRING string)
{
    U8 i;
    for(i = 1; i <= 32; i++)
    {
        CoreString_TrimStartChar(string,(CHAR)i);
    }
}
//------------------------------------------------------------------------------------------------//
void CoreString_TrimEnd(STRING string)
{
    U8 i;
    for(i = 1; i <= 32; i++)
    {
        CoreString_TrimEndChar(string,(CHAR)i);
    }
}
//------------------------------------------------------------------------------------------------//
void CoreString_TrimChar(STRING string, CHAR trim_char)
{
    CoreString_TrimStartChar(string, trim_char);
    CoreString_TrimEndChar(string, trim_char);
}
//------------------------------------------------------------------------------------------------//
void CoreString_TrimStartChar(STRING string, CHAR trim_char)
{
    U8 index = 0;
    U8 shift_count = 0;
    U8 length = CoreString_GetLength(string);

    //zoek eerst hoeveel trim chars er vooraanstaan
    while(index < length)
    {
        if (string[index] == trim_char)   //indien eerste char == trim char, vervang het door null char
        {
            shift_count++;
        }
        else
        {
            break;
        }
        index++;
    }

    //shift daarna alles naar voor (dus deleting de trim chars)
    index = 0;
    while(index <= length - shift_count + 1 ) //+1 omdat je de null char ook wilt kopieeren
    {
        string[index] = string[index + shift_count];
        index++;
    }
}
//------------------------------------------------------------------------------------------------//
void CoreString_TrimEndChar(STRING string, CHAR trim_char)
{
    U8 i = CoreString_GetLength(string);    //start met lengte + 1(null char)
    while(i) //zolang er nog chars zijn
    {
        if (string[i-1] == trim_char)   //indien laatste char == trim char, vervang het door null char
        {
            string[i-1] = 0;
        }
        else
        {
            return; //laatste char is niet meer hetzelfde, exit function
        }
        i--;
    }
}
//------------------------------------------------------------------------------------------------//
void CoreString_RemoveExcessiveChar(STRING string, CHAR excessive_char)
{
    U8 index = 0;
    U8 length = CoreString_GetLength(string);
    BOOL first_excessive_char = TRUE;
    U8 shift_length;
    U8 i;

    //zoek eerst hoeveel trim chars er vooraanstaan
    while(index < length)
    {
        if (string[index] == excessive_char) //excessive_char found
        {
            if (first_excessive_char)  //first reduce char, do nothing
            {
                first_excessive_char = FALSE;
                index++;
            }
            else
            {
                //exessive excessive_char found, shift all the rest 1 to the left
                shift_length = length - index;
                for(i = 0; i < shift_length; i++)
                {
                    string[index+i] = string[index+i+1];
                }
                //adapt length because you shortend the string by one
                length--;
            }
        }
        else
        {
            first_excessive_char = TRUE;   //reset to true
            index++;
        }
    }
}
//================================================================================================//
