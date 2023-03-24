CFLAGS = -Wall -Wextra -std=c99

all: tc

tc: tc.h list.h main.c lex.c
	gcc $(CFLAGS) -o tc main.c lex.c

clean:
	rm -f tc lex.o
