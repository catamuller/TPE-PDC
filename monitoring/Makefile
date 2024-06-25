CFLAGS:= -std=c11 -pedantic -lpthread -pedantic-errors -g -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-discarded-qualifiers -Wno-unused-const-variable -D_POSIX_C_SOURCE=200112L
METRICS_CLI:= metrics
METRICS_OBJS:= lib/display.o lib/connections_server_handler.o lib/args.o main.o

.PHONY: all clean

all: $(METRICS_CLI)

connections_server_handler.o: lib/headers/connections_server_handler.h

display.o: lib/headers/display.h

args.o: lib/headers/args.h

$(METRICS_CLI): $(METRICS_OBJS)
	$(CC) $(CFLAGS) $(METRICS_OBJS) -o $(METRICS_CLI)

clean:
	- rm -rf $(METRICS_OBJS) $(METRICS_CLI)
