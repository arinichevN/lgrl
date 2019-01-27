
#ifndef LIBPAS_MAX6675_H
#define LIBPAS_MAX6675_H

#include "../gpio.h"

typedef struct {
    int miso;
    float value;
    int state;
} MAX6675Data;

typedef struct {
    MAX6675Data *item;
    int length;
} MAX6675DataList;

extern int max6675_init(int sclk, int cs, int miso);

extern int max6675_read(float *result, int sclk, int cs, int miso );

extern int max6675_so_init(int sclk, int cs, int *miso, int miso_length);

extern void max6675_so_read(MAX6675DataList *data, int sclk, int cs);

#endif 

