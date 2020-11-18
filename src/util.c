#include "util.h"

#include <assert.h>
#include <stdlib.h>

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

size_t readBuffer(FILE* const inputFile, char** bufferP) {
    assert(inputFile);
    assert(bufferP);

    *bufferP = NULL;

    fseek(inputFile, 0, SEEK_END);
    long fileEnd = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    long fileStart = ftell(inputFile);

    if (ferror(inputFile)) {
        return 0;
    }

    size_t fileSize = fileEnd - fileStart;
    char* buffer = calloc(fileSize, sizeof(*buffer));
    if (!buffer) {
        return 0;
    }

    size_t readSize = fread(buffer, sizeof(*buffer), fileSize, inputFile);
    if (ferror(inputFile)) {
        free(buffer);
        return 0;
    }

    *bufferP = buffer;

    return readSize;
}
