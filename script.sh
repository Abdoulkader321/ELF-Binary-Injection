#!/bin/sh

BINARY_DIR=binaries
RED='\033[0;41m'  #0;31m for text
GREEN='\033[0;42m'  #0;32 for text
NC='\033[0m' # No Color

EXIT_SUCCESS=0
EXIT_FAILURE=1

test_exit_success(){
	echo -n "Result : "
	if [ $1 -eq $EXIT_SUCCESS ]; then
    		echo "${GREEN}*Passed* ${NC}\n"
	else
		echo "${NC}***Failed!***    ${NC}\n"
		cat error.txt
	fi
}

test_exit_failure(){
	echo -n "Result : "
	if [ $1 -eq $EXIT_FAILURE ]; then
    		echo "${GREEN}*Passed* ${NC}\n"
	else
		echo "${NC}***Failed!***    ${NC}\n"
		cat success.txt
	fi
}

for exec in ${BINARY_DIR}/*; 
do
    echo "========== $(basename ${exec}) ==========="

    echo "--- Test1 ---"
    ${exec} --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='1234' --modify-entry-function 1> success.txt 2> error.txt
    test_exit_success $?

    echo "--- Test2 ---"
    ${exec} --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='1234' 1> success.txt 2> error.txt
    test_exit_success $?

    echo "--- Test3 ---"
    ${exec} --path-to-elf=./doNotExist --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='1234' 1> success.txt 2> error.txt
    test_exit_failure $?

    echo "--- Test4 ---"
    ${exec} --path-to-elf=./date --path-to-code=./binary_to_inject --base-address='1234' 1> success.txt 2> error.txt
    test_exit_failure $?

    echo "--- Test5 ---"
    ${exec} --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='12A34' 1> success.txt 2> error.txt
    test_exit_failure $?

    echo "--- Test6 ---"
    ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ${exec} --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='1234' 1> success.txt 2> error.txt
    test_exit_success $?

    echo "--- Test7 ---"
    MSAN_OPTIONS=halt_on_error=1 ${exec} --path-to-elf=./date --path-to-code=./binary_to_inject --new-section-name='abcd' --base-address='1234' 1> success.txt 2> error.txt
    test_exit_success $?	

done
