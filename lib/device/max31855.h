
#ifndef LIBPAS_MAX31855_H
#define LIBPAS_MAX31855_H

#include "../gpio.h"

extern int max31855_init(int sclk, int cs, int miso);

extern int max31855_read(float *result, int sclk, int cs, int miso );

#endif 

