#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <aio.h>
#include <signal.h>

#define TRUE 1

#define MAX_PROCESSES 600
#define ERROR -1
#define SLAVE 0
#define BUFFER_MAX_SIZE 524

#define SLEEP_TIME 60

int pids[MAX_PROCESSES] = {0};

// NO HACER CTRL+C

int main(void) {
  for(int i=0;i<MAX_PROCESSES;i++) {
    pid_t forkPID = fork();
    char buffer[BUFFER_MAX_SIZE];
    switch(forkPID) {
      case ERROR:
        perror("Fork error.");
        exit(ERROR);
      case SLAVE:
        sprintf(buffer, "echo \"ehlo slave%d\r\n\" | nc -C localhost 2525 > /tmp/null", i);
        system(buffer);
        while(TRUE);
      default:
        pids[i] = forkPID;
    }
  }
  sleep(SLEEP_TIME);
  for(int i=0;i<MAX_PROCESSES;i++) {
    kill(pids[i], 9);
  }
}