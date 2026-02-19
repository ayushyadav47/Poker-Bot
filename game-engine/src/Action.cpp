#include "core/Action.h"

namespace poker::core {

std::string Action::actionTypeName(ActionType t) {
  switch (t) {
  case ActionType::Fold:
    return "Fold";
  case ActionType::Check:
    return "Check";
  case ActionType::Call:
    return "Call";
  case ActionType::Bet:
    return "Bet";
  case ActionType::Raise:
    return "Raise";
  case ActionType::AllIn:
    return "All-In";
  }
  return "Unknown";
}

std::string Action::toString() const {
  std::string s =
      "Player " + std::to_string(playerId) + ": " + actionTypeName(type);
  if (amount > 0) {
    s += " " + std::to_string(amount);
  }
  return s;
}

} // namespace poker::core
