#include "instruction_list.h"
#include "../sanity.h"
#include "./instruction.h"

#define OP(Opcode, ...) InstructionDescription{Opcode, {__VA_ARGS__, Part{DT_END}}}
#define BITS(Pattern, Size) Part{DT_BITS, {BitPattern{Pattern, Size}}}
#define D Part{DT_DEST_FLAG}
#define W Part{DT_WIDE_FLAG}
#define S Part{DT_SIGN_FLAG}
#define MOD Part{DT_MOD}
#define REG Part{DT_REG}
#define RM Part{DT_RM}
#define DATA Part{DT_DATA}
#define DATA_W Part{DT_DATA_W}
#define ADDR_LO Part{DT_ADDR_LO}
#define ADDR_HI Part{DT_ADDR_HI}
#define SR Part{DT_SR}

#define IMP_D(Value) Part{DT_IMP_D, Value}
#define IMP_W(Value) Part{DT_IMP_W, Value}
#define IMP_REG(Value) Part{DT_IMP_REG, Value}
#define IMP_MOD(Value) Part{DT_IMP_MOD, Value}
#define IMP_RM(Value) Part{DT_IMP_RM, Value}

static InstructionDescription instruction_list[] = {
    OP(OP_MOV, BITS(0b100010, 6), D, W, MOD, REG, RM),
    OP(OP_MOV, BITS(0b1100011, 7), W, MOD, BITS(0b000, 3), RM, DATA, DATA_W, IMP_D(0)),
    OP(OP_MOV, BITS(0b1011, 4), W, REG, DATA, DATA_W, IMP_D(0)),
    OP(OP_MOV, BITS(0b1010000, 7), W, ADDR_LO, ADDR_HI, IMP_D(1), IMP_REG(0), IMP_MOD(0), IMP_RM(0b110)),
    OP(OP_MOV, BITS(0b1010001, 7), W, ADDR_LO, ADDR_HI, IMP_D(0), IMP_REG(0), IMP_MOD(0), IMP_RM(0b110)),
    OP(OP_MOV, BITS(0b100011, 5), D, BITS(0b0, 1), MOD, BITS(0, 1), SR, RM, IMP_W(1)),

    OP(OP_ADD, BITS(0b000000, 6), D, W, MOD, REG, RM),
    OP(OP_ADD, BITS(0b100000, 6), S, W, MOD, BITS(0b000, 3), RM, DATA, DATA_W),
    OP(OP_ADD, BITS(0b0000010, 7), W, DATA, DATA_W, IMP_REG(0), IMP_D(1)),

    OP(OP_SUB, BITS(0b010100, 6), D, W, MOD, REG, RM),
    OP(OP_SUB, BITS(0b100000, 6), S, W, MOD, BITS(0b101, 3), RM, DATA, DATA_W),
    OP(OP_SUB, BITS(0b0010110, 7), W, DATA, DATA_W, IMP_REG(0), IMP_D(1)),
};

DescriptionList get_description_list() {
    DescriptionList result;

    result.instructions = instruction_list;
    result.length = ArrayLength(instruction_list);

    return result;
}

#undef OP
#undef PATTERN
#undef D
#undef W
#undef S
#undef MOD
#undef REG
#undef RM
#undef DATA
#undef DATA_W
#undef ADDR_LO
#undef ADDR_HI
#undef SR

#undef IMP_D
#undef IMP_REG
#undef IMP_MOD
#undef IMP_RM
