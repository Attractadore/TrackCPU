/**
THIS FILE WAS GENERATED AUTOMATICALLY
IF YOU WANT TO ALTER IT MAKE CHANGES TO codegen.py
**/

#ifndef READ
#error READ not defined
#endif
#ifndef HALT
#error HALT not defined
#endif
#ifndef IP
#error IP not defined
#endif
#ifndef DATA_TOP
#error DATA_TOP not defined
#endif
#ifndef PRINT
#error PRINT not defined
#endif
#ifndef CMD_CODE
#error CMD_CODE not defined
#endif
#ifndef DATA_PUSH
#error DATA_PUSH not defined
#endif
#ifndef WRITE_REGISTER
#error WRITE_REGISTER not defined
#endif
#ifndef IP_POP
#error IP_POP not defined
#endif
#ifndef LOAD_VALUE
#error LOAD_VALUE not defined
#endif
#ifndef SCAN
#error SCAN not defined
#endif
#ifndef STORE_VALUE
#error STORE_VALUE not defined
#endif
#ifndef UNKNOWN_COMMAND
#error UNKNOWN_COMMAND not defined
#endif
#ifndef IP_PUSH
#error IP_PUSH not defined
#endif
#ifndef READ_REGISTER
#error READ_REGISTER not defined
#endif
#ifndef DATA_POP
#error DATA_POP not defined
#endif

switch (CMD_CODE) {
    case CMD_HALT_CODE: {
        HALT();
    } break;
    case CMD_CALL_CODE: {
        READ(CPUAddr, arg0);
        IP_PUSH();
        IP = arg0;
    } break;
    case CMD_RETURN_CODE: {
        IP_POP();
    } break;
    case CMD_JUMP_CODE: {
        READ(CPUAddr, arg0);
        IP = arg0;
    } break;
    case CMD_JUMPT_CODE: {
        READ(CPUAddr, arg0);
        DATA_POP(CPUUInt, arg1);
        if (arg1) {
            IP = arg0;
        }
    } break;
    case CMD_JUMPF_CODE: {
        READ(CPUAddr, arg0);
        DATA_POP(CPUUInt, arg1);
        if (!arg1) {
            IP = arg0;
        }
    } break;
    case CMD_PUSH_CODE: {
        READ(RegisterCode, arg0);
        READ_REGISTER(arg0, RegisterCode, arg1);
        DATA_PUSH(CPUUInt, arg1);
    } break;
    case CMD_PUSHI_CODE: {
        READ(CPUInt, arg0);
        DATA_PUSH(CPUInt, arg0);
    } break;
    case CMD_PUSHU_CODE: {
        READ(CPUUInt, arg0);
        DATA_PUSH(CPUUInt, arg0);
    } break;
    case CMD_PUSHF_CODE: {
        READ(CPUFloat, arg0);
        DATA_PUSH(CPUFloat, arg0);
    } break;
    case CMD_POP_CODE: {
        READ(RegisterCode, arg0);
        DATA_POP(CPUUInt, arg1);
        WRITE_REGISTER(arg0, CPUUInt, arg1);
    } break;
    case CMD_POPS_CODE: {
        DATA_POP(CPUUInt, arg0);
    } break;
    case CMD_TOP_CODE: {
        READ(RegisterCode, arg0);
        DATA_TOP(CPUUInt, arg1);
        WRITE_REGISTER(arg0, CPUUInt, arg1);
    } break;
    case CMD_EQI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 == arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_EQU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 == arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_EQF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 == arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_NEI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 != arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_NEU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 != arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_NEF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 != arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_LI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 < arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_LU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 < arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_LF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 < arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_LEI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 <= arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_LEU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 <= arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_LEF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 <= arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_GI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 > arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_GU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 > arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_GF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 > arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_GEI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 >= arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_GEU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 >= arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_GEF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 >= arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_BAND_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 & arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_AND_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 && arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_BOR_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 | arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_OR_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 || arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_BXOR_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 ^ arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_BNOT_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            CPUUInt arg1 = ~arg0;
            DATA_PUSH(CPUUInt, arg1);
        }
    } break;
    case CMD_NOT_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            CPUUInt arg1 = !arg0;
            DATA_PUSH(CPUUInt, arg1);
        }
    } break;
    case CMD_LS_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 << arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_RS_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 >> arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_LRS_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 >> arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_ADDI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 + arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_ADDU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 + arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_ADDF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 + arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_SUBI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 - arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_SUBU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 - arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_SUBF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 - arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_MULI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 * arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_MULU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 * arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_MULF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 * arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_DIVI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 / arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_DIVU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 / arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_DIVF_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            DATA_POP(CPUFloat, arg1);
            CPUFloat arg2 = arg0 / arg1;
            DATA_PUSH(CPUFloat, arg2);
        }
    } break;
    case CMD_MODI_CODE: {
        {
            DATA_POP(CPUInt, arg0);
            DATA_POP(CPUInt, arg1);
            CPUInt arg2 = arg0 % arg1;
            DATA_PUSH(CPUInt, arg2);
        }
    } break;
    case CMD_MODU_CODE: {
        {
            DATA_POP(CPUUInt, arg0);
            DATA_POP(CPUUInt, arg1);
            CPUUInt arg2 = arg0 % arg1;
            DATA_PUSH(CPUUInt, arg2);
        }
    } break;
    case CMD_SQRT_CODE: {
        {
            DATA_POP(CPUFloat, arg0);
            CPUFloat arg1 = sqrt(arg0);
            DATA_PUSH(CPUFloat, arg1);
        }
    } break;
    case CMD_PRINTI_CODE: {
        DATA_TOP(CPUInt, arg0);
        PRINT("%" CPUINT_PRINT_FMT "\n", arg0);
    } break;
    case CMD_PRINTU_CODE: {
        DATA_TOP(CPUUInt, arg0);
        PRINT("%" CPUUINT_PRINT_FMT "\n", arg0);
    } break;
    case CMD_PRINTF_CODE: {
        DATA_TOP(CPUFloat, arg0);
        PRINT("%" CPUFLOAT_PRINT_FMT "\n", arg0);
    } break;
    case CMD_SCANI_CODE: {
        SCAN("%" CPUINT_SCAN_FMT, CPUInt, arg0);
        DATA_PUSH(CPUInt, arg0);
    } break;
    case CMD_SCANU_CODE: {
        SCAN("%" CPUUINT_SCAN_FMT, CPUUInt, arg0);
        DATA_PUSH(CPUUInt, arg0);
    } break;
    case CMD_SCANF_CODE: {
        SCAN("%" CPUFLOAT_SCAN_FMT, CPUFloat, arg0);
        DATA_PUSH(CPUFloat, arg0);
    } break;
    case CMD_LOAD_CODE: {
        READ(RegisterCode, arg0);
        READ_REGISTER(arg0, CPUUInt, load_temp);
        LOAD_VALUE(load_temp, CPUUInt, arg1);
        DATA_PUSH(CPUUInt, arg1);
    } break;
    case CMD_STORE_CODE: {
        READ(RegisterCode, arg0);
        DATA_TOP(CPUUInt, arg1);
        {
            READ_REGISTER(arg0, CPUUInt, store_temp);
            STORE_VALUE(store_temp, CPUUInt, arg1);
        }
    } break;
    case CMD_MOVE_CODE: {
        READ(RegisterCode, arg0);
        DATA_POP(CPUUInt, arg1);
        {
            READ_REGISTER(arg0, CPUUInt, store_temp);
            STORE_VALUE(store_temp, CPUUInt, arg1);
        }
    } break;
    default:
        UNKNOWN_COMMAND();
}