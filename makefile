CFLAGS = -Wall -Wextra -std=gnu99 -DTC_DEBUG

all: tc

tc: tc.h list.h main.c lex.c
	gcc $(CFLAGS) -o tc main.c lex.c

clean:
	rm -f tc lex.o
