
#ifndef LIBPAS_UTIL_H
#define LIBPAS_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"

extern void dumpStr(const char *buf) ;

extern void strnline(char **v) ;

extern char * bufCat(char * buf, const char * str, size_t size);

extern char * estostr(char *s);

extern double adifd(double v1, double v2) ;

extern int aeq(double v1, double v2, double acr);

extern int get_rand_fu(void *buf, size_t nbytes) ;

extern int get_rand(void *buf, size_t nbytes) ;

extern int get_rand_int(int min, int max) ;

extern unsigned int get_randb_uint(unsigned int min, unsigned int max) ;

extern unsigned int get_between_uint(unsigned int val,unsigned int min, unsigned int max);

#endif 

