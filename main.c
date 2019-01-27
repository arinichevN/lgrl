#include "main.h"

int app_state = APP_INIT;

TSVresult config_tsv = TSVRESULT_INITIALIZER;
char *db_prog_path;
char *db_log_path;
char *peer_id;
sqlite3 *db_log;

int sock_port = -1;
int sock_fd = -1;

Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};
struct timespec cycle_duration = {0, 0};
Mutex channel_list_mutex = MUTEX_INITIALIZER;
Mutex db_log_mutex = MUTEX_INITIALIZER;
Mutex db_mutex = MUTEX_INITIALIZER;

ChannelLList channel_list = LLIST_INITIALIZER;

#include "util.c"
#include "db.c"

int readSettings ( TSVresult* r, const char *data_path, char **peer_id, char **db_prog_path, char **db_log_path ) {
    if ( !TSVinit ( r, data_path ) ) {
        return 0;
    }
    char *_peer_id = TSVgetvalues ( r, 0, "peer_id" );
    char *_db_prog_path = TSVgetvalues ( r, 0, "db_prog_path" );
    char *_db_log_path = TSVgetvalues ( r, 0, "db_log_path" );
    if ( TSVnullreturned ( r ) ) {
        return 0;
    }
    *peer_id = _peer_id;
    *db_prog_path = _db_prog_path;
    *db_log_path = _db_log_path;
    return 1;
}

int initApp() {
    if ( !readSettings ( &config_tsv, CONFIG_FILE, &peer_id,&db_prog_path, &db_log_path ) ) {
        putsde ( "failed to read settings\n" );
        return 0;
    }
    if ( !initMutex ( &channel_list_mutex ) ) {
        putsde ( "failed to initialize channel mutex\n" );
        return 0;
    }
    if ( !initMutex ( &db_log_mutex ) ) {
        putsde ( "failed to initialize db mutex\n" );
        return 0;
    }
    if ( !config_getPort ( &sock_port, peer_id, NULL, db_prog_path ) ) {
        putsde ( "failed to read port\n" );
        return 0;
    }
    if ( !initServer ( &sock_fd, sock_port ) ) {
        putsde ( "failed to initialize udp server\n" );
        return 0;
    }
    return 1;
}

int initData() {
    if ( !db_open ( db_log_path, &db_log ) ) {
        return 0;
    }
    if ( !loadActiveChannel ( &channel_list,&channel_list_mutex,NULL, db_prog_path ) ) {
        freeChannelList ( &channel_list );
        return 0;
    }
    return 1;
}

void serverRun ( int *state, int init_state ) {
    SERVER_HEADER
    SERVER_APP_ACTIONS
    if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_STOP ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progStop ( &item->prog );
                    unlockMutex ( &item->mutex );
                }
                deleteChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
            }
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_START ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            addChannelById ( i1l.item[i], &channel_list, &channel_list_mutex, NULL, db_prog_path );
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_RESET ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progStop ( &item->prog );
                    unlockMutex ( &item->mutex );
                }
                deleteChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
            }
        }
        FORLISTN ( i1l, i ) {
            addChannelById ( i1l.item[i], &channel_list, &channel_list_mutex,NULL, db_prog_path );
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_ENABLE ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progEnable ( &item->prog );
                    if ( item->save ) db_saveTableFieldInt ( "channel", "enable", item->id, 1, NULL, db_prog_path );
                    unlockMutex ( &item->mutex );
                }
            }
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_DISABLE ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( lockMutex ( &item->mutex ) ) {
                    progDisable ( &item->prog );
                    if ( item->save ) db_saveTableFieldInt ( "channel", "enable", item->id, 0, NULL, db_prog_path );
                    unlockMutex ( &item->mutex );
                }
            }
        }
        return;
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_GET_INFO ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( !bufCatProgInfo ( item, &response ) ) {
                    return;
                }
            }
        }
    } else if ( ACP_CMD_IS ( ACP_CMD_CHANNEL_GET_ENABLED ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( !bufCatProgEnabled ( item, &response ) ) {
                    return;
                }
            }
        }
    } else if ( ACP_CMD_IS ( ACP_CMD_GET_FTS ) ) {
        SERVER_GET_I1LIST_FROM_REQUEST
        FORLISTN ( i1l, i ) {
            Channel *item;
            LLIST_GETBYID ( item, &channel_list, i1l.item[i] )
            if ( item != NULL ) {
                if ( !bufCatFTS ( item, &response ) ) {
                    return;
                }
            }
        }
    }
    acp_responseSend ( &response, &peer_client );
}



void cleanup_handler ( void *arg ) {
    Channel *item = arg;
    printf ( "cleaning up thread %d\n", item->prog.id );
}

void *threadFunction ( void *arg ) {
    Channel *item = arg;
    printdo ( "thread for channel with id=%d has been started\n", item->id );
#ifdef MODE_DEBUG
    pthread_cleanup_push ( cleanup_handler, item );
#endif
    while ( 1 ) {
        struct timespec t1 = getCurrentTime();
        int old_state;
        if ( threadCancelDisable ( &old_state ) ) {
            if ( lockMutex ( &item->mutex ) ) {
                progControl ( &item->prog, &item->sensor, item->id, db_log, &db_log_mutex );
                unlockMutex ( &item->mutex );
            }
            threadSetCancelState ( old_state );
        }
        delayTsIdleRest ( item->cycle_duration, t1 );
    }
#ifdef MODE_DEBUG
    pthread_cleanup_pop ( 1 );
#endif
}

void freeData() {
    STOP_ALL_CHANNEL_THREADS ( &channel_list );
    freeChannelList ( &channel_list );
    db_close ( db_log );
}

void freeApp() {
    freeData();
    freeSocketFd ( &sock_fd );
    freeMutex ( &channel_list_mutex );
    freeMutex ( &db_log_mutex );
    TSVclear ( &config_tsv );
}

void exit_nicely ( ) {
    freeApp();
    putsdo ( "\nexiting now...\n" );
    exit ( EXIT_SUCCESS );
}

int main ( int argc, char** argv ) {
#ifndef MODE_DEBUG
    daemon ( 0, 0 );
#endif
    conSig ( &exit_nicely );
    if ( mlockall ( MCL_CURRENT | MCL_FUTURE ) == -1 ) {
        perrorl ( "mlockall()" );
    }
    int data_initialized = 0;
    while ( 1 ) {
#ifdef MODE_DEBUG
        printf ( "%s(): %s %d\n", F, getAppState ( app_state ), data_initialized );
#endif
        switch ( app_state ) {
        case APP_INIT:
            if ( !initApp() ) {
                return ( EXIT_FAILURE );
            }
            app_state = APP_INIT_DATA;
            break;
        case APP_INIT_DATA:
            data_initialized = initData();
            app_state = APP_RUN;
            delayUsIdle ( 1000000 );
            break;
        case APP_RUN:
            serverRun ( &app_state, data_initialized );
            break;
        case APP_STOP:
            freeData();
            data_initialized = 0;
            app_state = APP_RUN;
            break;
        case APP_RESET:
            freeApp();
            delayUsIdle ( 1000000 );
            data_initialized = 0;
            app_state = APP_INIT;
            break;
        case APP_EXIT:
            exit_nicely();
            break;
        default:
            freeApp();
            putsde ( "unknown application state\n" );
            return ( EXIT_FAILURE );
        }
    }
    freeApp();
    putsde ( "unexpected while break\n" );
    return ( EXIT_FAILURE );
}
