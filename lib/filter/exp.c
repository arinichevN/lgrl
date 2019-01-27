#include "exp.h"

int fexp_init(FilterEXP *item, int id, double a) {
    if (a < 0.0 || a > 1.0) {
        return 0;
    }
    item->id = id;
    item->a = a;
    item->f = 0;
    return 1;
}

int fexp_initList(FilterEXPList *list, const char *config_path) {
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
        LIi.a = TSVgetfs(r, i, "a");
        if (LIi.a < 0.0 || LIi.a > 1.0) {
#ifdef MODE_DEBUG
            fprintf(stderr, "%s(): 0 >= a <= 1 expected where id=%d\n", F, LIi.id);
#endif
            break;
        }
        LIi.f = 0;
        LIi.vp=0;
        if (TSVnullreturned(r)) {
            break;
        }
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

void fexp_calc(double *v, void *filter) {
    FilterEXP *item=filter;
    if (!item->f) {
        item->vp = *v;
        item->f = 1;
        return;
    }
    *v = item->a * item->vp + (1.0 - item->a)*(*v);
    item->vp = *v;
}

void fexp_free(FilterEXP *item) {
    item->vp = 0.0;
    item->a = 0.0;
}

void fexp_freeList(FilterEXPList *list) {
    FORLi{
        fexp_free(&LIi);
    }
    FREE_LIST(list);
}
