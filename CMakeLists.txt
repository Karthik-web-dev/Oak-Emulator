cmake_minimum_required(VERSION 3.20)
project(Oak-Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# ----------------------------
# Core library (NO main.cpp)
# ----------------------------
add_library(oak_core
    src/GameBoy.cpp
    src/core/MMU.cpp
    src/cpu/CPU.cpp
    src/cpu/Instructions.cpp
    src/memory/Cartridge.cpp
    src/memory/IORegisters.cpp
    src/memory/Ram.cpp
    src/ppu/PPU.cpp
    src/ppu/Renderer.cpp
)

# ----------------------------
# Emulator executable
# ----------------------------
add_executable(Oak-Emulator
    src/main.cpp
)

target_link_libraries(Oak-Emulator PRIVATE oak_core)

# ----------------------------
# Catch2 (auto download)
# ----------------------------
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(Catch2)

# ----------------------------
# Tests executable
# ----------------------------
add_executable(oak_tests
    tests/cpu_tests.cpp
    tests/tests_main.cpp
    # add more test files here
)

target_link_libraries(oak_tests PRIVATE
    oak_core
    Catch2::Catch2WithMain
)

# ----------------------------
# Enable testing
# ----------------------------
include(CTest)
include(Catch)
catch_discover_tests(oak_tests)
