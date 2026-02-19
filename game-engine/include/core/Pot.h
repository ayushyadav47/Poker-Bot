#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>


namespace poker::core {

/// @brief Represents a single pot (main or side) with eligible players.
struct PotInfo {
  int64_t amount = 0;
  std::unordered_set<size_t> eligiblePlayers; ///< Player IDs eligible to win.
};

/// @brief Manages the pot system including side pot calculation.
///
/// Each player's total contribution is tracked. When side pots are needed
/// (due to all-ins at different stack sizes), calculateSidePots() produces
/// a vector of PotInfo ordered from main pot to successive side pots.
class Pot {
public:
  Pot() = default;

  /// Record a contribution from a player.
  void addContribution(size_t playerId, int64_t amount);

  /// Get total chips in all pots.
  [[nodiscard]] int64_t getTotal() const noexcept;

  /// Get a player's total contribution this hand.
  [[nodiscard]] int64_t getPlayerContribution(size_t playerId) const;

  /// Calculate main pot + side pots based on contributions.
  /// @param foldedPlayers  set of player IDs who folded (ineligible).
  [[nodiscard]] std::vector<PotInfo>
  calculateSidePots(const std::unordered_set<size_t> &foldedPlayers) const;

  /// Reset for a new hand.
  void reset();

private:
  /// playerId -> total contribution this hand.
  std::vector<std::pair<size_t, int64_t>> contributions_;
};

} // namespace poker::core
