#include <cstdio>
#include <cstring>
#include <stdint.h>
#include "../include/8086.h"

const uint8_t OP_MOV_REG = 0b100010;
const uint8_t OP_MOV_REG_IMM = 0b1011;
const uint8_t OP_MOV_MEM_IMM = 0b1100011;
const uint8_t OP_MOV_MEM_ACC = 0b101000;

const uint8_t OP_ADD_PATTERN = 0b000;
const uint8_t OP_SUB_PATTERN = 0b101;
const uint8_t OP_CMP_PATTERN = 0b111;

const uint8_t OP_MEM_REG_PATTERN = 0b000000;
const uint8_t OP_ACC_PATTERN = 0b0000010;
const uint8_t OP_IMM = 0b100000;

const uint8_t OP_ADD_MEM_REG = OP_MEM_REG_PATTERN | (OP_ADD_PATTERN << 1);
const uint8_t OP_ADD_ACC = OP_ACC_PATTERN | (OP_ADD_PATTERN << 2);
const uint8_t OP_SUB_MEM_REG = OP_MEM_REG_PATTERN | (OP_SUB_PATTERN << 1);
const uint8_t OP_SUB_ACC = OP_ACC_PATTERN | (OP_SUB_PATTERN << 2);
const uint8_t OP_CMP_MEM_REG = OP_MEM_REG_PATTERN | (OP_CMP_PATTERN << 1);
const uint8_t OP_CMP_ACC = OP_ACC_PATTERN | (OP_CMP_PATTERN << 2);

const uint8_t MOD_D0 = 0b00;
const uint8_t MOD_D8 = 0b01;
const uint8_t MOD_D16 = 0b10;
const uint8_t MOD_REG = 0b11;

const uint8_t AL = 0b000;
const uint8_t CL = 0b001;
const uint8_t DL = 0b010;
const uint8_t BL = 0b011;
const uint8_t AH = 0b100;
const uint8_t CH = 0b101;
const uint8_t DH = 0b110;
const uint8_t BH = 0b111;

const uint8_t AX = 0b000;
const uint8_t CX = 0b001;
const uint8_t DX = 0b010;
const uint8_t BX = 0b011;
const uint8_t SP = 0b100;
const uint8_t BP = 0b101;
const uint8_t SI = 0b110;
const uint8_t DI = 0b111;

const uint8_t RM_BX_SI = 0b000;
const uint8_t RM_BX_DI = 0b001;
const uint8_t RM_BP_SI = 0b010;
const uint8_t RM_BP_DI = 0b011;
const uint8_t RM_SI = 0b100;
const uint8_t RM_DI = 0b101;
const uint8_t RM_BP = 0b110;
const uint8_t RM_BX = 0b111;

struct Opts {
	uint8_t mod;
	uint8_t reg;
	uint8_t rm;
};

const char* op_pattern_str(uint8_t pattern) {
	switch(pattern) {
		case OP_ADD_PATTERN: return "add";
		case OP_SUB_PATTERN: return "sub";
		case OP_CMP_PATTERN: return "cmp";
	}

	return "";
}

const char* reg_str(uint8_t reg, bool is_wide) {
	if (is_wide) {
		switch (reg) {
			case AX: return "ax";
			case CX: return "cx";
			case DX: return "dx";
			case BX: return "bx";
			case SP: return "sp";
			case BP: return "bp";
			case SI: return "si";
			case DI: return "di";
		}
	}
	else {
		switch (reg) {
			case AL: return "al";
			case CL: return "cl";
			case DL: return "dl";
			case BL: return "bl";
			case AH: return "ah";
			case CH: return "ch";
			case DH: return "dh";
			case BH: return "bh";
		}
	}

	return "";
}

const char* rm_str(uint8_t rm) {
	switch (rm) {
		case RM_BX_SI: return "bx + si";
		case RM_BX_DI: return "bx + di";
		case RM_BP_SI: return "bp + si";
		case RM_BP_DI: return "bp + di";
		case RM_SI: return "si";
		case RM_DI: return "di";
		case RM_BP: return "bp";
		case RM_BX: return "bx";
	}

	return "";
}

Opts decode_opts(uint8_t byte) {
	Opts result;

	result.mod = byte >> 6;
	result.reg = byte >> 3 & 0b111;
	result.rm = byte & 0b111;

	return result;
}

bool decode_is_destination(uint8_t byte) {
	return (byte & 0b10) > 0;
}

bool decode_is_wide(uint8_t byte) {
	return (byte & 0b1) > 0;
}

uint16_t decode_uval(Stream* stream, bool is_wide) {
	if (is_wide) {
		uint8_t bytes[2];
		read_bytes(stream, bytes, 2);
		return (uint16_t)bytes[1] << 8 | bytes[0];
	}
	else {
		uint8_t byte;
		read_bytes(stream, &byte, 1);
		return (uint16_t)byte;
	}
}

int16_t decode_sval(Stream* stream, bool is_wide) {
	int16_t result;

	if (is_wide) {
		uint8_t bytes[2];
		read_bytes(stream, bytes, 2);
		memcpy(&result, &bytes, 2);
	}
	else {
		uint8_t byte;
		read_bytes(stream, &byte, 1);
		result = byte;
	}

	return result;
}

int16_t small_disp(uint16_t disp) {
	if (disp >= 128 && disp <= 255) {
		return (uint8_t)disp - 256;
	}
	return disp;
}

void decode_mov_mem_reg(Stream* stream) {
	uint8_t buff[2];
	read_bytes(stream, buff, 2);
	
	bool is_destination = decode_is_destination(buff[0]);
	bool is_wide = decode_is_wide(buff[0]);
	Opts opts = decode_opts(buff[1]);

	char dest[32];
	char src[32];

	if (opts.mod == MOD_REG) {
		strlcpy(dest, reg_str(opts.reg, is_wide), sizeof dest);
		strlcpy(src, reg_str(opts.rm, is_wide), sizeof src);
	}
	else if (opts.mod == MOD_D0) {
		strlcpy(dest, reg_str(opts.reg, is_wide), sizeof dest);

		if (opts.rm == RM_BP) {
			snprintf(src, sizeof src, "[%+d]", decode_uval(stream, true));
		}
		else {
			snprintf(src, sizeof src, "[%s]", rm_str(opts.rm));
		}
	}
	else if (opts.mod == MOD_D8 || opts.mod == MOD_D16) {
		int16_t val = small_disp(decode_uval(stream, opts.mod == MOD_D16));
		strlcpy(dest, reg_str(opts.reg, is_wide), sizeof dest);
		snprintf(src, sizeof src, "[%s %+d]", rm_str(opts.rm), val);
	}

	printf("mov %s, %s\n", is_destination ? dest : src, is_destination ? src : dest);
}

void decode_mov_mem_imm(Stream* stream) {
	uint8_t buff[2];
	read_bytes(stream, buff, 2);

	bool is_wide = decode_is_wide(buff[0]);
	Opts opts = decode_opts(buff[1]);

	if (opts.mod == MOD_D0) {
		uint16_t val = decode_uval(stream, is_wide);
		printf("mov [%s], %s %u\n", rm_str(opts.rm), is_wide ? "word" : "byte", val);
	}
	else if (opts.mod == MOD_D8 || opts.mod == MOD_D16) {
		int16_t disp = small_disp(decode_uval(stream, opts.mod == MOD_D16));
		uint16_t val = decode_uval(stream, is_wide);

		printf("mov [%s %+d], %s %u\n", rm_str(opts.rm), disp, is_wide ? "word" : "byte", val);
	}
}

void decode_mov_mem_acc(Stream* stream) {
	uint8_t cmd;
	read_bytes(stream, &cmd, 1);

	bool is_to_mem = (cmd & 0b10) > 0;
	bool is_wide = decode_is_wide(cmd);
	uint16_t adr = decode_uval(stream, true);

	if (is_to_mem) {
		printf("mov [%u], %s\n", adr, is_wide ? "ax" : "al");
	}
	else {
		printf("mov %s, [%u]\n", is_wide ? "ax" : "al", adr);
	}
}

void decode_mov_reg_imm(Stream* stream) {
	uint8_t opts;
	read_bytes(stream, &opts, 1);

	bool is_wide = (opts & 0b1000) > 0;
	uint8_t reg = opts & 0b111;
	uint16_t val = decode_uval(stream, is_wide);

	printf("mov %s, %u\n", reg_str(reg, is_wide), val);
}

void decode_mem_reg_op(Stream* stream) {
	uint8_t buff[2];
	read_bytes(stream, buff, 2);

	uint8_t op = (buff[0] >> 3) & 0b111;
	bool is_destination = decode_is_destination(buff[0]);
	bool is_wide = decode_is_wide(buff[0]);
	Opts opts = decode_opts(buff[1]);

	char op_str[4];
	char dest[32];
	char src[32];

	strlcpy(op_str, op_pattern_str(op), sizeof op_str);

	if (opts.mod == MOD_REG) {
		strlcpy(dest, reg_str(opts.reg, is_wide), sizeof dest);
		strlcpy(src, reg_str(opts.rm, is_wide), sizeof src);
	}
	else if (opts.mod == MOD_D0) {
		strlcpy(dest, reg_str(opts.reg, is_wide), sizeof dest);

		if (opts.rm == RM_BP) {
			snprintf(src, sizeof src, "[%+d]", decode_uval(stream, true));
		}
		else {
			snprintf(src, sizeof src, "[%s]", rm_str(opts.rm));
		}
	}
	else if (opts.mod == MOD_D8 || opts.mod == MOD_D16) {
		int16_t val = small_disp(decode_uval(stream, opts.mod == MOD_D16));
		strlcpy(dest, reg_str(opts.reg, is_wide), sizeof dest);
		snprintf(src, sizeof src, "[%s %+d]", rm_str(opts.rm), val);
	}

	printf("%s %s, %s\n", op_str, is_destination ? dest : src, is_destination ? src : dest);
}

void decode_imm_op(Stream* stream) {
	uint8_t buff[2];
	read_bytes(stream, buff, 2);

	bool is_signed = (buff[0] & 0b10) > 0;
	bool is_wide = decode_is_wide(buff[0]);
	Opts opts = decode_opts(buff[1]);
	uint8_t op = opts.reg;

	char op_str[4];
	char dest[32];
	char src[32];

	strlcpy(op_str, op_pattern_str(op), sizeof op_str);

	if (opts.mod == MOD_REG) {
		strlcpy(dest, reg_str(opts.rm, is_wide), sizeof dest);

		if (is_signed) {
			snprintf(src, sizeof src, "%+d", decode_sval(stream, false));
		}
		else {
			snprintf(src, sizeof src, "%u", decode_uval(stream, is_wide));
		}
	}
	else if (opts.mod == MOD_D0) {
		if (opts.rm == RM_BP) {
			snprintf(dest, sizeof dest, "%s [%u]", is_wide ? "word" : "byte", decode_uval(stream, is_wide));
		}
		else {
			snprintf(dest, sizeof dest, "%s [%s]", is_wide ? "word" : "byte", rm_str(opts.rm));
		}

		if (is_signed) {
			snprintf(src, sizeof src, "%+d", decode_sval(stream, false));
		}
		else {
			snprintf(src, sizeof src, "%u", decode_uval(stream, is_wide));
		}
	}
	else {
		int16_t disp = small_disp(decode_uval(stream, opts.mod == MOD_D16));
		snprintf(dest, sizeof dest, "%s [%s %+d]", is_wide ? "word" : "byte", rm_str(opts.rm), disp);

		if (is_signed) {
			snprintf(src, sizeof src, "%+d", decode_sval(stream, false));
		}
		else {
			snprintf(src, sizeof src, "%u", decode_uval(stream, is_wide));
		}
	}

	printf("%s %s, %s\n", op_str, dest, src);
}

void decode_acc_op(Stream* stream) {
	uint8_t cmd;
	read_bytes(stream, &cmd, 1);

	uint8_t op = (cmd >> 3) & 0b111;
	bool is_wide = decode_is_wide(cmd);
	uint16_t val = decode_uval(stream, is_wide);

	printf("%s %s, %u\n", op_pattern_str(op), is_wide ? "ax" : "al", val);
}

int decode_next(Stream* stream) {
	uint8_t opcode;
	peek_bytes(stream, &opcode, 1);

	if (opcode >> 2 == OP_MOV_REG) {
		decode_mov_mem_reg(stream);
	}
	else if (opcode >> 4 == OP_MOV_REG_IMM) {
		decode_mov_reg_imm(stream);
	}
	else if (opcode >> 1 == OP_MOV_MEM_IMM) {
		decode_mov_mem_imm(stream);
	}
	else if (opcode >> 2 == OP_MOV_MEM_ACC) {
		decode_mov_mem_acc(stream);
	}
	else if (opcode >> 2 == OP_IMM) {
		decode_imm_op(stream);
	}
	else if (
		opcode >> 2 == OP_ADD_MEM_REG
		|| opcode >> 2 == OP_SUB_MEM_REG
		|| opcode >> 2 == OP_CMP_MEM_REG
	) {
		decode_mem_reg_op(stream);
	}
	else if (
		opcode >> 1 == OP_ADD_ACC
		|| opcode >> 1 == OP_SUB_ACC
		|| opcode >> 1 == OP_CMP_ACC
	) {
		decode_acc_op(stream);
	}
	else {
		return 1;
	}

	return 0;
}

int main(int argc, char* argv[]) {
	Stream stream = open(argv[1]);

	if (stream.eof) {
		printf("Failed to open input file");
		return 1;
	}

	printf("bits 16\n\n");

	while(stream.eof == false) {
		int result = decode_next(&stream);

		if (result != 0) {
			uint8_t op;
			read_bytes(&stream, &op, 1);
			fprintf(stderr, "Failed to decode instruction: %x\n", op);
			return 1;
		}
	};

	close(&stream);

	return 0;
}

