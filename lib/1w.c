

#include "1w.h"

int onewire_reset(int pin) {
    pinPUD(pin, PUD_OFF);
    pinModeOut(pin);
    pinLow(pin);
    DELAY_US_BUSY(640); 
    pinHigh(pin); 
    pinModeIn(pin);
    for (int i = 80; i; i--) {
        if (!pinRead(pin)) {
            for (int i = 250; i; i--) {
                if(pinRead(pin)){
                    return 1;
                }
                DELAY_US_BUSY(1);
            }
            return 0;
        }
        DELAY_US_BUSY(1);
    }
    DELAY_US_BUSY(1);
    return 0;
}


/*
 *  works, but many crc mistakes
void onewire_send_bit1(int pin,int bit) {
    pinLow(pin);
    if (bit) {
        DELAY_US_BUSY(5); 
        pinHigh(pin);
        DELAY_US_BUSY(90); 
    } else {
        DELAY_US_BUSY(90); 
        pinHigh(pin);
        DELAY_US_BUSY(5); 
    }
}
*/

void onewire_send_bit(int pin,int bit) {
    pinModeOut(pin);
    pinLow(pin);
    if (bit) {
        DELAY_US_BUSY(1); 
        pinHigh(pin);
        DELAY_US_BUSY(60); 
    } else {
        DELAY_US_BUSY(60); 
        pinHigh(pin);
        DELAY_US_BUSY(1); 
    }
    DELAY_US_BUSY(60);
}


void onewire_send_byte(int pin,int b) {
    for (int p = 8; p; p--) {
        onewire_send_bit(pin, b & 1);
        b >>= 1;
    }
}


/*
 * works, but many crc mistakes
int onewire_read_bit(int pin) {
    pinLow(pin);
    DELAY_US_BUSY(2); 
    pinHigh(pin);
    DELAY_US_BUSY(8); 
    pinModeIn(pin);
    int r = pinRead(pin);
    pinModeOut(pin);
    DELAY_US_BUSY(80); 
    return r;
}
*/

int onewire_read_bit(int pin) {
    pinModeOut(pin);
    pinLow(pin);
    DELAY_US_BUSY(2); 
    pinHigh(pin);
    DELAY_US_BUSY(2); 
    pinModeIn(pin);
    int r = pinRead(pin);
    pinModeOut(pin);
    DELAY_US_BUSY(60); 
    return r;
}


uint8_t onewire_read_byte(int pin) {
    uint8_t r = 0;
    for (int p = 8; p; p--) {
        r >>= 1;
        if (onewire_read_bit(pin)) {
            r |= 0x80;
        }
    }
    return r;
}


uint8_t onewire_crc_update(uint8_t crc, uint8_t b) {
    for (int i = 8; i; i--) {
        crc = ((crc ^ b) & 1) ? (crc >> 1) ^ 0x8c : (crc >> 1);
        b >>= 1;
    }
    return crc;
}


int onewire_skip_rom(int pin) {
    if (!onewire_reset(pin)) {
        return 0;
    }
    onewire_send_byte(pin, OW_CMD_SKIP_ROM);
    return 1;
}


int onewire_read_rom(int pin, uint8_t * buf) {
    if (!onewire_reset(pin)) {
        return 0;
    }
    onewire_send_byte(pin, OW_CMD_READ_ROM);
    for (int i = 0; i < 8; i++) {
        *(buf++) = onewire_read_byte(pin);
    }
    return 1;
}


int onewire_match_rom(int pin, const uint8_t * addr) {
    if (!onewire_reset(pin)) {
        return 0;
    }
    onewire_send_byte(pin, OW_CMD_MATCH_ROM);
    for (int p = 0; p < 8; p++) {
        onewire_send_byte(pin, *(addr++));
    }
    return 1;
}
