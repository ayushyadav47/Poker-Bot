#pragma once

#include "core/Deck.h"
#include "core/GameState.h"
#include "engine/RuleEngine.h"
#include "interfaces/IActionProvider.h"
#include "interfaces/IRandomGenerator.h"

#include <functional>
#include <memory>

namespace poker::engine {

/// @brief Callback type for hand events (optional observer pattern).
using HandEventCallback =
    std::function<void(const std::string &event, const core::GameState &state)>;

/// @brief The main game engine that drives a complete Texas Hold'em hand.
///
/// PokerEngine controls the lifecycle:
///   deal → blinds → preflop → flop → turn → river → showdown → settle
///
/// It delegates action selection to IActionProvider and action validation
/// to RuleEngine. The engine itself contains no strategy logic.
class PokerEngine {
public:
  /// @param actionProvider  Provides player actions (strategy, human, AI).
  /// @param rng             Random generator for deck shuffling.
  PokerEngine(std::shared_ptr<interfaces::IActionProvider> actionProvider,
              std::shared_ptr<interfaces::IRandomGenerator> rng);

  /// Set the event callback for observing hand progress.
  void setEventCallback(HandEventCallback callback);

  /// Play one complete hand. Modifies state in-place.
  void playHand(core::GameState &state);

private:
  void postBlinds(core::GameState &state);
  void dealHoleCards(core::GameState &state);
  void dealCommunityCards(core::GameState &state, size_t count);
  void runBettingRound(core::GameState &state);
  void showdown(core::GameState &state);
  void settleHand(core::GameState &state);

  /// Get the first player to act for this street.
  [[nodiscard]] size_t getFirstToAct(const core::GameState &state) const;

  /// Check if hand should end early (all but one folded, or all-in showdown).
  [[nodiscard]] bool isHandOver(const core::GameState &state) const;

  void emitEvent(const std::string &event, const core::GameState &state);

  std::shared_ptr<interfaces::IActionProvider> actionProvider_;
  std::shared_ptr<interfaces::IRandomGenerator> rng_;
  HandEventCallback eventCallback_;
  core::Deck deck_;
};

} // namespace poker::engine
