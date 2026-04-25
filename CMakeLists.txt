cmake_minimum_required(VERSION 4.1)
project(GameBoyEmulator)

set(CMAKE_CXX_STANDARD 20)


add_executable(GameBoyEmulator src/CPU.cpp
        src/main.cpp
        src/Memory.cpp
        src/ALU.cpp
        src/Instructions.cpp)

add_executable(GameBoyEmuTests tests/cpu_tests.cpp
        src/CPU.cpp
        src/Memory.cpp
        src/ALU.cpp
        src/Instructions.cpp
        tests/tests_main.cpp)

#include_directories(include)
include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(Catch2)

target_include_directories(GameBoyEmulator PRIVATE include)
target_include_directories(GameBoyEmuTests PRIVATE include)

target_link_libraries(GameBoyEmuTests PRIVATE Catch2::Catch2WithMain)
