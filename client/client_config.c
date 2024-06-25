#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "headers/args.h"
#include "../monitoring/lib/headers/connections_server_handler.h"
#define  BUFF_COMMAND_SIZE 256


int main(int argc, char** argv){

    struct clientsargs args_struct;
    char command_buffer[BUFF_COMMAND_SIZE]={0};// se guarda el comando

    struct clientsargs * args = &args_struct;

    parse_args(argc, argv, args);

    int socket;
    if ((socket=connect_to_server(args->server_addr, args->server_port)) < 1) {
        perror("Failed to connect to server\n");
        goto final;
    }


    if(send(socket,command_buffer,BUFF_COMMAND_SIZE,0)==-1){
        perror("Failed to send command to server");
        goto final;
    }
    char server_command[BUFF_COMMAND_SIZE];
    char* token;
    const char* delim="=";


    while(fgets(command_buffer, BUFF_COMMAND_SIZE, stdin)!=NULL){
    strncpy(server_command,command_buffer,BUFF_COMMAND_SIZE);
        token=strtok(command_buffer,delim);
        if(token==NULL){
            perror("Invalid command format");
            continue;
        }
        token=strtok(NULL,delim);
        if(token==NULL){
            perror("Invalid command format");
            continue;
        }
        send(socket,server_command,strlen(server_command),0);

        ssize_t received=recv(socket,command_buffer,BUFF_COMMAND_SIZE-1,0);
        if(received<0){
            perror("Error receiving response");
            goto final;
        }

        command_buffer[received]='\0';
        printf("%s\n",command_buffer);

    }

    final:
        close(socket);
        return 0;

}
