
#include "tcp.h"

int initServer(int *fd, int port) {
    struct sockaddr_in addr;
    if ((*fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
#ifdef MODE_DEBUG
        perror("initServer: socket()\n");
#endif
        return 0;
    }
    memset((char *) &addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int flag = 1;
    if (setsockopt(*fd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof (int)) < 0) {
#ifdef MODE_DEBUG
        perror("initServer: setsockopt()");
#endif
        freeSocketFd(fd);
        return 0;
    }
    if (bind(*fd, (struct sockaddr*) (&addr), sizeof (addr)) == -1) {
#ifdef MODE_DEBUG
        perror("initServer: bind()");
#endif
        freeSocketFd(fd);
        return 0;
    }
    if (listen(*fd, 7) != 0) {
#ifdef MODE_DEBUG
        perror("initServer: listen()");
#endif
        freeSocketFd(fd);
        return 0;
    }
    return 1;
}

void freeSocketFd(int *udp_fd) {
    if (*udp_fd != -1) {
        if (close(*udp_fd) == -1) {
#ifdef MODE_DEBUG
            perror("freeSocketFd: close()");
#endif
        }
        *udp_fd = -1;
    }
}

int initClient(int *fd, __time_t tmo) {
    if ((*fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
#ifdef MODE_DEBUG
        perror("initClient: socket()");
#endif
        return 0;
    }
    struct timeval tv = {tmo, 0};
    if (setsockopt(*fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv)) < 0) {
#ifdef MODE_DEBUG
        perror("initClient: timeout set failure");
#endif
        freeSocketFd(fd);
        return 0;
    }
    return 1;
}

int makeClientAddr(struct sockaddr_in *addr, const char *addr_str, int port) {
    memset(addr, 0, sizeof (*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (inet_aton(addr_str, &(addr->sin_addr)) == 0) {
#ifdef MODE_DEBUG
        perror("makeClientAddr: check address");
#endif
        return 0;
    }
    return 1;
}

int sendBuf(void *buf, size_t buf_size, int fd, struct sockaddr *addr, socklen_t addr_len) {
    if (write(fd, buf, buf_size) < 0) {
#ifdef MODE_DEBUG
        fprintf(stderr, "sendBuf: error writing to socket (%64s)", (char *) buf);
        perror("detail");
#endif
        return 0;
    }
}

int serverRead(int *fd_cl,void *buf, size_t buf_size, int fd, struct sockaddr * addr, socklen_t * addr_len) {
   *fd_cl = accept(fd, addr, addr_len);
    if (*fd_cl < 0) {
#ifdef MODE_DEBUG
        perror("serverRead: accept()");
#endif
        return 0;
    }
    if (recv(*fd_cl, buf, buf_size, 0) < 0) {
#ifdef MODE_DEBUG
        perror("serverRead: recv()");
#endif
        close(*fd_cl);
        return 0;
    }
    return 1;
}
void serverFinish(int fd_cl){
    close(fd_cl);
}
int clientConnect(int fd, struct sockaddr * addr, socklen_t * addr_len) {
    if (connect(fd, addr, *addr_len) == -1) {
#ifdef MODE_DEBUG
        perror("clientConnect: connect()");
#endif
        return 0;
    }
    return 1;
}

int clientRead(int fd, void *buf, size_t buf_size) {
    if (recv(fd, buf, buf_size, 0) < 0) {
#ifdef MODE_DEBUG
        perror("clientRead: recv()");
#endif
        return 0;
    }
    return 1;
}