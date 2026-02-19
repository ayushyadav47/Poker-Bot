#include "utils/HandEvaluator.h"
#include <gtest/gtest.h>


using namespace poker::core;
using namespace poker::utils;

TEST(HandEvaluatorTest, RoyalFlush) {
  std::vector<Card> cards = {
      {Rank::Ace, Suit::Spades},   {Rank::King, Suit::Spades},
      {Rank::Queen, Suit::Spades}, {Rank::Jack, Suit::Spades},
      {Rank::Ten, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::RoyalFlush);
}

TEST(HandEvaluatorTest, StraightFlush) {
  std::vector<Card> cards = {
      {Rank::Nine, Suit::Hearts},  {Rank::Eight, Suit::Hearts},
      {Rank::Seven, Suit::Hearts}, {Rank::Six, Suit::Hearts},
      {Rank::Five, Suit::Hearts},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::StraightFlush);
}

TEST(HandEvaluatorTest, FourOfAKind) {
  std::vector<Card> cards = {
      {Rank::Ace, Suit::Spades},   {Rank::Ace, Suit::Hearts},
      {Rank::Ace, Suit::Diamonds}, {Rank::Ace, Suit::Clubs},
      {Rank::King, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::FourOfAKind);
}

TEST(HandEvaluatorTest, FullHouse) {
  std::vector<Card> cards = {
      {Rank::King, Suit::Spades},   {Rank::King, Suit::Hearts},
      {Rank::King, Suit::Diamonds}, {Rank::Queen, Suit::Clubs},
      {Rank::Queen, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::FullHouse);
}

TEST(HandEvaluatorTest, Flush) {
  std::vector<Card> cards = {
      {Rank::Ace, Suit::Clubs},   {Rank::Ten, Suit::Clubs},
      {Rank::Seven, Suit::Clubs}, {Rank::Five, Suit::Clubs},
      {Rank::Three, Suit::Clubs},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::Flush);
}

TEST(HandEvaluatorTest, Straight) {
  std::vector<Card> cards = {
      {Rank::Ten, Suit::Spades},     {Rank::Nine, Suit::Hearts},
      {Rank::Eight, Suit::Diamonds}, {Rank::Seven, Suit::Clubs},
      {Rank::Six, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::Straight);
}

TEST(HandEvaluatorTest, WheelStraight) {
  std::vector<Card> cards = {
      {Rank::Ace, Suit::Spades},     {Rank::Two, Suit::Hearts},
      {Rank::Three, Suit::Diamonds}, {Rank::Four, Suit::Clubs},
      {Rank::Five, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::Straight);
  EXPECT_EQ(result.kickers[0], 5); // 5-high straight
}

TEST(HandEvaluatorTest, ThreeOfAKind) {
  std::vector<Card> cards = {
      {Rank::Jack, Suit::Spades},   {Rank::Jack, Suit::Hearts},
      {Rank::Jack, Suit::Diamonds}, {Rank::Nine, Suit::Clubs},
      {Rank::Two, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::ThreeOfAKind);
}

TEST(HandEvaluatorTest, TwoPair) {
  std::vector<Card> cards = {
      {Rank::King, Suit::Spades},   {Rank::King, Suit::Hearts},
      {Rank::Nine, Suit::Diamonds}, {Rank::Nine, Suit::Clubs},
      {Rank::Five, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::TwoPair);
}

TEST(HandEvaluatorTest, Pair) {
  std::vector<Card> cards = {
      {Rank::Ace, Suit::Spades},    {Rank::Ace, Suit::Hearts},
      {Rank::King, Suit::Diamonds}, {Rank::Queen, Suit::Clubs},
      {Rank::Jack, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::Pair);
}

TEST(HandEvaluatorTest, HighCard) {
  std::vector<Card> cards = {
      {Rank::Ace, Suit::Spades},    {Rank::King, Suit::Hearts},
      {Rank::Nine, Suit::Diamonds}, {Rank::Seven, Suit::Clubs},
      {Rank::Two, Suit::Spades},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::HighCard);
}

TEST(HandEvaluatorTest, Best5From7) {
  // 7 cards: should find the flush among them.
  std::vector<Card> cards = {
      {Rank::Ace, Suit::Clubs},      {Rank::King, Suit::Clubs},
      {Rank::Ten, Suit::Clubs},      {Rank::Seven, Suit::Clubs},
      {Rank::Three, Suit::Clubs},    {Rank::Two, Suit::Hearts},
      {Rank::Queen, Suit::Diamonds},
  };
  auto result = HandEvaluator::evaluate(cards);
  EXPECT_EQ(result.rank, HandRank::Flush);
}

TEST(HandEvaluatorTest, CompareHands) {
  std::vector<Card> flush = {
      {Rank::Ace, Suit::Clubs},   {Rank::King, Suit::Clubs},
      {Rank::Ten, Suit::Clubs},   {Rank::Seven, Suit::Clubs},
      {Rank::Three, Suit::Clubs},
  };
  std::vector<Card> pair = {
      {Rank::Ace, Suit::Spades},    {Rank::Ace, Suit::Hearts},
      {Rank::King, Suit::Diamonds}, {Rank::Queen, Suit::Clubs},
      {Rank::Jack, Suit::Spades},
  };

  EXPECT_GT(HandEvaluator::compare(flush, pair), 0);
  EXPECT_LT(HandEvaluator::compare(pair, flush), 0);
}

TEST(HandEvaluatorTest, TieBreaking) {
  // Two pairs: KK99x vs KK88x. First should win.
  std::vector<Card> hand1 = {
      {Rank::King, Suit::Spades},   {Rank::King, Suit::Hearts},
      {Rank::Nine, Suit::Diamonds}, {Rank::Nine, Suit::Clubs},
      {Rank::Five, Suit::Spades},
  };
  std::vector<Card> hand2 = {
      {Rank::King, Suit::Diamonds}, {Rank::King, Suit::Clubs},
      {Rank::Eight, Suit::Spades},  {Rank::Eight, Suit::Hearts},
      {Rank::Five, Suit::Diamonds},
  };

  EXPECT_GT(HandEvaluator::compare(hand1, hand2), 0);
}
