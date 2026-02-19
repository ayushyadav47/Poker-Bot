#pragma once

#include "core/Action.h"
#include "core/GameState.h"


#include <vector>

namespace poker::interfaces {

/// @brief Pure virtual interface for a player's strategy.
///
/// This is the high-level strategy interface that external modules
/// (GTO solver, RL agent, Python AI) should implement.
/// It receives the game state and legal actions and returns a decision.
class IPlayerStrategy {
public:
  virtual ~IPlayerStrategy() = default;

  /// Choose an action given the game state and legal actions.
  virtual core::Action
  getAction(const core::GameState &state,
            const std::vector<core::Action> &legalActions) = 0;
};

} // namespace poker::interfaces
