
#include "spi.h"

int spi_rw(SPI *item, unsigned char *data, unsigned int len) {
    struct spi_ioc_transfer spi;
    spi.tx_buf = (unsigned long) data;
    spi.rx_buf = (unsigned long) data;
    spi.len = len;
    spi.delay_usecs = 0;
    spi.speed_hz = item->speed;
    spi.bits_per_word = item->bpw;

    return ioctl(item->fd, SPI_IOC_MESSAGE(1), &spi);
}

int spi_setup(SPI *item) {
    int fd = open(item->path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s(): %s", F, strerror(errno));
    }
    int mode = 0;
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        fprintf(stderr, "%s(): %s", F, strerror(errno));
        return 0;
    }
    item->bpw = 8;
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &item->bpw) < 0) {
        fprintf(stderr, "%s(): %s", F, strerror(errno));
        return 0;
    }
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &item->speed) < 0) {
        fprintf(stderr, "%s(): %s", F, strerror(errno));
        return 0;
    }
    return fd;
}
