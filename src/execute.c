#include "CPUTypes_Gen.h"

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

void storeValue(CPUMemory* mem, CPUData value, CPUAddr addr) {
    assert(mem);
    adjustMem(mem, addr);
    mem->data[addr - mem->reserveBegin] = value;
}

CPUData loadValue(CPUMemory* mem, CPUAddr addr) {
    assert(mem);
    adjustMem(mem, addr);
    return mem->data[addr - mem->reserveBegin];
}

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
    CommandCode cmdCode = CMD_INVALID_CODE;
    READ_ADVANCE(commandBuffer, commandBufferSize, ip, cmdCode, EXECUTE_INVALID_COMMAND);
    *ipp = ip;

#define CAT_HELPER(arg1, arg2) arg1##arg2
#define CAT(arg1, arg2) CAT_HELPER(arg1, arg2)
#define TO_CPUDATA(type, argName) \
    (CPUData) { CAT(.as, type) = argName }
#define TO_TYPE(res, type) res CAT(.as, type)

#define HALT() \
    return EXECUTE_TERMINATE

#define DATA_PUSH(type, argName)                              \
    StackPush_CPUData(valueStack, TO_CPUDATA(type, argName)); \
    CPU_CHECK_DATA_PUSH(valueStack)

#define DATA_POP(type, argName)                                 \
    type argName = TO_TYPE(StackPop_CPUData(valueStack), type); \
    CPU_CHECK_DATA_POP(valueStack)

#define DATA_TOP(type, argName)                                 \
    type argName = TO_TYPE(StackTop_CPUData(valueStack), type); \
    CPU_CHECK_DATA_TOP(valueStack)

#define IP_PUSH()                     \
    StackPush_CPUAddr(callStack, ip); \
    CPU_CHECK_ADDR_PUSH(callStack)

#define IP_POP()                      \
    ip = StackPop_CPUAddr(callStack); \
    CPU_CHECK_ADDR_POP(callStack)

#define READ(type, argName) \
    type argName = 0;       \
    READ_ADVANCE(commandBuffer, commandBufferSize, ip, argName, EXECUTE_CORRUPTION)

#define PRINT(fmt, value) \
    CPU_PRINT(fmt, value)

#define SCAN(fmt, type, argName) \
    type argName = 0;            \
    CPU_SCAN(fmt, &argName)

#define LOAD_VALUE(addr, type, argName)                 \
    type argName = TO_TYPE(loadValue(mem, addr), type); \
    CPU_CHECK_MEMORY(mem)

#define STORE_VALUE(addr, type, argName)              \
    storeValue(mem, TO_CPUDATA(type, argName), addr); \
    CPU_CHECK_MEMORY(mem)

#define UNKNOWN_COMMAND() \
    return EXECUTE_INVALID_COMMAND

#define READ_REGISTER(regCode, type, argName) \
    CPU_CHECK_REGISTER(regCode);              \
    type argName = TO_TYPE(registers[regCode], type)

#define WRITE_REGISTER(regCode, type, argName) \
    CPU_CHECK_REGISTER(regCode);               \
    TO_TYPE(registers[regCode], type) = argName

#define IP ip
#define CMD_CODE cmdCode

#include "CPUExec_Gen.c"

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
