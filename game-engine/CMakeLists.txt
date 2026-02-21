cmake_minimum_required(VERSION 3.20)
project(PokerEngine VERSION 1.0.0 LANGUAGES CXX)

# --- Options ---
option(BUILD_POKER_ENGINE "Build the PokerEngine library" ON)
option(BUILD_EXAMPLES "Build example executables" ON)
option(BUILD_TESTING "Build unit tests" ON)
option(BUILD_GUI "Build GUI examples with SFML and ImGui" ON)

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

# --- Dependencies ---
if(BUILD_GUI)
    include(FetchContent)

    # SFML
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "Disable SFML Audio" FORCE)
    FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.1
    )
    FetchContent_MakeAvailable(sfml)

    # ImGui
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.90.4-docking
    )
    FetchContent_MakeAvailable(imgui)

    # ImGui-SFML
    set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "Disable finding SFML" FORCE)
    FetchContent_Declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG        v2.6
    )
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
    FetchContent_MakeAvailable(imgui-sfml)
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
