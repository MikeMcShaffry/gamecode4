default:; @cat INSTALL
all: mingw cygwin

mingw:; $(MAKE) -C w32api ex.dll
cygwin:; $(MAKE) -C posix T=ex.dll ex.dll
linux:; $(MAKE) -C posix ex.so

clean:
	$(MAKE) -C posix clean
	$(MAKE) -C w32api clean

