/**
THIS FILE WAS GENERATED AUTOMATICALLY
IF YOU WANT TO ALTER IT MAKE CHANGES TO codegen.py
**/

#pragma once

#include "Commands_Gen.h"

#include <stddef.h>
typedef long long CPUInt;
typedef unsigned long long CPUUInt;
typedef double CPUFloat;
typedef size_t CPUAddr;

typedef union {
    CPUUInt asCPUUInt;
    RegisterCode asRegisterCode;
    CPUFloat asCPUFloat;
    CPUInt asCPUInt;
} CPUData;

#define CPUINT_PRINT_FMT "lld"
#define CPUUINT_PRINT_FMT "llu"
#define CPUFLOAT_PRINT_FMT "g"

#define CPUINT_SCAN_FMT "lld"
#define CPUUINT_SCAN_FMT "llu"
#define CPUFLOAT_SCAN_FMT "lg"