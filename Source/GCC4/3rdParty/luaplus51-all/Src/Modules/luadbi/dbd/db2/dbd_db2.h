#undef UNICODE

#include <sqlcli1.h>
#include <sqlutil.h>
#include <sqlenv.h>
#include <dbd/common.h>

#define DBD_DB2_CONNECTION   "DBD.DB2.Connection"
#define DBD_DB2_STATEMENT    "DBD.DB2.Statement"

/*
 * result set metadata
 */

typedef struct _resultset {
    SQLSMALLINT name_len;
    SQLSMALLINT type;
    SQLUINTEGER size;
    SQLSMALLINT scale;
    SQLCHAR name[32];
} resultset_t;

/*
 * bind parameters
 */
typedef struct _bindparams { 
    SQLCHAR *buffer;
    SQLINTEGER len;
    SQLINTEGER buffer_len;
} bindparams_t;

/*
 * connection object implentation
 */
typedef struct _connection {
    SQLHANDLE env;
    SQLHANDLE db2;
} connection_t;

/*
 * statement object implementation
 */
typedef struct _statement {
    resultset_t * resultset;
    bindparams_t * bind;
    unsigned char *buffer;
    SQLSMALLINT num_result_columns; /* variable for SQLNumResultCols */

    SQLHANDLE stmt;
    SQLHANDLE db2;
} statement_t;

