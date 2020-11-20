#pragma once

#include "commands.h"

#include <stdbool.h>
#include <stdio.h>

#define LABEL_START '.'
#define LABEL_END ':'

typedef enum {
    LABEL_DECLARATION,
    LABEL_USAGE,
    LABEL_INVALID,
} LabelType;

typedef struct {
    CPUAddr* writeAddresses;
    size_t size;
    size_t capacity;
    CPUAddr labelAddress;
    char const* labelName;
    bool bFound;
} LabelTableEntry;

typedef struct {
    LabelTableEntry* data;
    size_t size;
    size_t capacity;
} LabelTable;

typedef enum {
    LABEL_ADD_OK,
    LABEL_ADD_OOM,
    LABEL_ADD_DUPLICATE,
} LabelAddRes;

bool isLabel(char const* token);
LabelType labelType(char const* token);
int printLabel(FILE* outputFile, size_t labelNumber);

LabelTableEntry* findLabelByAddr(LabelTable const* table, CPUAddr ip);

LabelAddRes addLabelUseByAddr(LabelTable* table, CPUAddr ip);

LabelAddRes addLabelDefByName(LabelTable* table, char const* label, CPUAddr ip);
LabelAddRes addLabelUseByName(LabelTable* table, char const* label, CPUAddr ip);

void sortByAddress(LabelTable* table);

LabelTable* createLabelTable();
void freeLabelTable(LabelTable* table);
