CFLAGS:= -std=c11 -pedantic -lpthread -pedantic-errors -g -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-discarded-qualifiers -Wno-unused-const-variable -D_POSIX_C_SOURCE=200112L
CLIENT_CONFIG_CLI:= client_config
CLIENT_CONFIG_OBJ:= args.o client_server_connections.o client_config.o

.PHONY:all clean

all: $(CLIENT_CONFIG_CLI)

args.o: headers/args.h

client_server_connections.o: headers/client_server_connections.h


$(CLIENT_CONFIG_CLI): $(CLIENT_CONFIG_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_CONFIG_OBJ) -o $(CLIENT_CONFIG_CLI)

clean:
	- rm -rf $(CLIENT_CONFIG_OBJ) $(CLIENT_CONFIG_CLI)
