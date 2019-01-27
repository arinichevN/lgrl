
#ifndef LIBPAS_PWM_H
#define LIBPAS_PWM_H

#include <time.h>
#include "gpio.h"
#include "timef.h"

#define PWM_BUSY LOW
#define PWM_IDLE HIGH

typedef struct {
    int state;
    struct timespec start_time;
    struct timespec toggle_time;//time when we must change state from busy to idle or reverse
    struct timespec single_time;//(duty_cycle_max - duty_cycle_min)/resolution
    
    int resolution;//0 is 0% of power (duty_cycle will be set to PWM.duty_cycle_min) PWM.resolution is 100% of power (duty_cycle will be set to PWM.duty_cycle_max)
    struct timespec period;
    struct timespec duty_cycle_min; //0% of power
    struct timespec duty_cycle_max;//100% of power


} PWM;
extern int pwm_check ( PWM *item );
extern void pwm_init ( PWM *item ) ;
extern int pwm_setPeriod ( PWM *item, struct timespec v ) ;
extern int pwm_setResolution ( PWM *item, int v ) ;
extern int pwm_setDutyCycleMin ( PWM *item, struct timespec v ) ;
extern int pwm_setDutyCycleMax ( PWM *item, struct timespec v ) ;
extern int pwm_control( PWM *item, int duty_cycle );

#endif /* PWM_H */

