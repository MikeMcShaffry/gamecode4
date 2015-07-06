/*
* lmd5.h
* MD5 library for Lua 5.0 based on Rivest's API
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 26 Mar 2004 17:49:20
* This code is hereby placed in the public domain.
*/

#ifdef USE_SHA1_OPENSSL
#define USING_SHA1
#define AUTHOR			"openssl"
#define MD5_CTX			SHA_CTX
#define MD5Init			SHA1_Init
#define MD5Update		SHA1_Update
#define MD5Final		SHA1_Final
#include <openssl/sha.h>
#endif

#ifdef USE_SHA1_RFC
#define USING_SHA1
#define AUTHOR			"rfc3174"
#define MD5_CTX			SHA1Context
#define MD5Init			SHA1Reset
#define MD5Update		SHA1Input
#define MD5Final(digest,c)	SHA1Result(c,digest)
#include "sha1.h"
#endif

#ifdef USE_SHA1_SKALIBS
#define USING_SHA1
#define AUTHOR			"skalibs"
#define MD5_CTX			SHA1Schedule
#define MD5Init			sha1_init
#define MD5Update		sha1_update
#define MD5Final(digest,c)	sha1_final(c,digest)
#include "sha1.h"
#endif

#ifdef USE_SHA1_GLADMAN
#define USING_SHA1
#define AUTHOR			"gladman"
#define MD5_CTX			sha1_ctx
#define MD5Init			sha1_begin
#define MD5Update(c,s,l)	sha1_hash(s,l,c)
#define MD5Final		sha1_end
#include "sha1.h"
#endif

#ifdef USE_MD5_OPENSSL
#define AUTHOR			"openssl"
#define MD5CTX 			MD5_CTX
#define MD5Init			MD5_Init
#define MD5Update		MD5_Update
#define MD5Final		MD5_Final
#include <openssl/md5.h>
#endif

#ifdef USE_MD5_DEUTSCH
#define AUTHOR			"Deutsch"
#define MD5_CTX			md5_state_t
#define MD5Init(c)		md5_init(c)
#define MD5Update(c,s,n)	md5_append(c,(const md5_byte_t *)s,n)
#define MD5Final(digest,c)	md5_finish(c,digest)
#include "md5.h"
#endif

#ifdef USE_MD5_PLUMB
#define AUTHOR			"Plumb"
#include "md5.h"
#endif

#ifndef AUTHOR
#define USE_MD5_RIVEST
#endif

#ifdef USE_MD5_RIVEST
#define AUTHOR			"Rivest"
#include "md5global.h"
#include "md5.h"
#endif

#ifdef USING_SHA1
#define MYNAME			"sha1"
#define N			20
#define luaopen_md5		luaopen_sha1
#else
#define MYNAME			"md5"
#define N			16
#endif
