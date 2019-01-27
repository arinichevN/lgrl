
#ifndef LIBPAS_MCP3008_H
#define LIBPAS_MCP3008_H

#include "../gpio.h"


extern mcp3008_init(int sclk, int cs, int mosi, int miso);

extern int mcp3008_read(int *result, int id, int sclk, int cs, int mosi, int miso);


#endif 

