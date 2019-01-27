#include "pinout.h"
//32 and 64 bit
#define BLOCK_SIZE              (4*1024)

#define SUNXI_GPIO_BASE       (0x01c20800)
#define SUNXI_GPIO_LM_BASE    (0x01f02c00)
#define MAP_SIZE           (4096*2)
#define MAP_MASK           (MAP_SIZE - 1)

#define GPIO_BASE_LM_BP  (0x01f02000)   
#define GPIO_BASE_BP     (0x01C20000)


#define MAP_OFFSET(ADDR) (((ADDR) - ((ADDR) & ~MAP_MASK)) >> 2)

#define DATA_REG_VAL *(data_reg[pin])
#define CFG_REG_VAL *(cfg_reg[pin])
#define PULL_REG_VAL *(pull_reg[pin])

extern char *physToGpio[];

static volatile uint32_t *gpio;
static volatile uint32_t *gpio_lm;

static volatile uint32_t *data_reg [PIN_NUM];
static volatile uint32_t *cfg_reg [PIN_NUM];
static volatile uint32_t *pull_reg [PIN_NUM];

static int data_offset [PIN_NUM];
static int cfg_offset [PIN_NUM];
static int pull_offset [PIN_NUM];

static int gpio_port[PIN_NUM];
static int gpio_index[PIN_NUM];

void pinWrite(int pin, int value) {
    static uint32_t regval = 0;
    regval = DATA_REG_VAL;
    if (value) {
        regval |= (1 << data_offset[pin]);
    } else {
        regval &= ~(1 << data_offset[pin]);
    }
    DATA_REG_VAL = regval;
}

int pinRead(int pin) {
    static uint32_t regval = 0;
    regval = DATA_REG_VAL;
    regval = regval >> data_offset[pin];
    regval &= 1;
    return (int) regval;
}

void pinLow(int pin) {
    static uint32_t regval = 0;
    regval = DATA_REG_VAL;
    regval &= ~(1 << data_offset[pin]);
    DATA_REG_VAL = regval;
}

void pinHigh(int pin) {
    static uint32_t regval = 0;
    regval = DATA_REG_VAL;
    regval |= (1 << data_offset[pin]);
    DATA_REG_VAL = regval;
}

void pinModeIn(int pin) {
    uint32_t regval = 0;
    regval = CFG_REG_VAL;
    regval &= ~(7 << cfg_offset[pin]);
    CFG_REG_VAL = regval;
}

void pinModeOut(int pin) {
    uint32_t regval = 0;
    regval = CFG_REG_VAL;
    regval &= ~(7 << cfg_offset[pin]);
    regval |= (1 << cfg_offset[pin]);
    CFG_REG_VAL = regval;
}

void pinPUD(int pin, int pud) {
    uint32_t regval = 0;
    regval = PULL_REG_VAL;
    regval &= ~(3 << pull_offset[pin]);
    regval |= (pud << pull_offset[pin]);
    PULL_REG_VAL = regval;
    delayUsIdle(1);
}

int checkPin(int pin) {
    if (pin < 0 || pin >= PIN_NUM) {
        return 0;
    }
    if (gpio_port[pin] == -1) {
        return 0;
    }
    if (gpio_index[pin] == -1) {
        return 0;
    }
    return 1;
}

static void parse_pin(int *port, int *pin, const char *name) {
    if (strlen(name) < 3) {
        goto failed;
    }
    if (*name == 'P') {
        name++;
    } else {
        goto failed;
    }
    *port = *name++ -'A';
    *pin = atoi(name);
    return;
failed:
    *port = -1;
    *pin = -1;
}

static void makeData() {
    for (int i = 0; i < PIN_NUM; i++) {
        parse_pin(gpio_port + i, gpio_index + i, physToGpio[i]);
    }
    for (int i = 0; i < PIN_NUM; i++) {
        int sub = (gpio_index[i] >> 4);
        int sub_index = gpio_index[i] - 16 * sub;
        if (gpio_port[i] == 11){
			uint32_t data_phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[i] - 11) * 36) + 0x10;
			uint32_t cfg_phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[i] - 11) * 36) + ((gpio_index[i] >> 3) << 2);
			uint32_t pull_phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[i] - 11) * 36) + 0x1c + sub * 4;
			data_reg[i] = gpio_lm + MAP_OFFSET(data_phyaddr);
			cfg_reg[i] = gpio_lm + MAP_OFFSET(cfg_phyaddr);
			pull_reg[i] = gpio_lm + MAP_OFFSET(pull_phyaddr);
		}else{
			uint32_t data_phyaddr = SUNXI_GPIO_BASE + (gpio_port[i] * 36) + 0x10;
			uint32_t cfg_phyaddr = SUNXI_GPIO_BASE + (gpio_port[i] * 36) + ((gpio_index[i] >> 3) << 2);
			uint32_t pull_phyaddr = SUNXI_GPIO_BASE + (gpio_port[i] * 36) + 0x1c + sub * 4;
			data_reg[i] = gpio + MAP_OFFSET(data_phyaddr);
			cfg_reg[i] = gpio + MAP_OFFSET(cfg_phyaddr);
			pull_reg[i] = gpio + MAP_OFFSET(pull_phyaddr);
		}
        data_offset[i] = gpio_index[i];
        cfg_offset[i] = ((gpio_index[i] - ((gpio_index[i] >> 3) << 3)) << 2);
        pull_offset[i] = sub_index << 1;

    }
}

int gpioSetup() {
    int fd;
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
        fprintf(stderr, "%s(): ", __func__);
        perror("open()");
        return 0;
    }
    gpio_lm =  mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_LM_BP);
    gpio =  mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_BP);
    close(fd);
    if (( gpio == MAP_FAILED) || ( gpio_lm == MAP_FAILED)) {
        fprintf(stderr, "%s(): ", __func__);
        perror("mmap()");
        return 0;
    }
    makeData();
    return 1;
}

void gpioFree() {
    	if(gpio !=NULL && gpio!=MAP_FAILED){
		if(munmap((void*)gpio, BLOCK_SIZE)!=0){
			perrord("munmap()");
		}
	}
	    	if(gpio_lm !=NULL && gpio_lm!=MAP_FAILED){
		if(munmap((void*)gpio_lm, BLOCK_SIZE)!=0){
			perrord("munmap()");
		}
	}
}



