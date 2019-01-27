#include "timef.h"

struct timespec getCurrentTime() {
    struct timespec now;
    clock_gettime ( LIB_CLOCK, &now );
    return now;
}


void delayTsBusy ( struct timespec interval ) {
    struct timespec now, end;
    clock_gettime ( CLOCK_MONOTONIC, &now );
    timespecadd ( &now, &interval, &end );
    while ( timespeccmp ( &now, &end, < ) ) {
        clock_gettime ( CLOCK_MONOTONIC, &now );
    }
}

void delayUsBusy ( unsigned int td ) {
    struct timespec now, interval, end;
    clock_gettime ( CLOCK_MONOTONIC, &now );
    usec2timespec ( td, &interval )
    timespecadd ( &now, &interval, &end );
    while ( timespeccmp ( &now, &end, < ) ) {
        clock_gettime ( CLOCK_MONOTONIC, &now );
    }
}

void delayUsBusyC ( unsigned int td ) {
    clock_t now, interval, end;
    now = clock();
    interval = td * CLOCKS_PER_SEC / 1000000;
    end = now + interval;
    while ( now < end ) {
        now = clock();
    }
}

void delayUsIdle ( unsigned int td ) {
    struct timespec requested;
    usec2timespec ( td, &requested )
    nanosleep ( &requested, NULL );
}
void delayTsBusyRest ( struct timespec interval, struct timespec start ) {
    struct timespec now, end;
    timespecadd ( &start, &interval, &end );
    do {
        clock_gettime ( CLOCK_MONOTONIC, &now );
    } while ( timespeccmp ( &now, &end, < ) );
}

void delayTsIdleRest ( struct timespec interval, struct timespec start ) {
    struct timespec end;
    timespecadd ( &start, &interval, &end );
    clock_nanosleep ( LIB_CLOCK, TIMER_ABSTIME,&end, NULL );
}

void sleepRest ( struct timespec total, struct timespec start ) {
    struct timespec now, dif1, dif2;
    clock_gettime ( CLOCK_MONOTONIC, &now );
    timespecsub ( &now, &start, &dif1 );
    if ( timespeccmp ( &total, &dif1, > ) ) {
        timespecsub ( &total, &dif1, &dif2 );
        nanosleep ( &dif2, NULL );
    }
}

struct timespec usToTimespec ( long int us ) {
    struct timespec t = {0, 0};
    if ( us <= 0 ) {
        return t;
    }
    t.tv_sec = us / 1000000;
    t.tv_nsec = ( us % 1000000 ) * 1000;
    return t;
}

void getDate ( TOY *v_toy, int *wday, int *tod, int *y ) {
    struct tm *current;
    time_t now;
    time ( &now );
    current = localtime ( &now );
    v_toy->month = current->tm_mon;
    v_toy->mday = current->tm_mday;
    *wday = current->tm_wday;
    v_toy->tod = current->tm_sec + current->tm_min * 60 + current->tm_hour * 3600;
    v_toy->hour = current->tm_hour;
    v_toy->min = current->tm_min;
    v_toy->sec = current->tm_sec;
    *tod = v_toy->tod;
    *y = current->tm_year + 1900;
}

long int getCurrTOD() {
    struct tm *current;
    time_t now;
    time ( &now );
    current = localtime ( &now );
    return current->tm_sec + current->tm_min * 60 + current->tm_hour * 3600;
}

int ton_ts ( struct timespec interval, Ton_ts *t ) {
    struct timespec now, dif;
    if ( !t->ready ) {
        clock_gettime ( LIB_CLOCK, &t->start );
        t->ready = 1;
    }
    clock_gettime ( LIB_CLOCK, &now );
    timespecsub ( &now, &t->start, &dif );
    if ( timespeccmp ( &dif, &interval, > ) ) {
        t->ready = 0;
        return 1;
    }
    return 0;
}


void ton_ts_reset ( Ton_ts *t ) {
    t->ready=0;
}

void ton_ts_touch ( Ton_ts *t ) {
    clock_gettime ( LIB_CLOCK, &t->start );
    t->ready=1;
}

int ton ( Ton *item ) {
    struct timespec now;
    clock_gettime ( LIB_CLOCK, &now );
    if ( !item->ready ) {
        item->start=now;
        timespecadd ( &item->start, &item->interval, &item->end );
        item->ready = 1;
    }
    if ( timespeccmp ( &now, &item->end, > ) ) {
        item->ready = 0;
        return 1;
    }
    return 0;
}

int toni (struct timespec interval, Ton *item ) {
    struct timespec now;
    clock_gettime ( LIB_CLOCK, &now );
    if ( !item->ready ) {
        item->start=now;
        item->ready = 1;
    }
    timespecadd(&item->start, &interval, &item->end);
    if ( timespeccmp ( &now, &item->end, > ) ) {
        item->ready = 0;
        return 1;
    }
    return 0;
}

void tonSetInterval ( struct timespec interval, Ton *item ) {
    item->interval=interval;
    timespecadd ( &item->start, &item->interval, &item->end );
}

void tonReset ( Ton *item ) {
    item->ready=0;
}

struct timespec tonTimePassed ( const Ton *item ) {
    struct timespec now, out;
    clock_gettime ( LIB_CLOCK, &now );
    timespecsub ( &now, &item->start, &out );
    return out;
}

struct timespec tonTimeRest ( const Ton *item ) {
    struct timespec now, out;
    clock_gettime ( LIB_CLOCK, &now );
    if ( timespeccmp ( &item->end, &now, > ) ) {
        timespecsub ( &item->end, &now, &out );
    } else {
        out.tv_sec=0;
        out.tv_nsec=0;
    }
    return out;
}

struct timespec getTimePassed_tv ( const Ton_ts *t ) {
    struct timespec now, dif;
    clock_gettime ( LIB_CLOCK, &now );
    timespecsub ( &now, &t->start, &dif );
    return dif;
}

struct timespec getTimePassed_ts ( struct timespec t ) {
    struct timespec now, dif;
    clock_gettime ( LIB_CLOCK, &now );
    timespecsub ( &now, &t, &dif );
    return dif;
}

struct timespec getTimeRest_ts ( struct timespec t_interval, struct timespec t_start ) {
    struct timespec now, out, sum;
    clock_gettime ( LIB_CLOCK, &now );
    timespecadd ( &t_interval, &t_start, &sum );
    timespecsub ( &sum, &now, &out );
    return out;
}

struct timespec getTimeRestTmr ( struct timespec interval, Ton_ts tmr ) {
    struct timespec out = {-1, -1};
    if ( tmr.ready ) {
        out = getTimeRest_ts ( interval, tmr.start );
    }
    return out;
}

int toyHasCome ( const TOY *current, const TOY *wanted ) {
    if (
        current->month == wanted->month &&
        current->mday == wanted->mday &&
        todHasCome ( wanted->tod, current->tod ) == 1
    ) {
        return 1;
    }
    return 0;
}

/*
 * 0 - target time is ahead
 * 1 - target time has come
 * 2 - target time is behind
 */
int todHasCome ( long int target, long int current ) {
    if ( target > current ) {
        return 0;
    }
    if ( target + GOOD_TOD_DELAY < current ) {
        return 2;
    }
    return 1;
}

int timeHasPassed ( struct timespec interval, struct timespec start, struct timespec now ) {
    struct timespec dif;
    timespecsub ( &now, &start, &dif );
    if ( timespeccmp ( &interval, &dif, < ) ) {
        return 1;
    }
    return 0;
}

void changeInt ( int *v, int inc ) {
    if ( *v + inc > 0 ) {
        *v = *v + inc;
    }
}
