#include "util.h"

void dumpStr(const char *buf) {
    int i;
    for (i = 0; i < strlen(buf); i++) {
        printf("%hhu.", buf[i]);
    }
    putchar('\n');
}

void strnline(char **v) {
    char *c;
    c = strchr(*v, '\n');
    if (c == NULL) {
        *v = strchr(*v, '\0');
    } else {
        *v = c + 1;
        if (**v == '\n') {
            *v = strchr(*v, '\0');
        }
    }
}

char * bufCat(char * buf, const char *str, size_t size) {
    if (strlen(buf) + strlen(str) + 1 >= size) {
#ifdef MODE_DEBUG
        fprintf(stderr,"%s(): buffer overflow\n", __func__);
#endif
        return NULL;
    }
    return strcat(buf, str);
}

char * estostr(char *s) {
    int i, j = 0;
    int f = 0;
    char ss[strlen(s) + 1];
    memset(ss, 0, sizeof ss);
    for (i = 0; i < strlen(s) + 1; i++) {

        if (s[i] == '\\') {
            f = 1;
            continue;
        }
        if (f) {
            switch (s[i]) {
                case 'n':
                    ss[j] = '\n';
                    break;
                case 'r':
                    ss[j] = '\r';
                    break;
                case 't':
                    ss[j] = '\t';
                    break;
                case '0':
                    ss[j] = '\0';
                    break;
                case '\\':
                    ss[j] = '\\';
                    break;
                default:
                    ss[j] = '\\';
                    j++;
                    ss[j] = s[i];
                    break;
            }
            j++;
            f = 0;
            continue;
        }
        ss[j] = s[i];
        j++;
    }
    return strcpy(s, ss);
}

/*
absolute difference
 */
double adifd(double v1, double v2) {
    if (v1 > v2) {
        return v1 - v2;
    } else if (v2 > v1) {
        return v2 - v1;
    }
    return 0.0;
}

/*
check if values are approximately equal
 */
int aeq(double v1, double v2, double acr) {
    double ad = adifd(v1, v2);
    if (ad > acr) {
        return 0;
    }
    return 1;
}

int get_rand_fu(void *buf, size_t nbytes) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "%s(): ", __func__);
        perror("open()");
        return 0;
    }
    ssize_t n = read(fd, buf, nbytes);
    if (n == -1) {
        fprintf(stderr, "%s(): ", __func__);
        perror("read()");
        return 0;
    }
    close(fd);
    return 1;
}

int get_rand(void *buf, size_t nbytes) {
    for (int i = 0; i < nbytes; i++) {
        srand(clock());
        uint8_t *p = (uint8_t *) buf;
        p[i] = (uint8_t) rand();
    }
    return 1;
}

int get_rand_int(int min, int max) {
    srand(clock());
    int x = rand();
    int range = (max - min);
    int div = INT_MAX / range;
    int out = min + (x / div);
    return out;
}

unsigned int get_randb_uint(unsigned int min, unsigned int max) {
    srand(clock());
    unsigned int x = rand();
    unsigned int range = (max - min);
    unsigned int div = INT_MAX / range;
    unsigned int out = min + (x / div);
    return out;
}

unsigned int get_between_uint(unsigned int val,unsigned int min, unsigned int max) {
    unsigned int range = (max - min);
    unsigned int div = INT_MAX / range;
    unsigned int out = min + (val / div);
    return out;
}