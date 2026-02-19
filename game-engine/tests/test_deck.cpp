#include "core/Deck.h"
#include "interfaces/IRandomGenerator.h"
#include <gtest/gtest.h>


#include <string>
#include <unordered_set>


using namespace poker::core;

/// Deterministic "no-shuffle" RNG for testing — leaves deck in original order.
class NoShuffleRNG : public poker::interfaces::IRandomGenerator {
public:
  void shuffle(std::vector<Card> & /*cards*/) override {
    // Do nothing — keep original order.
  }
};

TEST(DeckTest, Has52Cards) {
  Deck deck;
  size_t count = 0;
  while (deck.deal().has_value()) {
    ++count;
  }
  EXPECT_EQ(count, 52u);
}

TEST(DeckTest, AllUnique) {
  Deck deck;
  std::unordered_set<std::string> seen;
  while (auto card = deck.deal()) {
    auto str = card->toString();
    EXPECT_TRUE(seen.find(str) == seen.end()) << "Duplicate: " << str;
    seen.insert(str);
  }
  EXPECT_EQ(seen.size(), 52u);
}

TEST(DeckTest, DealReturnsNulloptWhenEmpty) {
  Deck deck;
  for (int i = 0; i < 52; ++i) {
    EXPECT_TRUE(deck.deal().has_value());
  }
  EXPECT_FALSE(deck.deal().has_value());
}

TEST(DeckTest, RemainingCount) {
  Deck deck;
  EXPECT_EQ(deck.remaining(), 52u);
  deck.deal();
  EXPECT_EQ(deck.remaining(), 51u);
}

TEST(DeckTest, ResetRestoresDeck) {
  Deck deck;
  for (int i = 0; i < 10; ++i)
    deck.deal();
  EXPECT_EQ(deck.remaining(), 42u);

  deck.reset();
  EXPECT_EQ(deck.remaining(), 52u);
}

TEST(DeckTest, DeterministicShuffle) {
  Deck deck1, deck2;
  Mt19937Generator rng1(42), rng2(42);

  deck1.shuffle(rng1);
  deck2.shuffle(rng2);

  // Same seed → same order.
  for (int i = 0; i < 52; ++i) {
    auto c1 = deck1.deal();
    auto c2 = deck2.deal();
    ASSERT_TRUE(c1.has_value());
    ASSERT_TRUE(c2.has_value());
    EXPECT_EQ(c1->toString(), c2->toString());
  }
}

TEST(DeckTest, NoShuffleRNG) {
  Deck deck;
  NoShuffleRNG rng;
  deck.shuffle(rng);

  // First card should be 2h (first in generation order).
  auto first = deck.deal();
  ASSERT_TRUE(first.has_value());
  EXPECT_EQ(first->rank, Rank::Two);
  EXPECT_EQ(first->suit, Suit::Hearts);
}
