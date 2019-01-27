
#include "pinout.h"

#define GPIO_REG_CFG(B, N, I) ((B) + (N)*0x24 + ((I)<<2) + 0x00)
#define GPIO_REG_DLEVEL(B, N, I) ((B) + (N)*0x24 + ((I)<<2) + 0x14)
#define GPIO_REG_PULL(B, N, I) ((B) + (N)*0x24 + ((I)<<2) + 0x1C)
#define GPIO_REG_DATA(B, N) ((B) + (N)*0x24 + 0x10)

#define PUD_OFF 0
#define PUD_DOWN 2
#define PUD_UP 1

#define LE32TOH(X)  le32toh(*((uint32_t*)(X)))

size_t mmap_length=0;
volatile uint32_t *gpio_buf;

int gpio_port[PIN_NUM];
int gpio_port_num[PIN_NUM];


void pinWrite(int pin, int value) {
    uint32_t *addr, val;
    addr = (uint32_t*) GPIO_REG_DATA(gpio_buf, gpio_port[pin]);
    val = le32toh(*addr);
    if (value) {
        val |= (0x01 << gpio_port_num[pin]);
    } else {
        val &= ~(0x01 << gpio_port_num[pin]);
    }
    *addr = htole32(val);
}

int pinRead(int pin) {
    uint32_t val;
    val = LE32TOH(GPIO_REG_DATA(gpio_buf, gpio_port[pin]));
    return (int) (val >> gpio_port_num[pin]) & 0x01;
}

void pinLow(int pin) {
    uint32_t *addr, val;
    addr = (uint32_t*) GPIO_REG_DATA(gpio_buf, gpio_port[pin]);
    val = le32toh(*addr);
    val &= ~(0x01 << gpio_port_num[pin]);
    *addr = htole32(val);
}

void pinHigh(int pin) {
    uint32_t *addr, val;
    addr = (uint32_t*) GPIO_REG_DATA(gpio_buf, gpio_port[pin]);
    val = le32toh(*addr);
    val |= (0x01 << gpio_port_num[pin]);
    *addr = htole32(val);
}

void pinModeIn(int pin) {
    uint32_t *addr, val;
    uint32_t port_num_func;
    uint32_t offset_func;
    port_num_func = gpio_port_num[pin] >> 3;
    offset_func = ((gpio_port_num[pin] & 0x07) << 2);
    addr = (uint32_t*) GPIO_REG_CFG(gpio_buf, gpio_port[pin], port_num_func);
    val = le32toh(*addr);
    val &= ~(0x07 << offset_func);
    val |= (0 & 0x07) << offset_func;
    *addr = htole32(val);
}

void pinModeOut(int pin) {
    uint32_t *addr, val;
    uint32_t port_num_func;
    uint32_t offset_func;
    port_num_func = gpio_port_num[pin] >> 3;
    offset_func = ((gpio_port_num[pin] & 0x07) << 2);
    addr = (uint32_t*) GPIO_REG_CFG(gpio_buf, gpio_port[pin], port_num_func);
    val = le32toh(*addr);
    val &= ~(0x07 << offset_func);
    val |= (1 & 0x07) << offset_func;
    *addr = htole32(val);
}

void pinPUD(int pin, int pud) {
    uint32_t *addr, val;
    uint32_t port_num_pull;
    uint32_t offset_pull;
    port_num_pull = gpio_port_num[pin] >> 4;
    offset_pull = ((gpio_port_num[pin] & 0x0f) << 1);
    addr = (uint32_t*) GPIO_REG_PULL(gpio_buf, gpio_port[pin], port_num_pull);
    val = le32toh(*addr);
    val &= ~(0x03 << offset_pull);
    val |= (pud & 0x03) << offset_pull;
    *addr = htole32(val);
    delayUsIdle(1);
}

int checkPin(int pin) {
    if (pin < 0 || pin >= PIN_NUM) {
        return 0;
    }
    if (gpio_port[pin] == -1) {
        return 0;
    }
    if (gpio_port_num[pin] == -1) {
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
        parse_pin(gpio_port + i, gpio_port_num + i, physToGpio[i]);
    }
}

int gpioSetup() {
    int pagesize = sysconf(_SC_PAGESIZE);
    int fd;
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
        fprintf(stderr, "%s(): ", __func__);
        perror("open()");
        return 0;
    }
    int addr = 0x01c20800 & ~(pagesize - 1);
    int offset = 0x01c20800 & (pagesize - 1);
    size_t mmap_length=(0x800 + pagesize - 1) & ~(pagesize - 1);
    gpio_buf = mmap(NULL, mmap_length, PROT_WRITE | PROT_READ, MAP_SHARED, fd, addr);
    close(fd);
    if (gpio_buf == MAP_FAILED) {
        fprintf(stderr, "%s(): ", __func__);
        perror("mmap()");
        return 0;
    }
    gpio_buf += offset;
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
