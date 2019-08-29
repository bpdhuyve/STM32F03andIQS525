//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This is the source file of the encryption entity.
// This Encryption entity is designed to be used for encrypt the data bytes that should be programmed in the DSP.\n
// So it's unpossible to copy the code by others.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define ENCRYPT__STDENCRYPT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDENCRYPT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STDENCRYPT_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "encrypt\StdEncrypt.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define KEY_LENGTH             32
#define KEY_MASK               0x1F
#define DYNAMIC_KEY_LENGTH     64
#define DYNAMIC_KEY_MASK       0x3F
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U16    dyn[DYNAMIC_KEY_LENGTH];
    U16    counter_i;
    U16    counter_j;
    U16    init_const_1;
    U16    algo_const_1;
    U16    algo_const_2;
}
LOCAL_VAR_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static LOCAL_VAR_STRUCT                 locals;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdEncryptInitDecoder(U16* address_key_const_table_ptr)
{
    U16 tmp1;
    U16 tmp2;
    U16 tmp3;
    U16 *p_val1_ptr;
    U16 *p_val2_ptr;

    MEMCPY((VPTR)locals.dyn, 
			(VPTR)(address_key_const_table_ptr), 
			(KEY_LENGTH * SIZEOF(U16)));

    MEMCPY((VPTR)&locals.counter_i, 
			(VPTR)(address_key_const_table_ptr + KEY_LENGTH), 
			5 * (SIZEOF(U16)));

    p_val1_ptr = locals.dyn;
    for(tmp1 = KEY_LENGTH; tmp1 < DYNAMIC_KEY_LENGTH; tmp1++)
    {
        *(p_val1_ptr + KEY_LENGTH) = (*p_val1_ptr) + locals.init_const_1;
        p_val1_ptr++;
    }

    tmp2=0;
    for(tmp1 = 0; tmp1 < DYNAMIC_KEY_LENGTH; tmp1++)
    {
        p_val1_ptr = &locals.dyn[tmp1];
        tmp2 = (tmp2 + tmp1 + *p_val1_ptr) & DYNAMIC_KEY_MASK;
        p_val2_ptr = &locals.dyn[tmp2];

        tmp3 = *p_val1_ptr;
        *p_val1_ptr = *p_val2_ptr;
        *p_val2_ptr = tmp3;
    }
}
//------------------------------------------------------------------------------------------------//
U16 StdEncryptDecodeData(U16 data)
{
    U16  tmp;
    U16* val1_ptr;
    U16* val2_ptr;
    U16  data_decoded;

    locals.counter_i = (locals.counter_i + locals.algo_const_1) & DYNAMIC_KEY_MASK;
    val1_ptr = &locals.dyn[locals.counter_i];

    locals.counter_j = (locals.counter_j + *val1_ptr) & DYNAMIC_KEY_MASK;
    val2_ptr = &locals.dyn[locals.counter_j];

    tmp = *val1_ptr;
    *val1_ptr = *val2_ptr;
    *val2_ptr = tmp;

    data_decoded = data ^ locals.dyn[(*val1_ptr + *val2_ptr) & DYNAMIC_KEY_MASK];
    locals.counter_i = (locals.counter_i + data_decoded) & DYNAMIC_KEY_MASK;
    locals.counter_j = (locals.counter_j + data_decoded + locals.algo_const_2) & DYNAMIC_KEY_MASK;
    return data_decoded;
}

//================================================================================================//
