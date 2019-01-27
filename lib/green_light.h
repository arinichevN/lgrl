#ifndef LIBPAS_GREEN_LIGHT_H
#define LIBPAS_GREEN_LIGHT_H

#include "acp/main.h"

typedef struct {
    int id;
    RChannel remote_channel;
    double value;
    int active;
} GreenLight;
DEC_LIST(GreenLight)

extern int greenLight_init ( GreenLight *item, int *fd ) ;
extern int greenLight_isGreen(GreenLight *item);

#endif

