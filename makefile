ifndef debug
	debug := 2
endif

CFLAGS = -Wall -Wextra -std=gnu99 -g -DTC_DEBUG=$(debug)
CHECK_FLAGS = -lcheck -lm -pthread -lsubunit -lrt

all: tc

tc: tc.h list.h main.c lex.c parser.c analyzer.c
	gcc $(CFLAGS) -o tc main.c lex.c parser.c analyzer.c

test_lex: test/test_lex.c lex.c parser.c
	gcc -o test/test_lex test/test_lex.c lex.c parser.c $(CHECK_FLAGS)

check: test_lex
	test/test_lex

clean:
	rm -f tc test/test_lex
