#include "dbd_sqlite3.h"

int dbd_sqlite3_connection(lua_State *L);
int dbd_sqlite3_statement(lua_State *L);

/* 
 * library entry point
 */
LUA_EXPORT int luaopen_dbdsqlite3(lua_State *L) {
    dbd_sqlite3_connection(L);
    dbd_sqlite3_statement(L); 

    return 1;
}

