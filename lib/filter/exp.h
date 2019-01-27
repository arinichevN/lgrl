#ifndef LIBPAS_FILTER_EXP_H
#define LIBPAS_FILTER_EXP_H

#include <stdlib.h>
#include "../app.h"
#include "../dstructure.h"
#include "../tsv.h"

typedef struct {
    int id;
    double a;
    double vp;
    int f;
} FilterEXP;

#define FILTER_EXP_INITIALIZER {.f=0}

DEC_LIST(FilterEXP)

extern int fexp_init(FilterEXP *item, int id, double a);
extern int fexp_initList(FilterEXPList *list, const char *config_path);
extern void fexp_calc(double *v, void *filter);
extern void fexp_freeList(FilterEXPList *list);
extern void fexp_free(FilterEXP *item);

#endif

