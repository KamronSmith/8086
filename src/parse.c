#include "parse.h"
#include "kam.h"
#ifdef __APPLE__
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#define OPCODE_MOVE 26
#define DIRECTION_MASK (0b1)
#define WB_MASK (0b1)
#define MOD_FIELD_MASK (0b11 << 6u)
#define REG_FIELD_MASK (0b111)
#define RM_FIELD_MASK (0b111)

void print_instruction(Inst *mov) {
    sb_append_null(&mov->reg1);
    sb_append_null(&mov->reg2);
    sb_append_null(&mov->name);
    if (mov->direction == SOURCE) {
        printf("%s %s, %s\n", mov->name.items, mov->reg2.items, mov->reg1.items);
    } else {
        printf("%s %s, %s\n", mov->name.items, mov->reg1.items, mov->reg2.items);
    }

    free(mov->reg1.items);
    free(mov->name.items);
    free(mov->reg2.items);
}

i16 combine_i8_to_i16(i8 low_byte, i8 high_byte) {
    u16 high_shifted = ((u16)(u8)high_byte << 8);
    u16 combined = high_shifted | ((u16)(u8)low_byte);
    return (i16)combined;
}

void determine_instruction_type(Inst *ins, u8 bytes) {
    if (bytes == 0 || bytes <= 0x03) {
        sb_append_cstr(&ins->name, "add");
        ins->type = NORM;
    }
    else if (bytes == 0x04 || bytes == 0x05) {
        sb_append_cstr(&ins->name, "add");
        ins->type = IMMED_ACCUM;
    }
    else if (bytes >= 0x28 && bytes <= 0x2B) {
        sb_append_cstr(&ins->name, "sub");
        ins->type = NORM;
    }
    else if (bytes == 0x2C || bytes == 0x2D) {
        sb_append_cstr(&ins->name, "sub");
        ins->type = IMMED_ACCUM;
    }
    else if (bytes >= 0x38 && bytes <= 0x3B) {
        sb_append_cstr(&ins->name, "cmp");
        ins->type = NORM;
    }
    else if (bytes == 0x3C || bytes == 0x3D) {
        sb_append_cstr(&ins->name, "cmp");
        ins->type = IMMED_ACCUM;
    }
    else if (bytes == 0x70) {
        sb_append_cstr(&ins->name, "jo");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x71) {
        sb_append_cstr(&ins->name, "jno");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x72) {
        sb_append_cstr(&ins->name, "jb");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x73) {
        sb_append_cstr(&ins->name, "jnb");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x74) {
        sb_append_cstr(&ins->name, "je");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x75) {
        sb_append_cstr(&ins->name, "jnz");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x76) {
        sb_append_cstr(&ins->name, "jbe");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x77) {
        sb_append_cstr(&ins->name, "jnbe");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x78) {
        sb_append_cstr(&ins->name, "js");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x79) {
        sb_append_cstr(&ins->name, "jns");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x7A) {
        sb_append_cstr(&ins->name, "jp");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x7B) {
        sb_append_cstr(&ins->name, "jnp");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x7C) {
        sb_append_cstr(&ins->name, "jl");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x7D) {
        sb_append_cstr(&ins->name, "jnl");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x7E) {
        sb_append_cstr(&ins->name, "jle");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x7F) {
        sb_append_cstr(&ins->name, "jnle");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0xe0) {
        sb_append_cstr(&ins->name, "loopnz");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0xe1) {
        sb_append_cstr(&ins->name, "loopz");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0xe2) {
        sb_append_cstr(&ins->name, "loop");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0xe3) {
        sb_append_cstr(&ins->name, "jcxz");
        ins->type = JMP_CMP;
    }
    else if (bytes == 0x80 || bytes == 0x81 || bytes == 0x82 || bytes == 0x83) {
        ins->type = IMMED_REG_MEM;
    }
    else if (bytes >= 0x88 && bytes <= 0x8B) {
        sb_append_cstr(&ins->name, "mov");
        ins->type = NORM;
    }
    else if (bytes >= 0xB0 && bytes <= 0xBF) {
        sb_append_cstr(&ins->name, "mov");
        ins->type = IMMED;
    }
    return;
}

void determine_register(Inst *ins, String_Builder *sb) {
    if (ins->wb == WORD) {
        switch (ins->register_field) {
        case 0b000:
            sb_append_cstr(sb, "AX");
            break;
        case 0b001:
            sb_append_cstr(sb, "CX");
            break;
        case 0b010:
            sb_append_cstr(sb, "DX");
            break;
        case 0b011:
            sb_append_cstr(sb, "BX");
            break;
        case 0b100:
            sb_append_cstr(sb, "SP");
            break;
        case 0b101:
            sb_append_cstr(sb, "BP");
            break;
        case 0b110:
            sb_append_cstr(sb, "SI");
            break;
        case 0b111:
            sb_append_cstr(sb, "DI");
            break;
        default:
            printf("fuck\n");
        }
    }
    else {
        switch (ins->register_field) {
        case 0b000:
            sb_append_cstr(sb, "AL");
            break;
        case 0b001:
            sb_append_cstr(sb, "CL");
            break;
        case 0b010:
            sb_append_cstr(sb, "DL");
            break;
        case 0b011:
            sb_append_cstr(sb, "BL");
            break;
        case 0b100:
            sb_append_cstr(sb, "AH");
            break;
        case 0b101:
            sb_append_cstr(sb, "CH");
            break;
        case 0b110:
            sb_append_cstr(sb, "DH");
            break;
        case 0b111:
            sb_append_cstr(sb, "BH");
            break;
        default:
            printf("fuck\n");
        }
    }
}

void backup_reg_func(Inst *ins, String_Builder *sb) {
    if (ins->wb == WORD) {
        switch (ins->rm_field) {
        case 0b000:
            sb_append_cstr(sb, "AX");
            break;
        case 0b001:
            sb_append_cstr(sb, "CX");
            break;
        case 0b010:
            sb_append_cstr(sb, "DX");
            break;
        case 0b011:
            sb_append_cstr(sb, "BX");
            break;
        case 0b100:
            sb_append_cstr(sb, "SP");
            break;
        case 0b101:
            sb_append_cstr(sb, "BP");
            break;
        case 0b110:
            sb_append_cstr(sb, "SI");
            break;
        case 0b111:
            sb_append_cstr(sb, "DI");
            break;
        default:
            printf("fuck\n");
        }
    } else {
        switch (ins->rm_field) {
        case 0b000:
            sb_append_cstr(sb, "AL");
            break;
        case 0b001:
            sb_append_cstr(sb, "CL");
            break;
        case 0b010:
            sb_append_cstr(sb, "DL");
            break;
        case 0b011:
            sb_append_cstr(sb, "BL");
            break;
        case 0b100:
            sb_append_cstr(sb, "AH");
            break;
        case 0b101:
            sb_append_cstr(sb, "CH");
            break;
        case 0b110:
            sb_append_cstr(sb, "DH");
            break;
        case 0b111:
            sb_append_cstr(sb, "BH");
            break;
        default:
            printf("fuck\n");
        }
    }
}

void calculate_effective_address(Inst *ins, u8 *buf, size_t *index) {
    i16 disp;
    char str[10];
    switch (ins->mod) {
    case MOD_NO_DISP:
        switch (ins->rm_field) {
        case 0b000:
            sb_append_cstr(&ins->reg2, "[bx + si]");
            break;
        case 0b001:
            sb_append_cstr(&ins->reg2, "[bx + di]");
            break;
        case 0b010:
            sb_append_cstr(&ins->reg2, "[bp + si]");
            break;
        case 0b011:
            sb_append_cstr(&ins->reg2, "[bp + di]");
            break;
        case 0b100:
            sb_append_cstr(&ins->reg2, "[si]");
            break;
        case 0b101:
            sb_append_cstr(&ins->reg2, "[di]");
            break;
        case 0b110:
            break;
        case 0b111:
            sb_append_cstr(&ins->reg2, "[bx]");
            break;
        }
        break;
    case MOD_8B_DISP:
        *index += 1;
        switch (ins->rm_field) {
        case 0b000:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b001:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b010:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b011:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b100:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b101:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b110:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b111:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        }
        // (*index)++;
        break;
    case MOD_16_DISP:
        (*index)++;
        disp = combine_i8_to_i16(buf[*index], buf[*index + 1]);
        switch (ins->rm_field) {
        case 0b000:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b001:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b010:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b011:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b100:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b101:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b110:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b111:
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        }
        (*index)++;
        break;
    case MOD_REG_MODE:
        backup_reg_func(ins, &ins->reg2);
        break;
    }
}

void determine_rm_field(Inst *ins, u8 *buf, size_t *index) {
    if (ins->mod == MOD_REG_MODE && ins->rm_field == 0b110) {
        calculate_effective_address(ins, buf, index);
        return;
    }

    if (ins->mod != MOD_REG_MODE) {
        calculate_effective_address(ins, buf, index);
        return;
    }

    backup_reg_func(ins, &ins->reg2);
}

void parse_immed_instruction(Inst *ins, u8 *buf, size_t *index)
{

    // (*index)++; disabled for MOV immed instructions only
    // Need a new function just for MOV immed instructions
    ins->wb = (buf[*index] >> 3) & 0b1;
    ins->register_field = (buf[*index] & 0x3);
    ins->direction = true;
    (*index)++;
    i8 disp = buf[*index];

    if (ins->wb == WORD) {
        (*index)++;
        i16 disp_16 = combine_i8_to_i16(disp, buf[*index]);
        char str[7];
        snprintf(str, sizeof(str), "%i", disp_16);
        sb_append_cstr(&ins->reg2, str);
        determine_register(ins, &ins->reg1);
        print_instruction(ins);
        (*index)++;
        return;
    }

    char str[5];
    snprintf(str, sizeof(str), "%d", disp);
    sb_append_cstr(&ins->reg2, str);
    determine_register(ins, &ins->reg1);
    print_instruction(ins);
    (*index)++;
}

void parse_jmp_cmp_instruction(Inst *ins, u8 *buf, size_t *index) {
    (*index)++;
    i8 label = buf[*index];

    char str[5];
    snprintf(str, sizeof(str), "%i", label);
    sb_append_cstr(&ins->reg2, str);
    print_instruction(ins);
    (*index)++;
}

void parse_immed_accumulator(Inst *ins, u8 *buf, size_t *index) {
    ins->wb = buf[*index] & 1;
    ins->direction = DEST;

    (*index)++;
    i8 disp = buf[*index];

    if (ins->wb == WORD) {
        (*index)++;
        i16 disp_16 = combine_i8_to_i16(disp, buf[*index]);
        char str[7];
        snprintf(str, sizeof(str), "%i", disp_16);
        sb_append_cstr(&ins->reg2, str);
        determine_register(ins, &ins->reg1);
        print_instruction(ins);
        (*index)++;
        return;
    }

    char str[5];
    snprintf(str, sizeof(str), "%d", disp);
    sb_append_cstr(&ins->reg2, str);
    determine_register(ins, &ins->reg1);
    print_instruction(ins);
    (*index)++;
}

void parse_instruction(u8 *buf, size_t size) {
    size_t index = 0;

    while (index < size) {
        Inst ins = {0};
        u8 instruction_bytes = buf[index];
        determine_instruction_type(&ins, instruction_bytes);

        if (ins.type == IMMED) {
            parse_immed_instruction(&ins, buf, &index);
            continue;
        }

        if (ins.type == IMMED_ACCUM) {
            parse_immed_accumulator(&ins, buf, &index);
            continue;
        }

        if (ins.type == JMP_CMP) {
            parse_jmp_cmp_instruction(&ins, buf, &index);
            continue;
        }

        // ADD, SUB, CMP immediates need to be checked in the second byte
        if (ins.type == IMMED_REG_MEM) {
            ins.sign = (buf[index] >> 1) & 0x1;
            ins.wb = buf[index] & 0b1;
            index++;
            ins.register_field = (buf[index] >> 3) & 0x7;
            ins.mod = (buf[index] >> 6) & 0b11;
            ins.rm_field = buf[index] & 0b111;
            switch (ins.register_field) {
            case 0:
                sb_append_cstr(&ins.name, "add");
                break;
            case 5:
                sb_append_cstr(&ins.name, "sub");
                break;
            case 7:
                sb_append_cstr(&ins.name, "cmp");
                break;
            }

            if (ins.mod != MOD_REG_MODE) {
                calculate_effective_address(&ins, buf, &index);
            }
            else {
                backup_reg_func(&ins, &ins.reg1);
            }

            index++;
            i8 data = buf[index];
            char str[10];
            if (ins.wb == WORD) {
                snprintf(str, sizeof(str), "%d", data);
                sb_append_cstr(&ins.reg2, str);
                print_instruction(&ins);
                index++;
                continue;
            }

            snprintf(str, sizeof(str), "%i", data);
            sb_append_cstr(&ins.reg1, str);
            print_instruction(&ins);
            index++;
            continue;
        }

        ins.direction = (buf[index] >> 1) & 1;
        ins.wb = buf[index] & 0b1;
        index++;

        ins.mod = (buf[index] >> 6) & 0x3;
        ins.register_field = (buf[index] >> 3) & 0x7;
        ins.rm_field = buf[index] & 0b111;

        determine_register(&ins, &ins.reg1);
        determine_rm_field(&ins, buf, &index);
        print_instruction(&ins);
        index++;
    }
}
