# --- Source Files ---
file(GLOB_RECURSE ENGINE_SOURCES "*.cpp")

# --- Library Target ---
add_library(poker_engine STATIC ${ENGINE_SOURCES})

# --- Include Directories ---
target_include_directories(poker_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)
