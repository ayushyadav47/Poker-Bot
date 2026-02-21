#pragma once

#include <cstdint>
#include <cstddef>

namespace poker::core {

/// The current street / phase of the hand.
enum class Street : uint8_t { Preflop, Flop, Turn, River, Showdown };

/// @brief Tracks state within a single betting round.
class BettingRound {
public:
  explicit BettingRound(size_t numPlayers, size_t firstToAct,
                        int64_t existingBet = 0);

  /// Advance to the next player who hasn't folded or gone all-in.
  /// @param folded  function(playerId) -> true if folded or all-in
  template <typename Pred> void advanceToNextActive(Pred isFoldedOrAllIn) {
    do {
      currentIndex_ = (currentIndex_ + 1) % numPlayers_;
    } while (isFoldedOrAllIn(currentIndex_) && ++skipped_ < numPlayers_);
  }

  /// Mark that the current player has acted.
  void playerActed(int64_t newBet);

  /// Has every active player had a chance to act and the bets are settled?
  [[nodiscard]] bool isComplete() const noexcept;

  [[nodiscard]] size_t getCurrentPlayerIndex() const noexcept {
    return currentIndex_;
  }
  [[nodiscard]] int64_t getCurrentBet() const noexcept { return currentBet_; }
  [[nodiscard]] size_t getActionsThisRound() const noexcept {
    return actionsThisRound_;
  }
  [[nodiscard]] size_t getNumRaises() const noexcept { return numRaises_; }

  void setLastAggressor(size_t idx) noexcept { lastAggressor_ = idx; }
  [[nodiscard]] size_t getLastAggressor() const noexcept {
    return lastAggressor_;
  }

private:
  size_t numPlayers_;
  size_t currentIndex_;
  int64_t currentBet_;
  size_t actionsThisRound_ = 0;
  size_t numRaises_ = 0;
  size_t lastAggressor_;
  size_t playersToAct_;
  size_t skipped_ = 0;
};

} // namespace poker::core
