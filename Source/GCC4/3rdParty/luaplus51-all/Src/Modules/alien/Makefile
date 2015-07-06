
all: src/alien/core.so src/alien/struct.so tests/libalientest$(LIB_EXT)

src/alien/core.o: src/alien/core.c libffi/include/ffi.h
	$(CC) -c $(CFLAGS) -Ilibffi/include -o src/alien/core.o src/alien/core.c

libffi/include/ffi.h:
	cat executables | xargs chmod +x
	cd libffi && ./configure --disable-shared CC=gcc

libffi/Makefile:
	cat executables | xargs chmod +x
	cd libffi && ./configure --disable-shared CC=gcc

libffi/.libs/libffi.a: libffi/Makefile
	cd libffi && make CC=gcc

src/alien/core.so: src/alien/core.o libffi/.libs/libffi.a
	export MACOSX_DEPLOYMENT_TARGET=10.3; $(CC) $(LIB_OPTION) -o src/alien/core.so src/alien/core.o -lgcc -Llibffi/.libs -lffi

src/alien/struct.so: src/alien/struct.o 
	export MACOSX_DEPLOYMENT_TARGET=10.3; $(CC) $(LIB_OPTION) -o src/alien/struct.so src/alien/struct.o

install: src/alien/core.so src/alien/struct.so
	mkdir -p $(LUA_LIBDIR)/alien
	cp src/alien/core.so $(LUA_LIBDIR)/alien
	mkdir -p $(LUA_LIBDIR)/alien
	cp src/alien/struct.so $(LUA_LIBDIR)/alien
	cp src/alien.lua $(LUA_DIR)/
	chmod +x src/constants
	cp src/constants $(BIN_DIR)/
	cp -r tests $(PREFIX)/
	cp -r samples $(PREFIX)/
	cp -r doc $(PREFIX)/

clean:
	find . -name "*.so" -o -name "*.o" -o -name "*.dylib" | xargs rm -f

upload-cvs:
	git archive --output alien-current.tar.gz master
	ncftpput -u mascarenhas ftp.luaforge.net alien/htdocs alien-current.tar.gz

upload-dist:
	git archive --format=tar --prefix=alien-$(VERSION)/ $(VERSION) | gzip > alien-$(VERSION).tar.gz
	ncftpput -u mascarenhas ftp.luaforge.net alien/htdocs alien-$(VERSION).tar.gz
	ncftpput -u mascarenhas ftp.luaforge.net alien/htdocs doc/index.html

tests/libalientest$(LIB_EXT): tests/alientest.c
	$(CC) $(LIB_OPTION) $(CFLAGS) -o tests/libalientest$(LIB_EXT) tests/alientest.c

test:
	cd tests && lua -l luarocks.require test_alien.lua
