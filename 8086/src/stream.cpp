#include <cstdint>
#include <cstdio>
#include <cstring>
#include "../include/8086.h"

Stream open(const char path[]) {
	Stream result;
	
	result.index = 0;
	result.buff_length = 0;
	result.file = fopen(path, "r");
	result.eof = false;

	if (result.file == 0) {
		result.eof = true;
		return result;
	}

	return result;
}

size_t read_bytes(Stream* stream, uint8_t buff[], size_t length) {
	if (stream->eof) {
		return 0;
	}

	if (stream->index + length >= stream->buff_length) {
		int av_bytes = stream->buff_length - stream->index;
		int remainder = length - av_bytes;

		memcpy(buff, &stream->buff[stream->index], av_bytes);

		stream->buff_length = fread(stream->buff, 1, BUFF_SIZE, stream->file);
		stream->index = 0;

		if (stream->buff_length == 0) {
			stream->eof = true;
			return av_bytes;
		}

		size_t read = read_bytes(stream, &buff[av_bytes], remainder);

		return av_bytes + read;
	}
	else {
		memcpy(buff, &stream->buff[stream->index], length);
		stream->index += length;
		return length;
	};
}

size_t peek_bytes(Stream* stream, uint8_t buff[], size_t length) {
	if (stream->eof) {
		return 0;
	}

	if (stream->index + length >= stream->buff_length) {
		int av_bytes = stream->buff_length - stream->index;
		int remainder = length - av_bytes;

		memcpy(buff, &stream->buff[stream->index], av_bytes);

		long pos = ftell(stream->file);
		size_t read = fread(&buff[av_bytes], 1, remainder, stream->file);
		fseek(stream->file, pos, SEEK_SET);

		return av_bytes + read;
	}
	else {
		memcpy(buff, &stream->buff[stream->index], length);
		return length;
	};
}

void close(Stream* stream) {
	fclose(stream->file);
}
