#ifndef LIBPAS_REGPIDONF_H
#define LIBPAS_REGPIDONF_H

#include "timef.h"
#include "acp/main.h"
#include "reg.h"
#include "pid.h"

typedef struct {
    RChannel remote_channel;
    double delta;
    double output_min;
    double output_max;
    PID pid;
    double output;
    char mode;

} RegPIDOnfEM;

typedef struct {
    RegSensor sensor;
    RegPIDOnfEM em;
    
    double delta;
    PID pid;
    char mode;//heater or cooler
    double goal;

    char state;
    char state_r;
    char state_onf;
    int snsrf_count;
} RegPIDOnf;

extern void regpidonf_control(RegPIDOnf *item) ;

extern void regpidonf_enable(RegPIDOnf *item) ;

extern void regpidonf_disable(RegPIDOnf *item) ;

extern int regpidonf_getEnabled(const RegPIDOnf *item) ;

extern void regpidonf_setDelta(RegPIDOnf *item, double value) ;

extern void regpidonf_setKp(RegPIDOnf *item, double value) ;

extern void regpidonf_setKi(RegPIDOnf *item, double value) ;

extern void regpidonf_setKd(RegPIDOnf *item, double value) ;

extern void regpidonf_setGoal(RegPIDOnf *item, double value) ;

extern void regpidonf_setMode(RegPIDOnf *item, const char * value) ;

extern void regpidonf_setEMMode(RegPIDOnf *item, const char * value) ;

extern void regpidonf_setPower(RegPIDOnf *item, double value) ;

extern void regpidonf_turnOff(RegPIDOnf *item) ;

extern int regpidonf_check(const RegPIDOnf *item) ;

#endif 

