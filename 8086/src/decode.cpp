#include <cstdio>
#include <cstring>
#include <stdint.h>
#include "../include/8086.h"

const uint8_t OP_MOV_REG = 0b100010;
const uint8_t OP_MOV_REG_IMM = 0b1011;
const uint8_t OP_MOV_MEM_IMM = 0b1100011;
const uint8_t OP_MOV_MEM_AGG = 0b101000;

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

uint16_t decode_val(Stream* stream, bool is_wide) {
	if (is_wide) {
		uint8_t bytes[2];
		read_bytes(stream, bytes, 2);
		return ((uint16_t)bytes[1] << 8) | bytes[0];
	}
	else {
		uint8_t byte;
		read_bytes(stream, &byte, 1);
		return (uint16_t)byte;
	}
}

int16_t small_disp(uint16_t disp) {
	if (disp >= 128 && disp <= 255) {
		return (uint8_t)disp - 256;
	}
	return disp;
}

void decode_mov_reg(Stream* stream) {
	uint8_t buff[2];
	size_t read = read_bytes(stream, buff, 2);
	
	bool is_destination = (buff[0] & 0b10) > 0;
	bool is_wide = (buff[0] & 0b1) > 0;

	uint8_t mod = buff[1] >> 6;
	uint8_t reg = buff[1] >> 3 & 0b111;
	uint8_t rm = buff[1] & 0b111;

	if (mod == MOD_REG) {
		if (is_destination) {
			printf("mov %s, %s\n", reg_str(reg, is_wide), reg_str(rm, is_wide));
		}
		else {
			printf("mov %s, %s\n", reg_str(rm, is_wide), reg_str(reg, is_wide));
		}
	}
	else if (mod == MOD_D0) {
		if (rm == RM_BP) {
			if (is_destination) {
				printf("mov %s, [%u]\n", reg_str(reg, is_wide), decode_val(stream, true));
			}
			else {
				printf("mov [%u], %s\n", decode_val(stream, true), reg_str(reg, is_wide));
			}
		}
		else {
			if (is_destination) {
				printf("mov %s, [%s]\n", reg_str(reg, is_wide), rm_str(rm));
			}
			else {
				printf("mov [%s], %s\n", rm_str(rm), reg_str(reg, is_wide));
			}
		}
	}
	else if (mod == MOD_D8 || mod == MOD_D16) {
		int16_t val = small_disp(decode_val(stream, mod == MOD_D16));
		if (is_destination) {
			printf("mov %s, [%s %+d]\n", reg_str(reg, is_wide), rm_str(rm), val);
		}
		else {
			printf("mov [%s %+d], %s\n", rm_str(rm), val, reg_str(reg, is_wide));
		}
	}
}

void decode_mov_mem_imm(Stream* stream) {
	uint8_t buff[2];
	read_bytes(stream, buff, 2);

	bool is_wide = (buff[0] & 0b1) > 0;
	uint8_t mod = buff[1] >> 6;
	uint8_t rm = buff[1] & 0b111;

	if (mod == MOD_D0) {
		uint16_t val = decode_val(stream, is_wide);
		printf("mov [%s], %s %u\n", rm_str(rm), is_wide ? "word" : "byte", val);
	}
	else if (mod == MOD_D8 || mod == MOD_D16) {
		int16_t disp = small_disp(decode_val(stream, mod == MOD_D16));
		uint16_t val = decode_val(stream, is_wide);

		printf("mov [%s %+d], %s %u\n", rm_str(rm), disp, is_wide ? "word" : "byte", val);
	}
}

void decode_mov_mem_agg(Stream* stream) {
	uint8_t cmd;
	read_bytes(stream, &cmd, 1);

	bool is_to_mem = (cmd & 0b10) > 0;
	bool is_wide = (cmd & 0b1) > 0;
	uint16_t adr = decode_val(stream, true);

	if (is_to_mem) {
		printf("mov [%u], %s\n", adr, is_wide ? "ax" : "al");
	}
	else {
		printf("mov %s, [%u]\n", is_wide ? "ax" : "al", adr);
	}
}

int decode_mov_reg_imm(Stream* stream) {
	uint8_t opts;
	read_bytes(stream, &opts, 1);

	bool is_wide = (opts & 0b1000) > 0;
	uint8_t reg = opts & 0b111;
	uint16_t val = decode_val(stream, is_wide);

	printf("mov %s, %u\n", reg_str(reg, is_wide), val);

	return 0;
}

int decode_next(Stream* stream) {
	uint8_t opcode;
	peek_bytes(stream, &opcode, 1);

	if (opcode >> 2 == OP_MOV_REG) {
		decode_mov_reg(stream);
	}
	else if (opcode >> 4 == OP_MOV_REG_IMM) {
		decode_mov_reg_imm(stream);
	}
	else if (opcode >> 1 == OP_MOV_MEM_IMM) {
		decode_mov_mem_imm(stream);
	}
	else if (opcode >> 2 == OP_MOV_MEM_AGG) {
		decode_mov_mem_agg(stream);
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
			printf("Failed to decode instruction: %x\n", op);
			return 1;
		}
	};

	close(&stream);

	return 0;
}

