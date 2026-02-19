#include "core/Deck.h"

#include <algorithm>
#include <random>

namespace poker::core {

Deck::Deck() { reset(); }

void Deck::shuffle(interfaces::IRandomGenerator &rng) {
  dealIndex_ = 0;
  rng.shuffle(cards_);
}

std::optional<Card> Deck::deal() {
  if (dealIndex_ >= cards_.size()) {
    return std::nullopt;
  }
  return cards_[dealIndex_++];
}

void Deck::reset() {
  cards_.clear();
  cards_.reserve(52);
  for (uint8_t s = 0; s < 4; ++s) {
    for (uint8_t r = 2; r <= 14; ++r) {
      cards_.emplace_back(static_cast<Rank>(r), static_cast<Suit>(s));
    }
  }
  dealIndex_ = 0;
}

size_t Deck::remaining() const noexcept { return cards_.size() - dealIndex_; }

// --- Mt19937Generator ---

Mt19937Generator::Mt19937Generator(uint64_t seed) : engine_(seed) {}

Mt19937Generator::Mt19937Generator() : engine_(std::random_device{}()) {}

void Mt19937Generator::shuffle(std::vector<Card> &cards) {
  std::shuffle(cards.begin(), cards.end(), engine_);
}

} // namespace poker::core
