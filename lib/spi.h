
#ifndef LIBPAS_SPI_H
#define LIBPAS_SPI_H

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "common.h"

typedef struct {
    char *path;
    int fd;
    int speed;
    int bpw;
} SPI;

extern int spi_setup(SPI *item);

extern int spi_rw(SPI *item, unsigned char *data,unsigned int len);

#endif 

