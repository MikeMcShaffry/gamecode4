CC=cc
CFLAGS=-W -Wall -ansi -O2 -fpic -shared
INC=-I/usr/include/lua5.1

mixlua.so: mixlua.c
	@echo "CC mixlua.so"
	@${CC} ${CFLAGS} ${INC} -o mixlua.so mixlua.c

install:
	@echo "CP mixlua.so"
	@cp mixlua.so /usr/lib/lua/5.1

clean:
	@echo "RM mixlua.so"
	@rm -f mixlua.so

.PHONY: install clean

