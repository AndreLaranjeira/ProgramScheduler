cmake_minimum_required(VERSION 3.10)
project(ProgramScheduler C)

set(CMAKE_C_STANDARD 99)

include_directories(include)
add_executable(execute src/execute.c)
add_executable(scheduler src/scheduler.c)
add_executable(shutdown src/shutdown.c)
add_executable(node src/node.c)
add_executable(dummy src/dummy.c)
