
#ifndef LIBPAS_TIMEF_H
#define LIBPAS_TIMEF_H

#include <limits.h>
#include <time.h>
#include <sys/time.h>

#include "common.h"

#define GOOD_TOD_DELAY 3
#define TIME_T_MAX LONG_MAX
#define LIB_CLOCK CLOCK_REALTIME 

#define TARGET_AHEAD 0
#define TARGET_OK 1
#define TARGET_BEHIND 2
#define MONTH_NUM 12
#define WDAY_NUM 7
//CLOCK_PROCESS_CPUTIME_ID

//interval < 1000 usec
#define DELAY_US_BUSY(interval) delayTsBusy ( (struct timespec) {0,interval*1000} )
#define timespec2double(d, ts) d=(ts)->tv_sec + (double) (ts)->tv_nsec*NANO_FACTOR
#define double2timespec(ts, d)  (ts)->tv_sec = d; (ts)->tv_nsec = ( d - (long int ) d ) / NANO_FACTOR;
#define usec2timespec(us, ts)             \
  (ts)->tv_sec = (time_t) (us / 1000000);            \
  (ts)->tv_nsec = (long) (us % 1000000) * 1000;

#define timespeccmp(a, b, CMP)             \
  (((a)->tv_sec == (b)->tv_sec) ?            \
   ((a)->tv_nsec CMP (b)->tv_nsec) :            \
   ((a)->tv_sec CMP (b)->tv_sec))

#define timespecadd(a, b, result)            \
  do {               \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;         \
    (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;         \
    if ((result)->tv_nsec >= 1000000000)           \
      {               \
        ++(result)->tv_sec;            \
        (result)->tv_nsec -= 1000000000;           \
      }               \
  } while (0)

#define timespecsub(a, b, result)            \
  do {               \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;         \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;         \
    if ((result)->tv_nsec < 0) {           \
      --(result)->tv_sec;            \
      (result)->tv_nsec += 1000000000;           \
    }               \
  } while (0)
#define timespecclear(tvp) ((tvp)->tv_sec = (tvp)->tv_nsec = 0L)

typedef struct {
    int month;
    int mday;
    int tod;
    int hour;
    int min;
    int sec;
} TOY; //time of year

typedef struct {
    struct timespec start;
    int ready;
} Ton_ts;

typedef struct {
    struct timespec start;
    struct timespec interval;
    struct timespec end;
    int ready;
} Ton;

extern struct timespec getCurrentTime();

extern void delayUsBusy(unsigned int td);

extern void delayTsBusy(struct timespec interval);

extern void delayUsBusyC(unsigned int td);

extern void delayTsBusyRest ( struct timespec interval, struct timespec start );

extern void delayTsIdleRest ( struct timespec interval, struct timespec start );

extern void delayUsIdle(unsigned int td);

extern void sleepRest(struct timespec total, struct timespec start);

extern struct timespec usToTimespec(long int us);

extern void getDate(TOY *v_toy, int *wday, int *tod, int *y);

extern int ton_ts(struct timespec interval, Ton_ts *t);

extern void ton_ts_reset(Ton_ts *t);

extern void ton_ts_touch(Ton_ts *t);

extern int ton ( Ton *item ) ;

extern int toni (struct timespec interval, Ton *item );

extern void tonSetInterval ( struct timespec interval, Ton *item  ) ;

extern void tonReset ( Ton *item  ) ;

extern struct timespec tonTimePassed ( const Ton *item ) ;

extern struct timespec tonTimeRest ( const Ton *item ) ;

extern struct timespec getTimePassed_tv(const Ton_ts *t);

extern struct timespec getTimePassed_ts(struct timespec t);

extern struct timespec getTimeRest_ts(struct timespec t_interval, struct timespec t_start);

extern struct timespec getTimeRestTmr(struct timespec interval, Ton_ts tmr);

extern int toyHasCome(const TOY *current, const TOY *wanted);

extern int todHasCome(long int target, long int current);

extern int timeHasPassed(struct timespec interval, struct timespec start, struct timespec now);

extern void changeTimeT(time_t *slave, time_t change);

extern void changeInt(int *v, int inc);

extern long int getCurrTOD();


#endif 

