#include "utils/HandEvaluator.h"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace poker::utils {

// --- HandResult ---

std::string HandResult::rankName(HandRank r) {
  switch (r) {
  case HandRank::HighCard:
    return "High Card";
  case HandRank::Pair:
    return "Pair";
  case HandRank::TwoPair:
    return "Two Pair";
  case HandRank::ThreeOfAKind:
    return "Three of a Kind";
  case HandRank::Straight:
    return "Straight";
  case HandRank::Flush:
    return "Flush";
  case HandRank::FullHouse:
    return "Full House";
  case HandRank::FourOfAKind:
    return "Four of a Kind";
  case HandRank::StraightFlush:
    return "Straight Flush";
  case HandRank::RoyalFlush:
    return "Royal Flush";
  }
  return "Unknown";
}

std::string HandResult::toString() const { return rankName(rank); }

// --- HandEvaluator internals ---

namespace {

/// Check for flush: all 5 cards same suit.
bool isFlush(std::span<const core::Card, 5> cards) {
  auto suit = cards[0].suit;
  for (size_t i = 1; i < 5; ++i) {
    if (cards[i].suit != suit)
      return false;
  }
  return true;
}

/// Check for straight. Cards must be sorted descending by rank.
/// Returns the high card rank of the straight, or 0 if not a straight.
/// Handles the A-2-3-4-5 (wheel) special case.
uint8_t straightHighCard(const std::array<uint8_t, 5> &ranks) {
  // Normal straight: consecutive descending
  if (ranks[0] - ranks[4] == 4 && ranks[0] != ranks[1] &&
      ranks[1] != ranks[2] && ranks[2] != ranks[3] && ranks[3] != ranks[4]) {
    return ranks[0];
  }
  // Wheel: A-5-4-3-2
  if (ranks[0] == 14 && ranks[1] == 5 && ranks[2] == 4 && ranks[3] == 3 &&
      ranks[4] == 2) {
    return 5; // 5-high straight
  }
  return 0;
}

} // anonymous namespace

HandResult HandEvaluator::evaluate5(std::span<const core::Card, 5> cards) {
  // Get sorted ranks (descending).
  std::array<uint8_t, 5> ranks;
  for (size_t i = 0; i < 5; ++i) {
    ranks[i] = static_cast<uint8_t>(cards[i].rank);
  }
  std::sort(ranks.begin(), ranks.end(), std::greater<>());

  bool flush = isFlush(cards);
  uint8_t straightHigh = straightHighCard(ranks);
  bool straight = (straightHigh > 0);

  // Count rank frequencies.
  std::array<size_t, 15> freq = {}; // index by rank value (2-14)
  for (auto r : ranks)
    freq[r]++;

  // Categorize by frequency pattern.
  size_t fours = 0, threes = 0, pairs = 0;
  uint8_t fourRank = 0, threeRank = 0;
  std::array<uint8_t, 2> pairRanks = {};
  size_t pairIdx = 0;

  for (int r = 14; r >= 2; --r) {
    if (freq[r] == 4) {
      fours++;
      fourRank = static_cast<uint8_t>(r);
    } else if (freq[r] == 3) {
      threes++;
      threeRank = static_cast<uint8_t>(r);
    } else if (freq[r] == 2 && pairIdx < 2) {
      pairs++;
      pairRanks[pairIdx++] = static_cast<uint8_t>(r);
    }
  }

  HandResult result;

  if (straight && flush) {
    if (straightHigh == 14) {
      result.rank = HandRank::RoyalFlush;
    } else {
      result.rank = HandRank::StraightFlush;
    }
    result.kickers = {straightHigh, 0, 0, 0, 0};
  } else if (fours > 0) {
    result.rank = HandRank::FourOfAKind;
    uint8_t kicker = 0;
    for (auto r : ranks) {
      if (r != fourRank) {
        kicker = r;
        break;
      }
    }
    result.kickers = {fourRank, kicker, 0, 0, 0};
  } else if (threes > 0 && pairs > 0) {
    result.rank = HandRank::FullHouse;
    result.kickers = {threeRank, pairRanks[0], 0, 0, 0};
  } else if (flush) {
    result.rank = HandRank::Flush;
    result.kickers = {ranks[0], ranks[1], ranks[2], ranks[3], ranks[4]};
  } else if (straight) {
    result.rank = HandRank::Straight;
    result.kickers = {straightHigh, 0, 0, 0, 0};
  } else if (threes > 0) {
    result.rank = HandRank::ThreeOfAKind;
    size_t ki = 1;
    result.kickers[0] = threeRank;
    for (auto r : ranks) {
      if (r != threeRank && ki < 5) {
        result.kickers[ki++] = r;
      }
    }
  } else if (pairs >= 2) {
    result.rank = HandRank::TwoPair;
    uint8_t kicker = 0;
    for (auto r : ranks) {
      if (r != pairRanks[0] && r != pairRanks[1]) {
        kicker = r;
        break;
      }
    }
    result.kickers = {pairRanks[0], pairRanks[1], kicker, 0, 0};
  } else if (pairs == 1) {
    result.rank = HandRank::Pair;
    result.kickers[0] = pairRanks[0];
    size_t ki = 1;
    for (auto r : ranks) {
      if (r != pairRanks[0] && ki < 5) {
        result.kickers[ki++] = r;
      }
    }
  } else {
    result.rank = HandRank::HighCard;
    result.kickers = {ranks[0], ranks[1], ranks[2], ranks[3], ranks[4]};
  }

  return result;
}

HandResult HandEvaluator::evaluate(std::span<const core::Card> cards) {
  if (cards.size() < 5 || cards.size() > 7) {
    throw std::invalid_argument("HandEvaluator::evaluate requires 5-7 cards");
  }

  if (cards.size() == 5) {
    std::array<core::Card, 5> arr;
    std::copy_n(cards.begin(), 5, arr.begin());
    return evaluate5(std::span<const core::Card, 5>(arr));
  }

  // Generate all C(n,5) combinations, keep the best.
  HandResult best;
  best.rank = HandRank::HighCard;
  best.kickers = {};

  const size_t n = cards.size();
  for (size_t a = 0; a < n - 4; ++a) {
    for (size_t b = a + 1; b < n - 3; ++b) {
      for (size_t c = b + 1; c < n - 2; ++c) {
        for (size_t d = c + 1; d < n - 1; ++d) {
          for (size_t e = d + 1; e < n; ++e) {
            std::array<core::Card, 5> combo = {cards[a], cards[b], cards[c],
                                               cards[d], cards[e]};
            auto result = evaluate5(std::span<const core::Card, 5>(combo));
            if (result > best) {
              best = result;
            }
          }
        }
      }
    }
  }

  return best;
}

int HandEvaluator::compare(std::span<const core::Card> hand1,
                           std::span<const core::Card> hand2) {
  auto r1 = evaluate(hand1);
  auto r2 = evaluate(hand2);
  if (r1 > r2)
    return 1;
  if (r1 < r2)
    return -1;
  return 0;
}

} // namespace poker::utils
