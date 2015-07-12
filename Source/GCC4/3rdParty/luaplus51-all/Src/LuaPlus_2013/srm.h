/*======================================================================*\
* Safe Recursive mutex implementation.
* Diego Nehab, 12/3/2001
* RCS Id: $Id: srm.h,v 1.2 2001/12/14 21:55:13 diego Exp $
\*======================================================================*/

#ifndef _SRM_H
#define _SRM_H

#ifdef LUA_MTSUPPORT

#include "pt.h"

/* safe, recursive mutex structure */
typedef struct srm_t {
    pthread_mutex_t *mutex;
    pthread_mutex_t *barrier; 
    pthread_t owner;
    int locks; 
} srm_t;

int srm_init(srm_t *srm);
int srm_destroy(srm_t *srm);
int srm_lock(srm_t *srm);
int srm_unlock(srm_t *srm);
int srm_cond_wait(pthread_cond_t *cond, srm_t *srm);

#endif /* LUA_MTSUPPORT */

#endif
