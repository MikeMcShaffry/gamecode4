dnl configure.ac for MCPP   2003/11, 2008/11  kmatsui
dnl Process this file with autoconf to produce a configure script.

dnl WARNING: This script does not assume cross-compiling.
dnl To cross-compile you must edit here and there of this file.

AC_INIT( mcpp, 2.7.2, kmatsui@t3.rim.or.jp)
AC_CONFIG_SRCDIR( src/main.c)
AM_CONFIG_HEADER( src/config.h)
AC_CONFIG_AUX_DIR( config)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE( no-dependencies)

dnl Checks for programs.

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_EGREP
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

dnl WARNING: Configure must check the characteristics of target compiler.
dnl MCPP may modify the behavior of the target compiler-system.
dnl You must uninstall MCPP before re-configuring.

AC_MSG_CHECKING( [whether the cpp is target preprocessor])
mcpp=`$CPP $CFLAGS -xc -v /dev/null 2>&1 | grep 'MCPP'`
if test "x$mcpp" != x; then
    AC_MSG_ERROR( Do 'make uninstall' before configuring again.)
fi
AC_MSG_RESULT( yes)

dnl Define host system and target system

case $host_os in
linux*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_LINUX], [Define the host system.])
    ac_c_define_HOST_SYSTEM=[SYS_LINUX]
    ;;
freebsd*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_FREEBSD])
    ac_c_define_HOST_SYSTEM=[SYS_FREEBSD]
    ;;
darwin*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_MAC])
    ac_c_define_HOST_SYSTEM=[SYS_MAC]
    ;;
cygwin*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_CYGWIN])
    ac_c_define_HOST_SYSTEM=[SYS_CYGWIN]
    ;;
mingw*)
    AC_DEFINE( [HOST_SYSTEM], [SYS_MINGW])
    ac_c_define_HOST_SYSTEM=[SYS_MINGW]
    ;;
*)
    AC_MSG_WARN( [Unsupported host OS, assuming to be an UNIX variant])
    AC_DEFINE( [HOST_SYSTEM], [SYS_UNIX])
    ac_c_define_HOST_SYSTEM=[SYS_UNIX]
    ;;
esac
host_system=$ac_c_define_HOST_SYSTEM

case $target_os in
linux*)
    AC_DEFINE( [SYSTEM], [SYS_LINUX], [Define the target system.])
    ;;
freebsd*)
    AC_DEFINE( [SYSTEM], [SYS_FREEBSD])
    ;;
darwin*)
    AC_DEFINE( [SYSTEM], [SYS_MAC])
    ;;
cygwin*)
    AC_DEFINE( [SYSTEM], [SYS_CYGWIN])
    ;;
mingw*)
    AC_DEFINE( [SYSTEM], [SYS_MINGW])
    ;;
*)
    AC_MSG_WARN( Unsupported target OS, assuming to be an UNIX variant)
    AC_DEFINE( [SYSTEM], [SYS_UNIX])
    ;;
esac

## --enable-replace-cpp option
AC_ARG_ENABLE( replace-cpp,
    AC_HELP_STRING( [--enable-replace-cpp],
        [Replace the resident preprocessor with mcpp]),
    ,       ## the compiler name will be defined later
    AC_DEFINE( [COMPILER], [INDEPENDENT], [Define the target compiler.])
            ## else define COMPILER as INDEPENDENT
)

## Generally this configure does not support cross-compiling.
## Nevertheless, it can configure cross-compiler of Apple-GCC (i686 vs ppc).
if test x$enable_replace_cpp = xyes    \
        && test $ac_c_define_HOST_SYSTEM = SYS_MAC    \
        && test $host_cpu != $target_cpu; then
## Enable to build mcpp into cross-compiler of Apple-GCC
## --with-target-cc option
AC_ARG_WITH( target_cc,
    AC_HELP_STRING( [--with-target-cc],
        [Specify the target compiler to build mcpp into on Mac OS X]),
    if test x${withval+set} = xset; then
        target_cc=$withval
        target_name=`echo $target_cc | sed 's/-gcc.*$//'`
        if test $target_name != $target_alias; then
            AC_MSG_ERROR( [The argument of --with-target-cc option conflicts with --target option.])
        fi
        AC_SUBST( [target_cc])
    fi
)
fi

dnl Define a target specific macro.

AC_MSG_CHECKING( for target cpu)

## These are not predefined macros of MCPP.  MCPP will define predefined macros
## on compile time based on the CPU macro, and possibly redefine them at an
## execution time.

case $target_cpu in
x86_64|amd64)
    Target_Cpu=x86_64
    ;;
i?86*)
    Target_Cpu=i386
    ;;
powerpc64|ppc64)
    Target_Cpu=ppc64
    ;;
powerpc|ppc|ppc7400)
    Target_Cpu=ppc
    ;;
*)
    Target_Cpu=$target_cpu
    ;;
esac

AC_MSG_RESULT( $Target_Cpu)
AC_DEFINE_UNQUOTED( [CPU], "$Target_Cpu", [Define the cpu-specific-macro.])

## Build into cross-compiler on Mac OS
if test x$enable_replace_cpp = xyes    \
        && test $ac_c_define_HOST_SYSTEM = SYS_MAC    \
        && test x${target_cc+set} = xset; then
    arch_opt="-arch $Target_Cpu"
fi

dnl Checks for header files.

AC_CHECK_HEADERS( [unistd.h, stdint.h, inttypes.h])

dnl Checks for typedefs, and compiler characteristics.

AC_CHECK_TYPES( [intmax_t, long long])

dnl Check for library functions.

AC_CHECK_FUNCS( [stpcpy])

dnl Checks for some system characteristics.

AC_CACHE_CHECK( if the cases of filename are folded,
    ac_cv_fname_fold,
    [touch hJkL
    [ac_cv_fname_fold=no]
    AC_RUN_IFELSE( [[
#include    <stdio.h>
#include    <errno.h>
    void exit();
int main( void)
{
    if (fopen( "HjKl", "r") == NULL)
        exit( errno);
    else
        exit( 0);
} ]],
        [ac_cv_fname_fold=yes],
        [],
        AC_MSG_WARN( cannot run on cross-compiling so assumes that the cases of filename are not folded.)
    )
    rm hJkL],
)
if test x$ac_cv_fname_fold = xyes; then
    AC_DEFINE( [FNAME_FOLD], 1,
        [Define if the cases of file name are folded.])
fi

dnl Checks for some host compiler characteristics.

AC_DEFINE_UNQUOTED( [OBJEXT], ["$ac_cv_objext"],
    [Define the suffix of object file.])

dnl Check printf length modifier of the longest integer.

# AC_CHECK_MOD_LL( modifier, additional include header)
# Try the printf length modifier for the longest integer.
# The redundant sscanf() is for some compilers which cannot handle
# long long integer token.
AC_DEFUN([AC_CHECK_MOD_LL], [
    AC_CACHE_CHECK( [printf length modifier for the longest integer],
        [ac_cv_c_printf_modifier],
        AC_RUN_IFELSE( [[
#include    <stdio.h>
#include    <assert.h>
    int     strcmp();
    void    exit();
$3
int main( void)
{   char    buf@<:@ 20@:>@;
    $1  num;
    sscanf( "1234567890123456789", "%$2d", &num);
    sprintf( buf, "%$2d", num);
    assert( strcmp( buf, "1234567890123456789") == 0);
    exit( 0);
} ]],
            [ac_cv_c_printf_modifier=$2],
            ,
            AC_MSG_WARN( cannot run on cross-compiling so assumes the length modifier of long long is "%ll")
            AC_DEFINE( [LL_FORM], "ll",
                [Define printf length modifier for the longest integer.])
            [ac_cv_c_printf_modifier=ll]
        )
    )
] )

if test x$ac_cv_type_intmax_t = xyes; then
    AC_CHECK_MOD_LL( [intmax_t], [j], [#include <stdint.h>])
    if test $ac_c_define_HOST_SYSTEM = SYS_CYGWIN \
            || test $ac_c_define_HOST_SYSTEM = SYS_MINGW; then
        sleep 1         # wait for cleanup
    fi
fi

if test x$ac_cv_type_long_long = xyes; then
    for modifier in ll I64 q L
    do
        AC_CHECK_MOD_LL( [long long], ${modifier})
        if test x${ac_cv_c_printf_modifier+set} = xset; then
            break
        fi
        if test $ac_c_define_HOST_SYSTEM = SYS_CYGWIN \
                || test $ac_c_define_HOST_SYSTEM = SYS_MINGW; then
            sleep 1         # wait for cleanup
        fi
    done
fi

if test x${ac_cv_c_printf_modifier+set} = xset; then
    AC_DEFINE_UNQUOTED( [LL_FORM], "$ac_cv_c_printf_modifier",
        [Define printf length modifier for the longest integer.])
fi

if test $ac_c_define_HOST_SYSTEM = SYS_FREEBSD &&   \
        test x$mcpp_ac_c_define_SYSTEM_SP_STD != x__FreeBSD__; then
    fbsd_ver=`echo '__FreeBSD__' | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ .*//'`
    AC_DEFINE_UNQUOTED( [SYSTEM_EXT_VAL], "$fbsd_ver",
        [Define the version of FreeBSD.])
fi

if test x$ac_cv_c_compiler_gnu = xyes; then

    AC_DEFINE( [HOST_COMPILER], [GNUC], [Define the host compiler.])

    AC_DEFINE( [HOST_CMP_NAME], "GCC", [Define the host compiler name.])

    AC_MSG_CHECKING( for value of __GNUC__)
    gcc_maj_ver=`echo '__GNUC__' | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *//'`
    AC_MSG_RESULT( $gcc_maj_ver)
    AC_DEFINE_UNQUOTED( [GCC_MAJOR_VERSION], "$gcc_maj_ver",
        [Define gcc major version.])
    AC_MSG_CHECKING( for value of __GNUC_MINOR__)
    gcc_min_ver=`echo __GNUC_MINOR__ | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *//'`
    AC_MSG_RESULT( $gcc_min_ver)
    AC_DEFINE_UNQUOTED( [GCC_MINOR_VERSION], "$gcc_min_ver",
        [Define gcc minor version.])

    cw=`pwd`
    if test x$srcdir = "x." || test x$srcdir = x; then
        src_dir=$cw
    else
        src_dir=$srcdir
    fi
    if test $ac_c_define_HOST_SYSTEM = SYS_CYGWIN; then
        AC_CACHE_CHECK( the root directory,
            ac_cv_cygwin_root,
            [ac_cv_cygwin_root=`$src_dir/config/cygwin_root`]
        )
        AC_DEFINE_UNQUOTED( [CYGWIN_ROOT_DIRECTORY], "$ac_cv_cygwin_root",
            [Define root directory of CYGWIN.])
    fi
    if test $ac_c_define_HOST_SYSTEM = SYS_MINGW; then
        AC_CACHE_CHECK( the root directory,
            ac_cv_msys_root,
            cd /tmp
            cat > getcwd.c <<\_EOF
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
int main( int argc, char ** argv)
{   char    buf@<:@ PATH_MAX@:>@;
    getcwd( buf, PATH_MAX);
    puts( buf);
    return 0;
}
_EOF
            $CC getcwd.c
            cd /
            [ac_cv_msys_root=`$src_dir/config/mingw_root`]
        )
        AC_CACHE_CHECK( the /mingw directory,
            ac_cv_mingw_dir,
            cd /mingw
            [ac_cv_mingw_dir=`$src_dir/config/mingw_root`]
        )
        cd $cw
        rm -f /tmp/getcwd.c /tmp/a.exe
        AC_DEFINE_UNQUOTED( [MSYS_ROOT_DIRECTORY], "$ac_cv_msys_root",
            [Define root directory of MSYS.])
        AC_DEFINE_UNQUOTED( [MINGW_DIRECTORY], "$ac_cv_mingw_dir",
            [Define /mingw directory.])
    fi

else    # not gcc

# mcpp_AC_C_DEFINE( SYSTEM | CPU | COMPILER, STD | OLD, $enableval)
# Specify the target-specific predefined macro.
AC_DEFUN([mcpp_AC_C_DEFINE], [
    name=`echo $3 | sed 's/=.*//'`
    mcpp_macro=$1_SP_$2
    stripped_mcpp_macro=`echo $mcpp_macro | sed 's/STD//'`
    if test x$stripped_mcpp_macro != x$mcpp_macro; then
        stripped_name=`echo $name | sed 's/^_//'`
        if test x$stripped_name = x$name; then
            AC_MSG_ERROR( The macro name '$name' should begin with '_'.)
        fi
    else
        stripped_name=`echo $name | sed 's/^@<:@A-Za-z@:>@//'`
        if test x$stripped_name = x$name; then
            AC_MSG_ERROR( The macro name '$name' should begin with an alphabetic letter.)
        fi
    fi
    if test x$name = x$3; then
        [replace=1]
    else
        [replace=`echo $3 | sed 's/^.*=//'`]
    fi
    AC_DEFINE_UNQUOTED( [$mcpp_macro], "$name")
    AC_DEFINE_UNQUOTED( [${mcpp_macro}_VAL], "$replace")
    mcpp_ac_c_define_$1_$2=$name
])

# mcpp_AC_C_PREDEF( SYSTEM | CPU | COMPILER, STD | OLD, additional description)
# Specify the target-specific predefined macro via mcpp_AC_C_DEFINE.
AC_DEFUN([mcpp_AC_C_PREDEF], [
    AC_ARG_ENABLE( $1-$2-macro,
        AC_HELP_STRING( --enable-$1-$2-macro=name@<:@=replace@:>@,
            [specify the $1-specific $2-style predefined macro.  $3]),
        if test x$enableval = xyes || test x$enableval = xno; then
            AC_MSG_ERROR( This option requires an argument as 'name@<:@=replace@:>@'.)
        fi
        mcpp_AC_C_DEFINE( $1, $2, $enableval)
        AH_TEMPLATE( [$1_SP_$2],
            [Define the name of $1-specific $2-style predefined macro.])
        AH_TEMPLATE( [$1_SP_$2_VAL],
            [Define the value of $1-specific $2-style predefined macro.])
    )
])

mcpp_AC_C_PREDEF( [SYSTEM], [STD],
    [In all this and the following 5 options,
    'STD'-style name begins with an '_',
    'OLD'-style name begins with an alphabetic and
    if '=replace' is missing the macro is defined as 1.]
)
mcpp_AC_C_PREDEF( [SYSTEM], [OLD])
mcpp_AC_C_PREDEF( [CPU], [STD])
mcpp_AC_C_PREDEF( [CPU], [OLD])
mcpp_AC_C_PREDEF( [COMPILER], [STD])
mcpp_AC_C_PREDEF( [COMPILER], [OLD])

fi

## MacOS has "framework" directories.
if test $ac_c_define_HOST_SYSTEM = SYS_MAC; then
    AC_MSG_CHECKING( for the MacOS-specific framework directory)
    frameworks=`$CPP $CFLAGS $arch_opt -xc -v /dev/null 2>&1 | sed 's/^ *//' | sed 's/ .*$//' | $EGREP '/Frameworks$'`
    n=1
    for dir in $frameworks
    do
        case $n in
        1)  framework1=$dir;;
        2)  framework2=$dir;;
        3)  framework3=$dir;;
        esac
        let n=n+1
    done
    if test x${framework1+set} = xset; then
        AC_DEFINE_UNQUOTED( [FRAMEWORK1], "$framework1",
            [Define MacOS-specific framework directory 1.])
        AC_MSG_RESULT( $framework1)
    else
        AC_MSG_RESULT( none)
    fi
    if test x${framework2+set} = xset; then
        AC_DEFINE_UNQUOTED( [FRAMEWORK2], "$framework2",
            [Define MacOS-specific framework directory 2.])
        AC_MSG_RESULT( $framework2)
    fi
    if test x${framework3+set} = xset; then
        AC_DEFINE_UNQUOTED( [FRAMEWORK3], "$framework3",
            [Define MacOS-specific framework directory 3.])
        AC_MSG_RESULT( $framework3)
    fi
fi  ## SYS_MAC

## --enable-mcpplib option
AC_ARG_ENABLE( mcpplib,
    AC_HELP_STRING( [--enable-mcpplib],
        [Build libmcpp rather than stand-alone mcpp])
)

if test x$enable_mcpplib = xyes; then               ## mcpplib mode
    AC_DEFINE( [MCPP_LIB], 1, [Define if build libmcpp])
    if test x$enable_replace_cpp = xyes; then
        AC_MSG_WARN( Ignores enable-replace-cpp option)
        [enable_replace_cpp=no]
    fi

else

if test x$enable_replace_cpp = xyes; then           ## replace-cpp mode

dnl Check the characteristics of target compiler.
dnl Note: The following 3 hundred and more lines are for replace-cpp-mode.
dnl Target-compiler-specific configuration need these lines.
dnl These lines are not used in compiler-independent configuration.

dnl Check which header file CPP includes,
dnl relative to CURRENT or SOURCE directory.

# AC_C_INCLUDE_PREF( CURRENT | SOURCE | NEXT)
# Check the include preference ('NEXT' means CURRENT & SOURCE)
AC_DEFUN([AC_C_INCLUDE_PREF], [
    AC_EGREP_CPP( $1, [
#include "../next/header.h"
],
        [ac_cv_c_include_pref=$1],
    )
])

AC_CACHE_CHECK( include preference - relative to current or source directory,
    ac_cv_c_include_pref,
    mkdir mcpp-tmp
    cd mcpp-tmp
    mkdir dir ../next ../next/dir
    echo '#include "dir/what.h"'    > ../next/header.h
    echo '#include "next.h"'        >> ../next/header.h
    echo 'CURRENT'  > dir/what.h
    echo 'SOURCE'   > ../next/dir/what.h
    echo 'NEXT'     > ../next/next.h
    AC_C_INCLUDE_PREF( [SOURCE])
    if test x$ac_cv_c_include_pref != xSOURCE; then
        AC_C_INCLUDE_PREF( [CURRENT])
        if test x$ac_cv_c_include_pref = xCURRENT; then
            AC_C_INCLUDE_PREF( [NEXT])
            if test x$ac_cv_c_include_pref = xNEXT; then
                [ac_cv_c_include_pref="(CURRENT & SOURCE)"]
            fi
        else
            AC_MSG_ERROR( Failed to include header file.)
        fi
    fi
    cd ..
    rm -rf mcpp-tmp next
    if test $ac_c_define_HOST_SYSTEM = SYS_CYGWIN \
            || test $ac_c_define_HOST_SYSTEM = SYS_MINGW; then
        sleep 1         # wait for cleanup
    fi
)
AC_DEFINE_UNQUOTED( [SEARCH_INIT], $ac_cv_c_include_pref,
    [Define include preference.])
 
AC_CACHE_CHECK( if the argument of pragma is macro expanded,
    ac_cv_c_pragma_is_expanded,
    AC_COMPILE_IFELSE( [[
#define once
#pragma once]],
        [ac_cv_c_pragma_is_expanded=no],
        [ac_cv_c_pragma_is_expanded=yes]
    )
)
if test x$ac_cv_c_pragma_is_expanded = xyes; then
    AC_DEFINE( [EXPAND_PRAGMA], 1,
        [Define if the argument of pragma is macro expanded.])
fi

AC_CACHE_CHECK( if digraphs can be used,
    ac_cv_c_digraph,
    [AC_PREPROC_IFELSE( [
%:if 1
%:endif],
        [ac_cv_c_digraph=yes],
        [ac_cv_c_digraph=no]
    )]
)
if test x$ac_cv_c_digraph = xyes; then
    AC_DEFINE( [HAVE_DIGRAPHS], 1, [Define if digraphs are available.])
fi

AC_CACHE_CHECK( preprocessor output format of line directive,
    ac_cv_c_line_format,
    [ac_cv_c_line_format=\"`echo '#line 4321' | $CPP - | grep 432 | sed 's/432.*$//'`\"]
## Note: the output line number may be 4320 on GCC 2.* or other preprocessors.
)
AC_DEFINE_UNQUOTED( [LINE_PREFIX], $ac_cv_c_line_format,
    [Define output format of line directive.])
if test "x$ac_cv_c_line_format" = "x\"#line \""; then
    ac_cv_c_std_line_prefix=1
else
    ac_cv_c_std_line_prefix=0
fi
AC_DEFINE_UNQUOTED( [STD_LINE_PREFIX], $ac_cv_c_std_line_prefix,
    [Define whether output format of line directive is C source style.])

AC_CACHE_CHECK( default value of __STDC__,
    ac_cv_c_stdc,
    [stdc=`echo '__STDC__' | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ .*//'`]
    if test x"$stdc" = x"__STDC__"; then
        [ac_cv_c_stdc=0]
    else
        [ac_cv_c_stdc=$stdc]
    fi
)
AC_DEFINE_UNQUOTED( [STDC], $ac_cv_c_stdc,
    [Define the default value of __STDC__.])

AC_CACHE_CHECK( default value of __STDC_VERSION__,
    ac_cv_c_stdc_ver,
    [stdc_ver=`echo '__STDC_VERSION__' | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ .*//'`]
    if test x"$stdc_ver" = x"__STDC_VERSION__"; then
        [ac_cv_c_stdc_ver=0]
    else
        [ac_cv_c_stdc_ver=$stdc_ver]
    fi
)
AC_DEFINE_UNQUOTED( [STDC_VERSION], $ac_cv_c_stdc_ver,
    [Define the default value of __STDC_VERSION__.])

if test x${ac_cv_c_compiler_gnu} != xyes; then

# Specify the compiler's name
AC_ARG_WITH( compiler-name,
    AC_HELP_STRING( [--with-compiler-name=NAME],
        [specify the compiler name which is used as the definition of COMPILER macro]),
    AC_DEFINE_UNQUOTED( [COMPILER], $compiler_name, [specify the compiler name]),
    AC_MSG_ERROR( --with-compiler-name=NAME option should be specified to define [COMPILER] macro.)
)

# Specify the C++ header file directory
AC_ARG_WITH( cxx-include-dir,
    AC_HELP_STRING( [--with-cxx-include-dir=directory],
        [specify the C++ include directory by absolute path [[none]]]),
    [case "${withval}" in
    [\\/]* | [A-Za-z]:[\\/]*)
        cxx_include_dir=`echo $withval | sed 's,/$,,'; 's,\\$,,'`
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR1], $cxx_include_dir,
            [Define the C++ include directory.])
        ;;
    *)  AC_MSG_ERROR( $withval should be an absolute path.) ;;
    esac]
)

fi  # not gcc

dnl Define compiler specific elements.

AC_MSG_NOTICE( checking for some compiler specific elements)

# Define GCC specific elements.
if test x$ac_cv_c_compiler_gnu = xyes; then

    AC_DEFINE( [COMPILER], [GNUC], [Define the target compiler.])

    cpp_call=`$CPP $CFLAGS $arch_opt -xc -v /dev/null 2>&1 | $EGREP '/cpp|/cc1' | sed 's/^ *//; s/ .*//'` 
    AC_MSG_CHECKING( for the directory to install mcpp)
    cpp_name=`expr $cpp_call : '.*/\(.*\)'`
    bindir=`expr $cpp_call : "\(.*\)/$cpp_name"`
    AC_MSG_RESULT( $bindir)

    AC_PATH_PROG( [gcc_path], [$CC], , [$PATH])
    cc_path=`echo $CC | $EGREP "/"`
    if test x$cc_path != x; then
        AC_MSG_ERROR( [Environment variables CC and CXX for GCC-specific-build should not contain a directory part.]);
    fi
    ## force to set $prefix to GCC's prefix
    ## (i.e. "/usr/local" of "/usr/local/bin/gcc")
    gcc_prefix=`echo $gcc_path | $EGREP "/bin/$CC\$" | sed "s,/bin/$CC\$,,"`
    if test "x$gcc_prefix" != xNONE; then
        if test "x$prefix" != xNONE && "x$gcc_prefix" != "x$prefix"; then
            AC_MSG_WARN( [Ignored --prefix option, since it conflicts with GCC path])
        fi
        prefix=$gcc_prefix
    else
        if test "x$prefix" = xNONE; then
            prefix=$ac_default_prefix
        fi
    fi
    lang_save=$LANG
    export LANG=C

    AC_MSG_CHECKING( for the compiler-specific C include directory)
    exclude="^/usr(/local|)/include\$|^${prefix}(/local|)/include\$|/Frameworks|End of search list"
    c_include_dirs=`$CPP $CFLAGS $arch_opt -xc -v /dev/null 2>&1 | sed '1,/<...> search starts here/d' | sed '/End of search list/q' | sed 's/^ *//' | $EGREP -v "$exclude"`
    n=1
    for dir in $c_include_dirs
    do
        case $n in
        1)  c_include_dir1=$dir;;
        2)  c_include_dir2=$dir;;
        3)  c_include_dir3=$dir;;
        esac
        let n=n+1
    done
    if test x${c_include_dir1+set} = xset; then
        AC_DEFINE_UNQUOTED( [C_INCLUDE_DIR1], "$c_include_dir1",
            [Define compiler-specific C include directory 1.])
        AC_MSG_RESULT( $c_include_dir1)
    else
        AC_MSG_RESULT( none)
    fi
    if test x${c_include_dir2+set} = xset; then
        AC_DEFINE_UNQUOTED( [C_INCLUDE_DIR2], "$c_include_dir2",
            [Define compiler-specific C include directory 2.])
        AC_MSG_RESULT( $c_include_dir2)
    fi
    if test x${c_include_dir3+set} = xset; then
        AC_DEFINE_UNQUOTED( [C_INCLUDE_DIR3], "$c_include_dir3",
            [Define compiler-specific C include directory 3.])
        AC_MSG_RESULT( $c_include_dir3)
    fi

    AC_PROG_CXX
    AC_PROG_CXXCPP
    AC_LANG_PUSH( C++)
    AC_MSG_CHECKING( for the compiler-specific C++ include directory)
    exclude="^/usr(/local|)/include\$|^${prefix}(/local|)/include\$|/Frameworks|^${c_include_dir1}\$|^${c_include_dir2}\$|^${c_include_dir3}\$"
    cxx_include_dirs=`${CXXCPP} ${CXXFLAGS} $arch_opt -xc++ -v /dev/null 2>&1 | sed '1,/<...> search starts here/d' | sed '/End of search list/q' | sed '/End of search list/d' | sed 's/^ *//' | $EGREP -v "$exclude"`
    n=1
    for dir in ${cxx_include_dirs}
    do
        case $n in
        1)  cxx_include_dir1=$dir;;
        2)  cxx_include_dir2=$dir;;
        3)  cxx_include_dir3=$dir;;
        4)  cxx_include_dir4=$dir;;
        esac
        let n=n+1
    done
    if test x${cxx_include_dir1+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR1], "$cxx_include_dir1",
            [Define compiler-specific C++ include directory 1.])
        AC_MSG_RESULT( $cxx_include_dir1)
    fi
    if test x${cxx_include_dir2+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR2], "$cxx_include_dir2",
            [Define compiler-specific C++ include directory 2.])
        AC_MSG_RESULT( $cxx_include_dir2)
    fi
    if test x${cxx_include_dir3+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR3], "$cxx_include_dir3",
            [Define compiler-specific C++ include directory 3.])
        AC_MSG_RESULT( $cxx_include_dir3)
    fi
    if test x${cxx_include_dir4+set} = xset; then
        AC_DEFINE_UNQUOTED( [CPLUS_INCLUDE_DIR4], "$cxx_include_dir4",
            [Define compiler-specific C++ include directory 4.])
        AC_MSG_RESULT( $cxx_include_dir4)
    fi
    AC_LANG_POP( C++)

    export LANG=$lang_save

    ## the directory into which mcpp_g*.h header files are generated
    ## '/mcpp-gcc' will be appended to 'inc_dir'
    if test $ac_c_define_HOST_SYSTEM = SYS_MINGW; then
        inc_dir=$c_include_dir2
    else
        if test x${c_include_dir1+set} = xset; then
            inc_dir=$c_include_dir1
        else
            inc_dir="$prefix"/include
        fi
    fi
    AC_DEFINE_UNQUOTED( [INC_DIR], "$inc_dir",
        [Define include directory to install mcpp_g*.h header files.])

    AC_SUBST( [CPPFLAGS])
    AC_SUBST( [host_system])
    AC_SUBST( [Target_Cpu])
    AC_SUBST( [inc_dir])
    AC_SUBST( [gcc_maj_ver])
    AC_SUBST( [gcc_min_ver])
    AC_SUBST( [cpp_call])
    AC_PROG_LN_S

    if test x$gcc_maj_ver = x2; then

        AC_CACHE_CHECK( for value of predefined macro '__SIZE_TYPE__',
            size_type,
            [size_type=`echo '__SIZE_TYPE__' | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *//'`]
        )
        AC_DEFINE_UNQUOTED( [COMPILER_SP1_VAL], "$size_type",
            [Define the type of size_t.])

        AC_CACHE_CHECK( for value of predefined macro '__PTRDIFF_TYPE__',
            ptrdiff_type,
            [ptrdiff_type=`echo '__PTRDIFF_TYPE__' | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *$//'`]
        )
        AC_DEFINE_UNQUOTED( [COMPILER_SP2_VAL], "$ptrdiff_type",
            [Define the type of ptrdiff_t.])

        AC_CACHE_CHECK( for value of predefined macro '__WCHAR_TYPE__',
            wchar_type,
            [wchar_type=`echo '__WCHAR_TYPE__' | $CPP - | sed 's/^ *//' | $EGREP -v '^#|^$' | sed 's/ *$//'`]
        )
        AC_DEFINE_UNQUOTED( [COMPILER_SP3_VAL], "$wchar_type",
        [Define the type of wchar_t.])

    fi
fi

## Handling of multi-byte characters containing '0x5c' codes:
## These tests cannot be executed without appropriate options for the
## compiler.  So, here we define all of these macros as FALSE.
## User should edit config.h by hand after executing this script.
## These should be 0 for GCC at least upto V.4.1.
    AC_DEFINE( [SJIS_IS_ESCAPE_FREE], 0,
        [Define if '0x5c' in SJIS multi-byte character is safe.])
    AC_DEFINE( [ISO2022_JP_IS_ESCAPE_FREE], 0,
        [Define if '0x5c' in ISO2022-JP multi-byte character is safe.])
    AC_DEFINE( [BIGFIVE_IS_ESCAPE_FREE], 0,
        [Define if '0x5c' in BIG5 multi-byte character is safe.])

fi ## replace-cpp mode
fi ## build-cpp mode or replace-cpp mode

AC_ARG_WITH( [gcc-testsuite-dir],
    AC_HELP_STRING( [--with-gcc-testsuite-dir=directory],
        [specify the gcc testsuite directory to do make check.
        e.g. /usr/src/rpm/SOURCES/gcc-2.95.4/gcc/testsuite,
        /usr/local/src/gcc-3.2/gcc/testsuite, etc.
        The testsuite of mcpp uses gcc testsuite, hence needs gcc source
        or at least gcc testsuite.
        This option needs --enable-replace-cpp option.]),
    if test x${withval+set} = xset; then
        if test x$enable_replace_cpp = xyes; then
            [gcc_testsuite_dir=`echo $withval | sed 's,/$,,'`]
        else
            AC_MSG_ERROR( [--with-gcc-testsuite-dir option requires --enable-replace-cpp option.])
        fi
    else
        AC_MSG_ERROR( [--with-gcc-testsuite-dir option requires an argument.])
    fi
)
AC_SUBST( [gcc_testsuite_dir])
AM_CONDITIONAL( [GNUC], test x$ac_cv_c_compiler_gnu = xyes)
AM_CONDITIONAL( [MCPP_LIB], test x$enable_mcpplib = xyes)
AM_CONDITIONAL( [REPLACE_CPP], test x$enable_replace_cpp = xyes)
AM_CONDITIONAL( [MINGW], test $ac_c_define_HOST_SYSTEM = SYS_MINGW)

dnl Final output.

AC_OUTPUT( Makefile src/Makefile tests/Makefile)
