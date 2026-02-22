include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
enable_testing()

add_executable(poker_tests
  test_card.cpp
  test_deck.cpp
  test_hand_evaluator.cpp
  test_poker_engine.cpp
  test_pot.cpp
  test_rule_engine.cpp
)

target_link_libraries(poker_tests
    PRIVATE
        poker_engine
        GTest::gtest_main
)

gtest_discover_tests(poker_tests)
