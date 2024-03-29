//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the ADC driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DAC__DRVDAC_H
#define DAC__DRVDAC_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//ISYS include section
#include "dac/ISysDac.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          DAC_CHANNEL_ID;

typedef BOOL (*DAC_SET_VALUE)(DAC_CHANNEL_ID channel_id, U16 dac_value);
typedef BOOL (*DAC_GET_VALUE)(DAC_CHANNEL_ID channel_id, U16* dac_value);

typedef struct
{
    DAC_SET_VALUE            set_value_hook;
    DAC_GET_VALUE            get_value_hook;
}
DAC_CHANNEL_HOOK_LIST;

typedef struct
{
    DAC_CHANNEL_HOOK_LIST*	    hook_list_ptr;
    DAC_CHANNEL_ID                  channel_id;
}
DAC_CHANNEL_STRUCT;

typedef DAC_CHANNEL_STRUCT*         DAC_CHANNEL_HNDL;

//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark none
void DrvDac_Init(void);
// @remark none
BOOL DrvDac_SetValue(DAC_CHANNEL_HNDL channel_hndl, U16 dac_value);
// @remark none
BOOL DrvDac_GetValue(DAC_CHANNEL_HNDL channel_hndl, U16* dac_value);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* DAC__DRVDAC_H */
