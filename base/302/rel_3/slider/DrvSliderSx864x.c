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

#ifndef CHIP_TYPE
    #error "CHIP_TYPE not defined in AppConfig"
#endif

// the possibilities are:
#define CHIP_TYPE_SX8644     0
#define CHIP_TYPE_SX8645     1

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
//volatile int	                I2CState;
//LPC_I2C_Msg_t	                I2CMsg;
//I2CPORTS                      I2CPort = I2CPORT1;//WHICH PORT TO BE USED
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    IRQSRC      	        =	0x00,
    CAPSTATMSB  	        =	0x01,
    CAPSTATLSB		        =	0x02,
    WHLPOSMSB		        =	0x03,
    WHLPOSLSB		        =	0x04,
    GPISTAT		            =	0x07,
    SPMSTAT		            =	0x08,
    COMPOPMODE		        =	0x09,
    GPOCTRL		            =	0x0A,
    GPPID			        =	0x0B,
    GPPINTENSITY	        =	0x0C,
    SPMCFG		            =	0x0D,
    SPMBASEADDR	            =	0x0E,
    SPMKEYMSB		        =	0xAC,
    SPMKEYLSB		        =	0xAD,
    SOFTRESET		        =	0xB1
}REGISTER_ADRESS;

typedef enum
{
    I2CADDRESS	            =	0X04,
    ACTIVESCANPERIOD	    =	0X05,
    DOZESCANPERIOD	        =	0X06,
    PASSIVETIMER	        =	0X07,
    RESERVED	            =	0X08,
    CAPMODEMISC			    =	0X09,
    CAPMODE11_8			    =	0X0A,
    CAPMODE7_4			    =	0X0B,
    CAPMODE3_0			    =	0X0C,
    CAPSENSITIVITY0_		=	0X0D,
    CAPSENSITIVITY2_		=	0X0E,
    CAPSENSITIVITY4_		=	0X0F,
    CAPSENSITIVITY6_		=	0X10,
    CAPSENSITIVITY8_		=	0X11,
    CAPSENSITIVITY10_		=	0X12,
    CAPTHRESH0			    =	0X13,
    CAPTHRESH1			    =	0X14,
    CAPTHRESH2			    =	0X15,
    CAPTHRESH3			    =	0X16,
    CAPTHRESH4			    =	0X17,
    CAPTHRESH5			    =	0X18,
    CAPTHRESH6			    =	0X19,
    CAPTHRESH7			    =	0X1A,
    CAPTHRESH8			    =	0X1B,
    CAPTHRESH9			    =	0X1C,
    CAPTHRESH10			    =	0X1D,
    CAPTHRESH11			    =	0X1E,
    CAPPERCOMP			    =	0X1F,
    BTNCFG			        =	0X21,
    BTNAVGTHRESH		    =	0X22,
    BTNCOMPNEGTHRESH		=	0X23,
    BTNCOMPNEGCNTMAX		=	0X24,
    BTNHYSTERESIS		    =	0X25,
    BTNSTUCKATTIMEOUT		=	0X26,
    WHLCFG			        =	0X27,
    WHLSTUCKATTIMEOUT		=	0X28,
    WHLHYSTERESIS		    =	0X29,
    WHLNORMMSB			    =	0X2B,
    WHLNORMLSB			    =	0X2C,
    WHLAVGTHRESH		    =	0X2D,
    WHLCOMPNEGTHRESH		=	0X2E,
    WHLCOMPNEGCNTMAX		=	0X2F,
    WHLROTATETHRESH		    =	0X30,
    WHLOFFSET			    =	0X31,
    MAPWAKEUPSIZE		    =	0X33,
    MAPWAKEUPVALUE0		    =	0X34,
    MAPWAKEUPVALUE1		    =	0X35,
    MAPWAKEUPVALUE2		    =	0X36,
    MAPAUTOLIGHT0		    =	0X37,
    MAPAUTOLIGHT1		    =	0X38,
    MAPAUTOLIGHT2		    =	0X39,
    MAPAUTOLIGHT3		    =	0X3A,
    MAPAUTOLIGHTGRP0MSB		=	0X3B,
    MAPAUTOLIGHTGRP0LSB		=	0X3C,
    MAPAUTOLIGHTGRP1MSB		=	0X3D,
    MAPAUTOLIGHTGRP1LSB		=	0X3E,
    MAPSEGMENTHYSTERESIS	=	0X3F,
    GPIOMODE7_4			    =	0X40,
    GPIOMODE3_0			    =	0X41,
    GPIOOUTPWRUP		    =	0X42,
    GPIOAUTOLIGHT		    =	0X43,
    GPIOPOLARITY		    =	0X44,
    GPIOINTENSITYON0		=	0X45,
    GPIOINTENSITYON1		=	0X46,
    GPIOINTENSITYON2		=	0X47,
    GPIOINTENSITYON3		=	0X48,
    GPIOINTENSITYON4		=	0X49,
    GPIOINTENSITYON5		=	0X4A,
    GPIOINTENSITYON6		=	0X4B,
    GPIOINTENSITYON7		=	0X4C,
    GPIOINTENSITYOFF0		=	0X4D,
    GPIOINTENSITYOFF1		=	0X4E,
    GPIOINTENSITYOFF2		=	0X4F,
    GPIOINTENSITYOFF3		=	0X50,
    GPIOINTENSITYOFF4		=	0X51,
    GPIOINTENSITYOFF5		=	0X52,
    GPIOINTENSITYOFF6		=	0X53,
    GPIOINTENSITYOFF7		=	0X54,
    GPIOFUNCTION		    =	0X56,
    GPIOINCFACTOR		    =	0X57,
    GPIODECFACTOR		    =	0X58,
    GPIOINCTIME7_6		    =	0X59,
    GPIOINCTIME5_4		    =	0X5A,
    GPIOINCTIME3_2		    =	0X5B,
    GPIOINCTIME1_0		    =	0X5C,
    GPIODECTIME7_6		    =	0X5D,
    GPIODECTIME5_4		    =	0X5E,
    GPIODECTIME3_2		    =	0X5F,
    GPIODECTIME1_0		    =	0X60,
    GPIOOFFDELAY7_6		    =	0X61,
    GPIOOFFDELAY5_4		    =	0X62,
    GPIOOFFDELAY3_2		    =	0X63,
    GPIOOFFDELAY1_0		    =	0X64,
    GPIOPULLUPDOWN7_4		=	0X65,
    GPIOPULLUPDOWN3_0		=	0X66,
    GPIOINTERRUPT7_4		=	0X67,
    GPIOINTERRUPT3_0		=	0X68,
    GPIODEBOUNCE		    =	0X69
}SPM_ADRESS;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U8 Read_Register(REGISTER_ADRESS registeradress);
static void Write_Register(REGISTER_ADRESS regadress,U8 value);
//static void Configure_SX8644(void);
static void SpmWrite8(U8 baseadress , U8* data_ptr);
static void SpmRead8(U8 baseadress,U8* datapntr );
static void Configure_SX864x_Plexi(void);
static BOOL DrvSliderSx864x_Get(U8 slider_id, U16* slider_value_ptr);
static BOOL ValidateSPM(U8 startbyte);
static void SpmWriteAndRead(U8 baseadress);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static I2C_DEVICE_ID           i2c_device_hndl;
static SLIDER_STRUCT           slider_struct;
static U8 writeBuffer[9];
static U8 ReadBuffer[9];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// READ normal I2C registers of semtech 8644 chip
static U8 Read_Register(REGISTER_ADRESS registeradress)
{
    static U8 testbuffer[1];
    testbuffer[0] = registeradress;
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, &testbuffer[0], 1, TRUE);
    DrvI2cMasterDevice_ReadData(i2c_device_hndl, testbuffer, 1, TRUE);
    return testbuffer[0];
}
//------------------------------------------------------------------------------------------------//
// Write normal I2C registers of semtech 8644 chip
static void Write_Register(REGISTER_ADRESS regadress,U8 value)
{
    static U8 buffer[2];
    buffer[0]= regadress;
    buffer[1]=  value;
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, buffer, 2, TRUE);
}
//------------------------------------------------------------------------------------------------//
// Write into the Shadow proccessing memory, this consists of 4 steps
static void SpmWrite8(U8 baseadress , U8* data_ptr)
{
    static U8 localwriteBuffer[9];
    memcpy(localwriteBuffer,data_ptr,9);

    //STEP 1
    Write_Register(SPMCFG,0x10);//0x0D , 00 01 ENABLES SPM MODE  0 SPM WRITE ACCES 000  = 0001 0000

    //STEP 2
    Write_Register(SPMBASEADDR, baseadress);//WRITE BASE ADRESS TO REGISTER

    //STEP 3
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, localwriteBuffer, 9,TRUE);

    //STEP 4
    Write_Register(SPMCFG,0x00);//CLEAR SMPCFG to terminate message
}
//------------------------------------------------------------------------------------------------//
// Read into the Shadow proccessing memory, this consists of 4 steps
static void SpmRead8(U8 baseadress, U8* datapntr)
{
    static U8 emptybuffer[1] = {0};

    //STEP 1
    Write_Register(SPMCFG,0x18);//0x0D , 00 01 ENABLES SPM MODE  1 SPM READ ACCES 000  = 00011000

    //STEP 2
    Write_Register(SPMBASEADDR,baseadress);//WRITE BASE ADRESS TO REGISTER

    //Step 3
    DrvI2cMasterDevice_WriteData(i2c_device_hndl,emptybuffer, 1,TRUE);
    DrvI2cMasterDevice_ReadData(i2c_device_hndl, datapntr, 8,TRUE);

    //STEP 4
    Write_Register(SPMCFG,0x00);//CLEAR SMPCFG to terminate message

}
//------------------------------------------------------------------------------------------------//
//Configure Basic Sensetivity startup values into the the SPM
/*
static void Configure_SX8644(void)
{
    static U8 writeBuffer[9];
    static U8 ReadBuffer[9];

    U32 i=0;

    SoftReset:

    //--------------STEP 1---------------------
    Write_Register(SOFTRESET,0xDE);//RESET CHIP LOWER --> INTB GOES LOW
    Write_Register(SOFTRESET,0x00);//REQUIRED TO START THE CHIP AGAIN
    for(i=0;i<0x3FFFFF;i++){}//WAIT 200ms -- INT B SHOULD BE HIGH AGAIN (meaning not active)
    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT
    //-----------------------------------------

    WriteBlock1:

    //Block 1 ---------PREPARE SPM DATA------------
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//Reserved
    writeBuffer[2] = 0x00;//Reserved
    writeBuffer[3] = 0x10;//Reserved
    writeBuffer[4] = 0x00;//Reserved
    writeBuffer[5] = 0x2B;//I2CAddress
    writeBuffer[6] = 0x01;//ActiveScanPeriod        -----> IMPORTANT
    writeBuffer[7] = 0x0C;//DozeScanPeriod
    writeBuffer[8] = 0x00;//PassiveTimer
    SpmWrite8(0x00,&writeBuffer[0]); //BASE adress is 0x00 (needs to be modulo of 0x08
    for(i=0;i<0x1FFFFF;i++){}        //WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
    SpmRead8(0x00,ReadBuffer);       //CHECK IF WRITING WAS SUCCESFULL

    for(i=4;i<8;i++)
    {
      if (writeBuffer[i+1] == ReadBuffer[i])
      {
        //nop
      }else
      {
        goto WriteBlock1;
      }
    }

    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT

    WriteBlock2:
    //Block 2 SET CAP MODE & SENSITIVITY
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//Reserved
    writeBuffer[2] = 0x01;//Capmodeminsc            -----> IMPORTANT
    writeBuffer[3] = 0xAA;//CapMode11_8
    writeBuffer[4] = 0xAA;//CapMode7_4
    writeBuffer[5] = 0xAA;//CapMode3_0
    writeBuffer[6] = 0x00;//Capsense0_1             -----> IMPORTANT
    writeBuffer[7] = 0x55;//Capsense2_3
    writeBuffer[8] = 0x55;//Capsense4_5
    SpmWrite8(0x08,&writeBuffer[0]);//BASE adress is 0x08 (needs to be modulo of 0x08
    for(i=0;i<0x1FFFFF;i++){}//WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
    SpmRead8(0x08,ReadBuffer);

    for(i=0;i<8;i++)
    {
      if (writeBuffer[i+1] == ReadBuffer[i])
      {
        //nop
      }else
      {
        goto WriteBlock2;
      }
    }

    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT

    WriteBlock3:
    //Block 3
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x55;//Capsense6_7
    writeBuffer[2] = 0x55;//Capsense8_9
    writeBuffer[3] = 0x77;//Capsense10_11
    writeBuffer[4] = 0xA0;//CapTresh0
    writeBuffer[5] = 0xA0;//CapTresh1
    writeBuffer[6] = 0xA0;//CapTresh2
    writeBuffer[7] = 0xA0;//CapTresh3
    writeBuffer[8] = 0xA0;//CapTresh4
    SpmWrite8(CAPSENSITIVITY6_,&writeBuffer[0]);
    for(i=0;i<0x1FFFFF;i++){}//WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
    SpmRead8(CAPSENSITIVITY6_,ReadBuffer);

    for(i=0;i<8;i++)
    {
      if (writeBuffer[i+1] == ReadBuffer[i])
      {
        //nop
      }else
      {
        goto WriteBlock3;
      }
    }

    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT

    WriteBlock4:
    //Block 4
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x70;//CapTresh5
    writeBuffer[2] = 0x70;//CapTresh6
    writeBuffer[3] = 0x70;//CapTresh7
    writeBuffer[4] = 0x70;//CapTresh8
    writeBuffer[5] = 0x70;//CapTresh9
    writeBuffer[6] = 0x70;//CapTresh10
    writeBuffer[7] = 0x70;//CapTresh11
    writeBuffer[8] = 0x00;//CapPerComp
    SpmWrite8(CAPTHRESH5,&writeBuffer[0]);
    for(i=0;i<0x1FFFFF;i++){}//WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
    SpmRead8(CAPTHRESH5,ReadBuffer);

    for(i=0;i<8;i++)
    {
      if (writeBuffer[i+1] == ReadBuffer[i])
      {
        //nop
      }else
      {
        goto WriteBlock4;
      }
    }

    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT

    WriteBlock5:
    //Block 5
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//RESERVED
    writeBuffer[2] = 0x30;//BtnCfg
    writeBuffer[3] = 0x50;//BtnAvgTHresh
    writeBuffer[4] = 0x50;//BtnCompNegThresh
    writeBuffer[5] = 0x01;//BtnCompNegCntMax
    writeBuffer[6] = 0x0A;//BtnHysteresis
    writeBuffer[7] = 0x00;//BtnStuckAtTimeout
    writeBuffer[8] = 0x00;//Sldconfig
    SpmWrite8(0x20,&writeBuffer[0]);
    for(i=0;i<0x1FFFFF;i++){}//WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
    SpmRead8(0x20,ReadBuffer);

    for(i=0;i<8;i++)
    {
      if (writeBuffer[i+1] == ReadBuffer[i])
      {
        //nop
      }else
      {
        goto WriteBlock5;
      }
    }

    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT

    WriteBlock6:
    //Block 6
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//WhlStuckAtTimeOut
    writeBuffer[2] = 0x03;//WhlHysteresis
    writeBuffer[3] = 0xFF;//RESERVED
    writeBuffer[4] = 0xFF;//SldrNormMsb             -----> IMPORTANT
    writeBuffer[5] = 0xFF;//SldrNormLsb
    writeBuffer[6] = 0x50;//WhlAvgThresh
    writeBuffer[7] = 0x50;//SliderCompNegThresh
    writeBuffer[8] = 0x01;//SliderCompNegCntMax
    SpmWrite8(WHLSTUCKATTIMEOUT,&writeBuffer[0]);
    for(i=0;i<0x1FFFFF;i++){}//WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
    SpmRead8(WHLSTUCKATTIMEOUT,ReadBuffer);

    for(i=0;i<8;i++)
    {
      if (writeBuffer[i+1] == ReadBuffer[i])
      {
        //nop
      }else
      {
        goto WriteBlock6;
      }
    }

    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT

    //AFTER READING THIS ONCE, THE REGISTER SETTINGS WERE USED AS PROGRAMMED
    //AFTER RESETTING EVERY CONFIG IS LOST
}
*/
//------------------------------------------------------------------------------------------------//
static BOOL ValidateSPM(U8 startbyte)
{
    U8 i;
    U32 looper;

    for(i = startbyte;i<8;i++)
    {
      if (writeBuffer[i+1] == ReadBuffer[i])
      {
        //nop
      }
      else
      {
        return FALSE;
      }
    }

    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(looper=0;looper<0x0FFFFF;looper++){}//WAIT a small time to clear the INT
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static void SpmWriteAndRead(U8 baseadress)
{
      U32 i=0;
      SpmWrite8(baseadress,writeBuffer);     //BASE adress is 0x00 (needs to be modulo of 0x08
      for(i=0;i<0x1FFFFF;i++){}              //WAIT 100ms -- INT B SHOULD assert (go low after 30ms)
      SpmRead8(baseadress,ReadBuffer);       //CHECK IF WRITING WAS SUCCESFULL
}
//------------------------------------------------------------------------------------------------//
//Configure the Sensitivity settings for the PSIControl glass plate
static void Configure_SX864x_Plexi(void)
{
    U32 i=0;

    //--------------STEP 1---------------------
    Write_Register(SOFTRESET,0xDE);//RESET CHIP LOWER --> INTB GOES LOW
    Write_Register(SOFTRESET,0x00);//REQUIRED TO START THE CHIP AGAIN
    for(i=0;i<0x3FFFFF;i++){}//WAIT 200ms -- INT B SHOULD BE HIGH AGAIN (meaning not active)
    Read_Register(IRQSRC);//In the hope of clearing this register. So INTB SHOULD GO HIGH
    for(i=0;i<0x0FFFFF;i++){}//WAIT a small time to clear the INT
    //-----------------------------------------

    //Block 1 --> Change active scan period
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//Reserved
    writeBuffer[2] = 0x00;//Reserved
    #if CHIP_TYPE == CHIP_TYPE_SX8644
    writeBuffer[3] = 0x10;//Reserved
    #else
    writeBuffer[3] = 0x18;//Reserved
    #endif

    writeBuffer[4] = 0x00;//Reserved
    writeBuffer[5] = 0x2B;//I2CAddress
    writeBuffer[6] = 0x01;//ActiveScanPeriod        -----> IMPORTANT
    writeBuffer[7] = 0x0C;//DozeScanPeriod
    writeBuffer[8] = 0x00;//PassiveTimer
    do
    {
      SpmWriteAndRead(0x00);
    }
    while (ValidateSPM(4) == FALSE); //Start Checking from the 4th byte because the first 4 are Reserved Bytes

    //Block 2 --> SET CAP MODE & SENSITIVITY
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//Reserved
    writeBuffer[2] = 0x01;//Capmodeminsc            -----> IMPORTANT

    #if CHIP_TYPE == CHIP_TYPE_SX8644
        writeBuffer[3] = 0xAA;//CapMode11_8
        writeBuffer[4] = 0xAA;//CapMode7_4
        writeBuffer[5] = 0xAA;//CapMode3_0
    #else
        writeBuffer[3] = 0xFF;//CapMode11_8
        writeBuffer[4] = 0xFF;//CapMode7_4
        writeBuffer[5] = 0xFF;//CapMode3_0
    #endif

    writeBuffer[6] = 0x40;//Capsense0_1             -----> IMPORTANT //0x66 looks stable , The MSB defines the CAP0 sense vals for common setup
    writeBuffer[7] = 0x55;//Capsense2_3
    writeBuffer[8] = 0x55;//Capsense4_5
    do
    {
      SpmWriteAndRead(0x08);
    }
    while (ValidateSPM(0) == FALSE);    //Check all Bytes

    //Block 3 --> Change the induvidual treshhold levels
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x55;//Capsense6_7
    writeBuffer[2] = 0x55;//Capsense8_9
    writeBuffer[3] = 0x55;//Capsense10_11
    writeBuffer[4] = 0x50;//CapTresh0       A0 default -- 50 grote gaps -- 30 te sensitive
    writeBuffer[5] = 0x50;//CapTresh1
    writeBuffer[6] = 0x50;//CapTresh2
    writeBuffer[7] = 0x50;//CapTresh3
    writeBuffer[8] = 0x50;//CapTresh4
    do
    {
      SpmWriteAndRead(CAPSENSITIVITY6_);
    }
    while (ValidateSPM(0) == FALSE);

    //Block 4 --> Change the induvidual treshhold levels
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x50;//CapTresh5
    writeBuffer[2] = 0x50;//CapTresh6
    writeBuffer[3] = 0x50;//CapTresh7
    writeBuffer[4] = 0x50;//CapTresh8
    writeBuffer[5] = 0x50;//CapTresh9
    writeBuffer[6] = 0x50;//CapTresh10
    writeBuffer[7] = 0x50;//CapTresh11
    writeBuffer[8] = 0x01;//CapPerComp
    do
    {
      SpmWriteAndRead(CAPTHRESH5);
    }
    while (ValidateSPM(0) == FALSE);

    //Block 5
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//RESERVED
    writeBuffer[2] = 0x30;//BtnCfg
    writeBuffer[3] = 0x50;//BtnAvgTHresh
    writeBuffer[4] = 0x50;//BtnCompNegThresh
    writeBuffer[5] = 0x01;//BtnCompNegCntMax
    writeBuffer[6] = 0x0A;//BtnHysteresis
    writeBuffer[7] = 0x00;//BtnStuckAtTimeout
    writeBuffer[8] = 0x00;//Sldconfig
    do
    {
      SpmWriteAndRead(0x20);
    }
    while (ValidateSPM(0) == FALSE);

    //Block 6 --> Set the max return value if touched (NORM)
    writeBuffer[0] = 0x00;//Indicating 8ConWriteByte
    writeBuffer[1] = 0x00;//WhlStuckAtTimeOut
    writeBuffer[2] = 0x06;//WhlHysteresis
    writeBuffer[3] = 0xFF;//RESERVED
    writeBuffer[4] = 0xFF;//SldrNormMsb             -----> IMPORTANT
    writeBuffer[5] = 0xFF;//SldrNormLsb
    writeBuffer[6] = 0x50;//WhlAvgThresh
    writeBuffer[7] = 0x50;//SliderCompNegThresh
    writeBuffer[8] = 0x01;//SliderCompNegCntMax
    do
    {
      SpmWriteAndRead(WHLSTUCKATTIMEOUT);
    }
    while (ValidateSPM(0) == FALSE);

    //AFTER RESETTING EVERY CONFIG IS LOST
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSliderSx864x_Get(U8 slider_id, U16* slider_value_ptr)
{
    static U8 touchedbuffer[1];
    touchedbuffer[0] = 0x01;
    //CHECK if Slider Touched happened in register 0x01, door touchedbuffer static te maken kwam die soms 0
    DrvI2cMasterDevice_WriteData(i2c_device_hndl, &touchedbuffer[0], 1, TRUE);
    DrvI2cMasterDevice_ReadData(i2c_device_hndl, &touchedbuffer[0], 1, TRUE);

    if (touchedbuffer[0] && 0x10)// mask the value with Slider Touched bit on page 66 on datasheet
    {
        U8 WhlPosMsbBuf[1] = {0x03};
        U8 WhlPosLsbBuf[1] = {0x04};

        U8 testbuffer[2] = {0x00,0x00};//this buffer contains the adress of the READ LSB & MSB first
        DrvI2cMasterDevice_WriteData(i2c_device_hndl, WhlPosMsbBuf, 1, TRUE);
        DrvI2cMasterDevice_ReadData(i2c_device_hndl, testbuffer, 1, TRUE); //now the buffer contains the MSB
        DrvI2cMasterDevice_WriteData(i2c_device_hndl, WhlPosLsbBuf, 1, TRUE);
        DrvI2cMasterDevice_ReadData(i2c_device_hndl, &testbuffer[1], 1, TRUE); //and now the buffer contains the LSB

        LOG_TRM("WHLPOSMSB is  %d ", PU8(testbuffer[0]));
        LOG_TRM("WHLPOSLSB is  %d ", PU8(testbuffer[1]));

        //MAXIMAAL kunnen (0xFFFF(SliderNorm)* 11/32) terug krijgen, dat is dus 22527,6

        *slider_value_ptr = ((((testbuffer[0] << 8) + testbuffer[1]))*2.8);

        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
SLIDER_HNDL DrvSliderSx864x_Register(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
    i2c_device_hndl = DrvI2cMasterDevice_Register(i2c_channel,address, 400000);
    Configure_SX864x_Plexi();
    //Configure_SX8644();

    slider_struct.get_value_hook = DrvSliderSx864x_Get;//Addres of function in the hook
    slider_struct.slider_id = 0;

    return &slider_struct;
}
//================================================================================================//


