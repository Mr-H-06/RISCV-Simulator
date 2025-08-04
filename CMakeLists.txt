cmake_minimum_required(VERSION 3.28)
project(RISCV_Simulator)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
#add_compile_options(-Wall)

add_executable(code
        src/main.cpp
        #[[src/alu.h
        src/lsb.h
        src/memory.h
        src/returnlib.h
        src/predictor.h
        src/rf.h
        src/rob.h
        src/rs.h
        src/simulator.h]]#
)

target_include_directories(code PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)