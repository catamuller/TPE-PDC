#include "headers/utils.h"

#define SUCCESS 1
#define ERROR -1

void ipv4_addr(struct sockaddr * address, unsigned int port) {
  ((struct sockaddr_in *) address)->sin_family = AF_INET;
  ((struct sockaddr_in *) address)->sin_addr.s_addr = htonl(INADDR_ANY);
  ((struct sockaddr_in *) address)->sin_port = htons(port);
}

void ipv6_addr(struct sockaddr * address, unsigned int port) {
  ((struct sockaddr_in6 *) address)->sin6_family = AF_INET6;
  ((struct sockaddr_in6 *) address)->sin6_addr = in6addr_any;
  ((struct sockaddr_in6 *) address)->sin6_port = htons(port);
}

void ip_addr(struct sockaddr * addresses[2], unsigned int port) {
  ipv4_addr(addresses[0], port);
  ipv6_addr(addresses[1], port);
}

int init_addresses(struct sockaddr * addresses[2], const char ** errmsg) {
  if ((addresses[0] = calloc(1, sizeof(struct sockaddr_in))) == NULL) {
    *errmsg = "Unable to create memory for IPv4 address.";
    return ERROR;
  }
  if ((addresses[1] = calloc(1, sizeof(struct sockaddr_in6))) == NULL) {
    *errmsg = "Unable to create memory for IPv6 address.";
    return ERROR;
  }
  return SUCCESS;
}

int setsockoptip(int fd) {
  return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int));
}

int setsockopts(const int fds[2], const char ** errmsg) {
  if (setsockoptip(fds[0]) < 0) {
    *errmsg = "Unable to set IPv4 socket options.";
    return ERROR;
  }
  if (setsockoptip(fds[1]) < 0) {
    *errmsg = "Unable to set IPv6 socket options.";
    return ERROR;
  }
  return SUCCESS;
}

int init_sockets(int fds[2], const char ** errmsg) {
  if ((fds[0] = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    *errmsg = "Unable to create socket for IPv4.";
    return fds[0];
  }
  if ((fds[1] = socket(AF_INET6,SOCK_STREAM,0)) < 0) {
    *errmsg = "Unable to create socket for IPv6.";
    return fds[1];
  }
  return SUCCESS;
}

void close_sockets(int fds[2]) {
  if (fds[0] >= 0)
    close(fds[0]);
  if (fds[1] >= 0)
    close(fds[1]);
}

void mkmaildir(const char * dirname) {
  struct stat st = {0};
  if (stat(dirname, &st) == -1)
    mkdir(dirname, 0700);
}

int bindipv4(int fd, struct sockaddr* address) {
  return bind(fd, address,sizeof(struct sockaddr_in));
}

int bindipv6(int fd, struct sockaddr* address) {
  return bind(fd, address,sizeof(struct sockaddr_in6));
}

int bindips(int fds[2], struct sockaddr* addresses[2], const char ** errmsg) {
  int bind = 0;
  if ((bind = bindipv4(fds[0], addresses[0])) < 0) {
    *errmsg = "Unable to bind IPv4 socket.";
    return bind;
  }
  if ((bind = bindipv6(fds[1], addresses[1])) < 0) {
    *errmsg = "Unable to bind IPv6 socket.";
    return bind;
  }
  return SUCCESS;
}

int listenips(const int fds[2], int max_requests, const char ** errmsg) {
  int value = 0;
  if ((value = listen(fds[0], max_requests)) < 0) {
    *errmsg = "Unable to listen to IPv4 socket.";
    return ERROR;
  }
  if ((value = listen(fds[1], max_requests)) < 0) {
    *errmsg = "Unable to listen to IPv6 socket.";
    return ERROR;
  }
  return SUCCESS;
}
