/*
 * rm_once.c    2002/08, 2004/11    kmatsui
 *  to remove '#pragma once' line from the top of header files
 */

#include    "stdio.h"
#include    "stdlib.h"
#include    "string.h"

main( int argc, char ** argv)
{
    char    buf[ BUFSIZ];
    char    *once = "\n#pragma once\n\n";
    char    *tmp = "tmp.once";
    char    *fname;
    FILE    *fp_in, *fp_out;
    int     i;
    char    *cp;

    while (*++argv) {
        if ((fp_in = fopen( *argv, "r")) != NULL) {
            for (cp = buf, i = 0; i < 3; i++, cp += strlen( cp))
                fgets( cp, BUFSIZ, fp_in);
            if (strcmp( buf, once) != 0) {
                fprintf( stderr, "Skipped %s\n", *argv);
                fclose( fp_in);
                continue;
            }
            fprintf( stderr, "Converted %s\n", *argv);
            fp_out = fopen( tmp, "w");
            while (fgets( buf, BUFSIZ, fp_in) != NULL)
                fputs( buf, fp_out);
            fclose( fp_in);
            fclose( fp_out);
            remove( *argv);
            rename( tmp, *argv);
        }
    }
    return 0;
}
