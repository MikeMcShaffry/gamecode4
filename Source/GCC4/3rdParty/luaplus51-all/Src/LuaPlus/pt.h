/*======================================================================*\
* This module creates a compatibility layer between Win32 and pthreads.
* DISCLAMER:
*   This is NOT a full pthread implementation for Win32. Only the
*   functionality needed by luathreads was implemented. Use it at your 
*   own risk.
* Diego Nehab, 12/3/2001
* RCS Id: $Id: pt.h,v 1.6 2002/05/17 18:56:27 diego Exp $
\*======================================================================*/
#ifndef _PT
#define _PT

#ifdef LUA_MTSUPPORT

/*======================================================================*\
* Win32 stuff.
\*======================================================================*/
#ifdef WIN32

#include <stddef.h>
/*----------------------------------------------------------------------*\
* Exported data structures are completely opaque.
* To determine the size the structures, use the given sizeof functions.
\*----------------------------------------------------------------------*/
typedef void pthread_mutex_t;
typedef void pthread_mutexattr_t;
size_t pthread_mutex_sizeof(void);

typedef void pthread_cond_t;
typedef void pthread_condattr_t;
size_t pthread_cond_sizeof(void);

typedef unsigned long pthread_t;
typedef void pthread_attr_t;

typedef void *(pthread_start_t)(void *arg);

/*----------------------------------------------------------------------*\
* Implemented API functions.
\*----------------------------------------------------------------------*/
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_create(pthread_t *id, pthread_attr_t *attr, 
        pthread_start_t *start, void *arg);
pthread_t pthread_self(void);
int pthread_equal(pthread_t t1, pthread_t t2);

/*----------------------------------------------------------------------*\
* They are just present to avoid compile errors and memory leaks
* in luathreads under Win32.
\*----------------------------------------------------------------------*/
int pthread_detach(pthread_t th);
int pthread_cleanup(pthread_t th);

/*======================================================================*\
* Pthreads stuff.
\*======================================================================*/
#else 

struct timespec;

#include <pthread.h>

#define pthread_mutex_sizeof() (sizeof(pthread_mutex_t))
#define pthread_cond_sizeof() (sizeof(pthread_cond_t))
int pthread_cleanup(pthread_t th);

#endif /* WIN32 */

#endif /* LUA_MTSUPPORT */

#endif
