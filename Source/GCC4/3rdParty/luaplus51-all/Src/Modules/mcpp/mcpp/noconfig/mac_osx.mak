# makefile to compile MCPP version 2.7.2 for Mac OS X / GCC / GNU make
#       2008/09 kmatsui
#
# First, you must edit NAME, GCCDIR, BINDIR, INCDIR, 
#       gcc_maj_ver, gcc_min_ver, arch, target_cc.
# To make compiler-independent-build of MCPP:
#       make
#       sudo make install
# To make GCC-specific-build of MCPP:
#       make COMPILER=GNUC
#       sudo make COMPILER=GNUC install
# To re-compile MCPP using GCC-specific-build of MCPP do:
#       make COMPILER=GNUC PREPROCESSED=1
#       sudo make COMPILER=GNUC PREPROCESSED=1 install
# To link malloc() package of kmatsui do:
#       make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC
#       sudo make [COMPILER=GNUC] [PREPROCESSED=1] MALLOC=KMMALLOC install
# To make libmcpp (subroutine build of mcpp):
#       make MCPP_LIB=1 mcpplib
#    	sudo make MCPP_LIB=1 mcpplib_install
# To make testmain using libmcpp:
#       make [OUT2MEM=1] testmain
#       sudo make [OUT2MEM=1] testmain_install

# COMPILER:
#   Specify whether make a compiler-independent-build or GCC-specific-build
# compiler-independent-build:   empty
# compiler-specific-build:      GNUC

# NAME: name of mcpp executable
NAME = mcpp
#NAME = powerpc-apple-darwin9-mcpp
#NAME = i686-apple-darwin9-mcpp

# CC, CXX:  name of gcc, g++ executable
# CC:   default, GCC-specific-build
# $(arch)-apple-darwin?-gcc-VERSION:
#       cross-compile of compiler-independent-build
CC = gcc
#CC = powerpc-apple-darwin9-gcc-4.0.1
#CC = i686-apple-darwin9-gcc-4.0.1
CXX = g++
#CXX = powerpc-apple-darwin9-g++-4.0.1
#CXX = i686-apple-darwin9-g++-4.0.1
CFLAGS = -c -Wall -O2   # -ggdb -O0 -v
# for gcc 4.1 or later (Don't use this option to compile libmcpp)
#CFLAGS += -fstack-protector
CPPFLAGS =

LINKFLAGS = -o $(NAME)
# for gcc 4.1 or later (Don't use this option to compile libmcpp)
#LINKFLAGS += -fstack-protector  # for gcc 4.1 or later

# for universal binary of MacOS 10.4 and 10.5, x86 and ppc
#UFLAGS = -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -arch i386 -arch ppc

ifeq    ($(COMPILER), )
# compiler-independent-build
CPPOPTS =
# BINDIR:   directory to install mcpp: /usr/bin or /usr/local/bin
BINDIR = /usr/local/bin
# INCDIR:	directory to install mcpp_lib.h, mcpp_out.h for mcpplib
INCDIR = /usr/local/include

else
# compiler-specific-build:  Adjust for your system

ifeq    ($(COMPILER), GNUC)
# The directory where 'gcc' (cc) command is located
GCCDIR = /usr/bin
#GCCDIR = /usr/local/bin
# set GCC version
gcc_maj_ver = 4
gcc_min_ver = 0
# INCDIR:   GCC's version specific include directory
# Mac OS X Leopard
INCDIR = /usr/lib/gcc/i686-apple-darwin9/4.0.1/include
#INCDIR = /usr/lib/gcc/powerpc-apple-darwin9/4.0.1/include
CPPOPTS = -DCOMPILER=$(COMPILER)

# BINDIR:   Where mcpp should be installed
# Mac OS X Leopard
BINDIR = /usr/libexec/gcc/i686-apple-darwin9/4.0.1
#BINDIR = /usr/libexec/gcc/powerpc-apple-darwin9/4.0.1
# target_cc: the compiler into which mcpp is installed
# gcc: default
# $(arch)-apple-darwin?-gcc-VERSION: target is the cross-compiler
target_cc = gcc
#target_cc = powerpc-apple-darwin9-gcc-4.0.1
#target_cc = i686-apple-darwin9-gcc-4.0.1
arch = i386
#arch = ppc
ifeq ($(gcc_maj_ver), 2)
cpp_call = $(BINDIR)/cpp0
else
cpp_call = $(BINDIR)/cc1
endif
endif
endif

ifneq	($(MALLOC), )
ifeq	($(MALLOC), KMMALLOC)
	LINKFLAGS += -lkmmalloc_debug
	MEM_MACRO = -D_MEM_DEBUG -DXMALLOC
endif
	MEM_MACRO += -D$(MALLOC)
else
	MEM_MACRO =
endif

OBJS = main.o directive.o eval.o expand.o support.o system.o mbchar.o

all	:	$(NAME)
$(NAME): $(OBJS)
	$(CC) $(UFLAGS) $(OBJS) $(LINKFLAGS)

PREPROCESSED = 0

ifeq	($(PREPROCESSED), 1)
CMACRO = -DPREPROCESSED
#CPPFLAGS = -Wp,-v,-Q,-W3
    # for MCPP to output a bit verbose diagnosis to "mcpp.err"
# Make a "pre-preprocessed" header file to recompile MCPP with MCPP.
mcpp.H	: system.H noconfig.H internal.H
ifeq    ($(COMPILER), GNUC)
	$(CC) -E -Wp,-b  $(CPPFLAGS) $(CPPOPTS) $(MEM_MACRO) -o mcpp.H preproc.c
else
	@echo "Do 'sudo make COMPILER=GNUC install' prior to recompile."
	@echo "Then, do 'make COMPILER=GNUC PREPROCESSED=1'."
	@exit
endif
$(OBJS) : mcpp.H
else
CMACRO = $(MEM_MACRO) $(CPPOPTS)
$(OBJS) : noconfig.H
main.o directive.o eval.o expand.o support.o system.o mbchar.o:   \
        system.H internal.H
endif

.c.o	:
	$(CC) $(CFLAGS) $(UFLAGS) $(CMACRO) $(CPPFLAGS) $<

install :
	install -s $(NAME) $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(CPPFLAGS)' 'x' 'ln -s'  \
            '$(INCDIR)' SYS_MAC $(arch) $(target_cc)
endif

clean	:
	-rm *.o *.so $(NAME) mcpp.H mcpp.err libmcpp.*

uninstall:
	rm -f $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'  \
            '$(cpp_call)' '$(CC)' '$(CXX)' 'x' 'ln -s' '$(INCDIR)' SYS_MAC  \
            $(target_cc)
endif

ifeq    ($(COMPILER), )
ifeq    ($(MCPP_LIB), 1)
# compiler-independent-build and MCPP_LIB=1
CFLAGS += -DMCPP_LIB
LIBDIR = /usr/local/lib

mcpplib :   mcpplib_a mcpplib_so

mcpplib_a:	$(OBJS)
	ar -rv libmcpp.a $(OBJS)

# shared library
# mcpp 2.6.*: 0, mcpp 2.7: 1, mcpp 2.7.1: 2, mcpp 2.7.2: 3
CUR = 3
# mcpp 2.6.3: 0, mcpp 2.6.4: 1, mcpp 2.7, 2.7.1, 2.7.2: 0
REV = 0
# mcpp 2.6.*: 0, mcpp 2.7: 1, mcpp 2.7.1: 2, mcpp 2.7.2: 3
AGE = 3
SHLIB_VER = 0.$(CUR).$(REV)
SOBJS = main.so directive.so eval.so expand.so support.so system.so mbchar.so

.SUFFIXES: .so
.c.so	:
	$(CC) $(CFLAGS) $(UFLAGS) $(MEM_MACRO) -c -fno-common -o $*.so $*.c
mcpplib_so: $(SOBJS)
	$(CC) $(UFLAGS) -dynamiclib -o libmcpp.$(SHLIB_VER).dylib $(SOBJS)
	chmod a+x libmcpp.$(SHLIB_VER).dylib

mcpplib_install:
	cp libmcpp.a libmcpp.$(SHLIB_VER).dylib $(LIBDIR)
	ranlib $(LIBDIR)/libmcpp.a
	ln -sf libmcpp.$(SHLIB_VER).dylib $(LIBDIR)/libmcpp.dylib
	ln -sf libmcpp.$(SHLIB_VER).dylib $(LIBDIR)/libmcpp.0.dylib
	cp mcpp_lib.h mcpp_out.h $(INCDIR)
# You should do 'ldconfig' as a root after install.
	$(CC) -o $(NAME) main_libmcpp.c -l $(NAME)
	install -s $(NAME) $(BINDIR)
mcpplib_uninstall:
	rm -f $(LIBDIR)/libmcpp.a $(LIBDIR)/libmcpp.$(SHLIB_VER).dylib $(LIBDIR)/libmcpp.0.dylib $(LIBDIR)/libmcpp.dylib
	rm -f $(BINDIR)/$(NAME)
	rm -f $(INCDIR)/mcpp*
endif

# use mcpp as a subroutine from testmain.c
ifeq    ($(OUT2MEM), 1)
# output to memory buffer
CFLAGS += -DOUT2MEM
endif
TMAIN_LINKFLAGS = testmain.o -o testmain -l $(NAME)
ifeq	($(MALLOC), KMMALLOC)
	TMAIN_LINKFLAGS += -l kmmalloc_debug
endif
testmain	:	testmain.o
	$(CC) $(UFLAGS) $(TMAIN_LINKFLAGS)
testmain_install	:
	install -s testmain $(BINDIR)/testmain
testmain_uninstall	:
	rm -f $(BINDIR)/testmain

endif

