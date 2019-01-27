
#include "max6675.h"

static void printInt16(uint16_t d) {
    int i;
    for (i = 15; i >= 0; i--) {
        int v = (d >> i) & 1;
        printf("%d", v);
    }
    puts("");
}

int max6675_init(int sclk, int cs, int miso) {
    pinModeOut(cs);
    pinModeOut(sclk);
    pinModeIn(miso);
    pinPUD(miso, PUD_DOWN);
    pinHigh(cs);
    return 1;
}

int max6675_read(float *result, int sclk, int cs, int miso) {
    uint16_t v=0;
    pinLow(cs);
    delayUsBusy(1000);
    {
        int i;
        for (i = 15; i >= 0; i--) {
            pinLow(sclk);
            DELAY_US_BUSY(1000);
            if (pinRead(miso)) {
                v |= (1 << i);
            }
            pinHigh(sclk);
            DELAY_US_BUSY(1000);
        }
    }
    pinHigh(cs);
#ifdef MODE_DEBUG
    printInt16(v);
#endif
    if (v & 0x4) {
#ifdef MODE_DEBUG
        fprintf(stderr,"%s(): thermocouple input is open\n", __func__);
#endif
        return 0;
    }
    v >>= 3;
    *result = v * 0.25;
    return 1;
}


