CFLAGS = -Wall -Wextra -g -O2 -pedantic -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict

EXE = isos_inject

all: isos_inject

isos_inject: isos_inject.o
	$(CC) $(CFLAGS) -o $@ $^ -lbfd;

isos_inject.o: isos_inject.c argp_parser.c
	$(CC) $(CFLAGS) -c $^;

clean:
	rm -f *.o $(EXE) 

.PHONY: all clean