SHELL := /bin/bash

ifndef debug
	debug := 2
endif

CFLAGS = -Wall -std=gnu99 -g -DTC_DEBUG=$(debug)
CHECK_FLAGS = -lcheck -lm -pthread -lsubunit -lrt
LIST := $(patsubst test/%,%,$(shell find test -name '*.c' -not -name 'test_main.c'))

all: tc

tc: tc.h list.h main.c lex.c parser.c analyzer.c generator.c optimizer.c
	gcc $(CFLAGS) -o tc main.c lex.c parser.c analyzer.c generator.c optimizer.c

test_tc: test/test_main.c lex.c parser.c
	gcc -o test/test_tc test/test_main.c lex.c parser.c $(CHECK_FLAGS)

check: test_tc
	test/test_tc

clean:
	rm -f tc test/test_tc a.tc

run:
	@if [ -z "$(file)" ]; then \
		echo "Please specify file in the $(LIST). E.g, make run file=prime.c"; \
	else \
		make clean;make debug=1;./tc test/$(file); \
		echo -e "Program Output:";head -n1 test/$(file);time ./a.tc; \
	fi
