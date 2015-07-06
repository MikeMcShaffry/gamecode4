#include "dbd_db2.h"

int dbd_db2_statement_create(lua_State *L, connection_t *conn, const char *sql_query);

static int commit(connection_t *conn) {
    SQLRETURN rc = SQL_SUCCESS;

    rc = SQLEndTran(SQL_HANDLE_DBC, conn->db2, SQL_COMMIT);

    return rc != SQL_SUCCESS;
}

static int rollback(connection_t *conn) {
    SQLRETURN rc = SQL_SUCCESS;

    rc = SQLEndTran(SQL_HANDLE_DBC, conn->db2, SQL_ROLLBACK);

    return rc != SQL_SUCCESS;
}


/*
 * connection = DBD.DB2.New(dbname, user, password, host, port)
 */
static int connection_new(lua_State *L) {
    int n = lua_gettop(L);
    connection_t *conn = NULL;
    SQLRETURN rc = SQL_SUCCESS;

    const char *user = NULL;
    const char *password = NULL;
    const char *db = NULL;

    SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];
    SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
    SQLINTEGER sqlcode;
    SQLSMALLINT length;

    /* db, user, password */
    switch(n) {
    case 5:
    case 4:
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

    /* allocate an environment handle */
    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &conn->env);
    if (rc != SQL_SUCCESS) {
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, "Cannot allocate environment handle");
	return 2;
    }

    /* set attribute to enable application to run as ODBC 3.0 application */
    rc = SQLSetEnvAttr(conn->env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
    if (rc != SQL_SUCCESS) {
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, "Cannot set ODBC version attribute");
	return 2;
    }

    /* allocate a database connection handle */
    rc = SQLAllocHandle(SQL_HANDLE_DBC, conn->env, &conn->db2);
    if (rc != SQL_SUCCESS) {
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, "Cannot allocate database handle");
	return 2;
    }

    /* set AUTOCOMMIT off */
    rc = SQLSetConnectAttr(conn->db2, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_NTS);
    if (rc != SQL_SUCCESS) {
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, "Cannot turn off autocommit");
	return 2;
    }
    
    /* connect to the database */
    rc = SQLConnect(conn->db2, (SQLCHAR *)db, SQL_NTS, (SQLCHAR *)user, SQL_NTS, (SQLCHAR *)password, SQL_NTS);
    if (rc != SQL_SUCCESS) {
	SQLGetDiagRec(SQL_HANDLE_DBC, conn->db2, 1, sqlstate, &sqlcode, message, SQL_MAX_MESSAGE_LENGTH + 1, &length);

	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, message);
	return 2;
    }

    luaL_getmetatable(L, DBD_DB2_CONNECTION);
    lua_setmetatable(L, -2);

    return 1;
}

/*
 * success = connection:autocommit(on)
 */
static int connection_autocommit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_DB2_CONNECTION);
    int on = lua_toboolean(L, 2); 
    int err = 0;

    if (conn->db2) {
	int onval = on ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;

	SQLRETURN rc = SQLSetConnectAttr(conn->db2, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)onval, SQL_NTS);

	err = rc != SQL_SUCCESS;
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * success = connection:close()
 */
static int connection_close(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_DB2_CONNECTION);
    int disconnect = 0;   
    SQLRETURN rc = SQL_SUCCESS;

    if (conn->db2) {
	rollback(conn);

	/* disconnect from the database */
	rc = SQLDisconnect(conn->db2);

	/* free connection handle */
	rc = SQLFreeHandle(SQL_HANDLE_DBC, conn->db2);

	conn->db2 = 0;
    }

    if (conn->env) {
	/* free environment handle */
	rc = SQLFreeHandle(SQL_HANDLE_ENV, conn->env);
    }

    lua_pushboolean(L, disconnect);
    return 1;
}

/*
 * success = connection:commit()
 */
static int connection_commit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_DB2_CONNECTION);
    int err = 1;

    if (conn->db2) {
	err = commit(conn);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * ok = connection:ping()
 */
static int connection_ping(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_DB2_CONNECTION);
    int ok = 0;   

    if (conn->db2) {
	ok = 1;
    }

    lua_pushboolean(L, ok);
    return 1;
}

/*
 * statement = connection:prepare(sql_string)
 */
static int connection_prepare(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_DB2_CONNECTION);

    if (conn->db2) {
	return dbd_db2_statement_create(L, conn, luaL_checkstring(L, 2));
    }

    lua_pushnil(L);    
    lua_pushstring(L, DBI_ERR_DB_UNAVAILABLE);
    return 2;
}

/*
 * quoted = connection:quote(str)
 */
static int connection_quote(lua_State *L) {
    luaL_error(L, DBI_ERR_NOT_IMPLEMENTED, DBD_DB2_CONNECTION, "quote");
    return 0;
}

/*
 * success = connection:rollback()
 */
static int connection_rollback(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_DB2_CONNECTION);
    int err = 1;

    if (conn->db2) {
	err = rollback(conn);
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
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_DB2_CONNECTION);
    
    lua_pushfstring(L, "%s: %p", DBD_DB2_CONNECTION, conn);

    return 1;
}

int dbd_db2_connection(lua_State *L) {
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

    luaL_newmetatable(L, DBD_DB2_CONNECTION);
    luaL_register(L, 0, connection_methods);
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, connection_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, connection_tostring);
    lua_setfield(L, -2, "__tostring");

    luaL_register(L, DBD_DB2_CONNECTION, connection_class_methods);

    return 1;    
}
