#pragma once

#include <stddef.h>

#define COMMENT_START '#'
#define LABEL_START '.'
#define LABEL_END ':'

typedef enum command_code_e {
    CMD_HALT_CODE,

    CMD_CALL_CODE,
    CMD_RETURN_CODE,

    CMD_JUMP_CODE,
    CMD_JUMPEQ_CODE,
    CMD_JUMPNE_CODE,
    CMD_JUMPLE_CODE,

    CMD_PUSHW_CODE,
    CMD_PUSHULONG_CODE,
    CMD_PUSHDBL_CODE,

    CMD_POPQ_CODE,
    CMD_POPW_CODE,
    CMD_POPSTK_CODE,

    CMD_TOPW_CODE,

    CMD_CMPULONG_CODE,

    CMD_ADDULONG_CODE,
    CMD_ADDDBL_CODE,

    CMD_SUBULONG_CODE,
    CMD_SUBDBL_CODE,

    CMD_MULULONG_CODE,
    CMD_MULDBL_CODE,

    CMD_DIVULONG_CODE,
    CMD_DIVDBL_CODE,

    CMD_MODULONG_CODE,

    CMD_SQRTDBL_CODE,

    CMD_PRINTULONG_CODE,
    CMD_PRINTDBL_CODE,

    CMD_SCANULONG_CODE,
    CMD_SCANDBL_CODE,

    CMD_LOADWR_CODE,

    CMD_STOREWR_CODE,

    CMD_INVALID_CODE
} CommandCode;

typedef enum command_arg_type_e {
    CMD_ARG_TYPE_NONE = 0,
    CMD_ARG_TYPE_DOUBLE,
    CMD_ARG_TYPE_ULONG,
    CMD_ARG_TYPE_REGISTER_8,
    CMD_ARG_TYPE_REGISTER_16,
    CMD_ARG_TYPE_REGISTER_32,
    CMD_ARG_TYPE_REGISTER_64,
    CMD_ARG_TYPE_LABEL,
} CommandArgType;

enum {
    COMMAND_MAX_ARGS = 3
};

typedef struct command_t {
    char const* const name;
    const CommandCode code;
    const size_t numArgs;
    const CommandArgType argType[COMMAND_MAX_ARGS];
} Command;

Command const* getCommandByName(char const* str);
Command const* getCommandByCode(const CommandCode code);

typedef enum register_code_e {
    REG_CODE_WA0,
    REG_CODE_WB0,
    REG_CODE_WC0,
    REG_CODE_WD0,
    REG_CODE_WE0,
    REG_CODE_WF0,
    REG_CODE_WG0,
    REG_CODE_WH0,
    REG_CODE_WI0,
    REG_CODE_WJ0,
    REG_CODE_WK0,
    REG_CODE_WL0,
    REG_CODE_WM0,
    REG_CODE_WN0,
    REG_CODE_WO0,
    REG_CODE_WP0,

    REG_CODE_QA0,
    REG_CODE_QB0,
    REG_CODE_QC0,
    REG_CODE_QD0,
    REG_CODE_QE0,
    REG_CODE_QF0,
    REG_CODE_QG0,
    REG_CODE_QH0,
    REG_CODE_QI0,
    REG_CODE_QJ0,
    REG_CODE_QK0,
    REG_CODE_QL0,
    REG_CODE_QM0,
    REG_CODE_QN0,
    REG_CODE_QO0,
    REG_CODE_QP0,

    REG_CODE_QA1,
    REG_CODE_QB1,
    REG_CODE_QC1,
    REG_CODE_QD1,
    REG_CODE_QE1,
    REG_CODE_QF1,
    REG_CODE_QG1,
    REG_CODE_QH1,
    REG_CODE_QI1,
    REG_CODE_QJ1,
    REG_CODE_QK1,
    REG_CODE_QL1,
    REG_CODE_QM1,
    REG_CODE_QN1,
    REG_CODE_QO1,
    REG_CODE_QP1,

    REG_CODE_QA2,
    REG_CODE_QB2,
    REG_CODE_QC2,
    REG_CODE_QD2,
    REG_CODE_QE2,
    REG_CODE_QF2,
    REG_CODE_QG2,
    REG_CODE_QH2,
    REG_CODE_QI2,
    REG_CODE_QJ2,
    REG_CODE_QK2,
    REG_CODE_QL2,
    REG_CODE_QM2,
    REG_CODE_QN2,
    REG_CODE_QO2,
    REG_CODE_QP2,

    REG_CODE_QA3,
    REG_CODE_QB3,
    REG_CODE_QC3,
    REG_CODE_QD3,
    REG_CODE_QE3,
    REG_CODE_QF3,
    REG_CODE_QG3,
    REG_CODE_QH3,
    REG_CODE_QI3,
    REG_CODE_QJ3,
    REG_CODE_QK3,
    REG_CODE_QL3,
    REG_CODE_QM3,
    REG_CODE_QN3,
    REG_CODE_QO3,
    REG_CODE_QP3,
} RegisterCode;

typedef struct register_t {
    char const* const name;
    RegisterCode const code;
    CommandArgType const type;
} Register;

Register const* getRegisterByName(char const* name);
Register const* getRegisterByCode(RegisterCode code);
