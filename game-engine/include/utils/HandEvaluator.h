#pragma once

#include "core/Card.h"

#include <array>
#include <compare>
#include <cstdint>
#include <span>
#include <string>
#include <vector>


namespace poker::utils {

/// Hand ranking categories, ordered from weakest to strongest.
enum class HandRank : uint8_t {
  HighCard,
  Pair,
  TwoPair,
  ThreeOfAKind,
  Straight,
  Flush,
  FullHouse,
  FourOfAKind,
  StraightFlush,
  RoyalFlush
};

/// @brief The result of evaluating a 5-card hand.
/// Comparable: higher is better.
struct HandResult {
  HandRank rank;
  /// Kickers for tie-breaking, ordered highest-first.
  std::array<uint8_t, 5> kickers = {};

  auto operator<=>(const HandResult &other) const noexcept {
    if (rank != other.rank)
      return rank <=> other.rank;
    return kickers <=> other.kickers;
  }
  bool operator==(const HandResult &) const noexcept = default;

  [[nodiscard]] std::string toString() const;
  [[nodiscard]] static std::string rankName(HandRank r);
};

/// @brief Evaluates poker hands.
///
/// Given up to 7 cards (2 hole + 5 community), finds the best
/// 5-card hand by exhaustive C(7,5)=21 combination search.
/// This is simple, correct, and fast enough for game-play purposes.
class HandEvaluator {
public:
  /// Evaluate the best 5-card hand from a set of cards (5–7 cards).
  [[nodiscard]] static HandResult evaluate(std::span<const core::Card> cards);

  /// Compare two players' hands. Returns <0, 0, >0.
  [[nodiscard]] static int compare(std::span<const core::Card> hand1,
                                   std::span<const core::Card> hand2);

private:
  /// Evaluate exactly 5 cards.
  [[nodiscard]] static HandResult
  evaluate5(std::span<const core::Card, 5> cards);
};

} // namespace poker::utils
