#pragma once

#include "instruction.h"
#include "../sanity.h"
#include <cstdlib>

enum PartType {
    DT_BITS,
    DT_WIDE_FLAG,
    DT_DEST_FLAG,
    DT_MOD,
    DT_REG,
    DT_RM,
    DT_DATA,
    DT_DATA_W,
    DT_ADDR_LO,
    DT_ADDR_HI,
    DT_SR,

    DT_IMP_D,
    DT_IMP_W,
    DT_IMP_REG,
    DT_IMP_MOD,
    DT_IMP_RM,

    DT_END,
};

typedef struct {
    u8 pattern;
    u8 length;
} BitPattern;

typedef struct {
    PartType type;
    union {
        BitPattern pattern;
        u8 value;
    };
} Part;

typedef struct {
    Opcode opcode;
    Part parts[16];
} InstructionDescription;

typedef struct {
    InstructionDescription* instructions;
    size_t length;
} DescriptionList;

DescriptionList get_description_list();