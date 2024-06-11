CFLAGS:= -std=c11 -pedantic -pedantic-errors -g -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-const-variable -D_POSIX_C_SOURCE=200112L -fsanitize=address
SMTPD_CLI:= smtpd
SMTPD_OBJS:= lib/args.o lib/selector.o lib/smtp.o lib/stm.o lib/utils.o lib/buffer.o main.o 

.PHONY: all clean

all: $(SMTPD_CLI)

stm.o: lib/headers/stm.h

buffer.o: lib/headers/buffer.h

utils.o: lib/headers/utils.h

args.o: lib/headers/args.h

selector.o: lib/headers/selector.h

smtp.o: lib/headers/smtp.h

$(SMTPD_CLI): $(SMTPD_OBJS)
	$(CC) $(CFLAGS) $(SMTPD_OBJS) -o $(SMTPD_CLI)

clean:
	- rm -rf $(SMTPD_CLI) $(SMTPD_OBJS)
