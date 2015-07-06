lib_LTLIBRARIES = cURL.la

cURL_la_SOURCES = Lua-cURL.c Lua-utility.c Lua-cURL-getinfo.c Lua-cURL-setopt.c \
		  Lua-cURL-callback.c Lua-cURL-post.c Lua-cURL-multi.c \
		  Lua-cURL-share.c Lua-cURL.h  Lua-utility.h Lua-cURL-share.h
				
cURL_la_LDFLAGS = @LUA_LIBS@ @CURL_LIBS@ -avoid-version -module
cURL_la_CFLAGS =  @LUA_CFLAGS@ @CURL_CFLAGS@

libdir = @CMOD_INSTALL@




