# Poker Bot

A comprehensive poker bot project featuring a custom high-performance game engine, with planned integrations for GTO solvers and real-time AI decision making.

## Project Structure

### ✅ Game Engine (`game-engine/`)
A C++20-based Texas Hold'em poker engine that handles:
- **Game State Management:** Efficient tracking of players, chips, pots, and cards.
- **Rule Enforcement:** Strict validation of betting rules (Blind, Pre-Flop, Flop, Turn, River).
- **Hand Evaluation:** Standard 5-card hand ranking.
- **Extensible AI:** `IActionProvider` interface for plugging in bots or human players.

### 🚧 Planned Modules
- **gto-solver**: Tools for Game Theory Optimal (GTO) strategy calculation.
- **real-time AI**: Real-time decision making components.
- **strategy-database**: Storage for strategies and hand histories.

## Getting Started

### Prerequisites
- CMake 3.20+
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 19.29+)

### Building the Game Engine

1. Navigate to the game engine directory:
   ```bash
   cd game-engine
   ```

2. Build using CMake:
   ```bash
   cmake -S . -B build
   cmake --build build
   ```

3. Run the demo:
   ```bash
   ./build/poker_demo
   ```

## Testing

To run the game engine unit tests:

1. Navigate to the `game-engine` directory.
2. Ensure testing is enabled in `CMakeLists.txt` (uncomment the testing section).
3. Run the tests:
   ```bash
   cmake -S . -B build
   cmake --build build
   ctest --test-dir build --output-on-failure
   ```
