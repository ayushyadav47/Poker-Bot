#include "engine/PokerEngine.h"
#include "utils/HandEvaluator.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace poker::engine {

PokerEngine::PokerEngine(
    std::shared_ptr<interfaces::IActionProvider> actionProvider,
    std::shared_ptr<interfaces::IRandomGenerator> rng)
    : actionProvider_(std::move(actionProvider)), rng_(std::move(rng)) {
  if (!actionProvider_)
    throw std::invalid_argument("actionProvider cannot be null");
  if (!rng_)
    throw std::invalid_argument("rng cannot be null");
}

void PokerEngine::setEventCallback(HandEventCallback callback) {
  eventCallback_ = std::move(callback);
}

void PokerEngine::playHand(core::GameState &state) {
  state.resetForNewHand();

  // Shuffle and deal.
  deck_.reset();
  deck_.shuffle(*rng_);

  emitEvent("hand_start", state);

  postBlinds(state);
  dealHoleCards(state);

  // Street progression: Preflop → Flop → Turn → River → Showdown
  core::Street streets[] = {core::Street::Preflop, core::Street::Flop,
                            core::Street::Turn, core::Street::River};

  for (auto street : streets) {
    state.setStreet(street);

    // Deal community cards for post-flop streets.
    if (street == core::Street::Flop) {
      dealCommunityCards(state, 3);
    } else if (street == core::Street::Turn || street == core::Street::River) {
      dealCommunityCards(state, 1);
    }

    emitEvent("street_" + std::string(street == core::Street::Preflop
                                          ? "preflop"
                                      : street == core::Street::Flop ? "flop"
                                      : street == core::Street::Turn ? "turn"
                                                                     : "river"),
              state);

    // Reset per-round bets (except preflop where blinds are already posted).
    if (street != core::Street::Preflop) {
      for (auto &p : state.getMutablePlayers()) {
        p.resetCurrentBet();
      }
    }

    runBettingRound(state);

    if (isHandOver(state)) {
      break;
    }
  }

  // Showdown / settle.
  state.setStreet(core::Street::Showdown);
  showdown(state);
}

void PokerEngine::postBlinds(core::GameState &state) {
  auto &players = state.getMutablePlayers();
  size_t sbPos = state.getSmallBlindPosition();
  size_t bbPos = state.getBigBlindPosition();

  int64_t sbAmount = players[sbPos].placeBet(state.getSmallBlind());
  state.getMutablePot().addContribution(sbPos, sbAmount);
  state.recordAction(core::Action(core::ActionType::Bet, sbAmount, sbPos));
  emitEvent("post_sb", state);

  int64_t bbAmount = players[bbPos].placeBet(state.getBigBlind());
  state.getMutablePot().addContribution(bbPos, bbAmount);
  state.recordAction(core::Action(core::ActionType::Bet, bbAmount, bbPos));
  emitEvent("post_bb", state);
}

void PokerEngine::dealHoleCards(core::GameState &state) {
  auto &players = state.getMutablePlayers();
  // Deal 2 cards to each player, starting left of dealer.
  for (int round = 0; round < 2; ++round) {
    for (size_t i = 0; i < players.size(); ++i) {
      size_t idx = (state.getDealerPosition() + 1 + i) % players.size();
      auto card = deck_.deal();
      if (card) {
        players[idx].dealCard(*card);
      }
    }
  }
  emitEvent("deal_hole_cards", state);
}

void PokerEngine::dealCommunityCards(core::GameState &state, size_t count) {
  // Burn one card.
  deck_.deal();
  for (size_t i = 0; i < count; ++i) {
    auto card = deck_.deal();
    if (card) {
      state.addCommunityCard(*card);
    }
  }
}

size_t PokerEngine::getFirstToAct(const core::GameState &state) const {
  const auto &players = state.getPlayers();
  size_t numPlayers = players.size();

  size_t startPos;
  if (state.getStreet() == core::Street::Preflop) {
    // First to act is left of BB.
    startPos = (state.getBigBlindPosition() + 1) % numPlayers;
  } else {
    // First to act is left of dealer.
    startPos = (state.getDealerPosition() + 1) % numPlayers;
  }

  // Find first active player.
  for (size_t i = 0; i < numPlayers; ++i) {
    size_t idx = (startPos + i) % numPlayers;
    if (!players[idx].isFolded() && !players[idx].isAllIn()) {
      return idx;
    }
  }
  return startPos;
}

void PokerEngine::runBettingRound(core::GameState &state) {
  auto &players = state.getMutablePlayers();
  size_t numPlayers = players.size();

  // Count active players (not folded and not all-in).
  size_t activePlayers = state.getNumActivePlayers();
  if (activePlayers <= 1)
    return;

  size_t firstToAct = getFirstToAct(state);
  size_t currentIdx = firstToAct;

  // Track who still needs to act.
  // Initially everyone who is active.
  std::unordered_set<size_t> needsToAct;
  for (size_t i = 0; i < numPlayers; ++i) {
    if (!players[i].isFolded() && !players[i].isAllIn()) {
      needsToAct.insert(i);
    }
  }

  // In preflop, BB has already put in their bet but still gets to act.
  bool firstIteration = true;

  while (!needsToAct.empty()) {
    // Skip folded, all-in players.
    if (players[currentIdx].isFolded() || players[currentIdx].isAllIn() ||
        needsToAct.find(currentIdx) == needsToAct.end()) {
      currentIdx = (currentIdx + 1) % numPlayers;

      // Safety: if we've gone all the way around, break.
      if (currentIdx == firstToAct && !firstIteration) {
        break;
      }
      firstIteration = false;
      continue;
    }
    firstIteration = false;

    state.setCurrentPlayerIndex(currentIdx);

    // Get legal actions and request action from provider.
    auto legalActions = RuleEngine::getLegalActions(state, currentIdx);
    if (legalActions.empty()) {
      needsToAct.erase(currentIdx);
      currentIdx = (currentIdx + 1) % numPlayers;
      continue;
    }

    auto action = actionProvider_->getAction(currentIdx, state, legalActions);
    action.playerId = currentIdx; // Ensure correct player ID.

    // Apply action.
    switch (action.type) {
    case core::ActionType::Fold:
      players[currentIdx].fold();
      break;

    case core::ActionType::Check:
      // No chips to place.
      break;

    case core::ActionType::Call: {
      int64_t actual = players[currentIdx].placeBet(action.amount);
      state.getMutablePot().addContribution(currentIdx, actual);
      break;
    }

    case core::ActionType::Bet:
    case core::ActionType::Raise: {
      int64_t actual = players[currentIdx].placeBet(action.amount);
      state.getMutablePot().addContribution(currentIdx, actual);
      // Everyone else needs to act again.
      needsToAct.clear();
      for (size_t i = 0; i < numPlayers; ++i) {
        if (i != currentIdx && !players[i].isFolded() &&
            !players[i].isAllIn()) {
          needsToAct.insert(i);
        }
      }
      break;
    }

    case core::ActionType::AllIn: {
      int64_t actual = players[currentIdx].placeBet(action.amount);
      state.getMutablePot().addContribution(currentIdx, actual);

      // If this is a raise (more than current bet level), reopen action.
      int64_t maxBet = 0;
      for (const auto &p : players) {
        maxBet = std::max(maxBet, p.getCurrentBet());
      }
      if (players[currentIdx].getCurrentBet() >= maxBet) {
        // This was the largest bet — reopen.
        needsToAct.clear();
        for (size_t i = 0; i < numPlayers; ++i) {
          if (i != currentIdx && !players[i].isFolded() &&
              !players[i].isAllIn()) {
            needsToAct.insert(i);
          }
        }
      }
      break;
    }
    }

    state.recordAction(action);
    emitEvent("action", state);

    needsToAct.erase(currentIdx);

    // Check if hand is over.
    if (state.getNumPlayersInHand() <= 1) {
      return;
    }

    currentIdx = (currentIdx + 1) % numPlayers;
  }
}

bool PokerEngine::isHandOver(const core::GameState &state) const {
  if (state.getNumPlayersInHand() <= 1)
    return true;
  if (state.getNumActivePlayers() <= 1) {
    // All but one (or zero) are all-in. Need to run out community cards.
    // But we still let the hand proceed to deal remaining cards.
    return state.getNumActivePlayers() == 0;
  }
  return false;
}

void PokerEngine::showdown(core::GameState &state) {
  // Deal remaining community cards if needed (e.g. all-in before river).
  while (state.getCommunityCards().size() < 5) {
    dealCommunityCards(state, 1);
  }

  emitEvent("showdown", state);

  settleHand(state);
}

void PokerEngine::settleHand(core::GameState &state) {
  auto &players = state.getMutablePlayers();

  // If only one player remains, they win everything.
  if (state.getNumPlayersInHand() == 1) {
    for (auto &p : players) {
      if (!p.isFolded()) {
        p.awardChips(state.getPot().getTotal());
        emitEvent("winner_" + p.getName(), state);
        break;
      }
    }
    return;
  }

  // Build folded set.
  std::unordered_set<size_t> foldedSet;
  for (const auto &p : players) {
    if (p.isFolded()) {
      foldedSet.insert(p.getId());
    }
  }

  // Calculate side pots.
  auto pots = state.getPot().calculateSidePots(foldedSet);

  // For each pot, determine winner(s).
  const auto &community = state.getCommunityCards();

  for (const auto &pot : pots) {
    if (pot.eligiblePlayers.empty())
      continue;

    // Evaluate hands for eligible players.
    struct PlayerHand {
      size_t playerId;
      utils::HandResult result;
    };
    std::vector<PlayerHand> hands;

    for (size_t pid : pot.eligiblePlayers) {
      // Combine hole cards + community cards.
      std::vector<core::Card> allCards;
      const auto &holeCards = players[pid].getHoleCards();
      allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
      allCards.insert(allCards.end(), community.begin(), community.end());

      auto result = utils::HandEvaluator::evaluate(allCards);
      hands.push_back({pid, result});
    }

    // Find best hand(s).
    std::sort(hands.begin(), hands.end(),
              [](const PlayerHand &a, const PlayerHand &b) {
                return a.result > b.result;
              });

    // Collect winners (ties).
    std::vector<size_t> winners;
    winners.push_back(hands[0].playerId);
    for (size_t i = 1; i < hands.size(); ++i) {
      if (hands[i].result == hands[0].result) {
        winners.push_back(hands[i].playerId);
      } else {
        break;
      }
    }

    // Split pot evenly among winners.
    int64_t share = pot.amount / static_cast<int64_t>(winners.size());
    int64_t remainder = pot.amount % static_cast<int64_t>(winners.size());

    for (size_t i = 0; i < winners.size(); ++i) {
      int64_t award = share + (static_cast<int64_t>(i) < remainder ? 1 : 0);
      players[winners[i]].awardChips(award);
    }

    emitEvent("pot_awarded", state);
  }
}

void PokerEngine::emitEvent(const std::string &event,
                            const core::GameState &state) {
  if (eventCallback_) {
    eventCallback_(event, state);
  }
}

} // namespace poker::engine
