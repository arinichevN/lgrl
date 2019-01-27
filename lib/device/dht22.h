
#ifndef LIBPAS_DHT22_H
#define LIBPAS_DHT22_H

#include <stdint.h>

#include "../timef.h"
#include "../gpio.h"

extern int dht22_read(int pin, double *t, double *h);
extern void dht22_readp ( int *pin, double *t, double *h,int *success, size_t length );
#endif

