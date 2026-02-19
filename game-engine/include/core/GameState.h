#pragma once

#include "core/Action.h"
#include "core/BettingRound.h"
#include "core/Card.h"
#include "core/Player.h"
#include "core/Pot.h"


#include <cstdint>
#include <optional>
#include <string>
#include <vector>


namespace poker::core {

/// @brief Complete, queryable snapshot of the current hand state.
///
/// GameState is the primary data object shared with external modules
/// (solvers, AI, replay systems). It contains NO strategy logic.
/// It is designed to be serializable for hand history replay.
class GameState {
public:
  GameState() = default;

  // --- Setup ---
  void setPlayers(std::vector<Player> players);
  void setDealerPosition(size_t pos) noexcept { dealerPos_ = pos; }
  void setSmallBlind(int64_t sb) noexcept { smallBlind_ = sb; }
  void setBigBlind(int64_t bb) noexcept { bigBlind_ = bb; }

  // --- State transitions ---
  void setStreet(Street s) noexcept { street_ = s; }
  void addCommunityCard(Card c);
  void recordAction(Action a);
  void setCurrentPlayerIndex(size_t idx) noexcept { currentPlayerIdx_ = idx; }

  // --- Queries ---
  [[nodiscard]] const std::vector<Player> &getPlayers() const noexcept {
    return players_;
  }
  [[nodiscard]] std::vector<Player> &getMutablePlayers() noexcept {
    return players_;
  }
  [[nodiscard]] const Player &getPlayer(size_t id) const {
    return players_.at(id);
  }
  [[nodiscard]] Player &getMutablePlayer(size_t id) { return players_.at(id); }

  [[nodiscard]] const std::vector<Card> &getCommunityCards() const noexcept {
    return communityCards_;
  }
  [[nodiscard]] Street getStreet() const noexcept { return street_; }
  [[nodiscard]] size_t getDealerPosition() const noexcept { return dealerPos_; }
  [[nodiscard]] size_t getCurrentPlayerIndex() const noexcept {
    return currentPlayerIdx_;
  }
  [[nodiscard]] int64_t getSmallBlind() const noexcept { return smallBlind_; }
  [[nodiscard]] int64_t getBigBlind() const noexcept { return bigBlind_; }

  [[nodiscard]] Pot &getMutablePot() noexcept { return pot_; }
  [[nodiscard]] const Pot &getPot() const noexcept { return pot_; }

  [[nodiscard]] const std::vector<Action> &getActionHistory() const noexcept {
    return actionHistory_;
  }
  [[nodiscard]] size_t getNumActivePlayers() const;
  [[nodiscard]] size_t getNumPlayersInHand() const;

  [[nodiscard]] size_t getSmallBlindPosition() const noexcept;
  [[nodiscard]] size_t getBigBlindPosition() const noexcept;

  /// Reset for a new hand, keeping players and chip stacks.
  void resetForNewHand();

  // --- Serialization ---
  [[nodiscard]] std::string serialize() const;

private:
  std::vector<Player> players_;
  std::vector<Card> communityCards_;
  Pot pot_;
  Street street_ = Street::Preflop;

  size_t dealerPos_ = 0;
  size_t currentPlayerIdx_ = 0;
  int64_t smallBlind_ = 0;
  int64_t bigBlind_ = 0;

  std::vector<Action> actionHistory_;
};

} // namespace poker::core
