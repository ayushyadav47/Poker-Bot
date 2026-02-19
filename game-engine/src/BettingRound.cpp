#include "core/BettingRound.h"

namespace poker::core {

BettingRound::BettingRound(size_t numPlayers, size_t firstToAct,
                           int64_t existingBet)
    : numPlayers_(numPlayers), currentIndex_(firstToAct),
      currentBet_(existingBet), lastAggressor_(firstToAct),
      playersToAct_(numPlayers) {}

void BettingRound::playerActed(int64_t newBet) {
  ++actionsThisRound_;
  if (newBet > currentBet_) {
    currentBet_ = newBet;
    ++numRaises_;
    lastAggressor_ = currentIndex_;
    // Everyone else needs to act again.
    playersToAct_ = numPlayers_ - 1; // minus current player
  } else {
    if (playersToAct_ > 0) {
      --playersToAct_;
    }
  }
}

bool BettingRound::isComplete() const noexcept {
  return actionsThisRound_ > 0 && playersToAct_ == 0;
}

} // namespace poker::core
