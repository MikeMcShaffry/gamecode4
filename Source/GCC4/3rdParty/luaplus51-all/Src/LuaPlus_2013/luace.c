/**
 * LuaCE
 *
 *      Pedro Miller Rabinovitch, <miller@inf.puc-rio.br>
 *      http://www.inf.puc-rio.br/~miller/luace
 *
 * A quick port of Lua for Windows CE.
 **/

#ifdef _WIN32

#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include "luace.h"

#include <winbase.h>


const char *getenv( const char *name ) {
	return NULL;
}


char *strdup( const char *str ) {
	char *ptr = (char *)malloc( sizeof(char) * strlen( str ) + 1 );
	strcpy( ptr, str );
	return ptr;
}

/*
double strtod( const char *nptr, char const * *endptr ) {
	int adv = 0;
	double val = 0;

	sscanf( nptr, " %lf%n", &val, &adv );
	if( endptr )
		*endptr = nptr + adv;

	return val;
}

unsigned long strtoul( const char *nptr, char const **endptr, int base ) {
	int adv = 0;
	unsigned long  val = 0;

	if( base == 16 )
		sscanf( nptr, " %lx%n", &val, &adv );
	else
		sscanf( nptr, " %lu%n", &val, &adv );

	if( endptr )
		*endptr = nptr + adv;

	return val;
}
*/

/*
char *strpbrk( const char *string, const char *strCharSet ) {
	char flags[256/8];

	memset( flags, 0, sizeof( flags ));
	for( ; *strCharSet != '\0'; strCharSet++ ) {
		flags[ *strCharSet/8 ] |= 1 << (*strCharSet % 8);
	}

	for( ; *string != '\0'; string++ ) {
		if( flags[ *string/8 ] &  (1 << (*string % 8)))
			return (char *)string;
	}

	return NULL;
}
*/
/* unconforming: this file should be removed at closing time */
FILE *tmpfile() {
	char buf[MAX_PATH];
	tmpnam( buf );
	return fopen( buf, "w+b" );
}

/* unconforming: should use static buffer in case of NULL param */
char *tmpnam( char *str ) {
/*
	if( str == NULL ) {
		char buf[MAX_PATH];
		GetTempFileName( ".", pref, 1, buf );
		return strdup( buf );
	} else {
		GetTempFileName( ".", pref, 1, str );
		return str;
	}
*/
	return NULL;
}

int system( const char *cmd ) {
	return 0;
}

/* alloc's and returns a converted wide-char string from a normal one */
wchar_t *atowcs( const char *str ) {
	int len;
	wchar_t *wstr;
	if( str == NULL )
		return NULL;
	len = strlen( str );
	wstr = (wchar_t *)malloc( sizeof( wchar_t )*2*(len + 1) );
	wsprintf( wstr, L"%hs", str );
	return wstr;
}

int rename( const char *src, const char *dst ) {
	wchar_t *wsrc = atowcs( src ),
		    *wdst = atowcs( dst );
	int res = 0;
	
	if( !MoveFile( wsrc, wdst ))
		res = GetLastError();

	free( wsrc );
	free( wdst );

	return res;
}

int remove( const char *fname ) {
	wchar_t *wfname = atowcs( fname );
	int res = 0;

	if( !DeleteFile( wfname ))
		res = GetLastError();

	free( wfname );

	return 0;
}

struct tm *tm_struct_from_systime( SYSTEMTIME *systime ) {
	static struct tm gtm;

	gtm.tm_hour = systime->wHour;
	gtm.tm_isdst = 0;
	gtm.tm_mday = systime->wDay;
	gtm.tm_min = systime->wMinute;
	gtm.tm_mon = systime->wMonth;
	gtm.tm_sec = systime->wSecond;
	gtm.tm_wday = systime->wDayOfWeek;
	gtm.tm_yday = 0; /* unsupported for now */
	gtm.tm_year = systime->wYear;

	return &gtm;
}

struct tm *gmtime( const time_t *timer ) /* returns a pointer to a structure of type tm. The fields of the returned structure hold the evaluated value of the timer argument in UTC rather than in local time. Each of the structure fields is of type int, as follows: */
{
	SYSTEMTIME systime;
	GetSystemTime( &systime );

	return  tm_struct_from_systime( &systime );
}

struct tm *localtime( const time_t *timer ) /* same, but local */
{
	SYSTEMTIME systime;
	GetLocalTime( &systime );

	return  tm_struct_from_systime( &systime );
}

clock_t clock( void ) { /* elapsed ticks. n/a, will use GetTickCount */
	return GetTickCount();
}

size_t strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr ) {
	TCHAR myfmt[BUFSIZ], mybuf[BUFSIZ];
	size_t ret;

	wcscpy( myfmt, L"'teste: 'hh':'mm':'ss tt" );
	ret = GetTimeFormat( LOCALE_SYSTEM_DEFAULT, 0, NULL, myfmt, mybuf, sizeof(mybuf) );
	if( ret )
		sprintf( strDest, "%ls", mybuf );
	
	return ret;
}

time_t mktime( struct tm *ptm ) {
	/* for now, ignore leap years and month variations blatantly */
	/* also: don't normalize (Lua doesn't need it) */
	time_t t = ptm->tm_year;
	t = 12 * t + ptm->tm_mon;
	t = 30 * t + ptm->tm_mday; /* gaah! */
	t = 24 * t + ptm->tm_hour;
	t = 60 * t + ptm->tm_min;
	t = 60 * t + ptm->tm_sec;

	return t;
}

time_t time( time_t *timer ) {
	SYSTEMTIME systime;
	GetSystemTime( &systime );

	return mktime( tm_struct_from_systime( &systime ));
}

char *setlocale( int category, const char *locale ) {
	return NULL;
}

/*
void *calloc( size_t num, size_t size ) {
	void *ptr = malloc( num*size );
	if( ptr )
		memset( ptr, 0, num*size );
	return ptr;
}

*/

int errno = 0;

#endif
