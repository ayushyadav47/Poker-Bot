#include "core/Pot.h"

#include <algorithm>
#include <numeric>

namespace poker::core {

void Pot::addContribution(size_t playerId, int64_t amount) {
  for (auto &[pid, contrib] : contributions_) {
    if (pid == playerId) {
      contrib += amount;
      return;
    }
  }
  contributions_.emplace_back(playerId, amount);
}

int64_t Pot::getTotal() const noexcept {
  int64_t total = 0;
  for (const auto &[pid, contrib] : contributions_) {
    total += contrib;
  }
  return total;
}

int64_t Pot::getPlayerContribution(size_t playerId) const {
  for (const auto &[pid, contrib] : contributions_) {
    if (pid == playerId)
      return contrib;
  }
  return 0;
}

std::vector<PotInfo>
Pot::calculateSidePots(const std::unordered_set<size_t> &foldedPlayers) const {
  if (contributions_.empty())
    return {};

  // Collect unique contribution levels, sorted ascending.
  std::vector<int64_t> levels;
  for (const auto &[pid, contrib] : contributions_) {
    if (contrib > 0) {
      levels.push_back(contrib);
    }
  }
  std::sort(levels.begin(), levels.end());
  levels.erase(std::unique(levels.begin(), levels.end()), levels.end());

  std::vector<PotInfo> pots;
  int64_t prevLevel = 0;

  for (int64_t level : levels) {
    PotInfo pot;
    int64_t slicePerPlayer = level - prevLevel;

    for (const auto &[pid, contrib] : contributions_) {
      if (contrib >= level) {
        // This player contributed at least up to this level.
        pot.amount += slicePerPlayer;
        if (foldedPlayers.find(pid) == foldedPlayers.end()) {
          pot.eligiblePlayers.insert(pid);
        }
      } else if (contrib > prevLevel) {
        // Partial contribution in this slice.
        pot.amount += (contrib - prevLevel);
        if (foldedPlayers.find(pid) == foldedPlayers.end()) {
          pot.eligiblePlayers.insert(pid);
        }
      }
    }

    if (pot.amount > 0) {
      pots.push_back(std::move(pot));
    }
    prevLevel = level;
  }

  return pots;
}

void Pot::reset() { contributions_.clear(); }

} // namespace poker::core
