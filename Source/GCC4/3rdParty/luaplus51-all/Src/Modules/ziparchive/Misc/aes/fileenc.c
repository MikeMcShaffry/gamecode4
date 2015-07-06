/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman <                 >, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 -------------------------------------------------------------------------
 Issue Date: 26/08/2003

 This file implements password based file encryption and authentication
 using AES in CTR mode, HMAC-SHA1 authentication and RFC2898 password
 based key derivation.

*/

#include <memory.h>

#include "fileenc.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/* subroutine for data encryption/decryption    */
/* this could be speeded up a lot by aligning   */
/* buffers and using 32 bit operations          */

static void encr_data(unsigned char data[], unsigned long d_len, fcrypt_ctx cx[1])
{   unsigned long i = 0, pos = cx->encr_pos;

    while(i < d_len)
    {
        if(pos == BLOCK_SIZE)
        {   unsigned int j = 0;
            /* increment encryption nonce   */
            while(j < 8 && !++cx->nonce[j])
                ++j;
            /* encrypt the nonce to form next xor buffer    */
            aes_encrypt(cx->nonce, cx->encr_bfr, cx->encr_ctx);
            pos = 0;
        }

        data[i++] ^= cx->encr_bfr[pos++];
    }

    cx->encr_pos = pos;
}


static void SwapEndian128(unsigned char* src, unsigned char* dest)
{
	dest[0] = src[7];
	dest[1] = src[6];
	dest[2] = src[5];
	dest[3] = src[4];
	dest[4] = src[3];
	dest[5] = src[2];
	dest[6] = src[1];
	dest[7] = src[0];

	dest[8] = src[15];
	dest[9] = src[14];
	dest[10] = src[13];
	dest[11] = src[12];
	dest[12] = src[11];
	dest[13] = src[10];
	dest[14] = src[9];
	dest[15] = src[8];
}


static void encr_data_offset(unsigned char data[], unsigned long d_len, fcrypt_ctx cx[1], unsigned long offset)
{
	unsigned long i = 0;
	unsigned long pos = offset % BLOCK_SIZE;  //cx->encr_pos;
	unsigned char flipnonce[BLOCK_SIZE];
	int x = 1;

#if _MSC_VER  &&  _MSC_VER <= 1300
	*(unsigned __int64*)cx->nonce = 1 + (offset / BLOCK_SIZE);
#else
	*(unsigned long long*)cx->nonce = 1 + (offset / BLOCK_SIZE);
#endif

	/* endian check */
	if (*(unsigned char*)&x == 0)
	{
		memcpy(flipnonce, cx->nonce, sizeof(cx->nonce));
		SwapEndian128(flipnonce, cx->nonce);
	}

	aes_encrypt(cx->nonce, cx->encr_bfr, cx->encr_ctx);

    while(i < d_len)
    {
        if(pos == BLOCK_SIZE)
        {   unsigned int j = 0;
            /* increment encryption nonce   */
            while(j < 8 && !++cx->nonce[j])
                ++j;
            /* encrypt the nonce to form next xor buffer    */
            aes_encrypt(cx->nonce, cx->encr_bfr, cx->encr_ctx);
            pos = 0;
        }

        data[i++] ^= cx->encr_bfr[pos++];
    }

    cx->encr_pos = pos;
}


int fcrypt_init(
    int mode,                               /* the mode to be used (input)          */
    const unsigned char* pwd,              /* the user specified password (input)  */
    unsigned int pwd_len,                   /* the length of the password (input)   */
    const unsigned char* salt,             /* the salt (input)                     */
#ifdef PASSWORD_VERIFIER
    unsigned char pwd_ver[PWD_VER_LENGTH],  /* 2 byte password verifier (output)    */
#endif
    fcrypt_ctx      cx[1])                  /* the file encryption context (output) */
{   unsigned char kbuf[2 * MAX_KEY_LENGTH + PWD_VER_LENGTH];

    if(pwd_len > MAX_PWD_LENGTH)
        return PASSWORD_TOO_LONG;

    if(mode < 1 || mode > 3)
        return BAD_MODE;

    cx->mode = mode;
    cx->pwd_len = pwd_len;
    /* initialise the encryption nonce and buffer pos   */
    cx->encr_pos = BLOCK_SIZE;

	/* if we need a random component in the encryption  */
    /* nonce, this is where it would have to be set     */
    memset(cx->nonce, 0, BLOCK_SIZE * sizeof(unsigned char));
	/* initialise for authentication			        */
//jj    hmac_sha_begin(cx->auth_ctx);

	/* derive the encryption and authetication keys and the password verifier   */
    derive_key(pwd, pwd_len, salt, SALT_LENGTH(mode), KEYING_ITERATIONS,
                        kbuf, 2 * KEY_LENGTH(mode) + PWD_VER_LENGTH);
    /* set the encryption key							*/
    aes_encrypt_key(kbuf, KEY_LENGTH(mode), cx->encr_ctx);
    /* set the authentication key						*/
//jj    hmac_sha_key(kbuf + KEY_LENGTH(mode), KEY_LENGTH(mode), cx->auth_ctx);
#ifdef PASSWORD_VERIFIER
    memcpy(pwd_ver, kbuf + 2 * KEY_LENGTH(mode), PWD_VER_LENGTH);
#endif
	/* clear the buffer holding the derived key values	*/
	memset(kbuf, 0, 2 * KEY_LENGTH(mode) + PWD_VER_LENGTH);

	return GOOD_RETURN;
}

/* perform 'in place' encryption and authentication */

void fcrypt_encrypt(unsigned char* data, unsigned int data_len, fcrypt_ctx cx[1])
{
	encr_data(data, data_len, cx);
	//jj    hmac_sha_data(data, data_len, cx->auth_ctx);
}

void fcrypt_encrypt_offset(unsigned char* data, unsigned int data_len, fcrypt_ctx cx[1], unsigned long offset)
{
	encr_data_offset(data, data_len, cx, offset);
	//jj    hmac_sha_data(data, data_len, cx->auth_ctx);
}

/* perform 'in place' authentication and decryption */

void fcrypt_decrypt(unsigned char* data, unsigned int data_len, fcrypt_ctx cx[1])
{
	//jj    hmac_sha_data(data, data_len, cx->auth_ctx);
	encr_data(data, data_len, cx);
}

void fcrypt_decrypt_offset(unsigned char* data, unsigned int data_len, fcrypt_ctx cx[1], unsigned long offset)
{
	//jj    hmac_sha_data(data, data_len, cx->auth_ctx);
	encr_data_offset(data, data_len, cx, offset);
}

/* close encryption/decryption and return the MAC value */

int fcrypt_end(unsigned char mac[], fcrypt_ctx cx[1])
{	//unsigned int res = cx->mode;

//jj    hmac_sha_end(mac, MAC_LENGTH(cx->mode), cx->auth_ctx);
	memset(cx, 0, sizeof(fcrypt_ctx));	/* clear the encryption context	*/
	return MAC_LENGTH(res);				/* return MAC length in bytes   */
}

#if defined(__cplusplus)
}
#endif
