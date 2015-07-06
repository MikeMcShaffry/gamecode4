/*======================================================================*\
* This module creates a compatibility layer between Win32 and pthreads.
* DISCLAMER:
*   This is NOT a full pthread implementation for Win32. Only the
*   functionality needed by LuaThread was implemented. Use it at your 
*   own risk.
* Diego Nehab, 12/3/2001
* RCS Id: $Id: pt.c,v 1.7 2004/11/24 19:50:02 diego Exp $
\*======================================================================*/
#include "pt.h"
#include <stdio.h>
/*======================================================================*\
* Win32 stuff. There is nothing to do unless we are under Win32.
\*======================================================================*/
#ifdef WIN32

#include <windows.h>
#include <process.h>

/*----------------------------------------------------------------------*\
* Exported data structures are completely opaque. Internaly, we define 
* their real contents.
\*----------------------------------------------------------------------*/
typedef CRITICAL_SECTION _pthread_mutex_t;

size_t pthread_mutex_sizeof(void)
{
	return sizeof(_pthread_mutex_t);
}

typedef struct {
	HANDLE semaphore;
	int waiting;
} _pthread_cond_t;

size_t pthread_cond_sizeof(void)
{
	return sizeof(_pthread_cond_t);
}

typedef void (_pthread_start_t)(void *);

/*----------------------------------------------------------------------*\
* Mutex stuff.
\*----------------------------------------------------------------------*/
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
	(void) attr;
	InitializeCriticalSection((_pthread_mutex_t *) mutex);
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	EnterCriticalSection((_pthread_mutex_t *) mutex);
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	LeaveCriticalSection((_pthread_mutex_t *) mutex);
	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	DeleteCriticalSection((_pthread_mutex_t *) mutex);
	return 0;
}

/*----------------------------------------------------------------------*\
* Conditions stuff.
\*----------------------------------------------------------------------*/
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr)
{
    _pthread_cond_t *_cond = (_pthread_cond_t *) cond;
	_cond->semaphore = CreateSemaphore(NULL, 0, 128, NULL);
	_cond->waiting = 0;
	return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    _pthread_cond_t *_cond = (_pthread_cond_t *) cond;
	CloseHandle(_cond->semaphore);
	return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    _pthread_cond_t *_cond = (_pthread_cond_t *) cond;
	_cond->waiting++;
	pthread_mutex_unlock(mutex);
	WaitForSingleObject(_cond->semaphore, INFINITE);
	pthread_mutex_lock(mutex);
	return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    _pthread_cond_t *_cond = (_pthread_cond_t *) cond;
	if (_cond->waiting > 0) {
        if (ReleaseSemaphore(_cond->semaphore, 1, NULL) == 0) return -1;
		_cond->waiting--;
	}
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    _pthread_cond_t *_cond = (_pthread_cond_t *) cond;
	if (_cond->waiting > 0) {
		if (ReleaseSemaphore(_cond->semaphore, _cond->waiting, NULL) == 0) 
			return -1;
		_cond->waiting = 0;
	}
    return 0;
}

/*----------------------------------------------------------------------*\
* Threads stuff.
\*----------------------------------------------------------------------*/
int pthread_create(pthread_t *id, pthread_attr_t *attr, 
    pthread_start_t *start, void *arg)
{
	pthread_t tid = _beginthread((_pthread_start_t *) start, 0, arg);
    if (id) *id = tid;
    if (tid == -1) return -1;
	else return 0;
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
    return (t1 == t2);
}

pthread_t pthread_self(void)
{
	return (pthread_t) GetCurrentThreadId();
}

int pthread_detach(pthread_t th)
{
	(void) th;
	return 0;
}

int pthread_cleanup(pthread_t th)
{
    _endthread();
    return 0;
}

#else

int pthread_cleanup(pthread_t th)
{
    return 0;
}

#endif
