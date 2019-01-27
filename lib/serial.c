
#include "serial.h"

int initSerial(int *fd, const char *device, const int baud) {
    *fd = serialOpen(device, baud);
    if (*fd == -1) {
        return 0;
    }
    return 1;
}

int serialOpen(const char *device, const int baud) {
    struct termios options;
    speed_t myBaud;
    int status, fd;

    switch (baud) {
        case 50:
            myBaud = B50;
            break;
        case 75:
            myBaud = B75;
            break;
        case 110:
            myBaud = B110;
            break;
        case 134:
            myBaud = B134;
            break;
        case 150:
            myBaud = B150;
            break;
        case 200:
            myBaud = B200;
            break;
        case 300:
            myBaud = B300;
            break;
        case 600:
            myBaud = B600;
            break;
        case 1200:
            myBaud = B1200;
            break;
        case 1800:
            myBaud = B1800;
            break;
        case 2400:
            myBaud = B2400;
            break;
        case 4800:
            myBaud = B4800;
            break;
        case 9600:
            myBaud = B9600;
            break;
        case 19200:
            myBaud = B19200;
            break;
        case 38400:
            myBaud = B38400;
            break;
        case 57600:
            myBaud = B57600;
            break;
        case 115200:
            myBaud = B115200;
            break;
        case 230400:
            myBaud = B230400;
            break;

        default:
            return -2;
    }

    if ((fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1) {
        fprintf(stderr, "%s(): ", __func__);
        perror("open()");
        return -1;
    }

    fcntl(fd, F_SETFL, O_RDWR);

    tcgetattr(fd, &options);

    cfmakeraw(&options);
    cfsetispeed(&options, myBaud);
    cfsetospeed(&options, myBaud);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    options.c_cc [VMIN] = 0;
    options.c_cc [VTIME] = 100;

    tcsetattr(fd, TCSANOW | TCSAFLUSH, &options);

    ioctl(fd, TIOCMGET, &status);

    status |= TIOCM_DTR;
    status |= TIOCM_RTS;

    ioctl(fd, TIOCMSET, &status);

    usleep(10000); // 10mS

    return fd;
}

void serialFlush(const int fd) {
    tcflush(fd, TCIOFLUSH);
}

void serialClose(const int fd) {
    close(fd);
}

int serialAvailable(const int fd) {
    int result;
    if (ioctl(fd, FIONREAD, &result) == -1) {
        fprintf(stderr, "%s(): ", __func__);
        perror("ioctl()");
        return 0;
    }
    return result;
}

int serialWaitAvailable(int fd, struct timespec max_time) {
    Ton_ts tmr = {.ready = 0};
    while (1) {
        if (serialAvailable(fd)) {
            return 1;
        }
        if (ton_ts(max_time, &tmr)) {
            return 0;
        }
        delayUsIdle(100);
    }
}

int serialPuts(const int fd, char *str) {
#ifdef MODE_DEBUG
    printf("serialPuts: %s\n", str);
#endif
    size_t n, sn;
    sn = strlen(str);
    n = write(fd, str, strlen(str));
    if (n < sn) {
        return 0;
    }
    return 1;
}

void serialRead(int fd, void *buf, size_t buf_size) {
    uint8_t x;
    size_t i = 0;
    uint8_t * b = (uint8_t *) buf;
    while (i < buf_size && read(fd, &x, 1) == 1) {
        b[i] = x;
        i++;
    }
#ifdef MODE_DEBUG
    printf("response: %s\n", b);
    for (i = 0; i < buf_size; i++) {
        printf("%hhu.", b[i]);
    }
    puts(" ");
#endif
}



