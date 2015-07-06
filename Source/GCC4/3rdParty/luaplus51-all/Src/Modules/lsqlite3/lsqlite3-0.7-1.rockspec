package = "lsqlite3"
version = "0.7-1"
source = {
    url = "http://luaforge.net/frs/download.php/2565/lsqlite3-7.tar.gz"
}
description = {
    summary = "A binding for Lua to the SQLite3 database library",
    detailed = [[
        lsqlite3 is a thin wrapper around the public domain SQLite3 database engine. 
        The lsqlite3 module supports the creation and manipulation of SQLite3 databases. 
        After a require('lsqlite3') the exported functions are called with prefix sqlite3. 
        However, most sqlite3 functions are called via an object-oriented interface to 
        either database or SQL statement objects.
    ]],
    license = "MIT/X11",
    homepage = "http://luaforge.net/projects/luasqlite/"
}
dependencies = {
    "lua >= 5.1"
}
external_dependencies = {
    SQLITE = {
        header = "sqlite3.h"
    }
}
build = {
    type = "builtin",
    modules = {
        lsqlite3 = {
            sources = { "lsqlite3.c" },
            libraries = { "sqlite3" },
            incdirs = { "$(SQLITE_INCDIR)" },
            libdirs = { "$(SQLITE_LIBDIR)" }
        }
    },
	copy_directories = { 'doc', 'examples' }
}
