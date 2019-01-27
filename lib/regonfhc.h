#ifndef LIBPAS_REGONFHC_H
#define LIBPAS_REGONFHC_H

#include "timef.h"
#include "acp/main.h"
#include "reg.h"
#include "green_light.h"

typedef struct {
    RChannel remote_channel;
    double delta;
    double output;
    double output_max;
    double output_min;
    int use;
} RegOnfHCEM;

typedef struct {
    RegSensor sensor;
    RegOnfHCEM heater;
    RegOnfHCEM cooler;
    double goal;
    struct timespec change_gap;
    RegSecure secure_out;
    GreenLight green_light;
    
    char state;
    char state_r;
    char state_onf;
    int snsrf_count;
    Ton_ts tmr;
} RegOnfHC;

extern void regonfhc_control(RegOnfHC *item);

extern void regonfhc_enable(RegOnfHC *item);

extern void regonfhc_disable(RegOnfHC *item);

extern int regonfhc_getEnabled(const RegOnfHC *item);

extern void regonfhc_setCoolerDelta(RegOnfHC *item, double value);

extern void regonfhc_setHeaterDelta(RegOnfHC *item, double value);

extern void regonfhc_setGoal(RegOnfHC *item, double value);

extern void regonfhc_setChangeGap(RegOnfHC *item, int value);

extern void regonfhc_setHeaterPower(RegOnfHC *item, double value);

extern void regonfhc_setCoolerPower(RegOnfHC *item, double value);

extern void regonfhc_setEMMode(RegOnfHC *item, const char * value);

extern void regonfhc_turnOff(RegOnfHC *item);

extern void regonfhc_secureOutTouch(RegOnfHC *item);

#endif 

