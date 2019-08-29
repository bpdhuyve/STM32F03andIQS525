//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// CPU and System functionality
// This system module offers functionality for initialising CPU and other system parts.\n\n\n
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORE__SYSTEM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the clock source used as system
//          see IS_RCC_SYSCLK_SOURCE() for valid options
#ifndef SYS_SCLK_SOURCE
    #error "SYS_SCLK_SOURCE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This defines the PLL source
//          see IS_RCC_PLL_SOURCE() for valid options
// @remark  Only used when SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK
#ifndef SYS_PLL_SOURCE
    #error "SYS_PLL_SOURCE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This defines the PLL predivider
//          See IS_RCC_PREDIV1() for the valid options
// @remark  Only used when SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK
#ifndef SYS_PLL_PREDIV
    #error "SYS_PLL_PREDIV not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This defines the PLL multiplier
//          See IS_RCC_PLL_MUL() for the valid options
// @remark  Only used when SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK
#ifndef SYS_PLL_MUL
    #error "SYS_PLL_MUL not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the oscilator frequency in HZ (4MHz - 16MHz)
// @remark  Only used when HSE is used
#ifndef SYS_OSC_CLOCK_IN_HZ
    #error "SYS_OSC_CLOCK_IN_HZ not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the AHB clock.
//          see IS_RCC_HCLK() for valid options
#ifndef SYS_AHB_CONFIG
    #error "SYS_AHB_CONFIG not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the low speed APB clock (PCLK1)
//          This clock is derived from the AHB clock (HCLK).
//          see IS_RCC_PCLK() for valid options
#ifndef SYS_PCLK_CONFIG
    #error "SYS_PCLK_CONFIG not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the clock source used for the ADC
//          see IS_RCC_ADCCLK() for valid options
#ifndef SYS_ADC_CLK_SOURCE
    #error "SYS_ADC_CLK_SOURCE not defined in AppConfig.h"
#endif

//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "core\SysLibAll.h"
#include "core\stm32f0xx_pwr.h"
#include "core\stm32f0xx_iwdg.h"
#include "core\stm32f0xx_flash.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void System_InitDataSegments(void);
static void System_InitClocks(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
__no_init SYSTEM_SIGNATURES       system_signatures @ "ram_signature";
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief   Device reset function
// should be static, but CANNOT be because of debugger/IAR adding extra (overhead) startupcode to binary
static void System_InitDataSegments(void)
{
  unsigned char *Src;
  unsigned char *Dest;
  U32 size;
  U32 i;

  #pragma section = ".data_init"
  #pragma section = ".data"
  #pragma section = ".bss"
  #pragma section = ".noinit"

   Src = __section_begin(".data_init");
   Dest = __section_begin(".data");
   size = __section_size(".data_init");

   //copy the initializers from ROM to RAM
   for(i = 0; i < size; i++)
   {
      *Dest++=*Src++;
   }

   Dest = __section_begin(".bss");
   size = __section_size(".bss");
   
    //zero init bss segment
   for(i = 0; i < size; i++)
   {
      *Dest++=0;
   }
}
//------------------------------------------------------------------------------------------------//
static void System_InitClocks(void)
{
    if((SYS_SCLK_SOURCE == RCC_SYSCLKSource_HSE) || ((SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK) && (SYS_PLL_SOURCE != RCC_PLLSource_HSI_Div2)))
    {
        /* Enable HSE */
        RCC_HSEConfig(RCC_HSE_ON);
        
        /* Wait till HSE is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
    }
    else
    {
        /* Enable HSI */
        RCC_HSICmd(ENABLE);
    }
    
    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_1);

    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(ENABLE);

    if(SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK)
    {
        if(SYS_PLL_SOURCE == RCC_PLLSource_PREDIV1)
        {
            RCC_PREDIV1Config(SYS_PLL_PREDIV);
        }
        
        /* Config PLLCLK */
        RCC_PLLConfig(SYS_PLL_SOURCE, SYS_PLL_MUL);
    
        /* Enable PLL */
        RCC_PLLCmd(ENABLE);
    
        /* Wait till PLL is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(SYS_SCLK_SOURCE);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != (U8)(SYS_SCLK_SOURCE << 2));

    /* config HCLK, PCLK1 and PCLK2 */
    RCC_HCLKConfig(SYS_AHB_CONFIG);
    RCC_PCLKConfig(SYS_PCLK_CONFIG);

    /* Enable Peripheral clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
   
    /* Enable Voltage detector */
   // PWR_PVDLevelConfig(PWR_PVDLevel_2V5);
   // PWR_PVDCmd(ENABLE);

    /* Enable write access to Backup domain */
   // PWR_BackupAccessCmd(ENABLE);

    /* Clear Tamper pin Event(TE) pending flag */
   // BKP_ClearFlag();

    RCC_LSICmd(ENABLE); //enable internal low speed clock because the IWDG uses this
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    //enable watchdog
    IWDG_SetPrescaler(IWDG_Prescaler_16);
    
    RCC_USARTCLKConfig(RCC_USART1CLK_PCLK);
    RCC_ADCCLKConfig(SYS_ADC_CLK_SOURCE);
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void System_Init(void)
{
    System_InitDataSegments();
    
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();
    
    System_InitClocks();
}
//------------------------------------------------------------------------------------------------//
void System_EnableWatchdog(U32 overflow_time_in_ms)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetReload((overflow_time_in_ms * 5) >> 1);
    IWDG_ReloadCounter();
    IWDG_Enable();
}
//------------------------------------------------------------------------------------------------//
void System_KickDog(void)
{
    IWDG_ReloadCounter();
}
//------------------------------------------------------------------------------------------------//
U32 SysGetAHBClk(void)
{
    U32 sys_clk;
    U32 ahb_clk;

   sys_clk = SysGetSysClk();

    if (SYS_AHB_CONFIG == RCC_SYSCLK_Div1)
    {
        ahb_clk = sys_clk;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div2)
    {
        ahb_clk = sys_clk / 2;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div4)
    {
        ahb_clk = sys_clk / 4;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div8)
    {
        ahb_clk = sys_clk / 8;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div16)
    {
        ahb_clk = sys_clk / 16;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div64)
    {
        ahb_clk = sys_clk / 64;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div128)
    {
        ahb_clk = sys_clk / 128;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div256)
    {
        ahb_clk = sys_clk / 256;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div512)
    {
        ahb_clk = sys_clk / 512;
    }

    return(ahb_clk);
}
//------------------------------------------------------------------------------------------------//
U32 SysGetPreDivClk(void)
{
    if(SYS_PLL_PREDIV == RCC_PREDIV1_Div1)
    {
        return SYS_OSC_CLOCK_IN_HZ;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div2)
    {
        return SYS_OSC_CLOCK_IN_HZ / 2;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div3)
    {
        return SYS_OSC_CLOCK_IN_HZ / 3;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div4)
    {
        return SYS_OSC_CLOCK_IN_HZ / 4;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div5)
    {
        return SYS_OSC_CLOCK_IN_HZ / 5;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div6)
    {
        return SYS_OSC_CLOCK_IN_HZ / 6;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div7)
    {
        return SYS_OSC_CLOCK_IN_HZ / 7;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div8)
    {
        return SYS_OSC_CLOCK_IN_HZ / 8;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div9)
    {
        return SYS_OSC_CLOCK_IN_HZ / 9;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div10)
    {
        return SYS_OSC_CLOCK_IN_HZ / 10;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div11)
    {
        return SYS_OSC_CLOCK_IN_HZ / 11;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div12)
    {
        return SYS_OSC_CLOCK_IN_HZ / 12;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div13)
    {
        return SYS_OSC_CLOCK_IN_HZ / 13;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div14)
    {
        return SYS_OSC_CLOCK_IN_HZ / 14;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div15)
    {
        return SYS_OSC_CLOCK_IN_HZ / 15;
    }
    else
    {
        return SYS_OSC_CLOCK_IN_HZ / 16;
    }
}
//------------------------------------------------------------------------------------------------//
U32 SysGetSysClk(void)
{
    U32 pll_src;
    U32 pll_clk;
    
    if (SYS_SCLK_SOURCE == RCC_SYSCLKSource_HSI)
    {
        return 8000000;
    }
    else if (SYS_SCLK_SOURCE == RCC_SYSCLKSource_HSE)
    {
        return SYS_OSC_CLOCK_IN_HZ;
    }
    else
    {
        if (SYS_PLL_SOURCE == RCC_PLLSource_HSI_Div2)
        {
            pll_src = 4000000;
        }
        else if (SYS_PLL_SOURCE == RCC_PLLSource_PREDIV1)
        {
            pll_src = SysGetPreDivClk();
        }

        if (SYS_PLL_MUL == RCC_PLLMul_2)
        {
            pll_clk = 2 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_3)
        {
            pll_clk = 3 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_4)
        {
            pll_clk = 4 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_5)
        {
            pll_clk = 5 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_6)
        {
            pll_clk = 6 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_7)
        {
            pll_clk = 7 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_8)
        {
            pll_clk = 8 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_9)
        {
            pll_clk = 9 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_10)
        {
            pll_clk = 10 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_11)
        {
            pll_clk = 11 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_12)
        {
            pll_clk = 12 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_13)
        {
            pll_clk = 13 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_14)
        {
            pll_clk = 14 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_15)
        {
            pll_clk = 15 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_16)
        {
            pll_clk = 16 * pll_src;
        }
    }
    return (pll_clk);
}
//------------------------------------------------------------------------------------------------//
U32 SysGetAPBClk(void)
{
    if(SYS_PCLK_CONFIG == RCC_HCLK_Div2)
    {
        return (SysGetAHBClk() >> 1);
    }
    else if(SYS_PCLK_CONFIG == RCC_HCLK_Div4)
    {
        return (SysGetAHBClk() >> 2);
    }
    else if(SYS_PCLK_CONFIG == RCC_HCLK_Div8)
    {
        return (SysGetAHBClk() >> 3);
    }
    else if(SYS_PCLK_CONFIG == RCC_HCLK_Div16)
    {
        return (SysGetAHBClk() >> 4);
    }
    else
    {
        return SysGetAHBClk();
    }
}
//------------------------------------------------------------------------------------------------//
U32 SysGetTimClk(void)
{
    if(SYS_PCLK_CONFIG == RCC_HCLK_Div1)
    {
        return SysGetAPBClk();
    }
    else
    {
        return (SysGetAPBClk() << 1);
    }
}
//------------------------------------------------------------------------------------------------//
U32 SysGetHSEClk(void)
{
    return SYS_OSC_CLOCK_IN_HZ;
}
//------------------------------------------------------------------------------------------------//
void SysSetMCOClockSource(U8 clock_source)
{
    RCC_MCOConfig(clock_source, RCC_MCOPrescaler_1);
}
//------------------------------------------------------------------------------------------------//
void System_Stop(void)
{
    PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFE);
    
    System_InitClocks();
}
//================================================================================================//
