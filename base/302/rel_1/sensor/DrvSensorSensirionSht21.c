//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Sensirion SHT21 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORSENSIRIONSHT21_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORSENSIRIONSHT21_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORSENSIRIONSHT21_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef SHT21_COUNT
	#define SHT21_COUNT                 1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorSensirionSht21.h"
#include "i2c\DrvI2cMasterDevice.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    SHT21_NONE = 0x00,
    SHT21_TEMP = 0xE3,
    SHT21_RH   = 0xE5
}
SHT21_ACTION;

typedef struct
{
	I2C_DEVICE_ID	i2c_device_id;
	BOOL			i2c_is_present;
	S16				temperature;
	U16				rh_value;
    SHT21_ACTION    sht21_action;
}
SHT21_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorSensirionSht21_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorSensirionSht21_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorSensirionSht21_I2cMessageComplete(BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SENSOR_HOOK_LIST				sensor_sht21_hook_list;
static U8							sensor_sht21_count;
static SHT21_CTRL					sensor_sht21_ctrl[SHT21_COUNT];
static SENSOR_STRUCT				sensor_sht21_struct[SHT21_COUNT];
static BOOL							sensor_sht21_success;
static SENSOR_ID					sensor_sht21_active;
static U8							sensor_sht21_i2c_data[3];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_IsPresent(SENSOR_ID sensor_id)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_id];
	U8				data[8];

	sht21_ctrl_ptr->i2c_is_present = FALSE;
    data[0] = 0xFA;
    data[1] = 0x0F;
    if(DrvI2cMasterDevice_WriteData(sht21_ctrl_ptr->i2c_device_id, data, 2, TRUE))
    {
        MEMSET((VPTR)data, 0, SIZEOF(data));
        if((sensor_sht21_success) && DrvI2cMasterDevice_ReadData(sht21_ctrl_ptr->i2c_device_id, data, 8, TRUE))
        {
            sht21_ctrl_ptr->i2c_is_present = (BOOL)(sensor_sht21_success && (data[0] != 0) && (data[1] != 0));
        }
    }
    return sht21_ctrl_ptr->i2c_is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_id];
    U8              data = (U8)SHT21_NONE;

	if((sht21_ctrl_ptr->i2c_is_present) && (sensor_sht21_active == INVALID_SENSOR_ID))
	{
        if(sensor_type == SENSOR_TYPE_TEMPERATURE)
        {
            data = (U8)SHT21_TEMP;
        }
        else if(sensor_type == SENSOR_TYPE_RH)
        {
            data = (U8)SHT21_RH;
        }
        if((data != (U8)SHT21_NONE) && (DrvI2cMasterDevice_WriteData(sht21_ctrl_ptr->i2c_device_id, &data, 1, TRUE)))
        {
            if(sensor_sht21_success)
            {
                sensor_sht21_active = sensor_id;
                MEMSET((VPTR)sensor_sht21_i2c_data, 0, SIZEOF(sensor_sht21_i2c_data));
                sht21_ctrl_ptr->sht21_action = (SHT21_ACTION)data;
                if(DrvI2cMasterDevice_ReadData(sht21_ctrl_ptr->i2c_device_id, sensor_sht21_i2c_data, 3, wait_to_complete))
                {
                    return TRUE;
                }
                sensor_sht21_active = INVALID_SENSOR_ID;
            }
        }
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_id];
    
    if(sht21_ctrl_ptr->i2c_is_present)
    {
        if(sensor_type == SENSOR_TYPE_TEMPERATURE)
        {
            *data_ptr = (U16)sht21_ctrl_ptr->temperature;
            return TRUE;
        }
        if(sensor_type == SENSOR_TYPE_RH)
        {
            *data_ptr = (U16)sht21_ctrl_ptr->rh_value;
            return TRUE;
        }
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorSensirionSht21_I2cMessageComplete(BOOL success)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_sht21_active];

	sensor_sht21_success = success;

	if(sensor_sht21_active < sensor_sht21_count)
	{
		sensor_sht21_active = INVALID_SENSOR_ID;
        if(sht21_ctrl_ptr->sht21_action == SHT21_TEMP)
        {
            if(success)
            {
                sht21_ctrl_ptr->temperature = ((((S32)(((U16)sensor_sht21_i2c_data[0] << 8) | (U16)sensor_sht21_i2c_data[1]) * 17572) >> 16) - 4680) / 10;
            }
            else
            {
                sht21_ctrl_ptr->temperature = 0;
            }
        }
        else if(sht21_ctrl_ptr->sht21_action == SHT21_RH)
        {
            if(success)
            {
                sht21_ctrl_ptr->rh_value = (((U32)(((U16)sensor_sht21_i2c_data[0] << 8) | (U16)sensor_sht21_i2c_data[1]) * 12500L) >> 16) - 600;
            }
            else
            {
                sht21_ctrl_ptr->rh_value = 0;
            }
        }
        sht21_ctrl_ptr->sht21_action = SHT21_NONE;
	}
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorSensirionSht21_Init(void)
{
	sensor_sht21_hook_list.is_present_hook = DrvSensorSensirionSht21_IsPresent;
	sensor_sht21_hook_list.read_sensor_hook = DrvSensorSensirionSht21_ReadSensor;
	sensor_sht21_hook_list.get_value_hook = DrvSensorSensirionSht21_GetValue;

	sensor_sht21_count = 0;
	sensor_sht21_active = INVALID_SENSOR_ID;

	MEMSET((VPTR)sensor_sht21_ctrl, 0, SIZEOF(sensor_sht21_ctrl));
	MEMSET((VPTR)sensor_sht21_struct, 0, SIZEOF(sensor_sht21_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorSensirionSht21_Register(I2C_CHANNEL_HNDL i2c_channel)
{
	SENSOR_STRUCT*	sensor_struct_ptr = &sensor_sht21_struct[sensor_sht21_count];
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_sht21_count];

	if(sensor_sht21_count < SHT21_COUNT)
	{
		sht21_ctrl_ptr->i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, 0x40, 24000);
		if(sht21_ctrl_ptr->i2c_device_id == INVALID_I2C_DEVICE_ID)
		{
			LOG_WRN("SHT21 i2c registration failed");
			return NULL;
		}
		DrvI2cMasterDevice_MsgComplete(sht21_ctrl_ptr->i2c_device_id, DrvSensorSensirionSht21_I2cMessageComplete);
		sht21_ctrl_ptr->i2c_is_present = FALSE;
		sht21_ctrl_ptr->temperature = 0;
        sht21_ctrl_ptr->rh_value = 0;
        sht21_ctrl_ptr->sht21_action = SHT21_NONE;

		sensor_struct_ptr->hook_list_ptr = &sensor_sht21_hook_list;
		sensor_struct_ptr->sensor_id = sensor_sht21_count;
		sensor_sht21_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("SHT21 count overrun");
	return NULL;
}
//================================================================================================//
