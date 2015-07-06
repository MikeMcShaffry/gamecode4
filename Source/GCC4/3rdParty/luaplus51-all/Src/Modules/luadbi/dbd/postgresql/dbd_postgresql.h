#include <libpq-fe.h>
#include <postgres_fe.h>
#include <dbd/common.h>

/* 
 * length of a prepared statement ID
 * dbd-postgresql-\d{17}\0
 */
#define IDLEN 15+17+1

#define DBD_POSTGRESQL_CONNECTION   "DBD.PostgreSQL.Connection"
#define DBD_POSTGRESQL_STATEMENT    "DBD.PostgreSQL.Statement"

/*
 * connection object implentation
 */
typedef struct _connection {
    PGconn *postgresql;
    int autocommit;
    unsigned int statement_id; /* sequence for statement IDs */
} connection_t;

/*
 * statement object implementation
 */
typedef struct _statement {
    PGconn *postgresql;
    PGresult *result;
    char name[IDLEN]; /* statement ID */
    int tuple; /* number of rows returned */
} statement_t;

