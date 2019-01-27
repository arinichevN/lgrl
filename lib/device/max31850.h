
#ifndef LIBPAS_MAX31850_H
#define LIBPAS_MAX31850_H

#include <stdio.h>
#include <stdint.h>

#include "../timef.h"
#include "../1w.h"

#define MAX31850_SCRATCHPAD_BYTE_NUM 8 
#define MAX31850_EEPROM_BYTE_NUM 3
#define MAX31850_SCRATCHPAD_CONFIG_REG 4 

#define MAX31850_CMD_CONVERTT 0x44
#define MAX31850_CMD_READ_SCRATCHPAD 0xBE
#define MAX31850_CMD_READ_POWER_SUPPLY 0xB4


int max31850_read_scratchpad(int pin, const uint8_t *addr, uint8_t *sp);

void max31850_wait_convertion(int pin);

int max31850_convert_t(int pin, const uint8_t *addr);

int max31850_convert_t_all(int pin);

int max31850_read_temp(int pin, const uint8_t *addr, float * temp);

int max31850_get_temp(int pin, const uint8_t *addr, float * temp);

#endif /* MAX31850_H */

