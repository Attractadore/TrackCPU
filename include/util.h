#pragma once

#include <stdio.h>

char* readString(FILE* const inputFile);
size_t readBuffer(FILE* const inputFile, char** bufferP);
