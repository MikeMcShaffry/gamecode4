#include "dbd_db2.h"

int dbd_db2_connection(lua_State *L);
int dbd_db2_statement(lua_State *L);

/*
 * library entry point
 */
LUA_EXPORT int luaopen_dbddb2(lua_State *L) {
    dbd_db2_connection(L);
    dbd_db2_statement(L); 

    return 1;
}

