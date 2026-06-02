#include <cstdio>

#include "../sanity.h"
#include "./instruction_list.h"
#include "memory.h"

typedef struct {
    u8 rm;
    u8 reg;
    u8 mod;
    u8 sr;
    bool is_wide;
    bool is_signed;
    bool is_dest;
    bool has_reg;
    bool has_rm;
    bool has_mod;
    bool has_addr;
    bool has_sr;
    bool has_data;
    bool has_wide_data;
} InstructionData;

const DescriptionList instructions = get_description_list();

// reg / is_wide
const Register reg_lookup[8][2] = {
    {REG_AL, REG_AX},
    {REG_CL, REG_CX},
    {REG_DL, REG_DX},
    {REG_BL, REG_BX},
    {REG_AH, REG_SP},
    {REG_CH, REG_BP},
    {REG_DH, REG_SI},
    {REG_BH, REG_DI},
};

// rm
const Register rm_register_lookup[8][2] = {
    {REG_BX, REG_SI},
    {REG_BX, REG_DI},
    {REG_BP, REG_SI},
    {REG_BP, REG_DI},
    {REG_SI, REG_NONE},
    {REG_DI, REG_NONE},
    {REG_BP, REG_NONE},
    {REG_BX, REG_NONE},
};

u8 get_bits(u8* byte, int index, int length) {
    return (*byte >> (index - (length - 1))) & ((1 << length) - 1);
}

bool get_bit(u8* byte, int index) {
    return (*byte & (1 << index)) > 0;
}

s32 parse_value(MemAccess* access, bool exists, bool is_wide, bool is_signed) {
    if (!exists) {
        return 0;
    }

    if (is_wide) {
        u32 raw = (u32)access_memory(*access)[1] << 8 | (u32)access_memory(*access)[0];
        s32 result = is_signed ? (s32)(s16)raw : (s32)raw;
        access->offset += 2;
        return result;
    }
    else {
        u32 raw = access_memory(*access)[0];
        s32 result = is_signed ? (s32)(s8)raw : (s32)raw;
        access->offset += 1;
        return result;
    }
}

Instruction attempt_decode(MemAccess access, InstructionDescription* description) {
    int bit_index = 7;
    int part_index = 0;
    bool is_valid = true;

    u32 first_address = absolute_address(access.segment, access.offset);

    InstructionData data = {};

    u8* byte = access_memory(access);

    while (is_valid && description->parts[part_index].type != DT_END) {
        Part part = description->parts[part_index];

        switch (part.type) {
            case DT_BITS:
                if (get_bits(byte, bit_index, part.pattern.length) == part.pattern.pattern) {
                    bit_index -= part.pattern.length;
                    break;
                }
                is_valid = false;
                break;
            case DT_WIDE_FLAG:
                data.is_wide = get_bit(byte, bit_index);
                bit_index--;
                break;
            case DT_DEST_FLAG:
                data.is_dest = get_bit(byte, bit_index);
                bit_index--;
                break;
            case DT_SIGN_FLAG:
                data.is_signed = get_bit(byte, bit_index);
                bit_index--;
                break;
            case DT_MOD:
                data.mod = get_bits(byte, bit_index, 2);
                bit_index -= 2;
                data.has_mod = true;
                break;
            case DT_REG:
                data.reg = get_bits(byte, bit_index, 3);
                bit_index -= 3;
                data.has_reg = true;
                break;
            case DT_RM:
                data.rm = get_bits(byte, bit_index, 3);
                bit_index -= 3;
                data.has_rm = true;
                break;
            case DT_DATA:
                data.has_data = true;
                break;
            case DT_DATA_W:
                data.has_wide_data = true;
                break;
            case DT_ADDR_HI:
            case DT_ADDR_LO:
                break;
            case DT_SR:
                data.sr = get_bits(byte, bit_index, 2);
                bit_index = -2;
                data.has_sr = true;
                break;
            case DT_IMP_D:
                data.is_dest = part.value;
                break;
            case DT_IMP_W:
                data.is_wide = part.value;
                break;
            case DT_IMP_REG:
                data.has_reg = true;
                data.reg = part.value;
                break;
            case DT_IMP_MOD:
                data.has_mod = true;
                data.mod = part.value;
                break;
            case DT_IMP_RM:
                data.has_rm = true;
                data.rm = part.value;
                break;

            // should never be reached
            case DT_END:
                fprintf(stderr, "ERROR: Encountered END fragment.\n");
                exit(1);
        }

        if (bit_index < 0) {
            bit_index = 7;
            access.offset += 1;
            byte = access_memory(access);
        }

        if (absolute_address(access.segment, access.offset) - first_address >= 15) {
            is_valid = false;
            break;
        }

        part_index += 1;;
    }

    if (is_valid) {
        bool is_wide = data.is_wide;
        bool is_signed = data.is_signed;
        bool is_dest = data.is_dest;
        bool has_reg = data.has_reg;
        bool has_mod = data.has_mod;
        bool has_data = data.has_data;
        bool data_is_wide = data.has_wide_data && data.is_wide && !data.is_signed;
        bool has_direct_address = data.mod == 0b00 && data.rm == 0b110;
        bool has_displacement = data.mod == 0b01 || data.mod == 0b10;

        s32 direct_address = parse_value(&access, has_direct_address, is_wide, is_signed);
        s32 displacement = parse_value(&access, has_displacement, data.mod == 0b10, true);
        s32 data_value = parse_value(&access, has_data, data_is_wide, is_signed);

        Instruction result;

        result.address = first_address;
        result.size = absolute_address(access.segment, access.offset) - first_address;
        result.opcode = description->opcode;
        result.source = Operand{OP_T_NONE, REG_NONE};
        result.destination = Operand{OP_T_NONE, REG_NONE};
        result.flags = Flags{is_wide, is_signed, is_dest};

        Operand *reg_op = &(is_dest ? result.destination : result.source);
        Operand *mod_op = &(is_dest ? result.source : result.destination);

        if (has_reg) {
            *reg_op = Operand{OP_T_REG, reg_lookup[data.reg][is_wide]};
        }

        if (has_mod) {
            if (data.mod == 0b00) {
                if (has_direct_address) {
                    mod_op->type = OP_T_ADDRESS;
                    mod_op->effectiveAddress = EffectiveAddress{
                        REG_NONE,
                        REG_NONE,
                        direct_address
                    };
                }
                else {
                    mod_op->type = OP_T_ADDRESS;
                    mod_op->effectiveAddress = EffectiveAddress{
                        rm_register_lookup[data.rm][0],
                        rm_register_lookup[data.rm][1],
                        0,
                    };
                }
            }
            else if (data.mod == 0b11) {
                *mod_op = Operand{OP_T_REG, reg_lookup[data.rm][is_wide]};
            }
            else {
                mod_op->type = OP_T_ADDRESS;
                mod_op->effectiveAddress = EffectiveAddress{
                    rm_register_lookup[data.rm][0],
                    rm_register_lookup[data.rm][1],
                    displacement,
                };
            }
        }

        if (has_data) {
            Operand* target = &result.destination;

            if (result.source.type == OP_T_NONE) {
                target = &result.source;
            }

            target->type = OP_T_IMMEDIATE;
            target->immediate = Immediate{data_value, true, is_wide};
        }

        return result;
    }

    return Instruction{0, 0, OP_MOV};
}

Instruction decode(MemAccess access) {
    for (int i = 0; i < instructions.length; i++) {
        Instruction result = attempt_decode(access, &instructions.instructions[i]);
        if (result.size > 0) {
            return result;
        }
    }

    return Instruction{0, 0, OP_MOV};
}

