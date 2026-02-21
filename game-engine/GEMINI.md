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

The project uses standard CMake commands. By default, it builds the library, examples, and tests.

```bash
cmake -S . -B build
cmake --build build
```

**Build Options:**

You can toggle components using CMake options:
-   `BUILD_POKER_ENGINE` (Default: ON): Build the core library.
-   `BUILD_EXAMPLES` (Default: ON): Build example executables.
-   `BUILD_TESTING` (Default: ON): Build unit tests (requires internet to fetch GoogleTest).

Example:
```bash
cmake -S . -B build -DBUILD_TESTING=OFF
```

## Running the Demo

The project includes an interactive command-line demo (`poker_demo`) where you can play against a passive bot.

```bash
./build/examples/poker_demo
```

## Testing

Unit tests are located in the `tests/` directory and utilize the **GoogleTest** framework. Tests are enabled by default.

**To run tests:**

1.  Build the project (ensure `BUILD_TESTING` is ON).
2.  Run CTest:

```bash
ctest --test-dir build --output-on-failure
```

## Project Structure

-   `src/`: Implementation of the core engine logic.
-   `include/`: Public header files, organized by module (`core`, `engine`, `interfaces`, `utils`).
-   `examples/`: Example implementations, including the `poker_demo.cpp` CLI.
-   `tests/`: Unit tests for individual components (`Card`, `Deck`, `HandEvaluator`, etc.).

## Key Components

-   **`PokerEngine`**: The central controller that manages the flow of the game, transitions between betting rounds, and enforces rules.
-   **`GameState`**: A snapshot of the current game, including player statuses, pot amounts, and board cards.
-   **`IActionProvider`**: The interface you must implement to define player behavior. See `examples/poker_demo.cpp` for a reference implementation.
