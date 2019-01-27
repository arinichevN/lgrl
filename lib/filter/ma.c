#include "ma.h"

int fma_init(FilterMA *item, int id, int length) {
    if (length < 0) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): length >= 0 expected where id=%d\n", F, item->id);
#endif
        return 0;
    }
    if (length == 0) {
        item->id = id;
        item->length = length;
        item->i = 0;
        item->c_length = 0;
        return 1;
    }
    item->buf = malloc(length * (sizeof *item->buf));
    if (item->buf == NULL) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): failure while allocating memory for FilterMA (id=%d) buffer\n", F, item->id);
#endif
        return 0;
    }
    memset(item->buf, 0, length*(sizeof *item->buf));
    item->id = id;
    item->length = length;
    item->i = 0;
    item->c_length = 0;
    return 1;
}

int fma_initList(FilterMAList *list, const char *config_path) {
    TSVresult tsv = TSVRESULT_INITIALIZER;
    TSVresult* r = &tsv;
    if (!TSVinit(r, config_path)) {
        TSVclear(r);
        return 0;
    }

    int n = TSVntuples(r);
    if (n <= 0) {
        TSVclear(r);
        return 1;
    }
    RESIZE_M_LIST(list, n);
    if (LML != n) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): failure while resizing list\n", F);
#endif
        TSVclear(r);
        return 0;
    }
    NULL_LIST(list);
    for (int i = 0; i < LML; i++) {
        LIi.id = TSVgetis(r, i, "id");
        LIi.length = TSVgetis(r, i, "length");
        LIi.i = 0;
        LIi.c_length = 0;
        if (TSVnullreturned(r)) {
            break;
        }
        LIi.buf = malloc(LIi.length * (sizeof *LIi.buf));
        if (LIi.buf == NULL) {
#ifdef MODE_DEBUG
            fprintf(stderr, "%s(): failure while allocating memory for item (id=%d) buffer\n", F, LIi.id);
#endif
            break;
        }
        memset(LIi.buf, 0, LIi.length*(sizeof *LIi.buf));
        LL++;
    }
    TSVclear(r);
    if (LL != LML) {
#ifdef MODE_DEBUG

        fprintf(stderr, "%s(): failure while reading rows\n", F);
#endif
        return 0;
    }
    return 1;
}

void fma_freeList(FilterMAList *list) {
    FORL{
        fma_free(&LIi);
    }
    FREE_LIST(list);
}

void fma_free(FilterMA *item) {
    free(item->buf);
    item->buf = NULL;
    item->c_length = 0;
    item->length = 0;
    item->i = 0;
}

void fma_calc(double *v, void *filter) {
    FilterMA *item = filter;
    if (item->length <= 0) {
        return;
    }
    if (item->i >= item->length) {
        item->i = 0;
    }
    item->buf[item->i] = *v;
    item->i++;
    if (item->c_length < item->length) {
        item->c_length++;
    }
    double s = 0.0;
    for (int i = 0; i < item->c_length; i++) {
        s += item->buf[i];
    }
    *v = s / item->c_length;
}

