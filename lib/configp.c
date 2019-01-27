#include "configp.h"

int configp_getPort ( int * sock_port,const char * peer_id, PGconn * db_conn, const char * db_conninfo ) {
    if ( db_conn != NULL && db_conninfo != NULL ) {
        putsde ( "db_con xor db_conninfo expected\n" );
        return 0;
    }
    PGconn *db;
    int close=0;
    if ( db_path != NULL ) {
        if ( !dbp_open ( db_conninfo, &db ) ) {
            putsde ( "failed\n" );
            return 0;
        }
        close=1;
    } else {
        db = db_conn;
    }
    char q[LINE_SIZE];
    int _port=-1;
    snprintf ( q, sizeof q, "SELECT port FROM public.peer where id='%s'", peer_id );
    if ( !dbp_getInt ( &_port, db, q ) ) {
        putsde ( "failed\n" );
        if ( close ) dbp_close ( db );
        return 0;
    }
    if ( close ) dbp_close ( db );
    *port=_port;
    return 1;
}

int configp_checkPeerList ( const PeerList *list ) {
    //unique id
    for ( size_t i = 0; i < list->length; i++ ) {
        for ( size_t j = i + 1; j < list->length; j++ ) {
            if ( strcmp ( list->item[i].id, list->item[j].id ) == 0 ) {
                printde ( "id = %s is not unique\n", list->item[i].id );
                return 0;
            }
        }
    }
    return 1;
}

int configp_checkRChannelList ( const RChannelList *list ) {
    //unique id
    for ( size_t i = 0; i < list->length; i++ ) {
        for ( size_t j = i + 1; j < list->length; j++ ) {
            if ( strcmp ( list->item[i].id, list->item[j].id ) == 0 ) {
                printde ( "id = %s is not unique\n", list->item[i].id );
                return 0;
            }
        }
    }
    return 1;
}

int configp_getPeerList ( PeerList *list, int *fd, PGconn *db_conn, const char * db_conninfo ) {
    RESET_LIST ( list )
    PGconn *db;
    int close=0;
    if ( !dbp_getConnAlt ( &db, &close, db_conn, db_conninfo ) ) {
        putsde ( "DB connection failed\n" );
        return 0;
    }
    PGresult *r;
    char *q = "select id, port, ip_addr from public.peer";
    if ( ! dbp_exec ( &r, db, q ) ) {
        putsde ( "failed to read from DB\n" );
        if ( close ) PQfinish ( db );
        return 0;
    }
    int n = PQntuples ( r );
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
        putsde ( "failed to allocate memory\n" );
        PQclear ( r );
        if ( close ) PQfinish ( db );
        return 0;
    }

    DBP_DFN ( id );
    DBP_DFN ( port );
    DBP_DFN ( ip_addr );
    if ( DBP_FN ( id ) <0 || DBP_FN ( port ) <0 || DBP_FN ( ip_addr ) <0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): one of fields not found\n", F );
#endif
        PQclear ( r );
        if ( close ) PQfinish ( db );
        return 0;
    }
    for ( size_t i=0; i<list->max_length; i++ ) {
        strcpyma ( &list->item[i].id, PQgetvalue ( r, i, DBP_FN ( id ) ) );
        if ( list->item[i].id==NULL ) {
#ifdef MODE_DEBUG
            fprintf ( stderr, "%s(): error while reading peer.id where row=%d\n", F, i );
#endif
            PQclear ( r );
            if ( close ) PQfinish ( db );
            return 0;
        }
        list->item[i].port = DBP_CVI ( r, i, DBP_FN ( port ) );
        strcpyma ( &list->item[i].addr_str, PQgetvalue ( r, i, DBP_FN ( ip_addr ) ) );
        if ( list->item[i].addr_str==NULL ) {
#ifdef MODE_DEBUG
            fprintf ( stderr, "%s(): error while reading peer.ip_addr where row=%d\n", F, i );
#endif
            PQclear ( r );
            if ( close ) PQfinish ( db );
            return 0;
        }
        list->length++;
    }
    PQclear ( r );
    if ( close ) PQfinish ( db );
    if ( !configp_checkPeerList ( list ) ) {
        return 0;
    }
    return 1;
}

int configp_getRChannelList ( RChannelList *list, PeerList *peer_list, const char *db_schema, PGconn *db_conn, const char * db_conninfo ) {
    RESET_LIST ( list )
    PGconn *db;
    int close=0;
    if ( !dbp_getConnAlt ( &db, &close, db_conn, db_conninfo ) ) {
        putsde ( "DB connection failed\n" );
        return 0;
    }
    PGresult *r;
    char q[LINE_SIZE] = "select id, peer_id, channel_id from %s.remote_channel";
    snprintf ( q, sizeof q, "select id, peer_id, channel_id from %s.remote_channel", db_schema );
    if ( ! dbp_exec ( &r, db, q ) ) {
        putsde ( "failed to read from DB\n" );
        if ( close ) PQfinish ( db );
        return 0;
    }
    int n = PQntuples ( r );
    ALLOC_LIST ( list,n )
    if ( list->max_length!=n ) {
        putsde ( "failed to allocate memory\n" );
        PQclear ( r );
        if ( close ) PQfinish ( db );
        return 0;
    }

    DBP_DFN ( id );
    DBP_DFN ( peer_id );
    DBP_DFN ( channel_id );
    if ( DBP_FN ( id ) <0 || DBP_FN ( peer_id ) <0 || DBP_FN ( channel_id ) <0 ) {
        putsde ( "one of fields not found\n" );
        PQclear ( r );
        if ( close ) PQfinish ( db );
        return 0;
    }
    for ( size_t i=0; i<list->max_length; i++ ) {
        LIi.id=DBP_CVI ( r, i ,DBP_FN ( id ) );
        char * peer_id=PQgetvalue ( r, i, DBP_FN ( peer_id ) );
        Peer * peer;
        LIST_GETBYIDSTR ( peer, peer_list, peer_id )
        if ( peer == NULL ) {
            printde ( "peer not found where peer_id=%s\n", peer_id );
            PQclear ( r );
            if ( close ) PQfinish ( db );
            return 0;
        }
        item->peer = *peer;
        LIi.channel_id=DBP_CVI ( r, i ,DBP_FN ( channel_id ) );
        list->length++;
    }
    PQclear ( r );
    if ( close ) PQfinish ( db );
    if ( !configp_checkRChannelList ( list ) ) {
        return 0;
    }
    return 1;
}

int configp_getStrValFromTbl ( PGconn *db_conn, const char *id, char *value, const char *tbl, size_t value_size ) {
    PGresult *r;
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select value from public.%s where id='%s'", tbl, id );
    if ( ( r = dbGetDataT ( db_conn, q, q ) ) == NULL ) {
        return 0;
    }
    if ( PQntuples ( r ) == 1 ) {
        memcpy ( value, PQgetvalue ( r, 0, 0 ), value_size );
        PQclear ( r );
        return 1;
    }
#ifdef MODE_DEBUG
    fputs ( "configp_getPidPath: one tuple expected\n", stderr );
#endif
    PQclear ( r );
    return 0;
}

int configp_getBufSize ( PGconn *db_conn, const char *id, size_t *value ) {
    PGresult *r;
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select value from public.buf_size where id='%s'", id );
    if ( ( r = dbGetDataT ( db_conn, q, q ) ) == NULL ) {
        return 0;
    }
    if ( PQntuples ( r ) == 1 ) {
        *value = atoi ( PQgetvalue ( r, 0, 0 ) );
        PQclear ( r );
        return 1;
    }
#ifdef MODE_DEBUG
    fputs ( "configp_getBufSize: one tuple expected\n", stderr );
#endif
    PQclear ( r );
    return 0;
}

int configp_getCycleDurationUs ( PGconn *db_conn, const char *id, struct timespec *value ) {
    PGresult *r;
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select value from public.cycle_duration_us where id='%s'", id );
    if ( ( r = dbGetDataT ( db_conn, q, q ) ) == NULL ) {
        return 0;
    }
    if ( PQntuples ( r ) == 1 ) {
        *value = usToTimespec ( atoi ( PQgetvalue ( r, 0, 0 ) ) );
        PQclear ( r );
        return 1;
    }
#ifdef MODE_DEBUG
    fputs ( "configp_getCycleDurationUs: one tuple expected\n", stderr );
#endif
    PQclear ( r );
    return 0;
}

/*
int configp_getUDPPort(PGconn *db_conn, const char *id, size_t *value) {
    PGresult *r;
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "select udp_port from public.peer where id='%s'", id);
    if ((r = dbGetDataT(db_conn, q, q)) == NULL) {
        return 0;
    }
    if (PQntuples(r) == 1) {
        *value = atoi(PQgetvalue(r, 0, 0));
        PQclear(r);
        return 1;
    }
#ifdef MODE_DEBUG
    fputs("configp_getPort: one tuple expected\n", stderr);
#endif
    PQclear(r);
    return 0;
}
*/

int configp_getPidPath ( PGconn *db_conn, const char *id, char *value, size_t value_size ) {
    PGresult *r;
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select value from public.pid_path where id='%s'", id );
    if ( ( r = dbGetDataT ( db_conn, q, q ) ) == NULL ) {
        return 0;
    }
    if ( PQntuples ( r ) == 1 ) {
        memcpy ( value, PQgetvalue ( r, 0, 0 ), value_size );
        PQclear ( r );
        return 1;
    }
#ifdef MODE_DEBUG
    fputs ( "configp_getPidPath: one tuple expected\n", stderr );
#endif
    PQclear ( r );
    return 0;
}

int configp_getI2cPath ( PGconn *db_conn, const char *id, char *value, size_t value_size ) {
    PGresult *r;
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select value from public.i2c_path where id='%s'", id );
    if ( ( r = dbGetDataT ( db_conn, q, q ) ) == NULL ) {
        return 0;
    }
    if ( PQntuples ( r ) == 1 ) {
        memcpy ( value, PQgetvalue ( r, 0, 0 ), value_size );
        PQclear ( r );
        return 1;
    }
#ifdef MODE_DEBUG
    fputs ( "configp_getI2cPath: one tuple expected\n", stderr );
#endif
    PQclear ( r );
    return 0;
}

int configp_getDbConninfo ( PGconn *db_conn, const char *id, char *value, size_t value_size ) {
    PGresult *r;
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select value from public.db_conninfo where id='%s'", id );
    if ( ( r = dbGetDataT ( db_conn, q, q ) ) == NULL ) {
        return 0;
    }
    if ( PQntuples ( r ) == 1 ) {
        memcpy ( value, PQgetvalue ( r, 0, 0 ), value_size );
        PQclear ( r );
        return 1;
    }
#ifdef MODE_DEBUG
    fputs ( "configp_getPidPath: one tuple expected\n", stderr );
#endif
    PQclear ( r );
    return 0;
}

int configp_getLockKey ( PGconn *db_conn, const char *id, char *value, size_t value_size ) {
    PGresult *r;
    char q[LINE_SIZE];
    size_t i;
    for ( i = 0; i < value_size; i++ ) {
        value[i] = '\0';
    }
    snprintf ( q, sizeof q, "select value from public.lock_key where id='%s'", id );
    if ( ( r = dbGetDataT ( db_conn, q, q ) ) == NULL ) {
        return 0;
    }
    if ( PQntuples ( r ) == 1 ) {
        memcpy ( value, PQgetvalue ( r, 0, 0 ), value_size );
        PQclear ( r );
        return 1;
    }
#ifdef MODE_DEBUG
    fputs ( "configp_getLockKey: one tuple expected\n", stderr );
#endif
    PQclear ( r );
    return 0;
}


int configp_getPort ( int *port, const char *peer_id, PGconn *dbl, const char *db_conninfo ) {
    if ( dbl != NULL && db_conninfo != NULL ) {
        putsde ( "dbl xor db_path expected\n" );
        return 0;
    }
    sqlite3 *db;
    if ( db_path != NULL ) {
        if ( !db_openR ( db_path, &db ) ) {
            putsde ( "failed\n" );
            return 0;
        }
    } else {
        db = dbl;
    }
    char q[LINE_SIZE];
    int _port=-1;
    snprintf ( q, sizeof q, "SELECT port FROM peer where id='%s'", peer_id );
    if ( !db_getInt ( &_port, db, q ) ) {
        putsde ( "failed\n" );
        if ( db_path != NULL ) {
            sqlite3_close ( db );
        }
        return 0;
    }
    if ( db_path != NULL ) {
        sqlite3_close ( db );
    }
    *port=_port;
    return 1;
}
