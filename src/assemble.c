#include "commands.h"
#include "util.h"
#include "label.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define COMMENT_START '#'
#define STRIP_DELIM " "

void strip(char* inputString) {
    assert(inputString);

    char const* readP = inputString;
    char* writeP = inputString;

    bool inComment = false;
    bool bPreviousSpace = true;
    while (*readP) {
        inComment = inComment || *readP == COMMENT_START;
        bool bCurrentSpace = isspace(*readP);
        if (!inComment && !(bCurrentSpace && bPreviousSpace)) {
            *writeP = (bCurrentSpace) ? (*STRIP_DELIM) : (*readP);
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
    ASSEMBLY_MEMORY_EXHAUSTED,
    ASSEMBLY_INVALID_COMMAND,
    ASSEMBLY_INVALID_LABEL,
    ASSEMBLY_DUPLICATE_LABEL,
    ASSEMBLY_INVALID_ARGUMENT,
    ASSEMBLY_INVALID_ARGUMENT_COUNT,
    ASSEMBLY_UNKNOW_ERROR,
} AssemblyError;

char const* getAssemblyErrorString(AssemblyError e) {
    switch (e) {
        case ASSEMBLY_OK:
            return "No assembly error";
        case ASSEMBLY_MEMORY_EXHAUSTED:
            return "Ran out of memory during assembly";
        case ASSEMBLY_INVALID_LABEL:
            return "Source contains invalid label";
        case ASSEMBLY_DUPLICATE_LABEL:
            return "Source contains redefinition of label";
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

AssemblyError handleLabel(char** tokenP, size_t ip, LabelTable* labelTable) {
    assert(tokenP);
    assert(labelTable);

    char* token = *tokenP;
    assert(token);

    printf("Process label %s\n", token);

    switch (labelType(token)) {
        case LABEL_DECLARATION: {
            char* labelEnd = strchr(token, LABEL_END);
            assert(labelEnd);
            *labelEnd = '\0';
            LabelAddRes res = addLabelDefByName(labelTable, token, ip);
            switch (res) {
                case LABEL_ADD_OK:
                    *tokenP = strtok(NULL, STRIP_DELIM);
                    return ASSEMBLY_OK;
                case LABEL_ADD_OOM:
                    return ASSEMBLY_MEMORY_EXHAUSTED;
                case LABEL_ADD_DUPLICATE:
                    return ASSEMBLY_DUPLICATE_LABEL;
                default:
                    return ASSEMBLY_UNKNOW_ERROR;
            }
        } break;
        default:
            return ASSEMBLY_INVALID_LABEL;
    }

    assert(!"No result returned");
}

#define WRITE_ADVANCE(data, ip, outputFile) \
do { \
    fwrite(&data, sizeof(data), 1, outputFile); \
    ip += sizeof(data); \
} while(0)

AssemblyError handleArg(CommandArgType argType, char** tokenP, size_t* ipp, LabelTable* labelTable, FILE* outputFile) {
    assert(tokenP);
    assert(ipp);
    assert(labelTable);
    assert(outputFile);

    char* token = *tokenP;
    assert(token);
    size_t ip = *ipp;

    printf("Process arg %s\n", token);

    switch (argType) {
        case CMD_ARG_TYPE_INT: {
            CPUInt cmdArg;
            if (sscanf(token, "%" CPU_SFMT_I, &cmdArg) != 1) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            WRITE_ADVANCE(cmdArg, ip, outputFile);
        } break;
        case CMD_ARG_TYPE_UINT: {
            CPUUInt cmdArg;
            if (sscanf(token, "%" CPU_SFMT_UI, &cmdArg) != 1) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            WRITE_ADVANCE(cmdArg, ip, outputFile);
        } break;
        case CMD_ARG_TYPE_FLOAT: {
            CPUFloat cmdArg;
            if (sscanf(token, "%" CPU_SFMT_F, &cmdArg) != 1) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            WRITE_ADVANCE(cmdArg, ip, outputFile);
        } break;
        case CMD_ARG_TYPE_LABEL: {
            if (!isLabel(token) || labelType(token) != LABEL_USAGE) {
                return ASSEMBLY_INVALID_LABEL;
            }
            LabelAddRes res = addLabelUseByName(labelTable, token, ip);
            switch (res) {
                case LABEL_ADD_OK: {
                    size_t cmdArg = 0;
                    WRITE_ADVANCE(cmdArg, ip, outputFile);
                }
                    break;
                case LABEL_ADD_OOM:
                    return ASSEMBLY_MEMORY_EXHAUSTED;
                default:
                    return ASSEMBLY_UNKNOW_ERROR;
            }
        } break;
        case CMD_ARG_TYPE_REGISTER: {
            Register const* reg = getRegisterByName(token);
            if (!reg) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            CPURegisterID cmdArg = reg->code;
            WRITE_ADVANCE(cmdArg, ip, outputFile);
        } break;
        default:
            assert(!"Got unknown arg type");
    }

    *ipp = ip;
    *tokenP = token;

    return ASSEMBLY_OK;
}

AssemblyError handleCommand(char** tokenP, size_t* ipp, LabelTable* labelTable, FILE* outputFile) {
    assert(tokenP);
    assert(ipp);
    assert(labelTable);
    assert(outputFile);

    char* token = *tokenP;
    assert(token);
    size_t ip = *ipp;

    printf("Process command %s\n", token);

    Command const* cmd = getCommandByName(token);
    if (!cmd) {
        return ASSEMBLY_INVALID_COMMAND;
    }

    CPUCommandID cmdCode = cmd->code;
    WRITE_ADVANCE(cmdCode, ip, outputFile);
    token = strtok(NULL, STRIP_DELIM);

    assert(cmd->numArgs <= COMMAND_MAX_ARGS);
    for (size_t i = 0; i < cmd->numArgs; i++) {
        if (!token) {
            return ASSEMBLY_INVALID_ARGUMENT_COUNT;
        }
        AssemblyError res = handleArg(cmd->argType[i], &token, &ip, labelTable, outputFile);
        if (res != ASSEMBLY_OK) {
            return res;
        }
        token = strtok(NULL, STRIP_DELIM);
    }

    *tokenP = token;
    *ipp = ip;

    return ASSEMBLY_OK;
}

#undef WRITE_ADVANCE

AssemblyError processInput(char* inputString, LabelTable* labelTable, FILE* outputFile) {
    assert(inputString);
    assert(labelTable);
    assert(outputFile);

    size_t ip = 0;
    char* token = strtok(inputString, STRIP_DELIM);
    while (token) {
        AssemblyError res = ASSEMBLY_OK;
        if (isLabel(token)) {
            res = handleLabel(&token, ip, labelTable);
        } else {
            res = handleCommand(&token, &ip, labelTable, outputFile);
        }
        if (res != ASSEMBLY_OK) {
            return res;
        }
    }

    return ASSEMBLY_OK;
}

AssemblyError writeJumpAddresses(LabelTable const* labelTable, FILE* outputFile) {
    assert(labelTable);
    assert(outputFile);

    for (size_t i = 0; i < labelTable->size; i++) {
        LabelTableEntry* entry = labelTable->data + i;
        printf("Write addresses for label %s\n", entry->labelName);
        if (!entry->bFound) {
            return ASSEMBLY_INVALID_LABEL;
        }
        for (size_t j = 0; j < entry->size; j++) {
            fseek(outputFile, entry->writeAddresses[j], SEEK_SET);
            fwrite(&entry->labelAddress, sizeof(entry->labelAddress), 1, outputFile);
        }
    }

    return ASSEMBLY_OK;
}

AssemblyError assemble(char* inputString, FILE* outputFile) {
    assert(inputString);
    assert(outputFile);

    LabelTable* labelTable = createLabelTable();
    if (!labelTable) {
        return ASSEMBLY_MEMORY_EXHAUSTED;
    }

    AssemblyError res = processInput(inputString, labelTable, outputFile);
    if (res == ASSEMBLY_OK) {
        res = writeJumpAddresses(labelTable, outputFile);
    }

    freeLabelTable(labelTable);

    return res;
}

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        printf("Usage: %s input_file output_file\n", (argc) ? (argv[0]) : ("program_name"));
        return -1;
    }

    char const* inputFileName = argv[1];
    char const* outputFileName = argv[2];

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
