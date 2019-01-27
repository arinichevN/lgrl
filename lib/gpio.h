#ifndef LIBPAS_GPIO_H
#define LIBPAS_GPIO_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "timef.h"


#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

#define PUD_OFF 0
#define PUD_DOWN 2
#define PUD_UP 1


#define MODE_IN_STR "in"
#define MODE_OUT_STR "out"

#define PUD_OFF_STR "off"
#define PUD_DOWN_STR "down"
#define PUD_UP_STR "up"

extern void pinWrite(int pin, int value);
extern int pinRead(int pin);
extern void pinLow(int pin);
extern void pinHigh(int pin);
extern void pinModeIn(int pin);
extern void pinModeOut(int pin);
extern void pinPUD(int pin, int pud);
extern int checkPin(int pin);
extern int gpioSetup();
extern void gpioFree();


#endif 

