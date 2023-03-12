CFLAGS = -Wall -Wextra -pedantic -O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict

EXE = isos_inject isos_inject-1 isos_inject-2

all: isos_inject

isos_inject: isos_inject.o
	$(CC) $(CFLAGS) -o $@ $^ -lbfd;

isos_inject.o: isos_inject.c argp_parser.c
	$(CC) $(CFLAGS) -c $^;

check_warnings:
	make clean;
	clang -fsyntax-only -Wall -Wextra -Wuninitialized -Wpointer-arith -Wcast-qual -Wcast-align isos_inject.c argp_parser.c;
	gcc -O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict -o isos_inject isos_inject.c -lbfd;
	gcc -fanalyzer isos_inject.c -lbfd;
	clang -g -fsanitize=address,undefined -fno-omit-frame-pointer -o isos_inject-1 isos_inject.c -lbfd;
	make check_isos_inject-1;
	clang -g -fsanitize=memory -fno-omit-frame-pointer -o isos_inject-2 isos_inject.c -lbfd;
	make check_isos_inject-2;

check_isos_inject-1:
	ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ./isos_inject-1 --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='123'
	ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ./isos_inject-1 --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='123' --modify-entry-function 
			
check_isos_inject-2:
	MSAN_OPTIONS=halt_on_error=1 ./isos_inject-2 --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='123'
	MSAN_OPTIONS=halt_on_error=1 ./isos_inject-2 --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='123' --modify-entry-function 
		

clean:
	rm -f *.o *.out $(EXE) 

.PHONY: all clean