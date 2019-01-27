#ifndef LIBPAS_DBP_H
#define LIBPAS_DBP_H
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

#include "app.h"
#include "timef.h"

#define DBP_WAIT_DELAY_US 1000000

#define DBP_R PGresult
#define DBP_CR(result) PQclear(result)

#define DBP_CVI(result, row, field) atoi(PQgetvalue(result, row, field))
#define DBP_CVF(result, row, field) atof(PQgetvalue(result, row, field))

#define DBP_FN(n) n ## _dbpfn
#define DBP_DFN(n) int DBP_FN(n) = PQfnumber ( r, #n )

extern int dbp_open(const char *conninfo,PGconn **conn);

extern int dbp_wait(const char *conninfo);

extern int dbp_getConnAlt(PGconn **conn_out, int *close, const PGconn * conn, const char * conninfo );



extern int dbp_cmd(PGconn *conn, char *q);

extern int dbp_exec(PGresult **r , PGconn *conn, const char *q);

extern int dbp_getInt(int *item, PGconn *conn, char *q);

extern int dbp_conninfoParse(const char *buf, char *host, int *port, char *dbname, char *user, size_t str_size);

extern int dbp_conninfoEq(char *c1, char *c2);

extern int dbp_saveTableFieldInt(const char * table, const char *field, int id, int value, const char * schema,  PGconn *conn, const char* conninfo);

#endif 

