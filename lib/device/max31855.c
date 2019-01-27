/*

#include "max31855.h"
 */

/*
int max31855_init(int sclk, int cs, int miso) {
    pinModeOut(cs);
    pinModeOut(sclk);
    pinModeIn(miso);
    pinPUD(miso, PUD_DOWN);
    pinPUD(sclk, PUD_OFF);
    pinPUD(cs, PUD_OFF);
    pinHigh(cs);
    pinLow(sclk);
    return 1;
}

static void printInt32(uint32_t d) {
    int i;
    for (i = 31; i >= 0; i--) {
        int v = (d >> i) & 1;
        printf("%d", v);
    }
    puts("");
}
#define DELAY 1000

int max31855_read(float *result, int sclk, int cs, int miso) {
    uint32_t v=0;
    pinLow(cs);
    DELAY_US_BUSY(DELAY);
    {
        int i;
        for (i = 31; i >= 0; i--) {
            pinLow(sclk);
            DELAY_US_BUSY(DELAY);
            if (pinRead(miso)) {
                v |= (1 << i);
            }
            pinHigh(sclk);
            DELAY_US_BUSY(DELAY);
        }
    }
    pinHigh(cs);
#ifdef MODE_DEBUG
    printInt32(v);
#endif
    int error = 0;
    if (v & 0x20000) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: warning: bit 18 should be 0 where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
    }
    if (v & 0x8) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: warning: bit 4 should be 0 where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
    }
    if (v & 0x4) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: thermocouple is short-circuited to VCC where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        error = 1;
    }
    if (v & 0x2) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: thermocouple is short-circuited to GND where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        error = 1;
    }
    if (v & 0x1) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: thermocouple input is open where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        error = 1;
    }
    if (v & 0x8000) {
#ifdef MODE_DEBUG
        fprintf(stderr, "max31855_read: fault has been found where sclk=%d and cs=%d and miso=%d\n", sclk, cs, miso);
#endif
        error = 1;
    }
    if (error) {
        return 0;
    }
    if (v & 0x80000000) {
        v = 0xFFFFC000 | ((v >> 18) & 0x00003FFFF);
    } else {
        v >>= 18;
    }
 *result = v * 0.25;
    return 1;
}
 */
