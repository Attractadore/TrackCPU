#include "commands.h"
#include "label.h"
#include "util.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_INDENT "    "

typedef enum {
    DISASSEMBLY_OK,
    DISASSEMBLY_OOM,
    DISASSEMBLY_INVALID_COMMAND,
    DISASSEMBLY_INVALID_REGISTER_ID,
    DISASSEMBLY_INVALID_JUMP_ADDRESS,
    DISASSEMBLY_CORRUPTION,
} DisassemblyError;

char const* getDisassemblyErrorString(DisassemblyError e) {
    switch (e) {
        case DISASSEMBLY_OK:
            return "No disassembly error";
        case DISASSEMBLY_OOM:
            return "Memory exhausted during disassembly";
        case DISASSEMBLY_INVALID_COMMAND:
            return "Binary contains invalid command code";
        case DISASSEMBLY_INVALID_REGISTER_ID:
            return "Binary contains command with invalid register id";
        case DISASSEMBLY_INVALID_JUMP_ADDRESS:
            return "Binary contains jump command to invalid address";
        case DISASSEMBLY_CORRUPTION:
            return "Binary is corrupted";
        default:
            return "Unknown disassembly error";
    }
    assert(!"No error returned string returned");
}

#define ADVANCE_CHECK(end, ip, step, l) \
    do {                                \
        (ip) += (step);                 \
        if ((ip) > (end)) {             \
            goto l;                     \
        }                               \
    } while (0)

#define WRITE_ADVANCE_CHECK(src, end, ip, data, l)   \
    do {                                             \
        memcpy(&(data), (src) + (ip), sizeof(data)); \
        ADVANCE_CHECK(end, ip, sizeof(data), l);     \
    } while (0)

DisassemblyError preprocessArg(CommandArgType argType, char const* inputBuffer, size_t numBytes, CPUAddr* ipp, LabelTable* labelTable) {
    assert(inputBuffer);
    assert(ipp);
    assert(labelTable);

    CPUAddr ip = *ipp;

    switch (argType) {
        case CMD_ARG_TYPE_INT:
            ADVANCE_CHECK(numBytes, ip, sizeof(CPUInt), corruption);
            break;
        case CMD_ARG_TYPE_UINT:
            ADVANCE_CHECK(numBytes, ip, sizeof(CPUUInt), corruption);
            break;
        case CMD_ARG_TYPE_FLOAT:
            ADVANCE_CHECK(numBytes, ip, sizeof(CPUFloat), corruption);
            break;
        case CMD_ARG_TYPE_REGISTER: {
            CPURegisterID cmdArg = REG_CODE_INVALID;
            WRITE_ADVANCE_CHECK(inputBuffer, numBytes, ip, cmdArg, corruption);
            Register const* reg = getRegisterByCode(cmdArg);
            if (!reg) {
                return DISASSEMBLY_INVALID_REGISTER_ID;
            }
        } break;
        case CMD_ARG_TYPE_LABEL: {
            CPUAddr cmdArg = numBytes;
            WRITE_ADVANCE_CHECK(inputBuffer, numBytes, ip, cmdArg, corruption);
            if (cmdArg > numBytes) {
                return DISASSEMBLY_INVALID_JUMP_ADDRESS;
            }
            if (addLabelUseByAddr(labelTable, cmdArg) != LABEL_ADD_OK) {
                return DISASSEMBLY_OOM;
            }
        } break;
        default:
            goto corruption;
    }

    *ipp = ip;

    return DISASSEMBLY_OK;

corruption:
    return DISASSEMBLY_CORRUPTION;
}

DisassemblyError preprocessCommand(char const* inputBuffer, size_t numBytes, CPUAddr* ipp, LabelTable* labelTable) {
    assert(inputBuffer);
    assert(ipp);
    assert(labelTable);

    CPUAddr ip = *ipp;

    CPUCommandID cmdCode = CMD_INVALID_CODE;
    WRITE_ADVANCE_CHECK(inputBuffer, numBytes, ip, cmdCode, corruption);
    Command const* const cmd = getCommandByCode(cmdCode);
    if (!cmd) {
        return DISASSEMBLY_INVALID_COMMAND;
    }

    for (size_t i = 0; i < cmd->numArgs; i++) {
        DisassemblyError res = preprocessArg(cmd->argType[i], inputBuffer, numBytes, &ip, labelTable);
        if (res != DISASSEMBLY_OK) {
            return res;
        }
    }

    *ipp = ip;

    return DISASSEMBLY_OK;

corruption:
    return DISASSEMBLY_CORRUPTION;
}

#undef ADVANCE_CHECK
#undef WRITE_ADVANCE_CHECK

DisassemblyError preprocessInput(char const* inputBuffer, size_t numBytes, LabelTable* labelTable) {
    assert(inputBuffer);
    assert(labelTable);

    CPUAddr ip = 0;
    while (ip < numBytes) {
        DisassemblyError res = preprocessCommand(inputBuffer, numBytes, &ip, labelTable);
        if (res != DISASSEMBLY_OK) {
            return res;
        }
    }
    assert(ip == numBytes);

    return DISASSEMBLY_OK;
}

#define ADVANCE(end, ip, step) \
    do {                       \
        (ip) += (step);        \
        assert((ip) <= (end)); \
    } while (0)

#define WRITE_ADVANCE(src, end, ip, data)            \
    do {                                             \
        memcpy(&(data), (src) + (ip), sizeof(data)); \
        ADVANCE(end, ip, sizeof(data));              \
    } while (0)

DisassemblyError processArg(CommandArgType argType, char const* inputBuffer, size_t numBytes, CPUAddr* ipp, LabelTable const* labelTable, FILE* outputFile) {
    assert(inputBuffer);
    assert(ipp);
    assert(labelTable);
    assert(outputFile);

    CPUAddr ip = *ipp;

    fputc(' ', outputFile);
    switch (argType) {
        case CMD_ARG_TYPE_INT: {
            CPUInt cmdArg = 0;
            WRITE_ADVANCE(inputBuffer, numBytes, ip, cmdArg);
            fprintf(outputFile, "%" CPU_PFMT_I, cmdArg);
        } break;
        case CMD_ARG_TYPE_UINT: {
            CPUUInt cmdArg = 0;
            WRITE_ADVANCE(inputBuffer, numBytes, ip, cmdArg);
            fprintf(outputFile, "%" CPU_PFMT_UI, cmdArg);
        } break;
        case CMD_ARG_TYPE_FLOAT: {
            CPUFloat cmdArg = NAN;
            WRITE_ADVANCE(inputBuffer, numBytes, ip, cmdArg);
            fprintf(outputFile, "%" CPU_PFMT_F, cmdArg);
        } break;
        case CMD_ARG_TYPE_REGISTER: {
            CPURegisterID cmdArg = 0;
            WRITE_ADVANCE(inputBuffer, numBytes, ip, cmdArg);
            Register const* reg = getRegisterByCode(cmdArg);
            assert(reg);
            fprintf(outputFile, "%s", reg->name);
        } break;
        case CMD_ARG_TYPE_LABEL: {
            CPUAddr cmdArg = numBytes + 1;
            WRITE_ADVANCE(inputBuffer, numBytes, ip, cmdArg);
            assert(cmdArg <= numBytes);
            LabelTableEntry const* entry = findLabelByAddr(labelTable, cmdArg);
            assert(entry);
            printLabel(outputFile, entry - labelTable->data);
        } break;
        default:
            assert(!"Cannot get here");
            return DISASSEMBLY_CORRUPTION;
    }

    *ipp = ip;

    return DISASSEMBLY_OK;
}

DisassemblyError processCommand(char const* inputBuffer, size_t numBytes, CPUAddr* ipp, LabelTable const* labelTable, FILE* outputFile) {
    assert(inputBuffer);
    assert(ipp);
    assert(labelTable);
    assert(outputFile);

    CPUAddr ip = *ipp;

    CPUCommandID cmdCode = CMD_INVALID_CODE;
    WRITE_ADVANCE(inputBuffer, numBytes, ip, cmdCode);
    Command const* const cmd = getCommandByCode(cmdCode);
    assert(cmd);

    fprintf(outputFile, COMMAND_INDENT "%s", cmd->name);

    assert(cmd->numArgs <= COMMAND_MAX_ARGS);
    for (size_t i = 0; i < cmd->numArgs; i++) {
        DisassemblyError res = processArg(cmd->argType[i], inputBuffer, numBytes, &ip, labelTable, outputFile);
        if (res != DISASSEMBLY_OK) {
            return res;
        }
    }
    fprintf(outputFile, "\n");

    *ipp = ip;

    return DISASSEMBLY_OK;
}

#undef ADVANCE
#undef WRITE_ADVANCE

LabelTableEntry const* processNextLabel(LabelTableEntry const* nextLabel, LabelTableEntry const* firstLabel, CPUAddr ip, FILE* outputFile) {
    assert(nextLabel);
    assert(firstLabel);
    assert(nextLabel >= firstLabel);
    assert(outputFile);

    if (ip == nextLabel->labelAddress) {
        printLabel(outputFile, nextLabel - firstLabel);
        fprintf(outputFile, "%c\n", LABEL_END);
        nextLabel++;
    }
    return nextLabel;
}

DisassemblyError processInput(char const* inputBuffer, size_t numBytes, LabelTable const* labelTable, FILE* outputFile) {
    assert(inputBuffer);
    assert(labelTable);
    assert(outputFile);

    LabelTableEntry const* firstLabel = labelTable->data;
    LabelTableEntry const* nextLabel = labelTable->data;

    CPUAddr ip = 0;
    while (ip < numBytes) {
        nextLabel = processNextLabel(nextLabel, firstLabel, ip, outputFile);
        processCommand(inputBuffer, numBytes, &ip, labelTable, outputFile);
    }
    assert(ip == numBytes);
    nextLabel = processNextLabel(nextLabel, firstLabel, ip, outputFile);
    assert(nextLabel + 1 == firstLabel + labelTable->size);

    return DISASSEMBLY_OK;
}

DisassemblyError disassemble(char const* const inputBuffer, const size_t numBytes, FILE* const outputFile) {
    assert(inputBuffer);
    assert(outputFile);

    LabelTable* labelTable = createLabelTable();
    if (!labelTable) {
        goto oom;
    }

    DisassemblyError res = preprocessInput(inputBuffer, numBytes, labelTable);
    if (res != DISASSEMBLY_OK) {
        freeLabelTable(labelTable);
        return res;
    }

    LabelAddRes addRes = addLabelUseByAddr(labelTable, numBytes + 1);
    if (addRes != LABEL_ADD_OK) {
        assert(addRes == LABEL_ADD_OOM);
        goto oom;
    }

    sortByAddress(labelTable);

    res = processInput(inputBuffer, numBytes, labelTable, outputFile);

    freeLabelTable(labelTable);

    return res;

oom:
    freeLabelTable(labelTable);

    return DISASSEMBLY_OOM;
}

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        printf("Usage: %s input_file output_file\n", (argc) ? (argv[0]) : ("program_name"));
        return -1;
    }

    char const* const inputFileName = argv[1];
    char const* const outputFileName = argv[2];

    FILE* inputFile = fopen(inputFileName, "rb");
    if (!inputFile) {
        printf("Failed to open %s for reading\n", inputFileName);
        return -1;
    }

    FILE* outputFile = fopen(outputFileName, "wb");
    if (!outputFile) {
        printf("Failed to open %s for writing\n", outputFileName);
        fclose(inputFile);
        return -1;
    }

    char* buffer = NULL;
    size_t numBytes = readBuffer(inputFile, &buffer);
    DisassemblyError res = disassemble(buffer, numBytes, outputFile);

    fclose(inputFile);
    fclose(outputFile);
    free(buffer);

    if (res != DISASSEMBLY_OK) {
        printf("DISASSEMBLY ERROR: %s\n", getDisassemblyErrorString(res));
        return -1;
    }

    return 0;
}
