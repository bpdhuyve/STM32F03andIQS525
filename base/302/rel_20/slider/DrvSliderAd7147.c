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

#define CORELOG_LEVEL                                       LOG_LEVEL_TERM

//------------------------------------------------------------------------------------------------//
// general defines
//------------------------------------------------------------------------------------------------//
#ifndef DEVICE_ID
    #define AD7147_DEVICE_ID                                0x147
#endif
//------------------------------------------------------------------------------------------------//
#ifndef I2C_BUS_FREQ
    #define I2C_BUS_FREQ                                    400e3
#endif
//------------------------------------------------------------------------------------------------//
#ifndef NR_OF_USED_STAGES
    #define NR_OF_USED_STAGES                               12
#endif
//------------------------------------------------------------------------------------------------//
// slider position cal defines
//------------------------------------------------------------------------------------------------//
/// @brief  defines the touch pad where the slider starts from
#ifndef FIRST_SLIDER_PAD
    #define FIRST_SLIDER_PAD                                0
#endif
//------------------------------------------------------------------------------------------------//
/// @brief  defines how manny touch pads are used as slider pads (starting from FIRST_SLIDER_PAD)
#ifndef NR_OF_SLIDER_PADS
    #define NR_OF_SLIDER_PADS                               12
#endif
//------------------------------------------------------------------------------------------------//
#ifndef LINEAR_SLIDER
    #define LINEAR_SLIDER                                   0
#endif
//------------------------------------------------------------------------------------------------//
/// @brief  this value is unique for evry slider pattern
//          if this value is to low there will be jumps in the slider position while moving your finger
//          is this value is to high the slider will lock on too long on the center of a pad
#ifndef SLIDER_RANGE_EXTENTION_VALUE
    #define SLIDER_RANGE_EXTENTION_VALUE                    (0xBD0*2)       //old value used in previous revs of code, its best for new applications that you test this
#endif
//------------------------------------------------------------------------------------------------//
// defines used to setup wich pin is connected to wich stage
//------------------------------------------------------------------------------------------------//
#ifndef STAGE0_CONNECTION
    #define STAGE0_CONNECTION                               CIN0_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE1_CONNECTION
    #define STAGE1_CONNECTION                               CIN1_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE2_CONNECTION
    #define STAGE2_CONNECTION                               CIN2_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE3_CONNECTION
    #define STAGE3_CONNECTION                               CIN3_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE4_CONNECTION
    #define STAGE4_CONNECTION                               CIN4_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE5_CONNECTION
    #define STAGE5_CONNECTION                               CIN5_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE6_CONNECTION
    #define STAGE6_CONNECTION                               CIN6_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE7_CONNECTION
    #define STAGE7_CONNECTION                               CIN7_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE8_CONNECTION
    #define STAGE8_CONNECTION                               CIN8_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE9_CONNECTION
    #define STAGE9_CONNECTION                               CIN9_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE10_CONNECTION
    #define STAGE10_CONNECTION                              CIN10_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE11_CONNECTION
    #define STAGE11_CONNECTION                              CIN11_TO_POS
#endif
//------------------------------------------------------------------------------------------------//
// analog offset regulator register - trim these values until the ambient measurement is around 32767 !
//------------------------------------------------------------------------------------------------//
#ifdef AFE_OFFSET_CTRL
    #error obsolete define, use new STAGEx_AFE_OFFSET_REGISTER_VALUE defines to setup offset for each pad
#endif
//------------------------------------------------------------------------------------------------//
#define AFE_OFFSET_REGISTER_DEFAULT_VALUE                   0x0000
//------------------------------------------------------------------------------------------------//
#ifndef STAGE0_AFE_OFFSET_REGISTER_VALUE
    #define STAGE0_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE1_AFE_OFFSET_REGISTER_VALUE
    #define STAGE1_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE2_AFE_OFFSET_REGISTER_VALUE
    #define STAGE2_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE3_AFE_OFFSET_REGISTER_VALUE
    #define STAGE3_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE4_AFE_OFFSET_REGISTER_VALUE
    #define STAGE4_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE5_AFE_OFFSET_REGISTER_VALUE
    #define STAGE5_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE6_AFE_OFFSET_REGISTER_VALUE
    #define STAGE6_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE7_AFE_OFFSET_REGISTER_VALUE
    #define STAGE7_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE8_AFE_OFFSET_REGISTER_VALUE
    #define STAGE8_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE9_AFE_OFFSET_REGISTER_VALUE
    #define STAGE9_AFE_OFFSET_REGISTER_VALUE                AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE10_AFE_OFFSET_REGISTER_VALUE
    #define STAGE10_AFE_OFFSET_REGISTER_VALUE               AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE11_AFE_OFFSET_REGISTER_VALUE
    #define STAGE11_AFE_OFFSET_REGISTER_VALUE               AFE_OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
//sensitivity registers
//------------------------------------------------------------------------------------------------//
#ifdef SENSITIVITY_CTRL
    #error obsolete define, use new STAGEx_SENSITIVITY_REGISTER_VALUE defines to setup offset for each pad
#endif
//------------------------------------------------------------------------------------------------//
#define SENSITIVITY_REGISTER_DEFAULT_VALUE                  0x2929
//------------------------------------------------------------------------------------------------//
#ifndef STAGE0_SENSITIVITY_REGISTER_VALUE
    #define STAGE0_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE1_SENSITIVITY_REGISTER_VALUE
    #define STAGE1_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE2_SENSITIVITY_REGISTER_VALUE
    #define STAGE2_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE3_SENSITIVITY_REGISTER_VALUE
    #define STAGE3_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE4_SENSITIVITY_REGISTER_VALUE
    #define STAGE4_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE5_SENSITIVITY_REGISTER_VALUE
    #define STAGE5_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE6_SENSITIVITY_REGISTER_VALUE
    #define STAGE6_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE7_SENSITIVITY_REGISTER_VALUE
    #define STAGE7_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE8_SENSITIVITY_REGISTER_VALUE
    #define STAGE8_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE9_SENSITIVITY_REGISTER_VALUE
    #define STAGE9_SENSITIVITY_REGISTER_VALUE               SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE10_SENSITIVITY_REGISTER_VALUE
    #define STAGE10_SENSITIVITY_REGISTER_VALUE              SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE11_SENSITIVITY_REGISTER_VALUE
    #define STAGE11_SENSITIVITY_REGISTER_VALUE              SENSITIVITY_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
//offset high/low registers (not to be confused with afe offset, ze hadden beter deze registers anders genoemd) for explanation see page 28 of rev d datasheet
//deze registers bepalen de threshhold value volgens een formule in datasheet
//------------------------------------------------------------------------------------------------//
#ifdef GENERIC_OFFSET_SLIDER
    #error obsolete define, use new STAGEx_OFFSET_REGISTER_VALUE defines to setup offset for each pad
#endif
//------------------------------------------------------------------------------------------------//
#define OFFSET_REGISTER_DEFAULT_VALUE                  0x600    //starting expected max and min values, is updated by the chip
//------------------------------------------------------------------------------------------------//
#ifndef STAGE0_OFFSET_REGISTER_VALUE
    #define STAGE0_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE1_OFFSET_REGISTER_VALUE
    #define STAGE1_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE2_OFFSET_REGISTER_VALUE
    #define STAGE2_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE3_OFFSET_REGISTER_VALUE
    #define STAGE3_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE4_OFFSET_REGISTER_VALUE
    #define STAGE4_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE5_OFFSET_REGISTER_VALUE
    #define STAGE5_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE6_OFFSET_REGISTER_VALUE
    #define STAGE6_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE7_OFFSET_REGISTER_VALUE
    #define STAGE7_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE8_OFFSET_REGISTER_VALUE
    #define STAGE8_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE9_OFFSET_REGISTER_VALUE
    #define STAGE9_OFFSET_REGISTER_VALUE               OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE10_OFFSET_REGISTER_VALUE
    #define STAGE10_OFFSET_REGISTER_VALUE              OFFSET_REGISTER_DEFAULT_VALUE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef STAGE11_OFFSET_REGISTER_VALUE
    #define STAGE11_OFFSET_REGISTER_VALUE              OFFSET_REGISTER_DEFAULT_VALUE
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
#define NR_OF_STAGES    12
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    //---------- bank 1 ----------
    REGISTER_PWR_CONTROL                    =   0x000,
    REGISTER_STAGE_CAL_EN                   =   0x001,
    REGISTER_AMB_COMP_CTRL0                 =   0x002,
    REGISTER_AMB_COMP_CTRL1                 =   0x003,
    REGISTER_AMB_COMP_CTRL2                 =   0x004,
    REGISTER_STAGE_LOW_INT_ENABLE           =   0x005,
    REGISTER_STAGE_HIGH_INT_ENABLE          =   0x006,
    REGISTER_STAGE_COMPLETE_INT_ENABLE      =   0x007,
    REGISTER_STAGE_LOW_INT_STATUS           =   0x008,
    REGISTER_STAGE_HIGH_INT_STATUS          =   0x009,
    REGISTER_STAGE_COMPLETE_INT_STATUS      =   0x00A,
    REGISTER_STAGE0_CDC_RESULT              =   0x00B,
    //... continues upto stage11
    REGISTER_DEVID      	                =	0x017,
    REGISTER_STAGE0_PROXIMITY_STATUS        =   0x042,

    //---------- bank 2 = STAGE0 to STAGE11 Configuration Registers ----------
    REGISTER_STAGE0_CONNECTION_LOW          =   0x080,
    REGISTER_STAGE0_CONNECTION_HIGH         =   0x081,
    REGISTER_STAGE0_AFE_OFFSET              =   0x082,
    REGISTER_STAGE0_SENSITIVITY             =   0x083,
    REGISTER_STAGE0_OFFSET_LOW              =   0x084,
    REGISTER_STAGE0_OFFSET_HIGH             =   0x085,
    REGISTER_STAGE0_OFFSET_HIGH_CLAMP       =   0x086,
    REGISTER_STAGE0_OFFSET_LOW_CLAMP        =   0x087,
    //... continues upto stage11

    //---------- bank 3 results ----------
    REGISTER_STAGE0_CONV_DATA               =   0x0E0,
    REGISTER_STAGE0_SF_AMBIENT              =   0x0F1,
    REGISTER_STAGE0_HIGH_THRESHHOLD         =   0x0FA,
    REGISTER_STAGE0_LOW_THRESHOLD           =   0x101,

}REGISTER_ADDRESS;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U16 ReadRegister(U16 registeradress);
static void WriteRegister(U16 regadress,U16 value);
static void Delay(U32 value);
static void DeviceInit(void);
static void WriteConfigurationBank(void);
static void WriteControlBank(void);
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr);
static U16 GetPadValue(U8 stage_nr);
static U16 CalculateSliderPosition(U16 * slider_pad_values_ptr);
static BOOL CheckDeviceId(void);
static void ForcedCalibration(void);
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
static void Delay(U32 value)
{
    //dumb waiting Delay used when configuring device
    value = value * 0x2000 / 100;
    U32 i=0;
    for(i=0;i<value;i++){}
}
//------------------------------------------------------------------------------------------------//
//Configure the AD7147 according to datasheet power-up sequence (and demo board)
static void DeviceInit(void)
{
    WriteRegister(0x0000, 0x0400); //SW reset, outcommented because, 1.its not really needed, 2.The ad7147 does not generates a proper ACK after this last databyte, this causes a bus error on the stm32 side
    Delay(1500);	//wait time for sw reset, necessary?,
    WriteConfigurationBank(); //bank2
    WriteControlBank(); //bank1
    ForcedCalibration();
}
//------------------------------------------------------------------------------------------------//
//configure bank2 of AD7147, is specific for each application (see comment for connections)
static void WriteConfigurationBank(void)
{
    U16 i=0;

    //define wich stage is connected to wich pin of a stage
    U32 stage_connection_registers[NR_OF_STAGES];
    stage_connection_registers[0] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE0_CONNECTION));
    stage_connection_registers[1] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE1_CONNECTION));
    stage_connection_registers[2] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE2_CONNECTION));
    stage_connection_registers[3] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE3_CONNECTION));
    stage_connection_registers[4] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE4_CONNECTION));
    stage_connection_registers[5] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE5_CONNECTION));
    stage_connection_registers[6] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE6_CONNECTION));
    stage_connection_registers[7] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE7_CONNECTION));
    stage_connection_registers[8] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE8_CONNECTION));
    stage_connection_registers[9] =  (1<<(16+12))| (0xFFF3FFF & (~STAGE9_CONNECTION));
    stage_connection_registers[10] = (1<<(16+12))| (0xFFF3FFF & (~STAGE10_CONNECTION));
    stage_connection_registers[11] = (1<<(16+12))| (0xFFF3FFF & (~STAGE11_CONNECTION));

    //setup individual afe offset control per stage
    U16 stage_afe_offset_registers[NR_OF_STAGES];
    stage_afe_offset_registers[0] = STAGE0_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[1] = STAGE1_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[2] = STAGE2_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[3] = STAGE3_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[4] = STAGE4_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[5] = STAGE5_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[6] = STAGE6_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[7] = STAGE7_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[8] = STAGE8_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[9] = STAGE9_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[10] = STAGE10_AFE_OFFSET_REGISTER_VALUE;
    stage_afe_offset_registers[11] = STAGE11_AFE_OFFSET_REGISTER_VALUE;

    //setup individual sensitity per stage
    U16 stage_sensitivity_registers[NR_OF_STAGES];
    stage_sensitivity_registers[0] = STAGE0_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[1] = STAGE1_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[2] = STAGE2_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[3] = STAGE3_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[4] = STAGE4_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[5] = STAGE5_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[6] = STAGE6_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[7] = STAGE7_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[8] = STAGE8_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[9] = STAGE9_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[10] = STAGE10_SENSITIVITY_REGISTER_VALUE;
    stage_sensitivity_registers[11] = STAGE11_SENSITIVITY_REGISTER_VALUE;

    //setup individual stage high/low offset registers per stage
    U16 stage_offset_registers[NR_OF_STAGES];
    stage_offset_registers[0] = STAGE0_OFFSET_REGISTER_VALUE;
    stage_offset_registers[1] = STAGE1_OFFSET_REGISTER_VALUE;
    stage_offset_registers[2] = STAGE2_OFFSET_REGISTER_VALUE;
    stage_offset_registers[3] = STAGE3_OFFSET_REGISTER_VALUE;
    stage_offset_registers[4] = STAGE4_OFFSET_REGISTER_VALUE;
    stage_offset_registers[5] = STAGE5_OFFSET_REGISTER_VALUE;
    stage_offset_registers[6] = STAGE6_OFFSET_REGISTER_VALUE;
    stage_offset_registers[7] = STAGE7_OFFSET_REGISTER_VALUE;
    stage_offset_registers[8] = STAGE8_OFFSET_REGISTER_VALUE;
    stage_offset_registers[9] = STAGE9_OFFSET_REGISTER_VALUE;
    stage_offset_registers[10] = STAGE10_OFFSET_REGISTER_VALUE;
    stage_offset_registers[11] = STAGE11_OFFSET_REGISTER_VALUE;

    //first connect all stages to bias = all unsed pins must be connected to bias
    for(i = 0; i < NR_OF_STAGES; i++ )
    {
     WriteRegister(REGISTER_STAGE0_CONNECTION_LOW, 0x3FFF); //write setup CIN[6:0];
     WriteRegister((REGISTER_STAGE0_CONNECTION_LOW+1),0xFFFF); //write setup CIN[12:7];
    }

    //then connect & set up the used stages
    U16 current_start_address = REGISTER_STAGE0_CONNECTION_LOW;
    for(i = 0; i < NR_OF_USED_STAGES; i++ )
    {
        WriteRegister(current_start_address, 	 (stage_connection_registers[i])& 0xFFFF); //write setup CIN[6:0];
        WriteRegister((current_start_address+1), (stage_connection_registers[i]>>16)& 0xFFFF); //write setup CIN[12:7];
        WriteRegister((current_start_address+2), stage_afe_offset_registers[i]);      //AFE offset
        WriteRegister((current_start_address+3), stage_sensitivity_registers[i]);     //sensitivity
        WriteRegister((current_start_address+4), 0);//offset low
        WriteRegister((current_start_address+5), stage_offset_registers[i]);//offset high
        WriteRegister((current_start_address+6), (U16)(stage_offset_registers[i]*1.25));//offset high clamp    datasheet zegt Set STAGEx_OFFSET_HIGH register to 80% of the STAGEx_OFFSET_HIGH_CLAMP value. vandaar *1.25
        WriteRegister((current_start_address+7), 0);//offset low clamp
        current_start_address += 8;
    }
}
//------------------------------------------------------------------------------------------------//
//configure bank1 of AD7147
static void WriteControlBank(void)
{
    //voor swipe 0 = cdc bias op 0 gezet
      WriteRegister(REGISTER_PWR_CONTROL, /*0x82B0*/ (2<<14)|(0<<13)|(0<<12)|(0<<11)|(0<<10)|(2<<8)|(11<<4)|(0<<2)|(0<<0)); // powermode = full; lp_conv_delay = 200ms; seq_stage_num = 12; decimation = 64; sw_reset = 0, int_pol = active low; ext_source=enabled; cdc_bias= normal+35%, decimation verlaagd naar minimum omdat dit de touch beter deed werken
      WriteRegister(REGISTER_STAGE_CAL_EN, 0x0000);                 //set temporary to 0
      WriteRegister(REGISTER_AMB_COMP_CTRL0, 0x3230);               //[3:0] = 0 => no sequence results skipped, [7:4]/[11:8] => calibration period in FP/LP mode; FP to LP timeout ctrl = 2xFP_PROXIMITY_CNT; forced cal control is normal; conversion reset is off;
      WriteRegister(REGISTER_AMB_COMP_CTRL1, 0x0619);               //bits [13:8] represent proximity sensitivity, lower is more sensitive (min=1, default=4); bits [7:0] are prox. recalibration level
      WriteRegister(REGISTER_AMB_COMP_CTRL2, 0x0832);               //proximity recalibration time control
      WriteRegister(REGISTER_STAGE_LOW_INT_ENABLE, 0);              //all stages low INT disabled; gpio pin disabled; the idea is we never attach a CIN to a
      WriteRegister(REGISTER_STAGE_HIGH_INT_ENABLE, 0x0FFF);        //all stages high INT enabled;
      WriteRegister(REGISTER_STAGE_COMPLETE_INT_ENABLE, 0x0000);    //all stage conversion complete INT disabled;
      WriteRegister(REGISTER_STAGE_CAL_EN, 0x0FFF);                 //all stage calibration enabled; 3 samples skipped in FP mode; 0 samples skipped in LP mode
}
//------------------------------------------------------------------------------------------------//
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr)
{
    static U16 slider_pad_values[NR_OF_SLIDER_PADS];

//    //this piece of code was as a bugfix for CC001093 but it seems with this version of the driver the bug does not happen anymore (BUG was holding your finger verry close to the touch 0.5cm for one minute and that touchpad would not work anymore)
//    if ((ReadRegister(REGISTER_STAGE_LOW_INT_STATUS) & 0xFFF) != 0 )//check for negative interrupt on stage0->11 (indicates bug)
//    {
//         ForcedCalibration();
//    }

    if ((ReadRegister(REGISTER_STAGE_HIGH_INT_STATUS) & slider_pads_mask) != 0 )//check for interrupt on the pads that are used for the slider
    {
        for (U8 i = 0; i <  NR_OF_SLIDER_PADS; i++ )
        {
            slider_pad_values[i] = GetPadValue(FIRST_SLIDER_PAD+i);
        }
        *slider_value_ptr = CalculateSliderPosition(slider_pad_values); //calculate position with received values
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
// get the value of a stages referenced to the ambient value
static U16 GetPadValue(U8 stage_nr)
{
    if (stage_nr >= 12) //there are only 12 stages 0-11
    {
        LOG_ERR("internal DrvSliderAd7147 error");
    }

    U16 actual_value = ReadRegister((REGISTER_STAGE0_CONV_DATA+((stage_nr)*0x24)));
    U16 ambient_value = ReadRegister((REGISTER_STAGE0_SF_AMBIENT+((stage_nr)*0x24))); // sequential ambient registers are 0x24 apart

    // (small) negative values are rounded to 0, negative values are theoretically impossible for the implemented slider
    if ( actual_value > ambient_value )
    {
        return actual_value - ambient_value;
    }
    else
    {
        return 0;
    }
}
//------------------------------------------------------------------------------------------------//
/// @brief  Calculates and returns the position based on an array of values. Return value between 0 and FFFF
/// @param  "slider_pad_values_ptr" ptr to the array with the pad values
/// @return the slider position
static U16 CalculateSliderPosition(U16 * slider_pad_values_ptr)
{
    U16 slider_pos = 0;
    U32 relative_position;
    U16 pad_ranges = ( 0x10000 / NR_OF_SLIDER_PADS ) + 1; //0x10000 is full range
    U16 max_center_offset = ( pad_ranges / 2 );
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

    #if LINEAR_SLIDER == 1
    if ((pad_with_higest_value == 0) && (pad_before_higest_value > pad_after_higest_value)) //special case: user is on the beginning of the linear slider, so dotn do the normal calculation (only if lineair slider)
    {
    	return 0;
    }
	#endif

    if( slider_pad_values_ptr[pad_after_higest_value] > slider_pad_values_ptr[pad_before_higest_value] )
    {
        difference = ( slider_pad_values_ptr[pad_after_higest_value] - slider_pad_values_ptr[pad_before_higest_value] );
        relative_position = ( difference * SLIDER_RANGE_EXTENTION_VALUE ) / ( slider_pad_values_ptr[pad_after_higest_value] ); //range_extension mult. is used here instead of the next line to void a round-down to 0 when dividing
        if ( relative_position > max_center_offset )
        {
            relative_position = max_center_offset;
        }
        slider_pos = ( pad_with_higest_value * pad_ranges ) + relative_position;
    }
    else
    {
        difference = ( slider_pad_values_ptr[pad_before_higest_value] - slider_pad_values_ptr[pad_after_higest_value] );
        relative_position = ( difference * SLIDER_RANGE_EXTENTION_VALUE ) / ( slider_pad_values_ptr[pad_before_higest_value] ); //range_extension mult. is used here instead of the next line to void a round-down to 0 when dividing
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
//------------------------------------------------------------------------------------------------//
static void ForcedCalibration(void)
{
    U16 readback = ReadRegister(REGISTER_AMB_COMP_CTRL0);
    readback |= 0x4000; //set forced cal bit
    WriteRegister(REGISTER_AMB_COMP_CTRL0, readback);
    readback = ReadRegister(REGISTER_AMB_COMP_CTRL0);
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
    return ReadRegister(REGISTER_STAGE0_PROXIMITY_STATUS) & 0xFFF;     //het proximity register geeft weer welke pads proximity detecteren , check for proximity on stage0->11
}
//------------------------------------------------------------------------------------------------//
U16 DrvSliderAd7147_GetTouchMask(void)
{
    return ReadRegister(REGISTER_STAGE_HIGH_INT_STATUS) & 0xFFF;      //het proximity register geeft weer welke pads proximity detecteren , check for proximity on stage0->11
}
//------------------------------------------------------------------------------------------------//
void DrvSliderAd7147_PrintDebugData(void)
{
    //measue
    U8 i;
    U16 measured_values[NR_OF_USED_STAGES];
    U16 ambient_values[NR_OF_USED_STAGES];
    U16 threshhold_values[NR_OF_USED_STAGES];
    U16 pad_values[NR_OF_USED_STAGES];
    //U16 debug_val[NR_OF_USED_STAGES];

    for ( i = 0; i < NR_OF_USED_STAGES; i++ )
    {
        measured_values[i] = ReadRegister((REGISTER_STAGE0_CONV_DATA+((i)*0x24)));
        ambient_values[i] = ReadRegister((REGISTER_STAGE0_SF_AMBIENT+((i)*0x24))); // sequential ambient registers are 0x24 apart
        threshhold_values[i] = ReadRegister((REGISTER_STAGE0_HIGH_THRESHHOLD+((i)*0x24))) - ambient_values[i]; //deductthe ambient value to make it relative theshhold = better to debug in logger this way
        pad_values[i] = GetPadValue(i);
        //debug_val[i] = ReadRegister((REGISTER_STAGE0_OFFSET_HIGH+((i)*8)));
    }
    U16 touch_mask = DrvSliderAd7147_GetTouchMask();

    //log
    LOG_TRM("|=====================================================|");
    LOG_TRM("|Pad Nr  |Measured|Ambient |Relvalue|Relthres|Touched |");
    LOG_TRM("|--------|--------|--------|--------|--------|--------|");

    for ( i = 0; i < NR_OF_USED_STAGES; i++ )
    {
        LOG_TRM("|%8d|%8d|%8d|%8d|%8d|%8d|",PU16(i)/*PU16(debug_val[i])*/,PU16(measured_values[i]),PU16(ambient_values[i]),PU16(pad_values[i]),PU16(threshhold_values[i]),PU8((touch_mask>>i)&0x01));
    }
    LOG_TRM("|=====================================================|");
}
//================================================================================================//
