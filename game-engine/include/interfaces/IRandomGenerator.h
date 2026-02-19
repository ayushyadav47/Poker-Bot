#pragma once

#include <vector>

// Forward declaration
namespace poker::core {
struct Card;
}

namespace poker::interfaces {

/// @brief Abstract interface for random number generation.
/// Inject a concrete implementation to control shuffling behavior.
/// Use the default Mt19937Generator for production, or a deterministic
/// implementation for testing and CFR training.
class IRandomGenerator {
public:
  virtual ~IRandomGenerator() = default;

  /// Shuffle a vector of cards in-place.
  virtual void shuffle(std::vector<poker::core::Card> &cards) = 0;
};

} // namespace poker::interfaces
