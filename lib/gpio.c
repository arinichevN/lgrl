
#include "gpio.h"

#ifdef CPU_ALLWINNER_A20
//#include "gpio/allwinner_4.c"
#include "gpio/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H3
//#include "gpio/allwinner_2.c"
#include "gpio/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H2P
//#include "gpio/allwinner_2.c"
#include "gpio/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H5
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_R40
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A33
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A31s
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A64
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A83T
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ROCKCHIP_RK3288
#include "gpio/rk3288.c"
#else
//debugging mode (for machine with no GPIO)
#include "gpio/all.c"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
