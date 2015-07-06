# makefile to compile MCPP version 2.7.2 for Linux / GCC / GNU make
#       2008/09 kmatsui
#
# First, you must edit GCCDIR, BINDIR, INCDIR, gcc_maj_ver and gcc_min_ver.
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
# To make libmcpp (subroutine build of mcpp) and mcpp binary linking it:
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

# CC:   name of gcc executable
#       e.g. cc, gcc, gcc-2.95.3, i686-pc-linux-gnu-gcc-4.1.1
CC = gcc
CXX = g++
CFLAGS = -c -O2 -Wall   # -ggdb -v
# for gcc 4.1 or later (Don't use this option to compile libmcpp)
#CFLAGS += -fstack-protector
CPPFLAGS =

LINKFLAGS = -o $(NAME)
# for gcc 4.1 or later (Don't use this option to compile libmcpp)
#LINKFLAGS += -fstack-protector

ifeq    ($(COMPILER), )
# compiler-independent-build
CPPOPTS =
# BINDIR:   directory to install mcpp: /usr/bin or /usr/local/bin
BINDIR = /usr/local/bin
# INCDIR:   directory to install mcpp_lib.h, mcpp_out.h for mcpplib
INCDIR = /usr/local/include

else
# compiler-specific-build:  Adjust for your system

ifeq    ($(COMPILER), GNUC)
# The directory where 'gcc' (cc) command is located
GCCDIR = /usr/bin
#GCCDIR = /usr/local/bin
# set GCC version
gcc_maj_ver = 3
gcc_min_ver = 3
# INCDIR:   GCC's version specific include directory
#INCDIR = /usr/lib/gcc-lib/i386-vine-linux/2.95.3/include
#INCDIR = /usr/local/gcc-3.2/lib/gcc-lib/i686-pc-linux-gnu/3.2/include
# Vine 4.2
INCDIR = /usr/lib/gcc-lib/i386-vine-linux/3.3.6/include
#INCDIR = /usr/local/lib/gcc/i686-pc-linux-gnu/4.1.1/include
# Debian 4.0
#INCDIR = /usr/lib/gcc/i486-linux-gnu/4.1.2/include
CPPOPTS = -DCOMPILER=$(COMPILER)

#BINDIR = /usr/lib/gcc-lib/i386-vine-linux/2.95.3
#BINDIR = /usr/local/gcc-3.2/lib/gcc-lib/i686-pc-linux-gnu/3.2
# Vine 4.2
BINDIR = /usr/lib/gcc-lib/i386-vine-linux/3.3.6
#BINDIR = /usr/local/libexec/gcc/i686-pc-linux-gnu/4.1.1
# Debian 4.0
#BINDIR = /usr/lib/gcc/i486-linux-gnu/4.1.2
cpu = i386
#cpu = ppc
#cpu = x86_64
#cpu = ppc64
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
	$(CC) $(OBJS) $(LINKFLAGS)

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
	$(CC) $(CFLAGS) $(CMACRO) $(CPPFLAGS) $<

install :
	install -s $(NAME) $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./set_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'   \
            '$(cpp_call)' '$(CC)' '$(CXX)' 'x$(CPPFLAGS)' 'x' 'ln -s' \
            '$(INCDIR)' SYS_LINUX $(cpu)
endif

clean	:
	-rm *.o *.so $(NAME) mcpp.H mcpp.err libmcpp.*

uninstall:
	rm -f $(BINDIR)/$(NAME)
ifeq    ($(COMPILER), GNUC)
	@./unset_mcpp.sh '$(GCCDIR)' '$(gcc_maj_ver)' '$(gcc_min_ver)'  \
            '$(cpp_call)' '$(CC)' '$(CXX)' 'x' 'ln -s' '$(INCDIR)' SYS_LINUX
endif

ifeq    ($(COMPILER), )
# the directory to install libmcpp into
LIBDIR = /usr/local/lib
ifeq    ($(MCPP_LIB), 1)
# compiler-independent-build and MCPP_LIB=1
CFLAGS += -DMCPP_LIB

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
	$(CC) $(CFLAGS) $(MEM_MACRO) -c -fpic -o $*.so $*.c
mcpplib_so: $(SOBJS)
	$(CC) -shared -o libmcpp.so.$(SHLIB_VER) $(SOBJS)
	chmod a+x libmcpp.so.$(SHLIB_VER)

mcpplib_install:
	cp libmcpp.a libmcpp.so.$(SHLIB_VER) $(LIBDIR)
	ranlib $(LIBDIR)/libmcpp.a
	ln -sf libmcpp.so.$(SHLIB_VER) $(LIBDIR)/libmcpp.so
	ln -sf libmcpp.so.$(SHLIB_VER) $(LIBDIR)/libmcpp.so.$(CUR)
# You should do 'ldconfig' as a root after install.
	cp mcpp_lib.h mcpp_out.h $(INCDIR)
	$(CC) -o $(NAME) main_libmcpp.c -l $(NAME)
	install -s $(NAME) $(BINDIR)
mcpplib_uninstall:
	rm -f $(LIBDIR)/libmcpp.a $(LIBDIR)/libmcpp.so.$(SHLIB_VER) $(LIBDIR)/libmcpp.so.$(CUR) $(LIBDIR)/libmcpp.so 
	rm -f $(BINDIR)/$(NAME)
	rm -f $(INCDIR)/mcpp*
endif

# use mcpp as a subroutine from testmain.c
ifeq    ($(OUT2MEM), 1)
# output to memory buffer
CFLAGS += -DOUT2MEM
endif
TMAIN_LINKFLAGS = testmain.o -o testmain -L$(LIBDIR) -l$(NAME)
ifeq	($(MALLOC), KMMALLOC)
	TMAIN_LINKFLAGS += -lkmmalloc_debug
endif
testmain	:	testmain.o
	$(CC) $(TMAIN_LINKFLAGS)
testmain_install	:
	install -s testmain $(BINDIR)/testmain
testmain_uninstall	:
	rm -f $(BINDIR)/testmain

endif

