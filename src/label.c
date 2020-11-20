#include "label.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

bool isLabel(char const* token) {
    return *token == LABEL_START;
}

LabelType labelType(char const* token) {
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

int printLabel(FILE* outputFile, size_t labelNumber) {
    return fprintf(outputFile, "%cL%zu", LABEL_START, labelNumber);
}

enum {
    LABEL_TABLE_CAPACITY_INCREMENT = 100,
    LABEL_TABLE_ENTRY_CAPACITY_INCREMENT = 10,
};

LabelTableEntry* findLabelByName(LabelTable* table, char const* label) {
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

LabelTableEntry* findLabelByAddr(LabelTable const* table, size_t ip) {
    assert(table);

    for (size_t i = 0; i < table->size; i++) {
        LabelTableEntry* entry = table->data + i;
        if (entry->labelAddress == ip) {
            return entry;
        }
    }

    return NULL;
}

LabelTableEntry* addNewLabel(LabelTable* table) {
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
    table->size++;

    return newEntry;
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

LabelAddRes addLabelUseByAddr(LabelTable* table, size_t ip) {
    assert(table);

    LabelTableEntry* entry = findLabelByAddr(table, ip);
    if (!entry) {
        entry = addNewLabel(table);
        if (!entry) {
            return LABEL_ADD_OOM;
        }
        entry->bFound = true;
        entry->labelAddress = ip;
    }

    return LABEL_ADD_OK;
}

LabelAddRes addLabelDefByName(LabelTable* table, char const* label, size_t ip) {
    assert(table);
    assert(label);

    LabelTableEntry* entry = findLabelByName(table, label);
    if (!entry) {
        entry = addNewLabel(table);
        if (!entry) {
            return LABEL_ADD_OOM;
        }
    }

    if (entry->bFound) {
        return LABEL_ADD_DUPLICATE;
    }
    entry->bFound = true;
    entry->labelName = label;
    entry->labelAddress = ip;

    return LABEL_ADD_OK;
}

LabelAddRes addLabelUseByName(LabelTable* table, char const* label, size_t ip) {
    assert(table);
    assert(label);

    LabelTableEntry* entry = findLabelByName(table, label);
    if (!entry) {
        entry = addNewLabel(table);
        if (!entry) {
            return LABEL_ADD_OOM;
        }
        entry->labelName = label;
    }
    if (!addEntryAddress(entry, ip)) {
        return LABEL_ADD_OOM;
    }

    return LABEL_ADD_OK;
}

int cmpAddress(LabelTableEntry const* leftEntryP, LabelTableEntry const* rightEntryP) {
    assert(leftEntryP);
    assert(rightEntryP);

    if (leftEntryP->labelAddress < rightEntryP->labelAddress) {
        return -1;
    }
    if (leftEntryP->labelAddress > rightEntryP->labelAddress) {
        return 1;
    }
    return 0;
}

int cmpAddressQsort(void const* lp, void const* rp) {
    return cmpAddress(lp, rp);
}

void sortByAddress(LabelTable* table) {
    qsort(table->data, table->size, sizeof(*(table->data)), cmpAddressQsort);
}

LabelTable* createLabelTable() {
    LabelTable* table = calloc(1, sizeof(*table));
    return table;
}

void freeLabelTable(LabelTable* table) {
    if (table) {
        for (size_t i = 0; i < table->size; i++) {
            free(table->data[i].writeAddresses);
        }
        free(table->data);
    }
    free(table);
}
