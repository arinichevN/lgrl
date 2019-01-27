
#ifndef LIBPAS_APP_H
#define LIBPAS_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <sched.h>

#include <signal.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#include "common.h"
#include "acp/app.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define GOOD_FLOAT 1.0
#define BAD_FLOAT 0.0
#define GOOD_INT 1
#define BAD_INT 0

#define POSITIVE_FLOAT GOOD_FLOAT
#define NEGATIVE_FLOAT BAD_FLOAT
#define POSITIVE_INT GOOD_INT
#define NEGATIVE_INT BAD_INT

#define CDS ACP_DELIMITER_COLUMN_STR
#define RDS ACP_DELIMITER_ROW_STR

#define DEF_THREAD pthread_t thread;char thread_cmd=0;void *threadFunction(void *arg);
#define THREAD_CREATE createThread(&thread,&threadFunction,&thread_cmd)
#define THREAD_STOP thread_cmd = 1;pthread_join(thread, NULL);
#define THREAD_EXIT_ON_CMD if (*cmd) {*cmd = 0;return (EXIT_SUCCESS); }
#define THREAD_DEF_CMD char *cmd = (char *) arg;

#define PROG_ERROR_NO_SIGNAL_FROM_CLIENT 0x1
#define PROG_ERROR_NO_RESPONSE_FROM_SENSOR 0x2
#define PROG_ERROR_HARDWARE 0x4

#define BIT_ENABLE(buf,v) (buf)|=(v)
#define BIT_DISABLE(buf,v) (buf)&=~(v)
#define BIT_IS_ENABLED(buf,v) (buf)&(v)

#define FORLi for (size_t i = 0; i < list->length; i++) 
#define FORL FORLi
#define FORMLi for (size_t i = 0; i < list->max_length; i++) 
#define FORLISTP(L, I) for (size_t I = 0; I < (L)->length; I++) 
#define FORLISTN(L, I) for (size_t I = 0; I < (L).length; I++) 
#define FORLIST(I) for (size_t I = 0; I < list->length; I++) 
#define FORLLj  for (size_t j = i + 1; j < list->length; j++) 
#define FORLISTPL(L, I, J)  for (size_t J = I + 1; J < (L)->length; J++) 
#define FORLISTNL(L, I, J)  for (size_t J = I + 1; J < (L).length; J++) 
#define FFORLISTPL(L, I, J)  for (size_t I = 0; I < (L)->length; I++){for (size_t J = I + 1; J < (L)->length; J++)
#define FFORLISTNL(L, I, J)  for (size_t I = 0; I < (L).length; I++){for (size_t J = I + 1; J < (L).length; J++)

#define FOREACH_CHANNEL FOREACH_LLIST(item,&channel_list,Channel)

#define LI(L,I) (L)->item[I]
#define LIi LI(list,i)
#define LIj LI(list,j)
#define LIll list->item[list->length]
#define Lil list->length-1
#define LL list->length
#define LML list->max_length
#define LIiei(v) LIi.v=atoi(v)
#define LIief(v) LIi.v=atof(v)

#define SERVER_HEADER \
    ACPResponse response;ACPRequest request;\
    acp_requestInit(&request);\
    acp_responseInit(&response);\
    if (!acp_requestRead(&request, &peer_client)) {return;}\
    if (!acp_requestCheck(&request)) {return;}\
    acp_responseCoopRequest(&response, &request);

#define SERVER_APP_ACTIONS \
    if (acp_cmdcmp(&request, ACP_CMD_APP_START)) {\
        if (!init_state) {*state = APP_INIT_DATA;}\
        return;\
    } else if (acp_cmdcmp(&request, ACP_CMD_APP_STOP)) {\
        if (init_state) {*state = APP_STOP;}\
        return;\
    } else if (acp_cmdcmp(&request, ACP_CMD_APP_RESET)) {\
        *state = APP_RESET;\
        return;\
    } else if (acp_cmdcmp(&request, ACP_CMD_APP_EXIT)) {\
        *state = APP_EXIT;\
        return;\
    } else if (acp_cmdcmp(&request, ACP_CMD_APP_PING)) {\
        if (init_state) {acp_responseSendStr(ACP_RESP_APP_BUSY, ACP_LAST_PACK, &response, &peer_client);} else {acp_responseSendStr(ACP_RESP_APP_IDLE, ACP_LAST_PACK, &response, &peer_client);}\
        return;\
    } else if (acp_cmdcmp(&request, ACP_CMD_APP_PRINT)) {\
        printData(&response);\
        return;\
    } else if (acp_cmdcmp(&request, ACP_CMD_APP_HELP)) {\
        printHelp(&response);\
        return;\
    }else if (acp_cmdcmp(&request, ACP_CMD_APP_TIME)) {\
        acp_responseSendCurTime(&response, &peer_client);\
        return;\
    }\
    if (!init_state) {return;}

#define SERVER_DEF_I1LIST I1 i1_arr[request.data_rows_count];I1List i1l;i1l.item=i1_arr;i1l.max_length=request.data_rows_count;i1l.length=0;
#define SERVER_DEF_I2LIST I2 i2_arr[request.data_rows_count];I2List i2l;i2l.item=i2_arr;i2l.max_length=request.data_rows_count;i2l.length=0;
#define SERVER_DEF_I1F1LIST I1F1 i1f1_arr[request.data_rows_count];I1F1List i1f1l;i1f1l.item=i1f1_arr;i1f1l.max_length=request.data_rows_count;i1f1l.length=0;
#define DEF_SERVER_DEF_S1LIST(str_sz) S1 s1_arr[request.data_rows_count * str_sz];S1List s1l;s1l.item=s1_arr;s1l.max_length=request.data_rows_count * str_sz;s1l.length=0;
#define SERVER_DEF_S2LIST S2 s2_arr[request.data_rows_count];S2List s2l;s2l.item=s2_arr;s2l.max_length=request.data_rows_count;s2l.length=0;
#define SERVER_DEF_I1S1LIST I1S1 i1s1_arr[request.data_rows_count];I1S1List i1s1l;i1s1l.item=i1s1_arr;i1s1l.max_length=request.data_rows_count;i1s1l.length=0;

#define SERVER_PARSE_I1LIST acp_requestDataToI1List(&request, &i1l);if (i1l.length <= 0)return;
#define SERVER_PARSE_I1F1LIST acp_requestDataToI1F1List(&request, &i1f1l);if (i1f1l.length <= 0)return;
#define SERVER_PARSE_I2LIST acp_requestDataToI2List(&request, &i2l);if (i2l.length <= 0)return;
#define SERVER_PARSE_I1S1LIST acp_requestDataToI1S1List(&request, &i1s1l);if (i1s1l.length <= 0)return;

#define SERVER_GET_I1LIST_FROM_REQUEST SERVER_DEF_I1LIST SERVER_PARSE_I1LIST
#define SERVER_GET_I2LIST_FROM_REQUEST SERVER_DEF_I2LIST SERVER_PARSE_I2LIST
#define SERVER_GET_I1F1LIST_FROM_REQUEST SERVER_DEF_I1F1LIST SERVER_PARSE_I1F1LIST
#define SERVER_GET_I1S1LIST_FROM_REQUEST SERVER_DEF_I1S1LIST SERVER_PARSE_I1S1LIST

#define SEND_STR(V) acp_responseSendStr(V, ACP_MIDDLE_PACK, response, &peer_client);
#define SEND_STR_L(V) acp_responseSendStr(V, ACP_LAST_PACK, response, &peer_client);

#define SEND_STR_P(V) acp_responseSendStr(V, ACP_MIDDLE_PACK, &response, &peer_client);
#define SEND_STR_L_P(V) acp_responseSendStr(V, ACP_LAST_PACK, &response, &peer_client);

#ifdef MODE_DEBUG
#define STOP_CHANNEL_THREAD(channel) {printf("signaling thread %d to cancel...\n", (channel)->id);if (pthread_cancel((channel)->thread) != 0)perror("pthread_cancel()");void * App_result;printf("joining thread %d...\n", (channel)->id);if (pthread_join((channel)->thread, &App_result) != 0) perror("pthread_join()");if (App_result != PTHREAD_CANCELED) printf("thread %d not canceled\n", (channel)->id);}
#else
#define STOP_CHANNEL_THREAD(channel) {pthread_cancel((channel)->thread);void * App_result;pthread_join((channel)->thread, &App_result);}
#endif

#ifdef MODE_DEBUG
#define STOP_ALL_LLIST_THREADS(list, T) {FOREACH_LLIST(item,(list),T){printf("signaling thread %d to cancel...\n", item->id);if (pthread_cancel(item->thread) != 0) perror("pthread_cancel()");}FOREACH_LLIST(item,list,T){void * App_result;printf("joining thread %d...\n", item->id);if (pthread_join(item->thread, &App_result) != 0) perror("pthread_join()");if (App_result != PTHREAD_CANCELED) printf("thread %d not canceled\n", item->id);}}
#else
#define STOP_ALL_LLIST_THREADS(list, T) {FOREACH_LLIST(item,(list),T){pthread_cancel(item->thread);}FOREACH_LLIST(item,list,T){void * App_result;pthread_join(item->thread, &App_result);}}
#endif
#define STOP_ALL_CHANNEL_THREADS(channel_list) STOP_ALL_LLIST_THREADS(channel_list, Channel)

#ifdef MODE_DEBUG
#define STOP_ALL_LIST_THREADS(list) FORLISTP(list, i){printf("signaling thread %d to cancel...\n", (list)->item[i].id);if (pthread_cancel((list)->item[i].thread) != 0) perror("pthread_cancel()");}FORLISTP(list, i){void * App_result;printf("joining thread %d...\n", (list)->item[i].id);if (pthread_join((list)->item[i].thread, &App_result) != 0) perror("pthread_join()");if (App_result != PTHREAD_CANCELED) printf("thread %d not canceled\n", (list)->item[i].id);}
#else
#define STOP_ALL_LIST_THREADS(list) FORLISTP(list, i){pthread_cancel((list)->item[i].thread);} FORLISTP(list, i){void * result;pthread_join((list)->item[i].thread, &result);}
#endif

#define FUN_LOCK(T) int lock ## T (T *item) {if (item == NULL) {return 0;} if (pthread_mutex_lock(&(item->mutex.self)) != 0) {return 0;}return 1;}
#define FUN_TRYLOCK(T) int tryLock ## T (T  *item) {if (item == NULL) {return 0;} if (pthread_mutex_trylock(&(item->mutex.self)) != 0) {return 0;}return 1;}
#define FUN_UNLOCK(T) int unlock ## T (T *item) {if (item == NULL) {return 0;} if (pthread_mutex_unlock(&(item->mutex.self)) != 0) {return 0;}return 1;}

#define DEC_FUN_LOCK(T) extern int lock ## T (T *item);
#define DEC_FUN_TRYLOCK(T) extern int tryLock ## T (T  *item);
#define DEC_FUN_UNLOCK(T) extern int unlock ## T (T *item);

enum {
    APP_INIT = 90,
    APP_INIT_DATA,
    APP_RUN,
    APP_STOP,
    APP_RESET,
    APP_EXIT
} State;


typedef struct {
    char *buf;
    size_t *s1_offset;
    size_t buf_length;
    size_t buf_max_length;
    size_t length;
    size_t max_length;
} S1BList;
#define BLIST_ITEM(list,iname, ind)  ((list)->buf+(list)->iname##_offset[ind])
#define S1BLIST_INITIALIZER {.buf=NULL, .s1_offset=NULL, .buf_length=0, .buf_max_length=0, .length=0, .max_length=0}
#define FREE_S1BLIST(list) free((list)->s1_offset); free((list)->buf);  (list)->buf=NULL; (list)->s1_offset=NULL; (list)->buf_length=0;(list)->buf_max_length=0;(list)->length=0; (list)->max_length=0;
#define NULL_S1BLIST(list) memset((list)->buf,0,(list)->buf_max_length * sizeof (*(list)->buf));memset((list)->s1_offset,0,(list)->max_length * sizeof (*(list)->s1_offset));(list)->buf_length=0;(list)->length=0;

extern int s1blist_push(size_t min_buf_alloc_length, size_t min_item_alloc_length, S1BList *list, const char *str);

typedef struct {
    pthread_mutex_t self;
    pthread_mutexattr_t attr;
    int created;
    int attr_initialized;
} Mutex;
#define MUTEX_INITIALIZER {.self=PTHREAD_MUTEX_INITIALIZER, .created = 0, .attr_initialized = 0}
//#define MUTEX pthread_mutex_t
//#define DEF_MUTEX(V) pthread_mutex_t V = PTHREAD_MUTEX_INITIALIZER;
//#define IF_LOCK_MUTEX(P) if(pthread_mutex_lock(P) != 0)
//#define IF_TRYLOCK_MUTEX(P) if(pthread_mutex_trylock(P) != 0)
//#define UNLOCK_MUTEX(P) pthread_mutex_unlock(P)

struct channel_ts_st {
    int id;
    void * data;
    int save;
    uint32_t error_code;
    
    int sock_fd;
    struct timespec cycle_duration;
    pthread_t thread;
    Mutex mutex;
    struct channel_ts_st *next;
};


extern char * strcpyma(char **dest, char *src);

extern void putse(const char *str);

extern void printfe(const char *str, ...);

extern int file_exist(const char *filename);

extern int readConf(const char *path, char conninfo[LINE_SIZE], char app_class[NAME_SIZE]);

extern void conSig(void (*fn)());

extern void setPriorityMax(int policy);

extern int readHostName(char *hostname);

extern int initPid(int *pid_file, int *pid, const char *pid_path);

extern char * getAppState(char state);

extern void freePid(int *pid_file, int *pid, const char *pid_path);

extern int initMutex(Mutex *m);

extern void freeMutex(Mutex *m);

extern int lockMutex(Mutex *item);

extern int tryLockMutex(Mutex *item);

extern int unlockMutex(Mutex *item);

extern void skipLine(FILE* stream);

extern int createThread(pthread_t *new_thread,void *(*thread_routine) (void *),char *cmd);

extern int createMThread(pthread_t *new_thread, void *(*thread_routine) (void *), void * data);

extern int threadCancelDisable(int *old_state) ;

extern int threadSetCancelState(int state);



#endif 

