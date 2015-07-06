#undef LoadString

/* these are terrible, but just for CE quick-dirty (pedro) */
#define strcoll strcmp
/*#ifndef isalpha
#define isalpha(c)			( ('a' <= (c) && (c) <= 'z') || ('A' <= (c) && (c) <= 'Z') )
#define isdigit(c)			( '0' <= (c) && (c) <= '9' )
#define isalnum(c)			( isalpha(c) || isdigit(c) )
#define isspace(c)			( (c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' )
#define iscntrl(c)			( (0x00 <= (c) && (c) <= 0x1F) || (c) == 0x7F)
#define islower(c)			( 'a' <= (c) && (c) <= 'z' )
#define isprint(c)			( (0x20 <= (c) && (c) <= 0x7E) )
#define ispunct(c)			( isprint(c) && ( !isalnum(c) && !isspace(c) ))
#define isupper(c)			( 'A' <= (c) && (c) <= 'Z' )
#define isxdigit(c)			( isdigit(c) || ('a' <= (c) && (c) <= 'f') || ('a' <= (c) && (c) <= 'f') )
#endif
*/
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

#ifndef MAX_PATH
#define MAX_PATH 2048
#endif

#ifndef L_tmpnam
#define L_tmpnam MAX_PATH
#endif

const char *getenv( const char *name );
FILE *tmpfile();

int system( const char * );
int rename( const char *, const char * );
int remove( const char * );
char *tmpnam( char * );

//char *setlocale( int category, const char *locale );

#define strerror(errnum) ("(unsupported)")
extern int errno;

