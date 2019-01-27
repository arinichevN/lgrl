
#ifndef LIBPAS_SERIAL_H
#define LIBPAS_SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "timef.h"

extern int serialOpen(const char *device, const int baud);

extern int initSerial(int *fd, const char *device, const int baud);

extern void serialFlush(const int fd);

extern void serialClose(const int fd);

extern int serialAvailable(const int fd);

extern int serialWaitAvailable(int fd, struct timespec max_time);

extern int serialPuts(const int fd, char *str);

extern void serialRead(int fd, void *buf, size_t buf_size);

#endif

