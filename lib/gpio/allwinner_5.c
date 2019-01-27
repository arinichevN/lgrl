/*
 * This file is modification of pyA20 library.
 */
#include "pinout.h"

#define SUNXI_GPIO_BASE 0x01c20800

struct sunxi_gpio {
	unsigned int	cfg[4];
	unsigned int	dat;
	unsigned int	drv[2];
	unsigned int	pull[2];
};

struct sunxi_gpio_int {
	unsigned int	cfg[3];
	unsigned int	ctl;
	unsigned int	sta;
	unsigned int	deb;
};

struct sunxi_gpio_reg {
	struct sunxi_gpio	gpio_bank[9];
	unsigned char		res[0xbc];
	struct sunxi_gpio_int	gpio_int;
};

#define GPIO_BANK(pin)  ((pin) >> 5)
#define GPIO_NUM(pin)   ((pin) & 0x1F)

#define GPIO_CFG_INDEX(pin)     (((pin) & 0x1F) >> 3)
#define GPIO_CFG_OFFSET(pin)    ((((pin) & 0x1F) & 0x7) << 2)

#define GPIO_PUL_INDEX(pin)     (((pin) & 0x1F) >> 4)
#define GPIO_PUL_OFFSET(pin)    (((pin) & 0x0F) << 1)


unsigned int pio_base=0;
size_t mmap_length=0;
void *gpio_buf=NULL;

static int gpio_pin[PIN_NUM];

void pinWrite(int pin, int value) {
	unsigned int p = gpio_pin[pin];
	unsigned int bank = GPIO_BANK(p);
	unsigned int num = GPIO_NUM(p);
	struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)pio_base)->gpio_bank[bank];
	if (value)
		*(&pio->dat) |= 1 << num;
	else
		*(&pio->dat) &= ~(1 << num);
}

int pinRead(int pin) {
	unsigned int p = gpio_pin[pin];
	unsigned int bank = GPIO_BANK(p);
	unsigned int num = GPIO_NUM(p);
	struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)pio_base)->gpio_bank[bank];
	unsigned int dat = *(&pio->dat);
	dat >>= num;
	return dat & 0x1;
}

void pinLow(int pin) {
	unsigned int p = gpio_pin[pin];
	unsigned int bank = GPIO_BANK(p);
	unsigned int num = GPIO_NUM(p);
	struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)pio_base)->gpio_bank[bank];
	*(&pio->dat) &= ~(1 << num);
}

void pinHigh(int pin) {
    unsigned int p = gpio_pin[pin];
	unsigned int bank = GPIO_BANK(p);
	unsigned int num = GPIO_NUM(p);
	struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)pio_base)->gpio_bank[bank];
	*(&pio->dat) |= 1 << num;
}

void pinModeIn(int pin) {
    unsigned int p = gpio_pin[pin];
	unsigned int bank = GPIO_BANK(p);
	unsigned int index = GPIO_CFG_INDEX(p);
	unsigned int offset = GPIO_CFG_OFFSET(p);
	struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)pio_base)->gpio_bank[bank];
	unsigned int cfg = *(&pio->cfg[0] + index);
	cfg &= ~(0xf << offset);
	cfg |= 0 << offset;
	*(&pio->cfg[0] + index) = cfg;
}

void pinModeOut(int pin) {
	unsigned int p = gpio_pin[pin];
	unsigned int bank = GPIO_BANK(p);
	unsigned int index = GPIO_CFG_INDEX(p);
	unsigned int offset = GPIO_CFG_OFFSET(p);
	struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)pio_base)->gpio_bank[bank];
	unsigned int cfg = *(&pio->cfg[0] + index);
	cfg &= ~(0xf << offset);
	cfg |= 1 << offset;
	*(&pio->cfg[0] + index) = cfg;
}

void pinPUD(int pin, int pud) {
	unsigned int p = gpio_pin[pin];
	unsigned int bank = GPIO_BANK(p);
	unsigned int index = GPIO_PUL_INDEX(p);
	unsigned int offset = GPIO_PUL_OFFSET(p);
	struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)pio_base)->gpio_bank[bank];
	unsigned int cfg = *(&pio->pull[0] + index);
	cfg &= ~(0x3 << offset);
	cfg |= pud << offset;
	*(&pio->pull[0] + index) = cfg;
}

int checkPin(int pin) {
    if (pin < 0 || pin >= PIN_NUM) {
        return 0;
    }
    if (gpio_pin[pin] == -1) {
        return 0;
    }
    return 1;
}

static void parse_pin(int *pin, const char *name) {
    if (strlen(name) < 3) {
        goto failed;
    }
    if (*name == 'P') {
        name++;
    } else {
        goto failed;
    }
    int port = *name++ -'A';
    int index = atoi(name);
    *pin = port * 32 + index;
    return;
failed:
    *pin = -1;
}

static void makeData() {
    for (int i = 0; i < PIN_NUM; i++) {
        parse_pin(gpio_pin + i, physToGpio[i]);
    }
}

int gpioSetup() {
	int fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC);
	if (fd  < 0) {
        perrord("open() error");
        return 0;
    }
	unsigned int page_size = sysconf(_SC_PAGESIZE);
	unsigned int page_mask = (~(page_size - 1));
	unsigned int addr_start = SUNXI_GPIO_BASE & page_mask;
	unsigned int addr_offset = SUNXI_GPIO_BASE & ~page_mask;
    mmap_length = page_size*2;
	gpio_buf = (void *)mmap(0, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_start);
	if (gpio_buf == MAP_FAILED){
		perrord("mmap()");
		return 0;
	}

	pio_base = (unsigned int)gpio_buf;
	pio_base += addr_offset;

	close(fd);
	makeData();
	return 1;
}

void gpioFree() {
   	if(gpio_buf !=NULL && gpio_buf!=MAP_FAILED){
		if(munmap((void*)gpio_buf, mmap_length)!=0){
			perrord("munmap()");
		}
	}
}
