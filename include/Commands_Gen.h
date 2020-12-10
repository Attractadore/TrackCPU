/**
THIS FILE WAS GENERATED AUTOMATICALLY
IF YOU WANT TO ALTER IT MAKE CHANGES TO codegen.py
**/

#pragma once

#include <stddef.h>
typedef enum: size_t {
    CMD_HALT_CODE,
    CMD_CALL_CODE,
    CMD_RETURN_CODE,
    CMD_JUMP_CODE,
    CMD_JUMPT_CODE,
    CMD_JUMPF_CODE,
    CMD_PUSH_CODE,
    CMD_PUSHI_CODE,
    CMD_PUSHU_CODE,
    CMD_PUSHF_CODE,
    CMD_POP_CODE,
    CMD_POPS_CODE,
    CMD_TOP_CODE,
    CMD_EQI_CODE,
    CMD_EQU_CODE,
    CMD_EQF_CODE,
    CMD_NEI_CODE,
    CMD_NEU_CODE,
    CMD_NEF_CODE,
    CMD_LI_CODE,
    CMD_LU_CODE,
    CMD_LF_CODE,
    CMD_LEI_CODE,
    CMD_LEU_CODE,
    CMD_LEF_CODE,
    CMD_GI_CODE,
    CMD_GU_CODE,
    CMD_GF_CODE,
    CMD_GEI_CODE,
    CMD_GEU_CODE,
    CMD_GEF_CODE,
    CMD_BAND_CODE,
    CMD_AND_CODE,
    CMD_BOR_CODE,
    CMD_OR_CODE,
    CMD_BXOR_CODE,
    CMD_BNOT_CODE,
    CMD_NOT_CODE,
    CMD_LS_CODE,
    CMD_RS_CODE,
    CMD_LRS_CODE,
    CMD_ADDI_CODE,
    CMD_ADDU_CODE,
    CMD_ADDF_CODE,
    CMD_SUBI_CODE,
    CMD_SUBU_CODE,
    CMD_SUBF_CODE,
    CMD_MULI_CODE,
    CMD_MULU_CODE,
    CMD_MULF_CODE,
    CMD_DIVI_CODE,
    CMD_DIVU_CODE,
    CMD_DIVF_CODE,
    CMD_MODI_CODE,
    CMD_MODU_CODE,
    CMD_SQRT_CODE,
    CMD_PRINTI_CODE,
    CMD_PRINTU_CODE,
    CMD_PRINTF_CODE,
    CMD_SCANI_CODE,
    CMD_SCANU_CODE,
    CMD_SCANF_CODE,
    CMD_LOAD_CODE,
    CMD_STORE_CODE,
    CMD_MOVE_CODE,
    CMD_INVALID_CODE
} CommandCode;

typedef enum {
    CMD_ARG_TYPE_NONE,
    CMD_ARG_TYPE_INT,
    CMD_ARG_TYPE_UINT,
    CMD_ARG_TYPE_FLOAT,
    CMD_ARG_TYPE_LABEL,
    CMD_ARG_TYPE_REGISTER
} CommandArgType;

enum { COMMAND_MAX_ARGS = 1 };

#include <stddef.h>
typedef struct {
    char const* const name;
    const CommandCode code;
    const size_t numArgs;
    const CommandArgType argType[COMMAND_MAX_ARGS];
} Command;

#include <stddef.h>
typedef enum: size_t {
    REG_CODE_RA,
    REG_CODE_RB,
    REG_CODE_RC,
    REG_CODE_RD,
    REG_CODE_RE,
    REG_CODE_RF,
    REG_CODE_RG,
    REG_CODE_RH,
    REG_CODE_RI,
    REG_CODE_RJ,
    REG_CODE_RK,
    REG_CODE_RL,
    REG_CODE_RM,
    REG_CODE_RN,
    REG_CODE_RO,
    REG_CODE_RP,
    REG_CODE_INVALID
} RegisterCode;

enum { NUM_REGISTERS = 16 };

typedef struct {
    char const* const name;
    RegisterCode const code;
} Register;