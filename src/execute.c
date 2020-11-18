#define STACK_ELEM_TYPE double
#include "commands.h"
#include "stack_generic.h"
#include "util.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef enum execute_error_e {
    EXECUTE_OK,
    EXECUTE_TERMINATE,
    EXECUTE_INVALID_COMMAND,
    EXECUTE_INVALID_COMMAND_ARGUMENT_COUNT,
    EXECUTE_CORRUPTION,
    EXECUTE_STACK_UNDERFLOW,
    EXECUTE_INPUT_FAILURE,
    EXECUTE_INTERNAL_FAILURE,
} ExecuteError;

char const* getExecuteErrorString(ExecuteError e) {
    switch (e) {
        case EXECUTE_OK:
            return "No execution error";
        case EXECUTE_TERMINATE:
            return "Binary contains halt command";
        case EXECUTE_INVALID_COMMAND:
            return "Binary contains invalid command";
        case EXECUTE_INVALID_COMMAND_ARGUMENT_COUNT:
            return "Binary contains command with invalid argument count";
        case EXECUTE_CORRUPTION:
            return "Binary is corrupted";
        case EXECUTE_STACK_UNDERFLOW:
            return "Binary contains command sequence that has led to a stack underflow";
        case EXECUTE_INTERNAL_FAILURE:
            return "Runtime error";
        default:
            return "Unknown execution error";
    }
    assert(!"No error string returned");
}

typedef double (*OneArgFunc)(double);
typedef double (*TwoArgFunc)(double, double);

ExecuteError executeTwoArgCommand(Stack_double* valueStack, TwoArgFunc func) {
    double arg1, arg2;
    arg1 = StackPop_double(valueStack);
    arg2 = StackPop_double(valueStack);
    if (StackGetError_double(valueStack) == STACK_OPERATION_ERROR) {
        return EXECUTE_STACK_UNDERFLOW;
    }
    if (StackGetError_double(valueStack) != STACK_OK) {
        return EXECUTE_INTERNAL_FAILURE;
    }
    StackPush_double(valueStack, func(arg1, arg2));
    return EXECUTE_OK;
}

ExecuteError executeOneArgCommand(Stack_double* valueStack, OneArgFunc func) {
    double arg;
    arg = StackPop_double(valueStack);
    if (StackGetError_double(valueStack) == STACK_OPERATION_ERROR) {
        return EXECUTE_STACK_UNDERFLOW;
    }
    if (StackGetError_double(valueStack) != STACK_OK) {
        return EXECUTE_INTERNAL_FAILURE;
    }
    StackPush_double(valueStack, func(arg));
    return EXECUTE_OK;
}

double CPUAdd(double v1, double v2) {
    return v1 + v2;
}

double CPUSub(double v1, double v2) {
    return v1 - v2;
}

double CPUMul(double v1, double v2) {
    return v1 * v2;
}

double CPUDiv(double v1, double v2) {
    return v1 / v2;
}

double CPUSqrt(double v) {
    return sqrt(v);
}

double CPUSin(double v) {
    return sin(v);
}

double CPUCos(double v) {
    return cos(v);
}

ExecuteError processCommand(char const** const commandBufferP, size_t* const commandBufferSizeP, Stack_double* valueStack) {
    size_t cmdCode = CMD_INVALID_CODE;
    if (sizeof(cmdCode) > *commandBufferSizeP) {
        return EXECUTE_CORRUPTION;
    }
    memcpy(&cmdCode, *commandBufferP, sizeof(cmdCode));
    *commandBufferP += sizeof(cmdCode);
    *commandBufferSizeP -= sizeof(cmdCode);

    switch (cmdCode) {
        case CMD_PUSH_CODE:
            {
            double cmdArg;
            if (sizeof(cmdArg) > *commandBufferSizeP) {
                return EXECUTE_CORRUPTION;
            }
            memcpy(&cmdArg, *commandBufferP, sizeof(cmdArg));
            *commandBufferP += sizeof(cmdArg);
            *commandBufferSizeP -= sizeof(cmdCode);
            StackPush_double(valueStack, cmdArg);
            return EXECUTE_OK;
            }
        case CMD_ADD_CODE:
            return executeTwoArgCommand(valueStack, CPUAdd);
        case CMD_SUB_CODE:
            return executeTwoArgCommand(valueStack, CPUSub);
        case CMD_MUL_CODE:
            return executeTwoArgCommand(valueStack, CPUMul);
        case CMD_DIV_CODE:
            return executeTwoArgCommand(valueStack, CPUDiv);
        case CMD_SQRT_CODE:
            return executeOneArgCommand(valueStack, CPUSqrt);
        case CMD_SIN_CODE:
            return executeOneArgCommand(valueStack, CPUSin);
        case CMD_COS_CODE:
            return executeOneArgCommand(valueStack, CPUCos);
        case CMD_PRINT_CODE:
            {
            double arg = StackTop_double(valueStack);
            if (StackGetError_double(valueStack) == STACK_OPERATION_ERROR) {
                return EXECUTE_STACK_UNDERFLOW;
            }
            if (StackGetError_double(valueStack) != STACK_OK) {
                return EXECUTE_INTERNAL_FAILURE;
            }
            printf("%g\n", arg);
            return EXECUTE_OK;
            }
        case CMD_SCAN_CODE:
            {
            double arg;
            if (scanf("%lg", &arg) != 0) {
                return EXECUTE_INPUT_FAILURE;
            }
            StackPush_double(valueStack, arg);
            if (StackGetError_double(valueStack) != STACK_OK) {
                return EXECUTE_INTERNAL_FAILURE;
            }
            return EXECUTE_OK;
            }
        case CMD_HALT_CODE:
            return EXECUTE_TERMINATE;
        default:
            return EXECUTE_INVALID_COMMAND;
    }
    assert(!"processCommand did not return res");
}

ExecuteError execute(char const* commandBuffer, size_t commandBufferSize, Stack_double* const valueStack) {
    assert(commandBuffer);
    assert(valueStack);

    while (commandBufferSize) {
        ExecuteError res = processCommand(&commandBuffer, &commandBufferSize, valueStack);
        if (res != EXECUTE_OK) {
            return res;
        }
    }

    return EXECUTE_OK;
}

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        printf("Usage: %s input_file\n", (argc) ? (argv[0]) : "program_name");
        return -1;
    }
    char const* const inputFileName = argv[1];
    FILE* inputFile = fopen(inputFileName, "rb");
    if (!inputFile) {
        printf("Failed to open %s for reading\n", inputFileName);
        return -1;
    }
    char* commandBuffer = NULL;
    size_t commandBufferSize = readBuffer(inputFile, &commandBuffer);
    fclose(inputFile);
    if (!commandBufferSize) {
        printf("Failed to load executable %s\n", inputFileName);
        free(commandBuffer);
        return -1;
    }

    Stack_double* valueStack = StackAllocate_double();
    if (!valueStack) {
        printf("Failed to allocate execution stack\n");
        free(commandBuffer);
        return -1;
    }

    ExecuteError res = execute(commandBuffer, commandBufferSize, valueStack);
    free(commandBuffer);
    StackFree_double(valueStack);
    if (res != EXECUTE_OK) {
        printf("EXECUTION ERROR: %s\n", getExecuteErrorString(res));
        return -1;
    }

    return 0;
}
