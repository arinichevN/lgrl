#include "common.h"

static int countChannelMaExpLength(int *ma_length, int *exp_length, int channel_id, TSVresult* r_ma, TSVresult* r_exp, TSVresult* r_map, int n_map, int n_ma, int n_exp) {
    *ma_length = *exp_length = 0;
    for (int j = 0; j < n_map; j++) {
        int fchannel_id = TSVgetis(r_map, j, "channel_id");
        if (TSVnullreturned(r_map)) {
            return 0;
        }
        if (channel_id == fchannel_id) {
            (*ma_length)++;
            (*exp_length)++;
        }
    }
    return 1;
}

static int fillFilterLists(FilterMAList *ma_list, FilterEXPList *exp_list, AbstractFilterList *af_list, int channel_id, TSVresult* r_ma, TSVresult* r_exp, TSVresult* r_map, int n_map, int n_ma, int n_exp) {
    for (int j = 0; j < n_map; j++) {
        int fchannel_id = TSVgetis(r_map, j, "channel_id");
        if (TSVnullreturned(r_map)) {
            return 0;
        }
        if (channel_id == fchannel_id) {
            int filter_id = TSVgetis(r_map, j, "filter_id");
            if (TSVnullreturned(r_map)) {
                return 0;
            }
            for (int k = 0; k < n_ma; k++) {
                int filter_p_id = TSVgetis(r_ma, k, "id");
                int filter_p_length = TSVgetis(r_ma, k, "length");
                if (TSVnullreturned(r_ma)) {
                    return 0;
                }
                if (filter_p_id == filter_id) {
                    if (ma_list->length >= ma_list->max_length) {
                        printde("ma_list overflow where channel_id=%d and filter_id=%d\n", channel_id, filter_id);
                        return 0;
                    }
                    if (!fma_init(&ma_list->item[ma_list->length], filter_p_id, filter_p_length)) {
                        return 0;
                    }
                    ma_list->length++;
                    if (af_list->length >= af_list->max_length) {
                        printde("af_list overflow where channel_id=%d and filter_id=%d\n", channel_id, filter_id);
                        return 0;
                    }
                    af_list->item[af_list->length].ptr = &ma_list->item[ma_list->length-1];
                    af_list->item[af_list->length].fnc = fma_calc;
                    af_list->length++;
                }
            }
            for (int k = 0; k < n_exp; k++) {
                int filter_p_id = TSVgetis(r_exp, k, "id");
                float filter_p_a = TSVgetfs(r_exp, k, "a");
                if (TSVnullreturned(r_exp)) {
                    return 0;
                }
                if (filter_p_id == filter_id) {
                    if (exp_list->length >= exp_list->max_length) {
                        printde("exp_list overflow where channel_id=%d and filter_id=%d\n", channel_id, filter_id);
                        return 0;
                    }
                    if (!fexp_init(&exp_list->item[exp_list->length], filter_p_id, filter_p_a)) {
                        return 0;
                    }
                    exp_list->length++;
                    if (af_list->length >= af_list->max_length) {
                        printde("af_list overflow where channel_id=%d and filter_id=%d\n", channel_id, filter_id);
                        return 0;
                    }
                    af_list->item[af_list->length].ptr = &exp_list->item[exp_list->length-1];
                    af_list->item[af_list->length].fnc = fexp_calc;
                    af_list->length++;
                }
            }
        }
    }
    return 1;
}

int filter_initFilterList(FilterList *list, const char *fma_path, const char *fexp_path, const char *mapping_path) {
#define CLEAR_TSV_LIB  TSVclear(r_map);TSVclear(r_exp);TSVclear(r_ma);
#define RETURN_FAILURE  CLEAR_TSV_LIB return 0;
    TSVresult tsv1 = TSVRESULT_INITIALIZER;
    TSVresult* r_ma = &tsv1;
    if (!TSVinit(r_ma, fma_path)) {
        TSVclear(r_ma);
        return 0;
    }
    TSVresult tsv2 = TSVRESULT_INITIALIZER;
    TSVresult* r_exp = &tsv2;
    if (!TSVinit(r_exp, fexp_path)) {
        TSVclear(r_exp);
        TSVclear(r_ma);
        return 0;
    }
    TSVresult tsv3 = TSVRESULT_INITIALIZER;
    TSVresult* r_map = &tsv3;
    if (!TSVinit(r_map, mapping_path)) {
        RETURN_FAILURE;
    }
    int n_map = TSVntuples(r_map);
    int n_ma = TSVntuples(r_ma);
    int n_exp = TSVntuples(r_exp);
    FORLi{
        int ma_length = 0;
        int exp_length = 0;
        if (!countChannelMaExpLength(&ma_length, &exp_length, LIi.id, r_ma, r_exp, r_map, n_map, n_ma, n_exp)) {
            RETURN_FAILURE;
        }
        RESET_LIST(&LIi.fma_list);
        RESIZE_M_LIST(&LIi.fma_list, ma_length);
        if (LIi.fma_list.max_length != ma_length) {
            printde("failure while resizing fma_list where channel_id=%d\n", LIi.id);
            RETURN_FAILURE;
        }
        RESET_LIST(&LIi.fexp_list);
        RESIZE_M_LIST(&LIi.fexp_list, exp_length);
        if (LIi.fexp_list.max_length != exp_length) {
            printde("failure while resizing fexp_list where channel_id=%d\n", LIi.id);
            RETURN_FAILURE;
        }
        int f_length = exp_length + ma_length;
        RESET_LIST(&LIi.af_list);
        RESIZE_M_LIST(&LIi.af_list, f_length);
        if (LIi.af_list.max_length != f_length) {
            printde( "failure while resizing af_list where channel_id=%d\n", LIi.id);
            RETURN_FAILURE;
        }
        if (!fillFilterLists(&LIi.fma_list, &LIi.fexp_list, &LIi.af_list, LIi.id, r_ma, r_exp, r_map, n_map, n_ma, n_exp)) {
            RETURN_FAILURE;
        }
    }
    CLEAR_TSV_LIB;
    return 1;
#undef CLEAR_TSV_LIB
#undef RETURN_FAILURE 
}

void filter_freeList ( FilterList *list ) {
    FORLi {
        FREE_LIST ( &LIi.af_list );
        fma_freeList ( &LIi.fma_list );
        fexp_freeList ( &LIi.fexp_list );
    }
    FREE_LIST ( list );
}
