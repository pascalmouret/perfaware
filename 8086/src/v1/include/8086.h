#include <cstddef>
#include <cstdint>
#include <cstdio>

const size_t BUFF_SIZE = 1024;

struct Stream {
	FILE* file;
	char buff[BUFF_SIZE]; 
	size_t index;
	size_t buff_length;
	bool eof;
};

Stream open(const char path[]);
size_t read_bytes(Stream* stream, uint8_t buff[], size_t length);
size_t peek_bytes(Stream* stream, uint8_t buff[], size_t length);
void close(Stream* stream);
