#!/bin/zsh

set -e

make 8086

printf "INPUT\n%s\n" $1
printf "bits 16\n\n%s" $1 > ./dist/test_instruction.asm
nasm ./dist/test_instruction.asm
xxd -b ./dist/test_instruction

./dist/8086 ./dist/test_instruction --decode > ./dist/test_result.asm

printf "\nOUTPUT\n"
cat ./dist/test_result.asm
printf "\n"
nasm ./dist/test_result.asm
xxd -b ./dist/test_result

