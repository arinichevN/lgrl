
#ifndef LIBPAS_CRC_H
#define LIBPAS_CRC_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "util.h"

extern void crc_update(uint8_t *crc, uint8_t b) ;

extern void crc_update_by_str(uint8_t *crc, const char *str);

#endif

