#include <cstdio>

#include "decode.h"
#include "instruction.h"
#include "memory.h"

const char* get_memonic(Opcode opcode) {
    switch (opcode) {
        case OP_MOV: return "mov";
        case OP_ADD: return "add";
        case OP_SUB: return "sub";
        case OP_CMP: return "cmp";
        case OP_JE: return "je";
        case OP_JL: return "jl";
        case OP_JLE: return "jle";
        case OP_JB: return "jb";
        case OP_JBE: return "jbe";
        case OP_JP: return "jp";
        case OP_JO: return "jo";
        case OP_JS: return "js";
        case OP_JNE: return "jne";
        case OP_JNL: return "jnl";
        case OP_JNLE: return "jnle";
        case OP_JNB: return "jnb";
        case OP_JNBE: return "jnbe";
        case OP_JNP: return "jnp";
        case OP_JNO: return "jno";
        case OP_JNS: return "jns";
        case OP_LOOP: return "loop";
        case OP_LOOPZ: return "loopz";
        case OP_LOOPNZ: return "loopnz";
        case OP_JCXZ: return "jcxz";
    }
}

const char* get_reg(Register reg) {
    switch (reg) {
        case REG_AX: return "ax";
        case REG_BX: return "bx";
        case REG_CX: return "cx";
        case REG_DX: return "dx";
        case REG_AL: return "al";
        case REG_AH: return "ah";
        case REG_BL: return "bl";
        case REG_BH: return "bh";
        case REG_CL: return "cl";
        case REG_CH: return "ch";
        case REG_DL: return "dl";
        case REG_DH: return "dh";
        case REG_SP: return "sp";
        case REG_BP: return "bp";
        case REG_SI: return "si";
        case REG_DI: return "di";
        case REG_NONE: return "";
    }
}

void print_operand(Operand* operand, bool is_sized, bool is_wide, bool is_rel_jmp) {
    if (is_sized && !is_rel_jmp) {
        if (is_wide) {
            printf("word ");
        }
        else {
            printf("byte ");
        }
    }

    switch (operand->type) {
        case OP_T_REG:
            printf("%s", get_reg(operand->reg));
            break;
        case OP_T_IMMEDIATE:
            if (is_rel_jmp) {
                printf("$");
            }

            if (operand->immediate.is_signed) {
                printf("%+d", operand->immediate.value);
            }
            else {
                printf("%u", operand->immediate.value);
            }
            break;
        case OP_T_ADDRESS:
            printf("[");
            if (operand->effectiveAddress.regs[0] != REG_NONE) {
                printf("%s", get_reg(operand->effectiveAddress.regs[0]));
                if (operand->effectiveAddress.regs[1] != REG_NONE) {
                    printf(" + %s", get_reg(operand->effectiveAddress.regs[1]));
                }
            }
            printf(" %+d]", operand->effectiveAddress.displacement);
            break;
        case OP_T_NONE:
            printf("");
    }
}

void print_instruction(Instruction* instruction) {
    printf("%s", get_memonic(instruction->opcode));

    bool is_sized = instruction->destination.type != OP_T_REG;
    bool is_wide = instruction->flags.is_wide;

    if (instruction->destination.type != OP_T_NONE) {
        printf(" ");
        print_operand(&instruction->destination, is_sized, is_wide, instruction->flags.is_rel_jmp);
    }

    if (instruction->source.type != OP_T_NONE) {
        printf(", ");
        print_operand(&instruction->source, is_sized, is_wide, instruction->flags.is_rel_jmp);
    }

    printf("\n");
}

void debug_operand(Operand* operand) {
    switch (operand->type) {
        case OP_T_REG:
            printf("REG(%s)", get_reg(operand->reg));
            break;
        case OP_T_IMMEDIATE:
            printf("IMM(%+d)", operand->immediate.value);
            break;
        case OP_T_NONE:
            printf("NONE");
            break;
        case OP_T_ADDRESS:
            printf(
                "ADDR(%s, %s, %+d)",
                get_reg(operand->effectiveAddress.regs[0]),
                get_reg(operand->effectiveAddress.regs[1]),
                operand->effectiveAddress.displacement
            );
            break;
    }
}

void debug_instruction(Instruction* instruction) {
    printf("; ");
    printf("%u:%u %s", instruction->address, instruction->address + instruction->size - 1, get_memonic(instruction->opcode));
    printf(" ");
    debug_operand(&instruction->destination);
    printf(", ");
    debug_operand(&instruction->source);
    printf("\n");
}

void print_instructions(MemAccess access, size_t length) {
    printf("bits 16\n\n");

    int initial_offset = access.offset;

    while (access.offset < initial_offset + length) {
        Instruction instruction = decode(access);

        if (instruction.size == 0) {
            fprintf(stderr, "ERROR: Failed to decode instruction at %u\n", absolute_address(access.segment, access.offset));
            exit(1);
        }

        debug_instruction(&instruction);
        print_instruction(&instruction);

        access.offset += instruction.size;
    }
}
