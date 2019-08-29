//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// module to handle parameters
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define PARAMETER__STDPARAMETER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef PARAMETER__STDPARAMETER_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               PARAMETER__STDPARAMETER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef PARAMETER_LIST_COUNT
    #define PARAMETER_LIST_COUNT                1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef LOG_FLUSH_EVERY_X_PARAMETERS
    #define LOG_FLUSH_EVERY_X_PARAMETERS        10
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV

// STD
#include "parameter/StdParameter.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_PARA_HNDL                   0      
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    const PARAMETER_ITEM*   para_list_ptr;
    U8                      list_length;
    BOOL                    is_para_list;
    PARAMETER_ALLOWED_HOOK  allowed_hook;
}
PARA_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static PARA_LIST_HNDL Parameter_RegisterList(const PARAMETER_ITEM* para_list_ptr, U8 list_length, BOOL is_para_list);
static void Parameter_AppendUnit(const STRING unit_str);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static EVENT_CALLBACK               parameter_save_hook;
static PARA_STRUCT                  parameter_struct[PARAMETER_LIST_COUNT];
static U8                           parameter_list_count;

const STRING                        close_str   = ")      ";
static U8                           close_len;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
const STRING                        bool_enum_names[2]   = {"FALSE", "TRUE"};
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static PARA_LIST_HNDL Parameter_RegisterList(const PARAMETER_ITEM* para_list_ptr, U8 list_length, BOOL is_para_list)
{
    PARA_STRUCT*    para_struct_ptr = &parameter_struct[parameter_list_count];
    
    MODULE_CHECK();
    
    if(parameter_list_count >= PARAMETER_LIST_COUNT)
    {
        LOG_ERR("Failed to allocate");
        return INVALID_PARA_HNDL;
    }
    
    if((para_list_ptr == NULL) || (list_length == 0))
    {
        LOG_ERR("Illegal data");
        return INVALID_PARA_HNDL;
    }
    
    para_struct_ptr->para_list_ptr  = para_list_ptr;
    para_struct_ptr->list_length    = list_length;
    para_struct_ptr->is_para_list   = is_para_list;
    para_struct_ptr->allowed_hook   = NULL;
    
    parameter_list_count++;
    return (PARA_LIST_HNDL)parameter_list_count;
}
//------------------------------------------------------------------------------------------------//
static void Parameter_AppendUnit(const STRING unit_str)
{
    U8              unit_len    = CoreString_GetLength(unit_str);
    
    if(unit_len > (close_len - 2)) {unit_len = close_len - 2;}
    LOG("(%s%c", LOG_LEVEL_TERM, PCSTR(unit_str), PCHARA(close_str, close_len - unit_len));
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdParameter_Init(void)
{
    MODULE_INIT_ONCE();
    
    parameter_save_hook = NULL;
    parameter_list_count = 0;
    MEMSET((VPTR)parameter_struct, 0, SIZEOF(parameter_struct));
    
    close_len = CoreString_GetLength(close_str);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void StdParameter_RegisterSaveHook(EVENT_CALLBACK save_hook)
{
    MODULE_CHECK();
    
    parameter_save_hook = save_hook;
}
//------------------------------------------------------------------------------------------------//
PARA_LIST_HNDL StdParameter_RegisterParaList(const PARAMETER_ITEM* para_list_ptr, U8 list_length)
{
    return Parameter_RegisterList(para_list_ptr, list_length, TRUE);
}
//------------------------------------------------------------------------------------------------//
PARA_LIST_HNDL StdParameter_RegisterInfoList(const PARAMETER_ITEM* info_list_ptr, U8 list_length)
{
    return Parameter_RegisterList(info_list_ptr, list_length, FALSE);
}
//------------------------------------------------------------------------------------------------//
BOOL StdParameter_RegisterParameterAllowedHook(PARA_LIST_HNDL para_list_hndl, PARAMETER_ALLOWED_HOOK allowed_hook)
{
    PARA_STRUCT*            para_struct_ptr = &parameter_struct[(PARA_LIST_HNDL)(para_list_hndl - 1)];
    
    // check of legal hndl
    if((PARA_LIST_HNDL)(para_list_hndl - 1) >= parameter_list_count)
    {
        LOG_ERR("illegal hndl");
        return FALSE;
    }
    
    para_struct_ptr->allowed_hook = allowed_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
void StdParameter_Print(PARA_LIST_HNDL para_list_hndl)
{
    PARA_STRUCT*            para_struct_ptr = &parameter_struct[(PARA_LIST_HNDL)(para_list_hndl - 1)];
    const PARAMETER_ITEM*   item_ptr;
    U8                      index;
    U32                     value_now;
    PTYPE                   ptype;
    
    // check of legal hndl
    if((PARA_LIST_HNDL)(para_list_hndl - 1) >= parameter_list_count)
    {
        CoreTerm_PrintFailed();
        return;
    }
    
    for(index = 0, item_ptr = para_struct_ptr->para_list_ptr; index < para_struct_ptr->list_length; index++, item_ptr++)
    {
        if(StdParameter_GetPara(para_list_hndl, index, &value_now) == FALSE) {continue;}
        
        ptype = (PTYPE)(item_ptr->para_type & 0xFF);
        LOG("%2d. %-28s: %7d ", LOG_LEVEL_TERM, PU8(index), PCSTR(item_ptr->para_name), ptype, value_now);
        if(item_ptr->para_type & TYPE_BIT_ENUM)
        {
            if(value_now > item_ptr->max_value)
            {
                Parameter_AppendUnit("UNKNOWN");
            }
            else
            {
                Parameter_AppendUnit(item_ptr->unit.enum_name_list[value_now]);
            }
        }
        else
        {
            Parameter_AppendUnit(item_ptr->unit.unit_name);
        }
        if(para_struct_ptr->is_para_list)
        {
            LOG_TRM("[%d:%d:%d]", ptype, item_ptr->min_value, ptype, item_ptr->increment, ptype, item_ptr->max_value);
        }
        else
        {
            LOG_TRM("");
        }
        
#if (LOG_FLUSH_EVERY_X_PARAMETERS > 0)
        if((index % (LOG_FLUSH_EVERY_X_PARAMETERS)) == ((LOG_FLUSH_EVERY_X_PARAMETERS) - 1))
        {
            CoreLog_Flush();
        }
#endif
    }
    
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
BOOL StdParameter_GetPara(PARA_LIST_HNDL para_list_hndl, U8 index, U32* value_ptr)
{
    PARA_STRUCT*            para_struct_ptr = &parameter_struct[(PARA_LIST_HNDL)(para_list_hndl - 1)];
    const PARAMETER_ITEM*   item_ptr;
    
    // check of legal hndl
    if((PARA_LIST_HNDL)(para_list_hndl - 1) >= parameter_list_count)
    {
        LOG_ERR("illegal hndl");
        return FALSE;
    }
    
    if(index >= para_struct_ptr->list_length)       {return FALSE;}
    if((para_struct_ptr->allowed_hook != NULL) && (para_struct_ptr->allowed_hook(index) == FALSE))  {return FALSE;}
    
    item_ptr = &(para_struct_ptr->para_list_ptr[index]);
    switch(item_ptr->para_type & 0xFF)
    {
    case TYPE_U8:
        *value_ptr = (U32)(*((U8*)(item_ptr->value_ptr)));
        break;
    case TYPE_S8:
        *value_ptr = (U32)((S32)(*((S8*)(item_ptr->value_ptr))));
        break;
    case TYPE_U16:
        *value_ptr = (U32)(*((U16*)(item_ptr->value_ptr)));
        break;
    case TYPE_S16:
        *value_ptr = (U32)((S32)(*((S16*)(item_ptr->value_ptr))));
        break;
    case TYPE_U32:
        *value_ptr = *((U32*)(item_ptr->value_ptr));
        break;
    case TYPE_S32:
        *value_ptr = (U32)(*((S32*)(item_ptr->value_ptr)));
        break;
    default:
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdParameter_SetPara(PARA_LIST_HNDL para_list_hndl, U8 index, U32 value)
{
    PARA_STRUCT*            para_struct_ptr = &parameter_struct[(PARA_LIST_HNDL)(para_list_hndl - 1)];
    const PARAMETER_ITEM*   item_ptr;
    U32                     value_now;
    
    // check of legal hndl
    if((PARA_LIST_HNDL)(para_list_hndl - 1) >= parameter_list_count)
    {
        LOG_ERR("illegal hndl");
        return FALSE;
    }
    
    // if list is not a para list: deny
    if(para_struct_ptr->is_para_list == FALSE)
    {
        return FALSE;
    }
    
    // get current value and check if new is equal or not
    if(StdParameter_GetPara(para_list_hndl, index, &value_now) == FALSE)    {return FALSE;}
    if(value == value_now)                          {return TRUE;}
    
    item_ptr = &(para_struct_ptr->para_list_ptr[index]);
    // check limits
    if(item_ptr->para_type & LOG_BIT_SIGN)
    {
        if((S32)value < (S32)item_ptr->min_value)   {return FALSE;}
        if((S32)value > (S32)item_ptr->max_value)   {return FALSE;}
        if((item_ptr->increment > 1) && ((((S32)value - (S32)item_ptr->min_value) % (S32)item_ptr->increment) != 0))   {return FALSE;}
    }
    else
    {
        if(value < item_ptr->min_value)             {return FALSE;}
        if(value > item_ptr->max_value)             {return FALSE;}
        if((item_ptr->increment > 1) && (((value - item_ptr->min_value) % item_ptr->increment) != 0))   {return FALSE;}
    }
    
    // update parameter
    switch(item_ptr->para_type & 0xFF)
    {
    case TYPE_U8:
        *((U8*)(item_ptr->value_ptr)) = (U8)value;
        break;
    case TYPE_S8:
        *((S8*)(item_ptr->value_ptr)) = (S8)((S32)value);
        break;
    case TYPE_U16:
        *((U16*)(item_ptr->value_ptr)) = (U16)value;
        break;
    case TYPE_S16:
        *((S16*)(item_ptr->value_ptr)) = (S16)((S32)value);
        break;
    case TYPE_U32:
        *((U32*)(item_ptr->value_ptr)) = (U32)value;
        break;
    case TYPE_S32:
        *((S32*)(item_ptr->value_ptr)) = (S32)value;
        break;
    default:
        return FALSE;
    }
    
    // save data (if allowed)
    if((parameter_save_hook != NULL) && ((item_ptr->para_type & TYPE_BIT_DONT_SAVE) == 0))
    {
        parameter_save_hook();
    }
    return TRUE;
}
//================================================================================================//
