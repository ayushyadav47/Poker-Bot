#pragma once

#include <compare>
#include <cstdint>
#include <ostream>
#include <string>


namespace poker::core {

/// Four standard suits.
enum class Suit : uint8_t { Hearts = 0, Diamonds = 1, Clubs = 2, Spades = 3 };

/// Card ranks, Two (2) through Ace (14).
enum class Rank : uint8_t {
  Two = 2,
  Three = 3,
  Four = 4,
  Five = 5,
  Six = 6,
  Seven = 7,
  Eight = 8,
  Nine = 9,
  Ten = 10,
  Jack = 11,
  Queen = 12,
  King = 13,
  Ace = 14
};

/// @brief Immutable value type representing a single playing card.
struct Card {
  Rank rank;
  Suit suit;

  constexpr Card() noexcept : rank(Rank::Two), suit(Suit::Hearts) {}
  constexpr Card(Rank r, Suit s) noexcept : rank(r), suit(s) {}

  /// Compare by rank only (suit is irrelevant for hand evaluation ordering).
  constexpr auto operator<=>(const Card &other) const noexcept {
    return rank <=> other.rank;
  }
  constexpr bool operator==(const Card &other) const noexcept {
    return rank == other.rank && suit == other.suit;
  }

  /// Human-readable string, e.g. "As", "Td", "2c".
  [[nodiscard]] std::string toString() const;

  /// Rank character: '2'..'9','T','J','Q','K','A'.
  [[nodiscard]] static char rankChar(Rank r) noexcept;

  /// Suit character: 'h','d','c','s'.
  [[nodiscard]] static char suitChar(Suit s) noexcept;
};

inline std::ostream &operator<<(std::ostream &os, const Card &c) {
  return os << c.toString();
}

} // namespace poker::core
