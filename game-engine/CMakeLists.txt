cmake_minimum_required(VERSION 3.20)
project(PokerEngine VERSION 1.0.0 LANGUAGES CXX)

# --- Options ---
option(BUILD_POKER_ENGINE "Build the PokerEngine library" ON)
option(BUILD_EXAMPLES "Build example executables" ON)
option(BUILD_TESTING "Build unit tests" ON)

# --- Standard & Compiler Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Subdirectories ---
if(BUILD_POKER_ENGINE)
    add_subdirectory(src)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
