//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This entity bundles basic STRING operations
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORESTRING_H
#define CORESTRING_H
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//STRING INFO FUNCTIONS
//------------------------------------------------------------------------------------------------//
// @remark  returns the string length
U8 CoreString_GetLength(STRING string);

// @remark  returns TRUE if string 1 starts with string 2
BOOL CoreString_StartsWith(STRING string1, STRING string2);

// @remark  returns TRUE if the search string is found within the source string
BOOL CoreString_Contains(STRING source_string, STRING search_string);

// @remark  returns TRUE if both strings are identical
BOOL CoreString_Equals(STRING string1, STRING string2);

// @remark  returns TRUE if string1 comes before string2 in an alphabetically sorted list
BOOL CoreString_IsAlphabeticBefore(STRING string1, STRING string2);

//------------------------------------------------------------------------------------------------//
//STRING EDIT FUNCTIONS
//------------------------------------------------------------------------------------------------//
// @remark  clears the string
void CoreString_Clear(STRING string, U8 length);

// @remark  converts the string to lower case
void CoreString_ToLowerCase(STRING string);

// @remark  converts the string to upper case
void CoreString_ToUpperCase(STRING string);

// @remark  pad the string on the start to the specified length with pad character; make sure the string has enough space
void CoreString_PadStart(STRING string, U8 length, CHAR pad_character);

// @remark  pad the string on the end to the specified length with pad character; make sure the string has enough space
void CoreString_PadEnd(STRING string, U8 length, CHAR pad_character);

// @remark  Removes all leading and trailing non-visible characters + spaces from the string
void CoreString_Trim(STRING string);

// @remark  Removes all leading non-visible characters + spaces from the string
void CoreString_TrimStart(STRING string);

// @remark  Removes all trailing non-visible characters + spaces from the string
void CoreString_TrimEnd(STRING string);

// @remark  Removes all leading and trailing occurrences of the specific trim_char
void CoreString_TrimChar(STRING string, CHAR trim_char);

// @remark  Removes all leading occurrences of the specific trim_char
void CoreString_TrimStartChar(STRING string, CHAR trim_char);

// @remark  Removes all trailing occurrences of the specific trim_char
void CoreString_TrimEndChar(STRING string, CHAR trim_char);

// @remark  Removes excessive chars that follow eachother example"testt1" wil become "test1", most used to remove double spaces
void CoreString_RemoveExcessiveChar(STRING string, CHAR excessive_char);

// @remark  copy the string to another string; make sure the destination string has enough space
void CoreString_CopyString(STRING source_string, STRING destination_string);

//------------------------------------------------------------------------------------------------//
//STRING CONCAT/SUBSTRING FUNCTIONS
//------------------------------------------------------------------------------------------------//
// @remark  concatenate string 1 with string 2, result will be placed at string1; make sure that string1 has enough space
void CoreString_Concat(STRING string1, STRING string2);

// @remark  select substring from source_string; make sure the destination string has enough space
//          when length is 0, substring is taken till end of source string
void CoreString_SubString(STRING source_string, STRING destination_string, U8 start_index, U8 length);

//------------------------------------------------------------------------------------------------//
//SEARCH/COUNT FUNCTIONS
//------------------------------------------------------------------------------------------------//
// @remark  finds the first occurence of a search string within te source string, returns 255 if not found
U8 CoreString_Search(STRING source_string, STRING search_string);

// @remark  finds the first occurence of a character in a string, returns 255 if not found
U8 CoreString_SearchChar(STRING string, CHAR search_character);

// @remark  returns the number of occurences that the count_character is found in the string
U8 CoreString_CountChar(STRING string, CHAR count_character);

// @remark  finds the last occurence of a character in a string, returns 255 if not found
U8 CoreString_SearchLastChar(STRING string, CHAR search_character);
//------------------------------------------------------------------------------------------------//
//CHARACTER FUNCTIONS
//------------------------------------------------------------------------------------------------//
// @remark  returns true if character is a digit
BOOL CoreString_IsDigit(CHAR character);

//------------------------------------------------------------------------------------------------//
//FUTURE FUNCTIONS - NOT IMPLEMENTED YET - NAMES MAY STILL CHANGE
//------------------------------------------------------------------------------------------------//
// @remark  returns TRUE if string 1 ends with string 2
BOOL CoreString_EndsWith(STRING string1, STRING string2);

// @remark  Inserts a string in another string at specific index; make sure that target_string has enough space
void CoreString_Insert(STRING target_string, STRING insert_string, U8 index);

// @remark  Inserts a char in another string at specific index; make sure that target_string has enough space
void CoreString_InsertChar(STRING target_string, CHAR insert_char, U8 index);

// @remark  Replaces all old strings by a new strings; make sure that target_string has enough space
void CoreString_Replace(STRING target_string, STRING old_string, STRING new_string);

// @remark  Replaces all old chars by a new chars
void CoreString_ReplaceChar(STRING target_string, CHAR old_char, CHAR new_char);

// @remark  Removes all occurrences of remove_string inside the target string
void CoreString_Remove(STRING target_string, STRING remove_string);

// @remark  Removes all occurrences of remove_char inside the target string
void CoreString_RemoveChar(STRING target_string, CHAR remove_char);

// @remark  returns the number of occurences that the count_string is found in the string
U8 CoreString_Count(STRING string, STRING count_string);

// @remark  finds the last occurence of a search string within te source string, returns 255 if not found
U8 CoreString_SearchLast(STRING source_string, STRING search_string);
//================================================================================================//


//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//

//================================================================================================//
// REVERSE COMPATIBLE DEFINITION LIST
//------------------------------------------------------------------------------------------------//
#define CoreString_FindChar(string, search_character)  			    CoreString_SearchChar(string, search_character)
#define CoreString_Find(source_string, search_string)               CoreString_Search(source_string, search_string)
#define CoreString_AlphabeticBefore(string2, string1)               CoreString_IsAlphabeticBefore(string1, string2)
#define CoreString_PadLeft(string, length, pad_character)           CoreString_PadStart(string, length, pad_character)
#define CoreString_PadRight(string, length, pad_character)          CoreString_PadEnd(string, length, pad_character)
//================================================================================================//

#endif /* CORESTRING_H */
