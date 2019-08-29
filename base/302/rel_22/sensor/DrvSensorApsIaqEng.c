//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Applied Sensor iAQ engine sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORAPSIAQENG_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORAPSIAQENG_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORAPSIAQENG_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef VOC_COUNT
	#define VOC_COUNT                   1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorApsIaqEng.h"
#include "i2c\DrvI2cMasterDevice.h"
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
	I2C_DEVICE_ID	i2c_device_id;
	BOOL			i2c_is_present;
	U16				co2_level;
}
IAQ_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorApsIaqEng_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorApsIaqEng_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorApsIaqEng_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorApsIaqEng_I2cMessageComplete(BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SENSOR_HOOK_LIST				sensor_iaq_hook_list;
static U8							sensor_iaq_count;
static IAQ_CTRL					    sensor_iaq_ctrl[VOC_COUNT];
static SENSOR_STRUCT				sensor_iaq_struct[VOC_COUNT];
static BOOL							sensor_iaq_success;
static SENSOR_ID					sensor_iaq_active;
static U8							sensor_iaq_i2c_data[7];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorApsIaqEng_IsPresent(SENSOR_ID sensor_id)
{
	IAQ_CTRL*		iaq_ctrl_ptr = &sensor_iaq_ctrl[sensor_id];

	iaq_ctrl_ptr->i2c_is_present = FALSE;
    if(DrvI2cMasterDevice_ReadData(iaq_ctrl_ptr->i2c_device_id, sensor_iaq_i2c_data, 7, TRUE))
    {
        iaq_ctrl_ptr->i2c_is_present = sensor_iaq_success;
    }
    return iaq_ctrl_ptr->i2c_is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorApsIaqEng_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	IAQ_CTRL*		iaq_ctrl_ptr = &sensor_iaq_ctrl[sensor_id];

	if((sensor_type == SENSOR_TYPE_CO2) && (iaq_ctrl_ptr->i2c_is_present) && (sensor_iaq_active == INVALID_SENSOR_ID))
	{
		sensor_iaq_active = sensor_id;
		MEMSET((VPTR)sensor_iaq_i2c_data, 0, SIZEOF(sensor_iaq_i2c_data));
		if(DrvI2cMasterDevice_ReadData(iaq_ctrl_ptr->i2c_device_id, sensor_iaq_i2c_data, 7, wait_to_complete))
		{
			return TRUE;
		}
		sensor_iaq_active = INVALID_SENSOR_ID;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorApsIaqEng_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	IAQ_CTRL*		iaq_ctrl_ptr = &sensor_iaq_ctrl[sensor_id];

	if((sensor_type == SENSOR_TYPE_CO2) && (iaq_ctrl_ptr->i2c_is_present))
	{
		*data_ptr = (U16)iaq_ctrl_ptr->co2_level;
		return TRUE;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorApsIaqEng_I2cMessageComplete(BOOL success)
{
	IAQ_CTRL*		iaq_ctrl_ptr = &sensor_iaq_ctrl[sensor_iaq_active];

	sensor_iaq_success = success;

	if(sensor_iaq_active < sensor_iaq_count)
	{
		sensor_iaq_active = INVALID_SENSOR_ID;
		if(success)
		{
			iaq_ctrl_ptr->co2_level = (((U16)sensor_iaq_i2c_data[0] << 8) | (U16)sensor_iaq_i2c_data[1]);
		}
		else
		{
			iaq_ctrl_ptr->co2_level = 0;
		}
	}
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorApsIaqEng_Init(void)
{
	sensor_iaq_hook_list.is_present_hook = DrvSensorApsIaqEng_IsPresent;
	sensor_iaq_hook_list.read_sensor_hook = DrvSensorApsIaqEng_ReadSensor;
	sensor_iaq_hook_list.get_value_hook = DrvSensorApsIaqEng_GetValue;

	sensor_iaq_count = 0;
	sensor_iaq_active = INVALID_SENSOR_ID;

	MEMSET((VPTR)sensor_iaq_ctrl, 0, SIZEOF(sensor_iaq_ctrl));
	MEMSET((VPTR)sensor_iaq_struct, 0, SIZEOF(sensor_iaq_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorApsIaqEng_Register(I2C_CHANNEL_HNDL i2c_channel)
{
	SENSOR_STRUCT*	sensor_struct_ptr = &sensor_iaq_struct[sensor_iaq_count];
	IAQ_CTRL*		iaq_ctrl_ptr = &sensor_iaq_ctrl[sensor_iaq_count];

	if(sensor_iaq_count < VOC_COUNT)
	{
		iaq_ctrl_ptr->i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, 0x5A, 24000);
		if(iaq_ctrl_ptr->i2c_device_id == INVALID_I2C_DEVICE_ID)
		{
			LOG_WRN("IAQ i2c registration failed");
			return NULL;
		}
		DrvI2cMasterDevice_MsgComplete(iaq_ctrl_ptr->i2c_device_id, DrvSensorApsIaqEng_I2cMessageComplete);
		iaq_ctrl_ptr->i2c_is_present = FALSE;
		iaq_ctrl_ptr->co2_level = 0;

		sensor_struct_ptr->hook_list_ptr = &sensor_iaq_hook_list;
		sensor_struct_ptr->sensor_id = sensor_iaq_count;
		sensor_iaq_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("IAQ count overrun");
	return NULL;
}
//================================================================================================//
