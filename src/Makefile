CFLAGS:= -std=c11 -pedantic -pedantic-errors -g -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L -fsanitize=address
SMTPD_CLI:= smtpd
SMTPD_OBJS:= lib/args.o main.o

.PHONY: all clean

all: $(SMTPD_CLI)

args.o: args.h
	
$(EXEC): $(SMTPD_OBJS)
	$(CC) $(CFLAGS) $(SMTPD_OBJS) -o $(SMTPD_CLI)

clean:
	- rm -rf $(SMTPD_CLI) $(SMTPD_OBJS)