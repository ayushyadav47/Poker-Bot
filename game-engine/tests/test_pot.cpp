#include "core/Pot.h"
#include <gtest/gtest.h>


#include <unordered_set>

using namespace poker::core;

TEST(PotTest, BasicContribution) {
  Pot pot;
  pot.addContribution(0, 100);
  pot.addContribution(1, 100);
  EXPECT_EQ(pot.getTotal(), 200);
}

TEST(PotTest, PlayerContribution) {
  Pot pot;
  pot.addContribution(0, 50);
  pot.addContribution(1, 100);
  pot.addContribution(0, 50);

  EXPECT_EQ(pot.getPlayerContribution(0), 100);
  EXPECT_EQ(pot.getPlayerContribution(1), 100);
}

TEST(PotTest, SidePotSimple) {
  // Player 0 goes all-in for 50, Player 1 calls 50, Player 2 calls 50.
  Pot pot;
  pot.addContribution(0, 50);
  pot.addContribution(1, 50);
  pot.addContribution(2, 50);

  std::unordered_set<size_t> folded;
  auto pots = pot.calculateSidePots(folded);

  ASSERT_EQ(pots.size(), 1u);
  EXPECT_EQ(pots[0].amount, 150);
  EXPECT_EQ(pots[0].eligiblePlayers.size(), 3u);
}

TEST(PotTest, SidePotWithAllIn) {
  // Player 0: all-in 50, Player 1: 100, Player 2: 100
  Pot pot;
  pot.addContribution(0, 50);
  pot.addContribution(1, 100);
  pot.addContribution(2, 100);

  std::unordered_set<size_t> folded;
  auto pots = pot.calculateSidePots(folded);

  // Main pot: 50 x 3 = 150 (all 3 eligible)
  // Side pot: 50 x 2 = 100 (players 1 and 2 only)
  ASSERT_EQ(pots.size(), 2u);
  EXPECT_EQ(pots[0].amount, 150);
  EXPECT_EQ(pots[0].eligiblePlayers.size(), 3u);
  EXPECT_EQ(pots[1].amount, 100);
  EXPECT_EQ(pots[1].eligiblePlayers.size(), 2u);
}

TEST(PotTest, SidePotWithFolded) {
  // Player 0: 50 (folded), Player 1: 100, Player 2: 100
  Pot pot;
  pot.addContribution(0, 50);
  pot.addContribution(1, 100);
  pot.addContribution(2, 100);

  std::unordered_set<size_t> folded = {0};
  auto pots = pot.calculateSidePots(folded);

  // Player 0 contributed but is ineligible.
  ASSERT_GE(pots.size(), 1u);
  for (const auto &p : pots) {
    EXPECT_TRUE(p.eligiblePlayers.find(0) == p.eligiblePlayers.end())
        << "Folded player should not be eligible";
  }
}

TEST(PotTest, MultipleAllIns) {
  // Player 0: all-in 30, Player 1: all-in 60, Player 2: 100
  Pot pot;
  pot.addContribution(0, 30);
  pot.addContribution(1, 60);
  pot.addContribution(2, 100);

  std::unordered_set<size_t> folded;
  auto pots = pot.calculateSidePots(folded);

  // Main pot: 30 x 3 = 90
  // Side pot 1: 30 x 2 = 60  (players 1, 2)
  // Side pot 2: 40 x 1 = 40  (player 2 only)
  ASSERT_EQ(pots.size(), 3u);
  EXPECT_EQ(pots[0].amount, 90);
  EXPECT_EQ(pots[1].amount, 60);
  EXPECT_EQ(pots[2].amount, 40);
}

TEST(PotTest, Reset) {
  Pot pot;
  pot.addContribution(0, 100);
  pot.reset();
  EXPECT_EQ(pot.getTotal(), 0);
}
