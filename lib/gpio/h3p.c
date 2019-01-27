//Allwinner H3 CPU
#include "pinout.h"

#define PIO_BASE (0x01C20800)
#define BLOCK_SIZE  (4*1024)


#define PIN *(data_reg[pin])
#define PIN_MODE *(cfg_reg[pin])
#define PIN_PULL *(pull_reg[pin])


volatile uint32_t *base;

volatile uint32_t *cfg_reg[PIN_NUM];
volatile uint32_t *data_reg[PIN_NUM];
volatile uint32_t *pull_reg[PIN_NUM];

int data_index[PIN_NUM] = {
    -1, // 0
    -1, -1, // 1, 2
    12, -1, // 3, 4
    11, -1, // 5, 6
    6, 6, // 7, 8
    -1, 7, // 9, 10
    1, 7, //11, 12
    0, -1, //13, 14
    3, 19, //15, 16
    -1, 18, //17, 18
    15, -1, //19, 20
    16, 2, //21, 22
    14, 13, //23, 24
    -1, 10, //25, 26
};

int data_reg_offset[PIN_NUM] = {
    -1, // 0
    -1, -1, // 1, 2
    0x10, -1, // 3, 4
    0x10, -1, // 5, 6
    0x10, 0xE8, // 7, 8
    -1, 0xE8, // 9, 10
    0x10, 0x10, //11, 12
    0x10, -1, //13, 14
    0x10, 0x10, //15, 16
    -1, 0x10, //17, 18
    0x10, -1, //19, 20
    0x10, 0x10, //21, 22
    0x10, 0x10, //23, 24
    -1, 0x10, //25, 26
};

int cfg_reg_offset[PIN_NUM] = {
    -1, // 0
    -1, -1, // 1, 2
    0x04, -1, // 3, 4
    0x04, -1, // 5, 6
    0x00, 0xD8, // 7, 8
    -1, 0xD8, // 9, 10
    0x00, 0x00, //11, 12
    0x00, -1, //13, 14
    0x00, 0x08, //15, 16
    -1, 0x08, //17, 18
    0x00, -1, //19, 20
    0x04, 0x00, //21, 22
    0x04, 0x04, //23, 24
    -1, 0x04, //25, 26
};

int cfg_offset[PIN_NUM] = {
    -1, // 0
    -1, -1, // 1, 2
    16, -1, // 3, 4
    12, -1, // 5, 6
    24, 24, // 7, 8
    -1, 28, // 9, 10
    4, 28, //11, 12
    0, -1, //13, 14
    12, 12, //15, 16
    -1, 8, //17, 18
    28, -1, //19, 20
    0, 8, //21, 22
    24, 20, //23, 24
    -1, 8, //25, 26
};

int pull_index[PIN_NUM] = {
    -1, // 0
    -1, -1, // 1, 2
    12, -1, // 3, 4
    11, -1, // 5, 6
    6, 6, // 7, 8
    -1, 7, // 9, 10
    1, 7, //11, 12
    0, -1, //13, 14
    3, 19, //15, 16
    -1, 18, //17, 18
    15, -1, //19, 20
    16, 2, //21, 22
    14, 13, //23, 24
    -1, 10, //25, 26
};

int pull_reg_offset[PIN_NUM] = {
    -1, // 0
    -1, -1, // 1, 2
    0x1C, -1, // 3, 4
    0x1C, -1, // 5, 6
    0x1C, 0xF4, // 7, 8
    -1, 0xF4, // 9, 10
    0x1C, 0x1C, //11, 12
    0x1C, -1, //13, 14
    0x1C, 0x20, //15, 16
    -1, 0x20, //17, 18
    0x1C, -1, //19, 20
    0x20, 0x1C, //21, 22
    0x1C, 0x1C, //23, 24
    -1, 0x1C, //25, 26
};

void pinWrite(int pin, int value) {
    static uint32_t regval = 0;
    regval = PIN;
    if (value) {
        regval |= (1 << data_index[pin]);
    } else {
        regval &= ~(1 << data_index[pin]);
    }
    PIN = regval;
}

int pinRead(int pin) {
    static uint32_t regval = 0;
    regval = PIN;
    regval = regval >> data_index[pin];
    regval &= 1;
    return (int) regval;
}

void pinLow(int pin) {
    uint32_t regval = 0;
    regval = PIN;
    regval &= ~(1 << data_index[pin]);
    PIN = regval;
}

void pinHigh(int pin) {
    uint32_t regval = 0;
    regval = PIN;
    regval |= (1 << data_index[pin]);
    PIN = regval;
}

void pinModeIn(int pin) {
    uint32_t regval = 0;
    regval = PIN_MODE;
    regval &= ~(7 << cfg_offset[pin]);
    PIN_MODE = regval;
}

void pinModeOut(int pin) {
    uint32_t regval = 0;
    regval = PIN_MODE;
    regval &= ~(7 << cfg_offset[pin]);
    regval |= (1 << cfg_offset[pin]);
    PIN_MODE = regval;
}

void pinPUD(int pin, int pud) {
    if (pud == PUD_UP) {
        pud = 1;
    } else if (pud == PUD_DOWN) {
        pud = 2;
    } else {
        pud = 0;
    }
    uint32_t regval = 0;
    regval = PIN_PULL;
    regval &= ~(3 << pull_index[pin]);
    regval |= (pud << pull_index[pin]);
    PIN_PULL = regval;
    delayUsIdle(1);
}

void makeGpioDataOffset() {
    int i;
    for (i = 0; i < PIN_NUM; i++) {
        if (cfg_reg_offset[i] != -1) {
            cfg_reg[i] = base + cfg_reg_offset[i];
        } else {
            cfg_reg[i] = 0;
        }
        if (data_reg_offset[i] != -1) {
            data_reg[i] = base + data_reg_offset[i];
        } else {
            data_reg[i] = 0;
        }
        if (pull_reg_offset[i] != -1) {
            pull_reg[i] = base + pull_reg_offset[i];
        } else {
            pull_reg[i] = 0;
        }

    }
}

int checkPin(int pin) {
    if (pin < 0 || pin >= PIN_NUM) {
        return 0;
    }
    if (data_index[pin] == -1) {
        return 0;
    }
    return 1;
}

int gpioSetup() {
    int fd;
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
        fprintf(stderr, "%s(): ", __func__);
        perror("open()");
        return 0;
    }
    base =  mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PIO_BASE);
    close(fd);
    if (base == MAP_FAILED) {
       fprintf(stderr, "%s(): ", __func__);
        perror("mmap()");
        return 0;
    }
    makeGpioDataOffset();
    return 1;
}

void gpioFree() {
     if(base !=NULL && base!=MAP_FAILED){
		if(munmap((void*)base, BLOCK_SIZE)!=0){
			perrord("munmap()");
		}
	}
}



