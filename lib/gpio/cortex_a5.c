//Cortex A5 32 bit
#include "pinout.h"
#define GPIOA_BASE                         0x20930000
#define GPIOB_BASE                         0x20931000
#define GPIOC_BASE                         0x11A08000
#define GPIOD_BASE                         0x20932000
#define GPIO_BIT(x)                        (1UL << (x))

#define OEN_VAL_REGISTER                   (0x00)
#define OEN_SET_OUT_REGISTER               (0x04)
#define SET_IN_REGISTER                    (0x08)
#define VAL_REGISTER                       (0x0C)
#define SET_REGISTER                       (0x10)
#define CLR_REGISTER                       (0x14)
#define GPPUD 37

#define BLOCK_SIZE  (4*1024)
#define MAP_SIZE           (4 * 4096)
#define MAP_MASK           (MAP_SIZE - 1)

#define BANK_NUM 4
#define INDEX_NUM 32

static volatile uint32_t *gpio;
static volatile uint32_t *gpio_c;

volatile uint32_t *gpio_arrw_low [PIN_NUM];
volatile uint32_t *gpio_arrw_high [PIN_NUM];

int physToGpio[PIN_NUM] = {
    -1, // 0
    -1, -1, // 1, 2
    62, -1, // 3, 4
    63, -1, // 5, 6
    56, 72, // 7, 8
    -1, 71, // 9, 10
    70, 37, // 11, 12
    14, -1, // 13, 14
    15, 69, // 15, 16
    -1, 89, // 17, 18
    4, -1, // 19, 20
    3, 16, // 21, 22
    2, 5, // 23, 24
    -1, 6, // 25, 26
    1, 0, // 27, 28
    90, -1, // 29, 30
    91, 41, // 31, 32
    92, -1, // 33, 34
    93, 40, // 35, 36
    94, 38, // 37, 38
    -1, 39, // 39, 40
    // Padding:
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // ... 56
    -1, -1, -1, -1, -1, -1, -1, // ... 63
};

int pin_mask[BANK_NUM][INDEX_NUM] = //[BANK]  [INDEX]
{
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,}, //PA
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,}, //PB
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,}, //PC
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,}, //PD
};

uint8_t gpioToPUDCLK[] ={
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
    39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
};

unsigned int ca5_readR(unsigned int addr) {
    unsigned int val = 0;
    unsigned int mmap_base = (addr & ~MAP_MASK);
    unsigned int mmap_seek = (addr - mmap_base);

    if (mmap_base == 0x11a08000) {
        val = *((char *) gpio_c + mmap_seek);
    } else {
        val = *((char *) gpio + mmap_seek);
    }
    return val;
}

void ca5_writeR(unsigned int val, unsigned int addr) {
    unsigned int mmap_base = (addr & ~MAP_MASK);
    unsigned int mmap_seek = (addr - mmap_base);
    if (mmap_base == 0x11a08000) {
        *((char *) gpio_c + mmap_seek) = val;
    } else {
        *((char *) gpio + mmap_seek) = val;
    }
    printf("writeR: val: %u addr: %u, mmap_base: %u, mmap_seek: %u\n", val, addr, mmap_base, mmap_seek);
}

int ca5_set_gpio_mode(int pin, int mode) {
    unsigned int bank = pin >> 5;
    unsigned int index = pin - (bank << 5);
    unsigned int phyaddr = 0;
    unsigned int base_address = 0;
    /* Offset of register */
    if (bank == 0) /* group A */
        base_address = GPIOA_BASE;
    else if (bank == 1) /* group B */
        base_address = GPIOB_BASE;
    else if (bank == 2) /* group C */
        base_address = GPIOC_BASE;
    else if (bank == 3) /* group D */
        base_address = GPIOD_BASE;
    else
        printf("Bad pin number\n");

    if (mode == INPUT)
        phyaddr = base_address + SET_IN_REGISTER;
    else if (mode == OUTPUT)
        phyaddr = base_address + OEN_SET_OUT_REGISTER;
    else
        printf("Invalid mode\n");

    if (INPUT == mode) {
        ca5_writeR(GPIO_BIT(index), phyaddr);
    } else if (OUTPUT == mode) {
        ca5_writeR(GPIO_BIT(index), phyaddr);
        ca5_writeR(GPIO_BIT(index), base_address + CLR_REGISTER);

    } else {
        printf("Unknow mode\n");
    }

    return 0;
}

int ca5_digitalWrite(int pin, int value) {
    unsigned int bank = pin >> 5;
    unsigned int index = pin - (bank << 5);
    unsigned int phyaddr = 0;
    unsigned int base_address = 0;
    if (bank == 0)
        base_address = GPIOA_BASE;
    else if (bank == 1)
        base_address = GPIOB_BASE;
    else if (bank == 2)
        base_address = GPIOC_BASE;
    else if (bank == 3)
        base_address = GPIOD_BASE;
    else {
        printf("Bad pin number!\n");
        return 0;
    }
    if (value)
        phyaddr = base_address + SET_REGISTER;
    else
        phyaddr = base_address + CLR_REGISTER;
    ca5_writeR(GPIO_BIT(index), phyaddr);
    return 0;
}

int ca5_digitalRead(int pin) {
    int bank = pin >> 5;
    int index = pin - (bank << 5);
    int val;
    unsigned int base_address = 0;
    unsigned int phys_OEN_R;
    unsigned int phys_SET_R;
    unsigned int phys_VAL_R;

    /* version 0.1 not support GPIOC input function */
    if (bank == 2)
        return -1;
    if (bank == 0)
        base_address = GPIOA_BASE;
    else if (bank == 1)
        base_address = GPIOB_BASE;
    else if (bank == 2)
        base_address = GPIOC_BASE;
    else if (bank == 3)
        base_address = GPIOD_BASE;
    else {
        printf("Bad pin number\n");
        return -1;
    }
    phys_OEN_R = base_address + OEN_VAL_REGISTER;
    phys_SET_R = base_address + SET_REGISTER;
    phys_VAL_R = base_address + VAL_REGISTER;
    if (ca5_readR(phys_OEN_R) & GPIO_BIT(index))
        val = (ca5_readR(phys_VAL_R) & GPIO_BIT(index)) ? 1 : 0;
    else
        val = (ca5_readR(phys_SET_R) & GPIO_BIT(index)) ? 1 : 0;
    return val;
}

int checkPin(int pin) {
    if (pin < 0 || pin >= PIN_NUM) {
        return 0;
    }
    if (physToGpio[pin] == -1) {
        return 0;
    }
    int bank = pin >> 5;
    int index = pin - (bank << 5);
    if (bank < 0 || bank >= BANK_NUM) {
        return 0;
    }
    if (index < 0 || index >= INDEX_NUM) {
        return 0;
    }
    if (pin_mask[bank][index] == -1) {
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
    gpio = mmap(0, BLOCK_SIZE * 3, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOA_BASE);
    if (gpio == MAP_FAILED) {
        close(fd);
        fprintf(stderr, "%s(): ", __func__);
        perror("mmap1()");
        return 0;
    }
    gpio_c = mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOC_BASE);
    close(fd);
    if (gpio_c == MAP_FAILED) {
        fprintf(stderr, "%s(): ", __func__);
        perror("mmap2()");
        return 0;
    }


    return 1;
}

int gpioFree() {
    return 1;
}

void pinWrite(int pin, int value) {
    unsigned int bank = pin >> 5;
    unsigned int index = pin - (bank << 5);
    unsigned int phyaddr = 0;
    unsigned int base_address = 0;
    if (bank == 0)
        base_address = GPIOA_BASE;
    else if (bank == 1)
        base_address = GPIOB_BASE;
    else if (bank == 2)
        base_address = GPIOC_BASE;
    else if (bank == 3)
        base_address = GPIOD_BASE;
    else {
        return;
    }
    if (value)
        phyaddr = base_address + SET_REGISTER;
    else
        phyaddr = base_address + CLR_REGISTER;
    ca5_writeR(GPIO_BIT(index), phyaddr);
}

int pinRead(int pin) {
    int bank = pin >> 5;
    int index = pin - (bank << 5);
    int val;
    unsigned int base_address = 0;
    unsigned int phys_OEN_R;
    unsigned int phys_SET_R;
    unsigned int phys_VAL_R;

    /* version 0.1 not support GPIOC input function */
    if (bank == 2)
        return -1;
    if (bank == 0)
        base_address = GPIOA_BASE;
    else if (bank == 1)
        base_address = GPIOB_BASE;
    else if (bank == 2)
        base_address = GPIOC_BASE;
    else if (bank == 3)
        base_address = GPIOD_BASE;
    else {
        return -1;
    }
    phys_OEN_R = base_address + OEN_VAL_REGISTER;
    phys_SET_R = base_address + SET_REGISTER;
    phys_VAL_R = base_address + VAL_REGISTER;
    if (ca5_readR(phys_OEN_R) & GPIO_BIT(index))
        val = (ca5_readR(phys_VAL_R) & GPIO_BIT(index)) ? 1 : 0;
    else
        val = (ca5_readR(phys_SET_R) & GPIO_BIT(index)) ? 1 : 0;
    return val;
}

void pinLow(int pin) {
    unsigned int bank = pin >> 5;
    unsigned int index = pin - (bank << 5);
    unsigned int phyaddr = 0;
    unsigned int base_address = 0;
    if (bank == 0)
        base_address = GPIOA_BASE;
    else if (bank == 1)
        base_address = GPIOB_BASE;
    else if (bank == 2)
        base_address = GPIOC_BASE;
    else if (bank == 3)
        base_address = GPIOD_BASE;
    else {
        return;
    }
    phyaddr = base_address + CLR_REGISTER;
    ca5_writeR(GPIO_BIT(index), phyaddr);
}

void pinHigh(int pin) {
    unsigned int bank = pin >> 5;
    unsigned int index = pin - (bank << 5);
    unsigned int phyaddr = 0;
    unsigned int base_address = 0;
    if (bank == 0)
        base_address = GPIOA_BASE;
    else if (bank == 1)
        base_address = GPIOB_BASE;
    else if (bank == 2)
        base_address = GPIOC_BASE;
    else if (bank == 3)
        base_address = GPIOD_BASE;
    else {
        return;
    }
    phyaddr = base_address + SET_REGISTER;
    ca5_writeR(GPIO_BIT(index), phyaddr);
}

void pinModeIn(int pin) {
    unsigned int bank = pin >> 5;
    unsigned int index = pin - (bank << 5);
    unsigned int phyaddr = 0;
    unsigned int base_address = 0;
    /* Offset of register */
    if (bank == 0) /* group A */
        base_address = GPIOA_BASE;
    else if (bank == 1) /* group B */
        base_address = GPIOB_BASE;
    else if (bank == 2) /* group C */
        base_address = GPIOC_BASE;
    else if (bank == 3) /* group D */
        base_address = GPIOD_BASE;
    else {
        return;
    }
    phyaddr = base_address + SET_IN_REGISTER;
    ca5_writeR(GPIO_BIT(index), phyaddr);
}

void pinModeOut(int pin) {
    unsigned int bank = pin >> 5;
    unsigned int index = pin - (bank << 5);
    unsigned int phyaddr = 0;
    unsigned int base_address = 0;
    if (bank == 0)
        base_address = GPIOA_BASE;
    else if (bank == 1)
        base_address = GPIOB_BASE;
    else if (bank == 2)
        base_address = GPIOC_BASE;
    else if (bank == 3)
        base_address = GPIOD_BASE;
    else {
        return;
    }
    ca5_writeR(GPIO_BIT(index), phyaddr);
    ca5_writeR(GPIO_BIT(index), base_address + CLR_REGISTER);
}

void pinPUD(int pin, int pud) {
    pin = physToGpio[pin];
    *(gpio + GPPUD) = pud & 3;
    delayUsBusy(5);
    *(gpio + gpioToPUDCLK [pin]) = 1 << (pin & 31);
    delayUsBusy(5);
    *(gpio + GPPUD) = 0;
    delayUsBusy(5);
    *(gpio + gpioToPUDCLK [pin]) = 0;
    delayUsBusy(5);

}
