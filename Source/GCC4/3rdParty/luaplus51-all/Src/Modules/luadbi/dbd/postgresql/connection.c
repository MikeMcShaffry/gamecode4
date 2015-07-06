#include "dbd_postgresql.h"

int dbd_postgresql_statement_create(lua_State *L, connection_t *conn, const char *sql_query);

static int run(connection_t *conn, const char *command) {
    PGresult *result = PQexec(conn->postgresql, command);
    ExecStatusType status;

    if (!result)
        return 1;

    status = PQresultStatus(result);
    PQclear(result);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        return 1;
 
    return 0;
}

static int commit(connection_t *conn) {
    return run(conn, "COMMIT");
}


static int begin(connection_t *conn) {
    return run(conn, "BEGIN");
}


static int rollback(connection_t *conn) {
    return run(conn, "ROLLBACK");
}


/*
 * connection = DBD.PostgreSQL.New(dbname, user, password, host, port)
 */
static int connection_new(lua_State *L) {
    int n = lua_gettop(L);
    connection_t *conn = NULL;

    const char *host = NULL;
    const char *user = NULL;
    const char *password = NULL;
    const char *db = NULL;
    const char *port = NULL;

    const char *options = NULL; /* TODO always NULL */
    const char *tty = NULL; /* TODO always NULL */

    char portbuf[18];

    /* db, user, password, host, port */
    switch (n) {
    case 5:
	if (lua_isnil(L, 5) == 0) 
	{
	    int pport = luaL_checkint(L, 5);

	    if (pport >= 1 && pport <= 65535) {
		snprintf(portbuf, sizeof(portbuf), "%d", pport);
		port = portbuf;
	    } else {
		luaL_error(L, DBI_ERR_INVALID_PORT, pport);
	    }
	}
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

    conn->postgresql = PQsetdbLogin(host, port, options, tty, db, user, password);
    conn->statement_id = 0;
    conn->autocommit = 0;
    begin(conn);

    if (PQstatus(conn->postgresql) != CONNECTION_OK) {
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, PQerrorMessage(conn->postgresql));
	return 2;
    }

    luaL_getmetatable(L, DBD_POSTGRESQL_CONNECTION);
    lua_setmetatable(L, -2);

    return 1;
}

/*
 * success = connection:autocommit(on)
 */
static int connection_autocommit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);
    int on = lua_toboolean(L, 2); 
    int err = 0;

    if (conn->postgresql) {
	if (on)
	    err = rollback(conn);
	else
	    err = begin(conn);

	conn->autocommit = on;	
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * success = connection:close()
 */
static int connection_close(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);
    int disconnect = 0;   

    if (conn->postgresql) {
	/*
	 * if autocommit is turned off, we probably
	 * want to rollback any outstanding transactions.
	 */
	if (!conn->autocommit)
	    rollback(conn);

	PQfinish(conn->postgresql);
	disconnect = 1;
	conn->postgresql = NULL;
    }

    lua_pushboolean(L, disconnect);
    return 1;
}

/*
 * success = connection:commit()
 */
static int connection_commit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);
    int err = 0;

    if (conn->postgresql) {
	commit(conn);

	if (!conn->autocommit)
	    err = begin(conn);
	else
	    err = 1;
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * ok = connection:ping()
 */
static int connection_ping(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);
    int ok = 0;   

    if (conn->postgresql) {
	ConnStatusType status = PQstatus(conn->postgresql);

	if (status == CONNECTION_OK)
	    ok = 1;
    }

    lua_pushboolean(L, ok);
    return 1;
}

/*
 * statement = connection:prepare(sql_string)
 */
static int connection_prepare(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);

    if (conn->postgresql) {
	return dbd_postgresql_statement_create(L, conn, luaL_checkstring(L, 2));
    }

    lua_pushnil(L);    
    lua_pushstring(L, DBI_ERR_DB_UNAVAILABLE);
    return 2;
}

/*
 * quoted = connection:quote(str)
 */
static int connection_quote(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);
    size_t len;
    const char *from = luaL_checklstring(L, 2, &len);
    char *to = (char *)calloc(len*2+1, sizeof(char));
    int err = 0;
    int quoted_len;

    if (!conn->postgresql) {
        luaL_error(L, DBI_ERR_DB_UNAVAILABLE);
    }

    quoted_len = PQescapeStringConn(conn->postgresql, to, from, len, &err);

    if (err) {
        free(to);
        
       luaL_error(L, DBI_ERR_QUOTING_STR, PQerrorMessage(conn->postgresql));
    }

    lua_pushlstring(L, to, quoted_len);
    free(to);

    return 1;
}

/*
 * success = connection:rollback()
 */
static int connection_rollback(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);
    int err = 0;

    if (conn->postgresql) {
	rollback(conn);

	if (!conn->autocommit)
	    err = begin(conn);
	else
	    err = 1;
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
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_CONNECTION);

    lua_pushfstring(L, "%s: %p", DBD_POSTGRESQL_CONNECTION, conn);

    return 1;
}

int dbd_postgresql_connection(lua_State *L) {
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

    luaL_newmetatable(L, DBD_POSTGRESQL_CONNECTION);
    luaL_register(L, 0, connection_methods);
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, connection_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, connection_tostring);
    lua_setfield(L, -2, "__tostring");

    luaL_register(L, DBD_POSTGRESQL_CONNECTION, connection_class_methods);

    return 1;    
}
