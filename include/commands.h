#pragma once

#include "Commands_Gen.h"

Command const* getCommandByName(char const* str);
Command const* getCommandByCode(CommandCode code);
Register const* getRegisterByName(char const* name);
Register const* getRegisterByCode(RegisterCode code);
