//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Sensirion SDP6xx/5xx sensor driver
// Pressure is returned in tens of Pascal
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORSENSIRIONSDP6XX5XX_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORSENSIRIONSDP6XX5XX_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORSENSIRIONSDP6XX5XX_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of Sdp6xx/5xx sensors
#ifndef SDP6XX5XX_COUNT
	#define SDP6XX5XX_COUNT             1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the resolution of the Sdp6xx/5xx sensors
#ifndef SDP6XX5XX_RESOLUTION
    #define SDP6XX5XX_RESOLUTION            16
#else
    #if ((SDP6XX5XX_RESOLUTION < 9) || (SDP6XX5XX_RESOLUTION > 16))
        #undef SDP6XX5XX_RESOLUTION
        #define SDP6XX5XX_RESOLUTION        12
    #endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "sensor\DrvSensorSensirionSdp6xx5xx.h"
#include "i2c\DrvI2cMasterDevice.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SDP6XX5XX_CRC_POLYNOMIAL                            0x131               //P(x)=x^8+x^5+x^4+1
#define CMD_SDP6XX5XX_TRIGGER_MEASUREMENT                   0xF1
#define CMD_SDP6XX5XX_READ_EEPROM                           0xFA
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    USER_REGISTER               = 0xE3,
    ADVANCED_USER_REGISTER      = 0xE5,
    READ_ONLY_REGISTER1         = 0xE7,
    READ_ONLY_REGISTER2         = 0xE9
}
SDP6XX5XX_REGISTER;

typedef struct
{
	I2C_DEVICE_ID	i2c_device_id;
    BOOL            i2c_is_present;
    S16             pressure;
    U16             scale_factor;
}
SDP6XX5XX_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_CheckCrc(U8* data, U8 data_length, U8 checksum);
static BOOL DrvSensorSensirionSdp6xx5xx_ReadRegister(I2C_DEVICE_ID i2c_device_id, SDP6XX5XX_REGISTER sensor_register, U16* register_data);
static BOOL DrvSensorSensirionSdp6xx5xx_ReadEeprom(I2C_DEVICE_ID i2c_device_id, U16 eeprom_address, U8 size, U16* eeprom_data);
static BOOL DrvSensorSensirionSdp6xx5xx_GetScaleFactor(SENSOR_ID sensor_id);
static BOOL DrvSensorSensirionSdp6xx5xx_SetResolution(SENSOR_ID sensor_id);
static BOOL DrvSensorSensirionSdp6xx5xx_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorSensirionSdp6xx5xx_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorSensirionSdp6xx5xx_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorSensirionSdp6xx5xx_I2cMessageComplete(BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SENSOR_HOOK_LIST				sensor_hook_list;
static U8							sensor_count;
static SDP6XX5XX_CTRL_STRUCT        sensor_ctrl_struct[SDP6XX5XX_COUNT];
static SENSOR_STRUCT				sensor_struct[SDP6XX5XX_COUNT];
static BOOL							sensor_success;
static SENSOR_ID					sensor_active;
static U8							sensor_i2c_data[3];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_CheckCrc(U8* data, U8 data_length, U8 checksum)
{
    U8  crc = 0;
    U8  byte_counter;
    U8  i;
    
    for(byte_counter = 0; byte_counter < data_length; byte_counter++)
    {
        crc ^= data[byte_counter];
        for(i = 8; i > 0; i--)
        {
            if(crc & 0x80)
            {
               crc = (crc << 1) ^ SDP6XX5XX_CRC_POLYNOMIAL;
            }
            else
            {
               crc = (crc << 1);
            }
        }
    }
    return (BOOL)(crc == checksum);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_ReadRegister(I2C_DEVICE_ID i2c_device_id, SDP6XX5XX_REGISTER sensor_register, U16* register_data)
{
    U8      data[3];
    
    data[0] = (U8)sensor_register;
    if(DrvI2cMasterDevice_WriteData(i2c_device_id, &data[0], 1, TRUE))
    {
        MEMSET((VPTR)data, 0, SIZEOF(data));
        if(DrvI2cMasterDevice_ReadData(i2c_device_id, &data[0], 3, TRUE))
        {
            if(DrvSensorSensirionSdp6xx5xx_CheckCrc(&data[0], 2, data[2]) == TRUE)
            {
                *register_data = (U16)((data[0] << 8) | data[1]);
                return TRUE;
            }
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_ReadEeprom(I2C_DEVICE_ID i2c_device_id, U16 eeprom_address, U8 size, U16* eeprom_data)
{
    U8                          data[3];
    U8                          i;
    
    data[0] = (U8)CMD_SDP6XX5XX_READ_EEPROM;
    data[1] = (U8)(((eeprom_address << 4) & 0xFF00) >> 8);
    data[2] = (U8)((eeprom_address << 4) & 0x00FF);
    
    if(DrvI2cMasterDevice_WriteData(i2c_device_id, &data[0], 3, TRUE))
    {
        for(i = 0; i < size; i++)
        {
            MEMSET((VPTR)data, 0, SIZEOF(data));
            if(DrvI2cMasterDevice_ReadData(i2c_device_id, &data[0], 3, TRUE))
            {
                if(DrvSensorSensirionSdp6xx5xx_CheckCrc(&data[0], 2, data[2]) == TRUE)
                {
                    eeprom_data[i] = ((U16)(data[0] << 8) | (U16)(data[1]));
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_GetScaleFactor(SENSOR_ID sensor_id)
{
    SDP6XX5XX_CTRL_STRUCT*      sensor_ctrl_struct_ptr = &sensor_ctrl_struct[sensor_id];
    U16                         user_register;
    U16                         eeprom_address;
    U16                         scale_factor;
    
    if(DrvSensorSensirionSdp6xx5xx_ReadRegister(sensor_ctrl_struct_ptr->i2c_device_id, USER_REGISTER, &user_register) == TRUE)
    {
        eeprom_address = (((user_register & 0x0070) >> 4) * 0x0300) + 0x02B6;
        if(DrvSensorSensirionSdp6xx5xx_ReadEeprom(sensor_ctrl_struct_ptr->i2c_device_id, eeprom_address, 1, &scale_factor))
        {
            sensor_ctrl_struct_ptr->scale_factor = scale_factor;
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_SetResolution(SENSOR_ID sensor_id)
{
    SDP6XX5XX_CTRL_STRUCT*      sensor_ctrl_struct_ptr = &sensor_ctrl_struct[sensor_id];
    U16                         user_register;
    U8                          data[3];
    
    if(DrvSensorSensirionSdp6xx5xx_ReadRegister(sensor_ctrl_struct_ptr->i2c_device_id, ADVANCED_USER_REGISTER, &user_register) == TRUE)
    {
        data[0] = (U8)(ADVANCED_USER_REGISTER & 0xFE);
        data[1] = (U8)(((user_register & 0xF100) >> 8) | ((SDP6XX5XX_RESOLUTION - 9) << 1));
        data[2] = (U8)(user_register & 0x00FF);
        return (DrvI2cMasterDevice_WriteData(sensor_ctrl_struct_ptr->i2c_device_id, &data[0], 3, TRUE));
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_IsPresent(SENSOR_ID sensor_id)
{
    SDP6XX5XX_CTRL_STRUCT*      sensor_ctrl_struct_ptr = &sensor_ctrl_struct[sensor_id];
    U16                         read_only_register;
    BOOL                        get_scale_factor;
    BOOL                        set_resolution;
    
	sensor_ctrl_struct_ptr->i2c_is_present = FALSE;
    if(DrvSensorSensirionSdp6xx5xx_ReadRegister(sensor_ctrl_struct_ptr->i2c_device_id, READ_ONLY_REGISTER1, &read_only_register) == TRUE)
    {
        sensor_ctrl_struct_ptr->i2c_is_present = (BOOL)(sensor_success && (read_only_register != 0));
        get_scale_factor = DrvSensorSensirionSdp6xx5xx_GetScaleFactor(sensor_id);
        set_resolution = DrvSensorSensirionSdp6xx5xx_SetResolution(sensor_id);
    }
    return (BOOL)(sensor_ctrl_struct_ptr->i2c_is_present && get_scale_factor && set_resolution);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	SDP6XX5XX_CTRL_STRUCT*      sensor_ctrl_struct_ptr = &sensor_ctrl_struct[sensor_id];
    U8                          command;
    
    if((sensor_type == SENSOR_TYPE_PRESSURE) && (sensor_ctrl_struct_ptr->i2c_is_present) && (sensor_active == INVALID_SENSOR_ID))
	{
        command = (U8)CMD_SDP6XX5XX_TRIGGER_MEASUREMENT;
        if(DrvI2cMasterDevice_WriteData(sensor_ctrl_struct_ptr->i2c_device_id, &command, 1, TRUE))
        {
            if(sensor_success)
            {
                sensor_active = sensor_id;
                MEMSET((VPTR)sensor_i2c_data, 0, SIZEOF(sensor_i2c_data));
                if(DrvI2cMasterDevice_ReadData(sensor_ctrl_struct_ptr->i2c_device_id, &sensor_i2c_data[0], 3, wait_to_complete))
                {
                    return TRUE;
                }
                sensor_active = INVALID_SENSOR_ID;
            }
        }
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp6xx5xx_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	SDP6XX5XX_CTRL_STRUCT*      sensor_ctrl_struct_ptr = &sensor_ctrl_struct[sensor_id];
    
    if(sensor_ctrl_struct_ptr->i2c_is_present)
    {
        if(sensor_type == SENSOR_TYPE_PRESSURE)
        {
            *data_ptr = (U16)sensor_ctrl_struct_ptr->pressure;
            return TRUE;
        }
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorSensirionSdp6xx5xx_I2cMessageComplete(BOOL success)
{
    SDP6XX5XX_CTRL_STRUCT*		sensor_ctrl_struct_ptr = &sensor_ctrl_struct[sensor_active];

	sensor_success = success;

	if(sensor_active < sensor_count)
	{
		sensor_active = INVALID_SENSOR_ID;
        if(success)
        {
            if(DrvSensorSensirionSdp6xx5xx_CheckCrc(&sensor_i2c_data[0], 2, sensor_i2c_data[2]) == TRUE)
            {
                sensor_ctrl_struct_ptr->pressure = (((S16)((sensor_i2c_data[0] << 8) | sensor_i2c_data[1]) * 10) / sensor_ctrl_struct_ptr->scale_factor);
                return;
            }
        }
        sensor_ctrl_struct_ptr->pressure = 0;
	}
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorSensirionSdp6xx5xx_Init(void)
{
	sensor_hook_list.is_present_hook = DrvSensorSensirionSdp6xx5xx_IsPresent;
	sensor_hook_list.read_sensor_hook = DrvSensorSensirionSdp6xx5xx_ReadSensor;
	sensor_hook_list.get_value_hook = DrvSensorSensirionSdp6xx5xx_GetValue;
    
	sensor_count = 0;
	sensor_active = INVALID_SENSOR_ID;
    
	MEMSET((VPTR)sensor_ctrl_struct, 0, SIZEOF(sensor_ctrl_struct));
    MEMSET((VPTR)sensor_struct, 0, SIZEOF(sensor_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorSensirionSdp6xx5xx_Register(I2C_CHANNEL_HNDL i2c_channel)
{
    SENSOR_HNDL                 sensor_hndl = &sensor_struct[sensor_count];
    SDP6XX5XX_CTRL_STRUCT*      sensor_ctrl_struct_ptr = &sensor_ctrl_struct[sensor_count];
    
	if(sensor_count < SDP6XX5XX_COUNT)
	{
		sensor_ctrl_struct_ptr->i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, 0x40, 45000);
		if(sensor_ctrl_struct_ptr->i2c_device_id == INVALID_I2C_DEVICE_ID)
		{
			LOG_WRN("SDP6xx5xx i2c registration failed");
			return NULL;
		}
        DrvI2cMasterDevice_MsgComplete(sensor_ctrl_struct_ptr->i2c_device_id, DrvSensorSensirionSdp6xx5xx_I2cMessageComplete);
		sensor_ctrl_struct_ptr->i2c_is_present = FALSE;
		sensor_ctrl_struct_ptr->pressure = 0;
		sensor_hndl->hook_list_ptr = &sensor_hook_list;
		sensor_hndl->sensor_id = sensor_count;
		sensor_count++;
		return sensor_hndl;
	}
	LOG_WRN("SDP6xx5xx count overrun");
	return NULL;
}
//================================================================================================//