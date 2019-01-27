
#include "ds18b20.h"

double dsToFloat(uint16_t v) {
    double t;
    if (v >= 0x800) //temperture is negative
    {
        t = 0;
        //calculate the fractional part
        if (v & 0x0001) t += 0.0625;
        if (v & 0x0002) t += 0.1250;
        if (v & 0x0004) t += 0.2500;
        if (v & 0x0008) t += 0.5000;

        //calculate the whole number part
        v = (v >> 4) & 0x00FF;
        v = v - 0x0001; //subtract 1
        v = ~v; //ones compliment
        t = t - (double) (v & 0xFF);
    } else //temperture is positive
    {
        t = 0;
        //calculate the whole number part
        t = (v >> 4) & 0x00FF;
        //t = (v >> 4);
        //calculate the fractional part
        if (v & 0x0001) t = t + 0.06250;
        if (v & 0x0002) t = t + 0.12500;
        if (v & 0x0004) t = t + 0.25000;
        if (v & 0x0008) t = t + 0.50000;
    }
    return t;
}

int ds18b20_read_scratchpad(int pin, const uint8_t *addr, uint8_t *sp) {
    if (!onewire_match_rom(pin, addr)) {
        printde("onewire_match_rom() failed where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        return 0;
    }
    onewire_send_byte(pin, DS18B20_CMD_READ_SCRATCHPAD);
    uint8_t scratchpad[DS18B20_SCRATCHPAD_BYTE_NUM];
    uint8_t crc = 0;
    for (int i = 0; i < DS18B20_SCRATCHPAD_BYTE_NUM; i++) {
        scratchpad[i] = onewire_read_byte(pin);
        crc = onewire_crc_update(crc, scratchpad[i]);
    }
    if (onewire_read_byte(pin) != crc) {
        printde("scratchpad crc error where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        return 0;
    }
    for (int i = 0; i < DS18B20_SCRATCHPAD_BYTE_NUM; i++) {
        sp[i] = scratchpad[i];
    }
    return 1;
}

int ds18b20_write_scratchpad(int pin, const uint8_t *addr, const uint8_t *data) {
    if (!onewire_match_rom(pin, addr)) {
        printde("onewire_match_rom() failed where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        return 0;
    }
    onewire_send_byte(pin, DS18B20_CMD_WRITE_SCRATCHPAD);
    for (int i = 0; i < DS18B20_EEPROM_BYTE_NUM; i++) {
        onewire_send_byte(pin, data[i]);
    }
    DELAY_US_BUSY(480);
    return 1;
}

int ds18b20_copy_scratchpad(int pin, const uint8_t *addr) {
    if (!onewire_match_rom(pin, addr)) {
        putsde("not match\n");
        return 0;
    }
    onewire_send_byte(pin, DS18B20_CMD_COPY_SCRATCHPAD);
    DELAY_US_BUSY(480);
    return 1;
}

int ds18b20_recall(int pin, const uint8_t *addr) {
    if (!onewire_match_rom(pin, addr)) {
        putsde("not match\n");
        return 0;
    }
    onewire_send_byte(pin, DS18B20_CMD_RECALL);
    while (!onewire_read_bit(pin)) {
    }
    DELAY_US_BUSY(480);
    return 1;
}

int ds18b20_set_resolution(int pin, const uint8_t *addr, uint8_t res) {
    uint8_t scratchpad[DS18B20_SCRATCHPAD_BYTE_NUM];
    //read scratchpad
    if (!ds18b20_read_scratchpad(pin, addr, scratchpad)) {
        return 0;
    }
    //edit
    scratchpad[DS18B20_SCRATCHPAD_CONFIG_REG] = res;
    //write to scratchpad
    uint8_t *conf = scratchpad + 2;
    if (!ds18b20_write_scratchpad(pin, addr, conf)) {
        return 0;
    }
    //copy from scratchpad to eeprom
    if (!ds18b20_copy_scratchpad(pin, addr)) {
        return 0;
    }
    //recall from eeprom to scratchpad
    if (!ds18b20_recall(pin, addr)) {
        return 0;
    }
    //read scratchpad and check
    if (!ds18b20_read_scratchpad(pin, addr, scratchpad)) {
        return 0;
    }
    if (scratchpad[DS18B20_SCRATCHPAD_CONFIG_REG] != res) {
        putsde("check-up failed\n");
        return 0;
    }
    return 1;
}

int ds18b20_get_resolution(int pin, const uint8_t *addr, int *res) {
    uint8_t scratchpad[DS18B20_SCRATCHPAD_BYTE_NUM];
    if (!ds18b20_read_scratchpad(pin, addr, scratchpad)) {
        return 0;
    }
    switch (scratchpad[DS18B20_SCRATCHPAD_CONFIG_REG]) {
        case DS18B20_RES_9BIT:
            *res = 9;
            return 1;
            break;
        case DS18B20_RES_10BIT:
            *res = 10;
            return 1;
            break;
        case DS18B20_RES_11BIT:
            *res = 11;
            return 1;
            break;
        case DS18B20_RES_12BIT:
            *res = 12;
            return 1;
            break;
        default:
            return 0;
            break;
    }
}

void ds18b20_wait_convertion(int pin) {
    while (!onewire_read_bit(pin)) {
    }
    DELAY_US_BUSY(480);
}

int ds18b20_convert_t(int pin, const uint8_t *addr) {
    if (!onewire_match_rom(pin, addr)) {
        printde("onewire_match_rom() failed where pin = %d and address = %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\n", pin, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        return 0;
    }
    onewire_send_byte(pin, DS18B20_CMD_CONVERTT);
    ds18b20_wait_convertion(pin);
    return 1;
}

int ds18b20_convert_t_all(int pin) {
    if (!onewire_skip_rom(pin)) {
        printde("onewire_skip_rom() failed where pin = %d\n", pin);
        return 0;
    }
    onewire_send_byte(pin, DS18B20_CMD_CONVERTT);
    return 1;
}



int ds18b20_read_temp(int pin, const uint8_t *addr, double * temp) {
    uint8_t scratchpad[DS18B20_SCRATCHPAD_BYTE_NUM];
    if (!ds18b20_read_scratchpad(pin, addr, scratchpad)) {
        return 0;
    }
    uint16_t td = (scratchpad[1] << 8) | scratchpad[0];
    *temp = dsToFloat(td);
    return 1;
}

int ds18b20_get_temp(int pin, const uint8_t *addr, double * temp) {
    uint8_t scratchpad[DS18B20_SCRATCHPAD_BYTE_NUM];
    if (!ds18b20_convert_t(pin, addr)) {
        return 0;
    }
    if (!ds18b20_read_scratchpad(pin, addr, scratchpad)) {
        return 0;
    }
    uint16_t td = (scratchpad[1] << 8) | scratchpad[0];
    *temp = dsToFloat(td);
    return 1;
}

int ds18b20_parse_resolution(int r) {
    switch (r) {
        case 9:
            return (DS18B20_RES_9BIT);
        case 10:
            return (DS18B20_RES_10BIT);
        case 11:
            return (DS18B20_RES_11BIT);
        case 12:
            return (DS18B20_RES_12BIT);
        default:
            return -1;
    }
}
