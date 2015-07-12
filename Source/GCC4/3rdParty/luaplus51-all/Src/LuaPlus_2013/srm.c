/*======================================================================*\
* Safe Recursive mutex implementation.
* Diego Nehab, 12/3/2001
* RCS Id: $Id: srm.c,v 1.2 2001/12/14 21:55:13 diego Exp $
\*======================================================================*/

#include "lua.h"

#ifdef LUA_MTSUPPORT

#include <stdlib.h>

#include "pt.h"
#include "srm.h"

int srm_init(srm_t *srm)
{
    srm->locks = 0;
    srm->mutex = (pthread_mutex_t *) malloc(pthread_mutex_sizeof());
    srm->barrier = (pthread_mutex_t *) malloc(pthread_mutex_sizeof());
    if (!srm->mutex || !srm->barrier) goto failure;
    if (pthread_mutex_init(srm->mutex, NULL) == 0) {
        if (pthread_mutex_init(srm->barrier, NULL) == 0) return 0;
        else pthread_mutex_destroy(srm->mutex);
    } 
failure: /* sorry about the goto */
    free(srm->mutex);
    free(srm->barrier);
    srm->barrier = NULL;
    srm->mutex = NULL;
    return -1;
}

int srm_destroy(srm_t *srm)
{
    pthread_mutex_destroy(srm->mutex);
    pthread_mutex_destroy(srm->barrier);
    free(srm->mutex);
    free(srm->barrier);
    srm->mutex = NULL;
    srm->barrier = NULL;
    return 0;
}

int srm_lock(srm_t *srm)
{
    pthread_t self = pthread_self();
    pthread_mutex_lock(srm->barrier);
    if (srm->locks <= 0 || !pthread_equal(self, srm->owner)) {
        pthread_mutex_unlock(srm->barrier);
        pthread_mutex_lock(srm->mutex);
        pthread_mutex_lock(srm->barrier);
        srm->owner = self;
        srm->locks = 1;
    } else srm->locks++;
    pthread_mutex_unlock(srm->barrier);
    return 0;
}

int srm_unlock(srm_t *srm)
{
    pthread_t self = pthread_self();
    pthread_mutex_lock(srm->barrier);
    if (srm->locks > 0 && pthread_equal(self, srm->owner)) {
        srm->locks--;
        if (srm->locks <= 0) pthread_mutex_unlock(srm->mutex);
        pthread_mutex_unlock(srm->barrier);
        return 0;
    } else {
        pthread_mutex_unlock(srm->barrier);
        return -1;
    }
}

int srm_cond_wait(pthread_cond_t *cond, srm_t *srm)
{
    pthread_t owner;
    int locks;
    int ret;
    pthread_mutex_lock(srm->barrier);
    locks = srm->locks;
    owner = srm->owner;
    srm->locks = 0;
    pthread_mutex_unlock(srm->barrier);
    ret = pthread_cond_wait(cond, srm->mutex);
    pthread_mutex_lock(srm->barrier);
    srm->owner = owner;
    srm->locks = locks;
    pthread_mutex_unlock(srm->barrier);
    return ret;
}

#endif /* LUA_MTSUPPORT */
