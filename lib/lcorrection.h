#ifndef LIBPAS_LCORRECTION_H
#define LIBPAS_LCORRECTION_H

#include <stdlib.h>
#include "app.h"
#include "dstructure.h"
#include "tsv.h"

typedef struct {
    int id;
    double factor;
    double delta;
} LCorrection;

DEC_LIST(LCorrection)

extern int initLCorrection(LCorrectionList *list, const char *config_path);
extern void lcorrect(double *out, LCorrection *x);
#endif

