#include <oci.h>
#include <dbd/common.h>

#define DBD_ORACLE_CONNECTION	"DBD.Oracle.Connection"
#define DBD_ORACLE_STATEMENT	"DBD.Oracle.Statement"

typedef struct _bindparams {
    OCIParam *param;
    text *name;
    ub4 name_len;
    ub2 data_type;
    ub2 max_len;
    char *data;
    OCIDefine *define;
    sb2 null;
} bindparams_t;

/*
 * connection object
 */
typedef struct _connection {
    OCIEnv *oracle;
    OCISvcCtx *svc;
    OCIError *err;
    OCIServer *srv;
    OCISession *auth;
    int autocommit;
} connection_t;

/*
 * statement object
 */
typedef struct _statement {
    OCIStmt *stmt;
    connection_t *conn;
    int num_columns;
    bindparams_t *bind;

    int metadata;
} statement_t;

