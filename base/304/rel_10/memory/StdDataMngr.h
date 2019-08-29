//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of data manager for saving system settings/data structs on the internal flash 
// of the chip. This module requires that internal flash can be read directly by using
// its memory address.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STDDATAMNGR_H
#define STDDATAMNGR_H
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
// @brief   The module idea must be a unique ID used to identify the data stored in flash
typedef U8   DATAMNGR_MODULE_ID;

// @brief   Prototype of a function to load the defaults
typedef void (*DATAMNGR_LOAD_DEFAULTS)(void);

// @brief   Prototype of a function to convert data from a previous version to the latest version
typedef void (*DATAMNGR_CONVERT_DATA)(U8* stored_data_ptr, U16 stored_data_len, U8  stored_data_version);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialises the module
// This function will analyse the data structure stored in flash and check if it contains valuable data.
void StdDataMngr_Init(void);

// @brief   Function to register a data structure to the data manager that is related to a module
// The function will register the supplied data. If the data length and the data version match with the data stored in
// flash, it will load the data structure with the data from flash, otherwise it will load the data structure with
// defaults using the DATAMNGR_LOAD_DEFAULTS callback and convert the data from flash with the 
// DATAMNGR_CONVERT_DATA callback.
// @param module_id	        : unique ID used by a module to identify its data struct
// @param convert_data_hook	: hook to a function that can convert the data from an older version to the latest
// @param load_defaults_hook: hook to a function that can load the default data in the supplied struct
// @param data_ptr          : pointer to the data struct to be saved
// @param data_len          : length of the data struct in bytes
// @param data_version      : version of the data struct
void StdDataMngr_Register(DATAMNGR_MODULE_ID module_id,
                          DATAMNGR_CONVERT_DATA convert_data_hook,
                          DATAMNGR_LOAD_DEFAULTS load_defaults_hook,
                          VPTR data_ptr,
                          U16 data_len,
                          U8 data_version,
                          BOOL allow_loaddefaults,
                          STRING module_name);

// @brief   Function to save all data to flash
void StdDataMngr_SaveData(void);

// @brief   Function to load all defaults
void StdDataMngr_LoadDefaults(void);
//================================================================================================//



#endif /* STDDATAMNGR_H */
