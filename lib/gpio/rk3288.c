
#include "pinout.h"

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE  (4*1024)

#define RK3288_GPIO(x)  (GPIO0_BASE+x*GPIO_LENGTH+(x>0)*GPIO_CHANNEL)
#define GPIO_LENGTH   0x00010000
#define GPIO_CHANNEL   0x00020000
#define GPIO0_BASE  0xff750000
#define GPIO_BANK  9
#define RK3288_GRF_PHYS  0xff770000

#define GPIO_BANK_NUM  9

#define RK3288_PMU  0xff730000
#define PMU_GPIO0C_IOMUX 0x008c
#define PMU_GPIO0C_P  0x006c

#define GPIO_SWPORTA_DR_OFFSET  0x0000
#define GPIO_SWPORTA_DDR_OFFSET  0x0004
#define GPIO_EXT_PORTA_OFFSET  0x0050

#define GRF_GPIO5B_P 0x0184
#define GRF_GPIO5C_P 0x0188
#define GRF_GPIO6A_P 0x0190
#define GRF_GPIO6B_P 0x0194
#define GRF_GPIO6C_P 0x0198
#define GRF_GPIO7A_P 0x01a0
#define GRF_GPIO7B_P 0x01a4
#define GRF_GPIO7C_P 0x01a8
#define GRF_GPIO8A_P 0x01b0
#define GRF_GPIO8B_P 0x01b4

#define GPIO0_C1  17   //7----->17

#define GPIO5_B0  (8+152)  //7----->160
#define GPIO5_B1  (9+152)  //8----->161
#define GPIO5_B2  (10+152) //7----->162
#define GPIO5_B3  (11+152) //7----->163
#define GPIO5_B4  (12+152) //7----->164
#define GPIO5_B5  (13+152) //7----->165
#define GPIO5_B6  (14+152) //7----->166
#define GPIO5_B7  (15+152) //7----->167
#define GPIO5_C0  (16+152) //7----->168
#define GPIO5_C1  (17+152) //7----->169
#define GPIO5_C2  (18+152) //7----->170
#define GPIO5_C3  (19+152) //7----->171

#define GPIO6_A0  (184)  //7----->184
#define GPIO6_A1  (1+184)  //7----->185
#define GPIO6_A3  (3+184)  //7----->187
#define GPIO6_A4  (4+184)  //7----->188

#define GPIO7_A7  (7+216)  //7----->223
#define GPIO7_B0  (8+216)  //7----->224
#define GPIO7_B1  (9+216)  //7----->225
#define GPIO7_B2  (10+216) //7----->226
#define GPIO7_C1  (17+216) //7----->233
#define GPIO7_C2  (18+216) //7----->234
#define GPIO7_C6  (22+216) //7----->238
#define GPIO7_C7  (23+216) //7----->239

#define GPIO8_A3  (3+248)  //7----->251
#define GPIO8_A4  (4+248)  //3----->252
#define GPIO8_A5  (5+248)  //5----->253
#define GPIO8_A6  (6+248)  //7----->254
#define GPIO8_A7  (7+248)  //7----->255
#define GPIO8_B0  (8+248)  //7----->256
#define GPIO8_B1  (9+248)  //7----->257

#define GRF_GPIO5B_IOMUX 0x0050
#define GRF_GPIO5C_IOMUX 0x0054
#define GRF_GPIO6A_IOMUX 0x005c
#define GRF_GPIO6B_IOMUX 0x0060
#define GRF_GPIO6C_IOMUX 0x0064
#define GRF_GPIO7A_IOMUX 0x006c
#define GRF_GPIO7B_IOMUX 0x0070
#define GRF_GPIO7CL_IOMUX 0x0074
#define GRF_GPIO7CH_IOMUX 0x0078
#define GRF_GPIO8A_IOMUX 0x0080
#define GRF_GPIO8B_IOMUX 0x0084

#define PUD_OFF 0
#define PUD_DOWN 2
#define PUD_UP 1

#define RK3288_GRF_PHYS  0xff770000


volatile uint32_t *gpio[GPIO_BANK_NUM];
volatile uint32_t *grf;
volatile uint32_t *pmu;

int gpio_pin[PIN_NUM];

static void pinModeToGPIO(int pin) {
    switch (pin) {
            //GPIO0
        case 17:
            *(pmu + PMU_GPIO0C_IOMUX / 4) = (*(pmu + PMU_GPIO0C_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;
            //GPIO1D0:act-led
        case 48:
            break;
            //GPIO5B
        case 160:
        case 161:
        case 162:
        case 163:
        case 164:
        case 165:
        case 166:
        case 167:
            *(grf + GRF_GPIO5B_IOMUX / 4) = (*(grf + GRF_GPIO5B_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;

            //GPIO5C
        case 168:
        case 169:
        case 170:
        case 171:
            *(grf + GRF_GPIO5C_IOMUX / 4) = (*(grf + GRF_GPIO5C_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;

            //GPIO6A
        case 184:
        case 185:
        case 187:
        case 188:
            *(grf + GRF_GPIO6A_IOMUX / 4) = (*(grf + GRF_GPIO6A_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;

            //GPIO7A7
        case 223:
            *(grf + GRF_GPIO7A_IOMUX / 4) = (*(grf + GRF_GPIO7A_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;

            //GPIO7B
        case 224:
        case 225:
        case 226:
            *(grf + GRF_GPIO7B_IOMUX / 4) = (*(grf + GRF_GPIO7B_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;
            //GPIO7C
        case 233:
        case 234:
            *(grf + GRF_GPIO7CL_IOMUX / 4) = (*(grf + GRF_GPIO7CL_IOMUX / 4) | (0x0f << (16 + (pin % 8)*4))) & (~(0x0f << ((pin % 8)*4)));
            break;
        case 238:
        case 239:
            *(grf + GRF_GPIO7CH_IOMUX / 4) = (*(grf + GRF_GPIO7CH_IOMUX / 4) | (0x0f << (16 + (pin % 8 - 4)*4))) & (~(0x0f << ((pin % 8 - 4)*4)));
            break;

            //GPIO8A
        case 251:
        case 254:
        case 255:
        case 252:
        case 253:
            *(grf + GRF_GPIO8A_IOMUX / 4) = (*(grf + GRF_GPIO8A_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;
            //GPIO8B
        case 256:
        case 257:
            *(grf + GRF_GPIO8B_IOMUX / 4) = (*(grf + GRF_GPIO8B_IOMUX / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2)));
            break;
        default:
            break;
    }
}

void pinWrite(int pin, int value) {
    pin = gpio_pin[pin];
    if (value == 1) {
        if (pin >= 24) {
            *(gpio[(pin + 8) / 32] + GPIO_SWPORTA_DR_OFFSET / 4) |= (1 << ((pin - 24) % 32));
        } else {
            *(gpio[pin / 32] + GPIO_SWPORTA_DR_OFFSET / 4) |= (1 << (pin % 32));
        }
    } else {
        if (pin >= 24) {
            *(gpio[(pin + 8) / 32] + GPIO_SWPORTA_DR_OFFSET / 4) &= ~(1 << ((pin - 24) % 32));
        } else {
            *(gpio[pin / 32] + GPIO_SWPORTA_DR_OFFSET / 4) &= ~(1 << (pin % 32));
        }

    }
}

int pinRead(int pin) {
    pin = gpio_pin[pin];
    int value, mask;
    if (pin >= 24) {
        mask = (1 << (pin - 24) % 32);
        value = (((*(gpio[(pin - 24) / 32 + 1] + GPIO_EXT_PORTA_OFFSET / 4)) & mask)>>(pin - 24) % 32);
    } else {
        mask = (1 << pin % 32);
        value = (((*(gpio[pin / 32] + GPIO_EXT_PORTA_OFFSET / 4)) & mask) >> pin % 32);
    }
    return value;
}

void pinLow(int pin) {
    pin = gpio_pin[pin];
    if (pin >= 24) {
        *(gpio[(pin + 8) / 32] + GPIO_SWPORTA_DR_OFFSET / 4) &= ~(1 << ((pin - 24) % 32));
    } else {
        *(gpio[pin / 32] + GPIO_SWPORTA_DR_OFFSET / 4) &= ~(1 << (pin % 32));
    }
}

void pinHigh(int pin) {
    pin = gpio_pin[pin];
    if (pin >= 24) {
        *(gpio[(pin + 8) / 32] + GPIO_SWPORTA_DR_OFFSET / 4) |= (1 << ((pin - 24) % 32));
    } else {
        *(gpio[pin / 32] + GPIO_SWPORTA_DR_OFFSET / 4) |= (1 << (pin % 32));
    }
}

void pinModeIn(int pin) {
    pin = gpio_pin[pin];
    pinModeToGPIO(pin);
    if (pin >= 24) {
        *(gpio[(pin + 8) / 32] + GPIO_SWPORTA_DDR_OFFSET / 4) &= ~(1 << ((pin + 8) % 32));
    } else {
        *(gpio[pin / 32] + GPIO_SWPORTA_DDR_OFFSET / 4) &= ~(1 << (pin % 32));
    }
}

void pinModeOut(int pin) {
    pin = gpio_pin[pin];
    pinModeToGPIO(pin);
    if (pin >= 24) {
        *(gpio[(pin + 8) / 32] + GPIO_SWPORTA_DDR_OFFSET / 4) |= (1 << ((pin + 8) % 32));
    } else {
        *(gpio[pin / 32] + GPIO_SWPORTA_DDR_OFFSET / 4) |= (1 << (pin % 32));
    }
}

void pinPUD(int pin, int pud) {
    pin = gpio_pin[pin];
    static int bit0, bit1;
    if (pud == PUD_UP) {
        bit0 = 1;
        bit1 = 0;
    } else if (pud == PUD_DOWN) {
        bit0 = 0;
        bit1 = 1;
    } else {
        bit0 = 0;
        bit1 = 0;
    }

    switch (pin) {
            //GPIO0
        case 17:
            *(pmu + PMU_GPIO0C_P / 4) = (*(grf + PMU_GPIO0C_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;
            //case 17 : value =  0x00000003;  break;
            //GPIO5B
        case 160:
        case 161:
        case 162:
        case 163:
        case 164:
        case 165:
        case 166:
        case 167:
            *(grf + GRF_GPIO5B_P / 4) = (*(grf + GRF_GPIO5B_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;

            //GPIO5C
        case 168:
        case 169:
        case 170:
        case 171:
            *(grf + GRF_GPIO5C_P / 4) = (*(grf + GRF_GPIO5C_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;

            //GPIO6A
        case 184:
        case 185:
        case 187:
        case 188:
            *(grf + GRF_GPIO6A_P / 4) = (*(grf + GRF_GPIO6A_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;

            //GPIO7A7
        case 223:
            *(grf + GRF_GPIO7A_P / 4) = (*(grf + GRF_GPIO7A_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;

            //GPIO7B
        case 224:
        case 225:
        case 226:
            *(grf + GRF_GPIO7B_P / 4) = (*(grf + GRF_GPIO7B_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;
            //GPIO7C
        case 233:
        case 234:
        case 238:
        case 239:
            *(grf + GRF_GPIO7C_P / 4) = (*(grf + GRF_GPIO7C_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;

            //GPIO8A
        case 251:
        case 254:
        case 255:
        case 252:
        case 253:
            *(grf + GRF_GPIO8A_P / 4) = (*(grf + GRF_GPIO8A_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;
            //GPIO8B
        case 256:
        case 257:
            *(grf + GRF_GPIO8B_P / 4) = (*(grf + GRF_GPIO8B_P / 4) | (0x03 << ((pin % 8)*2 + 16))) & (~(0x03 << ((pin % 8)*2))) | (bit1 << ((pin % 8)*2 + 1)) | (bit0 << ((pin % 8)*2));
            break;
        default:
            break;
    }
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

static void parse_pin(int *npin, const char *name) {
    if (strlen(name) < 4) {
        goto failed;
    }
    if (*name != 'P') {
        goto failed;
    }
    int p1, p3;
    char p2;
    name++;
    int n = sscanf(name, "%d%c%d", &p1, &p2, &p3);
    if (n != 3) {
        goto failed;
    }
    int pp1 = p1 * 32 - 8;
    if (pp1 < 0) {
        pp1 = 0;
    }
    int pp2 = (p2 - 'A')*8 + p3;
    *npin = pp1 + pp2;
    //  printf("%s %d %d\n",name, pp1, pp2);
    return;
failed:
    *npin = -1;
}

static void makeData() {
    for (int i = 0; i < PIN_NUM; i++) {
        parse_pin(gpio_pin + i, physToGpio[i]);
    }
}

int gpioSetup() {
    int fd;
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
        fprintf(stderr, "%s(): ", __func__);
        perror("open()");
        return 0;
    }

    for (int i = 0; i < GPIO_BANK_NUM; i++) {
        gpio[i] = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, RK3288_GPIO(i));
        if (gpio[i] == MAP_FAILED) {
            fprintf(stderr, "%s(): ", __func__);
            perror("mmap1()");
            close(fd);
            return 0;
        }
    }

    grf = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, RK3288_GRF_PHYS);
    if (grf == MAP_FAILED) {
        fprintf(stderr, "%s(): ", __func__);
        perror("mmap2()");
        close(fd);
        return 0;
    }
    pmu = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, RK3288_PMU);
    if (pmu == MAP_FAILED) {
        fprintf(stderr, "%s(): ", __func__);
        perror("mmap3()");
        close(fd);
        return 0;
    }
    close(fd);
    makeData();
    return 1;
}

void gpioFree() {
    if(grf !=NULL && grf!=MAP_FAILED){
		if(munmap((void*)grf, PAGE_SIZE)!=0){
			perrord("munmap()");
		}
	}
    if(pmu !=NULL && pmu!=MAP_FAILED){
		if(munmap((void*)pmu, PAGE_SIZE)!=0){
			perrord("munmap()");
		}
	}
}
