CFLAGS:= -std=c11 -pedantic -lpthread -pedantic-errors -g -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-discarded-qualifiers -Wno-unused-const-variable -D_POSIX_C_SOURCE=200112L 
SMTPD_CLI:= smtpd
SMTPD_OBJS:= lib/args.o lib/selector.o lib/smtp.o lib/stm.o lib/utils.o lib/buffer.o lib/parser.o lib/parser_utils.o lib/smtp_parsing.o lib/logger.o main.o

.PHONY: all clean

all: $(SMTPD_CLI)

stm.o: lib/headers/stm.h

buffer.o: lib/headers/buffer.h

utils.o: lib/headers/utils.h

args.o: lib/headers/args.h

selector.o: lib/headers/selector.h

smtp.o: lib/headers/smtp.h 

parser.o: lib/headers/parser.h

parser_utils.o: lib/headers/parser_utils.h

smtp_parsing.o: lib/headers/smtp_parsing.h

logger.o: lib/headers/logger.h

$(SMTPD_CLI): $(SMTPD_OBJS)
	$(CC) $(CFLAGS) $(SMTPD_OBJS) -o $(SMTPD_CLI)

request_test: request_test.o request.o buffer.o
	$(CC) $(CFLAGS) -lcheck -o $@ $^

test: request_test
	./request_test

clean:
	- rm -rf $(SMTPD_CLI) $(SMTPD_OBJS)
