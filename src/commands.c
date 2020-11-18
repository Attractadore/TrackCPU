#include "commands.h"
#include "command_names.h"
#include "register_names.h"

#include <string.h>
#include <assert.h>

#define ARR_SIZE(arr) sizeof(arr) / sizeof(*arr)

static Command const commandArr[] = {
    {CMD_HALT_NAME, CMD_HALT_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_CALL_NAME, CMD_CALL_CODE, 1, {CMD_ARG_TYPE_LABEL}},
    {CMD_RETURN_NAME, CMD_RETURN_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_JUMP_NAME, CMD_JUMP_CODE, 1, {CMD_ARG_TYPE_LABEL}},
    {CMD_JUMPNE_NAME, CMD_JUMPNE_CODE, 1, {CMD_ARG_TYPE_LABEL}},
    {CMD_JUMPEQ_NAME, CMD_JUMPEQ_CODE, 1, {CMD_ARG_TYPE_LABEL}},
    {CMD_JUMPLE_NAME, CMD_JUMPLE_CODE, 1, {CMD_ARG_TYPE_LABEL}},

    {CMD_PUSHW_NAME, CMD_PUSHW_CODE, 1, {CMD_ARG_TYPE_REGISTER_64}},
    {CMD_PUSHULONG_NAME, CMD_PUSHULONG_CODE, 1, {CMD_ARG_TYPE_ULONG}},
    {CMD_PUSHDBL_NAME, CMD_PUSHDBL_CODE, 1, {CMD_ARG_TYPE_DOUBLE}},

    {CMD_POPQ_NAME, CMD_POPQ_CODE, 1, {CMD_ARG_TYPE_REGISTER_16}},
    {CMD_POPW_NAME, CMD_POPW_CODE, 1, {CMD_ARG_TYPE_REGISTER_64}},
    {CMD_POPSTK_NAME, CMD_POPSTK_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_TOPW_NAME, CMD_TOPW_CODE, 1, {CMD_ARG_TYPE_REGISTER_64}},

    {CMD_CMPULONG_NAME, CMD_CMPULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_ADDULONG_NAME, CMD_ADDULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_ADDDBL_NAME, CMD_ADDDBL_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_SUBULONG_NAME, CMD_SUBULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_SUBDBL_NAME, CMD_SUBDBL_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_MULULONG_NAME, CMD_MULULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_MULDBL_NAME, CMD_MULDBL_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_DIVULONG_NAME, CMD_DIVULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_DIVDBL_NAME, CMD_DIVDBL_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_MODULONG_NAME, CMD_MODULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_SQRTDBL_NAME, CMD_SQRTDBL_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_STOREWR_NAME, CMD_STOREWR_CODE, 2, {CMD_ARG_TYPE_REGISTER_64, CMD_ARG_TYPE_REGISTER_16}},

    {CMD_LOADWR_NAME, CMD_LOADWR_CODE, 2, {CMD_ARG_TYPE_REGISTER_64, CMD_ARG_TYPE_REGISTER_16}},

    {CMD_PRINTULONG_NAME, CMD_PRINTULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_PRINTDBL_NAME, CMD_PRINTDBL_CODE, 0, {CMD_ARG_TYPE_NONE}},

    {CMD_SCANULONG_NAME, CMD_SCANULONG_CODE, 0, {CMD_ARG_TYPE_NONE}},
    {CMD_SCANDBL_NAME, CMD_SCANDBL_CODE, 0, {CMD_ARG_TYPE_NONE}},
};

static Register const registerArr[] = {
    {REG_NAME_WA0, REG_CODE_WA0, CMD_ARG_TYPE_REGISTER_64},
    {REG_NAME_WB0, REG_CODE_WB0, CMD_ARG_TYPE_REGISTER_64},
    {REG_NAME_WC0, REG_CODE_WC0, CMD_ARG_TYPE_REGISTER_64},
    {REG_NAME_WD0, REG_CODE_WD0, CMD_ARG_TYPE_REGISTER_64},
    {REG_NAME_WE0, REG_CODE_WE0, CMD_ARG_TYPE_REGISTER_64},
    {REG_NAME_WF0, REG_CODE_WF0, CMD_ARG_TYPE_REGISTER_64},
    {REG_NAME_QC0, REG_CODE_QC0, CMD_ARG_TYPE_REGISTER_16},
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
