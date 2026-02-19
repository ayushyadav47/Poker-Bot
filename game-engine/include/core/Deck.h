#pragma once

#include "core/Card.h"
#include "interfaces/IRandomGenerator.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <random>
#include <vector>


namespace poker::core {

/// @brief Standard 52-card deck with injectable RNG.
class Deck {
public:
  /// Construct a full 52-card deck (unshuffled).
  Deck();

  /// Shuffle using the provided RNG.
  void shuffle(interfaces::IRandomGenerator &rng);

  /// Deal one card from the top. Returns nullopt if empty.
  [[nodiscard]] std::optional<Card> deal();

  /// Reset to a full 52-card deck (unshuffled).
  void reset();

  /// Number of remaining cards.
  [[nodiscard]] size_t remaining() const noexcept;

private:
  std::vector<Card> cards_;
  size_t dealIndex_ = 0;
};

/// @brief Default RNG implementation using std::mt19937.
class Mt19937Generator : public interfaces::IRandomGenerator {
public:
  explicit Mt19937Generator(uint64_t seed);
  Mt19937Generator(); ///< Seeds from std::random_device.

  void shuffle(std::vector<Card> &cards) override;

private:
  std::mt19937_64 engine_;
};

} // namespace poker::core
