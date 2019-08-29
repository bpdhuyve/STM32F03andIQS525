//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Angle implementation specific for PICAN/PCMS PT1024
// Known restrictions:
//   - only 1 ANGLE_HNDL at runtime, so not possible to register for 2 handles
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define ANGLE_DRVANGLEQUADENC_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRV_ANGLE_QUADENC_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               DRV_ANGLE_QUADENC_LOG_LEVEL
#endif

#ifndef ANGLE__DRV_QUADENC_COUNT
    #define ANGLE__DRV_QUADENC_COUNT    ANGLE_BUS_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
#include "angle\DrvAngleQuadEnc.h"

//STD lib include section

//COM lib include section

//APP include section
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
static BOOL OnIndexPulse(ANGLE_BUS angle_bus, U32 angle_at_index_pulse);
static void OnCompare(ANGLE_BUS angle_bus, U32 compare_angle);
static BOOL GlueFunc_AngleConfig(ANGLE_ID angle_id, ANGLE_CONFIG_STRUCT* config_struct_ptr);
static BOOL GlueFunc_AngleSetCompareAngle(ANGLE_ID angle_id, U32 angle);
static void GlueFunc_AngleSetAbsolutePosition(ANGLE_ID angle_id, U32 angle);
static U32 GlueFunc_AngleGetAbsolutePosition(ANGLE_ID angle_id);
static U16 GlueFunc_AngleGetRelativePosition(ANGLE_ID angle_id);
static void GlueFunc_AngleSetCountingMode(ANGLE_ID angle_id, COUNTING_MODE mode);
static U8 GetAngleStructIndex(ANGLE_BUS angle_bus);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static ANGLE_HOOK_LIST                 angle_hook_list;
static ANGLE_STRUCT                    angle_struct[ANGLE__DRV_QUADENC_COUNT];
static U8                              anlge_enc_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static U8 GetAngleStructIndex(ANGLE_BUS angle_bus)
{
    U8 return_value = 0xFF;
    for(U8 i = 0; i < ANGLE__DRV_QUADENC_COUNT; i++)
    {
        if(angle_struct[i].angle_id == angle_bus)     
        {
            return_value = i;
            break;
        }
    }
    return return_value;
}
//------------------------------------------------------------------------------------------------//
static BOOL OnIndexPulse(ANGLE_BUS angle_bus, U32 angle_at_index_pulse)
{
    BOOL return_value = FALSE;
    
    //TO DO CORRENCTIE????
   
    U8 index = GetAngleStructIndex(angle_bus);
    DrvAngle_OnIndexPulse(&angle_struct[index], angle_at_index_pulse );

    return return_value;
}
//------------------------------------------------------------------------------------------------//
static void OnCompare(ANGLE_BUS angle_bus, U32 compare_angle)
{
    U8 index = GetAngleStructIndex(angle_bus);
    DrvAngle_OnCompare(&angle_struct[index], compare_angle );
}
//------------------------------------------------------------------------------------------------//
static BOOL GlueFunc_AngleConfig(ANGLE_ID angle_id, ANGLE_CONFIG_STRUCT* config_struct_ptr)
{
    return SysAngleQuadEnc_ConfigEnc((ANGLE_BUS)angle_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL GlueFunc_AngleSetCompareAngle(ANGLE_ID angle_id, U32 angle)
{
    return SysAngleQuadEnc_SetCompareAngleEnc((ANGLE_BUS)angle_id, angle);
}
//------------------------------------------------------------------------------------------------//
static void GlueFunc_AngleSetAbsolutePosition(ANGLE_ID angle_id, U32 angle)
{
    SysAngleQuadEnc_SetAbsolutePositionEnc((ANGLE_BUS)angle_id, angle);
}
//------------------------------------------------------------------------------------------------//
static U32 GlueFunc_AngleGetAbsolutePosition(ANGLE_ID angle_id)
{
    return SysAngleQuadEnc_GetAbsolutePositionEnc((ANGLE_BUS)angle_id);
}
//------------------------------------------------------------------------------------------------//
static U16 GlueFunc_AngleGetRelativePosition(ANGLE_ID angle_id)
{
    return SysAngleQuadEnc_GetRelativePositionEnc((ANGLE_BUS)angle_id);
}
//------------------------------------------------------------------------------------------------//
static void GlueFunc_AngleSetCountingMode(ANGLE_ID angle_id, COUNTING_MODE mode)
{
    //NOT POSSIBLE
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvAngleQuadEnc_Init(void)
{
    MODULE_INIT_ONCE();
    
    //place init code which must only be executed once here
    SysAngleQuadEnc_Init();
    SysAngleQuadEnc_RegisterOnIndexPulseEnc(OnIndexPulse);
    SysAngleQuadEnc_RegisterOnCompareEnc(OnCompare);
    for(U8 i = 0; i < ANGLE__DRV_QUADENC_COUNT; i++)
    {
        angle_struct[i].angle_id = 0;
        angle_struct[i].cycle_step =  0x10000;
        angle_struct[i].hook_list_ptr = NULL;
    }

    //init glue
    angle_hook_list.config_hook = GlueFunc_AngleConfig;
    angle_hook_list.get_absolute_position_hook = GlueFunc_AngleGetAbsolutePosition;
    angle_hook_list.get_relative_position_hook = GlueFunc_AngleGetRelativePosition;
    angle_hook_list.set_absolute_position_hook = GlueFunc_AngleSetAbsolutePosition;
    angle_hook_list.set_compare_angle_hook = GlueFunc_AngleSetCompareAngle;
    angle_hook_list.set_counting_mode_hook = GlueFunc_AngleSetCountingMode;

    anlge_enc_count = 0;
    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here
}
//------------------------------------------------------------------------------------------------//
ANGLE_HNDL DrvAngleQuadEnc_Register(ANGLE_BUS angle_bus)
{
    //simplified code since we only have 1 runtime instance
    if(SysAngleQuadEnc_InitEnc(angle_bus) == TRUE)
    {
        angle_struct[anlge_enc_count].hook_list_ptr = &angle_hook_list;
        angle_struct[anlge_enc_count].angle_id = angle_bus;
        anlge_enc_count++;
        return &angle_struct[anlge_enc_count-1];
    }
    LOG_ERR("illegal bus - %d", PU8(angle_bus));
    return NULL;
}
//------------------------------------------------------------------------------------------------//
U32 DrvAngleQuadEnc_GetAbsolutePos(ANGLE_BUS angle_bus)
{
    U8 index = GetAngleStructIndex(angle_bus);
    if(angle_struct[index].hook_list_ptr == NULL)
    {
        LOG_WRN("quad encoder not known yet");
        return 0;
    }
    return angle_struct[index].hook_list_ptr->get_absolute_position_hook(angle_struct[index].angle_id);
}
//------------------------------------------------------------------------------------------------//
U16 DrvAngleQuadEnc_GetRelativePos(ANGLE_BUS angle_bus)
{
    U8 index = GetAngleStructIndex(angle_bus);
    if(angle_struct[index].hook_list_ptr == NULL)
    {
        LOG_WRN("quad encoder not known yet");
        return 0;
    }
    return angle_struct[index].hook_list_ptr->get_relative_position_hook(angle_struct[index].angle_id);
}
//================================================================================================//
