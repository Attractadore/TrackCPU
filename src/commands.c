#include "commands.h"

#include "Commands_Gen.c"

#include <assert.h>
#include <string.h>

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))

Command const* getCommandByName(char const* str) {
    assert(str);

    for (size_t i = 0; i < ARR_SIZE(commandArr); i++) {
        if (strcmp(str, commandArr[i].name) == 0) {
            return commandArr + i;
        }
    }
    return NULL;
}

Command const* getCommandByCode(const CommandCode code) {
    for (size_t i = 0; i < ARR_SIZE(commandArr); i++) {
        if (commandArr[i].code == code) {
            return commandArr + i;
        }
    }
    return NULL;
}

Register const* getRegisterByName(char const* name) {
    assert(name);

    for (size_t i = 0; i < ARR_SIZE(registerArr); i++) {
        if (strcmp(name, registerArr[i].name) == 0) {
            return registerArr + i;
        }
    }
    return NULL;
}

Register const* getRegisterByCode(RegisterCode code) {
    for (size_t i = 0; i < ARR_SIZE(registerArr); i++) {
        if (registerArr[i].code == code) {
            return registerArr + i;
        }
    }
    return NULL;
}
