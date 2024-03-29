//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file defines all application specific DEFINITIONS for library entities
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "AppVersion.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// LOG LEVEL
//------------------------------------------------------------------------------------------------//
#define LOG_LEVEL_DEFAULT                       LOG_LEVEL_NONE
//------------------------------------------------------------------------------------------------//
// SYS
//------------------------------------------------------------------------------------------------//
#ifdef CORE__SYSTEM_C
    #define SYS_SCLK_SOURCE                                     RCC_SYSCLKSource_HSI
    #define SYS_PLL_PREDIV                                      RCC_PREDIV1_Div2
    #define SYS_PLL_MUL                                         RCC_PLLMul_2
    #define SYS_PLL_SOURCE                                      RCC_CFGR_PLLSRC_HSI_Div2
    #define SYS_OSC_CLOCK_IN_HZ                                 8000000
    #define SYS_AHB_CONFIG                                      RCC_SYSCLK_Div1
    #define SYS_PCLK_CONFIG                                     RCC_HCLK_Div1
    #define SYS_ADC_CLK_SOURCE                                  RCC_ADCCLK_PCLK_Div2
#endif
//------------------------------------------------------------------------------------------------//
#ifdef CORE__SYSINT_C
    #define VECT_TAB_RAM
    #define ADDRESS_INITIAL_STACK_POINTER                       0x20000FFF
#endif
//------------------------------------------------------------------------------------------------//
// DRV
//------------------------------------------------------------------------------------------------//
#ifdef SCI__DRVSCICHANNEL_C
    #define SCI_RS485_COUNT                                     0
#endif
//------------------------------------------------------------------------------------------------//
#ifdef GPIO__DRVGPIOSYS_C
	#define DRVGPIOSYS_MAX_PINS			                        30
#endif
//------------------------------------------------------------------------------------------------//
#ifdef ADC__DRVADCSYSBLOCK_C
#endif
//------------------------------------------------------------------------------------------------//
#ifdef TIMER__DRVTIMERTICK_C
	#define DRVTIMERTICK_COUNT			                        2
#endif
//------------------------------------------------------------------------------------------------//
#ifdef TIMER_DRVPWMSYS_C
	#define DRVPWMSYS_COUNT			                            3
#endif
//------------------------------------------------------------------------------------------------//
// CORE
//------------------------------------------------------------------------------------------------//
#ifdef CORE_C
    #define INCLUDE_TESTMODE                                    0
    #define WATCHDOG_ENABLED                                    1
    #define WATCHDOG_TIMEOUT_IN_MS                              1000000
    #define USE_STACK_SAFETYCHECKER                             1
    #define STACK_POINTER_START_ADDRESS                         0x20000FFF
    #define STACK_POINTER_END_ADDRESS                           (0x20000FFF-0x400)
#endif
//------------------------------------------------------------------------------------------------//
#ifdef COREBUFFER_C
    #define TOTAL_BUFFER_SPACE                                  800
#endif
//------------------------------------------------------------------------------------------------//
#ifdef COREQ_C
    #define QUEUE_COUNT                                         7
#endif
//------------------------------------------------------------------------------------------------//
#ifdef CORELOG_C
    #define LOG_DATA_QUEUE_SIZE                                 400
    #define LOG_OUT_QUEUE_SIZE                                  100
#endif
//------------------------------------------------------------------------------------------------//
#ifdef CORELOG_H
    #define CORELOG_TAG                                         TAG_NONE
#endif
//------------------------------------------------------------------------------------------------//
#ifdef CORETERM_C
    #define TERMINAL_BAUDRATE                                   SCI_SPEED_115200_BPS
#endif
//------------------------------------------------------------------------------------------------//
//#ifdef CORETASK_C
    #define TASK_COUNT                                          6
    #define TASK_TICK_IN_US                                     100
//#endif
//------------------------------------------------------------------------------------------------//
// STD
//------------------------------------------------------------------------------------------------//

//------------------------------------------------------------------------------------------------//
// MISC
//------------------------------------------------------------------------------------------------//
#define MAX_NUMBER_OF_COMMANDS                                  30
#define SWITCH_COUNT                                            1
#define ANTIBOUNCE_COUNT                                        40
#define NEWLINE                                                 "\n"

#define COMMDLLDMX_BREAK_TIMING                                 COMMDLLDMX_BREAK_TIMING_BY_TASK
    #define COMMDLLDMX_MAX_DATA_SLOTS 3
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//

