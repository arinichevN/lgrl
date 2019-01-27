
#include "mcp3008.h"

static void printInt16(uint16_t d) {
    int i;
    for (i = 15; i >= 0; i--) {
        int v = (d >> i) & 1;
        printf("%d", v);
    }
    puts("");
}

int mcp3008_init(int sclk, int cs, int mosi, int miso) {
    pinModeOut(sclk);
    pinModeOut(cs);
    pinModeIn(miso);
    pinModeOut(mosi);
    return 1;
}
//10khz clock frequency (page 22)
int mcp3008_read(int *result, int id,int mode, int sclk, int cs, int mosi, int miso) {
    if (id > 7 || id < 0) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): bad id, 0 >= id <= 7 expected\n", F);
#endif
        return 0;
    }

    pinHigh(cs);
    pinLow(sclk);
    pinLow(cs);

/*
    int cmd = id;
    cmd |= 0x18;
    cmd <<= 3;
*/
    int cmd=id;
    cmd |=mode;
    cmd <<= 3;
    for (int i = 0; i < 5; i++) {
        if (cmd & 0x80) {
            pinHigh(mosi);
        } else {
            pinLow(mosi);
        }
        cmd <<= 1;
        pinHigh(sclk);
        pinLow(sclk);
    }

    int out = 0;
    for (int i = 0; i < 12; i++) {
        pinHigh(sclk);
        pinLow(sclk);
        out <<= 1;
        if (pinRead(miso)) {
            out |= 0x1;
        }
    }
    pinHigh(cs);
    out >>= 1;
    *result = out;
    return 1;
}


