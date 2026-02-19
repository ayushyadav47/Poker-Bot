#pragma once

#include "core/Action.h"
#include "core/GameState.h"


#include <vector>

namespace poker::engine {

/// @brief Validates player actions against the current game state.
///
/// RuleEngine is stateless — all validation is based on the GameState
/// snapshot passed to each method.
class RuleEngine {
public:
  /// Get all legal actions for a player given the current game state.
  [[nodiscard]] static std::vector<core::Action>
  getLegalActions(const core::GameState &state, size_t playerId);

  /// Check if a specific action is legal.
  [[nodiscard]] static bool isActionLegal(const core::GameState &state,
                                          const core::Action &action);

  /// Minimum raise size according to NLHE rules.
  [[nodiscard]] static int64_t getMinRaise(const core::GameState &state,
                                           size_t playerId);

  /// Maximum raise (player's remaining stack).
  [[nodiscard]] static int64_t getMaxRaise(const core::GameState &state,
                                           size_t playerId);

  /// Amount required to call.
  [[nodiscard]] static int64_t getCallAmount(const core::GameState &state,
                                             size_t playerId);
};

} // namespace poker::engine
