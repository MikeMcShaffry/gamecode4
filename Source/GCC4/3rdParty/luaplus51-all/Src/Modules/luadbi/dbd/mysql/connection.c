#include "dbd_mysql.h"

int dbd_mysql_statement_create(lua_State *L, connection_t *conn, const char *sql_query);

/*
 * connection,err = DBD.MySQl.New(dbname, user, password, host, port)
 */
static int connection_new(lua_State *L) {
    int n = lua_gettop(L);

    connection_t *conn = NULL;

    const char *host = NULL;
    const char *user = NULL;
    const char *password = NULL;
    const char *db = NULL;
    int port = 0;

    const char *unix_socket = NULL; /* TODO always NULL */
    int client_flag = 0; /* TODO always 0, set flags from options table */

    /* db, user, password, host, port */
    switch (n) {
    case 5:
	if (lua_isnil(L, 5) == 0) 
	    port = luaL_checkint(L, 5);
    case 4: 
	if (lua_isnil(L, 4) == 0) 
	    host = luaL_checkstring(L, 4);
    case 3:
	if (lua_isnil(L, 3) == 0) 
	    password = luaL_checkstring(L, 3);
    case 2:
	if (lua_isnil(L, 2) == 0) 
	    user = luaL_checkstring(L, 2);
    case 1:
	/*
	 * db is the only mandatory parameter
	 */
	db = luaL_checkstring(L, 1);
    }

    conn = (connection_t *)lua_newuserdata(L, sizeof(connection_t));

    conn->mysql = mysql_init(NULL);

    if (!mysql_real_connect(conn->mysql, host, user, password, db, port, unix_socket, client_flag)) {
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, mysql_error(conn->mysql));
	return 2;
    }

    /*
     * by default turn off autocommit
     */
    mysql_autocommit(conn->mysql, 0);

    luaL_getmetatable(L, DBD_MYSQL_CONNECTION);
    lua_setmetatable(L, -2);

    return 1;
}

/*
 * success = connection:autocommit(on)
 */
static int connection_autocommit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);
    int on = lua_toboolean(L, 2); 
    int err = 0;

    if (conn->mysql) {
	err = mysql_autocommit(conn->mysql, on);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * success = connection:close()
 */
static int connection_close(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);
    int disconnect = 0;   

    if (conn->mysql) {
	mysql_close(conn->mysql);
	disconnect = 1;
	conn->mysql = NULL;
    }

    lua_pushboolean(L, disconnect);
    return 1;
}

/*
 * success = connection:commit()
 */
static int connection_commit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);
    int err = 0;

    if (conn->mysql) {
	err = mysql_commit(conn->mysql);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/* 
 * ok = connection:ping()
 */
static int connection_ping(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);
    int err = 1;   

    if (conn->mysql) {
	err = mysql_ping(conn->mysql);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * statement,err = connection:prepare(sql_string)
 */
static int connection_prepare(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);

    if (conn->mysql) {
	return dbd_mysql_statement_create(L, conn, luaL_checkstring(L, 2)); 
    }

    lua_pushnil(L);    
    lua_pushstring(L, DBI_ERR_DB_UNAVAILABLE);    

    return 2;
}

/*
 * quoted = connection:quote(str)
 */
static int connection_quote(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);
    size_t len;
    const char *from = luaL_checklstring(L, 2, &len);
    char *to = (char *)calloc(len*2+1, sizeof(char));
    int quoted_len;

    if (!conn->mysql) {
        luaL_error(L, DBI_ERR_DB_UNAVAILABLE);
    }

    quoted_len = mysql_real_escape_string(conn->mysql, to, from, len);

    lua_pushlstring(L, to, quoted_len);
    free(to);

    return 1;
}

/*
 * success = connection:rollback()
 */
static int connection_rollback(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);
    int err = 0;

    if (conn->mysql) {
	err = mysql_rollback(conn->mysql);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * __gc
 */
static int connection_gc(lua_State *L) {
    /* always close the connection */
    connection_close(L);

    return 0;
}

/*
 * __tostring
 */
static int connection_tostring(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_MYSQL_CONNECTION);

    lua_pushfstring(L, "%s: %p", DBD_MYSQL_CONNECTION, conn);

    return 1;
}

int dbd_mysql_connection(lua_State *L) {
    static const luaL_Reg connection_methods[] = {
	{"autocommit", connection_autocommit},
	{"close", connection_close},
	{"commit", connection_commit},
	{"ping", connection_ping},
	{"prepare", connection_prepare},
	{"quote", connection_quote},
	{"rollback", connection_rollback},
	{NULL, NULL}
    };

    static const luaL_Reg connection_class_methods[] = {
	{"New", connection_new},
	{NULL, NULL}
    };

    luaL_newmetatable(L, DBD_MYSQL_CONNECTION);
    luaL_register(L, 0, connection_methods);
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, connection_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, connection_tostring);
    lua_setfield(L, -2, "__tostring");

    luaL_register(L, DBD_MYSQL_CONNECTION, connection_class_methods);

    return 1;    
}

