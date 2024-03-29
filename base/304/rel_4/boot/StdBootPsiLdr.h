//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this file contains the bootldr functionality
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef BOOT__STDBOOTPSILDR_H
#define BOOT__STDBOOTPSILDR_H
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern U32 stdbootpsildr_appaddress;
extern U32 stdbootpsildr_appendaddress;
extern U32 stdbootpsildr_newappaddress;
extern U32 stdbootpsildr_newappendaddress;
extern U32 stdbootpsildr_sectorsize;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
BOOL StdBootPsiLdr_CheckForNewAppHeaders(void);
BOOL StdBootPsiLdr_ValidateNewAppHeaders(void);
void StdBootPsiLdr_EraseNewAppHeaderDestinations(void);
void StdBootPsiLdr_LoadNewAppHeaders(void);
void StdBootPsiLdr_EraseNewAppRegion(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* BOOT__STDBOOTPSILDR_H */

