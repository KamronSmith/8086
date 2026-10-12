#include "kam.h"
#include <endian.h>

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

typedef struct {
    u32 raw_bytes;
    u8 instruction;
    DEST_field direction;
    WB_field wb;
    MOD_field mod;
    u8 register_field;
    u8 rm_field;
    Reg_Field reg;
    char reg1[3];
    char reg2[3];
    char EAC[16];
} Mov;

void print_instruction(Mov *mov) {
    printf("mov %s, %s\n", mov->reg2, mov->reg1);
}

void determine_register(Mov *mov, char *reg) {
    if (mov->wb == WORD) {
        switch (mov->register_field) {
        case 0b000:
            strcpy(reg, "AX");
            break;
        case 0b001:
            strcpy(reg, "CX");
            break;
        case 0b010:
            strcpy(reg, "DX");
            break;
        case 0b011:
            strcpy(reg, "BX");
            break;
        case 0b100:
            strcpy(reg, "SP");
            break;
        case 0b101:
            strcpy(reg, "BP");
            break;
        case 0b110:
            strcpy(reg, "SI");
            break;
        case 0b111:
            strcpy(reg, "DI");
            break;
        default:
            printf("fuck\n");
        }
    } else {
        switch (mov->register_field) {
        case 0b000:
            strcpy(reg, "AL");
            break;
        case 0b001:
            strcpy(reg, "CL");
            break;
        case 0b010:
            strcpy(reg, "DL");
            break;
        case 0b011:
            strcpy(reg, "BL");
            break;
        case 0b100:
            strcpy(reg, "AH");
            break;
        case 0b101:
            strcpy(reg, "CH");
            break;
        case 0b110:
            strcpy(reg, "DH");
            break;
        case 0b111:
            strcpy(reg, "BH");
            break;
        default:
            printf("fuck\n");
        }
    }
}
void calculate_effective_address(Mov *mov) {
    switch (mov->mod) {
    case MOD_NO_DISP:
        break;
    case MOD_8B_DISP:
        break;
    case MOD_16_DISP:
        break;
    case MOD_REG_MODE:
        break;
    }
}

void determine_rm_field(Mov *mov) {
    if (mov->mod != MOD_REG_MODE) {
        calculate_effective_address(mov);
    }

    determine_register(mov, mov->reg2);
}

void parse_instruction(Mov *mov, u32 bytes) {
    u32 correct = htobe32(bytes);
    mov->raw_bytes = correct;
    mov->instruction = (correct >> 26) & ((1u >> 6) - 1);
    mov->direction = (correct >> 25) & 1;
    mov->wb = (correct >> 24) & 1;
    mov->mod = (correct >> 22) & (0b11);
    mov->register_field = (correct >> 19) & (0b111);
    mov->rm_field = (correct >> 16) & (0b111);

    determine_register(mov, mov->reg1);
    determine_rm_field(mov);
}

int main() {
    FILE *f = fopen("bin/hmwk", "rb");
    
    if (f == NULL) {
        printf("File unavailable\n");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    u32 filesize = (u32)ftell(f);
    fseek(f, 0, SEEK_SET);

    u32 *buf = malloc(filesize);
    size_t read = fread(buf, 4, filesize, f);

    // 111111 11 11 111 111 11111111 11111111
    // ------ -- -- --- ---
    // mod    dw mo reg rm
    
    for (size_t i = 0; i < read; i++) {
        Mov *mov = malloc(sizeof(Mov));
        parse_instruction(mov, buf[i]);

        print_instruction(mov);
        free(mov);
    }

    free(buf);

    fclose(f);
}
