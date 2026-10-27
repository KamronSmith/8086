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
    switch (bytes) {
    case 0: // ADD norm
        sb_append_cstr(&ins->name, "add");
        ins->type = NORM;
        break;
    case 6:  // CMP NORM
        sb_append_cstr(&ins->name, "cmp");
        ins->type = NORM;
        break;
    case 10: // SUB norm
        sb_append_cstr(&ins->name, "sub");
        ins->type = NORM;
        break;
    case 32: // ADD SUB CMP IMMED REG MEM
        // sb_append_cstr(&ins->name, "fuck");
        ins->type = IMMED_REG_MEM;
        break;
    case 34: // MOV NORM
        sb_append_cstr(&ins->name, "mov");
        ins->type = NORM;
        break;
    case 44: // MOV IMMED REG 
        sb_append_cstr(&ins->name, "mov");
        ins->type = IMMED;
        break;
    case 46: // MOV IMMED REG WIDE
        sb_append_cstr(&ins->name, "mov");
        ins->type = IMMED;
        break;
    case 99: // MOV IMMED REG & MEM
        sb_append_cstr(&ins->name, "mov");
        ins->type = IMMED_REG_MEM;
        break;
    }    
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
    u16 disp = 0;
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
        // *index += 1;
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
        (*index)++;
        break;
    case MOD_16_DISP:
        // *index += 1;
        switch (ins->rm_field) {
        case 0b000:
            i16 disp = combine_i8_to_i16(buf[*index], buf[*index + 1]);
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b001:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b010:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b011:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b100:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[si + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b101:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[di + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b110:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bp + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        case 0b111:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&ins->reg2, "[bx + ");
            sb_append_cstr(&ins->reg2, str);
            sb_append_cstr(&ins->reg2, "]");
            break;
        }
        *index += 2;
        break; 
    case MOD_REG_MODE:
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

void parse_instruction(u8 *buf, size_t size) {
    size_t index = 0;

    while (index < size) {
        Inst ins = {0};
        u8 instruction_bytes = (buf[index] >> 2) & 0xFE;
        determine_instruction_type(&ins, instruction_bytes);

        if (ins.type == IMMED) {
            parse_immed_instruction(&ins, buf, &index);
            continue;
        }

        // ADD, SUB, CMP immediates need to be checked in the second byte
        if (ins.type == IMMED_REG_MEM && instruction_bytes == 32) {
            ins.sign = (buf[index] >> 1) & 0x1;
            ins.wb = buf[index] & 0b1;
            index++;
            ins.register_field = (buf[index] >> 3) & 0x7;
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

            ins.mod = MOD_16_DISP; 
            ins.rm_field = buf[index] & 0b111;
            backup_reg_func(&ins, &ins.reg1);
            determine_rm_field(&ins, buf, &index);
            print_instruction(&ins);
            continue;
        }

        ins.direction = (buf[index] >> 1) & 1;
        ins.wb = buf[index] & 0b1;
        index++;

        ins.mod = (buf[index] >> 6) & 0x3;
        ins.register_field = (buf[index] >> 3) & 0x7;
        ins.rm_field = buf[index] & 0b111;
        index++;

        determine_register(&ins, &ins.reg1);
        determine_rm_field(&ins, buf, &index);
        print_instruction(&ins);
    }
}
