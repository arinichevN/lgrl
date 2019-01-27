
#ifndef LGRL_H
#define LGRL_H

#include "lib/dbl.h"
#include "lib/configl.h"
#include "lib/util.h"
#include "lib/crc.h"
#include "lib/app.h"
#include "lib/timef.h"
#include "lib/udp.h"
#include "lib/tsv.h"
#include "lib/acp/main.h"
#include "lib/acp/app.h"
#include "lib/acp/channel.h"
#include "lib/acp/lgr.h"


#define APP_NAME lgr
#define APP_NAME_STR TOSTRING(APP_NAME)


#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./"
#endif
#define CONFIG_FILE "" CONF_DIR "config.tsv"

#define WAIT_RESP_TIMEOUT 3

#define LOG_KIND_FTS_STR "fts"
#define STATUS_SUCCESS "SUCCESS"
#define STATUS_FAILURE "FAILURE"
#define sensorRead(item) acp_getFTS ( &(item)->input, &(item)->remote_channel.peer, (item)->remote_channel.channel_id )
enum ProgState {
    OFF,
    INIT,
    RUN,
    LOG_KIND_FTS,
    UNDEFINED,
    DISABLE,
    FAILURE
} ;

typedef struct {
    RChannel remote_channel;
    FTS input;
} Sensor;

typedef struct {
    int id;
    struct timespec interval;
    size_t max_rows;
    int kind;
    int clear;

    int state;
    Ton tmr;
}Prog;

struct channel_st {
    int id;
    Prog prog;
    Sensor sensor;
    int sock_fd;
    int save;
    struct timespec cycle_duration;
    Mutex mutex;
    pthread_t thread;
    struct channel_st *next;
};

typedef struct channel_st Channel;

DEC_LLIST(Channel)

extern int readSettings();

extern int initData();

extern int initApp();

extern void serverRun(int *state, int init_state);

extern void *threadFunction(void *arg);

extern void freeData();

extern void freeApp();

extern void exit_nicely();

#endif 




