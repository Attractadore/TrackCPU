#pragma once

#include <stdbool.h>
#include <stdio.h>

#define LABEL_START '.'
#define LABEL_END ':'

typedef enum label_type_e {
    LABEL_DECLARATION,
    LABEL_USAGE,
    LABEL_INVALID,
} LabelType;

typedef struct label_table_entry_t {
    size_t* writeAddresses;
    size_t size;
    size_t capacity;
    size_t labelAddress;
    char const* labelName;
    bool bFound;
} LabelTableEntry;

typedef enum label_add_res_e {
    LABEL_ADD_OK,
    LABEL_ADD_OOM,
    LABEL_ADD_DUPLICATE,
} LabelAddRes;

typedef struct label_table_t {
    LabelTableEntry* data;
    size_t size;
    size_t capacity;
} LabelTable;

bool isLabel(char const* token);
LabelType labelType(char const* token);
int printLabel(FILE* outputFile, size_t labelNumber);

LabelTableEntry* findLabelByAddr(LabelTable const* table, size_t ip);

LabelAddRes addLabelUseByAddr(LabelTable* table, size_t ip);

LabelAddRes addLabelDefByName(LabelTable* table, char const* label, size_t ip);
LabelAddRes addLabelUseByName(LabelTable* table, char const* label, size_t ip);

void sortByAddress(LabelTable* table);

LabelTable* createLabelTable();
void freeLabelTable(LabelTable* table);