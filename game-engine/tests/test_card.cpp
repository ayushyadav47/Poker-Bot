#include "core/Card.h"
#include <gtest/gtest.h>


using namespace poker::core;

TEST(CardTest, Construction) {
  Card c(Rank::Ace, Suit::Spades);
  EXPECT_EQ(c.rank, Rank::Ace);
  EXPECT_EQ(c.suit, Suit::Spades);
}

TEST(CardTest, DefaultConstruction) {
  Card c;
  EXPECT_EQ(c.rank, Rank::Two);
  EXPECT_EQ(c.suit, Suit::Hearts);
}

TEST(CardTest, ToString) {
  EXPECT_EQ(Card(Rank::Ace, Suit::Spades).toString(), "As");
  EXPECT_EQ(Card(Rank::Ten, Suit::Hearts).toString(), "Th");
  EXPECT_EQ(Card(Rank::Two, Suit::Clubs).toString(), "2c");
  EXPECT_EQ(Card(Rank::King, Suit::Diamonds).toString(), "Kd");
  EXPECT_EQ(Card(Rank::Jack, Suit::Hearts).toString(), "Jh");
  EXPECT_EQ(Card(Rank::Queen, Suit::Clubs).toString(), "Qc");
}

TEST(CardTest, Equality) {
  Card c1(Rank::Ace, Suit::Spades);
  Card c2(Rank::Ace, Suit::Spades);
  Card c3(Rank::Ace, Suit::Hearts);

  EXPECT_EQ(c1, c2);
  EXPECT_NE(c1, c3); // Same rank, different suit
}

TEST(CardTest, Ordering) {
  Card two(Rank::Two, Suit::Hearts);
  Card ace(Rank::Ace, Suit::Spades);
  Card king(Rank::King, Suit::Clubs);

  EXPECT_LT(two, ace);
  EXPECT_LT(king, ace);
  EXPECT_GT(ace, two);
}

TEST(CardTest, RankChar) {
  EXPECT_EQ(Card::rankChar(Rank::Two), '2');
  EXPECT_EQ(Card::rankChar(Rank::Ten), 'T');
  EXPECT_EQ(Card::rankChar(Rank::Ace), 'A');
}

TEST(CardTest, SuitChar) {
  EXPECT_EQ(Card::suitChar(Suit::Hearts), 'h');
  EXPECT_EQ(Card::suitChar(Suit::Diamonds), 'd');
  EXPECT_EQ(Card::suitChar(Suit::Clubs), 'c');
  EXPECT_EQ(Card::suitChar(Suit::Spades), 's');
}
