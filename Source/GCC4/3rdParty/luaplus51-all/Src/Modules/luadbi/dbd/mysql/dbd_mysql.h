#ifdef _MSC_VER  /* all MS compilers define this (version) */
	#include <windows.h> 
#endif


#include <mysql.h>
#include <dbd/common.h>

#define DBD_MYSQL_CONNECTION	"DBD.MySQL.Connection"
#define DBD_MYSQL_STATEMENT	"DBD.MySQL.Statement"

/*
 * connection object implementation
 */
typedef struct _connection {
    MYSQL *mysql;
} connection_t;

/*
 * statement object implementation
 */
typedef struct _statement {
    MYSQL *mysql;
    MYSQL_STMT *stmt;
    MYSQL_RES *metadata; /* result dataset metadata */
} statement_t;

