#!/bin/zsh

set -e

make 8086
./dist/8086 $1 --decode > ./dist/result.asm
nasm ./dist/result.asm
cmp -l $1 ./dist/result
