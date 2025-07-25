cmake_minimum_required(VERSION 3.28)
project(RISCV_Simulator)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
add_compile_options(-Wall)

add_executable(RISCV_Simulator src/main.cpp)

target_include_directories(RISCV_Simulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)