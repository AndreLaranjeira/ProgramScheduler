cmake_minimum_required(VERSION 3.10)
project(ProgramScheduler C)

set(CMAKE_C_STANDARD 99)

set(EXECUTABLE_OUTPUT_PATH /build-debug)
add_executable(exec util/console.h exec/exec.c)
