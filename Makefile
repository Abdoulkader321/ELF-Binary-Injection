CFLAGS = -Wall -Wextra -pedantic -O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict

EXE = isos_inject

all: isos_inject

isos_inject: isos_inject.o
	$(CC) $(CFLAGS) -o $@ $^ -lbfd;

isos_inject.o: isos_inject.c argp_parser.c
	$(CC) $(CFLAGS) -c $^;

check_warnings:
	clang -fsyntax-only -Wall -Wextra -Wuninitialized -Wpointer-arith -Wcast-qual -Wcast-align isos_inject.c argp_parser.c;
	gcc -fanalyzer isos_inject.c -lbfd;
	gcc -O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict -o isos_inject isos_inject.c -lbfd;
	

clean:
	rm -f *.o *.out $(EXE) 

.PHONY: all clean