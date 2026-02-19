#include "core/Card.h"

namespace poker::core {

std::string Card::toString() const {
  return std::string(1, rankChar(rank)) + suitChar(suit);
}

char Card::rankChar(Rank r) noexcept {
  switch (r) {
  case Rank::Two:
    return '2';
  case Rank::Three:
    return '3';
  case Rank::Four:
    return '4';
  case Rank::Five:
    return '5';
  case Rank::Six:
    return '6';
  case Rank::Seven:
    return '7';
  case Rank::Eight:
    return '8';
  case Rank::Nine:
    return '9';
  case Rank::Ten:
    return 'T';
  case Rank::Jack:
    return 'J';
  case Rank::Queen:
    return 'Q';
  case Rank::King:
    return 'K';
  case Rank::Ace:
    return 'A';
  }
  return '?';
}

char Card::suitChar(Suit s) noexcept {
  switch (s) {
  case Suit::Hearts:
    return 'h';
  case Suit::Diamonds:
    return 'd';
  case Suit::Clubs:
    return 'c';
  case Suit::Spades:
    return 's';
  }
  return '?';
}

} // namespace poker::core
