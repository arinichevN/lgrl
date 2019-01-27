#ifndef LIBPAS_FILTER_COMMON_H
#define LIBPAS_FILTER_COMMON_H

#include <stdlib.h>
#include "../app.h"
#include "../dstructure.h"
#include "../tsv.h"
#include "ma.h"
#include "exp.h"

typedef struct {
    void *ptr;
    void (*fnc)(double *, void *);
} AbstractFilter;
DEC_LIST(AbstractFilter)

typedef struct  {
    int id;//in application it is usually copy of channel_id, we will get other filters by this id
    FilterMAList fma_list;
    FilterEXPList fexp_list;
    AbstractFilterList af_list;
}Filter;
DEC_LIST(Filter)
DEC_PLIST(Filter)

extern int filter_initFilterList(FilterList *list, const char *fma_path, const char *fexp_path, const char *mapping_path);
extern void filter_freeList ( FilterList *list );
#endif
