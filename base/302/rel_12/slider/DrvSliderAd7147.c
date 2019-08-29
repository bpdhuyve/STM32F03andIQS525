//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// brief explanation
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRV_SLIDER_Ad7147_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"

#define CORELOG_LEVEL                                       LOG_LEVEL_NONE

//------------------------------------------------------------------------------------------------//
/// @brief  defines the touch pad where the slider starts from
#ifndef FIRST_SLIDER_PAD
    #define FIRST_SLIDER_PAD                                0
#endif
//------------------------------------------------------------------------------------------------//
/// @brief  defines how manny touch pads are used as slider pads (starting from FIRST_SLIDER_PAD)
#ifndef NR_OF_SLIDER_PADS
    #define NR_OF_SLIDER_PADS                               0xC   //12 slider pads
#endif
//------------------------------------------------------------------------------------------------//
#ifndef WEIGHT_FACTOR
    #define WEIGHT_FACTOR                                   1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef NR_OF_VALUES_IN_CALC
    #define NR_OF_VALUES_IN_CALC                            5   //averaging over 5 pads
#endif
//------------------------------------------------------------------------------------------------//
#ifndef AFE_OFFSET_CTRL
    #define AFE_OFFSET_CTRL                                 0x0000 //analog offset regulator
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SENSITIVITY_CTRL
    #define SENSITIVITY_CTRL                                0x2929 //indicates percentage of max en min average value that will result in touch detect
#endif
//------------------------------------------------------------------------------------------------//
#ifndef GENERIC_OFFSET_SLIDER
    #define GENERIC_OFFSET_SLIDER                           0x600 //starting expected max and min values, is updated by the chip
#endif
//------------------------------------------------------------------------------------------------//
#ifndef GENERIC_OFFSET_BUTTON
    #define GENERIC_OFFSET_BUTTON                           0x1150 //starting expected max and min values, is updated by the chip
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEVICE_ID
    #define AD7147_DEVICE_ID                                0x147
#endif
//------------------------------------------------------------------------------------------------//
#ifndef I2C_BUS_FREQ
    #define I2C_BUS_FREQ                                    400e3
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "slider\DrvSliderAd7147.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define NR_OF_CAP_PADS  12
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    REGISTER_PWR_CONTROL                    =   0x0000,
    REGISTER_STAGE_CAL_EN                   =   0x0001,
    REGISTER_AMB_COMP_CTRL0                 =   0x0002,
    REGISTER_AMB_COMP_CTRL1                 =   0x0003,
    REGISTER_AMB_COMP_CTRL2                 =   0x0004,
    REGISTER_STAGE_LOW_INT_ENABLE           =   0x0005,
    REGISTER_STAGE_HIGH_INT_ENABLE          =   0x0006,
    REGISTER_STAGE_COMPLETE_INT_ENABLE      =   0x0007,
    REGISTER_INT_LOW_STATUS                 =   0x0008,
    REGISTER_INT_HIGH_STATUS                =   0x0009,
    REGISTER_INT_COMPLETE_STATUS            =   0x000A,
    REGISTER_CDC_RESULT_S0                  =   0x000B,
    REGISTER_DEVID      	                =	0x0017,
    REGISTER_PROXIMITY_STATUS               =   0x0042,
    REGISTER_STAGE_CONN_STARTADDRESS        =   0x0080,
    REGISTER_AMBIENT_S0                     =   0x00F1,
    REGISTER_RESULT_S0                      =   0x00E0

}REGISTER_ADDRESS;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U16  ReadRegister(U16 registeradress);
static void WriteRegister(U16 regadress,U16 value);
static void WriteConfigurationBank(void);
static void WriteControlBank(void);
//static void Delay(U32 value);
static void DeviceInit(void);
static void GetSliderPadValues(U16 * slider_pad_values_ptr);
static U16  CalculateSliderPosition(U16 * slider_pad_values_ptr);
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr);
static BOOL CheckDeviceId(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static I2C_DEVICE_ID                    i2c_device_hndl = NULL;
static SLIDER_STRUCT                    slider_struct;
static U16                              slider_pads_mask = 0;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// READ normal I2C registers of AD7147
static U16 ReadRegister(U16 registeradress)
{
    BOOL retval;
    static U8 databuffer[2];
    databuffer[1] = registeradress & 0xFF; //LSB
    databuffer[0] = registeradress>>8;  //MSB
    retval = DrvI2cMasterDevice_WriteData(i2c_device_hndl, &databuffer[0], 2, TRUE);
    if (retval)
    {
        retval = DrvI2cMasterDevice_ReadData(i2c_device_hndl, databuffer, 2, TRUE);
    }

    if (!retval)
    {
        LOG_WRN("failed register read");
        return 0;
    }
    return databuffer[0]<<8|databuffer[1];
}
//------------------------------------------------------------------------------------------------//
// Write normal I2C registers of AD7147 (2bytes adress + 2 bytes value)
static void WriteRegister(U16 regadress,U16 value)
{
    static U8 databuffer[4];
    BOOL retval;
    databuffer[1]= regadress & 0xFF; //LSB address
    databuffer[0]= regadress>>8; //MSB address
    databuffer[3]=  value & 0xFF; //LSB value
    databuffer[2]=  value>>8; //MSB value
    retval = DrvI2cMasterDevice_WriteData(i2c_device_hndl, databuffer, 4, TRUE);
    if (!retval)
    {
        LOG_WRN("failed register write");
    }
}
//------------------------------------------------------------------------------------------------//
/*static void Delay(U32 value)
{
    //dumb waiting Delay used when configuring device
    value = value * 0x2000 / 100;
    U32 i=0;
    for(i=0;i<value;i++){}
}*/
//------------------------------------------------------------------------------------------------//
//Configure the AD7147 according to datasheet power-up sequence (and demo board)
static void DeviceInit(void)
{
    //WriteRegister(0x0000, 0x0400); //SW reset, outcommented because, 1.its not really needed, 2.The ad7147 does not generates a proper ACK after this last databyte, this causes a bus error on the stm32 side
    //Delay(1500);	//wait time for sw reset, necessary?,
    WriteConfigurationBank(); //bank2
    WriteControlBank(); //bank1
}
//------------------------------------------------------------------------------------------------//
//configure bank2 of AD7147, is specific for each application (see comment for connections)
static void WriteConfigurationBank()
{
    U16 i=0;
    U32 stage_setup[NR_OF_CAP_PADS];

    /*SS-CAP SETUP*/
    stage_setup[0] = 0x1FFFFFFE;//connect CIN0 to +
    stage_setup[1] = 0x1FFFFFFB;//connect CIN1 to +
    stage_setup[2] = 0x1FFFFFEF;//connect CIN2 to +
    stage_setup[3] = 0x1FFFFFBF;//connect CIN3 to +
    stage_setup[4] = 0x1FFFFEFF;//connect CIN4 to +
    stage_setup[5] = 0x1FFFFBFF;//connect CIN5 to +
    stage_setup[6] = 0x1FFFEFFF;//connect CIN6 to +
    stage_setup[7] = 0x1FFEFFFF;//connect CIN7 to +
    stage_setup[8] = 0x1FFBFFFF;//connect CIN8 to +
    stage_setup[9] = 0x1FEFFFFF;//connect CIN9 to +
    stage_setup[10] = 0x1FBFFFFF;//connect CIN10 to +
    stage_setup[11] = 0x1EFFFFFF;//connect CIN11 to +
    // stage_setup[12] = 0x1BFFFFFF;//CIN12 not used


    U16 current_start_address = REGISTER_STAGE_CONN_STARTADDRESS;
    for(i = 0; i < NR_OF_CAP_PADS; i++ )
    {
        WriteRegister(current_start_address, stage_setup[i] & 0xFFFF); //write setup CIN[6:0];
        WriteRegister((current_start_address+1), stage_setup[i]>>16); //write setup CIN[12:4];
        WriteRegister((current_start_address+2), AFE_OFFSET_CTRL);  // AFE offset
        WriteRegister((current_start_address+3), SENSITIVITY_CTRL); //sensitivity
        WriteRegister((current_start_address+4), GENERIC_OFFSET_SLIDER); //offset low
        WriteRegister((current_start_address+5), GENERIC_OFFSET_SLIDER); //offset high
        WriteRegister((current_start_address+6), GENERIC_OFFSET_SLIDER); // offset high clamp
        WriteRegister((current_start_address+7), GENERIC_OFFSET_SLIDER); //offset low clamp
        current_start_address += 8;
    }
}
//------------------------------------------------------------------------------------------------//
//configure bank1 of AD7147
static void WriteControlBank()
{
      WriteRegister(REGISTER_PWR_CONTROL, 0x82B0); // powermode = full; lp_conv_delay = 220ms; seq_stage_num = 12; decimation = 64; sw_reset = 0, int_pol = active low; ext_source=enabled; cdc_bias= normal+35%, decimatio nverlaagd naar minimum omdat dit de touch beter deed werken
      WriteRegister(REGISTER_STAGE_CAL_EN, 0x0000); //set temporary to 0
      WriteRegister(REGISTER_AMB_COMP_CTRL0, 0x3230); //[3:0] = 0 => no sequence results skipped, [7:4]/[11:8] => calibration period in FP/LP mode; FP to LP timeout ctrl = 2xFP_PROXIMITY_CNT; forced cal control is normal; conversion reset is off;
      WriteRegister(REGISTER_AMB_COMP_CTRL1, 0x0619); //bits [13:8] represent proximity sensitivity, lower is more sensitive (min=1, default=4); bits [7:0] are prox. recalibration level
      WriteRegister(REGISTER_AMB_COMP_CTRL2, 0x0832); //proximity recalibration time control
      WriteRegister(REGISTER_STAGE_LOW_INT_ENABLE, 0x0CFF); //all stages low INT enabled; gpio pin disabled;
      WriteRegister(REGISTER_STAGE_HIGH_INT_ENABLE, 0x0CFF); //all stages high INT enabled;
      WriteRegister(REGISTER_STAGE_COMPLETE_INT_ENABLE, 0x0000); //all stage conversion complete INT disabled;
      WriteRegister(REGISTER_STAGE_CAL_EN, 0x0FFF); //all stage calibration enabled; 3 samples skipped in FP mode; 0 samples skipped in LP mode
}
//------------------------------------------------------------------------------------------------//
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr)
{
    static U16 pad_values[NR_OF_SLIDER_PADS];

    if ((ReadRegister(REGISTER_INT_HIGH_STATUS) & slider_pads_mask) != 0 )//check for interrupt on the pads that are used for the slider
    {
        GetSliderPadValues(pad_values); //get stage values, referenced to ambient values
        *slider_value_ptr = CalculateSliderPosition(pad_values); //calculate position with received values
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
// get actual and ambient values of certain stages and returns the array position of the maximum value
// argument is a ptr to the array with the pad values
static void GetSliderPadValues(U16 * slider_pad_values_ptr)
{
    U16 i;
    U16 actual_values[NR_OF_SLIDER_PADS], ambient_values[NR_OF_SLIDER_PADS];

    for ( i = 0; i < NR_OF_SLIDER_PADS; i++ )
    {
        actual_values[i] = ReadRegister((REGISTER_RESULT_S0+((FIRST_SLIDER_PAD+i)*0x24)));
        ambient_values[i] = ReadRegister((REGISTER_AMBIENT_S0+((FIRST_SLIDER_PAD+i)*0x24))); // sequential ambient registers are 0x24 apart

        // (small) negative values are rounded to 0, negative values are theoretically impossible for the implemented slider
        if ( actual_values[i] > ambient_values[i] )
        {
            slider_pad_values_ptr[i] = actual_values[i] - ambient_values[i];
        }
        else
        {
            slider_pad_values_ptr[i] = 0;
        }
    }
}
//------------------------------------------------------------------------------------------------//
// Calculates and returns the position based on an array of values. Return value between 0 and FFFF
// argument is a ptr to the array with the pad values
static U16 CalculateSliderPosition(U16 * slider_pad_values_ptr)
{
    U16 slider_pos = 0;
    U32 relative_position;
    U16 pad_ranges = ( 0x10000 / NR_OF_SLIDER_PADS ) + 1; //0x10000 is full range
    U16 max_center_offset = ( pad_ranges / 2 );
    U16 range_extension = 0xBD0*2; //increased to avoid bump
    U16 difference;

    //search pad with the highest value
    U8 pad_with_higest_value = 0;
    for (U8 i = 0; i < NR_OF_SLIDER_PADS; i++ )
    {
        if(slider_pad_values_ptr[pad_with_higest_value] < slider_pad_values_ptr[i] )
        {
            pad_with_higest_value = i;
        }
    }

    U16 pad_before_higest_value = ( pad_with_higest_value + NR_OF_SLIDER_PADS-1 ) % NR_OF_SLIDER_PADS;
    U16 pad_after_higest_value = ( pad_with_higest_value + 1 ) % NR_OF_SLIDER_PADS;

    if( slider_pad_values_ptr[pad_after_higest_value] > slider_pad_values_ptr[pad_before_higest_value] )
    {
        difference = ( slider_pad_values_ptr[pad_after_higest_value] - slider_pad_values_ptr[pad_before_higest_value] );
        relative_position = ( difference * range_extension ) / ( slider_pad_values_ptr[pad_after_higest_value] ); //range_extension mult. is used here instead of the next line to void a round-down to 0 when dividing
        if ( relative_position > max_center_offset )
        {
            relative_position = max_center_offset;
        }
        slider_pos = ( pad_with_higest_value * pad_ranges ) + relative_position;
    }
    else
    {
        difference = ( slider_pad_values_ptr[pad_before_higest_value] - slider_pad_values_ptr[pad_after_higest_value] );
        relative_position = ( difference * range_extension ) / ( slider_pad_values_ptr[pad_before_higest_value] ); //range_extension mult. is used here instead of the next line to void a round-down to 0 when dividing
        if ( relative_position > max_center_offset )
        {
            relative_position = max_center_offset;
        }
        slider_pos = ( pad_with_higest_value * pad_ranges ) - relative_position;
    }

    return slider_pos;
}
//------------------------------------------------------------------------------------------------//
static BOOL CheckDeviceId(void)
{
    U16 Read_DevReg = ReadRegister(REGISTER_DEVID)>>4;

    if (Read_DevReg == AD7147_DEVICE_ID) //validate if address register is what we expect
    {
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
SLIDER_HNDL DrvSliderAd7147_Register(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
    if (i2c_device_hndl == NULL)    //register i2c device if not done yet
    {
        i2c_device_hndl = DrvI2cMasterDevice_Register(i2c_channel,address, I2C_BUS_FREQ);
    }

    //generate a mask that defines which pads are used for the slider
    U8 i;
    for(i = 0; i < NR_OF_SLIDER_PADS; i++)
    {
        slider_pads_mask |= (1<<(i+FIRST_SLIDER_PAD));
    }

    if (!CheckDeviceId())
    {
        LOG_WRN("wrong device id");
        return NULL;
    }

    DeviceInit();

    slider_struct.get_value_hook = GetSliderValue;//Address of function in the hook
    slider_struct.slider_id = 0;

    return &slider_struct;
}
//------------------------------------------------------------------------------------------------//
//returns true if connection is ok, false if not, function can be performed before DrvSliderAd7147_Register
BOOL DrvSliderAd7147_TestConnection(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
    if (i2c_device_hndl == NULL)    //register i2c device if not done yet
    {
        i2c_device_hndl = DrvI2cMasterDevice_Register(i2c_channel,address, I2C_BUS_FREQ);
    }
    return CheckDeviceId();
}
//------------------------------------------------------------------------------------------------//
U16 DrvSliderAd7147_GetProximityMask(void)
{
    return ReadRegister(REGISTER_PROXIMITY_STATUS) & 0xFFF;     //het proximity register geeft weer welke pads proximity detecteren , check for proximity on stage0->11
}
//------------------------------------------------------------------------------------------------//
U16 DrvSliderAd7147_GetTouchMask(void)
{
    return ReadRegister(REGISTER_INT_HIGH_STATUS) & 0xFFF;      //het proximity register geeft weer welke pads proximity detecteren , check for proximity on stage0->11
}
//================================================================================================//
