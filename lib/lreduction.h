#ifndef LIBPAS_LREDUCTION_H
#define LIBPAS_LREDUCTION_H

#include <stdlib.h>
#include "app.h"
#include "dstructure.h"
#include "tsv.h"

typedef struct {
    int id;
    double min_in;
    double max_in;
    double min_out;
    double max_out;
} LReduction;

DEC_LIST(LReduction)

extern int initLReduction(LReductionList *list, const char *config_path);

extern void lreduct(double *out, LReduction *x);

#endif

