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

typedef enum {
    MOD_NO_DISP,
    MOD_8B_DISP,
    MOD_16_DISP,
    MOD_REG_MODE,
} MOD_field;

typedef enum {
    AX,
    CX,
    DX,
    BX,
    SP,
    BP,
    SI,
    DI,
    AL,
    CL,
    DL,
    BL,
    AH,
    CH,
    DH,
    BH,
} Reg_Field;

typedef enum {
    SOURCE,
    DEST,
} DEST_field;

typedef enum {
    BYTE,
    WORD,
} WB_field;

typedef enum {
    NORM,
    IMMED,
} Inst_type;

typedef struct {
    Inst_type type;
    DEST_field direction;
    WB_field wb;
    MOD_field mod;
    u8 register_field;
    u8 rm_field;
    String_Builder reg1;
    String_Builder reg2;
    i8 immed_value;
} Inst;

void print_instruction(Inst *mov) {
    sb_append_null(&mov->reg1);
    sb_append_null(&mov->reg2);
    if (mov->direction == SOURCE) {
        printf("mov %s, %s\n", mov->reg2.items, mov->reg1.items);
    } else {
        printf("mov %s, %s\n", mov->reg1.items, mov->reg2.items);
    }
}

void check_instruction(Inst *mov, u8 bytes) {
    if (bytes == 0b101100) {
        mov->type = IMMED;
    } else {
        mov->type = NORM;
    }
}

void determine_register(Inst *mov, String_Builder *sb) {
    if (mov->wb == WORD) {
        switch (mov->register_field) {
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
        switch (mov->register_field) {
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

void backup_reg_func(Inst *mov, String_Builder *sb) {
    if (mov->wb == WORD) {
        switch (mov->rm_field) {
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
        switch (mov->rm_field) {
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
void calculate_effective_address(Inst *mov, u8 *buf, size_t *index) {
    u16 disp = 0;
    char str[4];
    switch (mov->mod) {
    case MOD_NO_DISP:
        switch (mov->rm_field) {
        case 0b000:
            sb_append_cstr(&mov->reg2, "[bx + di]");
            break;
        case 0b001:
            sb_append_cstr(&mov->reg2, "[bx + di]");
            break;
        case 0b010:
            sb_append_cstr(&mov->reg2, "[bp + si]");
            break;
        case 0b011:
            sb_append_cstr(&mov->reg2, "[bp + di]");
            break;
        case 0b100:
            sb_append_cstr(&mov->reg2, "[si]");
            break;
        case 0b101:
            sb_append_cstr(&mov->reg2, "[di]");
            break;
        case 0b110:
            break;
        case 0b111:
            sb_append_cstr(&mov->reg2, "[bx]");
            break;
        }
        break;
    case MOD_8B_DISP:
        *index += 1;
        switch (mov->rm_field) {
        case 0b000:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bx + si + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b001:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bx + di + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b010:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bp + si + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b011:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bp + di + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b100:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[si + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b101:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[di + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b110:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bp + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b111:
            disp = buf[*index];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bx + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        }
        break;
    case MOD_16_DISP:
        *index += 2;
        switch (mov->rm_field) {
        case 0b000:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bx + si + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b001:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bx + di + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b010:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bp + si + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b011:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bp + di + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b100:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[si + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b101:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[di + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b110:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bp + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        case 0b111:
            disp = buf[*index] + buf[*index + 1];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov->reg2, "[bx + ");
            sb_append_cstr(&mov->reg2, str);
            sb_append_cstr(&mov->reg2, "]");
            break;
        }
        break; 
    case MOD_REG_MODE:
        break;
    }
}

void determine_rm_field(Inst *mov, u8 *buf, size_t *index) {
    if (mov->mod == MOD_REG_MODE && mov->rm_field == 0b110) {
        calculate_effective_address(mov, buf, index);
    } else if (mov->mod != MOD_REG_MODE) {
        calculate_effective_address(mov, buf, index);
    }

    backup_reg_func(mov, &mov->reg2);
}

void parse_instruction(u8 *buf, size_t size) {
    size_t index = 0;
    
    while (index < size) {
        Inst mov = {0};
        u8 instruction_bytes = (buf[index] >> 2) & 0x3F;
        check_instruction(&mov, instruction_bytes);
        // mov.instruction = (buf[index] >> 2) & 0x3F;

        if (mov.type == IMMED) {
            mov.wb = (buf[index] >> 3) & 0b1;
            mov.register_field = (buf[index] & 0x3);
            mov.direction = true;
            index++;
            // mov.immed_value = buf[index];
            i16 disp = buf[index];
            char str[5];
            snprintf(str, sizeof(str), "%d", disp);
            sb_append_cstr(&mov.reg2, str);
            determine_register(&mov, &mov.reg1);
            print_instruction(&mov);
            index++;
            continue;
        }
        
        
        mov.direction = (buf[index] >> index) & 1;
        mov.wb = buf[index] & 0b1;
        index++;
        mov.mod = (buf[index] >> 6) & 0x3;
        mov.register_field = (buf[index] >> 3) & 0x7;
        mov.rm_field = buf[index] & 0b111;
        index++;
        determine_register(&mov, &mov.reg1);

        if (mov.mod == MOD_REG_MODE && mov.rm_field == 0b110) {
            // index += 2;
        } else if (mov.mod == MOD_16_DISP) {
            index += 2;
        } else if (mov.mod == MOD_8B_DISP) {
            index += 1;
        }

        determine_rm_field(&mov, buf, &index);
        print_instruction(&mov);
    }
}

int main() {
    FILE *f = fopen("bin/new_hmwk", "rb");
    
    if (f == NULL) {
        printf("File unavailable\n");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    u8 filesize = (u8)ftell(f);
    fseek(f, 0, SEEK_SET);
    
    u8 buf[filesize];
    size_t size = fread(buf, 1, filesize, f);
    UNUSED(size);

    parse_instruction(buf, size);

    // 111111 11 11 111 111 11111111 11111111
    // ------ -- -- --- ---
    // mod    dw mo reg rm

    
    fclose(f);
}
