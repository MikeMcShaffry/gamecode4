/*
 * wuuid.h
 *
 * Emulation of libuuid.so API functions based on the native Windows
 * API. Only those functions actually used by luuid are provided.
 * Extending this work to a complete emulation of libuuid would be
 * possible, but of unknown value.
 *
 * Implemented by Ross Berteig <Ross@cheshireeng.com>
 * Released 2007-09-24.
 * This code is hereby placed in the public domain.
 */
#ifndef __WUUID_H__
#define __WUUID_H__

#ifndef UUID_DEFINED
#define _WIN32_WINNT 0x500		 /* include Win2K API for UuidCreateSequential() */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <rpc.h>
#endif
#include <time.h>

/*
 * Fix declaration of uuid_t (possibly made in <basetypes.h> included
 * from <windows.h> to by an array type. The declaration in
 * <basetypes.h> is as a synonym for UUID.  Real libuuid.so uses an
 * array of 16 unsigned chars. It is more consistent with Win32 to use
 * an array of 1 UUID.
 */
#ifdef uuid_t
#undef uuid_t
#endif
typedef UUID uuid_t[1];

/* Return the time stamp from a UUID. Note that uuid_time is implemented
 * because it is used by luuid, but returns the constant ~0.
 */
time_t uuid_time(const uuid_t uu, struct timeval *ret_tv);

/* Convert a uuid_t to a string in cannonical form */
void uuid_unparse(const uuid_t uu, char *out);

/* Convert a string in cannonical form to a uuid_t, returning nonzero
 * on error.
 */
int uuid_parse(const char *in, uuid_t uu);

/* Generate a UUID, using a suitable random number source. */
void uuid_generate_random(uuid_t out);

/* Generate a UUID based on a time stamp and MAC address. */
void uuid_generate_time(uuid_t out);

/* Generate a UUID the default way. */
void uuid_generate(uuid_t out);

#endif /*__WUUID_H__*/
