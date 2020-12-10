cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_subdirectory("TrackStack")

project(CPUExec)
project(CPUAss)
project(CPUDisAss)
project(CPUCommon)
project(CPULabels)

add_custom_command(OUTPUT 
                    "src/Commands_Gen.c" "src/CPUExec_Gen.c"
                    "include/CPUTypes_Gen.h" "include/CommandNames_Gen.h"
                    "include/RegisterNames_Gen.h" "include/Commands_Gen.h"
                    DEPENDS "codegen.py"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMAND python3 ./codegen.py && clang-format -i {src/*_Gen.c,include/*_Gen.h})
add_custom_target(CPUGen DEPENDS
                         "src/Commands_Gen.c" "src/CPUExec_Gen.c"
                         "include/CPUTypes_Gen.h" "include/CommandNames_Gen.h"
                         "include/RegisterNames_Gen.h" "include/Commands_Gen.h")
add_library(CPUCommon SHARED "src/util.c" "src/commands.c")
add_library(CPULabels SHARED "src/label.c")
add_executable(CPUExec "src/execute.c")
add_executable(CPUAss "src/assemble.c")
add_executable(CPUDisAss "src/disassemble.c")

target_include_directories(CPUCommon PUBLIC "include")
add_dependencies(CPUCommon CPUGen)

target_include_directories(CPULabels PUBLIC "include")
add_dependencies(CPULabels CPUGen)

target_link_libraries(CPUExec StackLib CPUCommon m)
add_dependencies(CPUExec CPUGen)

target_link_libraries(CPUAss CPUCommon CPULabels)
target_link_libraries(CPUDisAss CPUCommon CPULabels)
