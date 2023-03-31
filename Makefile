CFLAGS = -Wall -Wextra -pedantic -O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict

EXE = isos_inject inject_code isos_inject-1 isos_inject-2

all: isos_inject inject_code

isos_inject: isos_inject.o
	$(CC) $(CFLAGS) -o $@ $^ -lbfd;

isos_inject.o: isos_inject.c argp_parser.c
	$(CC) $(CFLAGS) -c $^;

inject_code: inject_code.asm
	nasm -o $@ $^

check_warnings:
	make clean;
	make inject_code;
	clang -fsyntax-only -Wall -Wextra -Wuninitialized -Wpointer-arith -Wcast-qual -Wcast-align isos_inject.c argp_parser.c;
	gcc -O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict -o binaries/isos_inject-1 isos_inject.c -lbfd;
	gcc -fanalyzer isos_inject.c -lbfd -o binaries/isos_inject-2;
	clang -g -fsanitize=address,undefined -fno-omit-frame-pointer -o binaries/isos_inject-3 isos_inject.c -lbfd;
	clang -g -fsanitize=memory -fno-omit-frame-pointer -o binaries/isos_inject-4 isos_inject.c -lbfd;

test:
	make check_warnings;	
	./script.sh

clean:
	rm -f *.o *.out *.txt $(EXE) binaries/*

.PHONY: all clean