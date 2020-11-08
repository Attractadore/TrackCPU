#pragma once

#include <stddef.h>

#define CMD_HALT_NAME "halt"
#define CMD_PUSH_NAME "push"
#define CMD_ADD_NAME "add"
#define CMD_SUB_NAME "sub"
#define CMD_MUL_NAME "mul"
#define CMD_DIV_NAME "div"
#define CMD_SQRT_NAME "sqrt"
#define CMD_SIN_NAME "sin"
#define CMD_COS_NAME "cos"
#define CMD_PRINT_NAME "print"

typedef enum command_code_e {
    CMD_HALT_CODE,
    CMD_PUSH_CODE,
    CMD_ADD_CODE,
    CMD_SUB_CODE,
    CMD_MUL_CODE,
    CMD_DIV_CODE,
    CMD_SQRT_CODE,
    CMD_SIN_CODE,
    CMD_COS_CODE,
    CMD_PRINT_CODE,
    CMD_INVALID_CODE
} CommandCode ;

typedef struct command_t {
    char const* const name;
    const CommandCode code;
    const size_t numArgs;
} Command;

static Command const commandArr[] = {
    {CMD_HALT_NAME, CMD_HALT_CODE, 0},
    {CMD_PUSH_NAME, CMD_PUSH_CODE, 1},
    {CMD_ADD_NAME, CMD_ADD_CODE, 0},
    {CMD_SUB_NAME, CMD_SUB_CODE, 0},
    {CMD_MUL_NAME, CMD_MUL_CODE, 0},
    {CMD_DIV_NAME, CMD_DIV_CODE, 0},
    {CMD_SQRT_NAME, CMD_SQRT_CODE, 0},
    {CMD_SIN_NAME, CMD_SIN_CODE, 0},
    {CMD_COS_NAME, CMD_COS_CODE, 0},
    {CMD_PRINT_NAME, CMD_PRINT_CODE, 0},
};

Command const* getCommandByName(char const* str);
Command const* getCommandByCode(const CommandCode code);
