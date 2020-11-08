#include "commands.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char* readString(FILE* const inputFile) {
    assert(inputFile);

    fseek(inputFile, 0, SEEK_END);
    long fileEnd = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    long fileStart = ftell(inputFile);
    
    if (ferror(inputFile)) {
        return NULL;
    }

    size_t fileSize = fileEnd - fileStart;
    char* buffer = calloc(fileSize + 1, sizeof(*buffer));
    if (!buffer) {
        return NULL;
    }

    fread(buffer, sizeof(*buffer), fileSize, inputFile);
    if (ferror(inputFile)) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

void strip(char* const inputString) {
    assert(inputString);

    char const* readP = inputString;
    char* writeP = inputString;
    
    bool inComment = false;
    bool bPreviousSpace = true;
    while (*readP) {
        inComment = inComment || *readP == ';';
        bool bCurrentSpace = isspace(*readP);
        if (!inComment && !(bCurrentSpace && bPreviousSpace)) {
            *writeP = (bCurrentSpace) ? (' ') : (*readP);
            bPreviousSpace = bCurrentSpace;
            writeP++;
        }
        inComment = inComment && (*readP != '\n');
        readP++;
    }

    *writeP = '\0';
}

typedef enum assembly_error_e {
    ASSEMBLY_OK,
    ASSEMBLY_INVALID_COMMAND,
    ASSEMBLY_INVALID_ARGUMENT,
    ASSEMBLY_INVALID_ARGUMENT_COUNT,
} AssemblyError;

char const* getAssemblyErrorString(AssemblyError e) {
    switch (e) {
        case ASSEMBLY_OK:
            return "No assembly error";
        case ASSEMBLY_INVALID_COMMAND:
            return "Source contains invalid command";
        case ASSEMBLY_INVALID_ARGUMENT:
            return "Source contains command with invalid argument";
        case ASSEMBLY_INVALID_ARGUMENT_COUNT:
            return "Source contains command with invalid argument count";
        default:
            return "Unknown assembly error";
    }
    assert(!"No error returned string returned");
}

AssemblyError assemble(char* const inputString, FILE* outputFile) {
    assert(inputString);

    char const delim [] = " ";

    char const* token = strtok(inputString, delim);
    while (token) {
        Command const* const cmd = getCommandByName(token);
        if (!cmd) {
            return ASSEMBLY_INVALID_COMMAND;
        }
        size_t cmdCode = cmd->code;
        fwrite(&cmdCode, sizeof(cmdCode), 1, outputFile);
        for (size_t i = 0; i < cmd->numArgs; i++) {
            token = strtok(NULL, delim);
            if (!token) {
                return ASSEMBLY_INVALID_ARGUMENT_COUNT;
            }
            double cmdArg;
            if (sscanf(token, "%lg", &cmdArg) != 1) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            fwrite(&cmdArg, sizeof(cmdArg), 1, outputFile);
        }
        
        token = strtok(NULL, delim);
    }

    return ASSEMBLY_OK;
}

int main(int argc, char const* argv []) {
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

    char* srcString = readString(inputFile);
    strip(srcString);
    AssemblyError res = assemble(srcString, outputFile);
    fclose(inputFile);
    fclose(outputFile);
    free(srcString);
    if (res != ASSEMBLY_OK) {
        printf("ASSEMBLY ERROR: %s\n", getAssemblyErrorString(res));
        return -1;
    }

    return 0;
}
