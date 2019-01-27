#ifndef LIBPAS_REG_H
#define LIBPAS_REG_H

#include "acp/main.h"
#include "dbl.h"

#define REG_MODE_PID_STR "pid"
#define REG_MODE_ONF_STR "onf"

#define SNSR_VAL item->sensor.input.value
#define SNSR_TM item->sensor.input.tm

#define VAL_IS_OUT_H SNSR_VAL > item->goal + item->heater.delta
#define VAL_IS_OUT_C SNSR_VAL < item->goal - item->cooler.delta
#define SNSRF_COUNT_MAX 7

#define REG_EM_MODE_COOLER_STR "cooler"
#define REG_EM_MODE_HEATER_STR "heater"
#define REG_EM_MODE_BOTH_STR "both"

typedef struct {
    int id;
    int active;
    struct timespec timeout;
    double heater_duty_cycle;
    double cooler_duty_cycle;
    Ton_ts tmr;
    int done;
    uint32_t *error_code;
} RegSecure;

DEC_LIST(RegSecure)

typedef struct {
    RChannel remote_channel;
    FTS input;
} RegSensor;

enum {
    REG_OFF,
    REG_INIT,
    REG_DO,
    REG_DISABLE,
    REG_WAIT,
    REG_BUSY,
    REG_SECURE,
    REG_COOLER,
    REG_HEATER,
    REG_MODE_PID,
    REG_MODE_ONF
} StateReg;

extern int reg_sensorRead ( RegSensor *item ) ;

extern char * reg_getStateStr(char state);

extern int reg_controlRChannel(RChannel *item, double output);

extern int reg_getSecureFDB(RegSecure *item, int id, sqlite3 *dbl, const char *db_path);

extern int reg_secureNeed(RegSecure *item);

extern void reg_secureTouch(RegSecure *item);

#endif

