
#ifndef LIBPAS_PID_H
#define LIBPAS_PID_H

#include <stdlib.h>

#include "common.h"

#define PID_MODE_HEATER 'h'
#define PID_MODE_COOLER 'c'
#define PID_MODE_UNKNOWN 'u'
#define PID_TUNE_START_VALUE 100.0f
#define PID_TUNE_NOISE 1.0f
#define PID_TUNE_STEP 50.0f
#define PID_TUNE_LOOP_BACK 20




typedef struct {
    double integral_error;
    double previous_error;
    double kp, ki, kd;
    double max_output;
    double min_output;
    double previous_output;
    struct timespec previous_time;
    int reset;
    char mode;// PID_MODE_HEATER or PID_MODE_COOLER
} PID;

typedef struct {
    int isMax, isMin;
    double setpoint;
    double noiseBand;
    int controlType;
    int running;
    struct timespec peak1, peak2, lastTime;
    struct timespec sampleTime;
    int nLookBack;
    int peakType;
    double lastInputs[101];
    double peaks[10];
    int peakCount;
    int justchanged;
    int justevaled;
    double absMax, absMin;
    double oStep;
    double outputStart;
    double Ku, Pu;
}PID_AT;


extern double pid(PID *p, double set_point, double input);

extern double pid_mx(PID *p, double set_point, double input);

extern double pidwt(PID *p, double set_point, double input, struct timespec tm);

extern double pidwt_mx(PID *p, double set_point, double input, struct timespec tm) ;

extern void stopPid(PID *p);

extern int pidAutoTune(PID_AT *at, PID *p, double input, double *output) ;
#endif 

