/* clock_of.c:  to measure the execution time of a program. */
/* 2002/08      kmatsui */

#include    "stdio.h"
#include    "stdlib.h"
#include    "string.h"
#include    "time.h"

void    usage( void);

int     main( int argc, char ** argv)
{
    char    command[ BUFSIZ];
    char    **arg, *buf;
    long    start_time, passed_time;
    int     ret;

    if (argc < 2)
        usage();
    ret = -1;
    arg = argv;
    buf = command;
    while (*++arg) {
        sprintf( buf, "%s ", *arg);
        buf += strlen( buf);
    }
    fprintf( stderr, "%s\n", command);
    start_time = clock();
    ret = system( command);
    if (ret == 0) {
        passed_time = clock() - start_time;
        fprintf( stderr, "%ld ticks passed (%f seconds).\n"
                , passed_time, (double) passed_time / CLOCKS_PER_SEC);
    } else {
        fprintf( stderr, "Command execution error! (status: %d)\n", ret);
    }

    return 0;
}

void    usage( void)
{
    fputs( "usage: clock_of <command> [<args>]\n", stderr);
    exit( 1);
}
