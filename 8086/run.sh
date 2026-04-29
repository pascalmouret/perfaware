clang ./src/decode.cpp ./src/stream.cpp -o decode && \
./decode $1 > result.asm && \
nasm result.asm && \
cmp -l $1 result