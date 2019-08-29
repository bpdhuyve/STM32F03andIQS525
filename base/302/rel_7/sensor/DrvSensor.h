//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Type and implementation independent prototypes and definitions for the sensor driver interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SENSOR__DRVSENSOR_H
#define SENSOR__DRVSENSOR_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_SENSOR_ID                       0xFF

#define DrvSensor_StartReadTemperature(x,y)     DrvSensor_StartRead(x, SENSOR_TYPE_TEMPERATURE, y)

#define DrvSensor_StartReadCo2Value(x,y)        DrvSensor_StartRead(x, SENSOR_TYPE_CO2, y)

#define DrvSensor_StartReadRhValue(x,y)         DrvSensor_StartRead(x, SENSOR_TYPE_RH, y)

#define DrvSensor_StartReadPressure(x,y)        DrvSensor_StartRead(x, SENSOR_TYPE_PRESSURE, y)

#define DrvSensor_GetTemperature(x,y)           DrvSensor_GetValue(x, SENSOR_TYPE_TEMPERATURE, (U16*)y)

#define DrvSensor_GetCo2Value(x,y)              DrvSensor_GetValue(x, SENSOR_TYPE_CO2, (U16*)y)

#define DrvSensor_GetRhValue(x,y)               DrvSensor_GetValue(x, SENSOR_TYPE_RH, (U16*)y)

#define DrvSensor_GetPressure(x,y)              DrvSensor_GetValue(x, SENSOR_TYPE_PRESSURE, (U16*)y)
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8                          SENSOR_ID;

typedef enum
{
    SENSOR_TYPE_TEMPERATURE,
    SENSOR_TYPE_CO2,
    SENSOR_TYPE_RH,
    SENSOR_TYPE_PRESSURE
}
SENSOR_TYPE;

typedef BOOL (*SENSOR_PRESENT_HOOK)(SENSOR_ID sensor_id);

typedef BOOL (*SENSOR_READ_SENSOR)(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);

typedef BOOL (*SENSOR_GET_VALUE)(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);

typedef struct
{
    SENSOR_PRESENT_HOOK             is_present_hook;
    SENSOR_READ_SENSOR        		read_sensor_hook;
    SENSOR_GET_VALUE             	get_value_hook;
}
SENSOR_HOOK_LIST;

typedef struct
{
    SENSOR_HOOK_LIST*	            hook_list_ptr;
    SENSOR_ID                       sensor_id;
}
SENSOR_STRUCT;

typedef SENSOR_STRUCT*              SENSOR_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvSensor_Init(void);

BOOL DrvSensor_DetectPresence(SENSOR_HNDL sensor_hndl);

BOOL DrvSensor_StartRead(SENSOR_HNDL sensor_hndl, SENSOR_TYPE sensor_type, BOOL wait_to_complete);

BOOL DrvSensor_GetValue(SENSOR_HNDL sensor_hndl, SENSOR_TYPE sensor_type, U16* data_ptr);

U16 DrvSensor_CompensateRhValueForTemperature(U16 rh_value, S16 temp_value, S16 temp_offset);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SENSOR__DRVSENSOR_H */
