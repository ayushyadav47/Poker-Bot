cmake_minimum_required(VERSION 3.20)
project(PokerEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Static library target ---
set(ENGINE_SOURCES
    src/Card.cpp
    src/Action.cpp
    src/Deck.cpp
    src/Player.cpp
    src/Pot.cpp
    src/BettingRound.cpp
    src/GameState.cpp
    src/HandEvaluator.cpp
    src/RuleEngine.cpp
    src/PokerEngine.cpp
)

add_library(poker_engine STATIC ${ENGINE_SOURCES})
target_include_directories(poker_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# --- Demo executable ---
add_executable(poker_demo examples/main.cpp)
target_link_libraries(poker_demo PRIVATE poker_engine)

# --- Tests ---
# Test source files are in tests/ and use GoogleTest.
# To enable, install GoogleTest locally and uncomment below:
#
# find_package(GTest REQUIRED)
# enable_testing()
# set(TEST_SOURCES
#     tests/test_card.cpp
#     tests/test_deck.cpp
#     tests/test_hand_evaluator.cpp
#     tests/test_pot.cpp
#     tests/test_rule_engine.cpp
#     tests/test_poker_engine.cpp
# )
# add_executable(poker_tests ${TEST_SOURCES})
# target_link_libraries(poker_tests PRIVATE poker_engine GTest::gtest_main)
# include(GoogleTest)
# gtest_discover_tests(poker_tests)
