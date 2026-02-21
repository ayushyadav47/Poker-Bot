# Poker Engine

A high-performance, C++20-based Texas Hold'em poker engine.

## Overview

This library provides the core logic for simulating poker games, designed for flexibility and performance. It handles:

-   **Game State Management:** efficient tracking of players, chips, pots, side pots, and community cards.
-   **Rule Enforcement:** strict validation of Texas Hold'em betting rules (Blind, Pre-Flop, Flop, Turn, River).
-   **Hand Evaluation:** standard 5-card hand ranking.
-   **Action Abstraction:** `IActionProvider` interface allows seamless integration of human inputs, heuristic bots, or AI agents.

## Build Instructions

**Prerequisites:**
-   CMake 3.20 or newer
-   C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 19.29+)

**Build Steps:**

```bash
cmake -S . -B build
cmake --build build
```

## Running the Demo

The project includes an interactive command-line demo (`poker_demo`) where you can play against a passive bot.

```bash
./build/poker_demo
```

## Testing

Unit tests are located in the `tests/` directory and utilize the **GoogleTest** framework.

**Note:** The testing configuration is currently commented out in `CMakeLists.txt`.

**To enable and run tests:**

1.  Open `CMakeLists.txt`.
2.  Uncomment the testing section at the bottom of the file (lines related to `GTest` and `poker_tests`).
3.  Ensure GoogleTest is installed or available to CMake.
4.  Rebuild and run tests:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Project Structure

-   `src/`: Implementation of the core engine logic.
-   `include/`: Public header files, organized by module (`core`, `engine`, `interfaces`, `utils`).
-   `examples/`: Example implementations, including the `poker_demo` CLI.
-   `tests/`: Unit tests for individual components (`Card`, `Deck`, `HandEvaluator`, etc.).

## Key Components

-   **`PokerEngine`**: The central controller that manages the flow of the game, transitions between betting rounds, and enforces rules.
-   **`GameState`**: A snapshot of the current game, including player statuses, pot amounts, and board cards.
-   **`IActionProvider`**: The interface you must implement to define player behavior. See `examples/main.cpp` for a reference implementation.
