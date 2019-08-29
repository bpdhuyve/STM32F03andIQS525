//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the NXP SE95 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORTITMP121_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORTITMP121_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORTITMP121_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef TMP121_COUNT
	#define TMP121_COUNT                  1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorTiTmp121.h"
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
	SPI_DEVICE_ID   spi_device_id;
    U8              data[2];
    BOOL            is_present;
}
TMP121_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTiTmp121_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorTiTmp121_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorTiTmp121_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SENSOR_HOOK_LIST				sensor_tmp121_hook_list;
static U8							sensor_tmp121_count;
static TMP121_CTRL					sensor_tmp121_ctrl[TMP121_COUNT];
static SENSOR_STRUCT				sensor_tmp121_struct[TMP121_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTiTmp121_IsPresent(SENSOR_ID sensor_id)
{
	TMP121_CTRL*	tmp121_ctrl_ptr = &sensor_tmp121_ctrl[sensor_id];

	tmp121_ctrl_ptr->is_present = FALSE;
    if(DrvSpiMasterDevice_SelectReadData(tmp121_ctrl_ptr->spi_device_id, tmp121_ctrl_ptr->data, 2))
    {
        LOG_DEV("tmp121 data %02h", PU8A(tmp121_ctrl_ptr->data, 2));
        tmp121_ctrl_ptr->is_present = (BOOL)((tmp121_ctrl_ptr->data[0] | tmp121_ctrl_ptr->data[1]) > 0);
    }
    return tmp121_ctrl_ptr->is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTiTmp121_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	TMP121_CTRL*		tmp121_ctrl_ptr = &sensor_tmp121_ctrl[sensor_id];

	if((sensor_type == SENSOR_TYPE_TEMPERATURE) && (tmp121_ctrl_ptr->is_present))
	{
        MEMSET((VPTR)tmp121_ctrl_ptr->data, 0, 2);
        return DrvSpiMasterDevice_SelectReadData(tmp121_ctrl_ptr->spi_device_id, tmp121_ctrl_ptr->data, 2);
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorTiTmp121_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	TMP121_CTRL*		tmp121_ctrl_ptr = &sensor_tmp121_ctrl[sensor_id];
    S16                 temperature;

	if((sensor_type == SENSOR_TYPE_TEMPERATURE) && (tmp121_ctrl_ptr->is_present))
	{
        temperature = (S16)(((U16)tmp121_ctrl_ptr->data[0] << 8) | (U16)tmp121_ctrl_ptr->data[1]);
        temperature >>= 3;  // loose last 3 bits    [unit: .0625°C]
        temperature *= 5;   //                      [unit: .0125°C]
        temperature >>= 3;  //                      [unit: .1°C]
        
		*data_ptr = (U16)temperature;
		return TRUE;
	}
	return FALSE;
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorTiTmp121_Init(void)
{
	sensor_tmp121_hook_list.is_present_hook = DrvSensorTiTmp121_IsPresent;
	sensor_tmp121_hook_list.read_sensor_hook = DrvSensorTiTmp121_ReadSensor;
	sensor_tmp121_hook_list.get_value_hook = DrvSensorTiTmp121_GetValue;

	sensor_tmp121_count = 0;

	MEMSET((VPTR)sensor_tmp121_ctrl, 0, SIZEOF(sensor_tmp121_ctrl));
	MEMSET((VPTR)sensor_tmp121_struct, 0, SIZEOF(sensor_tmp121_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorTiTmp121_Register(SPI_DEVICE_ID spi_device_id)
{
    SPI_CONFIG_STRUCT   spi_cfg_struct = {240000, MODE_0, 8, FALSE};
	SENSOR_STRUCT*	    sensor_struct_ptr = &sensor_tmp121_struct[sensor_tmp121_count];
	TMP121_CTRL*		tmp121_ctrl_ptr = &sensor_tmp121_ctrl[sensor_tmp121_count];

	if(sensor_tmp121_count < TMP121_COUNT)
	{
        DrvSpiMasterDevice_Config(spi_device_id, &spi_cfg_struct);
        
		tmp121_ctrl_ptr->spi_device_id = spi_device_id;
		tmp121_ctrl_ptr->is_present = FALSE;

		sensor_struct_ptr->hook_list_ptr = &sensor_tmp121_hook_list;
		sensor_struct_ptr->sensor_id = sensor_tmp121_count;
		sensor_tmp121_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("SE95 count overrun");
	return NULL;
}
//================================================================================================//
