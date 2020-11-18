#include "commands.h"
#include "util.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void strip(char* const inputString) {
    assert(inputString);

    char const* readP = inputString;
    char* writeP = inputString;

    bool inComment = false;
    bool bPreviousSpace = true;
    while (*readP) {
        inComment = inComment || *readP == COMMENT_START;
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

bool isLabel(char const* const token) {
    return *token == LABEL_START;
}

typedef enum label_type_e {
    LABEL_DECLARATION,
    LABEL_USAGE,
    LABEL_INVALID,
} LabelType;

LabelType labelType(char const* const token) {
    char const* labelEnd = strchr(token, '\0');
    LabelType lt = LABEL_INVALID;
    if (*(labelEnd - 1) == LABEL_END) {
        lt = LABEL_DECLARATION;
        labelEnd--;
    } else {
        lt = LABEL_USAGE;
    }
    if (labelEnd <= token + 1) {
        return LABEL_INVALID;
    }

    for (char const* labelStart = token + 1; labelStart < labelEnd; labelStart++) {
        const char lc = *labelStart;
        if (!isalnum(lc) && lc != '_') {
            return LABEL_INVALID;
        }
    }
    return lt;
}

typedef struct label_table_entry_t {
    size_t* writeAddresses;
    size_t size;
    size_t capacity;
    size_t labelAddress;
    char const* labelName;
    bool bFound;
} LabelTableEntry;

typedef struct label_table_t {
    LabelTableEntry* data;
    size_t size;
    size_t capacity;
} LabelTable;

typedef enum label_add_res_e {
    LABEL_ADD_OK,
    LABEL_ADD_OOM,
    LABEL_ADD_DUPLICATE,
} LabelAddRes;

enum {
    LABEL_TABLE_CAPACITY_INCREMENT = 100,
    LABEL_TABLE_ENTRY_CAPACITY_INCREMENT = 10,
};

LabelTableEntry* findLabel(LabelTable* table, char const* label) {
    assert(table);
    assert(label);

    for (size_t i = 0; i < table->size; i++) {
        LabelTableEntry* entry = table->data + i;
        if (strcmp(entry->labelName, label) == 0) {
            return entry;
        }
    }

    return NULL;
}

LabelTableEntry* addNewLabel(LabelTable* table, char const* label) {
    if (table->size == table->capacity) {
        size_t newCapacity = table->capacity + LABEL_TABLE_CAPACITY_INCREMENT;
        LabelTableEntry* newData = realloc(table->data, newCapacity * sizeof(*newData));
        if (!newData) {
            return NULL;
        }
        table->data = newData;
        table->capacity = newCapacity;
    }

    LabelTableEntry* newEntry = table->data + table->size;
    memset(newEntry, 0, sizeof(*newEntry));
    newEntry->labelName = label;
    table->size++;

    return newEntry;
}

LabelAddRes addFoundLabel(LabelTable* table, char const* label, size_t ip) {
    assert(table);
    assert(label);

    LabelTableEntry* entry = findLabel(table, label);
    if (!entry) {
        entry = addNewLabel(table, label);
        if (!entry) {
            return LABEL_ADD_OOM;
        }
    }

    if (entry->bFound) {
        return LABEL_ADD_DUPLICATE;
    }
    entry->bFound = true;
    entry->labelAddress = ip;

    return LABEL_ADD_OK;
}

size_t* addEntryAddress(LabelTableEntry* entry, size_t address) {
    assert(entry);
    assert(address);

    if (entry->size == entry->capacity) {
        size_t newCapacity = entry->capacity + LABEL_TABLE_ENTRY_CAPACITY_INCREMENT;
        size_t* newAddresses = realloc(entry->writeAddresses, newCapacity * sizeof(*newAddresses));
        if (!newAddresses) {
            return NULL;
        }
        entry->writeAddresses = newAddresses;
        entry->capacity = newCapacity;
    }
    entry->writeAddresses[entry->size++] = address;
    return entry->writeAddresses;
}

LabelAddRes addLabelAddress(LabelTable* table, char const* label, size_t address) {
    assert(table);
    assert(label);

    LabelTableEntry* entry = findLabel(table, label);
    if (!entry) {
        entry = addNewLabel(table, label);
        if (!entry) {
            return LABEL_ADD_OOM;
        }
    }
    if (!addEntryAddress(entry, address)) {
        return LABEL_ADD_OOM;
    }

    return LABEL_ADD_OK;
}

LabelTable* createLabelTable() {
    LabelTable* table = calloc(1, sizeof(*table));
    return table;
}

void freeLabelTable(LabelTable* table) {
    for (size_t i = 0; i < table->size; i++) {
        free(table->data[i].writeAddresses);
    }
    free(table);
}

AssemblyError handleLabel(char** tokenP, char const* delim, size_t ip, LabelTable* labelTable) {
    assert(tokenP);
    assert(delim);
    assert(labelTable);

    char* token = *tokenP;
    printf("Process label %s\n", token);

    switch (labelType(token)) {
        case LABEL_DECLARATION: {
            char* labelEnd = strchr(token, LABEL_END);
            assert(labelEnd);
            *labelEnd = '\0';
            LabelAddRes res = addFoundLabel(labelTable, token, ip);
            switch (res) {
                case LABEL_ADD_OK:
                    *tokenP = strtok(NULL, delim);
                    return ASSEMBLY_OK;
                    break;
                case LABEL_ADD_OOM:
                    freeLabelTable(labelTable);
                    return ASSEMBLY_MEMORY_EXHAUSTED;
                case LABEL_ADD_DUPLICATE:
                    freeLabelTable(labelTable);
                    return ASSEMBLY_DUPLICATE_LABEL;
                default:
                    freeLabelTable(labelTable);
                    return ASSEMBLY_UNKNOW_ERROR;
            }
        } break;
        default:
            return ASSEMBLY_INVALID_LABEL;
    }

    assert(!"No result returned");
}

AssemblyError handleArg(CommandArgType argType, char const** const tokenP, char const* delim, size_t* const ipp, LabelTable* labelTable, FILE* outputFile) {
    assert(tokenP);
    assert(delim);
    assert(ipp);
    assert(labelTable);
    assert(outputFile);
    char const* token = *tokenP;
    size_t ip = *ipp;

    assert(token);
    printf("Process arg %s\n", token);

    switch (argType) {
        case CMD_ARG_TYPE_INT: {
            int64_t cmdArg;
            if (sscanf(token, "%" SCNd64, &cmdArg) != 1) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            fwrite(&cmdArg, sizeof(cmdArg), 1, outputFile);
            ip += sizeof(cmdArg);
        } break;
        case CMD_ARG_TYPE_UINT: {
            uint64_t cmdArg;
            if (sscanf(token, "%" SCNu64, &cmdArg) != 1) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            fwrite(&cmdArg, sizeof(cmdArg), 1, outputFile);
            ip += sizeof(cmdArg);
        } break;
        case CMD_ARG_TYPE_FLOAT: {
            double cmdArg;
            if (sscanf(token, "%lg", &cmdArg) != 1) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            fwrite(&cmdArg, sizeof(cmdArg), 1, outputFile);
            ip += sizeof(cmdArg);
        } break;
        case CMD_ARG_TYPE_LABEL: {
            if (!isLabel(token) || labelType(token) != LABEL_USAGE) {
                return ASSEMBLY_INVALID_LABEL;
            }
            LabelAddRes res = addLabelAddress(labelTable, token, ip);
            switch (res) {
                case LABEL_ADD_OK:
                    break;
                case LABEL_ADD_OOM:
                    freeLabelTable(labelTable);
                    return ASSEMBLY_MEMORY_EXHAUSTED;
                default:
                    freeLabelTable(labelTable);
                    return ASSEMBLY_UNKNOW_ERROR;
            }
        } break;
        case CMD_ARG_TYPE_REGISTER: {
            Register const* reg = getRegisterByName(token);
            if (!reg) {
                return ASSEMBLY_INVALID_ARGUMENT;
            }
            fwrite(&(reg->code), sizeof(reg->code), 1, outputFile);
            ip += sizeof(reg->code);
        } break;

        default:
            assert(!"Got unknown arg type");
    }

    *ipp = ip;
    *tokenP = token;
    return ASSEMBLY_OK;
}

AssemblyError handleArgs(Command const* const cmd, char const** const tokenP, char const* delim, size_t* const ipp, LabelTable* labelTable, FILE* outputFile) {
    assert(cmd);
    assert(tokenP);
    assert(delim);
    assert(ipp);
    assert(labelTable);
    assert(outputFile);
    char const* token = *tokenP;
    size_t ip = *ipp;

    assert(cmd->numArgs <= COMMAND_MAX_ARGS);

    for (size_t i = 0; i < cmd->numArgs; i++) {
        if (!token) {
            return ASSEMBLY_INVALID_ARGUMENT_COUNT;
        }
        AssemblyError res = handleArg(cmd->argType[i], &token, delim, &ip, labelTable, outputFile);
        if (res != ASSEMBLY_OK) {
            return res;
        }
        token = strtok(NULL, delim);
    }

    *ipp = ip;
    *tokenP = token;
    return ASSEMBLY_OK;
}

AssemblyError handleCommand(char const** const tokenP, char const* delim, size_t* const ipp, LabelTable* labelTable, FILE* outputFile) {
    assert(tokenP);
    assert(delim);
    assert(ipp);
    assert(labelTable);
    assert(outputFile);

    char const* token = *tokenP;
    printf("Process command %s\n", token);
    size_t ip = *ipp;
    Command const* const cmd = getCommandByName(token);
    if (!cmd) {
        return ASSEMBLY_INVALID_COMMAND;
    }

    size_t cmdCode = cmd->code;
    fwrite(&cmdCode, sizeof(cmdCode), 1, outputFile);
    ip += sizeof(cmdCode);
    token = strtok(NULL, delim);
    AssemblyError res = handleArgs(cmd, &token, delim, &ip, labelTable, outputFile);
    *tokenP = token;
    *ipp = ip;

    return res;
}

AssemblyError assemble(char* const inputString, FILE* outputFile) {
    assert(inputString);

    printf("Assembly source: %s\n", inputString);

    char const delim[] = " ";

    LabelTable* labelTable = createLabelTable();
    if (!labelTable) {
        return ASSEMBLY_MEMORY_EXHAUSTED;
    }
    size_t ip = 0;
    char* token = strtok(inputString, delim);
    while (token) {
        AssemblyError res = ASSEMBLY_OK;
        if (isLabel(token)) {
            res = handleLabel(&token, delim, ip, labelTable);
        } else {
            res = handleCommand(&token, delim, &ip, labelTable, outputFile);
        }
        if (res != ASSEMBLY_OK) {
            freeLabelTable(labelTable);
            return res;
        }
    }

    for (size_t i = 0; i < labelTable->size; i++) {
        LabelTableEntry* entry = labelTable->data + i;
        printf("Write addresses for label %s\n", entry->labelName);
        if (!entry->bFound) {
            freeLabelTable(labelTable);
            return ASSEMBLY_INVALID_LABEL;
        }
        for (size_t j = 0; j < entry->size; j++) {
            fseek(outputFile, entry->writeAddresses[j], SEEK_SET);
            fwrite(&entry->labelAddress, sizeof(entry->labelAddress), 1, outputFile);
        }
    }

    freeLabelTable(labelTable);

    return ASSEMBLY_OK;
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
