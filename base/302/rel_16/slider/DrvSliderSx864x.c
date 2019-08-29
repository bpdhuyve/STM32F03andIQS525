//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// brief explanation
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRV_SLIDER_SX864x_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"

#define CORELOG_LEVEL               LOG_LEVEL_NONE

//------------------------------------------------------------------------------------------------//
#ifndef CHIP_TYPE
    #error "CHIP_TYPE not defined in AppConfig"
#endif
// the possibilities are:
#define CHIP_TYPE_SX8644            0
#define CHIP_TYPE_SX8645            1
//------------------------------------------------------------------------------------------------//
#ifndef CAP_SENSITIVITY
    #define CAP_SENSITIVITY         0x7
#endif
//------------------------------------------------------------------------------------------------//
#ifndef CAP_THRESHHOLD
    #define CAP_THRESHHOLD          0xA0
#endif
//0x50 oorsprongkelijke waarde gekozen voor swipestat met glassplate, expirementeel bepaald voor eengoede werking, niet goed voor emc instraling, chip detecteert touch tijdens instraling, sinds dit geweten is (9/1/2015) hebben we teruggeaan naar 0xA0 omdat dit wel er tegen kan, downside is dat er nu terug dode zones zijn, om dit op te lossen moet de vorm van de pcb terug aangepast worden
//0x78 beter bestand tegen instraling, maar volgens just er onder
//0xA0 default chip value, bestand tegen instraling maar dode zones zijn te groot met pcb 4v1
//------------------------------------------------------------------------------------------------//
//capmode is anders bij sx8644 tegenover sx8645
#if CHIP_TYPE == CHIP_TYPE_SX8644
    #define SPM_CAPMODE             2 //slider, this chip cannot work in wheel mode
#else
    #define SPM_CAPMODE             3 //wheel mode
#endif
//------------------------------------------------------------------------------------------------//
#ifndef AUTO_CHECK_CONFIGURATION
    #define AUTO_CHECK_CONFIGURATION         1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "slider\DrvSliderSx864x.h"
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
    REGISTER_IRQSRC      	            =	0x00,
    REGISTER_CAPSTATMSB  	            =	0x01,
    REGISTER_CAPSTATLSB		            =	0x02,
    REGISTER_WHLPOSMSB		            =	0x03,
    REGISTER_WHLPOSLSB		            =	0x04,
    REGISTER_GPISTAT		            =	0x07,
    REGISTER_SPMSTAT		            =	0x08,
    REGISTER_COMPOPMODE		            =	0x09,
    REGISTER_GPOCTRL		            =	0x0A,
    REGISTER_GPPID			            =	0x0B,
    REGISTER_GPPINTENSITY	            =	0x0C,
    REGISTER_SPMCFG		                =	0x0D,
    REGISTER_SPMBASEADDR	            =	0x0E,
    REGISTER_SPMKEYMSB		            =	0xAC,
    REGISTER_SPMKEYLSB		            =	0xAD,
    REGISTER_SOFTRESET		            =	0xB1
}REGISTER_ADDRESS;


typedef enum
{
    SPM_I2CADDRESS	                    =   0X04,
    SPM_ACTIVESCANPERIOD	            =	0X05,
    SPM_DOZESCANPERIOD	                =	0X06,
    SPM_PASSIVETIMER	                =	0X07,
    SPM_RESERVED	                    =	0X08,
    SPM_CAPMODEMISC			            =	0X09,
    SPM_CAPMODE11_8			            =	0X0A,
    SPM_CAPMODE7_4			            =	0X0B,
    SPM_CAPMODE3_0			            =	0X0C,
    SPM_CAPSENSITIVITY0_1	            =	0X0D,
    SPM_CAPSENSITIVITY2_3	            =	0X0E,
    SPM_CAPSENSITIVITY4_5	            =	0X0F,
    SPM_CAPSENSITIVITY6_7	            =	0X10,
    SPM_CAPSENSITIVITY8_9	            =	0X11,
    SPM_CAPSENSITIVITY10_11	            =   0X12,
    SPM_CAPTHRESH0			            =	0X13,
    SPM_CAPTHRESH1			            =	0X14,
    SPM_CAPTHRESH2			            =	0X15,
    SPM_CAPTHRESH3			            =	0X16,
    SPM_CAPTHRESH4			            =	0X17,
    SPM_CAPTHRESH5			            =	0X18,
    SPM_CAPTHRESH6			            =	0X19,
    SPM_CAPTHRESH7			            =	0X1A,
    SPM_CAPTHRESH8			            =	0X1B,
    SPM_CAPTHRESH9			            =	0X1C,
    SPM_CAPTHRESH10			            =	0X1D,
    SPM_CAPTHRESH11			            =	0X1E,
    SPM_CAPPERCOMP			            =	0X1F,
    SPM_BTNCFG			                =	0X21,
    SPM_BTNAVGTHRESH		            =	0X22,
    SPM_BTNCOMPNEGTHRESH	            =	0X23,
    SPM_BTNCOMPNEGCNTMAX	            =	0X24,
    SPM_BTNHYSTERESIS		            =	0X25,
    SPM_BTNSTUCKATTIMEOUT	            =	0X26,
    SPM_WHLCFG			                =	0X27,
    SPM_WHLSTUCKATTIMEOUT	            =	0X28,
    SPM_WHLHYSTERESIS		            =	0X29,
    SPM_WHLNORMMSB			            =	0X2B,
    SPM_WHLNORMLSB			            =	0X2C,
    SPM_WHLAVGTHRESH		            =	0X2D,
    SPM_WHLCOMPNEGTHRESH	            =	0X2E,
    SPM_WHLCOMPNEGCNTMAX	            =	0X2F,
    SPM_WHLROTATETHRESH		            =	0X30,
    SPM_WHLOFFSET			            =	0X31,
    SPM_MAPWAKEUPSIZE		            =	0X33,
    SPM_MAPWAKEUPVALUE0		            =	0X34,
    SPM_MAPWAKEUPVALUE1		            =	0X35,
    SPM_MAPWAKEUPVALUE2		            =	0X36,
    SPM_MAPAUTOLIGHT0		            =	0X37,
    SPM_MAPAUTOLIGHT1		            =	0X38,
    SPM_MAPAUTOLIGHT2		            =	0X39,
    SPM_MAPAUTOLIGHT3		            =	0X3A,
    SPM_MAPAUTOLIGHTGRP0MSB	            =	0X3B,
    SPM_MAPAUTOLIGHTGRP0LSB	            =	0X3C,
    SPM_MAPAUTOLIGHTGRP1MSB	            =	0X3D,
    SPM_MAPAUTOLIGHTGRP1LSB	            =	0X3E,
    SPM_MAPSEGMENTHYSTERESIS            =	0X3F,
    SPM_GPIOMODE7_4			            =	0X40,
    SPM_GPIOMODE3_0			            =	0X41,
    SPM_GPIOOUTPWRUP		            =	0X42,
    SPM_GPIOAUTOLIGHT		            =	0X43,
    SPM_GPIOPOLARITY		            =	0X44,
    SPM_GPIOINTENSITYON0	            =	0X45,
    SPM_GPIOINTENSITYON1	            =	0X46,
    SPM_GPIOINTENSITYON2	            =	0X47,
    SPM_GPIOINTENSITYON3	            =	0X48,
    SPM_GPIOINTENSITYON4	            =	0X49,
    SPM_GPIOINTENSITYON5	            =	0X4A,
    SPM_GPIOINTENSITYON6	            =	0X4B,
    SPM_GPIOINTENSITYON7	            =	0X4C,
    SPM_GPIOINTENSITYOFF0	            =	0X4D,
    SPM_GPIOINTENSITYOFF1	            =	0X4E,
    SPM_GPIOINTENSITYOFF2	            =	0X4F,
    SPM_GPIOINTENSITYOFF3	            =	0X50,
    SPM_GPIOINTENSITYOFF4	            =	0X51,
    SPM_GPIOINTENSITYOFF5	            =	0X52,
    SPM_GPIOINTENSITYOFF6	            =	0X53,
    SPM_GPIOINTENSITYOFF7	            =	0X54,
    SPM_GPIOFUNCTION		            =	0X56,
    SPM_GPIOINCFACTOR		            =	0X57,
    SPM_GPIODECFACTOR		            =	0X58,
    SPM_GPIOINCTIME7_6		            =	0X59,
    SPM_GPIOINCTIME5_4		            =	0X5A,
    SPM_GPIOINCTIME3_2		            =	0X5B,
    SPM_GPIOINCTIME1_0		            =	0X5C,
    SPM_GPIODECTIME7_6		            =	0X5D,
    SPM_GPIODECTIME5_4		            =	0X5E,
    SPM_GPIODECTIME3_2		            =	0X5F,
    SPM_GPIODECTIME1_0		            =	0X60,
    SPM_GPIOOFFDELAY7_6		            =	0X61,
    SPM_GPIOOFFDELAY5_4		            =	0X62,
    SPM_GPIOOFFDELAY3_2		            =	0X63,
    SPM_GPIOOFFDELAY1_0		            =	0X64,
    SPM_GPIOPULLUPDOWN7_4	            =	0X65,
    SPM_GPIOPULLUPDOWN3_0	            =	0X66,
    SPM_GPIOINTERRUPT7_4	            =	0X67,
    SPM_GPIOINTERRUPT3_0	            =	0X68,
    SPM_GPIODEBOUNCE		            =	0X69
}SPM_ADDRESS;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U8   ReadRegister(REGISTER_ADDRESS registeradress);
static void WriteRegister(REGISTER_ADDRESS regadress,U8 value);
static void WriteSpm8(SPM_ADDRESS baseadress , U8* data_ptr);
static void ReadSpm8(SPM_ADDRESS baseadress, U8* data_ptr);
static void WriteSpm(SPM_ADDRESS address, U8 value);
static void FlushSpm(void);
static void Delay(U32 value);
static void DeviceInit(void);
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static I2C_DEVICE_ID                    i2c_device_hndl = NULL;
static SLIDER_STRUCT                    slider_struct;

static SPM_ADDRESS                      spm_current_base_address = (SPM_ADDRESS)0xFF;  //init with invalid address
static U8                               spm_write_buffer[8];

static U8                               device_address = 0;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// READ normal I2C registers of semtech chip
static U8 ReadRegister(REGISTER_ADDRESS registeradress)
{
    static U8 databuffer[1];
    databuffer[0] = registeradress;
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, &databuffer[0], 1, TRUE);
    DrvI2cMasterDevice_ReadData(i2c_device_hndl, databuffer, 1, TRUE);
    return databuffer[0];
}
//------------------------------------------------------------------------------------------------//
// Write normal I2C registers of semtech chip
static void WriteRegister(REGISTER_ADDRESS regadress,U8 value)
{
    static U8 databuffer[2];
    databuffer[0]= regadress;
    databuffer[1]=  value;
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, databuffer, 2, TRUE);
}
//------------------------------------------------------------------------------------------------//
// Write into the Shadow Parameter Memory, this consists of 4 steps
static void WriteSpm8(SPM_ADDRESS baseadress , U8* data_ptr)
{
    static U8 databuffer[9];
    databuffer[0] = 0;
    memcpy(&(databuffer[1]),data_ptr,8);

    //STEP 1
    WriteRegister(REGISTER_SPMCFG,0x10);//0x0D , 00 01 ENABLES SPM MODE  0 SPM WRITE ACCES 000  = 0001 0000

    //STEP 2
    WriteRegister(REGISTER_SPMBASEADDR, baseadress);//WRITE BASE ADRESS TO REGISTER

    //STEP 3
  //onduidelijk in datasheet maar er moet nog een extra 0 gezonden worden bij stap 3, zie figure54 in datasheet
    DrvI2cMasterDevice_WriteData(i2c_device_hndl,databuffer, 9,TRUE);

    //STEP 4
    WriteRegister(REGISTER_SPMCFG,0x00);//CLEAR SMPCFG to terminate message
}
//------------------------------------------------------------------------------------------------//
// Read into the Shadow Parameter Memory, this consists of 4 steps
static void ReadSpm8(SPM_ADDRESS baseadress, U8* data_ptr)
{
    static U8 dummybuffer[1] = {0};

    //STEP 1
    WriteRegister(REGISTER_SPMCFG,0x18);//0x0D , 00 01 ENABLES SPM MODE  1 SPM READ ACCES 000  = 00011000

    //STEP 2
    WriteRegister(REGISTER_SPMBASEADDR,baseadress);//WRITE BASE ADRESS TO REGISTER

    //Step 3
    DrvI2cMasterDevice_WriteData(i2c_device_hndl,dummybuffer, 1,TRUE); //onduidelijk in datasheet maar er moet nog een extra 0 gezonden worden bij stap 3, zie figure55 in datasheet
    DrvI2cMasterDevice_ReadData(i2c_device_hndl,data_ptr, 8,TRUE);

    //STEP 4
    WriteRegister(REGISTER_SPMCFG,0x00);//CLEAR SMPCFG to terminate message
}
//------------------------------------------------------------------------------------------------//
//function to edit spm values at byte level, this function will not immediately write the spm bytes (spm bytes needs no be written in bursts of 8 byte and always 8 byte aligned)
//the data will be buffered, this buffered data will be flushed when you write to an spm address with another baseaddress, or until you manually call the FlushSpm() function
static void WriteSpm(SPM_ADDRESS address, U8 value)
{
    //check if address hase the same base adress as current base address, if no flush first and read out all 8 registers from baseaddress
    SPM_ADDRESS base_address = (SPM_ADDRESS)(address - (address%8));
    if (base_address != spm_current_base_address)
    {
        FlushSpm();

        //update baseaddress and read out registers incase you dont want to edit them all
        spm_current_base_address = base_address;
        ReadSpm8(base_address,spm_write_buffer);
    }

    //put data in buffer
    spm_write_buffer[address%8] = value;
}
//------------------------------------------------------------------------------------------------//
static void FlushSpm(void)
{
    //send  the 8 bytes and verify that they are written by continuessly reading them
    //we do it this way instead of using the interupt pin because the int pin does not always assert !!! -> unreliable
    static U8 read_buffer[8];
    U8 i;
    BOOL valid;

    if (spm_current_base_address >= 0x6A)   //out of valid address range
    {
        return; //do nothing
    }
    if (spm_current_base_address % 8 != 0 )   //no modulo of 8 !!!
    {
        LOG_ERR("attempt to write to unaligned spm address");
        return;
    }

    do
    {
        WriteSpm8(spm_current_base_address,spm_write_buffer);
        Delay(100);                                             //WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
        ReadSpm8(spm_current_base_address,read_buffer);         //CHECK IF WRITING WAS SUCCESFULL by reading out all the data again

        valid = TRUE;
        for(i = 0;i<8;i++)
        {
            if (spm_write_buffer[i] != read_buffer[i])
            {
                if (!(spm_current_base_address == 0 && (i == 0 || i == 1 || i ==3)))    //igore check for spm registers 0,1,3 because they will read unknown data
                {
                    valid = FALSE;  //not the same, data is not valid
                }
            }
        }

        ReadRegister(REGISTER_IRQSRC);  //do this in the hope of clearing this register. this way INTB SHOULD GO HIGH
        Delay(50);                      //WAIT a small time to clear the INT
    }
    while (!valid);
}
//------------------------------------------------------------------------------------------------//
static U8 ReadSpm(SPM_ADDRESS address)
{
    //check if address hase the same base adress as current base address, if no flush first and read out all 8 registers from baseaddress
    SPM_ADDRESS base_address = (SPM_ADDRESS)(address - (address%8));
    static U8                       temp_read_buffer[8];

    ReadSpm8(base_address,temp_read_buffer);

    return temp_read_buffer[address%8];
}
//------------------------------------------------------------------------------------------------//
static void Delay(U32 value)
{
    //dumb waiting Delay used when configuring device
    value = value * 0x200000 / 100;
    U32 i=0;
    for(i=0;i<value;i++){}
}
//------------------------------------------------------------------------------------------------//
//Configure the Sensitivity settings for the PSIControl glass plate
static void DeviceInit(void)
{
    //perform soft reset
    WriteRegister(REGISTER_SOFTRESET,0xDE);                 //RESET CHIP LOWER --> INTB GOES LOW
    WriteRegister(REGISTER_SOFTRESET,0x00);                 //REQUIRED TO START THE CHIP AGAIN
    Delay(200);                                             //WAIT 200ms -- INT B SHOULD BE HIGH AGAIN (meaning not active)
    ReadRegister(REGISTER_IRQSRC);                          //In the hope of clearing this register. So INTB SHOULD GO HIGH
    Delay(50);                                              //WAIT a small time to clear the INT

    while (DrvSliderSx864x_TestConnection(NULL,device_address) == FALSE)    //check if device answers correctly again before proceding CC001217
    {
        Delay(50);
    }

    //setup touch parameters in the spm (these are in natural sequence of the device because otherwise ther would be unneeded read writes (because of the 8byte burst/alignment system and the way the WriteSpm function works)
    WriteSpm(SPM_ACTIVESCANPERIOD,      0x01);              //Change active scan period to the shortes possible time 1 = 15ms
    //WriteSpm(SPM_DOZESCANPERIOD,        0x0C);            //trbs disabled -> not really done for specific reason, leftover from testing fase

    WriteSpm(SPM_CAPMODEMISC,           (1<<2) | 1);        //Individual CAP sensitivity settings  (last 1 is something that was already there, see datasheet)
    WriteSpm(SPM_CAPMODE11_8,           (SPM_CAPMODE<<6) | (SPM_CAPMODE<<4) | (SPM_CAPMODE<<2) | (SPM_CAPMODE));
    WriteSpm(SPM_CAPMODE7_4,		    (SPM_CAPMODE<<6) | (SPM_CAPMODE<<4) | (SPM_CAPMODE<<2) | (SPM_CAPMODE));
    WriteSpm(SPM_CAPMODE3_0,		    (SPM_CAPMODE<<6) | (SPM_CAPMODE<<4) | (SPM_CAPMODE<<2) | (SPM_CAPMODE));
    WriteSpm(SPM_CAPSENSITIVITY0_1,     (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    WriteSpm(SPM_CAPSENSITIVITY2_3,     (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    WriteSpm(SPM_CAPSENSITIVITY4_5,     (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    WriteSpm(SPM_CAPSENSITIVITY6_7,	    (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    WriteSpm(SPM_CAPSENSITIVITY8_9,	    (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    WriteSpm(SPM_CAPSENSITIVITY10_11,	(CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    WriteSpm(SPM_CAPTHRESH0,		    CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH1,		    CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH2,		    CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH3,			CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH4,			CAP_THRESHHOLD);

    WriteSpm(SPM_CAPTHRESH5,	        CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH6,	        CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH7,	        CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH8,	        CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH9,	        CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH10,	        CAP_THRESHHOLD);
    WriteSpm(SPM_CAPTHRESH11,	        CAP_THRESHHOLD);
    //WriteSpm(SPM_CAPPERCOMP,	        0x01);              //automatic Periodic Offset Compensation on @ 1 second, trbs disabled because this caused (more) problems with emc

    WriteSpm(SPM_WHLHYSTERESIS,         0x06);              //slightly increased the Hysteresis in an attemt to make the touch less vulnerable for interferance
    WriteSpm(SPM_WHLNORMMSB,	        0xFF);              //Set the max return value if touched, we do this to achieve the highest resolution possible
    WriteSpm(SPM_WHLNORMLSB,	        0xFF);

    FlushSpm();//force flush the remainder of the settings to the device

    //NOTE: AFTER RESETTING EVERY CONFIG IS LOST! this is normal, otherwise the NVM must be used
}
//------------------------------------------------------------------------------------------------//
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr)
{
    if (ReadRegister(REGISTER_CAPSTATMSB)&(1<<4))//readout Wheel Touched bit
    {
        //read out raw wheel position data
        U8 WhlPosMsbBuf = ReadRegister(REGISTER_WHLPOSMSB);
        U8 WhlPosLsbBuf = ReadRegister(REGISTER_WHLPOSLSB);
        U32 position = (WhlPosMsbBuf<<8) | WhlPosLsbBuf;

        //convert raw position to range 0-65535 - MAXIMAAL kunnen we dus (0xFFFF*12)/32) terug krijgen, dat is dus 24575 (datasheet p54)
        position = (position*0xFFFF)/((0xFFFF*12)/32);

        //check if slider value is overflowed because of rounding errors in calculation
        if (position >= 0xFFFF)
        {
            *slider_value_ptr = 0xFFFF; //yes trim to max
        }
        else
        {
            *slider_value_ptr = position; //no just copy
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
static void Task_CheckConfigurationFailsafe(VPTR data_ptr)
{
    if (ReadSpm(SPM_CAPMODE3_0) == 0x55)    //instelling staan terug op default (buttons) !!! -> reinit chip
    {
        LOG_WRN("Detected Sx864x \"forgot\" his settings -> reiniting Sx864x");
        DeviceInit();
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
SLIDER_HNDL DrvSliderSx864x_Register(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
    device_address = address;

    if (i2c_device_hndl == NULL)    //register i2c device if not done yet
    {
        i2c_device_hndl = DrvI2cMasterDevice_Register(i2c_channel,device_address, 400000);
    }

    DeviceInit();

    slider_struct.get_value_hook = GetSliderValue;//Addres of function in the hook
    slider_struct.slider_id = 0;

    #if AUTO_CHECK_CONFIGURATION == 1
        //semtec chip verliest soms zijn settings (ongewenste reset ?) komt voor bij rgb demo's, emc instraling en ook bij testsystemen, workaround, kijk iedere 10 seconden of settings nog ok zijn, zo niet, herinit chip
        CoreTask_Start(CoreTask_RegisterTask(10e6, Task_CheckConfigurationFailsafe, NULL, 255, ""));
    #endif

    return &slider_struct;
}
//------------------------------------------------------------------------------------------------//
//returns true if connection is ok, false if not, function can be performed before DrvSliderSx864x_Register
BOOL DrvSliderSx864x_TestConnection(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
    U8 read_buffer[8];
    read_buffer[SPM_I2CADDRESS] = 0; //init to 0

    if (i2c_device_hndl == NULL)    //register i2c device if not done yet
    {
        i2c_device_hndl = DrvI2cMasterDevice_Register(i2c_channel,address, 400000);
    }

    ReadSpm8((SPM_ADDRESS)0,read_buffer);    //read out first 8 bytes from spm

    if (read_buffer[SPM_I2CADDRESS] == address) //validate if address register is what we expect
    {
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//


