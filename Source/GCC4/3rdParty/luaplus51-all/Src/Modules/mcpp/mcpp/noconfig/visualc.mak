# makefile to compile MCPP version 2.7.1 and later for Visual C / nmake
#       2008/11 kmatsui
# You must first edit BINDIR, LIBDIR and INCDIR according to your system.
# To make compiler-independent-build of MCPP do:
#       nmake
# To make Visual-C-specific-build of MCPP do:
#       nmake COMPILER=MSC
# To re-compile MCPP using Visual-C-specific-build of MCPP do:
#       nmake COMPILER=MSC PREPROCESSED=1
# To link kmmalloc V.2.5.3 (malloc() package of kmatsui) or later do:
#   (Note: Visual C 2005 and 2008 cannot coexist with kmmalloc)
#       nmake [PREPROCESSED=1] KMMALLOC=1
# To make mcpp.lib (subroutine-build of mcpp) do:
#       nmake MCPP_LIB=1 mcpplib
#       nmake MCPP_LIB=1 mcpplib_install
# To make testmain.c (sample to use mcpp.lib) against mcpp.lib do
#   (add 'DLL_IMPORT=1' to link against the DLL):
#       nmake [OUT2MEM=1] testmain
#       nmake [OUT2MEM=1] testmain_install
# To use this Makefile in IDE of Visual C, include $(targ)_install target
#	in $(targ) target, since the IDE can't handle install target.

NAME = mcpp

CC = cl
CFLAGS = $(CFLAGS) -Za -c	# -Zi
	# Add -Zi for debugging on Visual C / IDE
LINKFLAGS = -Fe$(NAME)	# -Zi
CPPFLAGS = $(CPPFLAGS) -D_CRT_SECURE_NO_DEPRECATE # -Za
	# -D_CRT_SECURE_NO_DEPRECATE for Visual C 2005, 2008
	# -Za should not be specified for compiler-independent-built MCPP

!if "$(COMPILER)"=="MSC"
CPPFLAGS = $(CPPFLAGS) -DCOMPILER=MSC
# BINDIR : Adjust to your system.
#	for Visual C 2003
#BINDIR = "$(MSVCDIR)"\bin
#	for Visual C 2005, 2008
BINDIR = "$(VCINSTALLDIR)"\bin
!else
# compiler-independent-build: use compiler-independent directory
BINDIR = \PUB\bin
!endif

!ifdef KMMALLOC
MEM_MACRO = -DKMMALLOC -D_MEM_DEBUG -DXMALLOC
MEMLIB = kmmalloc_debug.lib
!else
MEM_MACRO =
MEMLIB =
!endif

OBJS = main.obj directive.obj eval.obj expand.obj support.obj system.obj \
        mbchar.obj

$(NAME).exe : $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(MEMLIB)
#install :
# Visual C++ / IDE "makefile project" does not have "make install" command
	copy /b $(NAME).exe $(BINDIR)\$(NAME).exe

!ifdef PREPROCESSED
# make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H internal.H
	$(BINDIR)\$(NAME) $(CPPFLAGS) $(LANG) $(MEM_MACRO) preproc.c mcpp.H
$(OBJS) : mcpp.H
system.H: noconfig.H
!else
$(OBJS) : noconfig.H
main.obj directive.obj eval.obj expand.obj support.obj system.obj mbchar.obj: \
        system.H internal.H
!endif

!ifdef PREPROCESSED
.c.obj	:
	$(BINDIR)\$(NAME) -DPREPROCESSED $(CPPFLAGS) $< $(<B).i
	$(CC) $(CFLAGS) -TC $(<B).i
!else
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) $<
!endif

clean	:
	-del *.obj *.i mcpp.H *.exe *.lib *.dll *.exp *.so

!ifdef	MCPP_LIB
#LIBDIR = "$(MSVCDIR)"\lib
LIBDIR = "$(VCINSTALLDIR)"\lib
INCDIR = "$(VCINSTALLDIR)"\include
CFLAGS = $(CFLAGS) -DMCPP_LIB

mcpplib: mcpplib_lib mcpplib_dll
# To use in Visual C IDE
#mcpplib: mcpplib_lib mcpplib_dll mcpplib_install

# Debug mode: Do 'nmake DEBUG=1 ...'
!ifdef DEBUG
CFLAGS = $(CFLAGS) -MDd -D_DEBUG
LIBSUFFIX = d
!else
CFLAGS = $(CFLAGS) -O2 -MD -DNDEBUG
!endif

mcpplib_lib:	$(OBJS)
	lib -out:mcpp$(LIBSUFFIX).lib $(OBJS)

# DLL
DLL_VER = 0
SOBJS = main.so directive.so eval.so expand.so support.so system.so mbchar.so
.SUFFIXES: .so
.c.so	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) -DDLL_EXPORT -TC -Fo$*.so $<
mcpplib_dll:	$(SOBJS)
	$(CC) -LD -Femcpp$(DLL_VER)$(LIBSUFFIX) $(SOBJS) $(MEMLIB)
mcpplib_install:
	copy mcpp$(LIBSUFFIX).lib $(LIBDIR)
	copy mcpp$(DLL_VER)$(LIBSUFFIX).lib $(LIBDIR)
	copy mcpp$(DLL_VER)$(LIBSUFFIX).dll $(BINDIR)
	copy mcpp_lib.h $(INCDIR)
	copy mcpp_out.h $(INCDIR)
	$(CC) main_libmcpp.c -Fe$(NAME).exe 	\
			$(LIBDIR)\mcpp$(DLL_VER)$(LIBSUFFIX).lib	\
			-link -force:multiple
	copy $(NAME).exe $(BINDIR)
mcpplib_uninstall:
	del $(LIBDIR)\mcpp$(LIBSUFFIX).lib	\
			$(LIBDIR)\mcpp$(DLL_VER)$(LIBSUFFIX).lib	\
			$(BINDIR)\mcpp$(DLL_VER)$(LIBSUFFIX).dll
	del $(BINDIR)\$(NAME).exe
	del $(INCDIR)\mcpp*
!endif

# use mcpp as a subroutine from testmain.c
!ifdef	DLL_IMPORT
CFLAGS = $(CFLAGS) -DDLL_IMPORT
LINKLIB = mcpp$(DLL_VER)$(LIBSUFFIX).lib
!else
LINKLIB = mcpp$(LIBSUFFIX).lib
!endif
TMAIN_LINKFLAGS = testmain.obj -Fetestmain.exe $(LIBDIR)\$(LINKLIB) \
			-link -force:multiple
!ifdef	OUT2MEM
# output to memory buffer
CFLAGS = $(CFLAGS) -DOUT2MEM
!endif
testmain	: testmain.obj
	$(CC) $(TMAIN_LINKFLAGS)
testmain_install	:
	copy testmain.exe $(BINDIR)
testmain_uninstall	:
	del $(BINDIR)\testmain.exe
