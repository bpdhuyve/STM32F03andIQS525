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
//  Functional     |  MATS+ | MarchC- | MarchB | PMOVI | MarchU | MarchLR | MarchSR | MarchSS |
//  fault models   |   (5n) |  (10n)  |  (17n) | (13n) |  (13n) |  (14n)  |  (14n)  |  (22n)  |
//  ---------------+--------+---------+--------+-------+--------+---------+---------+---------+
//  SF             |   2/2  |   2/2   |   2/2  |  2/2  |   2/2  |   2/2   |   2/2   |   2/2   |
//  TF             |   1/2  |   2/2   |   2/2  |  2/2  |   2/2  |   2/2   |   2/2   |   2/2   |
//  WDF            |   0/2  |   0/2   |   0/2  |  0/2  |   0/2  |   0/2   |   0/2   |   2/2   |
//  RDF            |   2/2  |   2/2   |   2/2  |  2/2  |   2/2  |   2/2   |   2/2   |   2/2   |
//  DRDF           |   0/2  |   0/2   |   0/2  |  2/2  |   0/2  |   0/2   |   2/2   |   2/2   |
//  IRF            |   2/2  |   2/2   |   2/2  |  2/2  |   2/2  |   2/2   |   2/2   |   2/2   |
//  CFst           |   4/8  |   8/8   |   6/8  |  8/8  |   8/8  |   8/8   |   8/8   |   8/8   |
//  CFdsrx         |   3/8  |   8/8   |   7/8  |  8/8  |   8/8  |   8/8   |   8/8   |   8/8   |
//  CFdsxwx        |   3/8  |   8/8   |   8/8  |  7/8  |   8/8  |   8/8   |   8/8   |   8/8   |
//  CFdsxwx        |   0/8  |   0/8   |   0/8  |  0/8  |   0/8  |   0/8   |   0/8   |   8/8   |
//  CFtr           |   2/8  |   8/8   |   4/8  |  8/8  |   8/8  |   8/8   |   8/8   |   8/8   |
//  CFwd           |   0/8  |   0/8   |   0/8  |  0/8  |   0/8  |   0/8   |   0/8   |   8/8   |
//  CFrd           |   4/8  |   8/8   |   4/8  |  8/8  |   8/8  |   8/8   |   8/8   |   8/8   |
//  CFdrd          |   0/8  |   0/8   |   0/8  |  0/8  |   0/8  |   0/8   |   6/8   |   8/8   |
//  CFir           |   4/8  |   8/8   |   4/8  |  8/8  |   8/8  |   8/8   |   8/8   |   8/8   |
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SYSEXT__STDMEMCHECK_H
#define SYSEXT__STDMEMCHECK_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the size (in bits) of a single memory location (a single address)
#ifndef MEM_WIDTH
	#error "MEM_WIDTH not defined in AppConfig.h"
#else
    #if ((MEM_WIDTH == 8) | (MEM_WIDTH == 16) | (MEM_WIDTH == 32))
        //value ok
    #else
        #error "The value for MEM_WIDTH is not valid"
    #endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
#if (MEM_WIDTH == 8)
    typedef U8    MEM_VAL;
    typedef U8*   MEM_VAL_PTR;
#elif (MEM_WIDTH == 16)
    typedef U16   MEM_VAL;
    typedef U16*  MEM_VAL_PTR;
#elif (MEM_WIDTH == 32)
    typedef U32   MEM_VAL;
    typedef U32*  MEM_VAL_PTR;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief March A detection algorithm
// Detects AFs, SAFs, TFs, CFins, CFsts, CFids
// Detects Linked CFids, but not linked with TFs 
// Test length: 15*n
//  {up/down(w0);up(r0,w1,w0,w1);up(r1,w0,w1);down(r1,w0,w1,w0);down(r0,w1,w0)}
//   M0          M1              M2           M3                M4
// @param   mem_start_ptr        pointer to the beginning of the memory under test
// @param   mem_end_ptr          pointer to the end of the memory under test (mem_start_ptr < mem_end_ptr)
// @return TRUE if memory OK, false if problem was detected 
BOOL StdMemCheckMarchA(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr);

// @brief March B detection algorithm
// Detects all faults of March A.
// Detects CFids linked with TFs, because M1 detects all TFs
// Test length: 17*n
//  {up/down(w0);up(r0,w1,r1,w0,r0,w1);up(r1,w0,w1);down(r1,w0,w1,w0);down(r0,w1,w0)}
//   M0          M1                    M2           M3                M4
// @param   mem_start_ptr        pointer to the beginning of the memory under test
// @param   mem_end_ptr          pointer to the end of the memory under test (mem_start_ptr < mem_end_ptr)
// @return TRUE if memory OK, false if problem was detected 
BOOL StdMemCheckMarchB(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr);

// @brief March SS detection algorithm
// Detects all all simple static faults.
// Uncertain how much of the linked faults are detected.
// Test length: 22*n
//  {up/down(w0);up(r0,r0,w0,r0,w1);up(r1,r1,w1,r1,w0);down(r0,r0,w0,r0,w1);down(r1,r1,w1,r1,w0);up/down(r0)}
//   M0          M1                 M2                 M3                   M4
// @param   mem_start_ptr        pointer to the beginning of the memory under test
// @param   mem_end_ptr          pointer to the end of the memory under test (mem_start_ptr < mem_end_ptr)
// @return TRUE if memory OK, false if problem was detected 
BOOL StdMemCheckMarchSS(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr);

// @brief March LA detection algorithm
// Detects all simple faults as well as all linked faults, involving an arbitrary number of simple faults.
// Test length: 22*n
//  {up/down(w0);up(r0,w1,w0,w1,r1);up(r1,w0,w1,w0,r0);down(r0,w1,w0,w1,r1);down(r1,w0,w1,w0,r0);up/down(r0)}
//   M0          M1                 M2                 M3                   M4
// @param   mem_start_ptr        pointer to the beginning of the memory under test
// @param   mem_end_ptr          pointer to the end of the memory under test (mem_start_ptr < mem_end_ptr)
// @return TRUE if memory OK, false if problem was detected 
BOOL StdMemCheckMarchLA(MEM_VAL_PTR mem_start_ptr, MEM_VAL_PTR mem_end_ptr);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SYSEXT__STDMEMCHECK_H */
