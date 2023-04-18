# README

## Introduction

This project is a toy compiler written in C. It was an attempt to learn AI-assisted programming, such as Github Copilot, and to find out how AI is useful in programming. The compiler is designed to compile a simple C-like programming language into GNU assembly code that runs on the x86-64 arch.

## Usage

**Download**

```bash
$ git clone git@github.com:liuy/tc.git
or
$ git clone https://github.com/liuy/tc.git
```

**Compile**

```bash
$ make # This will make a compiler named 'tc'
or
$ make debug=1 # if you want to see assembly output while using 'tc'
```

**Run**

```bash
# if you make tc with 'make'
$ ./tc -s 'int main(){printf("hello world!\n");}' # use tc to compile code in quotes
$ ./a.tc # a.tc is produced by above command and run it
hello world!
# if you make tc with 'make debug=1'
$ ./tc -s 'int main(){printf("hello world!\n");}'
[INFO] The source code:
int main(){printf("hello world!\n");}
[INFO] The assembly code:
        .section        .rodata
.LC0:
        .string "hello world!\n"

        .globl main
        .text
        .type main, @function
main:
        endbr64
        pushq %rbp
        movq %rsp, %rbp
        leaq .LC0(%rip), %rax
        movq %rax, %rdi
        movl $0, %eax
        call printf
        leave
        ret
$ ./a.tc
hello world!
```

```bash
# some more funny c files.
$ make run file=
Please specify file in the plane.c prime.c snake.c fibo.c pi.c twin.c gold.c. E.g, make run file=prime.c
$ make run file=fibo.c # compile and run a file in test/fibo.c that print 42 Fibonacci numbers
...
Program Output:
// Print 42 Fibonacci numbers
1 2 3 5 8 13 21 34 55 89 144 233 377 610 987 1597 2584 4181 6765 10946 17711 28657 46368 75025 121393 196418 317811 514229 832040 1346269 2178309 3524578 5702887 9227465 14930352 24157817 39088169 63245986 102334155 165580141 267914296 433494437

real    0m6.034s
user    0m6.034s
sys     0m0.000s
# I even wrote 2 simple console games that tc can compile, have fun!
$ make run file=plane.c # this will drop you into a plane game in the terminal!
$ make run file=snake.c # The famous snake game, have FUN!
```

To keep it simple, we only support two options and a single file path

```bash
Usage: ./tc [-s source_code] [-l linker arg] [input_file]
-s option: as above suggested, accept a code stream in quotes
-l option: is to pass the linker argument to gcc linker 'ld', by which we can call external functions in the shared library like glibc and others, e.g, ncurses that our two games need to do the console io.
input_file: path to the file to be compiled.
```

## Language Syntax

The language supported by this compiler is a simple C-like language with the following syntax:

### Data Types

- int: 32-bit integer
- int array
- global variables
- local variables

### Operators

- \+ addition
- \- subtraction
- \* multiplication
- \/ division
- % modulus
- = assignment
- == equality
- != inequality
- < less than
- \> greater than
- <= less than or equal to
- = greater than or equal to
- || and && logic operator

### statement

- return statement
- assign statement
- conpound statement
- function definition with at most 6 arguments

### Control Flow

- **if-else statement:** if (expression) { statement } else { statement }
- **while loop**: while (expression) { statement }Functions
- **call expression** with extern function call support

## Limitations

1. no function type checking yet, that's why we don't need headers to call functions outside
...
