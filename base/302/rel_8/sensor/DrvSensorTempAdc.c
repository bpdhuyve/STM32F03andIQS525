//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of temperature measurements based on ADC into DrvSensor
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORTEMPADC_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORTEMPADC_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORTEMPADC_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of temperature adc sensors
#ifndef SENSOR_TEMP_ADC_COUNT
	#define SENSOR_TEMP_ADC_COUNT       1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// APP
#include "DrvSensorTempAdc.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    ADC_CHANNEL_HNDL        adc_channel_hndl;
    INTERP_EQUIDISTANT_Y    temp_adc_config;
    U16                     adc_value;
    S16                     temperature;
    BOOL                    successfull_reading;
}
TEMP_ADC_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTempAdc_PresentSensor(SENSOR_ID sensor_id);
static BOOL DrvSensorTempAdc_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorTempAdc_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorTempAdc_ConversionComplete(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SENSOR_HOOK_LIST   sensor_temp_adc_hook_list = {DrvSensorTempAdc_PresentSensor, DrvSensorTempAdc_ReadSensor, DrvSensorTempAdc_GetValue};
static SENSOR_STRUCT            sensor_struct[SENSOR_TEMP_ADC_COUNT];
static TEMP_ADC_CTRL            sensor_temp_adc_ctrl[SENSOR_TEMP_ADC_COUNT];
static U8					    sensor_temp_adc_count;
static SENSOR_ID			    sensor_temp_adc_active;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTempAdc_PresentSensor(SENSOR_ID sensor_id)
{
    if(sensor_id >= sensor_temp_adc_count)
    {
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTempAdc_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	TEMP_ADC_CTRL*		sensor_temp_adc_ctrl_ptr = &sensor_temp_adc_ctrl[sensor_id];

    if(sensor_id >= sensor_temp_adc_count)
    {
        return FALSE;
    }
    
    if(sensor_type != SENSOR_TYPE_TEMPERATURE)
    {
        return FALSE;
    }
    
    sensor_temp_adc_ctrl_ptr->temperature = 0;
    sensor_temp_adc_ctrl_ptr->successfull_reading = FALSE;
    
	if(sensor_temp_adc_active == INVALID_SENSOR_ID)
	{
		sensor_temp_adc_active = sensor_id;
        
        if(DrvAdc_StartConversion(sensor_temp_adc_ctrl_ptr->adc_channel_hndl, &sensor_temp_adc_ctrl_ptr->adc_value, wait_to_complete) == TRUE)
        {
            return TRUE;
        }
		sensor_temp_adc_active = INVALID_SENSOR_ID;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTempAdc_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	TEMP_ADC_CTRL*		sensor_temp_adc_ctrl_ptr = &sensor_temp_adc_ctrl[sensor_id];

    if(sensor_id >= sensor_temp_adc_count)
    {
        return FALSE;
    }

	if(sensor_type == SENSOR_TYPE_TEMPERATURE)
	{
		*data_ptr = (U16)sensor_temp_adc_ctrl_ptr->temperature;
		return sensor_temp_adc_ctrl_ptr->successfull_reading;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorTempAdc_ConversionComplete(void)
{
	TEMP_ADC_CTRL*		sensor_temp_adc_ctrl_ptr = &sensor_temp_adc_ctrl[sensor_temp_adc_active];
    
    if(sensor_temp_adc_active < sensor_temp_adc_count)
    {
        sensor_temp_adc_active = INVALID_SENSOR_ID;
        if(StdInterpolate_EquidistantY(sensor_temp_adc_ctrl_ptr->adc_value, &sensor_temp_adc_ctrl_ptr->temp_adc_config, &sensor_temp_adc_ctrl_ptr->temperature) == TRUE)
        {
            sensor_temp_adc_ctrl_ptr->successfull_reading = TRUE;
        }
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorTempAdc_Init(void)
{
    StdInterpolate_Init();
    
	sensor_temp_adc_count   = 0;
	sensor_temp_adc_active  = INVALID_SENSOR_ID;
    
	MEMSET((VPTR)sensor_temp_adc_ctrl, 0, SIZEOF(sensor_temp_adc_ctrl));
    MEMSET((VPTR)sensor_struct, 0, SIZEOF(sensor_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorTempAdc_Register(ADC_CHANNEL_HNDL adc_channel_hndl, INTERP_EQUIDISTANT_Y* tempadc_config_struct_ptr)
{
	SENSOR_STRUCT*	        sensor_struct_ptr        = &sensor_struct[sensor_temp_adc_count];
	TEMP_ADC_CTRL*	        sensor_temp_adc_ctrl_ptr = &sensor_temp_adc_ctrl[sensor_temp_adc_count];
    
	if(sensor_temp_adc_count < SENSOR_TEMP_ADC_COUNT)
	{
        sensor_temp_adc_ctrl_ptr->adc_channel_hndl  = adc_channel_hndl;
        sensor_temp_adc_ctrl_ptr->temp_adc_config   = *tempadc_config_struct_ptr;
		sensor_temp_adc_ctrl_ptr->temperature       = 0;
        DrvAdc_RegisterConversionComplete(sensor_temp_adc_ctrl_ptr->adc_channel_hndl, DrvSensorTempAdc_ConversionComplete);
        
		sensor_struct_ptr->hook_list_ptr = (SENSOR_HOOK_LIST*)&sensor_temp_adc_hook_list;
		sensor_struct_ptr->sensor_id     = sensor_temp_adc_count;
		sensor_temp_adc_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("Temperature ADC count overrun");
	return NULL;    
}
//================================================================================================//
