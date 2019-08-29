//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Application main header file.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRV_SLIDER_Ad7147_H
#define DRV_SLIDER_Ad7147_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//#include "sys.h"
#include "i2c\DrvI2cMasterChannel.h"
//#include "i2c\DrvI2cMasterChannelSysInt.h"
#include "i2c\DrvI2cMasterDevice.h"
#include "slider\DrvSlider.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DEFAULT_AD7147_SLAVE_ADDRESS        0x2C

//defines used to refefine the STAGEx_CONNECTION defien in the c file
#define CIN0_TO_POS   (1<<(2*0))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN1_TO_POS   (1<<(2*1))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN2_TO_POS   (1<<(2*2))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN3_TO_POS   (1<<(2*3))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN4_TO_POS   (1<<(2*4))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN5_TO_POS   (1<<(2*5))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN6_TO_POS   (1<<(2*6))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN7_TO_POS   (1<<(2*8))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN8_TO_POS   (1<<(2*9))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN9_TO_POS   (1<<(2*10))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN10_TO_POS  (1<<(2*11))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN11_TO_POS  (1<<(2*12))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN12_TO_POS  (1<<(2*13))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register

//the idea is we never use this, the isr,s for low are disabled and the thresholds are not programmed
#define CIN0_TO_NEG   (2<<(2*0))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN1_TO_NEG   (2<<(2*1))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN2_TO_NEG   (2<<(2*2))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN3_TO_NEG   (2<<(2*3))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN4_TO_NEG   (2<<(2*4))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN5_TO_NEG   (2<<(2*5))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN6_TO_NEG   (2<<(2*6))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN7_TO_NEG   (2<<(2*8))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN8_TO_NEG   (2<<(2*9))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN9_TO_NEG   (2<<(2*10))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN10_TO_NEG  (2<<(2*11))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN11_TO_NEG  (2<<(2*12))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN12_TO_NEG  (2<<(2*13))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register

//all pins are connected to bias by default so this is also not needed
#define CIN0_TO_BIAS   (0<<(2*0))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN1_TO_BIAS   (0<<(2*1))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN2_TO_BIAS   (0<<(2*2))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN3_TO_BIAS   (0<<(2*3))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN4_TO_BIAS   (0<<(2*4))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN5_TO_BIAS   (0<<(2*5))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN6_TO_BIAS   (0<<(2*6))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN7_TO_BIAS   (0<<(2*8))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN8_TO_BIAS   (0<<(2*9))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN9_TO_BIAS   (0<<(2*10))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN10_TO_BIAS  (0<<(2*11))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN11_TO_BIAS  (0<<(2*12))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
#define CIN12_TO_BIAS  (0<<(2*13))		//inverse waarden volgens register omdat deze inverted worden als ze effectief geschreven worden in het register
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/// @brief  register all of the Ad7147 cap inputs as oen slider
SLIDER_HNDL DrvSliderAd7147_Register(I2C_CHANNEL_HNDL i2c_channel, U8 address);

/// @return true if connection is ok, false if not
/// @remark function can be performed before DrvSliderAd7147_Register
BOOL DrvSliderAd7147_TestConnection(I2C_CHANNEL_HNDL i2c_channel, U8 address);

/// @return the proximity mask for all pads, a '1' means proximity detected
U16 DrvSliderAd7147_GetProximityMask(void);

/// @return the touch mask for all pads, a '1' means the pad is touched
U16 DrvSliderAd7147_GetTouchMask(void);

void DrvSliderAd7147_PrintDebugData(void);
//================================================================================================//

#endif /* DRV_SLIDER_Ad7147_H */