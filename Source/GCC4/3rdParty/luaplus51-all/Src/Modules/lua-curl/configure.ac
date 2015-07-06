AC_INIT([Lua-cURL],[0.2.1])
AM_INIT_AUTOMAKE
AC_PROG_CC
dnl libtool magic 
AC_DISABLE_STATIC		
AC_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES(LUA, lua >= 5.1)
PKG_CHECK_MODULES(CURL, libcurl)

AC_ARG_WITH(cmoddir,
	AC_HELP_STRING([--with-cmoddir=DIR],
		[Specify the location of Lua module directory to use.]),	
	dnl override pkgconfig module dir 
	[CMOD_INSTALL=$withval],
	[CMOD_INSTALL="`${PKG_CONFIG} --variable=INSTALL_CMOD lua`"])

AC_MSG_NOTICE([using Lua module directory $CMOD_INSTALL])
AC_SUBST(CMOD_INSTALL)

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
