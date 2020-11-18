#include "commands.h"
#include "util.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum disassembly_error_e {
    DISASSEMBLY_OK,
    DISASSEMBLY_INVALID_COMMAND,
    DISASSEMBLY_INVALID_ARGUMENT_COUNT,
    DISASSEMBLY_CORRUPTION,
} DisassemblyError;

char const* getDisassemblyErrorString(DisassemblyError e) {
    switch (e) {
        case DISASSEMBLY_OK:
            return "No disassembly error";
        case DISASSEMBLY_INVALID_COMMAND:
            return "Binary contains invalid command code";
        case DISASSEMBLY_INVALID_ARGUMENT_COUNT:
            return "Binary contains command with invalid argument count";
        case DISASSEMBLY_CORRUPTION:
            return "Binary is corrupted";
        default:
            return "Unknown disassembly error";
    }
    assert(!"No error returned string returned");
}

DisassemblyError disassemble(char const* const inputBuffer, const size_t numBytes, FILE* const outputFile) {
    assert(inputBuffer);
    assert(outputFile);

    size_t bufI = 0;
    while (bufI < numBytes) {
        size_t cmdCode = CMD_INVALID_CODE;
        if (bufI + sizeof(cmdCode) > numBytes) {
            return DISASSEMBLY_CORRUPTION;
        }
        memcpy(&cmdCode, inputBuffer + bufI, sizeof(cmdCode));
        bufI += sizeof(cmdCode);

        Command const* const cmd = getCommandByCode(cmdCode);
        if (!cmd) {
            return DISASSEMBLY_INVALID_COMMAND;
        }

        fprintf(outputFile, "%s", cmd->name);
        for (size_t i = 0; i < cmd->numArgs; i++) {
            double cmdArg;
            if (bufI + sizeof(cmdArg) > numBytes) {
                return DISASSEMBLY_CORRUPTION;
            }
            memcpy(&cmdArg, inputBuffer + bufI, sizeof(cmdArg));
            bufI += sizeof(cmdArg);
            fprintf(outputFile, " %g", cmdArg);
        }
        fprintf(outputFile, "\n");
    }

    return DISASSEMBLY_OK;
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
