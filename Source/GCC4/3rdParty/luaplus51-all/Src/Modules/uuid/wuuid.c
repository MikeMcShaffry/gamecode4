/*
 * wuuid.c
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
#include "wuuid.h"
#include <rpc.h>


/*
 * Note that no attempt is made here to really implement uuid_time()
 * because it really isn't that useful given the general recommendation
 * to only use random UUIDs that better protect the privacy of the
 * issuer. 
 */
time_t uuid_time(const uuid_t uu, struct timeval *ret_tv)
{
    return (time_t)-1;
}


/* convert a uuid_t to a string in cannonical form */
void uuid_unparse(const uuid_t u, char *s)
{
    unsigned char __RPC_FAR *rs;
    if (RPC_S_OK == UuidToStringA((UUID *)u,&rs)) {
	strncpy(s,(char __RPC_FAR *)rs,2*sizeof(UUID)+4+1);
	RpcStringFree(&rs);
    } else {
	s[0] = '\0';
    }
}

/* convert a string in cannonical form to a uuid_t, returning nonzero
 * on error.
 */
int uuid_parse(const char *s, uuid_t u)
{
    return RPC_S_OK != UuidFromStringA((unsigned char *)s, u);
}

/* generate a UUID, using a suitable random number source. */
void uuid_generate_random(uuid_t out)
{
    UuidCreate(out);
}

/* generate a UUID based on a time stamp and MAC address. */
void uuid_generate_time(uuid_t out)
{
    UuidCreateSequential(out);
}

/* generate a UUID the default way. */
void uuid_generate(uuid_t out)
{
    UuidCreate(out);
}
