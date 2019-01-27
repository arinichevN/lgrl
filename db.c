
#include "main.h"
int getProg_callback ( void *data, int argc, char **argv, char **azColName ) {
    Prog *item = data;
    int c = 0;
    DB_FOREACH_COLUMN {
        if ( DB_COLUMN_IS ( "id" ) ) {
            item->id = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "kind" ) ) {
            if ( strcmp ( DB_COLUMN_VALUE, LOG_KIND_FTS_STR ) == 0 ) {
                item->kind=LOG_KIND_FTS;
            } else {
                item->kind=UNDEFINED;
            }
            c++;
        } else if ( DB_COLUMN_IS ( "interval_sec" ) ) {
            item->interval.tv_nsec = 0;
            item->interval.tv_sec = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "max_rows" ) ) {
            item->max_rows = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "clear" ) ) {
            item->clear = DB_CVI;
            c++;
        } else {
            printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
            c++;
        }
    }
#define N 5
    if ( c != N ) {
        printde ( "required %d columns but %d found\n", N, c );
        return EXIT_FAILURE;
    }
#undef N
    return EXIT_SUCCESS;
}
int getProgByIdFromDB ( Prog *item,int id, sqlite3 *dbl, const char *db_path ) {
    int close=0;
    sqlite3 *db=db_openAlt ( dbl, db_path, &close );
    if ( db==NULL ) {
        putsde ( " failed\n" );
        return 0;
    }
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select * from prog where id=%d limit 1", id );
    memset ( item, 0, sizeof *item );
    if ( !db_exec ( db, q, getProg_callback, item ) ) {
        putsde ( " failed\n" );
        if ( close ) db_close ( db );
        return 0;
    }
    if ( close ) db_close ( db );
    return 1;
}

int getChannel_callback ( void *data, int argc, char **argv, char **azColName ) {
    struct ds {
        void *p1;
        void *p2;
    } *d;
    d=data;
    sqlite3 *db=d->p2;
    Channel *item = d->p1;
    int load = 0, enable = 0;

    int c = 0;
    DB_FOREACH_COLUMN {
        if ( DB_COLUMN_IS ( "id" ) ) {
            item->id = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "description" ) ) {
            c++;
        } else if ( DB_COLUMN_IS ( "prog_id" ) ) {
            item->prog.id=DB_CVI;
            if ( !getProgByIdFromDB ( &item->prog,item->prog.id, db, NULL ) ) {
                return EXIT_FAILURE;
            }
            c++;
        } else if ( DB_COLUMN_IS ( "sensor_remote_channel_id" ) ) {
            item->sensor.remote_channel.id=DB_CVI;
            if ( !config_getRChannel ( &item->sensor.remote_channel, item->sensor.remote_channel.id, db, NULL ) ) {
                return EXIT_FAILURE;
            }
            c++;
        } else if ( DB_COLUMN_IS ( "cycle_duration_sec" ) ) {
            item->cycle_duration.tv_sec=DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "cycle_duration_nsec" ) ) {
            item->cycle_duration.tv_nsec=DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "enable" ) ) {
            enable = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "load" ) ) {
            load = DB_CVI;
            c++;
        } else if ( DB_COLUMN_IS ( "save" ) ) {
            item->save = DB_CVI;
            c++;
        } else {
            printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
        }
    }
#define N 9
    if ( c != N ) {
        printde ( "required %d columns but %d found\n", N, c );
        return EXIT_FAILURE;
    }
#undef N
    if ( enable ) {
        item->prog.state=INIT;
    } else {
        item->prog.state=OFF;
    }
    if ( !load ) {
        if ( item->save ) db_saveTableFieldInt ( "channel", "load", item->id, 1,db, NULL );
    }
    return EXIT_SUCCESS;
}
int getChannelByIdFromDB ( Channel *item,int channel_id, sqlite3 *dbl, const char *db_path ) {
    int close=0;
    sqlite3 *db=db_openAlt ( dbl, db_path, &close );
    if ( db==NULL ) {
        putsde ( " failed\n" );
        return 0;
    }
    char q[LINE_SIZE];
    snprintf ( q, sizeof q, "select * from channel where id=%d limit 1", channel_id );
    struct ds {
        void *p1;
        void *p2;
    } data= {.p1=item, .p2=db};
    memset ( item, 0, sizeof *item );
    if ( !db_exec ( db, q, getChannel_callback, &data ) ) {
        putsde ( " failed\n" );
        if ( close ) db_close ( db );
        return 0;
    }
    if ( close ) db_close ( db );
    return 1;
}
int addChannel ( Channel *item, ChannelLList *list, Mutex *list_mutex ) {
    if ( list->length >= INT_MAX ) {
        printde ( "can not load channel with id=%d - list length exceeded\n", item->id );
        return 0;
    }
    if ( list->top == NULL ) {
        lockMutex ( list_mutex );
        list->top = item;
        unlockMutex ( list_mutex );
    } else {
        lockMutex ( &list->last->mutex );
        list->last->next = item;
        unlockMutex ( &list->last->mutex );
    }
    list->last = item;
    list->length++;
    printdo ( "channel with id=%d loaded\n", item->id );
    return 1;
}

//returns deleted channel
Channel * deleteChannel ( int id, ChannelLList *list, Mutex *list_mutex ) {
    Channel *prev = NULL;
    FOREACH_LLIST ( curr,list,Channel ) {
        if ( curr->id == id ) {
            if ( prev != NULL ) {
                lockMutex ( &prev->mutex );
                prev->next = curr->next;
                unlockMutex ( &prev->mutex );
            } else {//curr=top
                lockMutex ( list_mutex );
                list->top = curr->next;
                unlockMutex ( list_mutex );
            }
            if ( curr == list->last ) {
                list->last = prev;
            }
            list->length--;
            return curr;
        }
        prev = curr;
    }
    return NULL;
}

int addChannelById ( int channel_id, ChannelLList *list, Mutex *list_mutex, sqlite3 *dbl, const char *db_path ) {
    {
        Channel *item;
        LLIST_GETBYID ( item,list,channel_id )
        if ( item != NULL ) {
            printde ( "channel with id = %d is being controlled\n", item->id );
            return 0;
        }
    }

    Channel *item = malloc ( sizeof * ( item ) );
    if ( item == NULL ) {
        putsde ( "failed to allocate memory\n" );
        return 0;
    }
    memset ( item, 0, sizeof *item );
    item->id = channel_id;
    item->next = NULL;
    if ( !getChannelByIdFromDB ( item, channel_id, dbl, db_path ) ) {
        free ( item );
        return 0;
    }
    if ( !checkChannel ( item ) ) {
        free ( item );
        return 0;
    }
    if ( !checkProg ( &item->prog ) ) {
        free ( item );
        return 0;
    }
    if ( !initMutex ( &item->mutex ) ) {
        free ( item );
        return 0;
    }
    if ( !initClient ( &item->sock_fd, WAIT_RESP_TIMEOUT ) ) {
        freeMutex ( &item->mutex );
        free ( item );
        return 0;
    }
    if ( !initRChannel ( &item->sensor.remote_channel, &item->sock_fd ) ) {
        freeMutex ( &item->mutex );
        free ( item );
        return 0;
    }
    if ( !addChannel ( item, list, list_mutex ) ) {
        freeSocketFd ( &item->sock_fd );
        freeMutex ( &item->mutex );
        free ( item );
        return 0;
    }
    if ( !createMThread ( &item->thread, &threadFunction, item ) ) {
        deleteChannel ( item->id, list, list_mutex );
        freeSocketFd ( &item->sock_fd );
        freeMutex ( &item->mutex );
        free ( item );
        return 0;
    }
    return 1;
}

int deleteChannelById ( int id, ChannelLList *list, Mutex *list_mutex, sqlite3 *dbl, const char *db_path ) {
    printdo ( "channel to delete: %d\n", id );
    Channel *del_channel= deleteChannel ( id, list, list_mutex );
    if ( del_channel==NULL ) {
        putsdo ( "channel to delete not found\n" );
        return 0;
    }
    STOP_CHANNEL_THREAD ( del_channel );
    if ( del_channel->save ) db_saveTableFieldInt ( "channel", "load", del_channel->id, 0, dbl, db_path );
    freeChannel ( del_channel );
    printdo ( "channel with id: %d has been deleted from channel_list\n", id );
    return 1;
}

int loadActiveChannel_callback ( void *data, int argc, char **argv, char **azColName ) {
    struct ds {
        void *a;
        void *b;
        void *c;
        const void *d;
    };
    struct ds *d=data;
    ChannelLList *list=d->a;
    Mutex *list_mutex=d->b;
    sqlite3 *db=d->c;
    const char *db_path=d->d;
    DB_FOREACH_COLUMN {
        if ( DB_COLUMN_IS ( "id" ) ) {
            addChannelById ( DB_CVI, list,  list_mutex, db, db_path );
        } else {
            printde ( "unknown column (we will skip it): %s\n", DB_COLUMN_NAME );
        }
    }
    return EXIT_SUCCESS;
}

int loadActiveChannel ( ChannelLList *list, Mutex *list_mutex, sqlite3 *dbl, const char *db_path ) {
    int close=0;
    sqlite3 *db=db_openAlt ( dbl, db_path, &close );
    if ( db==NULL ) {
        putsde ( " failed\n" );
        return 0;
    }
    struct ds {
        void *a;
        void *b;
        void *c;
        const void *d;
    };
    struct ds data = {.a = list, .b = list_mutex, .c = db, .d = db_path};
    char *q = "select id from channel where load=1";
    if ( !db_exec ( db, q, loadActiveChannel_callback, &data ) ) {
        if ( close ) db_close ( db );
        return 0;
    }
    if ( close ) db_close ( db );
    return 1;
}



