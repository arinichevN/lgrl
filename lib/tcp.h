
#ifndef LIBPAS_TCP_H
#define LIBPAS_TCP_H
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "common.h"

extern int initServer(int *fd, int port);

extern void freeSocketFd(int *udp_fd);

extern int initClient(int *fd, __time_t tmo);

extern int makeClientAddr(struct sockaddr_in *addr, const char *addr_str, int port);

extern int sendBuf(void *buf, size_t buf_size, int fd, struct sockaddr *addr, socklen_t addr_len);

extern int serverRead(int *fd_cl,void *buf, size_t buf_size, int fd, struct sockaddr * addr, socklen_t * addr_len);

extern void serverFinish(int fd_cl);

extern int clientConnect(int fd, struct sockaddr * addr, socklen_t * addr_len) ;

extern int clientRead(int fd, void *buf, size_t buf_size);

#endif

