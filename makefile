CFLAGS = -Wall -Wextra -std=gnu99 -DTC_DEBUG=0 -g
CHECK_FLAGS = -lcheck -lm -pthread -lsubunit -lrt

all: tc

tc: tc.h list.h main.c lex.c parser.c
	gcc $(CFLAGS) -o tc main.c lex.c parser.c

test_lex: test/test_lex.c lex.c
	gcc -o test/test_lex test/test_lex.c lex.c $(CHECK_FLAGS)

check: test_lex
	test/test_lex

clean:
	rm -f tc test/test_lex
