#include "dbl.h"

int db_open(const char *path, sqlite3 **db) {
    char q[LINE_SIZE * 2];
    snprintf(q, sizeof q, "file://%s", path);
    int rc = sqlite3_open_v2(path, db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) {
        printde("path: %s, status: %d, message: %s\n", path, rc, sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return 0;
    }
    return 1;
}

int db_openR(const char *path, sqlite3 **db) {
    char q[LINE_SIZE * 2];
    snprintf(q, sizeof q, "file://%s", path);
    int rc = sqlite3_open_v2(path, db, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) {
        printde("path: %s, status: %d, message: %s\n", path, rc, sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return 0;
    }
    //int rs= sqlite3_busy_timeout(*db, 2000);
    return 1;
}

sqlite3 *db_openAlt(sqlite3 *dbl, const char *db_path, int *close){
    *close=0;
    if(dbl!=NULL) return dbl;
    if (db_path != NULL) {
        sqlite3 *db;
        if (!db_open(db_path, &db)) {
            putsde(" failed\n");
            return NULL;
        }
        *close=1;
        return db;
    }
    putsde("dbl or db_path expected\n");
    return NULL;
}

sqlite3 *db_openRAlt(sqlite3 *dbl, const char *db_path, int *close){
    *close=0;
    if(dbl!=NULL) return dbl;
    if (db_path != NULL) {
        sqlite3 *db;
        if (!db_openR(db_path, &db)) {
            putsde(" failed\n");
            return NULL;
        }
        *close=1;
        return db;
    }
    putsde("dbl or db_path expected\n");
    return NULL;
}

void db_close(sqlite3 *db){
	sqlite3_close_v2(db);
}
int db_exec(sqlite3 *db, char *q, int (*callback)(void*, int, char**, char**), void * data) {
    char *errMsg = 0;
    int rc = sqlite3_exec(db, q, callback, data, &errMsg);
    if (rc != SQLITE_OK) {
        printde("status: %d, query: %s, message: %s\n", rc, q, errMsg);
        sqlite3_free(errMsg);
        return 0;
    }
    return 1;
}

static int getInt_callback(void *data, int argc, char **argv, char **azColName) {
    int * item = data;
    if (argc >= 1) {
        *item = atoi(argv[0]);
    }
    return 0;
}

int db_getInt(int *item, sqlite3 *db, char *q) {
    void *data = item;
    if (!db_exec(db, q, getInt_callback, data)) {
        putsde("failed\n");
        return 0;
    }
    return 1;
}

int db_saveTableFieldFloat(const char * table, const char *field, int id, float value, sqlite3 *dbl, const char* db_path) {
    if (dbl != NULL && db_path != NULL) {
        putsde("dbl xor db_path expected\n");
        return 0;
    }
    sqlite3 *db;
    int close = 0;
    if (db_path != NULL) {
        if (!db_open(db_path, &db)) {
            putsde("failed\n");
            return 0;
        }
        close = 1;
    } else {
        db = dbl;
    }
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "update '%s' set '%s'=%f where id=%d", table, field, value, id);
    if (!db_exec(db, q, 0, 0)) {
        putsde("failed\n");
        if (close) sqlite3_close(db);
        return 0;
    }
    if (close) sqlite3_close(db);
    return 1;
}

int db_saveTableFieldInt(const char * table, const char *field, int id, int value, sqlite3 *dbl, const char* db_path) {
    if (dbl != NULL && db_path != NULL) {
        putsde("dbl xor db_path expected\n");
        return 0;
    }
    sqlite3 *db;
    int close = 0;
    if (db_path != NULL) {
        if (!db_open(db_path, &db)) {
            putsde("failed\n");
            return 0;
        }
        close = 1;
    } else {
        db = dbl;
    }
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "update '%s' set '%s'=%d where id=%d", table, field, value, id);
    if (!db_exec(db, q, 0, 0)) {
        putsde("failed\n");
        if (close) sqlite3_close(db);
        return 0;
    }
    if (close) sqlite3_close(db);
    return 1;
}

int db_saveTableFieldText(const char * table, const char *field, int id, const char * value, sqlite3 *dbl, const char* db_path) {
    if (dbl != NULL && db_path != NULL) {
        putsde("dbl xor db_path expected\n");
        return 0;
    }
    sqlite3 *db;
    int close = 0;
    if (db_path != NULL) {
        if (!db_open(db_path, &db)) {
            putsde("failed\n");
            return 0;
        }
        close = 1;
    } else {
        db = dbl;
    }
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "update '%s' set '%s'='%s' where id=%d", table, field, value, id);
    if (!db_exec(db, q, 0, 0)) {
        putsde("failed\n");
        if (close) sqlite3_close(db);
        return 0;
    }
    if (close) sqlite3_close(db);
    return 1;
}
