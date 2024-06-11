#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "lib/headers/args.h"
#include "lib/headers/selector.h"
#include "lib/headers/utils.h"
#include "lib/headers/smtp.h"

#define MAX_REQUESTS 20
#define INITIAL_SELECTOR 1024

static bool done = false;

static void sigterm_handler(const int signal) {
    printf("Signal %d, cleaning up and exiting\n",signal);
    done = true;
}

int main(int argc, char ** argv) {
  unsigned int port = 2525;

  struct smtpargs args;
  args.mail_dir = "./mail";

  parse_args(argc, argv, &args);


  close(0);

  const char       *err_msg = NULL;
  selector_status   ss      = SELECTOR_SUCCESS;
  fd_selector selector      = NULL;

  int master_sockets[2] = {-1, -1}; // IPv4 e IPv6
  struct sockaddr * addresses[2];
  if (init_addresses(addresses, &err_msg) < 0) goto finally;
  ip_addr(addresses, port);
  if (init_sockets(master_sockets, &err_msg) < 0) goto finally;
  mkmaildir(args.mail_dir);
  
  fprintf(stdout, "Listening on SMTP port %d\n", port);
  if (setsockopts(master_sockets, &err_msg) < 0) goto finally;
  if (bindips(master_sockets, addresses, &err_msg) < 0) goto finally;
  if (listenips(master_sockets, MAX_REQUESTS, &err_msg)) goto finally;

  signal(SIGTERM, sigterm_handler);
  signal(SIGINT, sigterm_handler);

  const struct selector_init conf = {
    .signal = SIGALRM,
    .select_timeout = {
        .tv_sec  = 10,
        .tv_nsec = 0,
    }
  };

  if (selector_init(&conf) != 0) {
    err_msg = "Unable to initialize selector.";
    goto finally;
  }

  selector = selector_new(INITIAL_SELECTOR);
  if (selector == NULL) {
    err_msg = "Unable to create selector";
    goto finally;
  }

  const struct fd_handler smtp = {
    .handle_read = smtp_passive_accept,
    .handle_write = NULL,
    .handle_close = NULL,
  };

  ss = selector_register(selector,master_sockets[0],&smtp,OP_READ,NULL);// 2 ss?
  if (ss != SELECTOR_SUCCESS) {
    err_msg = "Unable to register FD for IPv4.";
    goto finally;
  }

  ss = selector_register(selector,master_sockets[1],&smtp,OP_READ,NULL);// 2 ss?
  if (ss != SELECTOR_SUCCESS) {
    err_msg = "Unable to register FD for IPv6.";
    goto finally;
  }

  while(!done) {
    err_msg = NULL;
    ss = selector_select(selector);
    if (ss != SELECTOR_SUCCESS) {
      err_msg = "Unable to serve for selector.";
      goto finally;
    }
  }
  if (err_msg == NULL) {
    err_msg = "Closing...";
  }

  finally:
  if(ss != SELECTOR_SUCCESS) {
    fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "": err_msg,
                                  ss == SELECTOR_IO
                                  ? strerror(errno)
                                  : selector_error(ss));
  } else if(err_msg) {
    perror(err_msg);
  }
  if (selector != NULL)
    selector_destroy(selector);
  selector_close();
  close_sockets(master_sockets);
  return 0;
  
}