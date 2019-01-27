#ifndef LIBPAS_REGPIDONFHC_H
#define LIBPAS_REGPIDONFHC_H

#include "timef.h"
#include "acp/main.h"
#include "reg.h"
#include "green_light.h"
#include "pid.h"

typedef struct {
    RChannel remote_channel;
    double delta;
    double output_min;
    double output_max;
    PID pid;
    double output;
    char mode;
    int use;
} RegPIDOnfHCEM;

typedef struct {
    RegSensor sensor;
    RegPIDOnfHCEM heater;
    RegPIDOnfHCEM cooler;
    double goal;
    struct timespec change_gap;
    RegSecure secure_out;
    GreenLight green_light;

    char state;
    char state_r;
    char state_onf;
    int snsrf_count;
    Ton_ts tmr;
    double output_pid;
} RegPIDOnfHC;

extern int regpidonfhc_init(RegPIDOnfHC *item, int *fd);

extern int regpidonfhc_check (const RegPIDOnfHC *item );

extern void regpidonfhc_control(RegPIDOnfHC *item);

extern void regpidonfhc_enable(RegPIDOnfHC *item);

extern void regpidonfhc_disable(RegPIDOnfHC *item);

extern int regpidonfhc_getEnabled(const RegPIDOnfHC *item);

extern void regpidonfhc_setCoolerDelta(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setHeaterDelta(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setHeaterKp(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setHeaterKi(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setHeaterKd(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setCoolerKp(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setCoolerKi(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setCoolerKd(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setGoal(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setEMMode(RegPIDOnfHC *item, const char * value);

extern void regpidonfhc_setHeaterMode(RegPIDOnfHC *item, const char * value);

extern void regpidonfhc_setCoolerMode(RegPIDOnfHC *item, const char * value);

extern void regpidonfhc_setChangeGap(RegPIDOnfHC *item, int value);

extern void regpidonfhc_setHeaterPower(RegPIDOnfHC *item, double value);

extern void regpidonfhc_setCoolerPower(RegPIDOnfHC *item, double value);

extern void regpidonfhc_turnOff(RegPIDOnfHC *item);

extern void regpidonfhc_secureOutTouch(RegPIDOnfHC *item);

#endif 

