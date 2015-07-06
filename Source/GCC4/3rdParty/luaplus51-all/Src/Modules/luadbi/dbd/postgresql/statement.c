#include "dbd_postgresql.h"

#define BOOLOID                 16
#define INT2OID                 21
#define INT4OID                 23
#define INT8OID                 20
#define FLOAT4OID		700
#define FLOAT8OID		701

static lua_push_type_t postgresql_to_lua_push(unsigned int postgresql_type) {
    lua_push_type_t lua_type;

    switch(postgresql_type) {
    case INT2OID:
    case INT4OID:
    case INT8OID:
        lua_type =  LUA_PUSH_INTEGER;
        break;

    case FLOAT4OID:
    case FLOAT8OID:
        lua_type = LUA_PUSH_NUMBER;
        break;

    case BOOLOID:
	    lua_type = LUA_PUSH_BOOLEAN;
	    break;

    default:
        lua_type = LUA_PUSH_STRING;
    }

    return lua_type;
}

static int deallocate(statement_t *statement) {
    char command[IDLEN+13];
	PGresult *result;
	ExecStatusType status;

	snprintf(command, IDLEN+13, "DEALLOCATE \"%s\"", statement->name);    
    result = PQexec(statement->postgresql, command);

    if (!result)
        return 1;

    status = PQresultStatus(result);
    PQclear(result);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        return 1;

    return 0;
}


/*
 * num_affected_rows = statement:affected()
 */
static int statement_affected(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_STATEMENT);

    if (!statement->result) {
        luaL_error(L, DBI_ERR_INVALID_STATEMENT);
    }

    lua_pushinteger(L, atoi(PQcmdTuples(statement->result)));

    return 1;
}

/*
 * success = statement:close()
 */
static int statement_close(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_STATEMENT);

    if (statement->result) {
        /*
         * Deallocate prepared statement on the
         * server side
         */ 
        deallocate(statement); 

	PQclear(statement->result);
	statement->result = NULL;
    }

    return 0;    
}

/*
 * column_names = statement:columns()
 */
static int statement_columns(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_STATEMENT);

    int i;
    int num_columns;
    int d = 1;

    if (!statement->result) {
        luaL_error(L, DBI_ERR_INVALID_STATEMENT);
        return 0;
    }

    num_columns = PQnfields(statement->result);
    lua_newtable(L);
    for (i = 0; i < num_columns; i++) {
        const char *name = PQfname(statement->result, i);

	LUA_PUSH_ARRAY_STRING(d, name);
    }

    return 1;
}

/*
 * success = statement:execute(...)
 */
static int statement_execute(lua_State *L) {
    int n = lua_gettop(L);
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_STATEMENT);
    int num_bind_params = n - 1;   
    ExecStatusType status;
    int p;
    const char *errstr = NULL;

    const char **params;
    PGresult *result = NULL;

    statement->tuple = 0;

    params = malloc(num_bind_params * sizeof(params));
    memset(params, 0, num_bind_params * sizeof(params));

    /*
     * convert and copy parameters into a string array
     */ 
    for (p = 2; p <= n; p++) {
        int i = p - 2;	
        int type = lua_type(L, p);
        char err[64];

        switch(type) {
        case LUA_TNIL:
            params[i] = NULL;
            break;
        case LUA_TBOOLEAN:
            /*
             * boolean values in postgresql can either be
             * t/f or 1/0. Pass integer values rather than
             * strings to maintain semantic compatibility
             * with other DBD drivers that pass booleans
             * as integers.
             */
            params[i] = lua_toboolean(L, p) ?  "1" : "0";
            break;
        case LUA_TNUMBER:
        case LUA_TSTRING:
            params[i] = lua_tostring(L, p);
            break;
        default:
            snprintf(err, sizeof(err)-1, DBI_ERR_BINDING_TYPE_ERR, lua_typename(L, type));
            errstr = err;
            goto cleanup;
        }
    }

    result = PQexecPrepared(
        statement->postgresql,
        statement->name,
        num_bind_params,
        (const char **)params,
        NULL,
        NULL,
        0
    );

cleanup:
    free(params);

    if (errstr) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, DBI_ERR_BINDING_PARAMS, errstr);
        return 2;
    }

    if (!result) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, DBI_ERR_ALLOC_RESULT,  PQerrorMessage(statement->postgresql));
        return 2;
    }
    
    status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        lua_pushboolean(L, 0);
        lua_pushfstring(L, DBI_ERR_BINDING_EXEC, PQresultErrorMessage(result));
        return 2;
    }
    
    statement->result = result;

    lua_pushboolean(L, 1);
    return 1;
}

/*
 * can only be called after an execute
 */
static int statement_fetch_impl(lua_State *L, statement_t *statement, int named_columns) {
    int tuple = statement->tuple++;
    int i;
    int num_columns;
    int d = 1;

    if (!statement->result) {
        luaL_error(L, DBI_ERR_FETCH_INVALID);
        return 0;
    }

    if (PQresultStatus(statement->result) != PGRES_TUPLES_OK) {
        lua_pushnil(L);
        return 1;
    }

    if (tuple >= PQntuples(statement->result)) {
        lua_pushnil(L);  /* no more results */
        return 1;
    }

    num_columns = PQnfields(statement->result);
    lua_newtable(L);
    for (i = 0; i < num_columns; i++) {
        const char *name = PQfname(statement->result, i);

        if (PQgetisnull(statement->result, tuple, i)) {
            if (named_columns) {
                LUA_PUSH_ATTRIB_NIL(name);
            } else {
                LUA_PUSH_ARRAY_NIL(d);
            }	    
        } else {
            const char *value = PQgetvalue(statement->result, tuple, i);
            lua_push_type_t lua_push = postgresql_to_lua_push(PQftype(statement->result, i));

            /*
             * data is returned as strings from PSQL
             * convert them here into Lua types
             */ 

            if (lua_push == LUA_PUSH_NIL) {
                if (named_columns) {
                    LUA_PUSH_ATTRIB_NIL(name);
                } else {
                    LUA_PUSH_ARRAY_NIL(d);
                }
            } else if (lua_push == LUA_PUSH_INTEGER) {
                int val = atoi(value);

                if (named_columns) {
                    LUA_PUSH_ATTRIB_INT(name, val);
                } else {
                    LUA_PUSH_ARRAY_INT(d, val);
                }
            } else if (lua_push == LUA_PUSH_NUMBER) {
                double val = strtod(value, NULL);

                if (named_columns) {
                    LUA_PUSH_ATTRIB_FLOAT(name, val);
                } else {
                    LUA_PUSH_ARRAY_FLOAT(d, val);
                }
            } else if (lua_push == LUA_PUSH_STRING) {
                if (named_columns) {
                    LUA_PUSH_ATTRIB_STRING(name, value);
                } else {
                    LUA_PUSH_ARRAY_STRING(d, value);
                }
            } else if (lua_push == LUA_PUSH_BOOLEAN) {
                /* 
                 * booleans are returned as a string
                 * either 't' or 'f'
                 */
                int val = value[0] == 't' ? 1 : 0;

                if (named_columns) {
                    LUA_PUSH_ATTRIB_BOOL(name, val);
                } else {
                    LUA_PUSH_ARRAY_BOOL(d, val);
                }
            } else {
                luaL_error(L, DBI_ERR_UNKNOWN_PUSH);
            }
        }
    }

    return 1;    
}


static int next_iterator(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, lua_upvalueindex(1), DBD_POSTGRESQL_STATEMENT);
    int named_columns = lua_toboolean(L, lua_upvalueindex(2));

    return statement_fetch_impl(L, statement, named_columns);
}

/*
 * table = statement:fetch(named_indexes)
 */
static int statement_fetch(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_STATEMENT);
    int named_columns = lua_toboolean(L, 2);

    return statement_fetch_impl(L, statement, named_columns);
}

/*
 * num_rows = statement:rowcount()
 */
static int statement_rowcount(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_STATEMENT);

    if (!statement->result) {
        luaL_error(L, DBI_ERR_INVALID_STATEMENT);
    }

    lua_pushinteger(L, PQntuples(statement->result));

    return 1;
}

/*
 * iterfunc = statement:rows(named_indexes)
 */
static int statement_rows(lua_State *L) {
    if (lua_gettop(L) == 1) {
        lua_pushvalue(L, 1);
        lua_pushboolean(L, 0);
    } else {
        lua_pushvalue(L, 1);
        lua_pushboolean(L, lua_toboolean(L, 2));
    }

    lua_pushcclosure(L, next_iterator, 2);
    return 1;
}

/*
 * __gc
 */
static int statement_gc(lua_State *L) {
    /* always free the handle */
    statement_close(L);

    return 0;
}

/*
 * __tostring
 */
static int statement_tostring(lua_State *L) {
    statement_t *statement = (statement_t *)luaL_checkudata(L, 1, DBD_POSTGRESQL_STATEMENT);

    lua_pushfstring(L, "%s: %p", DBD_POSTGRESQL_STATEMENT, statement);

    return 1;
}

int dbd_postgresql_statement_create(lua_State *L, connection_t *conn, const char *sql_query) { 
    statement_t *statement = NULL;
    ExecStatusType status;
    PGresult *result = NULL;
    char *new_sql;
    char name[IDLEN];

    /*
     * convert SQL string into a PSQL API compatible SQL statement
     */ 
    new_sql = replace_placeholders(L, '$', sql_query);

    snprintf(name, IDLEN, "dbd-postgresql-%017u", ++conn->statement_id);

    result = PQprepare(conn->postgresql, name, new_sql, 0, NULL);

    /*
     * free converted statement after use
     */
    free(new_sql);

    if (!result) {
        lua_pushnil(L);
        lua_pushfstring(L, DBI_ERR_ALLOC_STATEMENT, PQerrorMessage(statement->postgresql));
        return 2;
    }
    
    status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        const char *err_string = PQresultErrorMessage(result);
        PQclear(result);

        lua_pushnil(L);
        lua_pushfstring(L, DBI_ERR_PREP_STATEMENT, err_string);
        return 2;
    }

    PQclear(result);

    statement = (statement_t *)lua_newuserdata(L, sizeof(statement_t));
    statement->postgresql = conn->postgresql;
    statement->result = NULL;
    statement->tuple = 0;
    strncpy(statement->name, name, IDLEN-1);
    statement->name[IDLEN-1] = '\0';

    luaL_getmetatable(L, DBD_POSTGRESQL_STATEMENT);
    lua_setmetatable(L, -2);

    return 1;
} 

int dbd_postgresql_statement(lua_State *L) {
    static const luaL_Reg statement_methods[] = {
        {"affected", statement_affected},
        {"close", statement_close},
        {"columns", statement_columns},
        {"execute", statement_execute},
        {"fetch", statement_fetch},
        {"rowcount", statement_rowcount},
        {"rows", statement_rows},
        {NULL, NULL}
    };

    static const luaL_Reg statement_class_methods[] = {
        {NULL, NULL}
    };

    luaL_newmetatable(L, DBD_POSTGRESQL_STATEMENT);
    luaL_register(L, 0, statement_methods);
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, statement_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, statement_tostring);
    lua_setfield(L, -2, "__tostring");

    luaL_register(L, DBD_POSTGRESQL_STATEMENT, statement_class_methods);

    return 1;    
}
