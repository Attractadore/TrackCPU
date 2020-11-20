#include "commands.h"
#include "command_names.h"
#include "register_names.h"

#include <assert.h>
#include <string.h>

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))

static Command const commandArr[] = {
    {CMD_HALT_NAME, CMD_HALT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_CALL_NAME, CMD_CALL_CODE, 1, {CMD_ARG_TYPE_LABEL}},
    {CMD_RETURN_NAME, CMD_RETURN_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_JUMP_NAME, CMD_JUMP_CODE, 1, {CMD_ARG_TYPE_LABEL}},
    {CMD_JUMP_TRUE_NAME, CMD_JUMP_TRUE_CODE, 1, {CMD_ARG_TYPE_LABEL}},
    {CMD_JUMP_FALSE_NAME, CMD_JUMP_FALSE_CODE, 1, {CMD_ARG_TYPE_LABEL}},

    {CMD_PUSH_NAME, CMD_PUSH_CODE, 1, {CMD_ARG_TYPE_REGISTER}},
    {CMD_PUSH_INT_NAME, CMD_PUSH_INT_CODE, 1, {CMD_ARG_TYPE_INT}},
    {CMD_PUSH_UINT_NAME, CMD_PUSH_UINT_CODE, 1, {CMD_ARG_TYPE_UINT}},
    {CMD_PUSH_FLOAT_NAME, CMD_PUSH_FLOAT_CODE, 1, {CMD_ARG_TYPE_FLOAT}},

    {CMD_POP_NAME, CMD_POP_CODE, 1, {CMD_ARG_TYPE_REGISTER}},
    {CMD_POPS_NAME, CMD_POPS_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_TOP_NAME, CMD_TOP_CODE, 1, {CMD_ARG_TYPE_REGISTER}},

    {CMD_EQUAL_INT_NAME, CMD_EQUAL_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_EQUAL_UINT_NAME, CMD_EQUAL_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_EQUAL_FLOAT_NAME, CMD_EQUAL_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_LESS_INT_NAME, CMD_LESS_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_LESS_UINT_NAME, CMD_LESS_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_LESS_FLOAT_NAME, CMD_LESS_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_LESS_EQUAL_INT_NAME, CMD_LESS_EQUAL_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_LESS_EQUAL_UINT_NAME, CMD_LESS_EQUAL_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_LESS_EQUAL_FLOAT_NAME, CMD_LESS_EQUAL_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_AND_NAME, CMD_AND_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_OR_NAME, CMD_OR_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_XOR_NAME, CMD_XOR_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_NOT_NAME, CMD_NOT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_LEFT_SHIFT_NAME, CMD_LEFT_SHIFT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_RIGHT_SHIFT_NAME, CMD_RIGHT_SHIFT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_RIGHT_SHIFT_ARITHMETIC_NAME, CMD_RIGHT_SHIFT_ARITHMETIC_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_ADD_INT_NAME, CMD_ADD_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_ADD_UINT_NAME, CMD_ADD_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_ADD_FLOAT_NAME, CMD_ADD_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_SUB_INT_NAME, CMD_SUB_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_SUB_UINT_NAME, CMD_SUB_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_SUB_FLOAT_NAME, CMD_SUB_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_MUL_INT_NAME, CMD_MUL_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_MUL_UINT_NAME, CMD_MUL_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_MUL_FLOAT_NAME, CMD_MUL_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_DIV_INT_NAME, CMD_DIV_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_DIV_UINT_NAME, CMD_DIV_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_DIV_FLOAT_NAME, CMD_DIV_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_MOD_INT_NAME, CMD_MOD_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_MOD_UINT_NAME, CMD_MOD_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_SQRT_NAME, CMD_SQRT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_PRINT_INT_NAME, CMD_PRINT_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_PRINT_UINT_NAME, CMD_PRINT_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_PRINT_FLOAT_NAME, CMD_PRINT_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_SCAN_INT_NAME, CMD_SCAN_INT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_SCAN_UINT_NAME, CMD_SCAN_UINT_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_SCAN_FLOAT_NAME, CMD_SCAN_FLOAT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_LOAD_NAME, CMD_LOAD_CODE, 1, {CMD_ARG_TYPE_REGISTER}},
    {CMD_STORE_NAME, CMD_STORE_CODE, 1, {CMD_ARG_TYPE_REGISTER}},
    {CMD_MOVE_NAME, CMD_MOVE_CODE, 1, {CMD_ARG_TYPE_REGISTER}},
};

static Register const registerArr[] = {
    {REG_NAME_RA, REG_CODE_RA},
    {REG_NAME_RB, REG_CODE_RB},
    {REG_NAME_RC, REG_CODE_RC},
    {REG_NAME_RD, REG_CODE_RD},
    {REG_NAME_RE, REG_CODE_RE},
    {REG_NAME_RF, REG_CODE_RF},
    {REG_NAME_RG, REG_CODE_RG},
    {REG_NAME_RH, REG_CODE_RH},
    {REG_NAME_RI, REG_CODE_RI},
    {REG_NAME_RJ, REG_CODE_RJ},
    {REG_NAME_RK, REG_CODE_RK},
    {REG_NAME_RL, REG_CODE_RL},
    {REG_NAME_RM, REG_CODE_RM},
    {REG_NAME_RN, REG_CODE_RN},
    {REG_NAME_RO, REG_CODE_RO},
    {REG_NAME_RP, REG_CODE_RP},
};

Command const* getCommandByName(char const* str) {
    assert(str);

    for (size_t i = 0; i < ARR_SIZE(commandArr); i++) {
        if (strcmp(str, commandArr[i].name) == 0) {
            return commandArr + i;
        }
    }
    return NULL;
}

Command const* getCommandByCode(const CommandCode code) {
    for (size_t i = 0; i < ARR_SIZE(commandArr); i++) {
        if (commandArr[i].code == code) {
            return commandArr + i;
        }
    }
    return NULL;
}

Register const* getRegisterByName(char const* name) {
    assert(name);

    for (size_t i = 0; i < ARR_SIZE(registerArr); i++) {
        if (strcmp(name, registerArr[i].name) == 0) {
            return registerArr + i;
        }
    }
    return NULL;
}

Register const* getRegisterByCode(RegisterCode code) {
    for (size_t i = 0; i < ARR_SIZE(registerArr); i++) {
        if (registerArr[i].code == code) {
            return registerArr + i;
        }
    }
    return NULL;
}
