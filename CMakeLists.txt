cmake_minimum_required(VERSION 3.10)
project(ProgramScheduler C)

set(CMAKE_C_STANDARD 99)

add_executable(execute include/console.h src/execute.c)
add_executable(scheduler src/scheduler.c)
add_executable(shutdown include/console.h src/shutdown.c)
add_executable(node src/node.c)
