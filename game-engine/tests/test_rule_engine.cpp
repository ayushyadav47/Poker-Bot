#include "core/GameState.h"
#include "core/Player.h"
#include "engine/RuleEngine.h"
#include <gtest/gtest.h>


using namespace poker::core;
using namespace poker::engine;

class RuleEngineTest : public ::testing::Test {
protected:
  GameState state;

  void SetUp() override {
    std::vector<Player> players;
    players.emplace_back(0, "Alice", 1000);
    players.emplace_back(1, "Bob", 1000);
    state.setPlayers(std::move(players));
    state.setSmallBlind(5);
    state.setBigBlind(10);
    state.setDealerPosition(0);
    state.setStreet(Street::Preflop);
  }
};

TEST_F(RuleEngineTest, FoldAlwaysLegal) {
  // Post blinds first.
  state.getMutablePlayer(0).placeBet(5);  // SB
  state.getMutablePlayer(1).placeBet(10); // BB

  auto actions = RuleEngine::getLegalActions(state, 0);
  bool hasFold = false;
  for (const auto &a : actions) {
    if (a.type == ActionType::Fold) {
      hasFold = true;
      break;
    }
  }
  EXPECT_TRUE(hasFold);
}

TEST_F(RuleEngineTest, CheckWhenNoBet) {
  // Postflop, no bets placed.
  state.setStreet(Street::Flop);
  auto actions = RuleEngine::getLegalActions(state, 0);
  bool hasCheck = false;
  for (const auto &a : actions) {
    if (a.type == ActionType::Check) {
      hasCheck = true;
      break;
    }
  }
  EXPECT_TRUE(hasCheck);
}

TEST_F(RuleEngineTest, CallWhenBetFacing) {
  // Player 0 bet 100, Player 1 should have Call available.
  state.setStreet(Street::Flop);
  state.getMutablePlayer(0).placeBet(100);

  auto actions = RuleEngine::getLegalActions(state, 1);
  bool hasCall = false;
  for (const auto &a : actions) {
    if (a.type == ActionType::Call) {
      hasCall = true;
      break;
    }
  }
  EXPECT_TRUE(hasCall);
}

TEST_F(RuleEngineTest, CallAmountCorrect) {
  state.setStreet(Street::Flop);
  state.getMutablePlayer(0).placeBet(100);

  int64_t callAmt = RuleEngine::getCallAmount(state, 1);
  EXPECT_EQ(callAmt, 100);
}

TEST_F(RuleEngineTest, FoldedPlayerNoActions) {
  state.getMutablePlayer(0).fold();
  auto actions = RuleEngine::getLegalActions(state, 0);
  EXPECT_TRUE(actions.empty());
}

TEST_F(RuleEngineTest, AllInWhenNotEnoughChips) {
  // Player with only 5 chips facing a 100 bet.
  std::vector<Player> players;
  players.emplace_back(0, "Alice", 1000);
  players.emplace_back(1, "Bob", 5);
  state.setPlayers(std::move(players));
  state.setStreet(Street::Flop);
  state.getMutablePlayer(0).placeBet(100);

  auto actions = RuleEngine::getLegalActions(state, 1);
  bool hasAllIn = false;
  for (const auto &a : actions) {
    if (a.type == ActionType::AllIn) {
      hasAllIn = true;
      break;
    }
  }
  EXPECT_TRUE(hasAllIn);

  // Should NOT have Call (can't afford full call).
  bool hasCall = false;
  for (const auto &a : actions) {
    if (a.type == ActionType::Call) {
      hasCall = true;
      break;
    }
  }
  EXPECT_FALSE(hasCall);
}
