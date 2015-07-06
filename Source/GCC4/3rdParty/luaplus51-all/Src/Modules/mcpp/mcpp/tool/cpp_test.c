/*
 * cpp_test.c:  to compile and run n_*.c, i_*.c;
 *              make a summary of the results.
 *      1998/08 by kmatsui
 *              Made after runtest.c and summtest.c of
 *              "Plum-Hall Validation Sampler".
 *      2005/03 by kmatsui
 *              Changed 'PGNUM'.
 *      2006/07 by kmatsui
 *              Changed 'PGNUM'.
 *              Changed non-prototype declarations to prototype ones.
 *              Removed DOS-extender option.
 */

#include    "stdio.h"
#include    "stdlib.h"
#include    "string.h"
#include    "ctype.h"
#include    "errno.h"

#define NAMEMAX     8
#define PGMNUM      35

#if     __MSDOS__ || __WIN32__ || _WIN32
#define PATH_DELIM  '\\'
#else
#define PATH_DELIM  '/'
#endif

#if 0
extern int  sleep( unsigned int seconds);
#endif

void    test_cpp( int argc, char **argv);
void    sum_test( void);
void    usage( void);

char    comp_fmt[ BUFSIZ/2] = "cc -o%s %s.c";
char    cmp_name[ NAMEMAX+1];
char    out_file[ NAMEMAX+5];
char    err_file[ NAMEMAX+5];
char    sum_file[ NAMEMAX+5];
char    err_name[ PGMNUM][ NAMEMAX+5];
char    buf[ BUFSIZ];

int main( int argc, char **argv) {
    char    *cp;
    int     i;

    if (argc < 2)
        usage();
    if (! isalpha( argv[ 1][ 0])
            || strchr( argv[ 1], '.') != NULL
            || strlen( argv[ 1]) > NAMEMAX) {
        usage();
    }
    strcpy( cmp_name, argv[ 1]);
    sprintf( sum_file, "%s.sum", cmp_name);
    sprintf( out_file, "%s.out", cmp_name);
    sprintf( err_file, "%s.err", cmp_name);
    for (cp = cmp_name; (i = *cp) != 0; cp++) {
        if (i == '-' || i == '_' || i == '~')
            *cp = '|';      /* Convert horizontal line to vertical line */
    }
    if (argc > 2)
        strcpy( comp_fmt, argv[ 2]);
    if (freopen( out_file, "w", stdout) == NULL)
        usage();
    if (freopen( err_file, "w", stderr) == NULL)
        usage();
    setbuf( stdout, NULL);
    setbuf( stderr, NULL);

    test_cpp( argc, argv);
    sum_test();

    return  0;
}

void    test_cpp( int argc, char ** argv) {
    int     i, len;
    int     pgm_num;

    for (pgm_num = 0; pgm_num < PGMNUM && ! feof( stdin); pgm_num++) {
        /* for each input program   */
        char    pgm_name[ NAMEMAX+2];

        fgets( pgm_name, NAMEMAX+1, stdin);
        if ((len = strlen( pgm_name)) < 2 || *(pgm_name + len - 1) != '\n')
            continue;           /* skip erroneous line      */
        else
            *(pgm_name + len - 1) = '\0';   /* remove '\n'  */

        sprintf( buf, comp_fmt, pgm_name, pgm_name);
        system( buf);
        printf( "COMPILE:    %s\n", buf);
#if 0
        sleep( 1);                      /* Wait a moment    */
#endif
        sprintf( err_name[ pgm_num], "%s.err", pgm_name);
        freopen( err_name[ pgm_num], "w", stderr);
        sprintf( buf, ".%c%s", PATH_DELIM, pgm_name);
        system( buf);
        printf( "EXECUTE:    %s\n", buf);
#if 0
        sleep( 1);                      /* Wait a moment    */
#endif
        freopen( err_file, "a", stderr);
        for (i = 3; i < argc; ++i) {
            sprintf( buf, argv[i], pgm_name, pgm_name);
            system( buf);
            printf( "CLEANUP:    %s\n", buf);
        }
    }   /* end loop over each program   */
}

void    sum_test( void)
{
    FILE    *sumfp, *errfp;
    char    *cp;
    int     i, len;
    int     nerror = 0;
    int     pgm_num;

    /* Make a column of summary */
    if ((sumfp = fopen( sum_file, "w")) == NULL) {
        fprintf( stderr, "Can't open %s\n", sum_file);
        exit( errno);
    }
    len = strlen( cmp_name);
    for (i = 0; i < len; i++)
        fprintf( sumfp, "  %c\n", cmp_name[i]);
    for (i = len; i <= NAMEMAX; i++)
        fputs( "   \n", sumfp);

    for (pgm_num = 0; pgm_num < PGMNUM; pgm_num++) {
        if ((errfp = fopen( err_name[ pgm_num], "r")) == NULL) {
            fputs( "  -\n", sumfp);
            nerror++;
        } else {
            cp = fgets( buf, BUFSIZ, errfp);
            if (cp == NULL || strcmp( buf, "started\n") != 0) {
                fputs( "  -\n", sumfp);
                nerror++;
            } else {
                fgets( buf, BUFSIZ, errfp);
                if (strcmp( "success\n", buf) != 0) {
                    fputs( "  o\n", sumfp);
                    nerror++;
                } else {
                    fputs( "  *\n", sumfp);
                }
            }
            fclose( errfp);
        }
    }

    fprintf( sumfp, "\n%3d\n", nerror);
}

void    usage( void)
{
    fputs( "Usage:\n"
    "  cpp_test compiler-name \"compile command\" \"cleaning command\"s\n"
    "  Compiler-name must be 8 bytes or less and must be without dot.\n"
    "  Number of \"cleaning command\"s can be any.\n"
    "  Do this command in mcpp's test-c directory.\n"
    "Examples:\n"
	"    cpp_test GCC332 \"gcc -std=iso9899:199409 -o%s %s.c\""
    " \"rm %s\" < n_i_.lst\n"
    "    cpp_test VC2005 \"cl -Za -D_CRT_SECURE_NO_DEPRECATE -Fe%s.exe %s.c\""
    " \"del %s.exe\" < n_i_.lst\n"
    "    cpp_test MCPP26 \"gcc -Wp,-23 -std=iso9899:199409 -o%s %s.c\""
    " \"rm %s\" < n_i_.lst\n"
            , stderr);
    exit( 0);
}

