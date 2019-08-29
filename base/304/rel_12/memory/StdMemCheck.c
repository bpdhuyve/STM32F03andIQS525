//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Memory checker algorithms
// Module offering several algorithms to check memory for possible faults :\n
//   SAF   Stuck-At Fault\n
//               Cell stuck\n
//               Driver stuck\n
//               Read/write line stuck\n
//               Chip-select line stuck\n
//               Data line stuck\n
//               Open in data line\n
//   TF    Transition Fault\n
//               Cell can be set to 0 but not to 1 (or vice-versa)\n
//   AF    Address Fault\n
//               Address line stuck\n
//               Open in address line\n
//               Open decoder\n
//               Shorts between address lines\n
//               Wrong access\n
//               Multiple access\n
//   CF    Coupling Fault\n
//               Short between data lines\n
//               Crosstalk between data lines\n
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SYSEXT__STDMEMCHECK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEMORY__STDMEMCHECKER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MEMORY__STDMEMCHECKER_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "memory\StdMemCheck.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define LOOP_UP_BEGIN     mem_ptr = mem_start_ptr;                                           \
                          while(mem_ptr <= mem_end_ptr)                                      \
                          {                                                                  \
                              for(mem_bitmask = 0x01; mem_bitmask != 0; mem_bitmask <<= 1)   \
                              {
#define LOOP_UP_END           }                                                              \
                              mem_ptr++;                                                     \
                          }

#define LOOP_DOWN_BEGIN   mem_ptr = mem_end_ptr;                                             \
                          while(mem_ptr >= mem_start_ptr)                                    \
                          {                                                                  \
                              for(mem_bitmask = (((MEM_VAL)0x01) << (MEM_WIDTH-1));mem_bitmask != 0;mem_bitmask >>= 1) \
                              {
#define LOOP_DOWN_END         }                                                              \
                              mem_ptr--;                                                     \
                          }

#define RD_0                      if((*mem_ptr & mem_bitmask) != 0) return FALSE;
#define RD_1                      if((*mem_ptr & mem_bitmask) == 0) return FALSE;
#define WR_0                      *mem_ptr &= (MEM_VAL)(~((U32)mem_bitmask));
#define WR_1                      *mem_ptr |= mem_bitmask;
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void StdMemCheckWriteZero(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void StdMemCheckWriteZero(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr)
{
    MEM_VAL_PTR mem_ptr;
    //up or down : write 0
    mem_ptr = mem_end_ptr;
    while(mem_ptr >= mem_start_ptr)
    {
        System_KickDog();
        *mem_ptr = 0;
        mem_ptr--;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief March A detection algortihm
// @note : detects AF + SAF + TF + CF (unlinked)
BOOL StdMemCheckMarchA(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr)
{
    MEM_VAL_PTR mem_ptr;
    MEM_VAL mem_val;
    MEM_VAL mem_bitmask;

    //up or down : write 0
    StdMemCheckWriteZero(mem_start_ptr, mem_end_ptr);

    //up : read 0, write 1, write 0, write 1
    mem_ptr = mem_start_ptr;
    while(mem_ptr <= mem_end_ptr)
    {
        mem_val = 0;
        for(mem_bitmask = 0x01; mem_bitmask != 0; mem_bitmask <<= 1)
        {
            if(*mem_ptr != mem_val) return FALSE; //r0
            *mem_ptr = mem_val + mem_bitmask;     //w1
            *mem_ptr = mem_val;                   //w0
            mem_val += mem_bitmask;
            *mem_ptr = mem_val;                   //w1
        }
        mem_ptr++;
        System_KickDog();
    }

    //up : read 1, write 0, write 1
    mem_ptr = mem_start_ptr;
    while(mem_ptr <= mem_end_ptr)
    {
        for(mem_bitmask = 0x01; mem_bitmask != 0; mem_bitmask <<= 1)
        {
            if(*mem_ptr != ((MEM_VAL) 0xFFFFFFFF)) return FALSE; //r1
            *mem_ptr = ((MEM_VAL) 0xFFFFFFFF) - mem_bitmask;     //w0
            *mem_ptr = ((MEM_VAL) 0xFFFFFFFF);                   //w1
        }
        mem_ptr++;
        System_KickDog();
    }

    //down : read 1, write 0, write 1, write 0
    mem_ptr = mem_end_ptr;
    while(mem_ptr >= mem_start_ptr)
    {
        mem_val = (MEM_VAL) 0xFFFFFFFF;
        for(mem_bitmask = (((MEM_VAL)0x01) << (MEM_WIDTH-1)); mem_bitmask != 0; mem_bitmask >>= 1)
        {
            if(*mem_ptr != mem_val) return FALSE; //r1
            *mem_ptr = mem_val - mem_bitmask;     //w0
            *mem_ptr = mem_val;                   //w1
            mem_val -= mem_bitmask;
            *mem_ptr = mem_val;                   //w0
        }
        mem_ptr--;
        System_KickDog();
    }

    //down : read 0, write 1, write 0
    mem_ptr = mem_end_ptr;
    while(mem_ptr >= mem_start_ptr)
    {
        for(mem_bitmask = (((MEM_VAL)0x01) << (MEM_WIDTH-1)); mem_bitmask != 0; mem_bitmask >>= 1)
        {
            if(*mem_ptr != 0) return FALSE; //r0
            *mem_ptr = mem_bitmask;         //w1
            *mem_ptr = 0;                   //w0
        }
        mem_ptr--;
        System_KickDog();
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdMemCheckMarchB(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr)
{
    MEM_VAL_PTR mem_ptr;
    MEM_VAL mem_val;
    MEM_VAL mem_bitmask;
    MEM_VAL mem_bitmask_new;

    //up or down : write 0
    StdMemCheckWriteZero(mem_start_ptr, mem_end_ptr);

    //up : read 0, write 1, read 1, write 0, read 0, write 1
    mem_ptr = mem_start_ptr;
    while(mem_ptr <= mem_end_ptr)
    {
        mem_val = 0;
        for(mem_bitmask = 0x01; mem_bitmask != 0; mem_bitmask <<= 1)
        {
        	mem_bitmask_new = mem_val + mem_bitmask;
            if(*mem_ptr != mem_val) return FALSE;         //r0
            *mem_ptr = mem_bitmask_new;                   //w1
            if(*mem_ptr != mem_bitmask_new) return FALSE; //r1
            *mem_ptr = mem_val;                           //w0
            if(*mem_ptr != mem_val) return FALSE;         //r0
            *mem_ptr = mem_bitmask_new;                   //w1
            mem_val = mem_bitmask_new;
        }
        mem_ptr++;
        System_KickDog();
    }

    //up : read 1, write 0, write 1
    mem_ptr = mem_start_ptr;
    while(mem_ptr <= mem_end_ptr)
    {
        for(mem_bitmask = 0x01; mem_bitmask != 0; mem_bitmask <<= 1)
        {
            if(*mem_ptr != ((MEM_VAL) 0xFFFFFFFF)) return FALSE; //r1
            *mem_ptr = ((MEM_VAL) 0xFFFFFFFF) - mem_bitmask;     //w0
            *mem_ptr = ((MEM_VAL) 0xFFFFFFFF);                   //w1
        }
        mem_ptr++;
        System_KickDog();
    }

    //down : read 1, write 0, write 1, write 0
    mem_ptr = mem_end_ptr;
    while(mem_ptr >= mem_start_ptr)
    {
        mem_val = (MEM_VAL) 0xFFFFFFFF;
        for(mem_bitmask = (((MEM_VAL)0x01) << (MEM_WIDTH-1)); mem_bitmask != 0; mem_bitmask >>= 1)
        {
            if(*mem_ptr != mem_val) return FALSE; //r1
            *mem_ptr = mem_val - mem_bitmask;     //w0
            *mem_ptr = mem_val;                   //w1
            mem_val -= mem_bitmask;
            *mem_ptr = mem_val;                   //w0
        }
        mem_ptr--;
        System_KickDog();
    }

    //down : read 0, write 1, write 0
    mem_ptr = mem_end_ptr;
    while(mem_ptr >= mem_start_ptr)
    {
        for(mem_bitmask = (((MEM_VAL)0x01) << (MEM_WIDTH-1)); mem_bitmask != 0; mem_bitmask >>= 1)
        {
            if(*mem_ptr != 0) return FALSE; //r0
            *mem_ptr = mem_bitmask;         //w1
            *mem_ptr = 0;                   //w0
        }
        mem_ptr--;
        System_KickDog();
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdMemCheckMarchSS(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr)
{
    MEM_VAL_PTR mem_ptr;
    MEM_VAL mem_bitmask;

    //up or down : write 0
    StdMemCheckWriteZero(mem_start_ptr, mem_end_ptr);

    //up(r0,r0,w0,r0,w1);
    LOOP_UP_BEGIN
            RD_0  RD_0  WR_0  RD_0  WR_1
            System_KickDog();
    LOOP_UP_END

    //up(r1,r1,w1,r1,w0);
    LOOP_UP_BEGIN
            RD_1  RD_1  WR_1  RD_1  WR_0
            System_KickDog();
    LOOP_UP_END

    //down(r0,r0,w0,r0,w1);
    LOOP_DOWN_BEGIN
            RD_0  RD_0  WR_0  RD_0  WR_1
            System_KickDog();
    LOOP_DOWN_END

    //down(r1,r1,w1,r1,w0);
    LOOP_DOWN_BEGIN
            RD_1  RD_1  WR_1  RD_1  WR_0
            System_KickDog();
    LOOP_DOWN_END

    //up/down(r0)
    LOOP_DOWN_BEGIN
            RD_0
            System_KickDog();
    LOOP_DOWN_END

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdMemCheckMarchLA(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr)
{
    MEM_VAL_PTR mem_ptr;
    MEM_VAL mem_bitmask;

    //up or down : write 0
    StdMemCheckWriteZero(mem_start_ptr, mem_end_ptr);

    //up(r0,w1,w0,w1,r1);
    LOOP_UP_BEGIN
            RD_0  WR_1  WR_0  WR_1  RD_1
            System_KickDog();
    LOOP_UP_END

    //up(r1,w0,w1,w0,r0);
    LOOP_UP_BEGIN
            RD_1  WR_0  WR_1  WR_0  RD_0
            System_KickDog();
    LOOP_UP_END

    //down(r0,w1,w0,w1,r1);
    LOOP_DOWN_BEGIN
            RD_0  WR_1  WR_0  WR_1  RD_1
             System_KickDog();
    LOOP_DOWN_END

    //down(r1,w0,w1,w0,r0);
    LOOP_DOWN_BEGIN
            RD_1  WR_0  WR_1  WR_0  RD_0
            System_KickDog();
    LOOP_DOWN_END

    //up/down(r0)
    LOOP_DOWN_BEGIN
            RD_0
            System_KickDog();
    LOOP_DOWN_END

    return TRUE;
}
//================================================================================================//
