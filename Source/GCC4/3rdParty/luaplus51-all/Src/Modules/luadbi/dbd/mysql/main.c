#include "dbd_mysql.h"

int dbd_mysql_connection(lua_State *L);
int dbd_mysql_statement(lua_State *L);

/*
 * libabry entry point
 */
LUA_EXPORT int luaopen_dbdmysql(lua_State *L) {
    dbd_mysql_connection(L);
    dbd_mysql_statement(L);

    return 1;
}

