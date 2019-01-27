
#ifndef LIBPAS_CONFIGP_H
#define LIBPAS_CONFIGP_H

#include "dbp.h"
#include "acp/main.h"
#include "udp.h"
#include "timef.h"

extern int configp_getStrValFromTbl(PGconn *db_conn, const char *id, char *value, const char *tbl, size_t value_size);

extern int configp_getBufSize(PGconn *db_conn, const char *id, size_t *value) ;

extern int configp_getCycleDurationUs(PGconn *db_conn, const char *id, struct timespec *value) ;

extern int configp_getUDPPort(PGconn *db_conn, const char *id, size_t *value) ;

extern int configp_getPidPath(PGconn *db_conn, const char *id, char *value, size_t value_size) ;

extern int configp_getI2cPath(PGconn *db_conn, const char *id, char *value, size_t value_size) ;

extern int configp_getDbConninfo(PGconn *db_conn, const char *id, char *value, size_t value_size) ;

extern int configp_checkPeerList(const PeerList *list);
extern int configp_getPeerList( PeerList *list, int *fd, PGconn *db_conn, const char * db_conninfo);

extern int configp_getRChannelList ( RChannelList *list, PeerList *peer_list, const char *db_schema, PGconn *db_conn, const char * db_conninfo );

extern int configp_getLockKey(PGconn *db_conn, const char *id, char *value, size_t value_size);

#endif 

