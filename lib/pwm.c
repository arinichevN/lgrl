/*
 *  pulse-width modulation
 */
#include "pwm.h"


int pwm_check ( PWM *item ) {
    if ( item->resolution <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid resolution: %d\n", F, item->resolution );
#endif
        return 0;
    }
    if ( item->period.tv_sec <= 0 && item->period.tv_nsec <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid period: %ld sec %ld nsec\n", F, item->period.tv_sec , item->period.tv_nsec );
#endif
        return 0;
    }
    if ( timespeccmp ( &item->period, &item->duty_cycle_max, < ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid period or duty_cycle_max\n", F );
#endif
        return 0;
    }
    if ( timespeccmp ( &item->duty_cycle_max, &item->duty_cycle_min, <= ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid duty_cycle_max or duty_cycle_min\n", F );
#endif
        return 0;
    }
    return 1;
}
static void calc_single_time ( PWM *item ) {
    //next lines shortly: item->single_time = ( item->duty_cycle_max - item->duty_cycle_min ) / item->resolution;
    struct timespec dif;
    timespecsub ( &item->duty_cycle_max, &item->duty_cycle_min, &dif );
    double single_time_d;
    timespec2double ( single_time_d, &dif );
    single_time_d=single_time_d / item->resolution;
    double2timespec ( &item->single_time, single_time_d );
//   /* */ printf ( "INIT: single_time_d=%f a1=%f a1t=%ld s %ld ns\n",single_time_d, item->single_time.tv_sec, item->single_time.tv_nsec );
}

//period, resolution, duty_cycle_min, duty_cycle_max are expected to has been set and checked
void pwm_init ( PWM *item ) {
    struct timespec  now = getCurrentTime();
    item->start_time = now;
    timespecclear ( &item->toggle_time );
    calc_single_time ( item );
    item->state = PWM_IDLE;
}

//use pwm_set... functions when pwm_control() is running. when first initializing PWM structure, access its fields directly and finally call pwm_check()
int pwm_setPeriod ( PWM *item, struct timespec v ) {
    if ( timespeccmp ( &v, &item->duty_cycle_max, < ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid period\n", F );
#endif
        return 0;
    }
    item->period=v;
    pwm_init ( item );
    return 1;
}
int pwm_setResolution ( PWM *item, int v ) {
    if ( v <= 0 ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid resolution: %d\n", F, item->resolution );
#endif
        return 0;
    }
    item->resolution=v;
    pwm_init ( item );
    return 1;
}
int pwm_setDutyCycleMin ( PWM *item, struct timespec v ) {
    if ( timespeccmp ( &item->duty_cycle_max, &item->duty_cycle_min, <= ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid duty_cycle_min\n", F );
#endif
        return 0;
    }
    item->duty_cycle_min=v;
    pwm_init ( item );
    return 1;
}
int pwm_setDutyCycleMax ( PWM *item, struct timespec v ) {
    if ( timespeccmp ( &item->period, &item->duty_cycle_max, < ) ) {
#ifdef MODE_DEBUG
        fprintf ( stderr, "%s(): invalid duty_cycle_max\n", F );
#endif
        return 0;
    }
    item->duty_cycle_max=v;
    pwm_init ( item );
    return 1;
}
int pwm_control ( PWM *item, int duty_cycle ) {
    if ( duty_cycle >= item->resolution ) {
        item->state = PWM_BUSY;
        return item->state;
    }
    if ( duty_cycle <= 0 ) {
        item->state = PWM_IDLE;
        return item->state;
    }
    struct timespec  now = getCurrentTime();
    switch ( item->state ) {
    case PWM_BUSY: {
        if ( timespeccmp ( &now, &item->toggle_time, >= ) ) {
            item->start_time = now;
            timespecclear ( &item->toggle_time );
            //next lines shortly: item->toggle_time=item->start_time + item->period - item->single_time * duty_cycle;
            double item_single_time=item->single_time.tv_sec + ( double ) item->single_time.tv_nsec*NANO_FACTOR;
            double a1=item_single_time*duty_cycle;
            struct timespec a1t;
            double2timespec ( &a1t, a1 );
            timespecadd ( &item->toggle_time, &item->start_time, &item->toggle_time );
            timespecadd ( &item->toggle_time, &item->period, &item->toggle_time );
            timespecsub ( &item->toggle_time, &a1t, &item->toggle_time );
            item->state = PWM_IDLE;
        }

        break;
    }
    case PWM_IDLE: {
        if ( timespeccmp ( &now, &item->toggle_time, >= ) ) {
            item->start_time = now;
            timespecclear ( &item->toggle_time );
            //next lines shortly: item->toggle_time=item->start_time + item->single_time * duty_cycle);
            double item_single_time=item->single_time.tv_sec + ( double ) item->single_time.tv_nsec*NANO_FACTOR;
            double a1=item_single_time*duty_cycle;
            struct timespec a1t;
            double2timespec ( &a1t, a1 );
            timespecadd ( &item->toggle_time, &item->start_time, &item->toggle_time );
            timespecadd ( &item->toggle_time, &a1t, &item->toggle_time );
            item->state = PWM_BUSY;
        }
        break;
    }
    default:
        item->state = PWM_IDLE;
        break;
    }
    return item->state;
}

