#pragma once

#include "core/Card.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>


namespace poker::core {

/// @brief Represents a player at the table.
/// Holds chip stack, hole cards, and per-hand state flags.
class Player {
public:
  Player(size_t id, std::string name, int64_t chips);

  // --- Accessors ---
  [[nodiscard]] size_t getId() const noexcept { return id_; }
  [[nodiscard]] const std::string &getName() const noexcept { return name_; }
  [[nodiscard]] int64_t getChips() const noexcept { return chips_; }
  [[nodiscard]] bool isFolded() const noexcept { return folded_; }
  [[nodiscard]] bool isAllIn() const noexcept { return allIn_; }
  [[nodiscard]] bool isActive() const noexcept {
    return !folded_ && chips_ > 0;
  }
  [[nodiscard]] bool isInHand() const noexcept { return !folded_; }
  [[nodiscard]] int64_t getCurrentBet() const noexcept { return currentBet_; }
  [[nodiscard]] const std::vector<Card> &getHoleCards() const noexcept {
    return holeCards_;
  }

  // --- Mutators ---
  void dealCard(Card c);
  void fold();

  /// Place a bet of the given amount. Returns actual amount bet
  /// (may be less if player goes all-in).
  int64_t placeBet(int64_t amount);

  /// Award chips to this player.
  void awardChips(int64_t amount);

  /// Reset per-hand state (hole cards, fold flag, current bet, all-in).
  void resetForNewHand();

  /// Reset per-round bet tracking.
  void resetCurrentBet() noexcept { currentBet_ = 0; }

private:
  size_t id_;
  std::string name_;
  int64_t chips_;
  std::vector<Card> holeCards_;
  bool folded_ = false;
  bool allIn_ = false;
  int64_t currentBet_ = 0;
};

} // namespace poker::core
