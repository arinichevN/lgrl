#include "pinout.h"
//32 and 64 bit
#define BLOCK_SIZE              (4*1024)

#define SUNXI_GPIO_BASE       (0x01c20800)
#define SUNXI_GPIO_LM_BASE    (0x01f02c00)
#define MAP_SIZE           (4096*2)
#define MAP_MASK           (MAP_SIZE - 1)

#define GPIO_BASE_LM_BP  (0x01f02000)   
#define GPIO_BASE_BP     (0x01C20000)

static volatile uint32_t *gpio_buf=NULL;
static volatile uint32_t *gpio_lm_buf=NULL;
extern char *physToGpio[];
static int gpio_port[PIN_NUM];
static int gpio_index[PIN_NUM];

static inline uint32_t gpio_readl(uint32_t addr, int port) {
    uint32_t val = 0;
    uint32_t mmap_base = (addr & ~MAP_MASK);
    uint32_t mmap_seek = ((addr - mmap_base) >> 2);

    /* DK, for PL and PM */
    if (port == 11)
        val = *(gpio_lm_buf + mmap_seek);
    else
        val = *(gpio_buf + mmap_seek);

    return val;
}

static inline void gpio_writel(uint32_t val, uint32_t addr, int port) {
    uint32_t mmap_base = (addr & ~MAP_MASK);
    uint32_t mmap_seek = ((addr - mmap_base) >> 2);

    if (port == 11)
        *(gpio_lm_buf + mmap_seek) = val;
    else
        *(gpio_buf + mmap_seek) = val;
}

void pinWrite(int pin, int value) {
    uint32_t regval = 0;
    uint32_t phyaddr = 0;
    if (gpio_port[pin] == 11)
        phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[pin] - 11) * 36) + 0x10;
    else
        phyaddr = SUNXI_GPIO_BASE + (gpio_port[pin] * 36) + 0x10;
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    if (value) {
        regval |= (1 << gpio_index[pin]);
    } else {
        regval &= ~(1 << gpio_index[pin]);
    }
    gpio_writel(regval, phyaddr, gpio_port[pin]);
    regval = gpio_readl(phyaddr, gpio_port[pin]);
}

int pinRead(int pin) {
    uint32_t regval = 0;
    uint32_t phyaddr = 0;

    /* for M2 PM and PL */
    if (gpio_port[pin] == 11)
        phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[pin] - 11) * 36) + 0x10;
    else
        phyaddr = SUNXI_GPIO_BASE + (gpio_port[pin] * 36) + 0x10;
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    regval = regval >> gpio_index[pin];
    regval &= 1;
    return regval;
}

void pinLow(int pin) {
    uint32_t regval = 0;
    uint32_t phyaddr = 0;
    if (gpio_port[pin] == 11)
        phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[pin] - 11) * 36) + 0x10;
    else
        phyaddr = SUNXI_GPIO_BASE + (gpio_port[pin] * 36) + 0x10;
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    regval &= ~(1 << gpio_index[pin]);
    gpio_writel(regval, phyaddr, gpio_port[pin]);
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    return;
}

void pinHigh(int pin) {
    uint32_t regval = 0;
    uint32_t phyaddr = 0;
    if (gpio_port[pin] == 11)
        phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[pin] - 11) * 36) + 0x10;
    else
        phyaddr = SUNXI_GPIO_BASE + (gpio_port[pin] * 36) + 0x10;
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    regval |= (1 << gpio_index[pin]);
    gpio_writel(regval, phyaddr, gpio_port[pin]);
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    return;
}

void pinModeIn(int pin) {
    uint32_t regval = 0;
    int offset = ((gpio_index[pin] - ((gpio_index[pin] >> 3) << 3)) << 2);
    uint32_t phyaddr = 0;

    /* for M2 PM and PL */
    if (gpio_port[pin] == 11)
        phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[pin] - 11) * 36) + ((gpio_index[pin] >> 3) << 2);
    else
        phyaddr = SUNXI_GPIO_BASE + (gpio_port[pin] * 36) + ((gpio_index[pin] >> 3) << 2);
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    regval &= ~(7 << offset);
    gpio_writel(regval, phyaddr, gpio_port[pin]);
    regval = gpio_readl(phyaddr, gpio_port[pin]);
}

void pinModeOut(int pin) {
    uint32_t regval = 0;
    int offset = ((gpio_index[pin] - ((gpio_index[pin] >> 3) << 3)) << 2);
    uint32_t phyaddr = 0;

    /* for M2 PM and PL */
    if (gpio_port[pin] == 11)
        phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[pin] - 11) * 36) + ((gpio_index[pin] >> 3) << 2);
    else
        phyaddr = SUNXI_GPIO_BASE + (gpio_port[pin] * 36) + ((gpio_index[pin] >> 3) << 2);
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    regval &= ~(7 << offset);
    regval |= (1 << offset);
    gpio_writel(regval, phyaddr, gpio_port[pin]);
    regval = gpio_readl(phyaddr, gpio_port[pin]);


}

void pinPUD(int pin, int pud) {
    uint32_t regval = 0;
    int sub = gpio_index[pin] >> 4;
    int sub_index = gpio_index[pin] - 16 * sub;
    uint32_t phyaddr = 0;

    /* for M2 PM and PL */
    if (gpio_port[pin] == 11)
        phyaddr = SUNXI_GPIO_LM_BASE + ((gpio_port[pin] - 11) * 36) + 0x1c + sub * 4;
    else
        phyaddr = SUNXI_GPIO_BASE + (gpio_port[pin] * 36) + 0x1c + sub * 4;
    regval = gpio_readl(phyaddr, gpio_port[pin]);
    regval &= ~(3 << (sub_index << 1));
    regval |= (pud << (sub_index << 1));
    gpio_writel(regval, phyaddr, gpio_port[pin]);
    regval = gpio_readl(phyaddr, gpio_port[pin]);
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
}

int gpioSetup() {
    int fd;
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
        fprintf(stderr, "%s(): ", __func__);
        perror("open()");
        return 0;
    }
    gpio_lm_buf =  mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_LM_BP);
    gpio_buf =  mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_BP);
    close(fd);
    if (( gpio_buf == MAP_FAILED) || ( gpio_lm_buf == MAP_FAILED)) {
        fprintf(stderr, "%s(): ", __func__);
        perror("mmap()");
        return 0;
    }
    makeData();
    return 1;
}

void gpioFree() {
	if(gpio_buf !=NULL && gpio_buf!=MAP_FAILED){
		if(munmap((void*)gpio_buf, BLOCK_SIZE)!=0){
			perrord("munmap()");
		}
	}
	if(gpio_lm_buf !=NULL && gpio_lm_buf!=MAP_FAILED){
       if(munmap((void*)gpio_lm_buf, BLOCK_SIZE)!=0){
			perrord("munmap()");
		}
    }
}
