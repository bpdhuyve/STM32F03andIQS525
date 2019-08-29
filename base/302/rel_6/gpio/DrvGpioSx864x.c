//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of GPIO using Sx864x
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define GPIO__DRVGPIOSX864X_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef GPIO__DRVGPIOSX864X_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               GPIO__DRVGPIOSX864X_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef CHIP_TYPE
    #define CHIP_TYPE                   CHIP_TYPE_SX8645
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
#ifndef AUTO_CHECK_CONFIGURATION
    #define AUTO_CHECK_CONFIGURATION         1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of gpio pins that can be registered
#ifndef SX864X_MAX_PINS
	#define SX864X_MAX_PINS			        12
#elif SX864X_MAX_PINS > 12
    #error "max 12 pins allowed"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "gpio\DrvGpioSx864x.h"
#include "i2c\DrvI2cMasterDevice.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SPM_CAPMODE                     1   // button
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
    REGISTER_SOFTRESET		            =	0xB1,
}
REGISTER_ADDRESS;

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
}
SPM_ADDRESS;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void Sx864x_I2cComplete(BOOL success);
static U8   Sx864x_ReadRegister(REGISTER_ADDRESS registeradress);
static void Sx864x_WriteRegister(REGISTER_ADDRESS regadress, U8 value);
static void Sx864x_WriteSpm8(SPM_ADDRESS baseadress);
static void Sx864x_ReadSpm8(SPM_ADDRESS baseadress, U8* data_ptr);
static void Sx864x_WriteSpm(SPM_ADDRESS address, U8 value);
static U8   Sx864x_ReadSpm(SPM_ADDRESS address);
static void Sx864x_FlushSpm(void);
static void Sx864x_Delay(U32 value);
static void Sx864x_DeviceInit(void);
static void Sx864x_GetButtonMask(void);
static BOOL Sx864x_GetPin(U16 pin_id);
static BOOL Sx864x_GetPinBuffered(U16 pin_id);
static void Sx864x_ControlTask(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static I2C_DEVICE_ID                    i2c_device_hndl;
static BOOL                             i2c_device_present;

static SPM_ADDRESS                      spm_current_base_address;
static U8                               spm_write_buffer_full[9];    // extra dummy byte needed for writing
static U8*                              spm_write_buffer;
static U8                               spm_write_mask;

static DRV_GPIO_PIN_STRUCT              sx864x_pin_struct[SX864X_MAX_PINS];
static U8                               sx864x_pin_count;
static const GPIO_DRV_HOOK_LIST         sx864x_pin_hook_list = {NULL, NULL, Sx864x_GetPin};
static const GPIO_DRV_HOOK_LIST         sx864x_pin_hook_list_buffered = {NULL, NULL, Sx864x_GetPinBuffered};
static U16                              sx864x_status;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void Sx864x_I2cComplete(BOOL success)
{
    if(i2c_device_present != success)
        LOG_DBG("presence %d", PU8(success));
    
    i2c_device_present = success;
}
//------------------------------------------------------------------------------------------------//
// READ normal I2C registers of semtech chip
static U8 Sx864x_ReadRegister(REGISTER_ADDRESS registeradress)
{
    U8  data_byte = registeradress;
    
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, &data_byte, 1, TRUE);
    DrvI2cMasterDevice_ReadData(i2c_device_hndl, &data_byte, 1, TRUE);
    
    return data_byte;
}
//------------------------------------------------------------------------------------------------//
// Write normal I2C registers of semtech chip
static void Sx864x_WriteRegister(REGISTER_ADDRESS regadress, U8 value)
{
    U8  data[2];
    
    data[0]= regadress;
    data[1]=  value;
    
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, data, 2, TRUE);
}
//------------------------------------------------------------------------------------------------//
// Write into the Shadow Parameter Memory, this consists of 4 steps
static void Sx864x_WriteSpm8(SPM_ADDRESS baseadress)
{
    //STEP 1
    Sx864x_WriteRegister(REGISTER_SPMCFG,0x10);//0x0D , 00 01 ENABLES SPM MODE  0 SPM WRITE ACCES 000  = 0001 0000

    //STEP 2
    Sx864x_WriteRegister(REGISTER_SPMBASEADDR, baseadress);//WRITE BASE ADRESS TO REGISTER

    //STEP 3
    //onduidelijk in datasheet maar er moet nog een extra 0 gezonden worden bij stap 3, zie figure54 in datasheet
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, spm_write_buffer_full, 9,TRUE);

    //STEP 4
    Sx864x_WriteRegister(REGISTER_SPMCFG,0x00);//CLEAR SMPCFG to terminate message
}
//------------------------------------------------------------------------------------------------//
// Read into the Shadow Parameter Memory, this consists of 4 steps
static void Sx864x_ReadSpm8(SPM_ADDRESS baseadress, U8* data_ptr)
{
    U8  dummy_byte = 0;
    
    //STEP 1
    Sx864x_WriteRegister(REGISTER_SPMCFG,0x18);//0x0D , 00 01 ENABLES SPM MODE  1 SPM READ ACCES 000  = 00011000

    //STEP 2
    Sx864x_WriteRegister(REGISTER_SPMBASEADDR, baseadress);//WRITE BASE ADRESS TO REGISTER

    //Step 3
    //onduidelijk in datasheet maar er moet nog een extra 0 gezonden worden bij stap 3, zie figure55 in datasheet
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, &dummy_byte, 1, TRUE);
    DrvI2cMasterDevice_ReadData(i2c_device_hndl, data_ptr, 8, TRUE);

    //STEP 4
    Sx864x_WriteRegister(REGISTER_SPMCFG,0x00);//CLEAR SMPCFG to terminate message
}
//------------------------------------------------------------------------------------------------//
//function to edit spm values at byte level, this function will not immediately write the spm bytes (spm bytes needs no be written in bursts of 8 byte and always 8 byte aligned)
//the data will be buffered, this buffered data will be flushed when you write to an spm address with another baseaddress, or until you manually call the Sx864x_FlushSpm() function
static void Sx864x_WriteSpm(SPM_ADDRESS address, U8 value)
{
    //check if address hase the same base adress as current base address, if no flush first and read out all 8 registers from baseaddress
    SPM_ADDRESS base_address = (SPM_ADDRESS)(address & 0xF8);
    
    if (base_address != spm_current_base_address)
    {
        Sx864x_FlushSpm();

        //update baseaddress and read out registers incase you dont want to edit them all
        spm_current_base_address = base_address;
        Sx864x_ReadSpm8(base_address, spm_write_buffer);
        spm_write_mask = 0;
    }

    //put data in buffer
    spm_write_buffer[(address & 0x07)] = value;
    spm_write_mask |= (1<<(address & 0x07));
}
//------------------------------------------------------------------------------------------------//
static void Sx864x_FlushSpm(void)
{
    //send  the 8 bytes and verify that they are written by continuessly reading them
    //we do it this way instead of using the interupt pin because the int pin does not always assert !!! -> unreliable
    U8      read_buffer[8];
    U8      i;
    BOOL    valid;
#if ((CORELOG_LEVEL) & LOG_LEVEL_DEBUG)
    U8      irqsrc;
#endif

    if (spm_current_base_address >= 0x6A)   //out of valid address range
    {
        return; //do nothing
    }
    if ((spm_current_base_address & 0x07) != 0 )   //no modulo of 8 !!!
    {
        LOG_ERR("attempt to write to unaligned spm address");
        return;
    }

    do
    {
        // write data
        LOG_DBG("WR: [%02x] %02x (%02x)", PU8(spm_current_base_address), PU8A(spm_write_buffer, 8), PU8(spm_write_mask));
        Sx864x_WriteSpm8(spm_current_base_address);
        
        // wait some time (could also check INTB)
        Sx864x_Delay(25);
        
        // read status register, this should indicate 0x20 (SPM write effective), if not, data will not be correct anyway
        // the reading will clear the flag (if it was there in the first place)
#if ((CORELOG_LEVEL) & LOG_LEVEL_DEBUG)
        irqsrc = Sx864x_ReadRegister(REGISTER_IRQSRC);
        LOG_DBG("IRQSRC %02x", PU8(irqsrc));
#else
        Sx864x_ReadRegister(REGISTER_IRQSRC);
#endif
        
        // small delay to allow INTB to re-assert
        Sx864x_Delay(1);
        
        // read back the data (prediciton: if irqsrc did not have 0x20 flag, data will not be correct)
        Sx864x_ReadSpm8(spm_current_base_address, read_buffer);
        LOG_DBG("RD: %02x", PU8A(read_buffer, 8));
        
        // check if valid: only check bytes that were written, others may change on their own (reserved bytes etc.)
        valid = TRUE;
        for(i = 0; i<8; i++)
        {
            if((spm_write_mask & (1<<i)) && (spm_write_buffer[i] != read_buffer[i]))
            {
                valid = FALSE;
            }
        }

#if ((CORELOG_LEVEL) & LOG_LEVEL_DEBUG)
        CoreLog_Flush();
#endif
        
        // if presence seems to be lost: abort
        if(i2c_device_present == FALSE)
        {
            break;
        }
    }
    while(!valid);
    
    spm_current_base_address = (SPM_ADDRESS)0xFF;
}
//------------------------------------------------------------------------------------------------//
static U8 Sx864x_ReadSpm(SPM_ADDRESS address)
{
    U8              temp_read_buffer[8];
    SPM_ADDRESS     base_address = (SPM_ADDRESS)(address & 0xF8);

    Sx864x_ReadSpm8(base_address, temp_read_buffer);
    LOG_DBG("addr %02x : %02x", PU8(address & 0xF8), PU8A(temp_read_buffer, 8));

    return temp_read_buffer[address & 0x07];
}
//------------------------------------------------------------------------------------------------//
static void Sx864x_Delay(U32 value)
{
    //dumb waiting Sx864x_Delay used when configuring device
    U32 i=0;
    
    value = value * 0x200000 / 100;
    for(i=0;i<value;i++){}
}
//------------------------------------------------------------------------------------------------//
//Configure the Sensitivity settings for the PSIControl glass plate
static void Sx864x_DeviceInit(void)
{
    //perform soft reset
    Sx864x_WriteRegister(REGISTER_SOFTRESET,0xDE);                 //RESET CHIP LOWER --> INTB GOES LOW
    Sx864x_WriteRegister(REGISTER_SOFTRESET,0x00);                 //REQUIRED TO START THE CHIP AGAIN
    
    // if presence not detected: abort
    if(i2c_device_present == FALSE)
    {
        return;
    }
    
    Sx864x_Delay(200);                                             //WAIT 200ms -- INT B SHOULD BE HIGH AGAIN (meaning not active)
    Sx864x_ReadRegister(REGISTER_IRQSRC);                          //In the hope of clearing this register. So INTB SHOULD GO HIGH
    Sx864x_Delay(1);                                              //WAIT a small time to clear the INT

    //setup touch parameters in the spm (these are in natural sequence of the device because otherwise ther would be unneeded read writes (because of the 8byte burst/alignment system and the way the Sx864x_WriteSpm function works)
    Sx864x_WriteSpm(SPM_ACTIVESCANPERIOD,       0x01);              //Change active scan period to the shortes possible time 1 = 15ms

    Sx864x_WriteSpm(SPM_CAPMODEMISC,            (1<<2) | 1);        //Individual CAP sensitivity settings  (last 1 is something that was already there, see datasheet)
    Sx864x_WriteSpm(SPM_CAPSENSITIVITY0_1,      (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    Sx864x_WriteSpm(SPM_CAPSENSITIVITY2_3,      (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    Sx864x_WriteSpm(SPM_CAPSENSITIVITY4_5,      (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    Sx864x_WriteSpm(SPM_CAPSENSITIVITY6_7,	    (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    Sx864x_WriteSpm(SPM_CAPSENSITIVITY8_9,	    (CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    Sx864x_WriteSpm(SPM_CAPSENSITIVITY10_11,	(CAP_SENSITIVITY << 4) | CAP_SENSITIVITY);
    Sx864x_WriteSpm(SPM_CAPTHRESH0,		        CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH1,		        CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH2,		        CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH3,			    CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH4,			    CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH5,	            CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH6,	            CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH7,	            CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH8,	            CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH9,	            CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH10,	        CAP_THRESHHOLD);
    Sx864x_WriteSpm(SPM_CAPTHRESH11,	        CAP_THRESHHOLD);
    //Sx864x_WriteSpm(SPM_CAPPERCOMP,	        0x01);              //automatic Periodic Offset Compensation on @ 1 second, trbs disabled because this caused (more) problems with emc
    
    Sx864x_WriteSpm(SPM_BTNCFG,                 0x00);              // multi report, no interrupt, direct input
    Sx864x_WriteSpm(SPM_BTNHYSTERESIS,          0x0A);              // 0x0A is default hysteresis (10%)
    Sx864x_WriteSpm(SPM_BTNSTUCKATTIMEOUT,      0x20);              // 32sec timeout
    
    // set CAP modes as last, because for some reason their values change
    Sx864x_WriteSpm(SPM_CAPMODE11_8,            (SPM_CAPMODE<<6) | (SPM_CAPMODE<<4) | (SPM_CAPMODE<<2) | (SPM_CAPMODE));
    Sx864x_WriteSpm(SPM_CAPMODE7_4,		        (SPM_CAPMODE<<6) | (SPM_CAPMODE<<4) | (SPM_CAPMODE<<2) | (SPM_CAPMODE));
    Sx864x_WriteSpm(SPM_CAPMODE3_0,		        (SPM_CAPMODE<<6) | (SPM_CAPMODE<<4) | (SPM_CAPMODE<<2) | (SPM_CAPMODE));
    
    Sx864x_FlushSpm();//force flush the remainder of the settings to the device
    
    //NOTE: AFTER RESETTING EVERY CONFIG IS LOST! this is normal, otherwise the NVM must be used
    LOG_DBG("INIT DONE");
}
//------------------------------------------------------------------------------------------------//
static void Sx864x_GetButtonMask(void)
{
    sx864x_status = ((U16)Sx864x_ReadRegister(REGISTER_CAPSTATMSB) << 8) | ((U16)Sx864x_ReadRegister(REGISTER_CAPSTATLSB));
    if(i2c_device_present == FALSE)
    {
        sx864x_status = 0;
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL Sx864x_GetPin(U16 pin_id)
{
    Sx864x_GetButtonMask();
    return (BOOL)((sx864x_status & (1 << pin_id)) > 0);
}
//------------------------------------------------------------------------------------------------//
static BOOL Sx864x_GetPinBuffered(U16 pin_id)
{
    return (BOOL)((sx864x_status & (1 << pin_id)) > 0);
}
//------------------------------------------------------------------------------------------------//
static void Sx864x_ControlTask(VPTR data_ptr)
{
#if AUTO_CHECK_CONFIGURATION == 1
    static U16  i = 0;
    U8  reading;
#endif
    
    if(i2c_device_present == FALSE)
    {
        // if presence was lost: try to re-configure
        Sx864x_DeviceInit();
        i = 0;
    }
    else
    {
        // try to get button mask
        Sx864x_GetButtonMask();
        if(sx864x_status != 0)
        {
            LOG_DEV("%04h", PU16(sx864x_status));
        }
        
#if AUTO_CHECK_CONFIGURATION == 1
        if(++i >= 1000)
        {
            i = 0;
            reading = Sx864x_ReadSpm(SPM_CAPMODE11_8);
            if(reading != 0x55)    //instelling staan terug op default (wheel ) !!! -> reinit chip
            {
                LOG_WRN("Detected Sx864x \"forgot\" his settings -> reiniting Sx864x (%02x)", PU8(reading));
                Sx864x_DeviceInit();
            }
        }
#endif
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvGpioSx864x_Init(I2C_CHANNEL_HNDL i2c_channel)
{
    spm_current_base_address = (SPM_ADDRESS)0xFF;  //init with invalid address
    MEMSET((VPTR)spm_write_buffer_full, 0, SIZEOF(spm_write_buffer_full));
    spm_write_buffer = &spm_write_buffer_full[1];
    spm_write_mask = 0;

    MEMSET((VPTR)sx864x_pin_struct, 0, SIZEOF(sx864x_pin_struct));
    sx864x_pin_count = 0;
    
    CoreTask_Start(CoreTask_RegisterTask(10e3, Sx864x_ControlTask, NULL, 128, "Sx864x"));
    sx864x_status = 0;
    
    i2c_device_hndl = DrvI2cMasterDevice_Register(i2c_channel, 0x2B, 100000);
    DrvI2cMasterDevice_MsgComplete(i2c_device_hndl, Sx864x_I2cComplete);
    i2c_device_present = FALSE;
    
    Sx864x_DeviceInit();
}
//------------------------------------------------------------------------------------------------//
DRVGPIO_PIN_HNDL DrvGpioSx864x_Register(U8 button_nr, SYS_PIN_FUNC pin_func, BOOL is_buffered)
{
    DRVGPIO_PIN_HNDL        pin_hndl;
    
    if(pin_func != PIN_INPUT_FLOATING)
    {
        LOG_WRN("button %d: only floating inputs allowed (0x%08h)", PU8(button_nr), PU8(pin_func));
        return NULL;
    }
    if(button_nr >= 12)
    {
        LOG_WRN("button %d overflow (max 11)", PU8(button_nr));
        return NULL;
    }

    //check all registred pins and check if it has been registred yet
    for(pin_hndl = sx864x_pin_struct; pin_hndl < &sx864x_pin_struct[sx864x_pin_count]; pin_hndl++)
    {
        if(pin_hndl->pin_id == button_nr)
        {
            LOG_WRN("re-register button %d", PU8(button_nr));
            return pin_hndl;  //same pin_id found, return same handle
        }
    }

    if(sx864x_pin_count < SX864X_MAX_PINS)
    {
        pin_hndl->pin_id = button_nr;
        if(is_buffered)
        {
            pin_hndl->hook_list_ptr = (GPIO_DRV_HOOK_LIST*)&sx864x_pin_hook_list_buffered;
        }
        else
        {
            pin_hndl->hook_list_ptr = (GPIO_DRV_HOOK_LIST*)&sx864x_pin_hook_list;
        }
        sx864x_pin_count++;
        return pin_hndl;
    }
    LOG_ERR("Pin register count overrun");
    return NULL;   //pin hndl null means no pin
}
//================================================================================================//


