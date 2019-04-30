cmake_minimum_required(VERSION 3.10)
project(ProgramScheduler C)

set(CMAKE_C_STANDARD 99)

add_executable(exec util/console.h exec/exec.c)
add_executable(scheduler scheduler/scheduler.c)
add_executable(shutdown shutdown/shutdown.c)
add_executable(node node/node.c)
