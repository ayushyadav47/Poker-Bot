#pragma once

#include "core/Action.h"
#include "core/GameState.h"


#include <vector>

namespace poker::core {
class GameState; // forward decl (already included, but for clarity)
}

namespace poker::interfaces {

/// @brief Pure virtual interface for providing player actions.
///
/// Implement this to plug in human input, random strategies, AI modules,
/// GTO solvers, or Python-bound strategies via pybind11.
class IActionProvider {
public:
  virtual ~IActionProvider() = default;

  /// Called by the engine to get the next action for a player.
  /// @param playerId  Index of the player who must act.
  /// @param state     Current game state snapshot.
  /// @param legalActions All legal actions for this player.
  /// @return The chosen action.
  virtual core::Action
  getAction(size_t playerId, const core::GameState &state,
            const std::vector<core::Action> &legalActions) = 0;
};

} // namespace poker::interfaces
