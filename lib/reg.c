
#include "reg.h"

char * reg_getStateStr(char state) {
    switch (state) {
        case REG_OFF:
            return "OFF";
        case REG_INIT:
            return "INIT";
        case REG_DO:
            return "DO";
        case REG_WAIT:
            return "WAIT";
        case REG_BUSY:
            return "BUSY";
        case REG_SECURE:
            return "SECURE";
        case REG_COOLER:
            return "COOLER";
        case REG_HEATER:
            return "HEATER";
        case REG_DISABLE:
            return "DISABLE";
        case REG_MODE_PID:
            return "PID";
        case REG_MODE_ONF:
            return "ONF";
    }
    return "";
}

 int reg_sensorRead ( RegSensor *item ){
     return acp_getRChannelFTS ( &item->input, &item->remote_channel );
 }

int reg_controlRChannel(RChannel *item, double output) {
    if (item == NULL) {
        return 0;
    }
    return acp_setRChannelFloat(item, output);
}

static int getRegSecureList_callback(void *data, int argc, char **argv, char **azColName) {
    RegSecureList *list = data;
    int c = 0;
#define Li list->item[list->length]
    for (int i = 0; i < argc; i++) {
        if (DB_COLUMN_IS("id")) {
            Li.id = atoi(argv[i]);
            c++;
        } else if (DB_COLUMN_IS("timeout_sec")) {
            Li.timeout.tv_sec = atoi(argv[i]);
            Li.timeout.tv_nsec = 0;
            c++;
        } else if (DB_COLUMN_IS("heater_duty_cycle")) {
            Li.heater_duty_cycle = atof(argv[i]);
            c++;
        } else if (DB_COLUMN_IS("cooler_duty_cycle")) {
            Li.cooler_duty_cycle = atof(argv[i]);
            c++;
        } else {
#ifdef MODE_DEBUG
            fprintf(stderr, "%s(): unknown column\n", __func__);
#endif
        }
    }

#define N 4
    if (c != N) {
        list->length++;
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): required %d columns but %d found\n", __func__, N, c);
#endif
        return EXIT_FAILURE;
    }
#undef N
    Li.active = 1;
    list->length++;
#undef Li
    return EXIT_SUCCESS;
}

int reg_getSecureFDB(RegSecure *item, int id, sqlite3 *dbl, const char *db_path) {
    item->id=id;
    if (dbl != NULL && db_path != NULL) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): dbl xor db_path expected\n", __func__);
#endif
        return 0;
    }
    sqlite3 *db;
    if (db_path != NULL) {
        if (!db_openR(db_path, &db)) {
#ifdef MODE_DEBUG
            fprintf(stderr, "%s(): failed to open database\n", __func__);
#endif
            return 0;
        }
    } else {
        db = dbl;
    }
    char q[LINE_SIZE];
    RegSecureList data = {.item = item, .length = 0, .max_length = 1};
    memset(item, 0, sizeof *item);
    snprintf(q, sizeof q, "SELECT id, timeout_sec, heater_duty_cycle, cooler_duty_cycle FROM secure where id=%d", id);
    if (!db_exec(db, q, getRegSecureList_callback, &data)) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): failed\n", __func__);
#endif
        if (db_path != NULL) {
            sqlite3_close(db);
        }
        return 0;
    }
    if (db_path != NULL) {
        sqlite3_close(db);
    }
    if (data.length != 1) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): secure with id=%d not found\n", __func__, id);
#endif
        return 0;
    }
    return 1;
}

int reg_secureNeed(RegSecure *item) {
    if (!item->active) {
        return 0;
    }
    if (item->done) {
        return 1;
    }
    if (ton_ts(item->timeout, &item->tmr)) {
        item->done = 1;
        BIT_ENABLE(*item->error_code, PROG_ERROR_NO_SIGNAL_FROM_CLIENT);
        return 1;
    }
    return 0;
}

void reg_secureTouch(RegSecure *item) {
    item->done = 0;
    ton_ts_reset(&item->tmr);
     BIT_DISABLE(*item->error_code, PROG_ERROR_NO_SIGNAL_FROM_CLIENT);
}

