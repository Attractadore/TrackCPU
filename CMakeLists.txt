cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_subdirectory("TrackStack")

project(CPUExec)
project(CPUAss)
project(CPUDisAss)
project(CPUCommon)

add_library(CPUCommon SHARED "src/commands.c" "src/util.c")
add_executable(CPUExec "src/execute.c")
add_executable(CPUAss "src/assemble.c")
add_executable(CPUDisAss "src/disassemble.c")

target_include_directories(CPUCommon PUBLIC "include")
target_link_libraries(CPUExec StackLib CPUCommon m)
target_link_libraries(CPUAss CPUCommon)
target_link_libraries(CPUDisAss CPUCommon)
