#ifndef UTILS_H
#define UTILS_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>

void ipv4_addr(struct sockaddr * address, unsigned int port);
void ipv6_addr(struct sockaddr * address, unsigned int port);
void ip_addr(struct sockaddr * addresses[2], unsigned int port);
int init_sockets(int fds[2], const char ** errmsg);
void close_sockets(int fds[2]);
int init_addresses(struct sockaddr * addresses[2], const char ** errmsg);
int setsockoptip(int fd);
int setsockoptipv6(int fd);
int setsockopts(const int fds[2], const char ** errmsg);
void mkmaildir(const char * dirname);
int bindipv4(int fd, struct sockaddr* address);
int bindipv6(int fd, struct sockaddr* address);
int bindips(int fds[2], struct sockaddr* addresses[2], const char ** errmsg);
int listenips(const int fds[2], int max_requests, const char ** errmsg);
#endif