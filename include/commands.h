#pragma once

#include <assert.h>
#include <stddef.h>

typedef enum command_code_e {
    CMD_HALT_CODE,

    CMD_CALL_CODE,
    CMD_RETURN_CODE,

    CMD_JUMP_CODE,
    CMD_JUMP_TRUE_CODE,
    CMD_JUMP_FALSE_CODE,

    CMD_PUSH_CODE,
    CMD_PUSH_INT_CODE,
    CMD_PUSH_UINT_CODE,
    CMD_PUSH_FLOAT_CODE,

    CMD_POP_CODE,
    CMD_POPS_CODE,

    CMD_TOP_CODE,

    CMD_EQUAL_INT_CODE,
    CMD_EQUAL_UINT_CODE,
    CMD_EQUAL_FLOAT_CODE,

    CMD_LESS_INT_CODE,
    CMD_LESS_UINT_CODE,
    CMD_LESS_FLOAT_CODE,

    CMD_LESS_EQUAL_INT_CODE,
    CMD_LESS_EQUAL_UINT_CODE,
    CMD_LESS_EQUAL_FLOAT_CODE,

    CMD_AND_CODE,
    CMD_OR_CODE,
    CMD_XOR_CODE,
    CMD_NOT_CODE,
    CMD_LEFT_SHIFT_CODE,
    CMD_RIGHT_SHIFT_CODE,
    CMD_RIGHT_SHIFT_ARITHMETIC_CODE,

    CMD_ADD_INT_CODE,
    CMD_ADD_UINT_CODE,
    CMD_ADD_FLOAT_CODE,

    CMD_SUB_INT_CODE,
    CMD_SUB_UINT_CODE,
    CMD_SUB_FLOAT_CODE,

    CMD_MUL_INT_CODE,
    CMD_MUL_UINT_CODE,
    CMD_MUL_FLOAT_CODE,

    CMD_DIV_INT_CODE,
    CMD_DIV_UINT_CODE,
    CMD_DIV_FLOAT_CODE,

    CMD_MOD_INT_CODE,
    CMD_MOD_UINT_CODE,

    CMD_SQRT_CODE,

    CMD_SCAN_INT_CODE,
    CMD_SCAN_UINT_CODE,
    CMD_SCAN_FLOAT_CODE,

    CMD_PRINT_INT_CODE,
    CMD_PRINT_UINT_CODE,
    CMD_PRINT_FLOAT_CODE,

    CMD_LOAD_CODE,
    CMD_STORE_CODE,
    CMD_MOVE_CODE,

    CMD_INVALID_CODE
} CommandCode;

typedef double CPUFloat;
typedef long long CPUInt;
typedef unsigned long long CPUUInt;
typedef size_t CPUCommandID;
typedef size_t CPURegisterID;
static_assert(sizeof(CPUFloat) == sizeof(CPUInt) && sizeof(CPUFloat) == sizeof(CPUUInt),
              "CPU data types are of different sizes");
#define CPU_PFMT_F "lg"
#define CPU_PFMT_I "lld"
#define CPU_PFMT_UI "llu"
#define CPU_SFMT_F "lg"
#define CPU_SFMT_I "lld"
#define CPU_SFMT_UI "llu"

typedef enum command_arg_type_e {
    CMD_ARG_TYPE_NONE = 0,
    CMD_ARG_TYPE_INT,
    CMD_ARG_TYPE_UINT,
    CMD_ARG_TYPE_FLOAT,
    CMD_ARG_TYPE_REGISTER,
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
Command const* getCommandByCode(CommandCode code);

typedef enum register_code_e {
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
    REG_CODE_INVALID,
} RegisterCode;

typedef struct register_t {
    char const* const name;
    RegisterCode const code;
} Register;

Register const* getRegisterByName(char const* name);
Register const* getRegisterByCode(RegisterCode code);
