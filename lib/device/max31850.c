
#include "max31850.h"

int max31850_read_scratchpad(int pin, const uint8_t *addr, uint8_t *sp) {
    if (!onewire_match_rom(pin, addr)) {
        printde("onewire_match_rom() failed where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        return 0;
    }
    onewire_send_byte(pin, MAX31850_CMD_READ_SCRATCHPAD);
    uint8_t scratchpad[MAX31850_SCRATCHPAD_BYTE_NUM];
    uint8_t crc = 0;
    for (int i = 0; i < MAX31850_SCRATCHPAD_BYTE_NUM; i++) {
        scratchpad[i] = onewire_read_byte(pin);
        crc = onewire_crc_update(crc, scratchpad[i]);
    }
    if (onewire_read_byte(pin) != crc) {
        printde("scratchpad crc error where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        return 0;
    }
    for (int i = 0; i < MAX31850_SCRATCHPAD_BYTE_NUM; i++) {
        sp[i] = scratchpad[i];
    }
    return 1;
}

void max31850_wait_convertion(int pin) {
    while (!onewire_read_bit(pin)) {
    }
    delayUsBusy(480);
}

int max31850_convert_t(int pin, const uint8_t *addr) {
    if (!onewire_match_rom(pin, addr)) {
        printde("onewire_match_rom() failed where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        return 0;
    }
    onewire_send_byte(pin, MAX31850_CMD_CONVERTT);
    max31850_wait_convertion(pin);
    return 1;
}

int max31850_convert_t_all(int pin) {
    if (!onewire_skip_rom(pin)) {
        printde("onewire_skip_rom() failed where pin = %d\n", pin);
        return 0;
    }
    onewire_send_byte(pin, MAX31850_CMD_CONVERTT);
    return 1;
}

static float dsToFloat(uint8_t v0, uint8_t v1) {
    uint16_t vv0 = v0;
    uint16_t vv1 = v1;
    uint16_t uwp = ((vv1 & 0x007F) << 4) | (vv0 >> 4);
    float out = (float) uwp;
    if (vv0 & 0x0004) out += 0.25f;
    if (vv0 & 0x0008) out += 0.5f;
    if (vv1 & 0x0080) out *= -1.0f;
    return out;
}

static int getTempFromScratchpad(float * temp, uint8_t scratchpad[MAX31850_SCRATCHPAD_BYTE_NUM]) {
    int ok = 1;
    if (scratchpad[0] & 0x1) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): fault detected\n", F);
#endif
        ok = 0;
    }
    if (scratchpad[2] & 0x1) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): open circuit\n", F);
#endif
        ok = 0;
    }
    if (scratchpad[2] & 0x2) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): short to GND\n", F);
#endif
        ok = 0;
    }
    if (scratchpad[2] & 0x4) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): short to Vdd\n", F);
#endif
        ok = 0;
    }
    if (!ok) {
        return 0;
    }
    *temp = dsToFloat(scratchpad[0], scratchpad[1]);
    return 1;
}

int max31850_read_temp(int pin, const uint8_t *addr, float * temp) {
    uint8_t scratchpad[MAX31850_SCRATCHPAD_BYTE_NUM];
    if (!max31850_read_scratchpad(pin, addr, scratchpad)) {
        return 0;
    }
    if (!getTempFromScratchpad(temp, scratchpad)) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): fault where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", F, pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
#endif
        return 0;
    }
    return 1;
}

int max31850_get_temp(int pin, const uint8_t *addr, float * temp) {
    if (!max31850_convert_t(pin, addr)) {
        return 0;
    }
    if (!max31850_read_temp(pin, addr, temp)) {
        return 0;
    }
    return 1;
}
