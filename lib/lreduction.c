#include "lreduction.h"

int initLReduction(LReductionList *list, const char *config_path) {
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
        LIi.min_in = TSVgetfs(r, i, "min_in");
        LIi.max_in = TSVgetfs(r, i, "max_in");
        LIi.min_out = TSVgetfs(r, i, "min_out");
        LIi.max_out = TSVgetfs(r, i, "max_out");
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

void lreduct(double *out, LReduction *x) {
    if (x == NULL) {
        return;
    }
    double in = *out;
    double d_in = x->max_in - x->min_in;
    double d_out;
    if (x->max_out >= x->min_out) {
        d_out = x->max_out - x->min_out;
        *out = (in / d_in) * d_out;
    } else {
        d_out = (x->min_out - x->max_out);
        *out = ((d_in - in) / d_in) * d_out;
    }
}
