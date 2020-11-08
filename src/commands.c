#include "commands.h"

#include <string.h>
#include <assert.h>

#define ARR_SIZE(arr) sizeof(arr) / sizeof(*arr)

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
