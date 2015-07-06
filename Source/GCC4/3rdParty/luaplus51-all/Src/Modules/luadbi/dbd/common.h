#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if !defined(LUA_VERSION_NUM) || (LUA_VERSION_NUM < 501)
#include <compat-5.1.h>
#endif

#ifdef _WIN32
    #define LUA_EXPORT __declspec(dllexport)
#else
    #define LUA_EXPORT
#endif

#ifdef _MSC_VER  /* all MS compilers define this (version) */
     #define snprintf _snprintf
#endif

/*
 *
 * Table construction helper functions
 *
 * LUA_PUSH_ATTRIB_* creates string indexed (hashmap)
 * LUA_PUSH_ARRAY_* creates integer indexed (array)
 *
 */

#define LUA_PUSH_ATTRIB_INT(n, v) \
    lua_pushstring(L, n); \
    lua_pushinteger(L, v); \
    lua_rawset(L, -3); 

#define LUA_PUSH_ATTRIB_FLOAT(n, v) \
    lua_pushstring(L, n); \
    lua_pushnumber(L, v); \
    lua_rawset(L, -3); 

#define LUA_PUSH_ATTRIB_STRING(n, v) \
    lua_pushstring(L, n); \
    lua_pushstring(L, v); \
    lua_rawset(L, -3); 

#define LUA_PUSH_ATTRIB_BOOL(n, v) \
    lua_pushstring(L, n); \
    lua_pushboolean(L, v); \
    lua_rawset(L, -3); 

#define LUA_PUSH_ATTRIB_NIL(n) \
    lua_pushstring(L, n); \
    lua_pushnil(L); \
    lua_rawset(L, -3); 



#define LUA_PUSH_ARRAY_INT(n, v) \
    lua_pushinteger(L, v); \
    lua_rawseti(L, -2, n); \
    n++; 

#define LUA_PUSH_ARRAY_FLOAT(n, v) \
    lua_pushnumber(L, v); \
    lua_rawseti(L, -2, n); \
    n++; 

#define LUA_PUSH_ARRAY_STRING(n, v) \
    lua_pushstring(L, v); \
    lua_rawseti(L, -2, n); \
    n++;

#define LUA_PUSH_ARRAY_BOOL(n, v) \
    lua_pushboolean(L, v); \
    lua_rawseti(L, -2, n); \
    n++;

#define LUA_PUSH_ARRAY_NIL(n) \
    lua_pushnil(L); \
    lua_rawseti(L, -2, n); \
    n++;

/*
 *
 * Describes SQL to Lua API type conversions
 *
 */

typedef enum lua_push_type {
    LUA_PUSH_NIL = 0,
    LUA_PUSH_INTEGER,
    LUA_PUSH_NUMBER,
    LUA_PUSH_STRING,
    LUA_PUSH_BOOLEAN,

    LUA_PUSH_MAX
} lua_push_type_t;

/*
 * used for placeholder translations
 * from '?' to the .\d{4}
 */
#define MAX_PLACEHOLDERS        9999 
#define MAX_PLACEHOLDER_SIZE    (1+4) /* .\d{4} */

/*
 *
 * Common error strings
 * defined here for consistency in driver implementations
 *
 */

#define	DBI_ERR_CONNECTION_FAILED   "Failed to connect to database: %s"
#define DBI_ERR_DB_UNAVAILABLE	    "Database not available"
#define DBI_ERR_EXECUTE_INVALID	    "Execute called on a closed or invalid statement"
#define DBI_ERR_EXECUTE_FAILED	    "Execute failed %s"
#define DBI_ERR_FETCH_INVALID	    "Fetch called on a closed or invalid statement"
#define DBI_ERR_FETCH_FAILED	    "Fetch failed %s"
#define DBI_ERR_PARAM_MISCOUNT	    "Statement expected %d parameters but received %d"
#define DBI_ERR_BINDING_PARAMS	    "Error binding statement parameters: %s"
#define DBI_ERR_BINDING_EXEC	    "Error executing statement parameters: %s"
#define DBI_ERR_FETCH_NO_EXECUTE    "Fetch called before execute"
#define DBI_ERR_BINDING_RESULTS	    "Error binding statement results: %s"
#define DBI_ERR_UNKNOWN_PUSH	    "Unknown push type in result set"
#define DBI_ERR_ALLOC_STATEMENT	    "Error allocating statement handle: %s"
#define DBI_ERR_PREP_STATEMENT	    "Error preparing statement handle: %s"
#define DBI_ERR_INVALID_PORT	    "Invalid port: %d"
#define DBI_ERR_ALLOC_RESULT	    "Error allocating result set: %s"
#define DBI_ERR_DESC_RESULT	    "Error describing result set: %s"
#define DBI_ERR_BINDING_TYPE_ERR    "Unknown or unsupported type `%s'"
#define DBI_ERR_INVALID_STATEMENT   "Invalid statement handle"
#define DBI_ERR_NOT_IMPLEMENTED     "Method %s.%s is not implemented"
#define DBI_ERR_QUOTING_STR         "Error quoting string: %s"

/*
 * convert string to lower case
 */
const char *strlower(char *in);

/*
 * replace '?' placeholders with .\d+ placeholders
 * to be compatible with the native driver API
 */
char *replace_placeholders(lua_State *L, char native_prefix, const char *sql);

