#include "dbd_oracle.h"

int dbd_oracle_statement_create(lua_State *L, connection_t *conn, const char *sql_query);

static int commit(connection_t *conn) {
    int rc = OCITransCommit(conn->svc, conn->err, OCI_DEFAULT);
    return rc;
}

static int rollback(connection_t *conn) {
    int rc = OCITransRollback(conn->svc, conn->err, OCI_DEFAULT);
    return rc;
}


/* 
 * connection,err = DBD.Oracle.New(dbfile)
 */
static int connection_new(lua_State *L) {
    int n = lua_gettop(L);

    int rc = 0;

    const char *user = NULL;
    const char *password = NULL;
    const char *db = NULL;

    OCIEnv *env = NULL;
    OCIError *err = NULL;
    OCISvcCtx *svc = NULL;

    connection_t *conn = NULL;

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

    /*
     * initialise OCI
     */
    OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0, (dvoid * (*)(dvoid *, size_t))0, (dvoid * (*)(dvoid *, dvoid *, size_t))0, (void (*)(dvoid *, dvoid *))0);

    /*
     * initialise environment
     */
    OCIEnvInit((OCIEnv **)&env, OCI_DEFAULT, 0, (dvoid **)0);

    /* 
     * server contexts 
     */
    OCIHandleAlloc((dvoid *)env, (dvoid **)&err, OCI_HTYPE_ERROR, 0, (dvoid **)0);
    OCIHandleAlloc((dvoid *)env, (dvoid **)&svc, OCI_HTYPE_SVCCTX, 0, (dvoid **)0);

    /*
     * connect to database server
     */
    rc = OCILogon(env, err, &svc, user, strlen(user), password, strlen(password), db, strlen(db));
    if (rc != 0) {
	char errbuf[100];
	int errcode;

	OCIErrorGet((dvoid *)err, (ub4) 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
	
	lua_pushnil(L);
	lua_pushfstring(L, DBI_ERR_CONNECTION_FAILED, errbuf);

	return 2;
    }

    conn = (connection_t *)lua_newuserdata(L, sizeof(connection_t));
    conn->oracle = env;
    conn->err = err;
    conn->svc = svc;
    conn->autocommit = 0;

    luaL_getmetatable(L, DBD_ORACLE_CONNECTION);
    lua_setmetatable(L, -2);

    return 1;
}

/*
 * success = connection:autocommit(on)
 */
static int connection_autocommit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_ORACLE_CONNECTION);
    int on = lua_toboolean(L, 2); 
    int err = 1;

    if (conn->oracle) {
	if (on)
	    rollback(conn);

	conn->autocommit = on;
	err = 0;
    }

    lua_pushboolean(L, !err);
    return 1;
}


/*
 * success = connection:close()
 */
static int connection_close(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_ORACLE_CONNECTION);
    int disconnect = 0;   

    if (conn->oracle) {
	rollback(conn);

	OCILogoff(conn->svc, conn->err);
	
	if (conn->svc)
	    OCIHandleFree((dvoid *)conn->svc, OCI_HTYPE_ENV);
        if (conn->err)
            OCIHandleFree((dvoid *)conn->err, OCI_HTYPE_ERROR);

	disconnect = 1;
	conn->oracle = NULL;
    }

    lua_pushboolean(L, disconnect);
    return 1;
}

/*
 * success = connection:commit()
 */
static int connection_commit(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_ORACLE_CONNECTION);
    int err = 1;

    if (conn->oracle) {
	err = commit(conn);
    }

    lua_pushboolean(L, !err);
    return 1;
}

/*
 * ok = connection:ping()
 */
static int connection_ping(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_ORACLE_CONNECTION);
    int ok = 0;   

    if (conn->oracle) {
	ok = 1;
    }

    lua_pushboolean(L, ok);
    return 1;
}

/*
 * statement,err = connection:prepare(sql_str)
 */
static int connection_prepare(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_ORACLE_CONNECTION);

    if (conn->oracle) {
	return dbd_oracle_statement_create(L, conn, luaL_checkstring(L, 2));
    }

    lua_pushnil(L);    
    lua_pushstring(L, DBI_ERR_DB_UNAVAILABLE);
    return 2;
}

/*
 * quoted = connection:quote(str)
 */
static int connection_quote(lua_State *L) {
    luaL_error(L, DBI_ERR_NOT_IMPLEMENTED, DBD_ORACLE_CONNECTION, "quote");
    return 0;
}

/*
 * success = connection:rollback()
 */
static int connection_rollback(lua_State *L) {
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_ORACLE_CONNECTION);
    int err = 1;

    if (conn->oracle) {
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
    connection_t *conn = (connection_t *)luaL_checkudata(L, 1, DBD_ORACLE_CONNECTION);

    lua_pushfstring(L, "%s: %p", DBD_ORACLE_CONNECTION, conn);

    return 1;
}

int dbd_oracle_connection(lua_State *L) {
    /*
     * instance methods
     */
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

    /*
     * class methods
     */
    static const luaL_Reg connection_class_methods[] = {
	{"New", connection_new},
	{NULL, NULL}
    };

    luaL_newmetatable(L, DBD_ORACLE_CONNECTION);
    luaL_register(L, 0, connection_methods);
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, connection_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, connection_tostring);
    lua_setfield(L, -2, "__tostring");

    luaL_register(L, DBD_ORACLE_CONNECTION, connection_class_methods);

    return 1;    
}
