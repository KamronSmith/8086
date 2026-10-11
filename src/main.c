#include "kam.h"
#include <endian.h>

#define OPCODE_MASK (0x3F << 10u)
#define DIRECTION_MASK (0b1)
#define WB_MASK (0b1)
#define MOD_FIELD_MASK (0b11 << 6u)
#define REG_FIELD_MASK (0b111)
#define RM_FIELD_MASK (0b111)

typedef struct {
    u8 instruction;
    bool direction;
    bool wb;
    u8 mod;
    u8 register_field;
    u8 rm_field;
    char reg1[3];
    char reg2[3];
} Mov;

void print_instruction(Mov *mov) {
    printf("mov %s, %s\n", mov->reg2, mov->reg1);
}

void determine_registers(Mov *mov) {
    if (mov->wb == 1) {
        switch (mov->register_field) {
        case 0b000:
            strcpy(mov->reg1, "AX");
            break;
        case 0b001:
            strcpy(mov->reg1, "CX");
            break;
        case 0b010:
            strcpy(mov->reg1, "DX");
            break;
        case 0b011:
            strcpy(mov->reg1, "BX");
            break;
        case 0b100:
            strcpy(mov->reg1, "SP");
            break;
        case 0b101:
            strcpy(mov->reg1, "BP");
            break;
        case 0b110:
            strcpy(mov->reg1, "SI");
            break;
        case 0b111:
            strcpy(mov->reg1, "DI");
            break;
        default:
            printf("fuck\n");
        }

        switch (mov->rm_field) {
        case 0b000:
            strcpy(mov->reg2, "AX");
            break;
        case 0b001:
            strcpy(mov->reg2, "CX");
            break;
        case 0b010:
            strcpy(mov->reg2, "DX");
            break;
        case 0b011:
            strcpy(mov->reg2, "BX");
            break;
        case 0b100:
            strcpy(mov->reg2, "SP");
            break;
        case 0b101:
            strcpy(mov->reg2, "BP");
            break;
        case 0b110:
            strcpy(mov->reg2, "SI");
            break;
        case 0b111:
            strcpy(mov->reg2, "DI");
            break;
        }
    } else {
        switch (mov->register_field) {
        case 0b000:
            strcpy(mov->reg1, "AL");
            break;
        case 0b001:
            strcpy(mov->reg1, "CL");
            break;
        case 0b010:
            strcpy(mov->reg1, "DL");
            break;
        case 0b011:
            strcpy(mov->reg1, "BL");
            break;
        case 0b100:
            strcpy(mov->reg1, "AH");
            break;
        case 0b101:
            strcpy(mov->reg1, "CH");
            break;
        case 0b110:
            strcpy(mov->reg1, "DH");
            break;
        case 0b111:
            strcpy(mov->reg1, "BH");
            break;
        default:
            printf("fuck\n");
        }
        
        switch (mov->rm_field) {
        case 0b000:
            strcpy(mov->reg2, "AL");
            break;
        case 0b001:
            strcpy(mov->reg2, "CL");
            break;
        case 0b010:
            strcpy(mov->reg2, "DL");
            break;
        case 0b011:
            strcpy(mov->reg2, "BL");
            break;
        case 0b100:
            strcpy(mov->reg2, "AH");
            break;
        case 0b101:
            strcpy(mov->reg2, "CH");
            break;
        case 0b110:
            strcpy(mov->reg2, "DH");
            break;
        case 0b111:
            strcpy(mov->reg2, "BH");
            break;
        default:
            printf("fuck\n");
        }
    }
}

int main() {

    FILE *f = fopen("complex", "rb");
    
    if (f == NULL) {
        printf("File unavailable");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    u32 filesize = (u32)ftell(f);
    fseek(f, 0, SEEK_SET);

    u16 *buf = malloc(filesize);
    size_t read = fread(buf, 2, filesize, f);
    
    for (size_t i = 0; i < read; i++) {
        u16 correct = htobe16(buf[i]);
        Mov *mov = malloc(sizeof(Mov));
        mov->instruction = (correct & OPCODE_MASK) >> 10;
        mov->direction = DIRECTION_MASK & (correct >> 9);
        mov->wb = WB_MASK & (correct >> 8);
        mov->mod = (correct & MOD_FIELD_MASK) >> 6;
        mov->register_field = REG_FIELD_MASK & (correct >> 3);
        mov->rm_field = correct & RM_FIELD_MASK;

        determine_registers(mov);
        print_instruction(mov);
        free(mov);
    }

    free(buf);

    fclose(f);
}
