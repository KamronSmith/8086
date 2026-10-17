#ifndef _PARSE_H_
#define _PARSE_H_
#include "kam.h"

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
    IMMED_REG_MEM,
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

void print_instruction(Inst *mov);
i16 combine_i8_to_i16(i8 low_byte, i8 high_byte);
void check_instruction(Inst *mov, u8 bytes);
void determine_register(Inst *mov, String_Builder *sb);
void backup_reg_func(Inst *mov, String_Builder *sb);
void calculate_effective_address(Inst *mov, u8 *buf, size_t *index);
void determine_rm_field(Inst *mov, u8 *buf, size_t *index);
void parse_immed_instruction(Inst *mov, u8 *buf, size_t *index);
void parse_instruction(u8 *buf, size_t size);
#endif // PARSE_H
