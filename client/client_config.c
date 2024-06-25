#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include "headers/args.h"
#include "../monitoring/lib/headers/connections_server_handler.h"
#define  BUFF_COMMAND_SIZE 256

char command_buffer[BUFF_COMMAND_SIZE]={0};// se guarda el comando
int socket_fd;

static void sig_term_handler(){
    printf("\nExiting command promt\n");
    close(socket_fd);
}

static void display_help(){
    printf(
        "Commands available:\n\n"
        "QUIT quits the session.\n"
        "logging=<(1|0)> enables or disables server logging.\n"
        );
}

int parse_promt(char* command_buffer){
    char* token;
    const char* delim="=";
    token=strtok(command_buffer,delim);
    if(strcasecmp(token,"QUIT\n")==0){
        printf("Bye Bye\n");
        return -1;
    }
    if(strcasecmp(token,"HELP\n")==0){
        display_help();
        return 1;
    }

    if(token==NULL||strcasecmp(token,"logging")!=0){
        printf("Invalid command format\n");
        return 1;
    }
    token=strtok(NULL,delim);

    return 0;
}


int main(int argc, char** argv){

    struct clientsargs args_struct;

    struct clientsargs * args = &args_struct;

    parse_args(argc, argv, args);


    socket_fd=connect_to_server(args->server_addr,args->server_port);
    if(socket_fd<0){
        printf("Error creating socket\n");
        goto final;
    }

    signal(SIGTERM,sig_term_handler);
    signal(SIGINT,sig_term_handler);

    char server_command[BUFF_COMMAND_SIZE];

    printf("\n>");
    while(fgets(command_buffer, BUFF_COMMAND_SIZE, stdin)!=NULL){


    int len=strlen(command_buffer);
    strncpy(server_command,command_buffer,len-1);
    server_command[len]='\0';


    int parse_result=parse_promt(command_buffer);
    if(parse_result==-1){
        goto exit;
    }
    else if(parse_result==1){
        printf("\n>");
        continue;
    }
    if (send(socket_fd,server_command,strlen(server_command),0)<0 ){
        printf("Failed to send command to server\n");
        goto connection_error;
    }

    ssize_t received=recv(socket_fd,command_buffer,BUFF_COMMAND_SIZE-1,0);
    if(received<0){
        printf("Error receiving server response\n");
        goto connection_error;
    }

    command_buffer[received]='\0';
    printf("Server response: %s\n>",command_buffer);
    }

    exit:
        close(socket_fd);
        return 0;
    connection_error:
        close(socket_fd);
    final:
        return 1;
}
