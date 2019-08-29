//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to handle parameters
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef PARAMETER__STDPARAMETER_H
#define PARAMETER__STDPARAMETER_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @brief   marco to link to a list of parameters
#define LIST_REF(x)                 x, SIZEOF(x)/SIZEOF(PARAMETER_ITEM)
//------------------------------------------------------------------------------------------------//
// @brief   marco to link to a string array containing names of the enum values
#define ENUM_LIST(x)                (VPTR)x, 0, 1, SIZEOF(x)/SIZEOF(const STRING) - 1
//------------------------------------------------------------------------------------------------//
// @brief   marco to link to a string array containing names of the bool values
#define BOOL_LIST(x)                ENUM_LIST(bool_enum_names)
//------------------------------------------------------------------------------------------------//
// @remark  parameter type component definitions
#define TYPE_BIT_ENUM               0x100       // indicate parameter is a enum
//------------------------------------------------------------------------------------------------//
#define TYPE_BIT_DONT_SAVE          0x200       // indicate save function should not be called on when parameter is set
#define DONTSAVE(x)                 (PARAMETER_TYPE)(x|TYPE_BIT_DONT_SAVE)
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8              PARA_LIST_HNDL;

typedef BOOL (*PARAMETER_ALLOWED_HOOK)(U8 index);

typedef enum
{
    TYPE_U8         = PTYPE_U8,
    TYPE_U16        = PTYPE_U16,
    TYPE_U32        = PTYPE_U32,
    
    TYPE_S8         = PTYPE_S8,
    TYPE_S16        = PTYPE_S16,
    TYPE_S32        = PTYPE_S32,
    
    TYPE_ENUM_U8    = PTYPE_U8  | TYPE_BIT_ENUM,    // only usable for enums that use 1 byte
    TYPE_BOOL       = PTYPE_U8  | TYPE_BIT_ENUM,    // special case of previous
}
PARAMETER_TYPE;

typedef struct
{
    VPTR                    value_ptr;
    PARAMETER_TYPE          para_type;
    const STRING            para_name;
    union
    {
        const STRING        unit_name;          // used for all but TYPE_ENUM_U8 and TYPE_BOOL.
        const STRING*       enum_name_list;     // used for TYPE_ENUM_U8. Use ENUM_LIST(x) macro for easier referencing
    }
    unit;
    U32                     min_value;
    U32                     increment;
    U32                     max_value;
}
PARAMETER_ITEM;

// --- USAGE EXAMPLE ---
//typedef enum
//{
//    ENUM_0  = 0,
//    ENUM_1  = 1,
//    ENUM_2  = 2,
//}
//ENUM;
//
//static U8                               temp_u8;
//static S16                              temp_s16;
//static BOOL                             temp_bool;
//static ENUM                             temp_enum;
//
//static PARA_LIST_HNDL                   module_para_hndl;
//const STRING                            module_enum_names[] = {"ENUM_0", "ENUM_1", "ENUM_2"};
//const PARAMETER_ITEM                    module_paramlist[] = 
//{
//    {(VPTR)&temp_u8,        TYPE_U8,        "TEMP U8",      "unit_1",   0, 1, 200},
//    {(VPTR)&temp_s16,       TYPE_S16,       "TEMP S16",     "unit_2",   (U32)-500, 5, 500},
//    {(VPTR)&temp_bool,      TYPE_BOOL,      "TEMP BOOL",    BOOL_LIST()},
//    {(VPTR)&temp_enum,      TYPE_ENUM_U8,   "TEMP ENUM",    ENUM_LIST(module_enum_names)},
//};
//
//module_para_hndl = StdParameter_RegisterParaList(LIST_REF(module_paramlist));
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern const STRING                        bool_enum_names[2];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void StdParameter_Init(void);

// @remark  Function to register the function to be called to save the data after update
void StdParameter_RegisterSaveHook(EVENT_CALLBACK save_hook);

// @remark  Function to register a parameter list. Use LIST_REF(x) macro for easier referencing
// @return  Returns a handle, LOG_ERR is called if PARAMETER_LIST_COUNT is insufficient
PARA_LIST_HNDL StdParameter_RegisterParaList(const PARAMETER_ITEM* para_list_ptr, U8 list_length);

// @remark  Function to register a hook that is called to check if a parameter can be printed or get.
BOOL StdParameter_RegisterParameterAllowedHook(PARA_LIST_HNDL para_list_hndl, PARAMETER_ALLOWED_HOOK allowed_hook);

// @remark  Function to print the parameter list using LOG_TRM
void StdParameter_Print(PARA_LIST_HNDL para_list_hndl);

// @remark  Function to get a parameter of a list.
BOOL StdParameter_GetPara(PARA_LIST_HNDL para_list_hndl, U8 index, U32* value_ptr);

// @remark  Function to set a parameter of a list. If the value is different, the registered save hook will be called.
BOOL StdParameter_SetPara(PARA_LIST_HNDL para_list_hndl, U8 index, U32 value);
//================================================================================================//



#endif /* PARAMETER__STDPARAMETER_H */
