//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Commn Application Link Layer of the OpenTherm communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef OPENTHERM__COMMALOPENTHERMCOMMON_H
#define OPENTHERM__COMMALOPENTHERMCOMMON_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef OPENTHERM_MEMBER_ID_CODE
	#define OPENTHERM_MEMBER_ID_CODE            0xFF
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define OPENTHERM_PROTOCOL_VERSION              0x0400
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef S16             F8_8;

typedef enum
{
    READ_OK                     = 0,
    READ_DATA_NOT_AVAILABLE     = 1,
    READ_UNKNOWN_DATAID         = 2,
}
DATA_READ_RESPONSE;

typedef enum
{
    WRITE_OK                    = 0,
    WRITE_DATA_INVALID          = 1,
    WRITE_UNKNOWN_DATAID        = 2,
}
DATA_WRITE_RESPONSE;

typedef enum
{
    SOLAR_MODE_OFF                      = 0,
    SOLAR_MODE_DHW_ECO                  = 1,
    SOLAR_MODE_DHW_COMFORT              = 2,
    SOLAR_MODE_DHW_SINGLE_BOOST         = 3,
    SOLAR_MODE_DHW_CONTINUOUS_BOOST     = 4,
}
SOLAR_MODE;

typedef enum
{
    SOLAR_STATUS_STANDBY                = 0,
    SOLAR_STATUS_LOADING_BY_SUN         = 1,
    SOLAR_STATUS_LOADING_BY_BOILER      = 2,
    SOLAR_STATUS_ANTI_LEGIONELLA        = 3,
}
SOLAR_STATUS;

typedef enum
{
    CONTROL_TYPE_MODULATING             = 0,
    CONTROL_TYPE_ONOFF                  = 1,
}
CONTROL_TYPE;

typedef enum
{
    VENT_SYSTYPE_CENTRAL_EXHAUST        = 0,
    VENT_SYSTYPE_HEAT_RECOVERY          = 1,
}
VENT_SYSTYPE;

typedef enum
{
    SPEED_CONTROL_3_WAY                 = 0,
    SPEED_CONTROL_VARIABLE              = 1,
}
SPEED_CONTROL;

typedef enum
{
    REQUEST_CODE_NORMAL                 =  0,   // Back to normal operation
    REQUEST_CODE_BLOR                   =  1,   // Boiler lock-out request
    REQUEST_CODE_CHWF                   =  2,   // CH water filling request
    REQUEST_CODE_SERVICE_MAX_POWER      =  3,   // In service mode: max power request
    REQUEST_CODE_SERVICE_MIN_POWER      =  4,   // In service mode: min power request
    REQUEST_CODE_SERVICE_SPARK_TEST     =  5,   // In service mode: spark test request
    REQUEST_CODE_SERVICE_FAN_MAX_SPEED  =  6,   // In service mode: max speed on fan request
    REQUEST_CODE_SERVICE_FAN_MIN_SPEED  =  7,   // In service mode: min speed on fan request
    REQUEST_CODE_SERVICE_3WAY_TO_CH     =  8,   // In service mode: 3-way valve to CH
    REQUEST_CODE_SERVICE_3WAY_TO_DHW    =  9,   // In service mode: 3-way valve to DHW
    REQUEST_CODE_RESET_REQUEST_FLAG     = 10,   // Reset request flag
    REQUEST_CODE_SERVICE_TEST_1         = 11,   // OEM defined test
    REQUEST_CODE_AUTO_HYDR_AIR_PURGE    = 12,   // Automatic hydronic air purge
}
REQUEST_CODE;

typedef enum
{
    DOW_UNDEFINED                       = 0,
    DOW_MONDAY                          = 1,
    DOW_TUESDAY                         = 2,
    DOW_WEDNESDAY                       = 3,
    DOW_THURSDAY                        = 4,
    DOW_FRIDAY                          = 5,
    DOW_SATURDAY                        = 6,
    DOW_SUNDAY                          = 7,
}
DOW;

typedef enum
{
    BATTERY_INDICATION_NONE             = 0,
    BATTERY_INDICATION_LOW              = 1,
    BATTERY_INDICATION_NEARLY_LOW       = 2,
    BATTERY_INDICATION_NOT_LOW          = 3,
}
BATTERY_INDICATION;

typedef enum
{
    RF_SIGNAL_STRENGTH_NONE             = 0,
    RF_SIGNAL_STRENGTH_1                = 1,
    RF_SIGNAL_STRENGTH_2                = 2,
    RF_SIGNAL_STRENGTH_3                = 3,
    RF_SIGNAL_STRENGTH_4                = 4,
    RF_SIGNAL_STRENGTH_5                = 5,
}
RF_SIGNAL_STRENGTH;

typedef enum
{
    TYPE_OF_SENSOR_ROOM_TEMP_CONTROL    =  0,
    TYPE_OF_SENSOR_ROOM_TEMP_SENSOR     =  1,
    TYPE_OF_SENSOR_OUTSIDE_TEMP_SENSOR  =  2,
    TYPE_OF_SENSOR_UNDEFINED            = 15,
}
TYPE_OF_SENSOR;

typedef enum
{
    CH_OPER_MODE_NO_OVERRIDE            = 0,
    CH_OPER_MODE_AUTO                   = 1,        // time switch program
    CH_OPER_MODE_COMFORT                = 2,
    CH_OPER_MODE_PRECOMFORT             = 3,
    CH_OPER_MODE_REDUCED                = 4,
    CH_OPER_MODE_PROTECTION             = 5,
    CH_OPER_MODE_OFF                    = 6,
}
CH_OPER_MODE;

typedef enum
{
    DHW_OPER_MODE_NO_OVERRIDE           = 0,
    DHW_OPER_MODE_AUTO                  = 1,        // time switch program
    DHW_OPER_MODE_ANTI_LEGIONELLA       = 2,
    DHW_OPER_MODE_COMFORT               = 3,
    DHW_OPER_MODE_REDUCED               = 4,
    DHW_OPER_MODE_PROTECTION            = 5,
    DHW_OPER_MODE_OFF                   = 6,
}
DHW_OPER_MODE;

typedef union
{
    // CLASS 1 - CONTROL AND STATUS INFORMATION
    
    struct
    {
        BOOL    slave_fault_indication          : 1;
        BOOL    slave_ch_mode                   : 1;
        BOOL    slave_dhw_mode                  : 1;
        BOOL    slave_flame_status              : 1;
        BOOL    slave_cooling_status            : 1;
        BOOL    slave_ch2_mode                  : 1;
        BOOL    slave_diagnostics_indication    : 1;
        BOOL    slave_electricity_production    : 1;
        
        BOOL    master_ch_mode                  : 1;
        BOOL    master_dhw_mode                 : 1;
        BOOL    master_cooling_enable           : 1;
        BOOL    master_otc_enable               : 1;
        BOOL    master_ch2_enable               : 1;
        BOOL    master_summer_mode              : 1;
        BOOL    master_dhw_blocking             : 1;
        U8      master_reserved                 : 1;
    }
    id_000;     // OT_DATA_ID_000_MASTER_SLAVE_STATUS;
    
    struct
    {
        BOOL    slave_fault_indication          : 1;
        BOOL    slave_ventilation_mode          : 1;
        BOOL    slave_bypass_open               : 1;
        BOOL    slave_bypass_automatic          : 1;
        BOOL    slave_free_ventilation          : 1;
        U8      slave_reserved_1                : 1;
        BOOL    slave_diagnostics_indication    : 1;
        U8      slave_reserved_2                : 1;
        
        BOOL    master_ventilation_enable       : 1;
        BOOL    master_bypass_open_in_manual    : 1;
        BOOL    master_bypass_automatic         : 1;
        BOOL    master_free_ventilation         : 1;
        U8      master_reserved                 : 4;
    }
    id_070;     // OT_DATA_ID_070_MASTER_SLAVE_VENTILATION_STATUS;
    
    struct
    {
        BOOL    slave_fault_indication          : 1;
        SOLAR_MODE      slave_solar_mode        : 3;
        SOLAR_STATUS    slave_solar_status      : 2;
        U8      slave_reserved_1                : 2;
        
        SOLAR_MODE      master_solar_mode       : 3;
        U8      master_reserved                 : 5;
    }
    id_101;     // OT_DATA_ID_101_MASTER_SLAVE_SOLAR_STORAGE_STATUS;
    
    struct
    {
        F8_8    setpoint_in_degrees_c;
    }
    id_001;     // OT_DATA_ID_001_CONTROL_SETPOINT
    
    struct
    {
        U8      ventilation_position;
        U8      reserved;
    }
    id_071;     // OT_DATA_ID_071_CONTROL_SETPOINT_VENTILATION
    
    struct
    {
        BOOL    service_request                 : 1;
        BOOL    lockout_reset                   : 1;
        BOOL    low_water_pressure              : 1;
        BOOL    gas_flame_fault                 : 1;
        BOOL    air_pressure_fault              : 1;
        BOOL    water_overtemperature           : 1;
        U8      reserved                        : 2;
        
        U8      oem_fault_code;
    }
    id_005;     // OT_DATA_ID_005_APPLICATION_FAULT_CODES
    
    struct
    {
        BOOL    service_request                 : 1;
        BOOL    exhaust_fan_fault               : 1;
        BOOL    inlet_fan_fault                 : 1;
        BOOL    frost_protection_active         : 1;
        U8      reserved                        : 4;
        
        U8      oem_fault_code;
    }
    id_072;     // OT_DATA_ID_072_APPLICATION_FAULT_CODES_VENTILATION
    
    struct
    {
        U8      reserved;
        U8      oem_fault_code;
    }
    id_102;     // OT_DATA_ID_102_APPLICATION_FAULT_CODES_SOLAR_STORAGE
    
    struct
    {
        F8_8    setpoint_in_degrees_c;
    }
    id_008;     // OT_DATA_ID_008_CONTROL_SETPOINT_2
    
    struct
    {
        U16     diagnostics_code;
    }
    id_115;     // OT_DATA_ID_115_OEM_DIAGNOSTICS_CODE
    
    struct
    {
        U16     diagnostics_code;
    }
    id_073;     // OT_DATA_ID_073_OEM_DIAGNOSTICS_CODE_VENTILATION
    
    /*----------------------------------------------------*/
    
    // CLASS 2 - CONFIGURATION INFORMATION
    
    struct
    {
        U8      member_id_code;
        
        BOOL    smart_power_implemented         : 1;
        U8      reserved                        : 7;
    }
    id_002;     // OT_DATA_ID_002_MASTER_CONFIG_AND_ID
    
    struct
    {
        U8      member_id_code;
        
        BOOL    dhw_present                     : 1;
        CONTROL_TYPE    control_type            : 1;
        BOOL    cooling_supported               : 1;
        BOOL    dhw_storage_tank_present        : 1;
        BOOL    master_pump_control_allowed     : 1;
        BOOL    ch2_present                     : 1;
        BOOL    remote_water_filling_function   : 1;
        BOOL    heat_cool_control_by_slave      : 1;
    }
    id_003;     // OT_DATA_ID_003_SLAVE_CONFIG_AND_ID
    
    struct
    {
        
        U8      member_id_code;
        
        VENT_SYSTYPE    ventilation_system      : 1;
        BOOL    bypass_present                  : 1;
        SPEED_CONTROL   speed_control           : 1;
        U8      reserved                        : 5;
    }
    id_074;     // OT_DATA_ID_074_VENTILATION_CONFIG_AND_ID
    
    struct
    {
        
        U8      member_id_code;
        
        BOOL    dwh_parallel_system             : 1;
        U8      reserved                        : 7;
    }
    id_103;     // OT_DATA_ID_103_SOLAR_STORAGE_CONFIG_AND_ID
    
    struct
    {
        F8_8    master_opentherm_protocol_version;
    }
    id_124;     // OT_DATA_ID_124_MASTER_OPENTHERM_PROTOCOL_VERSION
    
    struct
    {
        F8_8    slave_opentherm_protocol_version;
    }
    id_125;     // OT_DATA_ID_125_SLAVE_OPENTHERM_PROTOCOL_VERSION
    
    struct
    {
        F8_8    vent_opentherm_protocol_version;
    }
    id_075;     // OT_DATA_ID_075_VENTILATION_OPENTHERM_PROTOCOL_VERSION
    
    struct
    {
        U8      master_product_version;
        U8      master_product_type;
    }
    id_126;     // OT_DATA_ID_126_MASTER_PRODUCT_TYPE_AND_VERSION
    
    struct
    {
        U8      slave_product_version;
        U8      slave_product_type;
    }
    id_127;     // OT_DATA_ID_127_SLAVE_PRODUCT_TYPE_AND_VERSION
    
    struct
    {
        U8      vent_product_version;
        U8      vent_product_type;
    }
    id_076;     // OT_DATA_ID_076_VENTILATION_PRODUCT_TYPE_AND_VERSION
    
    struct
    {
        U8      solar_product_version;
        U8      solar_product_type;
    }
    id_104;     // OT_DATA_ID_104_SOLAR_STORAGE_PRODUCT_TYPE_AND_VERSION
    
    
    /*----------------------------------------------------*/
    
    // CLASS 3 - REMOTE REQUEST
    
    struct
    {
        U8      reponse_code                    : 7;
        BOOL    reponse_accepted                : 1;
        
        REQUEST_CODE    request_code;
    }
    id_004;     // OT_DATA_ID_004_REMOTE_REQUEST
    
    /*----------------------------------------------------*/
    
    // CLASS 4 - SENSOR AND INFORMATIONAL DATA
    
    struct
    {
        F8_8    room_setpoint_in_degrees_c;
    }
    id_016;     // OT_DATA_ID_016_ROOM_SETPOINT
    
    struct
    {
        F8_8    relative_modulation_level;
    }
    id_017;     // OT_DATA_ID_017_MODULATION_LEVEL
    
    struct
    {
        F8_8    ch_water_pressure_in_bar;
    }
    id_018;     // OT_DATA_ID_018_CH_WATER_PRESSURE
    
    struct
    {
        F8_8    dhw_flowrate_in_liter_per_min;
    }
    id_019;     // OT_DATA_ID_019_DHW_FLOW_RATE
    
    struct
    {
        U8      minutes;
        U8      hours                           : 5;
        DOW     day_of_week                     : 3;
    }
    id_020;     // OT_DATA_ID_020_DAY_OF_WEEK_AND_TIME_OF_DAY
    
    struct
    {
        U8      day_of_month;                           // 1-31
        U8      month;                                  // 1-12
    }
    id_021;     // OT_DATA_ID_021_DATE
    
    struct
    {
        U16     year;
    }
    id_022;     // OT_DATA_ID_022_YEAR
    
    struct
    {
        F8_8    room_setpoint_ch2_in_degrees_c;
    }
    id_023;     // OT_DATA_ID_023_ROOM_SETPOINT_CH2
    
    struct
    {
        F8_8    current_room_temperature_in_degrees_c;
    }
    id_024;     // OT_DATA_ID_024_CURRENT_ROOM_TEMPERATURE
    
    struct
    {
        F8_8    boiler_water_temperature_in_degrees_c;
    }
    id_025;     // OT_DATA_ID_025_BOILER_WATER_TEMPERATURE
    
    struct
    {
        F8_8    dhw_temperature_in_degrees_c;
    }
    id_026;     // OT_DATA_ID_026_DHW_TEMPERATURE
    
    struct
    {
        F8_8    outside_air_temperature_in_degrees_c;
    }
    id_027;     // OT_DATA_ID_027_OUTSIDE_AIR_TEMPERATURE
    
    struct
    {
        F8_8    return_water_temperature_in_degrees_c;
    }
    id_028;     // OT_DATA_ID_028_RETURN_WATER_TEMPERATURE
    
    struct
    {
        F8_8    solar_storage_temperature_in_degrees_c;
    }
    id_029;     // OT_DATA_ID_029_SOLAR_STORAGE_TEMPERATURE
    
    struct
    {
        S16     solar_collector_temperature_in_degrees_c;
    }
    id_030;     // OT_DATA_ID_030_SOLAR_COLLECTOR_TEMPERATURE
    
    struct
    {
        F8_8    ch2_flow_temperature_in_degrees_c;
    }
    id_031;     // OT_DATA_ID_031_CH2_FLOW_TEMPERATURE
    
    struct
    {
        F8_8    dhw2_temperature_in_degrees_c;
    }
    id_032;     // OT_DATA_ID_032_DHW2_TEMPERATURE
    
    struct
    {
        S16     exhaust_temperature_in_degrees_c;
    }
    id_033;     // OT_DATA_ID_033_EXHAUST_TEMPERATURE
    
    struct
    {
        F8_8    boiler_heat_exchanger_temperature_in_degrees_c;
    }
    id_034;     // OT_DATA_ID_034_BOILER_HEAT_EXHANGER_TEMPERATURE
    
    struct
    {
        U8      actual_boiler_fan_speed_in_hertz;
        U8      setpoint_boiler_fan_speed_in_hertz;
    }
    id_035;     // OT_DATA_ID_035_BOILER_FAN_SPEED
    
    struct
    {
        F8_8    electric_current_through_flame_in_microamp;
    }
    id_036;     // OT_DATA_ID_036_FLAME_CURRENT
    
    struct
    {
        F8_8    ch2_current_room_temperature_in_degrees_c;
    }
    id_037;     // OT_DATA_ID_037_CH2_CURRENT_ROOM_TEMPERATURE
    
    struct
    {
        F8_8    relative_humidity_as_percentage;
    }
    id_038;     // OT_DATA_ID_038_RELATIVE_HUMIDITY
    
    struct
    {
        U8      relative_ventilation_as_percentage;
        U8      reserved;
    }
    id_077;     // OT_DATA_ID_077_RELATIVE_VENTILATION
    
    struct
    {
        U8      relative_humidity_as_percentage;
        U8      reserved;
    }
    id_078;     // OT_DATA_ID_078_RELATIVE_HUMIDITY_EXHAUST_AIR
    
    struct
    {
        U16     co2_level_in_ppm;
    }
    id_079;     // OT_DATA_ID_079_CO2_LEVEL
    
    struct
    {
        F8_8    supply_inlet_temperature_in_degrees_c;
    }
    id_080;     // OT_DATA_ID_080_SUPPLY_INLET_TEMPERATURE
    
    struct
    {
        F8_8    supply_outlet_temperature_in_degrees_c;
    }
    id_081;     // OT_DATA_ID_081_SUPPLY_OUTLET_TEMPERATURE
    
    struct
    {
        F8_8    exhaust_inlet_temperature_in_degrees_c;
    }
    id_082;     // OT_DATA_ID_082_EXHAUST_INLET_TEMPERATURE
    
    struct
    {
        F8_8    exhaust_outlet_temperature_in_degrees_c;
    }
    id_083;     // OT_DATA_ID_083_EXHAUST_OUTLET_TEMPERATURE
    
    struct
    {
        U16     actual_exhaust_fan_speed_in_rpm;
    }
    id_084;     // OT_DATA_ID_084_EXHAUST_FAN_SPEED
    
    struct
    {
        U16     actual_inlet_fan_speed_in_rpm;
    }
    id_085;     // OT_DATA_ID_085_INLET_FAN_SPEED
    
    struct
    {
        BATTERY_INDICATION  battery_inducation  : 2;
        RF_SIGNAL_STRENGTH  rf_signal_strength  : 3;
        U8      reserved                        : 3;
        
        U8      sensor_index                    : 4;
        TYPE_OF_SENSOR      type_of_sensor      : 4;
    }
    id_098;     // OT_DATA_ID_098_SENSOR_INFO
    
    struct
    {
        U16     electricity_producer_starts;
    }
    id_109;     // OT_DATA_ID_109_ELECTRICITY_PRODUCER_STARTS
    
    struct
    {
        U16     electricity_producer_hours;
    }
    id_110;     // OT_DATA_ID_110_ELECTRICITY_PRODUCER_HOURS
    
    struct
    {
        U16     electricity_production_in_watt;
    }
    id_111;     // OT_DATA_ID_111_ELECTRICITY_PRODUCTION
    
    struct
    {
        U16     cumulative_electricity_production_in_kwh;
    }
    id_112;     // OT_DATA_ID_112_CUMULATIVE_ELECTRICITY_PRODUCTION
    
    struct
    {
        U16     number_of_unsuccessful_burner_starts;
    }
    id_113;     // OT_DATA_ID_113_NUMBER_OF_UNSUCCESSFUL_BURNER_STARTS
    
    struct
    {
        U16     number_of_times_flame_signal_too_low;
    }
    id_114;     // OT_DATA_ID_114_NUMBER_OF_TIMES_FLAME_SIGNAL_TOO_LOW
    
    struct
    {
        U16     number_of_successful_burner_starts;
    }
    id_116;     // OT_DATA_ID_116_NUMBER_OF_SUCCESSFUL_BURNER_STARTS
    
    struct
    {
        U16     number_of_ch_pump_starts;
    }
    id_117;     // OT_DATA_ID_117_NUMBER_OF_CH_PUMP_STARTS
    
    struct
    {
        U16     number_of_dhw_pump_starts;
    }
    id_118;     // OT_DATA_ID_118_NUMBER_OF_DHW_PUMP_STARTS
    
    struct
    {
        U16     number_of_dhw_burner_starts;
    }
    id_119;     // OT_DATA_ID_119_NUMBER_OF_DHW_BURNER_STARTS
    
    struct
    {
        U16     burner_operation_hours;
    }
    id_120;     // OT_DATA_ID_120_BURNER_OPERATION_HOURS
    
    struct
    {
        U16     ch_pump_operation_hours;
    }
    id_121;     // OT_DATA_ID_121_CH_PUMP_OPERATION_HOURS
    
    struct
    {
        U16     dhw_pump_operation_hours;
    }
    id_122;     // OT_DATA_ID_122_DHW_PUMP_OPERATION_HOURS
    
    struct
    {
        U16     dhw_burner_operation_hours;
    }
    id_123;     // OT_DATA_ID_123_DHW_BURNER_OPERATION_HOURS
    
    /*----------------------------------------------------*/
    
    // CLASS 5 - PRE-DEFINED REMOTE BOILER PARAMETERS
    
    struct
    {
        BOOL    read_write_dhw_setpoint         : 1;
        BOOL    read_write_max_ch_setpoint      : 1;
        U8      reserved1                       : 6;
        
        BOOL    transfer_enabled_dhw_setpoint   : 1;
        BOOL    transfer_enabled_max_ch_setpoint: 1;
        U8      reserved2                       : 6;
    }
    id_006;     // OT_DATA_ID_006_REMOTE_TRANSFER
    
    struct
    {
        BOOL    read_write_nom_vent_value       : 1;
        U8      reserved1                       : 6;
        
        BOOL    transfer_enabled_nom_vent_value : 1;
        U8      reserved2                       : 6;
    }
    id_086;     // OT_DATA_ID_086_REMOTE_TRANSFER_VENTILATION
    
    struct
    {
        S8      dhw_setpoint_low_bound_in_degrees;
        S8      dhw_setpoint_high_bound_in_degrees;
    }
    id_048;     // OT_DATA_ID_048_DHW_SETPOINT_BOUNDS
    
    struct
    {
        S8      max_ch_setpoint_low_bound_in_degrees;
        S8      max_ch_setpoint_high_bound_in_degrees;
    }
    id_049;     // OT_DATA_ID_049_MAX_CH_SETPOINT_BOUNDS
    
    struct
    {
        F8_8    dhw_setpoint_in_degrees;
    }
    id_056;     // OT_DATA_ID_056_DHW_SETPOINT
    
    struct
    {
        F8_8    max_ch_setpoint_in_degrees;
    }
    id_057;     // OT_DATA_ID_057_MAX_CH_SETPOINT
    
    struct
    {
        U8      reserved;
        U8      nom_vent_value_as_percentage;
    }
    id_087;     // OT_DATA_ID_087_NOMINAL_VENTILATION_VALUE
    
    /*----------------------------------------------------*/
    
    // CLASS 6 - TRANSPARENT SLAVE PARAMETERS (TSP)
    
    struct
    {
        U8      reserved;
        U8      number_of_tsp;
    }
    id_010;     // OT_DATA_ID_010_NUMBER_OF_TSP
    
    struct
    {
        U8      tsp_value;
        U8      tsp_index;
    }
    id_011;     // OT_DATA_ID_011_TSP_READ_WRITE
    
    struct
    {
        U8      reserved;
        U8      number_of_tsp_vent;
    }
    id_088;     // OT_DATA_ID_088_NUMBER_OF_TSP_VENTILATION
    
    struct
    {
        U8      tsp_vent_value;
        U8      tsp_vent_index;
    }
    id_089;     // OT_DATA_ID_089_TSP_VENTILATION_READ_WRITE
    
    struct
    {
        U8      reserved;
        U8      number_of_tsp_solar;
    }
    id_105;     // OT_DATA_ID_105_NUMBER_OF_TSP_SOLAR_STORAGE
    
    struct
    {
        U8      tsp_solar_value;
        U8      tsp_solar_index;
    }
    id_106;     // OT_DATA_ID_106_TSP_SOLAR_STORAGE_READ_WRITE
    
    /*----------------------------------------------------*/
    
    // CLASS 7 - FAULT HISTORY DATA (FHD)
    
    struct
    {
        U8      reserved;
        U8      fault_buffer_size;
    }
    id_012;     // OT_DATA_ID_012_FAULT_BUFFER_SIZE
    
    struct
    {
        U8      fhd_value;
        U8      fhd_index;
    }
    id_013;     // OT_DATA_ID_013_FAULT_READ
    
    struct
    {
        U8      reserved;
        U8      fault_buffer_size_vent;
    }
    id_090;     // OT_DATA_ID_090_FAULT_BUFFER_SIZE_VENTILATION
    
    struct
    {
        U8      fhd_vent_value;
        U8      fhd_vent_index;
    }
    id_091;     // OT_DATA_ID_091_FAULT_READ_VENTILATION
    
    struct
    {
        U8      reserved;
        U8      fault_buffer_size_solar;
    }
    id_107;     // OT_DATA_ID_107_FAULT_BUFFER_SIZE_SOLAR_STORAGE
    
    struct
    {
        U8      fhd_solar_value;
        U8      fhd_solar_index;
    }
    id_108;     // OT_DATA_ID_108_FAULT_READ_STORAGE_STORAGE
    
    /*----------------------------------------------------*/
    
    // CLASS 8 - CONTROL OF SPECIAL APPLICATIONS
    
    struct
    {
        F8_8    signal_for_cooling_plant_as_percentage;
    }
    id_007;     // OT_DATA_ID_007_COOLING_CONTROL_SIGNAL
    
    struct
    {
        F8_8    max_relative_modulation_as_percentage;
    }
    id_014;     // OT_DATA_ID_014_MAX_RELATIVE_MODULATION_SETTING;
    
    struct
    {
        U8      min_relative_modulation_as_percentage;
        U8      max_boiler_capacity_in_kw;
    }
    id_015;     // OT_DATA_ID_015_MAX_BOILER_CAPACITY_AND_MIN_MODULATION
    
    struct
    {
        F8_8    remote_override_room_setpoint;
    }
    id_009;     // OT_DATA_ID_009_REMOTE_OVERRIDE_ROOM_SETPOINT
    
    struct
    {
        CH_OPER_MODE    ch1_operating_mode      : 4;
        CH_OPER_MODE    ch2_operating_mode      : 4;
        
        DHW_OPER_MODE   dhw_operating_mode      : 4;
        BOOL    manual_dhw_push                 : 1;
        U8      reserved                        : 3;
    }
    id_099;     // OT_DATA_ID_099_REMOTE_OVERRIDE_OPERATING_MODE_HEATING
    
    struct
    {
        BOOL    enable_overrule_by_manual_setpoint_change   : 1;
        BOOL    enable_overrule_by_program_setpoint_change  : 1;
        U8      reserved1                                   : 6;
        
        U8      reserved2;
    }
    id_100;     // OT_DATA_ID_100_REMOTE_OVERRIDE_ROOM_SETPOINT_FUNCTION
    
    /*----------------------------------------------------*/
    
    // GENERIC
    U16     u16_value;
}
OT_DATA_VALUE;

typedef enum
{
    // CLASS 1 - CONTROL AND STATUS INFORMATION
    OT_DATA_ID_000_MASTER_SLAVE_STATUS                      =   0,
    OT_DATA_ID_070_MASTER_SLAVE_VENTILATION_STATUS          =  70,
    OT_DATA_ID_101_MASTER_SLAVE_SOLAR_STORAGE_STATUS        = 101,
    OT_DATA_ID_001_CONTROL_SETPOINT                         =   1,
    OT_DATA_ID_071_CONTROL_SETPOINT_VENTILATION             =  71,
    OT_DATA_ID_005_APPLICATION_FAULT_CODES                  =   5,
    OT_DATA_ID_072_APPLICATION_FAULT_CODES_VENTILATION      =  72,
    OT_DATA_ID_102_APPLICATION_FAULT_CODES_SOLAR_STORAGE    = 102,
    OT_DATA_ID_008_CONTROL_SETPOINT_2                       =   8,
    OT_DATA_ID_115_OEM_DIAGNOSTICS_CODE                     = 115,
    OT_DATA_ID_073_OEM_DIAGNOSTICS_CODE_VENTILATION         =  73,
    
    // CLASS 2 - CONFIGURATION INFORMATION
    OT_DATA_ID_002_MASTER_CONFIG_AND_ID                     =   2,
    OT_DATA_ID_003_SLAVE_CONFIG_AND_ID                      =   3,
    OT_DATA_ID_074_VENTILATION_CONFIG_AND_ID                =  74,
    OT_DATA_ID_103_SOLAR_STORAGE_CONFIG_AND_ID              = 103,
    OT_DATA_ID_124_MASTER_OPENTHERM_PROTOCOL_VERSION        = 124,
    OT_DATA_ID_125_SLAVE_OPENTHERM_PROTOCOL_VERSION         = 125,
    OT_DATA_ID_075_VENTILATION_OPENTHERM_PROTOCOL_VERSION   =  75,
    OT_DATA_ID_126_MASTER_PRODUCT_TYPE_AND_VERSION          = 126,
    OT_DATA_ID_127_SLAVE_PRODUCT_TYPE_AND_VERSION           = 127,
    OT_DATA_ID_076_VENTILATION_PRODUCT_TYPE_AND_VERSION     =  76,
    OT_DATA_ID_104_SOLAR_STORAGE_PRODUCT_TYPE_AND_VERSION   = 104,
    
    // CLASS 3 - REMOTE REQUEST
    OT_DATA_ID_004_REMOTE_REQUEST                           =   4,
    
    // CLASS 4 - SENSOR AND INFORMATIONAL DATA
    OT_DATA_ID_016_ROOM_SETPOINT                            =  16,
    OT_DATA_ID_017_MODULATION_LEVEL                         =  17,
    OT_DATA_ID_018_CH_WATER_PRESSURE                        =  18,
    OT_DATA_ID_019_DHW_FLOW_RATE                            =  19,
    OT_DATA_ID_020_DAY_OF_WEEK_AND_TIME_OF_DAY              =  20,
    OT_DATA_ID_021_DATE                                     =  21,
    OT_DATA_ID_022_YEAR                                     =  22,
    OT_DATA_ID_023_ROOM_SETPOINT_CH2                        =  23,
    OT_DATA_ID_024_CURRENT_ROOM_TEMPERATURE                 =  24,
    OT_DATA_ID_025_BOILER_WATER_TEMPERATURE                 =  25,
    OT_DATA_ID_026_DHW_TEMPERATURE                          =  26,
    OT_DATA_ID_027_OUTSIDE_AIR_TEMPERATURE                  =  27,
    OT_DATA_ID_028_RETURN_WATER_TEMPERATURE                 =  28,
    OT_DATA_ID_029_SOLAR_STORAGE_TEMPERATURE                =  29,
    OT_DATA_ID_030_SOLAR_COLLECTOR_TEMPERATURE              =  30,
    OT_DATA_ID_031_CH2_FLOW_TEMPERATURE                     =  31,
    OT_DATA_ID_032_DHW2_TEMPERATURE                         =  32,
    OT_DATA_ID_033_EXHAUST_TEMPERATURE                      =  33,
    OT_DATA_ID_034_BOILER_HEAT_EXHANGER_TEMPERATURE         =  34,
    OT_DATA_ID_035_BOILER_FAN_SPEED                         =  35,
    OT_DATA_ID_036_FLAME_CURRENT                            =  36,
    OT_DATA_ID_037_CH2_CURRENT_ROOM_TEMPERATURE             =  37,
    OT_DATA_ID_038_RELATIVE_HUMIDITY                        =  38,
    OT_DATA_ID_077_RELATIVE_VENTILATION                     =  77,
    OT_DATA_ID_078_RELATIVE_HUMIDITY_EXHAUST_AIR            =  78,
    OT_DATA_ID_079_CO2_LEVEL                                =  79,
    OT_DATA_ID_080_SUPPLY_INLET_TEMPERATURE                 =  80,
    OT_DATA_ID_081_SUPPLY_OUTLET_TEMPERATURE                =  81,
    OT_DATA_ID_082_EXHAUST_INLET_TEMPERATURE                =  82,
    OT_DATA_ID_083_EXHAUST_OUTLET_TEMPERATURE               =  83,
    OT_DATA_ID_084_EXHAUST_FAN_SPEED                        =  84,
    OT_DATA_ID_085_INLET_FAN_SPEED                          =  85,
    OT_DATA_ID_098_SENSOR_INFO                              =  98,
    OT_DATA_ID_109_ELECTRICITY_PRODUCER_STARTS              = 109,
    OT_DATA_ID_110_ELECTRICITY_PRODUCER_HOURS               = 110,
    OT_DATA_ID_111_ELECTRICITY_PRODUCTION                   = 111,
    OT_DATA_ID_112_CUMULATIVE_ELECTRICITY_PRODUCTION        = 112,
    OT_DATA_ID_113_NUMBER_OF_UNSUCCESSFUL_BURNER_STARTS     = 113,
    OT_DATA_ID_114_NUMBER_OF_TIMES_FLAME_SIGNAL_TOO_LOW     = 114,
    OT_DATA_ID_116_NUMBER_OF_SUCCESSFUL_BURNER_STARTS       = 116,
    OT_DATA_ID_117_NUMBER_OF_CH_PUMP_STARTS                 = 117,
    OT_DATA_ID_118_NUMBER_OF_DHW_PUMP_STARTS                = 118,
    OT_DATA_ID_119_NUMBER_OF_DHW_BURNER_STARTS              = 119,
    OT_DATA_ID_120_BURNER_OPERATION_HOURS                   = 120,
    OT_DATA_ID_121_CH_PUMP_OPERATION_HOURS                  = 121,
    OT_DATA_ID_122_DHW_PUMP_OPERATION_HOURS                 = 122,
    OT_DATA_ID_123_DHW_BURNER_OPERATION_HOURS               = 123,
    
    // CLASS 5 - PRE-DEFINED REMOTE BOILER PARAMETERS
    OT_DATA_ID_006_REMOTE_TRANSFER                          =   6,
    OT_DATA_ID_086_REMOTE_TRANSFER_VENTILATION              =  86,
    OT_DATA_ID_048_DHW_SETPOINT_BOUNDS                      =  48,
    OT_DATA_ID_049_MAX_CH_SETPOINT_BOUNDS                   =  49,
    OT_DATA_ID_056_DHW_SETPOINT                             =  56,
    OT_DATA_ID_057_MAX_CH_SETPOINT                          =  57,
    OT_DATA_ID_087_NOMINAL_VENTILATION_VALUE                =  87,
    
    // CLASS 6 - TRANSPARENT SLAVE PARAMETERS (TSP)
    OT_DATA_ID_010_NUMBER_OF_TSP                            =  10,
    OT_DATA_ID_011_TSP_READ_WRITE                           =  11,
    OT_DATA_ID_088_NUMBER_OF_TSP_VENTILATION                =  88,
    OT_DATA_ID_089_TSP_VENTILATION_READ_WRITE               =  89,
    OT_DATA_ID_105_NUMBER_OF_TSP_SOLAR_STORAGE              = 105,
    OT_DATA_ID_106_TSP_SOLAR_STORAGE_READ_WRITE             = 106,
    
    // CLASS 7 - FAULT HISTORY DATA (FHD)
    OT_DATA_ID_012_FAULT_BUFFER_SIZE                        =  12,
    OT_DATA_ID_013_FAULT_READ                               =  13,
    OT_DATA_ID_090_FAULT_BUFFER_SIZE_VENTILATION            =  90,
    OT_DATA_ID_091_FAULT_READ_VENTILATION                   =  91,
    OT_DATA_ID_107_FAULT_BUFFER_SIZE_SOLAR_STORAGE          = 107,
    OT_DATA_ID_108_FAULT_READ_STORAGE_STORAGE               = 108,
    
    // CLASS 8 - CONTROL OF SPECIAL APPLICATIONS
    OT_DATA_ID_007_COOLING_CONTROL_SIGNAL                   =   7,
    OT_DATA_ID_014_MAX_RELATIVE_MODULATION_SETTING          =  14,
    OT_DATA_ID_015_MAX_BOILER_CAPACITY_AND_MIN_MODULATION   =  15,
    OT_DATA_ID_009_REMOTE_OVERRIDE_ROOM_SETPOINT            =   9,
    OT_DATA_ID_099_REMOTE_OVERRIDE_OPERATING_MODE_HEATING   =  99,
    OT_DATA_ID_100_REMOTE_OVERRIDE_ROOM_SETPOINT_FUNCTION   = 100,
}
OT_DATA_ID;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* OPENTHERM__COMMALOPENTHERMCOMMON_H */

