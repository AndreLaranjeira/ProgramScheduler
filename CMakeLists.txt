cmake_minimum_required(VERSION 3.10)
project(ProgramScheduler C CXX)

set(CMAKE_C_STANDARD 99)

include_directories(
  include
  src
)
add_executable(execute src/execute.c src/data_structures.c)
add_executable(scheduler src/scheduler.c src/data_structures.c)
add_executable(shutdown src/shutdown.c src/data_structures.c)
add_executable(node src/node.c src/data_structures.c)

# add_executable(tests test/tests.cpp src/data_structures.cpp)
add_executable(dummy src/dummy.c)
