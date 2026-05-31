#pragma once

#include "../sanity.h"

enum Opcode {
    OP_MOV,
    OP_ADD,
};

enum Register {
    REG_NONE,
    REG_AX,
    REG_AL,
    REG_AH,
    REG_BX,
    REG_BL,
    REG_BH,
    REG_CX,
    REG_CL,
    REG_CH,
    REG_DX,
    REG_DL,
    REG_DH,
    REG_SP,
    REG_BP,
    REG_SI,
    REG_DI,
};

typedef struct {
    Register reg;
} RegisterAccess;

typedef struct {
    s32 value;
    bool is_signed;
    bool is_wide;
} Immediate;

typedef struct {
    Register regs[2];
    s32 displacement;
} EffectiveAddress;

typedef struct {
    bool is_wide;
    bool is_signed;
    bool is_dest;
} Flags;

enum OperandType {
    OP_T_NONE,
    OP_T_REG,
    OP_T_IMMEDIATE,
    OP_T_ADDRESS,
};

typedef struct  {
    OperandType type;
    union {
        RegisterAccess reg;
        Immediate immediate;
        EffectiveAddress effectiveAddress;
    };
} Operand;

typedef struct {
    u32 address;
    u32 size;

    Opcode opcode;
    Operand source;
    Operand destination;

    Flags flags;
} Instruction;