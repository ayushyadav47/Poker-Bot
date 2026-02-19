#include "engine/PokerEngine.h"
#include "interfaces/IActionProvider.h"
#include "interfaces/IRandomGenerator.h"
#include <gtest/gtest.h>


using namespace poker::core;
using namespace poker::engine;

/// Deterministic RNG for testing.
class TestRNG : public poker::interfaces::IRandomGenerator {
public:
  explicit TestRNG(uint64_t seed) : engine_(seed) {}
  void shuffle(std::vector<Card> &cards) override {
    std::shuffle(cards.begin(), cards.end(), engine_);
  }

private:
  std::mt19937_64 engine_;
};

/// Action provider that always calls or checks (never folds or raises).
class PassiveActionProvider : public poker::interfaces::IActionProvider {
public:
  Action getAction(size_t /*playerId*/, const GameState & /*state*/,
                   const std::vector<Action> &legalActions) override {
    // Prefer Check, then Call, then first available.
    for (const auto &a : legalActions) {
      if (a.type == ActionType::Check)
        return a;
    }
    for (const auto &a : legalActions) {
      if (a.type == ActionType::Call)
        return a;
    }
    return legalActions.front();
  }
};

class PokerEngineTest : public ::testing::Test {
protected:
  std::shared_ptr<TestRNG> rng;
  std::shared_ptr<PassiveActionProvider> actionProvider;
  std::unique_ptr<PokerEngine> engine;
  GameState state;

  void SetUp() override {
    rng = std::make_shared<TestRNG>(42);
    actionProvider = std::make_shared<PassiveActionProvider>();
    engine = std::make_unique<PokerEngine>(actionProvider, rng);

    std::vector<Player> players;
    players.emplace_back(0, "Alice", 1000);
    players.emplace_back(1, "Bob", 1000);
    state.setPlayers(std::move(players));
    state.setSmallBlind(5);
    state.setBigBlind(10);
    state.setDealerPosition(0);
  }
};

TEST_F(PokerEngineTest, HandCompletes) {
  // A full hand should complete without crashing.
  ASSERT_NO_THROW(engine->playHand(state));
}

TEST_F(PokerEngineTest, ChipsConserved) {
  int64_t totalBefore = 0;
  for (const auto &p : state.getPlayers()) {
    totalBefore += p.getChips();
  }

  engine->playHand(state);

  int64_t totalAfter = 0;
  for (const auto &p : state.getPlayers()) {
    totalAfter += p.getChips();
  }

  EXPECT_EQ(totalBefore, totalAfter) << "Total chips must be conserved";
}

TEST_F(PokerEngineTest, CommunityCardsDealt) {
  engine->playHand(state);
  // After showdown, there should be 5 community cards.
  EXPECT_EQ(state.getCommunityCards().size(), 5u);
}

TEST_F(PokerEngineTest, PlayersHaveHoleCards) {
  engine->playHand(state);
  for (const auto &p : state.getPlayers()) {
    EXPECT_EQ(p.getHoleCards().size(), 2u);
  }
}

TEST_F(PokerEngineTest, MultipleHandsDeterministic) {
  // Play 5 hands with same seed — should be deterministic.
  auto rng2 = std::make_shared<TestRNG>(42);
  auto ap2 = std::make_shared<PassiveActionProvider>();
  PokerEngine engine2(ap2, rng2);

  GameState state2;
  std::vector<Player> players2;
  players2.emplace_back(0, "Alice", 1000);
  players2.emplace_back(1, "Bob", 1000);
  state2.setPlayers(std::move(players2));
  state2.setSmallBlind(5);
  state2.setBigBlind(10);
  state2.setDealerPosition(0);

  for (int i = 0; i < 5; ++i) {
    engine->playHand(state);
    engine2.playHand(state2);

    // Same seed + same strategy → same chip stacks.
    EXPECT_EQ(state.getPlayer(0).getChips(), state2.getPlayer(0).getChips());
    EXPECT_EQ(state.getPlayer(1).getChips(), state2.getPlayer(1).getChips());

    state.setDealerPosition((state.getDealerPosition() + 1) % 2);
    state2.setDealerPosition((state2.getDealerPosition() + 1) % 2);
  }
}

TEST_F(PokerEngineTest, EventCallbackFires) {
  int eventCount = 0;
  engine->setEventCallback(
      [&](const std::string &, const GameState &) { ++eventCount; });

  engine->playHand(state);
  EXPECT_GT(eventCount, 0) << "Events should fire during hand";
}
