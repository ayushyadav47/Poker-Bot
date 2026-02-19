#include "core/Player.h"

#include <algorithm>
#include <stdexcept>

namespace poker::core {

Player::Player(size_t id, std::string name, int64_t chips)
    : id_(id), name_(std::move(name)), chips_(chips) {
  if (chips < 0) {
    throw std::invalid_argument("Player cannot start with negative chips");
  }
}

void Player::dealCard(Card c) {
  if (holeCards_.size() >= 2) {
    throw std::logic_error("Player already has 2 hole cards");
  }
  holeCards_.push_back(c);
}

void Player::fold() { folded_ = true; }

int64_t Player::placeBet(int64_t amount) {
  int64_t actual = std::min(amount, chips_);
  chips_ -= actual;
  currentBet_ += actual;
  if (chips_ == 0) {
    allIn_ = true;
  }
  return actual;
}

void Player::awardChips(int64_t amount) { chips_ += amount; }

void Player::resetForNewHand() {
  holeCards_.clear();
  folded_ = false;
  allIn_ = false;
  currentBet_ = 0;
}

} // namespace poker::core
