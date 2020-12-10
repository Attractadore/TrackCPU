#!/usr/bin/python3

from sys import argv
from os.path import relpath
from string import ascii_lowercase as letters
commands = {
    "halt": (("TERM",),),
    "call": (("READ", "CPUAddr"), ("PUSHIP",), ("SETIP", 0)),
    "return": (("POPIP",),),
    "jump": (("READ", "CPUAddr"), ("SETIP", 0)),
    "jumpt": (("READ", "CPUAddr"), ("POPD", "CPUUInt"), ("CHECKT", 1), ("SETIP", 0), ("ENDCHECK",)),
    "jumpf": (("READ", "CPUAddr"), ("POPD", "CPUUInt"), ("CHECKF", 1), ("SETIP", 0), ("ENDCHECK",)),
    "push": (("READ", "RegisterCode"), ("LOADREG", 0), ("PUSHD",  1)),
    "push(T)": (("READ", "T"), ("PUSHD", 0)),
    "pop": (("READ", "RegisterCode"), ("POPD", "CPUUInt"), ("WRITEREG", 0, 1)),
    "pops": (("POPD", "CPUUInt"),),
    "top": (("READ", "RegisterCode"), ("TOPD", "CPUUInt"), ("WRITEREG", 0, 1)),

    "eq(T)": (("APPLY", 2, "T", "{0} == {1}"),),
    "ne(T)": (("APPLY", 2, "T", "{0} != {1}"),),
    "l(T)": (("APPLY", 2, "T", "{0} < {1}"),),
    "le(T)": (("APPLY", 2, "T", "{0} <= {1}"),),
    "g(T)": (("APPLY", 2, "T", "{0} > {1}"),),
    "ge(T)": (("APPLY", 2, "T", "{0} >= {1}"),),

    "band": (("APPLY", 2, "CPUUInt", "{0} & {1}"),),
    "and": (("APPLY", 2, "CPUUInt", "{0} && {1}"),),
    "bor": (("APPLY", 2, "CPUUInt", "{0} | {1}"),),
    "or": (("APPLY", 2, "CPUUInt", "{0} || {1}"),),
    "bxor": (("APPLY", 2, "CPUUInt", "{0} ^ {1}"),),
    "bnot": (("APPLY", 1, "CPUUInt", "~{0}"),),
    "not": (("APPLY", 1, "CPUUInt", "!{0}"),),
    "ls": (("APPLY", 2, "CPUUInt", "{0} << {1}"),),
    "rs": (("APPLY", 2, "CPUUInt", "{0} >> {1}"),),
    "lrs": (("APPLY", 2, "CPUInt", "{0} >> {1}"),),

    "add(T)": (("APPLY", 2, "T", "{0} + {1}"),),
    "sub(T)": (("APPLY", 2, "T", "{0} - {1}"),),
    "mul(T)": (("APPLY", 2, "T", "{0} * {1}"),),
    "div(T)": (("APPLY", 2, "T", "{0} / {1}"),),

    "modi": (("APPLY", 2, "CPUInt", "{0} % {1}"),),
    "modu": (("APPLY", 2, "CPUUInt", "{0} % {1}"),),

    "sqrt": (("APPLY", 1, "CPUFloat", "sqrt({0})"),),
    "print(T)": (("TOPD", "T"), ("PRINT", 0)),
    "scan(T)": (("SCAN", "T"), ("PUSHD", 0)),
    "load": (("READ", "RegisterCode"), ("LOAD", 0), ("PUSHD", 1)),
    "store": (("READ", "RegisterCode"), ("TOPD", "CPUUInt"), ("STORE", 1, 0)),
    "move": (("READ", "RegisterCode"), ("POPD", "CPUUInt"), ("STORE", 1, 0)),
}

CPUShortTypes = {
    "CPUInt": "i",
    "CPUUInt": "u",
    "CPUFloat": "f"
}
CPUTypesPrintf = {
    "CPUInt": "lld",
    "CPUUInt": "llu",
    "CPUFloat": "g",
}
CPUTypesScanf = {
    "CPUInt": "lld",
    "CPUUInt": "llu",
    "CPUFloat": "lg",
}

newCommands = {}
for k, v in commands.items():
    if (k[-3:] == "(T)"):
        for l, s in CPUShortTypes.items():
            newCommands[k[:-3] + s] = tuple(tuple(arg if arg !=
                                                  "T" else l for arg in command) for command in v)
    else:
        newCommands[k] = v
commands = newCommands


def CmdNameSymbol(name):
    return "CMD_{0}_NAME".format(name.upper())


def CmdCodeSymbol(name):
    return "CMD_{0}_CODE".format(name.upper())


def RegNameSymbol(name):
    return "REG_NAME_{}".format(name.upper())


def RegCodeSymbol(name):
    return "REG_CODE_{}".format(name.upper())


def CPUTypePrintSymbol(CPUType):
    return "{}_PRINT_FMT".format(CPUType.upper())


def CPUTypeScanSymbol(CPUType):
    return "{}_SCAN_FMT".format(CPUType.upper())


CPUAssArgTypes = {
    None: "NONE",
    "CPUInt": "INT",
    "CPUUInt": "UINT",
    "CPUFloat": "FLOAT",
    "CPUAddr": "LABEL",
    "RegisterCode": "REGISTER",
}


def ArgTypeSymbol(name):
    return "CMD_ARG_TYPE_{0}".format(CPUAssArgTypes[name].upper())


def getArgNameType(args, arg):
    return "arg{0}".format(int(arg)), args[int(arg)]


CPUExecDefines = {
    "HALT",
    "DATA_PUSH",
    "DATA_POP",
    "DATA_TOP",
    "IP_PUSH",
    "IP_POP",
    "READ",
    "PRINT",
    "SCAN",
    "LOAD_VALUE",
    "STORE_VALUE",
    "READ_REGISTER",
    "WRITE_REGISTER",
    "IP",
    "CMD_CODE",
    "UNKNOWN_COMMAND"
}


def command_execute_def(name, definition):
    args = []
    c_code = ""
    argTypes = []
    for cmd in definition:
        cmd_name = cmd[0]
        argName = "arg{0}".format(len(args))
        if cmd_name == "TERM":
            c_code += "HALT();"
            break
        elif cmd_name == "READ":
            argType = cmd[1]
            args.append(argType)
            argTypes.append(argType)
            c_code += "READ({0}, {1});".format(argType, argName)
        elif cmd_name == "PUSHIP":
            c_code += "IP_PUSH();"
        elif cmd_name == "POPIP":
            c_code += "IP_POP();".format(argName)
        elif cmd_name == "SETIP":
            newIp, _ = getArgNameType(args, cmd[1])
            c_code += "IP = {0};".format(newIp)
        elif cmd_name == "PUSHD":
            pushArg, pushArgType = getArgNameType(args, cmd[1])
            c_code += "DATA_PUSH({0}, {1});".format(
                pushArgType, pushArg)
        elif cmd_name == "POPD":
            argType = cmd[1]
            args.append(argType)
            c_code += "DATA_POP({0}, {1});".format(argType, argName)
        elif cmd_name == "TOPD":
            argType = cmd[1]
            args.append(argType)
            c_code += "DATA_TOP({0}, {1});".format(argType, argName)
        elif cmd_name == "CHECKT":
            checkArg, _ = getArgNameType(args, cmd[1])
            c_code += "if ({0}) {{".format(checkArg)
        elif cmd_name == "CHECKF":
            checkArg, _ = getArgNameType(args, cmd[1])
            c_code += "if (!{0}) {{".format(checkArg)
        elif cmd_name == "ENDCHECK":
            c_code += "}"
        elif cmd_name == "LOADREG":
            loadReg, _ = getArgNameType(args, cmd[1])
            loadArgType = "CPUUInt"
            args.append(loadArgType)
            c_code += "READ_REGISTER({2}, {0}, {1});".format(
                argType, argName, loadReg)
        elif cmd_name == "WRITEREG":
            writeReg, _ = getArgNameType(args, cmd[1])
            writeArg, writeArgType = getArgNameType(args, cmd[2])
            c_code += "WRITE_REGISTER({0}, {2}, {1});".format(writeReg,
                                                              writeArg, writeArgType)
        elif cmd_name == "APPLY":
            num = cmd[1]
            argType = cmd[2]
            op = cmd[3]
            fmtList = []
            c_code += "{"
            for i in range(num):
                argName = "arg{0}".format(i)
                c_code += "DATA_POP({0}, {1});".format(argType, argName)
                fmtList.append(argName)
            c_code += "{0} arg{1} = {2};".format(argType,
                                                 len(args) + num, op.format(*fmtList))
            argName = "arg{0}".format(num)
            c_code += "DATA_PUSH({0}, {1});".format(
                argType, argName)
            c_code += "}"
        elif cmd_name == "PRINT":
            printArg, printArgType = getArgNameType(args, cmd[1])
            fmt = CPUTypePrintSymbol(printArgType)
            c_code += "PRINT(\"%\" {0} \"\\n\", {1});".format(fmt, printArg)
        elif cmd_name == "SCAN":
            scanType = cmd[1]
            args.append(scanType)
            fmt = CPUTypeScanSymbol(scanType)
            c_code += "SCAN(\"%\" {1}, {0}, {2});".format(scanType,
                                                          fmt, argName)
        elif cmd_name == "LOAD":
            loadedArgType = "CPUUInt"
            args.append(loadedArgType)
            loadRegName, _ = getArgNameType(args, cmd[1])
            c_code += "READ_REGISTER({2}, CPUUInt, load_temp); LOAD_VALUE(load_temp, {0}, {1});".format(
                loadedArgType, argName, loadRegName)
        elif cmd_name == "STORE":
            storeArgName, storeArgType = getArgNameType(args, cmd[1])
            storeRegName, _ = getArgNameType(args, cmd[2])
            c_code += "{{READ_REGISTER({2}, CPUUInt, store_temp); STORE_VALUE(store_temp, {0}, {1});}}".format(
                storeArgType, storeArgName, storeRegName)
        else:
            assert(not "Undefined command")

    return c_code, argTypes


def command_def(name, argTypes):
    return (CmdNameSymbol(name), CmdCodeSymbol(name), len(argTypes),
            tuple(ArgTypeSymbol(arg) for arg in argTypes) if len(argTypes) else (ArgTypeSymbol(None),))


def CommandDefStr(d):
    return "{{ {}, {}, {}, {{ {} }} }}".format(d[0], d[1], d[2], *d[3])


def getDefinitionsSrcCodes():
    srcCodes = {}
    definitions = {}
    for k, v in commands.items():
        code, argTypes = command_execute_def(k, v)
        srcCodes[k] = code
        definitions[k] = command_def(k, argTypes)
    return definitions, srcCodes


def Include(f, glob=False):
    fs = "#include <{}>" if glob else "#include \"{}\""
    return fs.format(f)


def GenerateCommandArray(definitions):
    c_code = "static Command const commandArr[] = {\n"
    c_code += ",\n".join(CommandDefStr(d) for d in definitions.values())
    c_code += "};"
    return c_code


def GenerateCommandNames(names):
    c_code = "\n".join("#define {} \"{}\"".format(
        CmdNameSymbol(name), name) for name in names)
    return c_code


def GenerateCommandCodeEnum(names):
    c_code = Include("stddef.h", True) + "\ntypedef enum : size_t {\n"
    c_code += ",\n".join(CmdCodeSymbol(name) for name in (*names, "INVALID"))
    c_code += "} CommandCode;"
    return c_code


def GenerateCommandArgTypeEnum():
    c_code = "typedef enum {\n"
    c_code += ",\n".join(ArgTypeSymbol(argType) for argType in CPUAssArgTypes)
    c_code += "} CommandArgType;"
    return c_code


def GenerateCommandMaxArgs(maxArgs):
    c_code = "enum {{ COMMAND_MAX_ARGS = {} }};".format(maxArgs)
    return c_code


def GenerateCommandStruct():
    c_code = Include("stddef.h", True) + """
                                         typedef struct {
                                         char const* const name;
                                         const CommandCode code;
                                         const size_t numArgs;
                                         const CommandArgType argType[COMMAND_MAX_ARGS];
                                         } Command;"""
    return c_code


def GetRegisterNames(numRegisters):
    return ("r" + letter for letter in letters[:numRegisters])


def GenerateRegisterNames(numRegisters):
    c_code = "\n".join("#define {} \"{}\"".format(RegNameSymbol(
        regName), regName) for regName in GetRegisterNames(numRegisters))
    return c_code


def GenerateRegisterCodeEnum(numRegisters):
    c_code = Include("stddef.h", True) + "\ntypedef enum : size_t {\n"
    c_code += ",\n".join(RegCodeSymbol(regName) for regName in
                         (*GetRegisterNames(numRegisters), "INVALID",))
    c_code += "} RegisterCode;"
    return c_code


def GenerateNumRegisters(numRegisters):
    c_code = "enum {{ NUM_REGISTERS = {} }};".format(numRegisters)
    return c_code


def GenerateRegisterStruct():
    c_code = """typedef struct {
               char const* const name;
               RegisterCode const code;
               } Register;"""
    return c_code


def GenerateRegisterArray(numRegisters):
    c_code = "static Register const registerArr[] = {\n"
    c_code += ",\n".join("{{ {}, {} }}".format(RegNameSymbol(regName), RegCodeSymbol(regName))
                         for regName in GetRegisterNames(numRegisters))
    c_code += "};"
    return c_code


CPUTypes = {
    "CPUInt": "long long",
    "CPUUInt": "unsigned long long",
    "CPUFloat": "double",
    "CPUAddr": "size_t",
}


def GenerateCPUTypes():
    c_code = "\n".join((
        Include("stddef.h", True),
        *("typedef {} {};".format(baseType, CPUType) for CPUType, baseType in CPUTypes.items())
    )
    )
    return c_code


CPUDataTypes = {
    "CPUInt",
    "CPUUInt",
    "CPUFloat",
    "RegisterCode",
}


def GenerateCPUData():
    c_code = "\n".join((
        "typedef union {",
        *(
            "{0} as{0};".format(CPUDataType) for CPUDataType in CPUDataTypes
        ),
        "} CPUData;"
    )
    )
    return c_code


def GenerateCPUPrintFormats():
    c_code = "\n".join(
        "#define {} \"{}\"".format(CPUTypePrintSymbol(CPUType), CPUPrintFormat) for CPUType, CPUPrintFormat in CPUTypesPrintf.items()
    )
    return c_code


def GenerateCPUScanFormats():
    c_code = "\n".join(
        "#define {} \"{}\"".format(CPUTypeScanSymbol(CPUType), CPUScanFormat) for CPUType, CPUScanFormat in CPUTypesScanf.items()
    )
    return c_code


def GenerateCPUExecSwitch(srcCodes):
    c_code = "\n".join((
        "switch(CMD_CODE) {",
        *("case {0}: {{ {1} }} break;".format(CmdCodeSymbol(cmdName), cmdSrc)
          for cmdName, cmdSrc in srcCodes.items()),
        "default: UNKNOWN_COMMAND();",
        "}"
    )
    )
    return c_code


def GenerateCPUExecDefCheck():
    c_code = "\n".join(
        "#ifndef {0}\n#error {0} not defined\n#endif".format(CPUExecDefine) for CPUExecDefine in CPUExecDefines
    )
    return c_code


Disclaimer = """/**
THIS FILE WAS GENERATED AUTOMATICALLY
IF YOU WANT TO ALTER IT MAKE CHANGES TO {}
**/""".format(relpath(argv[0]))
PragmaOnce = "#pragma once"


def GenerateCommandNamesH(commandNames):
    c_code = "\n\n".join((
        Disclaimer,
        PragmaOnce,
        GenerateCommandNames(commandNames)))
    return c_code


def GenerateCommandsH(commandNames, maxArgs, numRegisters):
    c_code = "\n\n".join((
        Disclaimer,
        PragmaOnce,
        GenerateCommandCodeEnum(commandNames),
        GenerateCommandArgTypeEnum(),
        GenerateCommandMaxArgs(maxArgs),
        GenerateCommandStruct(),
        GenerateRegisterCodeEnum(numRegisters),
        GenerateNumRegisters(numRegisters),
        GenerateRegisterStruct()))
    return c_code


def GenerateCommandsC(commandDefinitions, numRegisters):
    c_code = "\n\n".join((
        Disclaimer,
        PragmaOnce,
        Include("CommandNames_Gen.h"),
        Include("Commands_Gen.h"),
        GenerateCommandArray(commandDefinitions),
        Include("RegisterNames_Gen.h"),
        GenerateRegisterArray(numRegisters),
    )
    )
    return c_code


def GenerateRegisterNamesH(numRegisters):
    c_code = "\n\n".join((
        Disclaimer,
        PragmaOnce,
        GenerateRegisterNames(numRegisters)))
    return c_code


def GenerateCPUTypesH():
    c_code = "\n\n".join((
        Disclaimer,
        PragmaOnce,
        Include("Commands_Gen.h"),
        GenerateCPUTypes(),
        GenerateCPUData(),
        GenerateCPUPrintFormats(),
        GenerateCPUScanFormats())
    )
    return c_code


def GenerateCPUExecC(srcCodes):
    c_code = "\n\n".join((
        Disclaimer,
        GenerateCPUExecDefCheck(),
        GenerateCPUExecSwitch(srcCodes),
    )
    )
    return c_code


def GenerateAll(prefix=""):
    definitions, srcCodes = getDefinitionsSrcCodes()
    maxArgs = max(d[2] for d in definitions.values())
    numRegisters = 16
    commandNames = definitions.keys()
    with open("include/CommandNames_Gen.h", "w") as f:
        commandNamesH = GenerateCommandNamesH(commandNames)
        f.write(commandNamesH)
    with open("include/RegisterNames_Gen.h", "w") as f:
        registerNamesH = GenerateRegisterNamesH(numRegisters)
        f.write(registerNamesH)
    with open("include/Commands_Gen.h", "w") as f:
        commandsH = GenerateCommandsH(commandNames, maxArgs, numRegisters)
        f.write(commandsH)
    with open("src/Commands_Gen.c", "w") as f:
        commandsC = GenerateCommandsC(definitions, numRegisters)
        f.write(commandsC)
    with open("include/CPUTypes_Gen.h", "w") as f:
        CPUTypesH = GenerateCPUTypesH()
        f.write(CPUTypesH)
    with open("src/CPUExec_Gen.c", "w") as f:
        CPUExecC = GenerateCPUExecC(srcCodes)
        f.write(CPUExecC)


if __name__ == "__main__":
    GenerateAll()
