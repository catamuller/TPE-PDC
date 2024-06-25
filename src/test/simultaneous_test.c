#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <aio.h>
#include <signal.h>

#define TRUE 1

#define MAX_PROCESSES 800
#define DELAY (int)(MAX_PROCESSES / 7)

#define ERROR -1
#define SLAVE 0
#define BUFFER_MAX_SIZE 524

#define SLEEP_TIME 120

int pids[MAX_PROCESSES] = {0};

// NO HACER CTRL+C


static void sigterm_handler(const int signal) {
    for(int i=0;i<MAX_PROCESSES;i++) {
        if(pids[i] != 0)
            kill(pids[i], 9);
    }
}

int main(void) {
  signal(SIGTERM, sigterm_handler);
  for(int i=0;i<MAX_PROCESSES;i++) {
    const struct timespec ts ={
      .tv_nsec = 100000000,
      .tv_sec  = 0
    };
    nanosleep(&ts,NULL);
    pid_t forkPID = fork();
    char buff[256];
    switch(forkPID) {
      case ERROR:
        perror("Fork error.");
        exit(ERROR);
      case SLAVE:
        sprintf(buff, "{ printf \"EHLO stress_test\\r\\n\"; sleep %d && printf \"QUIT\\r\\n\"; } | nc -C localhost 2525 > /tmp/null &", DELAY);
        system(buff);
        while(1);
      default:
        pids[i] = forkPID;
    }
  }
  sleep(SLEEP_TIME);
  for(int i=0;i<MAX_PROCESSES;i++) {
    kill(pids[i], 9);
  }
}
