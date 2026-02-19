#pragma once

#include <cstdint>
#include <ostream>
#include <string>


namespace poker::core {

/// Types of actions a player can take.
enum class ActionType : uint8_t { Fold, Check, Call, Bet, Raise, AllIn };

/// @brief Represents a single player action during a betting round.
struct Action {
  ActionType type;
  int64_t amount;  ///< Chip amount (0 for fold/check).
  size_t playerId; ///< Index of the acting player.

  constexpr Action() noexcept
      : type(ActionType::Fold), amount(0), playerId(0) {}

  constexpr Action(ActionType t, int64_t amt, size_t pid) noexcept
      : type(t), amount(amt), playerId(pid) {}

  [[nodiscard]] std::string toString() const;
  [[nodiscard]] static std::string actionTypeName(ActionType t);
};

inline std::ostream &operator<<(std::ostream &os, const Action &a) {
  return os << a.toString();
}

} // namespace poker::core
