#include "CPUTypes.h"

#define STACK_ELEM_TYPE CPUData
#include "stack_generic.h"
#define STACK_ELEM_TYPE CPUAddr
#include "stack_generic.h"

#include "commands.h"
#include "util.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    CPU_EXEC_SAFE,
    CPU_EXEC_FAST,
    CPU_EXEC_UNSAFE,
} CPU_EXECUTION_MODE;

#define UNKNOW_EXEC_MODE                             \
    do {                                             \
        assert(!"CPUExecMode set to unknown value"); \
    } while (0)

CPU_EXECUTION_MODE CPUExecMode = CPU_EXEC_SAFE;

typedef enum {
    EXECUTE_OK,
    EXECUTE_TERMINATE,
    EXECUTE_INVALID_COMMAND,
    EXECUTE_INVALID_COMMAND_ARGUMENT_COUNT,
    EXECUTE_INVALID_REGISTER,
    EXECUTE_CORRUPTION,
    EXECUTE_OOM,
    EXECUTE_VALUE_STACK_UNDERFLOW,
    EXECUTE_CALL_STACK_UNDERFLOW,
    EXECUTE_INPUT_FAILURE,
    EXECUTE_OUTPUT_FAILURE,
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
        case EXECUTE_INVALID_REGISTER:
            return "Binary contains command with invalid register argument";
        case EXECUTE_CORRUPTION:
            return "Binary is corrupted";
        case EXECUTE_OOM:
            return "Memory exhausted during execution";
        case EXECUTE_VALUE_STACK_UNDERFLOW:
            return "Pop from empty stack value stack requested";
        case EXECUTE_CALL_STACK_UNDERFLOW:
            return "Return from base frame requested";
        case EXECUTE_INTERNAL_FAILURE:
            return "Runtime error";
        case EXECUTE_INPUT_FAILURE:
            return "Failed to get input from user";
        case EXECUTE_OUTPUT_FAILURE:
            return "Failed to get write output";
        default:
            return "Unknown execution error";
    }
    assert(!"No error string returned");
}

enum {
    CPU_MEM_CHUNK_SIZE = 256
};

typedef struct {
    CPUData* data;
    size_t reserveBegin;
    size_t reserveEnd;
} CPUMemory;

CPUMemory* initMemory() {
    CPUMemory* mem = calloc(1, sizeof(*mem));
    return mem;
}

void freeMemory(CPUMemory* mem) {
    if (mem) {
        free(mem->data);
        free(mem);
    }
}

size_t calcReserveBegin(size_t addr) {
    return addr / CPU_MEM_CHUNK_SIZE * CPU_MEM_CHUNK_SIZE;
}

size_t calcReserveEnd(size_t addr) {
    return calcReserveBegin(addr) + CPU_MEM_CHUNK_SIZE;
}

void initMem(CPUMemory* mem, size_t addr) {
    assert(mem);

    size_t newReserveBegin = calcReserveBegin(addr);
    size_t newReserveEnd = calcReserveEnd(addr);

    CPUData* newData = calloc(newReserveEnd - newReserveBegin, sizeof(*newData));
    if (!newData) {
        return;
    }

    mem->reserveBegin = newReserveBegin;
    mem->reserveEnd = newReserveEnd;
    mem->data = newData;
}

void adjustBegin(CPUMemory* mem, size_t addr) {
    assert(mem);
    assert(mem->data);
    assert(addr < mem->reserveBegin);

    size_t newReserveBegin = calcReserveBegin(addr);
    size_t size = mem->reserveEnd - mem->reserveBegin;
    size_t newSize = mem->reserveEnd - newReserveBegin;
    CPUData* newData = realloc(mem->data, newSize * sizeof(*newData));
    if (!newData) {
        return;
    }
    memmove(newData + (newSize - size), newData, size * sizeof(*newData));
    memset(newData, 0, (newSize - size) * sizeof(*newData));
    mem->reserveBegin = newReserveBegin;
    mem->data = newData;
}

void adjustEnd(CPUMemory* mem, size_t addr) {
    assert(mem);
    assert(mem->data);
    assert(addr >= mem->reserveEnd);

    size_t newReserveEnd = calcReserveEnd(addr);
    size_t size = mem->reserveEnd - mem->reserveBegin;
    size_t newSize = newReserveEnd - mem->reserveBegin;
    CPUData* newData = realloc(mem->data, newSize * sizeof(*newData));
    if (!newData) {
        return;
    }
    memset(newData + size, 0, (newSize - size) * sizeof(*newData));
    mem->reserveEnd = newReserveEnd;
    mem->data = newData;
}

void adjustMem(CPUMemory* mem, size_t addr) {
    assert(mem);
    if (!mem->data) {
        initMem(mem, addr);
        return;
    }
    if (addr < mem->reserveBegin) {
        adjustBegin(mem, addr);
        return;
    }
    if (addr >= mem->reserveEnd) {
        adjustEnd(mem, addr);
        return;
    }
}

void storeValue(CPUMemory* mem, CPUData value, size_t addr) {
    assert(mem);
    adjustMem(mem, addr);
    mem->data[addr - mem->reserveBegin] = value;
}

CPUData loadValue(CPUMemory* mem, size_t addr) {
    assert(mem);
    adjustMem(mem, addr);
    return mem->data[addr - mem->reserveBegin];
}

CPUData CPUDataFromCPUFloat(CPUFloat v) {
    return *(CPUData*) &v;
}

CPUFloat CPUFloatFromCPUData(CPUData v) {
    return *(CPUFloat*) &v;
}

CPUInt CPUEqualI(CPUInt v1, CPUInt v2) {
    return v1 == v2;
}

CPUUInt CPUEqualUI(CPUUInt v1, CPUUInt v2) {
    return v1 == v2;
}

CPUFloat CPUEqualF(CPUFloat v1, CPUFloat v2) {
    return v1 == v2;
}

CPUInt CPULessI(CPUInt v1, CPUInt v2) {
    return v1 < v2;
}

CPUUInt CPULessUI(CPUUInt v1, CPUUInt v2) {
    return v1 < v2;
}

CPUFloat CPULessF(CPUFloat v1, CPUFloat v2) {
    return v1 < v2;
}

CPUInt CPULessEqualI(CPUInt v1, CPUInt v2) {
    return v1 <= v2;
}

CPUUInt CPULessEqualUI(CPUUInt v1, CPUUInt v2) {
    return v1 <= v2;
}

CPUFloat CPULessEqualF(CPUFloat v1, CPUFloat v2) {
    return v1 <= v2;
}

CPUInt CPUAddI(CPUInt v1, CPUInt v2) {
    return v1 + v2;
}

CPUUInt CPUAddUI(CPUUInt v1, CPUUInt v2) {
    return v1 + v2;
}

CPUFloat CPUAddF(CPUFloat v1, CPUFloat v2) {
    return v1 + v2;
}

CPUInt CPUSubI(CPUInt v1, CPUInt v2) {
    return v1 - v2;
}

CPUUInt CPUSubUI(CPUUInt v1, CPUUInt v2) {
    return v1 - v2;
}

CPUFloat CPUSubF(CPUFloat v1, CPUFloat v2) {
    return v1 - v2;
}

CPUInt CPUMulI(CPUInt v1, CPUInt v2) {
    return v1 * v2;
}

CPUUInt CPUMulUI(CPUUInt v1, CPUUInt v2) {
    return v1 * v2;
}

CPUFloat CPUMulF(CPUFloat v1, CPUFloat v2) {
    return v1 * v2;
}

CPUInt CPUDivI(CPUInt v1, CPUInt v2) {
    return v1 / v2;
}

CPUUInt CPUDivUI(CPUUInt v1, CPUUInt v2) {
    return v1 / v2;
}

CPUFloat CPUDivF(CPUFloat v1, CPUFloat v2) {
    return v1 / v2;
}

CPUInt CPUModI(CPUInt v1, CPUInt v2) {
    return v1 % v2;
}

CPUUInt CPUModUI(CPUUInt v1, CPUUInt v2) {
    return v1 % v2;
}

CPUFloat CPUSqrt(CPUFloat v) {
    return sqrt(v);
}

CPUUInt CPUAnd(CPUUInt v1, CPUUInt v2) {
    return v1 & v2;
}

CPUUInt CPUOr(CPUUInt v1, CPUUInt v2) {
    return v1 | v2;
}

CPUUInt CPUXor(CPUUInt v1, CPUUInt v2) {
    return v1 ^ v2;
}

CPUUInt CPUNot(CPUUInt v) {
    return ~v;
}

CPUUInt CPULeftShift(CPUUInt v1, CPUUInt v2) {
    return v1 << v2;
}

CPUUInt CPURightShift(CPUUInt v1, CPUUInt v2) {
    return v1 >> v2;
}

CPUUInt CPURightShiftArithmetic(CPUUInt v1, CPUUInt v2) {
    return ((CPUInt) v1) >> v2;
}

typedef CPUInt (*oneIntFunc)(CPUInt);
typedef CPUUInt (*oneUIntFunc)(CPUUInt);
typedef CPUFloat (*oneFloatFunc)(CPUFloat);
typedef CPUInt (*twoIntFunc)(CPUInt, CPUInt);
typedef CPUUInt (*twoUIntFunc)(CPUUInt, CPUUInt);
typedef CPUFloat (*twoFloatFunc)(CPUFloat, CPUFloat);

#define CPU_CHECK_COMMON_POP(stk, sge)                        \
    do {                                                      \
        switch (CPUExecMode) {                                \
            case CPU_EXEC_SAFE:                               \
                switch (sge(stk)) {                           \
                    case STACK_OK:                            \
                        break;                                \
                    case STACK_OPERATION_ERROR:               \
                        return EXECUTE_VALUE_STACK_UNDERFLOW; \
                    default:                                  \
                        return EXECUTE_INTERNAL_FAILURE;      \
                }                                             \
                break;                                        \
            case CPU_EXEC_FAST:                               \
                switch (sge(stk)) {                           \
                    case STACK_OPERATION_ERROR:               \
                        return EXECUTE_VALUE_STACK_UNDERFLOW; \
                    default:                                  \
                        break;                                \
                }                                             \
                break;                                        \
            case CPU_EXEC_UNSAFE:                             \
                break;                                        \
            default:                                          \
                UNKNOW_EXEC_MODE;                             \
        }                                                     \
    } while (0)

#define CPU_CHECK_COMMON_PUSH(stk, sge)                  \
    do {                                                 \
        switch (CPUExecMode) {                           \
            case CPU_EXEC_SAFE:                          \
                switch (sge(stk)) {                      \
                    case STACK_OK:                       \
                        break;                           \
                    default:                             \
                        return EXECUTE_INTERNAL_FAILURE; \
                }                                        \
            case CPU_EXEC_FAST:                          \
            case CPU_EXEC_UNSAFE:                        \
                break;                                   \
            default:                                     \
                UNKNOW_EXEC_MODE;                        \
        }                                                \
    } while (0)

#define CPU_CHECK_DATA_POP(stk) CPU_CHECK_COMMON_POP(stk, StackGetError_CPUData)
#define CPU_CHECK_DATA_TOP(stk) CPU_CHECK_DATA_POP(stk)
#define CPU_CHECK_DATA_PUSH(stk) CPU_CHECK_COMMON_PUSH(stk, StackGetError_CPUData)

#define CPU_CHECK_ADDR_POP(stk) CPU_CHECK_COMMON_POP(stk, StackGetError_CPUAddr)
#define CPU_CHECK_ADDR_TOP(stk) CPU_CHECK_ADDR_POP(stk)
#define CPU_CHECK_ADDR_PUSH(stk) CPU_CHECK_COMMON_PUSH(stk, StackGetError_CPUAddr)

ExecuteError stackApplyOneIntFunc(Stack_CPUData* valueStack, oneIntFunc f) {
    assert(valueStack);
    CPUData topValue = StackPop_CPUData(valueStack);
    CPU_CHECK_DATA_POP(valueStack);
    StackPush_CPUData(valueStack, f(topValue));
    CPU_CHECK_DATA_PUSH(valueStack);
    return EXECUTE_OK;
}

ExecuteError stackApplyOneUIntFunc(Stack_CPUData* valueStack, oneUIntFunc f) {
    assert(valueStack);
    CPUData topValue = StackPop_CPUData(valueStack);
    CPU_CHECK_DATA_POP(valueStack);
    StackPush_CPUData(valueStack, f(topValue));
    CPU_CHECK_DATA_PUSH(valueStack);
    return EXECUTE_OK;
}

ExecuteError stackApplyOneFloatFunc(Stack_CPUData* valueStack, oneFloatFunc f) {
    assert(valueStack);
    CPUFloat topValue = CPUFloatFromCPUData(StackPop_CPUData(valueStack));
    CPU_CHECK_DATA_POP(valueStack);
    StackPush_CPUData(valueStack, CPUDataFromCPUFloat(f(topValue)));
    CPU_CHECK_DATA_PUSH(valueStack);
    return EXECUTE_OK;
}

ExecuteError stackApplyTwoIntFunc(Stack_CPUData* valueStack, twoIntFunc f) {
    assert(valueStack);
    CPUData value1 = StackPop_CPUData(valueStack);
    CPU_CHECK_DATA_POP(valueStack);
    CPUData value2 = StackPop_CPUData(valueStack);
    CPU_CHECK_DATA_POP(valueStack);
    StackPush_CPUData(valueStack, f(value1, value2));
    CPU_CHECK_DATA_PUSH(valueStack);
    return EXECUTE_OK;
}

ExecuteError stackApplyTwoUIntFunc(Stack_CPUData* valueStack, twoUIntFunc f) {
    assert(valueStack);
    CPUData value1 = StackPop_CPUData(valueStack);
    CPU_CHECK_DATA_POP(valueStack);
    CPUData value2 = StackPop_CPUData(valueStack);
    CPU_CHECK_DATA_POP(valueStack);
    StackPush_CPUData(valueStack, f(value1, value2));
    CPU_CHECK_DATA_PUSH(valueStack);
    return EXECUTE_OK;
}

ExecuteError stackApplyTwoFloatFunc(Stack_CPUData* valueStack, twoFloatFunc f) {
    assert(valueStack);
    CPUFloat value1 = CPUFloatFromCPUData(StackPop_CPUData(valueStack));
    CPU_CHECK_DATA_POP(valueStack);
    CPUFloat value2 = CPUFloatFromCPUData(StackPop_CPUData(valueStack));
    CPU_CHECK_DATA_POP(valueStack);
    StackPush_CPUData(valueStack, CPUDataFromCPUFloat(f(value1, value2)));
    CPU_CHECK_DATA_PUSH(valueStack);
    return EXECUTE_OK;
}

#define READ_ADVANCE(commandBuffer, commandBufferSize, ip, data, retval) \
    do {                                                                 \
        if (ip + sizeof(data) > commandBufferSize) {                     \
            return (retval);                                             \
        }                                                                \
        memcpy(&(data), commandBuffer + ip, sizeof(data));               \
        (ip) += sizeof(data);                                            \
    } while (0)

#define CPU_PRINT(fmt, arg)                        \
    do {                                           \
        switch (CPUExecMode) {                     \
            case CPU_EXEC_SAFE: {                  \
                bool bFail = printf(fmt, arg) < 0; \
                if (bFail) {                       \
                    return EXECUTE_OUTPUT_FAILURE; \
                }                                  \
            } break;                               \
            case CPU_EXEC_FAST:                    \
            case CPU_EXEC_UNSAFE:                  \
                printf(fmt, arg);                  \
                break;                             \
            default:                               \
                UNKNOW_EXEC_MODE;                  \
        }                                          \
    } while (0)

#define CPU_SCAN(fmt, argp)                         \
    do {                                            \
        switch (CPUExecMode) {                      \
            case CPU_EXEC_SAFE:                     \
            case CPU_EXEC_FAST: {                   \
                bool bFail = scanf(fmt, argp) != 1; \
                if (bFail) {                        \
                    return EXECUTE_INPUT_FAILURE;   \
                }                                   \
            } break;                                \
            case CPU_EXEC_UNSAFE:                   \
                scanf(fmt, argp);                   \
                break;                              \
            default:                                \
                UNKNOW_EXEC_MODE;                   \
        }                                           \
    } while (0)

#define CPU_CHECK_REGISTER(reg)                       \
    do {                                              \
        switch (CPUExecMode) {                        \
            case CPU_EXEC_SAFE:                       \
            case CPU_EXEC_FAST: {                     \
                bool bInvalid = reg >= NUM_REGISTERS; \
                if (bInvalid) {                       \
                    return EXECUTE_INVALID_REGISTER;  \
                }                                     \
            } break;                                  \
            case CPU_EXEC_UNSAFE:                     \
                break;                                \
            default:                                  \
                UNKNOW_EXEC_MODE;                     \
        }                                             \
    } while (0)

#define CPU_CHECK_MEMORY(mem)           \
    do {                                \
        switch (CPUExecMode) {          \
            case CPU_EXEC_SAFE:         \
            case CPU_EXEC_FAST:         \
                if (!mem->data) {       \
                    return EXECUTE_OOM; \
                }                       \
                break;                  \
            case CPU_EXEC_UNSAFE:       \
                break;                  \
            default:                    \
                UNKNOW_EXEC_MODE;       \
        }                               \
    } while (0)

ExecuteError processCommand(char const* commandBuffer, size_t commandBufferSize, CPUAddr* ipp, Stack_CPUData* valueStack, Stack_CPUAddr* callStack, CPUData* registers, CPUMemory* mem) {
    assert(commandBuffer);
    assert(ipp);

    CPUAddr ip = *ipp;
    CPUCommandID cmdCode = CMD_INVALID_CODE;
    READ_ADVANCE(commandBuffer, commandBufferSize, ip, cmdCode, EXECUTE_INVALID_COMMAND);
    *ipp = ip;

    switch (cmdCode) {
        case CMD_HALT_CODE:
            return EXECUTE_TERMINATE;
        case CMD_CALL_CODE: {
            CPUAddr jumpAddr = commandBufferSize + 1;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, jumpAddr, EXECUTE_CORRUPTION);
            StackPush_CPUAddr(callStack, ip);
            CPU_CHECK_ADDR_PUSH(callStack);
            ip = jumpAddr;
        } break;
        case CMD_RETURN_CODE: {
            ip = StackPop_CPUAddr(callStack);
            CPU_CHECK_ADDR_POP(callStack);
        } break;

        case CMD_JUMP_CODE: {
            CPUAddr jumpAddr = commandBufferSize + 1;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, jumpAddr, EXECUTE_CORRUPTION);
            ip = jumpAddr;
        } break;
        case CMD_JUMP_TRUE_CODE: {
            CPUAddr jumpAddr = commandBufferSize + 1;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, jumpAddr, EXECUTE_CORRUPTION);
            if (StackPop_CPUData(valueStack)) {
                ip = jumpAddr;
            }
        } break;
        case CMD_JUMP_FALSE_CODE: {
            CPUAddr jumpAddr = commandBufferSize + 1;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, jumpAddr, EXECUTE_CORRUPTION);
            if (!StackPop_CPUData(valueStack)) {
                ip = jumpAddr;
            }
        } break;

        case CMD_PUSH_CODE: {
            CPURegisterID pushFromReg = REG_CODE_INVALID;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, pushFromReg, EXECUTE_CORRUPTION);
            CPU_CHECK_REGISTER(pushFromReg);
            StackPush_CPUData(valueStack, registers[pushFromReg]);
        } break;
        case CMD_PUSH_INT_CODE: {
            CPUInt inArg = 0;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, inArg, EXECUTE_CORRUPTION);
            StackPush_CPUData(valueStack, inArg);
        } break;
        case CMD_PUSH_UINT_CODE: {
            CPUUInt inArg = 0;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, inArg, EXECUTE_CORRUPTION);
            StackPush_CPUData(valueStack, inArg);
        } break;
        case CMD_PUSH_FLOAT_CODE: {
            CPUFloat inArg = NAN;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, inArg, EXECUTE_CORRUPTION);
            StackPush_CPUData(valueStack, *(CPUData*) &inArg);
        } break;

        case CMD_POP_CODE: {
            CPURegisterID popToReg = REG_CODE_INVALID;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, popToReg, EXECUTE_CORRUPTION);
            CPU_CHECK_REGISTER(popToReg);
            registers[popToReg] = StackPop_CPUData(valueStack);
        } break;
        case CMD_POPS_CODE:
            StackPop_CPUData(valueStack);
            break;

        case CMD_TOP_CODE: {
            CPURegisterID topToReg = REG_CODE_INVALID;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, topToReg, EXECUTE_CORRUPTION);
            CPU_CHECK_REGISTER(topToReg);
            registers[topToReg] = StackTop_CPUData(valueStack);
        } break;

        case CMD_EQUAL_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPUEqualI);
        } break;
        case CMD_EQUAL_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUEqualUI);
        } break;
        case CMD_EQUAL_FLOAT_CODE: {
            return stackApplyTwoFloatFunc(valueStack, CPUEqualF);
        } break;

        case CMD_LESS_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPULessI);
        } break;
        case CMD_LESS_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPULessUI);
        } break;
        case CMD_LESS_FLOAT_CODE: {
            return stackApplyTwoFloatFunc(valueStack, CPULessF);
        } break;

        case CMD_LESS_EQUAL_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPULessEqualI);
        } break;
        case CMD_LESS_EQUAL_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPULessEqualUI);
        } break;
        case CMD_LESS_EQUAL_FLOAT_CODE: {
            return stackApplyTwoFloatFunc(valueStack, CPULessEqualF);
        } break;

        case CMD_AND_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUAnd);
        } break;
        case CMD_OR_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUOr);
        } break;
        case CMD_XOR_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUXor);
        } break;
        case CMD_NOT_CODE: {
            return stackApplyOneUIntFunc(valueStack, CPUNot);
        } break;
        case CMD_LEFT_SHIFT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPULeftShift);
        } break;
        case CMD_RIGHT_SHIFT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPURightShift);
        } break;
        case CMD_RIGHT_SHIFT_ARITHMETIC_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPURightShiftArithmetic);
        } break;

        case CMD_ADD_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPUAddI);
        } break;
        case CMD_ADD_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUAddUI);
        } break;
        case CMD_ADD_FLOAT_CODE: {
            return stackApplyTwoFloatFunc(valueStack, CPUAddF);
        } break;

        case CMD_SUB_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPUSubI);
        } break;
        case CMD_SUB_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUSubUI);
        } break;
        case CMD_SUB_FLOAT_CODE: {
            return stackApplyTwoFloatFunc(valueStack, CPUSubF);
        } break;

        case CMD_MUL_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPUMulI);
        } break;
        case CMD_MUL_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUMulUI);
        } break;
        case CMD_MUL_FLOAT_CODE: {
            return stackApplyTwoFloatFunc(valueStack, CPUMulF);
        } break;

        case CMD_DIV_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPUDivI);
        } break;
        case CMD_DIV_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUDivUI);
        } break;
        case CMD_DIV_FLOAT_CODE: {
            return stackApplyTwoFloatFunc(valueStack, CPUDivF);
        } break;

        case CMD_MOD_INT_CODE: {
            return stackApplyTwoIntFunc(valueStack, CPUModI);
        } break;
        case CMD_MOD_UINT_CODE: {
            return stackApplyTwoUIntFunc(valueStack, CPUModUI);
        } break;

        case CMD_SQRT_CODE: {
            return stackApplyOneFloatFunc(valueStack, CPUSqrt);
        } break;

        case CMD_PRINT_INT_CODE: {
            CPUInt printv = StackTop_CPUData(valueStack);
            CPU_CHECK_DATA_TOP(valueStack);
            CPU_PRINT("%" CPU_PFMT_I "\n", printv);
        } break;
        case CMD_PRINT_UINT_CODE: {
            CPUUInt printv = StackTop_CPUData(valueStack);
            CPU_CHECK_DATA_TOP(valueStack);
            CPU_PRINT("%" CPU_PFMT_UI "\n", printv);
        } break;
        case CMD_PRINT_FLOAT_CODE: {
            CPUFloat printv = CPUFloatFromCPUData(StackTop_CPUData(valueStack));
            CPU_CHECK_DATA_TOP(valueStack);
            CPU_PRINT("%" CPU_PFMT_F "\n", printv);
        } break;

        case CMD_SCAN_INT_CODE: {
            CPUInt inArg = 0;
            CPU_SCAN("%" CPU_SFMT_UI, &inArg);
            StackPush_CPUData(valueStack, inArg);
            CPU_CHECK_DATA_PUSH(valueStack);
        } break;
        case CMD_SCAN_UINT_CODE: {
            CPUUInt inArg = 0;
            CPU_SCAN("%" CPU_SFMT_UI, &inArg);
            StackPush_CPUData(valueStack, inArg);
            CPU_CHECK_DATA_PUSH(valueStack);
        } break;
        case CMD_SCAN_FLOAT_CODE: {
            CPUFloat inArg = NAN;
            CPU_SCAN("%" CPU_SFMT_F, &inArg);
            StackPush_CPUData(valueStack, CPUDataFromCPUFloat(inArg));
            CPU_CHECK_DATA_PUSH(valueStack);
        } break;

        case CMD_LOAD_CODE: {
            CPURegisterID addrReg = REG_CODE_INVALID;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, addrReg, EXECUTE_CORRUPTION);
            CPU_CHECK_REGISTER(addrReg);
            CPUData loadv = loadValue(mem, registers[addrReg]);
            CPU_CHECK_MEMORY(mem);
            StackPush_CPUData(valueStack, loadv);
            CPU_CHECK_DATA_PUSH(valueStack);
        } break;
        case CMD_STORE_CODE: {
            CPURegisterID addrReg = REG_CODE_INVALID;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, addrReg, EXECUTE_CORRUPTION);
            CPU_CHECK_REGISTER(addrReg);
            CPUData storev = StackTop_CPUData(valueStack);
            CPU_CHECK_DATA_TOP(valueStack);
            storeValue(mem, storev, registers[addrReg]);
            CPU_CHECK_MEMORY(mem);
        } break;
        case CMD_MOVE_CODE: {
            CPURegisterID addrReg = REG_CODE_INVALID;
            READ_ADVANCE(commandBuffer, commandBufferSize, ip, addrReg, EXECUTE_CORRUPTION);
            CPU_CHECK_REGISTER(addrReg);
            CPUData movev = StackPop_CPUData(valueStack);
            storeValue(mem, movev, registers[addrReg]);
            CPU_CHECK_MEMORY(mem);
        } break;

        default:
            return EXECUTE_INVALID_COMMAND;
    }

    *ipp = ip;

    return EXECUTE_OK;
}

ExecuteError execute(char const* commandBuffer, size_t commandBufferSize) {
    assert(commandBuffer);

    static CPUData registers[REG_CODE_INVALID - REG_CODE_RA];

    Stack_CPUData* valueStack = StackAllocate_CPUData();
    Stack_CPUAddr* callStack = StackAllocate_CPUAddr();
    CPUMemory* mem = initMemory();
    if (!valueStack || !callStack || !mem) {
        StackFree_CPUData(valueStack);
        StackFree_CPUAddr(callStack);
        freeMemory(mem);
        return EXECUTE_OOM;
    }

    CPUAddr ip = 0;
    ExecuteError res = EXECUTE_OK;
    while (ip < commandBufferSize && res != EXECUTE_TERMINATE) {
        if (res != EXECUTE_OK) {
            return res;
        }
        res = processCommand(commandBuffer, commandBufferSize, &ip, valueStack, callStack, registers, mem);
    }

    StackFree_CPUData(valueStack);
    StackFree_CPUAddr(callStack);
    freeMemory(mem);

    return EXECUTE_OK;
}

#undef CPU_CHECK_DATA_POP
#undef CPU_CHECK_DATA_TOP
#undef CPU_CHECK_DATA_PUSH
#undef CPU_CHECK_ADDR_POP
#undef CPU_CHECK_ADDR_TOP
#undef CPU_CHECK_ADDR_PUSH
#undef CPU_CHECK_COMMON_POP
#undef CPU_CHECK_COMMON_PUSH

#undef CPU_PRINT
#undef CPU_SCAN
#undef READ_ADVANCE

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

    CPUExecMode = CPU_EXEC_SAFE;
    ExecuteError res = execute(commandBuffer, commandBufferSize);
    free(commandBuffer);

    if (res != EXECUTE_OK) {
        printf("EXECUTION ERROR: %s\n", getExecuteErrorString(res));
        return -1;
    }

    return 0;
}
