//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the blocking ADC system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef ADC__ISYSADCBLOCK_H
#define ADC__ISYSADCBLOCK_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "adc\ISysAdc.h"
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
// @remark none
void SysAdcBlock_Init(void);

// @remark none
BOOL SysAdcBlock_Module_Init(ADC_MODULE module);

// @remark none
BOOL SysAdcBlock_RegisterAdcComplete(ADC_CONVERSION_COMPLETE adc_complete_hook);

// @remark none
BOOL SysAdcBlock_Channel_Init(ADC_CHANNEL channel, ADC_CONFIG_STRUCT* config_struct_ptr);

// @remark none
BOOL SysAdcBlock_Channel_StartConversion(ADC_CHANNEL channel, U16* value_ptr);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



#endif /* ADC__ISYSADCBLOCK_H */
