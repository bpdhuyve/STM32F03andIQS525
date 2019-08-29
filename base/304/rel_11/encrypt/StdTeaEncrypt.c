//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This is the source file of the TEA (Tiny Encryption Algorithm) encryption entity.
// This Encryption entity is designed to be used for encrypt the data bytes that should be programmed in the DSP.\n
// So it's unpossible to copy the code by others.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define ENCRYPT__STDTEAENCRYPT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDTEAENCRYPT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STDTEAENCRYPT_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "encrypt\StdTeaEncrypt.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define add_carry(x,y)  { U16 total = (x + y); carry = (total > 0xFF); x = (U8) (total % 0x100); }
#define add_carry2(x,y) { U16 total = (x + y + carry); carry = (total > 0xFF); x = (U8) (total % 0x100); }

#define sub_carry(x,y)  { U16 total = (x - y); carry = (total > 0xFF); x = (U8) (total % 0x100); }
#define sub_carry2(x,y) { U16 total = (x - y - carry); carry = (total > 0xFF); x = (U8) (total % 0x100); }

#define shift_carry(x)  { carry = 0; if( x & 0x01 ) carry = 1; x >>= 1; }
#define shift_carry2(x) { U8 tmp_carry = 0; if(x & 0x01) tmp_carry = 1; x >>= 1;  x |= (carry << 7); carry = tmp_carry; }

#define swap_nibbles(x) { U16 ext = (x << 4); x <<=4 ; x += (ext >> 8); }
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef	struct
{
	U8 byte0;
	U8 byte1;
	U8 byte2;
	U8 byte3;
}
COMPOSED_VAR;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void YK23SUB(void);
static void ZK01SUB(void);
static void StdTeaEncrypt8Bytes(U8* ptr, BOOL encode);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static COMPOSED_VAR     sum;
static COMPOSED_VAR     L;
static COMPOSED_VAR     R;
static COMPOSED_VAR     temp_y;
static COMPOSED_VAR     temp_z;
static U8               carry;
static U8*              tea_encrypt_key_ptr;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void YK23SUB(void)
{
	U8 acc = 0;

	acc = temp_y.byte0;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte0 = acc;
	acc = temp_y.byte1;
	swap_nibbles( acc );
	acc &= 0x0F;
	acc |= L.byte0;
	L.byte0 = acc;

	acc = temp_y.byte1;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte1 = acc;
	acc = temp_y.byte2;
	swap_nibbles( acc );
	acc &= 0x0F;
	acc |= L.byte1;
	L.byte1 = acc;

	acc = temp_y.byte2;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte2 = acc;
	acc = temp_y.byte3;
	swap_nibbles( acc );
	acc &= 0x0F;
	acc |= L.byte2;
	L.byte2 = acc;

	acc = temp_y.byte3;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte3 = acc;


	acc = temp_y.byte0;
	swap_nibbles(acc);
	shift_carry(acc);
	acc &= 0x0F;
	R.byte0 = acc;

	acc = L.byte0;
	shift_carry2(acc);
	R.byte1 = acc;

	acc = L.byte1;
	shift_carry2(acc);
	R.byte2 = acc;

	acc = L.byte2;
	shift_carry2(acc);
	R.byte3 = acc;

	add_carry( L.byte3, tea_encrypt_key_ptr[11] );
	add_carry2( L.byte2, tea_encrypt_key_ptr[9] );
	add_carry2( L.byte1, tea_encrypt_key_ptr[8] );
	add_carry2( L.byte0, tea_encrypt_key_ptr[7] );

	add_carry( R.byte3, tea_encrypt_key_ptr[15] );
	add_carry2( R.byte2, tea_encrypt_key_ptr[14] );
	add_carry2( R.byte1, tea_encrypt_key_ptr[13] );
	add_carry2( R.byte0, tea_encrypt_key_ptr[12] );

	acc = temp_y.byte3;
	add_carry( acc, sum.byte3);
	acc ^= R.byte3;
	acc ^= L.byte3;
	L.byte3 = acc;

	acc = temp_y.byte2;
	add_carry2( acc, sum.byte2);
	acc ^= R.byte2;
	acc ^= L.byte2;
	L.byte2 = acc;

	acc = temp_y.byte1;
	add_carry2( acc, sum.byte1);
	acc ^= R.byte1;
	acc ^= L.byte1;
	L.byte1 = acc;


	acc = temp_y.byte0;
	add_carry2( acc, sum.byte0);
	acc ^= R.byte0;
	acc ^= L.byte0;
	L.byte0 = acc;
}
//------------------------------------------------------------------------------------------------//
static void ZK01SUB(void)
{
	U8 acc = 0;

	acc = temp_z.byte0;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte0 = acc;
	acc = temp_z.byte1;
	swap_nibbles( acc );
	acc &= 0x0F;
	acc |= L.byte0;
	L.byte0 = acc;

	acc = temp_z.byte1;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte1 = acc;
	acc = temp_z.byte2;
	swap_nibbles( acc );
	acc &= 0x0F;
	acc |= L.byte1;
	L.byte1 = acc;

	acc = temp_z.byte2;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte2 = acc;
	acc = temp_z.byte3;
	swap_nibbles( acc );
	acc &= 0x0F;
	acc |= L.byte2;
	L.byte2 = acc;

	acc = temp_z.byte3;
	swap_nibbles( acc );
	acc &= 0xF0;
	L.byte3 = acc;


	acc = temp_z.byte0;
	swap_nibbles(acc);
	shift_carry(acc);
	acc &= 0x0F;
	R.byte0 = acc;

	acc = L.byte0;
	shift_carry2(acc);
	R.byte1 = acc;

	acc = L.byte1;
	shift_carry2(acc);
	R.byte2 = acc;

	acc = L.byte2;
	shift_carry2(acc);
	R.byte3 = acc;


	add_carry( L.byte3, tea_encrypt_key_ptr[3] );
	add_carry2( L.byte2, tea_encrypt_key_ptr[2] );
	add_carry2( L.byte1, tea_encrypt_key_ptr[1] );
	add_carry2( L.byte0, tea_encrypt_key_ptr[0] );

	add_carry( R.byte3, tea_encrypt_key_ptr[7] );
	add_carry2( R.byte2, tea_encrypt_key_ptr[6] );
	add_carry2( R.byte1, tea_encrypt_key_ptr[5] );
	add_carry2( R.byte0, tea_encrypt_key_ptr[4] );

	acc = temp_z.byte3;
	add_carry( acc, sum.byte3);
	acc ^= R.byte3;
	acc ^= L.byte3;
	L.byte3 = acc;

	acc = temp_z.byte2;
	add_carry2( acc, sum.byte2);
	acc ^= R.byte2;
	acc ^= L.byte2;
	L.byte2 = acc;


	acc = temp_z.byte1;
	add_carry2( acc, sum.byte1);
	acc ^= R.byte1;
	acc ^= L.byte1;
	L.byte1 = acc;


	acc = temp_z.byte0;
	add_carry2( acc, sum.byte0);
	acc ^= R.byte0;
	acc ^= L.byte0;
	L.byte0 = acc;
}
//------------------------------------------------------------------------------------------------//
static void StdTeaEncrypt8Bytes(U8* ptr, BOOL encode)
{
	U8 cycles = 32;

	if(!encode)     //decode
	{
		sum.byte0 = 0x00C6;
		sum.byte1 = 0x00EF;
		sum.byte2 = 0x0037;
		sum.byte3 = 0x0020;
	}
	else            //encode
	{
		sum.byte0 = 0;
		sum.byte1 = 0;
		sum.byte2 = 0;
		sum.byte3 = 0;
	}

	temp_y.byte0 = ptr[0];
	temp_y.byte1 = ptr[1];
	temp_y.byte2 = ptr[2];
	temp_y.byte3 = ptr[3];
	temp_z.byte0 = ptr[4];
	temp_z.byte1 = ptr[5];
	temp_z.byte2 = ptr[6];
	temp_z.byte3 = ptr[7];

	while(cycles-- > 0)
	{
		if(!encode)
		{
			YK23SUB();

			sub_carry(temp_z.byte3, L.byte3);
			sub_carry2(temp_z.byte2, L.byte2);
			sub_carry2(temp_z.byte1, L.byte1);
			sub_carry2(temp_z.byte0, L.byte0);

			ZK01SUB();

			sub_carry(temp_y.byte3, L.byte3);
			sub_carry2(temp_y.byte2, L.byte2);
			sub_carry2(temp_y.byte1, L.byte1);
			sub_carry2(temp_y.byte0, L.byte0);

			sub_carry(sum.byte3, 0xB9);
			sub_carry2(sum.byte2, 0x79);
			sub_carry2(sum.byte1, 0x37);
			sub_carry2(sum.byte0, 0x9E);
		}
		else
		{
			add_carry(sum.byte3, 0xB9);
			add_carry2(sum.byte2, 0x79);
			add_carry2(sum.byte1, 0x37);
			add_carry2(sum.byte0, 0x9E);

			ZK01SUB();

			add_carry(temp_y.byte3, L.byte3);
			add_carry2(temp_y.byte2, L.byte2);
			add_carry2(temp_y.byte1, L.byte1);
			add_carry2(temp_y.byte0, L.byte0);

			YK23SUB();

			add_carry(temp_z.byte3, L.byte3);
			add_carry2(temp_z.byte2, L.byte2);
			add_carry2(temp_z.byte1, L.byte1);
			add_carry2(temp_z.byte0, L.byte0);
		}
	}

	ptr[0] = temp_y.byte0;
	ptr[1] = temp_y.byte1;
	ptr[2] = temp_y.byte2;
	ptr[3] = temp_y.byte3;
	ptr[4] = temp_z.byte0;
	ptr[5] = temp_z.byte1;
	ptr[6] = temp_z.byte2;
	ptr[7] = temp_z.byte3;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
BOOL StdTeaEncryptByteBuffer(U8* key,
                             U8* buffer,
                             U16 buffer_byte_length,
                             BOOL encode)
{
    U16 i;
    U16 cnt;

    if(buffer_byte_length & 0x0007)
    {
        //buffer_byte_length must be a multiple of 8
        return FALSE;
    }
	tea_encrypt_key_ptr = key;
    cnt = buffer_byte_length >> 3;

    for(i = 0; i < cnt; i++)
    {
        StdTeaEncrypt8Bytes(buffer + (i * 8), encode);
    }
    return TRUE;
}
//================================================================================================//
