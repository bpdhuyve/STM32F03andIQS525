//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor independent prototypes and definitions of the pin configuration
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef GPIO__ISYSPIN_H
#define GPIO__ISYSPIN_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "gpio\ISysGpio.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SysPin_InitPin(port, pin, func)      SysPin_InitPinMask(port, (0x0001 << pin), func)
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
// @remark this function inits stuff needed for syspin to work, this fucntion could be empty
void SysPin_Init(void);

// @remark none
void SysPin_InitPinMask(GPIO_PORT io_port, U32 io_pin_mask, SYS_PIN_FUNC pin_func);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif  /* GPIO__ISYSPIN_H */
