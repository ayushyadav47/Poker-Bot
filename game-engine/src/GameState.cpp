#include "core/GameState.h"

#include <sstream>

namespace poker::core {

void GameState::setPlayers(std::vector<Player> players) {
  players_ = std::move(players);
}

void GameState::addCommunityCard(Card c) { communityCards_.push_back(c); }

void GameState::recordAction(Action a) { actionHistory_.push_back(a); }

size_t GameState::getNumActivePlayers() const {
  size_t count = 0;
  for (const auto &p : players_) {
    if (!p.isFolded() && !p.isAllIn()) {
      ++count;
    }
  }
  return count;
}

size_t GameState::getNumPlayersInHand() const {
  size_t count = 0;
  for (const auto &p : players_) {
    if (!p.isFolded()) {
      ++count;
    }
  }
  return count;
}

size_t GameState::getSmallBlindPosition() const noexcept {
  if (players_.size() == 2) {
    return dealerPos_; // Heads-up: dealer is SB
  }
  return (dealerPos_ + 1) % players_.size();
}

size_t GameState::getBigBlindPosition() const noexcept {
  if (players_.size() == 2) {
    return (dealerPos_ + 1) % players_.size();
  }
  return (dealerPos_ + 2) % players_.size();
}

void GameState::resetForNewHand() {
  communityCards_.clear();
  actionHistory_.clear();
  pot_.reset();
  street_ = Street::Preflop;
  for (auto &p : players_) {
    p.resetForNewHand();
  }
}

std::string GameState::serialize() const {
  std::ostringstream oss;
  oss << "=== GameState ===\n";
  oss << "Street: ";
  switch (street_) {
  case Street::Preflop:
    oss << "Preflop";
    break;
  case Street::Flop:
    oss << "Flop";
    break;
  case Street::Turn:
    oss << "Turn";
    break;
  case Street::River:
    oss << "River";
    break;
  case Street::Showdown:
    oss << "Showdown";
    break;
  }
  oss << "\nDealer: " << dealerPos_;
  oss << "\nBlinds: " << smallBlind_ << "/" << bigBlind_;
  oss << "\nPot: " << pot_.getTotal();
  oss << "\nCommunity: [";
  for (size_t i = 0; i < communityCards_.size(); ++i) {
    if (i > 0)
      oss << " ";
    oss << communityCards_[i].toString();
  }
  oss << "]\nPlayers:\n";
  for (const auto &p : players_) {
    oss << "  " << p.getName() << " [" << p.getChips() << " chips]";
    if (p.isFolded())
      oss << " (folded)";
    if (p.isAllIn())
      oss << " (all-in)";
    oss << " Cards: [";
    for (size_t i = 0; i < p.getHoleCards().size(); ++i) {
      if (i > 0)
        oss << " ";
      oss << p.getHoleCards()[i].toString();
    }
    oss << "]\n";
  }
  oss << "Actions:\n";
  for (const auto &a : actionHistory_) {
    oss << "  " << a.toString() << "\n";
  }
  return oss.str();
}

} // namespace poker::core
