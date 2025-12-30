#include <cstdio>
#include <cstring>
#include <stdint.h>

const size_t BUFF_SIZE = 1024;

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

uint16_t decode_val(uint8_t buff[], bool is_wide) {
	if (is_wide) {
		return ((uint16_t)buff[1] << 8) | buff[0];
	}
	else {
		return (uint16_t)buff[0];
	}
}

int16_t small_disp(uint16_t disp) {
	if (disp >= 128 && disp <= 255) {
		return (uint8_t)disp - 256;
	}
	return disp;
}

int decode_mov_reg(uint8_t buff[]) {
	bool is_destination = (buff[0] & 0b10) > 0;
	bool is_wide = (buff[0] & 0b1) > 0;
	uint8_t mod = buff[1] >> 6;
	uint8_t reg = buff[1] >> 3 & 0b111;
	uint8_t rm = buff[1] & 0b111;

	const char* dest;
	const char* src;

	if (mod == MOD_REG) {
		if (is_destination) {
			printf("mov %s, %s\n", reg_str(reg, is_wide), reg_str(rm, is_wide));
		}
		else {
			printf("mov %s, %s\n", reg_str(rm, is_wide), reg_str(reg, is_wide));
		}
		return 2;
	}
	else if (mod == MOD_D0) {
		if (rm == RM_BP) {
			if (is_destination) {
				printf("mov %s, [%u]\n", reg_str(reg, is_wide), decode_val(&buff[2], true));
			}
			else {
				printf("mov [%u], %s\n", decode_val(&buff[2], true), reg_str(reg, is_wide));
			}
			return is_wide ? 4 : 3;
		}
		else {
			if (is_destination) {
				printf("mov %s, [%s]\n", reg_str(reg, is_wide), rm_str(rm));
			}
			else {
				printf("mov [%s], %s\n", rm_str(rm), reg_str(reg, is_wide));
			}
			return 2;
		}
	}
	else if (mod == MOD_D8 || mod == MOD_D16) {
		int16_t val = small_disp(decode_val(&buff[2], mod == MOD_D16));
		if (is_destination) {
			printf("mov %s, [%s %+d]\n", reg_str(reg, is_wide), rm_str(rm), val);
		}
		else {
			printf("mov [%s %+d], %s\n", rm_str(rm), val, reg_str(reg, is_wide));
		}
		return mod == MOD_D16 ? 4 : 3;	
	}

	return 0;
}

int decode_mov_mem_imm(uint8_t buff[]) {
	bool is_wide = (buff[0] & 0b1) > 0;
	uint8_t mod = buff[1] >> 6;
	uint8_t rm = buff[1] & 0b111;
	// mod can be used as length of displacement
	uint16_t val = decode_val(&buff[mod + 2], is_wide);

	if (mod == MOD_D0) {
		printf("mov [%s], %s %u\n", rm_str(rm), is_wide ? "word" : "byte", val);
		return 3 + is_wide;
	}
	else if (mod == MOD_D8 || mod == MOD_D16) {
		int16_t disp = small_disp(decode_val(&buff[2], mod == MOD_D16));
		printf("mov [%s %+d], %s %u\n", rm_str(rm), disp, is_wide ? "word" : "byte", val);
		return 3 + mod + is_wide;
	}

	return 0;
}

int decode_mov_mem_agg(uint8_t buff[]) {
	bool is_to_mem = (buff[0] & 0b10) > 0;
	bool is_wide = (buff[0] & 0b1) > 0;
	uint16_t adr = decode_val(&buff[1], true);

	if (is_to_mem) {
		printf("mov [%u], %s\n", adr, is_wide ? "ax" : "al");
	}
	else {
		printf("mov %s, [%u]\n", is_wide ? "ax" : "al", adr);
	}

	return 3;
}

int decode_mov_reg_imm(uint8_t buff[]) {
	bool is_wide = (buff[0] & 0b1000) > 0;
	uint8_t reg = buff[0] & 0b111;
	uint16_t val = decode_val(&buff[1], is_wide);

	printf("mov %s, %u\n", reg_str(reg, is_wide), val);

	return is_wide ? 3 : 2;
}

int decode_next(uint8_t buff[]) {
	if (buff[0] >> 2 == OP_MOV_REG) {
		return decode_mov_reg(buff);
	}
	else if (buff[0] >> 4 == OP_MOV_REG_IMM) {
		return decode_mov_reg_imm(buff);
	}
	else if (buff[0] >> 1 == OP_MOV_MEM_IMM) {
		return decode_mov_mem_imm(buff);
	}
	else if (buff[0] >> 2 == OP_MOV_MEM_AGG) {
		return decode_mov_mem_agg(buff);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	FILE* in_file = fopen(argv[1], "r");

	if (in_file == 0) {
		printf("Failed to open input file");
		return 1;
	}

	printf("bits 16\n\n");

	uint8_t buff[BUFF_SIZE];
	size_t read_bytes = 0;
	while(true) {
		read_bytes = fread(buff, 1, BUFF_SIZE, in_file);

		size_t result = 0;
		int index = 0;
		while (index < read_bytes) {
			result = decode_next(&buff[index]);
			if (result == 0) {
				printf("Failed to decode instruction %x", buff[index]);
				return 1;
			}
			index += result;

		}

		if (read_bytes < BUFF_SIZE) {
			break;
		}
	}

	fclose(in_file);

	return 0;
}

