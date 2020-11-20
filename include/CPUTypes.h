#pragma once

#include <assert.h>
#include <stddef.h>

typedef double CPUFloat;
typedef long long CPUInt;
typedef unsigned long long CPUUInt;
typedef CPUUInt CPURegData;
typedef size_t CPUAddr;
typedef size_t CPUCommandID;
typedef size_t CPURegisterID;
static_assert(sizeof(CPUFloat) == sizeof(CPUInt) && sizeof(CPUFloat) == sizeof(CPUUInt),
              "CPU data types are of different sizes");
#define CPU_PFMT_F "lg"
#define CPU_PFMT_I "lld"
#define CPU_PFMT_UI "llu"
#define CPU_SFMT_F "lg"
#define CPU_SFMT_I "lld"
#define CPU_SFMT_UI "llu"
