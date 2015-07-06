# makefile to compile MCPP version 2.7.1 and later for Borland C / BC make
#       2008/11 kmatsui
# You must first edit BINDIR, INCDIR, LIBDIR and LINKER according to your
#		system.
# To make compiler-independent-build of MCPP do:
#       make
#       make install
# To make Borland-C-specific-build of MCPP do:
#       make -DCOMPILER=BORLANDC
#       make -DCOMPILER=BORLANDC install
# To re-compile MCPP using Borland-C-specific-build of MCPP do:
#       make -DCOMPILER=BORLANDC -DPREPROCESSED
#       make -DCOMPILER=BORLANDC -DPREPROCESSED install
# To link kmmalloc V.2.5.1 (malloc() package of kmatsui) or later do:
#       make [-DPREPROCESSED] -DKMMALLOC
#       make [-DPREPROCESSED] -DKMMALLOC install
# To make mcpp.lib (subroutine-build of mcpp) do:
#       make -DMCPP_LIB mcpplib
#       make -DMCPP_LIB mcpplib_install
# To make testmain.c (sample to use mcpp.lib) against mcpp.lib do
#   (add '-DDLL_IMPORT' to link against the DLL):
#       make [-DOUT2MEM] testmain
#       make [-DOUT2MEM] testmain_install

NAME = mcpp

CC = bcc32
CFLAGS = $(CFLAGS) -c -a -d -f- -G -5 -DWIN32
	# -DWIN32 is nessecary to compile with bcc32
LINKFLAGS = -e$(NAME)

!if		$d( COMPILER)
CPPFLAGS = -DCOMPILER=BORLANDC
# BINDIR : Adjust to your system.
#	for Borland C V.5.5
CFLAGS = $(CFLAGS) -Oi
BINDIR = \PUB\COMPILERS\BCC55\BIN
!else
BINDIR = \PUB\BIN
!endif

!if 	$d( KMMALLOC)
MEM_MACRO = -DKMMALLOC=1 -D_MEM_DEBUG=1 -DXMALLOC=1
MEMLIB = kmmalloc_debug32.lib
!else
MEM_MACRO =
MEM_LIB =
!endif

OBJS = main.obj directive.obj eval.obj expand.obj support.obj system.obj  \
		mbchar.obj

all:	$(NAME).exe

$(NAME).exe : $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(MEMLIB)

!if 	$d( PREPROCESSED)
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
	$(BINDIR)\$(NAME) $(CPPFLAGS) $(MEM_MACRO) preproc.c mcpp.H
$(OBJS) : mcpp.H
!else
main.obj directive.obj eval.obj expand.obj support.obj system.obj mbchar.obj: \
		system.H internal.H noconfig.H
!endif

!if 	$d( PREPROCESSED)
.c.obj	:
	$(BINDIR)\$(NAME) -DPREPROCESSED=1 $(CPPFLAGS) $< $(<B).i
	$(CC) $(CFLAGS) $(<B).i
!else
.c.obj	:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(MEM_MACRO) $<
!endif

install :
	copy /b $(NAME).exe $(BINDIR)\$(NAME).exe

clean	:
	-del *.obj *.exe *.bak mcpp.H *.i *.tds *.lib *.dll mcpp$(DLL_VER).* *.so

!if 	$d( MCPP_LIB)
# subroutine-build
CFLAGS = $(CFLAGS) -DMCPP_LIB=1
LIBDIR = \PUB\COMPILERS\BCC55\LIB
INCDIR = \PUB\COMPILERS\BCC55\INCLUDE
LINKER = ilink32
ADD_OBJS = +main +directive +eval +expand +support +system +mbchar

mcpplib:	mcpplib_lib mcpplib_dll

mcpplib_lib:	$(OBJS)
	tlib mcpp.lib $(ADD_OBJS)

# DLL
DLL_VER = 0
SOBJS = main.so directive.so eval.so expand.so support.so system.so mbchar.so
.SUFFIXES: .so
.c.so   :
	$(CC) $(CFLAGS) $(MEM_MACRO) -DDLL_EXPORT=1 -o$*.so $<
mcpplib_dll: $(SOBJS)
	$(LINKER) -Tpd c0d32.obj $(SOBJS), mcpp$(DLL_VER).dll, , \
            cw32.lib import32.lib $(MEMLIB)
	implib mcpp$(DLL_VER).lib mcpp$(DLL_VER).dll

mcpplib_install:
	copy mcpp.lib $(LIBDIR)
	copy mcpp$(DLL_VER).lib $(LIBDIR)
	copy mcpp$(DLL_VER).dll $(BINDIR)
	copy mcpp_lib.h $(INCDIR)
	copy mcpp_out.h $(INCDIR)
	$(CC) -e$(NAME).exe main_libmcpp.c mcpp$(DLL_VER).lib
	copy $(NAME).exe $(BINDIR)

mcpplib_uninstall:
	del $(LIBDIR)\mcpp.lib $(LIBDIR)\mcpp$(DLL_VER).lib \
            $(BINDIR)\mcpp$(DLL_VER).dll
	del $(BINDIR)\$(NAME).exe
	del $(INCDIR)\mcpp*
!endif

# use mcpp as a subroutine from testmain.c
!if	$d( DLL_IMPORT)
CFLAGS = $(CFLAGS) -DDLL_IMPORT=1
LINKLIB = mcpp$(DLL_VER).lib
!else
LINKLIB = mcpp.lib
!endif
!if    $d( OUT2MEM)
# output to memory buffer
CFLAGS = $(CFLAGS) -DOUT2MEM=1
!endif
TMAIN_LINKFLAGS = testmain.obj -etestmain.exe $(LINKLIB)
testmain	:	testmain.obj
	$(CC) $(TMAIN_LINKFLAGS)
testmain_install	:
	copy testmain.exe $(BINDIR)
testmain_uninstall	:
	del $(BINDIR)\testmain.exe
